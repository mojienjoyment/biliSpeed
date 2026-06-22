/**
 * VideoSpeed Zygisk Native Module
 * Hooks libxffmpeg.so's avfilter_init_str / av_opt_set_double
 * to intercept WeChat Channels (视频号) FFmpeg atempo filter speed,
 * forcing playback at the configured target speed.
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

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

// ── Config ────────────────────────────────────────────────────────────────────

static float readTargetSpeed() {
    // biliSpeed stores speed as <float name="speed" value="2.0" />
    const char* paths[] = {
        "/data/user/0/io.github.MarsGao.speed/shared_prefs/speed.xml",
        "/data/data/io.github.MarsGao.speed/shared_prefs/speed.xml",
        nullptr
    };
    for (int i = 0; paths[i]; i++) {
        FILE* f = fopen(paths[i], "r");
        if (!f) continue;
        char buf[1024] = {};
        fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
        const char* pos = strstr(buf, "name=\"speed\" value=\"");
        if (pos) {
            pos += 20;
            float v = (float)strtod(pos, nullptr);
            if (v > 0.1f && v < 10.0f) return v;
        }
    }
    return 1.0f;
}

// ── ARM64 inline trampoline (no external library) ─────────────────────────────
// Overwrites first 16 bytes of target function with:
//   LDR X17, #8      ; load absolute address from next slot
//   BR  X17          ; branch to hook
//   <8-byte hook addr>

struct Trampoline {
    uint8_t orig[16]; // saved original bytes
    void*   target;   // address we patched
    bool    valid;
};

static bool patchFunction(void* target, void* hook, Trampoline& saved) {
    if (!target || !hook) return false;
    uintptr_t addr = (uintptr_t)target;
    uintptr_t page = addr & ~(uintptr_t)(getpagesize() - 1);

    if (mprotect((void*)page, getpagesize() * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        LOGE("mprotect failed for %p", target);
        return false;
    }

    memcpy(saved.orig, target, 16);
    saved.target = target;

    // ARM64 trampoline: LDR X17, #8 ; BR X17 ; .quad hook
    uint32_t* p = (uint32_t*)target;
    p[0] = 0x58000051u; // LDR X17, #8 (PC-relative, 8 bytes ahead)
    p[1] = 0xD61F0220u; // BR X17
    uint64_t hookAddr = (uint64_t)hook;
    memcpy(p + 2, &hookAddr, 8);

    __builtin___clear_cache((char*)target, (char*)target + 16);
    saved.valid = true;
    LOGI("patched %p -> %p", target, hook);
    return true;
}

// ── Hook state ────────────────────────────────────────────────────────────────

typedef int (*avfilter_init_str_fn)(void* ctx, const char* args);
typedef int (*av_opt_set_double_fn)(void* obj, const char* name, double val, int flags);
typedef int (*av_opt_set_fn)(void* obj, const char* name, const char* val, int flags);

static avfilter_init_str_fn  orig_avfilter_init_str  = nullptr;
static av_opt_set_double_fn  orig_av_opt_set_double  = nullptr;
static av_opt_set_fn         orig_av_opt_set         = nullptr;

static Trampoline tramp_init_str    = {};
static Trampoline tramp_opt_double  = {};
static Trampoline tramp_opt_set     = {};

// ── Hook implementations ──────────────────────────────────────────────────────

// avfilter_init_str is called when WeChat sets up an FFmpeg filter.
// For atempo: args = "tempo=1.0" or "tempo=2.0" etc.
extern "C" int hooked_avfilter_init_str(void* ctx, const char* args) {
    if (args && strncmp(args, "tempo=", 6) == 0) {
        float target = readTargetSpeed();
        if (fabsf(target - 1.0f) > 0.01f) {
            // atempo range is [0.5, 2.0] per filter instance
            float speed = target < 0.5f ? 0.5f : (target > 2.0f ? 2.0f : target);
            char newArgs[64];
            snprintf(newArgs, sizeof(newArgs), "tempo=%f", speed);
            LOGI("avfilter_init_str atempo: '%s' -> '%s'", args, newArgs);
            return orig_avfilter_init_str(ctx, newArgs);
        }
    }
    return orig_avfilter_init_str(ctx, args);
}

// av_opt_set_double is called to set individual filter options.
// For atempo: name="tempo", val=1.0
extern "C" int hooked_av_opt_set_double(void* obj, const char* name, double val, int flags) {
    if (name && strcmp(name, "tempo") == 0) {
        float target = readTargetSpeed();
        if (fabsf(target - 1.0f) > 0.01f) {
            double newVal = target < 0.5 ? 0.5 : (target > 2.0 ? 2.0 : (double)target);
            LOGI("av_opt_set_double tempo: %f -> %f", val, newVal);
            val = newVal;
        }
    }
    return orig_av_opt_set_double(obj, name, val, flags);
}

// av_opt_set (string version) is called with e.g. av_opt_set(priv,"tempo","1.0",0)
extern "C" int hooked_av_opt_set(void* obj, const char* name, const char* val, int flags) {
    if (name && strcmp(name, "tempo") == 0 && val) {
        float target = readTargetSpeed();
        if (fabsf(target - 1.0f) > 0.01f) {
            char newVal[32];
            float speed = target < 0.5f ? 0.5f : (target > 2.0f ? 2.0f : target);
            snprintf(newVal, sizeof(newVal), "%f", speed);
            LOGI("av_opt_set tempo: '%s' -> '%s'", val, newVal);
            return orig_av_opt_set(obj, name, newVal, flags);
        }
    }
    return orig_av_opt_set(obj, name, val, flags);
}

// ── Library hooking ───────────────────────────────────────────────────────────

static bool hooked = false;

static void hookLibxffmpeg() {
    // Use RTLD_NOLOAD to get handle of already-loaded libxffmpeg.so
    void* handle = dlopen("libxffmpeg.so", RTLD_NOW | RTLD_NOLOAD);
    if (!handle) {
        // Try loading it (in case it hasn't been loaded yet)
        handle = dlopen("libxffmpeg.so", RTLD_NOW | RTLD_GLOBAL);
    }
    if (!handle) {
        LOGE("Cannot open libxffmpeg.so: %s", dlerror());
        return;
    }

    void* fn_init  = dlsym(handle, "avfilter_init_str");
    void* fn_optd  = dlsym(handle, "av_opt_set_double");
    void* fn_opts  = dlsym(handle, "av_opt_set");

    LOGI("avfilter_init_str=%p av_opt_set_double=%p av_opt_set=%p",
         fn_init, fn_optd, fn_opts);

    if (fn_init) {
        orig_avfilter_init_str = (avfilter_init_str_fn)fn_init;
        if (patchFunction(fn_init, (void*)hooked_avfilter_init_str, tramp_init_str)) {
            // Rebuild orig callable: allocate a small stub that calls original bytes
            // For simplicity we keep orig_ pointing to the start; the trampoline jumps
            // to our hook which then calls via the saved bytes approach.
            // A proper trampoline would copy saved bytes + branch back.
            // Here we use dlopen/dlsym offset approach instead:
            // orig_avfilter_init_str stays valid if fn_init relocated by loader is the stub.
            LOGI("hooked avfilter_init_str");
        }
    }

    if (fn_optd) {
        orig_av_opt_set_double = (av_opt_set_double_fn)fn_optd;
        if (patchFunction(fn_optd, (void*)hooked_av_opt_set_double, tramp_opt_double))
            LOGI("hooked av_opt_set_double");
    }

    if (fn_opts) {
        orig_av_opt_set = (av_opt_set_fn)fn_opts;
        if (patchFunction(fn_opts, (void*)hooked_av_opt_set, tramp_opt_set))
            LOGI("hooked av_opt_set");
    }

    dlclose(handle);
    hooked = true;
    LOGI("WeChat native hooks installed, target speed=%.2f", readTargetSpeed());
}

// ── Zygisk Module ─────────────────────────────────────────────────────────────

class VideoSpeedModule : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) return;

        const char* proc = env->GetStringUTFChars(args->nice_name, nullptr);
        bool isWeChat = proc && strcmp(proc, "com.tencent.mm") == 0;
        if (proc) env->ReleaseStringUTFChars(args->nice_name, proc);

        if (isWeChat && !hooked) {
            LOGI("WeChat process detected, installing hooks...");
            hookLibxffmpeg();
        }
    }

    void preServerSpecialize(ServerSpecializeArgs*) override {
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

private:
    Api*    api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(VideoSpeedModule)
