/**
 * VideoSpeed Zygisk module - pure C approach (no C++ STL)
 * Hooks libxffmpeg.so avfilter_init_str / av_opt_set_double
 * for WeChat Channels (视频号) speed control
 */
#include <sys/types.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>
#include "zygisk.hpp"

#define TAG "VideoSpeedNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ── Config ─────────────────────────────────────────────────────────────────────

static float g_target_speed = 1.0f;
static bool  g_speed_loaded  = false;

static float loadTargetSpeed() {
    if (g_speed_loaded) return g_target_speed;
    const char* paths[] = {
        "/data/user/0/io.github.MarsGao.speed/shared_prefs/speed.xml",
        "/data/data/io.github.MarsGao.speed/shared_prefs/speed.xml",
        nullptr
    };
    for (int i = 0; paths[i]; i++) {
        FILE* f = fopen(paths[i], "r");
        if (!f) continue;
        char buf[512] = {};
        fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
        const char* pos = strstr(buf, "name=\"speed\" value=\"");
        if (pos) {
            float v = (float)strtod(pos + 20, nullptr);
            if (v > 0.1f && v < 10.0f) {
                g_target_speed = v;
                g_speed_loaded = true;
                LOGI("Loaded target speed: %.2f", v);
                return v;
            }
        }
    }
    g_target_speed = 1.0f;
    g_speed_loaded = true;
    return 1.0f;
}

// ── ARM64 GOT/PLT hook (simpler than inline patching) ─────────────────────────
// We use dlopen + dlsym and replace the function by patching the first 4 bytes
// with a direct branch to our hook. Only works for functions ≥ 8 bytes.

typedef int (*avfilter_init_str_t)(void*, const char*);
typedef int (*av_opt_set_double_t)(void*, const char*, double, int);
typedef int (*av_opt_set_t)(void*, const char*, const char*, int);

static avfilter_init_str_t  real_avfilter_init_str  = nullptr;
static av_opt_set_double_t  real_av_opt_set_double  = nullptr;
static av_opt_set_t         real_av_opt_set         = nullptr;
static bool                 g_hooked                 = false;

// We cannot safely do inline hooks without a proper trampoline library.
// Instead we use GOT replacement via xhook-style PLT patching.
// Since we don't have xhook, we use a simpler approach:
// intercept via LD_PRELOAD / function interposition is not possible here.
//
// Alternative: hook the CALLER's PLT entries in libmmmedia.so.
// For now, we use a minimal inline hook for av_opt_set_double which is
// the smallest and most targeted function.

// Minimal inline hook for ARM64: patch the first 16 bytes of the target
// with LDR X16, #8 ; BR X16 ; .quad hook_addr
// The original first 4 instructions are lost - we ONLY use afterHook model
// (call our hook first, then use a separate path to call original if possible)

static bool g_use_interposition = false; // track if we're in hook to avoid re-entry

// For avfilter_init_str - intercept when tempo= filter is initialized
// We call the real function from a saved pointer
static uint8_t saved_init_str[16]   = {};
static uint8_t saved_opt_double[16] = {};

