// Minimal Zygisk module - just log on load to confirm injection works
#include <sys/types.h>
#include <android/log.h>
#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class MinimalModule : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        __android_log_print(ANDROID_LOG_INFO, "VideoSpeedTest", "MinimalModule loaded!");
    }
    void postAppSpecialize(const AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) return;
        const char* proc = env->GetStringUTFChars(args->nice_name, nullptr);
        __android_log_print(ANDROID_LOG_INFO, "VideoSpeedTest", "postAppSpecialize: %s", proc ? proc : "null");
        if (proc) env->ReleaseStringUTFChars(args->nice_name, proc);
    }
    void preServerSpecialize(ServerSpecializeArgs*) override {
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }
private:
    Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MinimalModule)