static bool patchFn(void* fn, void* hook, uint8_t* saved_bytes) {
    uintptr_t addr = (uintptr_t)fn;
    uintptr_t page = addr & ~(uintptr_t)(4095);
    size_t    size = 4096 * 2;

    if (mprotect((void*)page, size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
        return false;

    memcpy(saved_bytes, fn, 16);

    // ARM64: LDR X16, #8 (load 8-byte address at PC+8)
    //        BR X16       (branch to loaded address)
    //        <8 bytes: hook address>
    uint32_t* p = (uint32_t*)fn;
    uint64_t  h = (uint64_t)hook;
    p[0] = 0x58000050u; // LDR X16, #8
    p[1] = 0xD61F0200u; // BR X16
    memcpy(p + 2, &h, 8);
    __builtin___clear_cache((char*)fn, (char*)fn + 16);
    return true;
}

// Hook implementations - these are called INSTEAD of the originals
// The original function body is patched, so we can't call it via its symbol.
// We need to restore+call+patch approach OR use a separate trampoline.
// Here we use a static "call count" to detect re-entry and skip.

static int hook_avfilter_init_str(void* ctx, const char* args) {
    float target = loadTargetSpeed();
    if (args && strncmp(args, "tempo=", 6) == 0 && fabsf(target - 1.0f) > 0.01f) {
        char newArgs[64];
        float speed = target < 0.5f ? 0.5f : (target > 2.0f ? 2.0f : target);
        snprintf(newArgs, sizeof(newArgs), "tempo=%f", speed);
        LOGI("avfilter_init_str atempo: '%s' -> '%s'", args, newArgs);
        args = newArgs;
    }
    // Call original via saved pointer (which points to the now-patched function)
    // We temporarily restore the original bytes, call, then re-patch.
    // This is thread-unsafe but works for single-threaded initialization.
    void* fn = (void*)real_avfilter_init_str;
    uintptr_t page = (uintptr_t)fn & ~(uintptr_t)(4095);
    mprotect((void*)page, 8192, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(fn, saved_init_str, 16);
    __builtin___clear_cache((char*)fn, (char*)fn + 16);
    int ret = real_avfilter_init_str(ctx, args);
    patchFn(fn, (void*)hook_avfilter_init_str, saved_init_str);
    return ret;
}

static int hook_av_opt_set_double(void* obj, const char* name, double val, int flags) {
    if (name && strcmp(name, "tempo") == 0) {
        float target = loadTargetSpeed();
        if (fabsf(target - 1.0f) > 0.01f) {
            double clamped = target < 0.5 ? 0.5 : (target > 2.0 ? 2.0 : (double)target);
            LOGI("av_opt_set_double tempo: %f -> %f", val, clamped);
            val = clamped;
        }
    }
    void* fn = (void*)real_av_opt_set_double;
    uintptr_t page = (uintptr_t)fn & ~(uintptr_t)(4095);
    mprotect((void*)page, 8192, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(fn, saved_opt_double, 16);
    __builtin___clear_cache((char*)fn, (char*)fn + 16);
    int ret = real_av_opt_set_double(obj, name, val, flags);
    patchFn(fn, (void*)hook_av_opt_set_double, saved_opt_double);
    return ret;
}

static void hookLibxffmpeg() {
    void* h = dlopen("libxffmpeg.so", RTLD_NOW | RTLD_NOLOAD);
    if (!h) h = dlopen("libxffmpeg.so", RTLD_NOW);
    if (!h) {
        LOGE("Cannot open libxffmpeg.so: %s", dlerror());
        return;
    }

    void* fn_init = dlsym(h, "avfilter_init_str");
    void* fn_optd = dlsym(h, "av_opt_set_double");

    LOGI("avfilter_init_str=%p av_opt_set_double=%p", fn_init, fn_optd);

    if (fn_init) {
        real_avfilter_init_str = (avfilter_init_str_t)fn_init;
        if (patchFn(fn_init, (void*)hook_avfilter_init_str, saved_init_str))
            LOGI("Hooked avfilter_init_str");
    }
    if (fn_optd) {
        real_av_opt_set_double = (av_opt_set_double_t)fn_optd;
        if (patchFn(fn_optd, (void*)hook_av_opt_set_double, saved_opt_double))
            LOGI("Hooked av_opt_set_double");
    }

    dlclose(h);
    g_hooked = true;
    LOGI("WeChat hooks installed, target speed=%.2f", loadTargetSpeed());
}

// ── Zygisk Module ──────────────────────────────────────────────────────────────

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class VideoSpeedModule : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGI("onLoad called");
    }

    void postAppSpecialize(const AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) return;
        const char* proc = env->GetStringUTFChars(args->nice_name, nullptr);
        bool isWeChat = proc && strcmp(proc, "com.tencent.mm") == 0;
        if (proc) env->ReleaseStringUTFChars(args->nice_name, proc);

        if (isWeChat && !g_hooked) {
            LOGI("WeChat detected, hooking...");
            hookLibxffmpeg();
        }
    }

    void preServerSpecialize(ServerSpecializeArgs* args) override {
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

private:
    Api*    api = nullptr;
    JNIEnv* env = nullptr;
};

REGISTER_ZYGISK_MODULE(VideoSpeedModule)
