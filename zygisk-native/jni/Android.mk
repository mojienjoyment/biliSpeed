LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE           := videospeed_zygisk
LOCAL_SRC_FILES        := module_c.cpp crt_stubs.cpp
LOCAL_C_INCLUDES       := $(LOCAL_PATH)
LOCAL_CPPFLAGS         := -std=c++17 -O2 -fvisibility=hidden -fno-exceptions -fno-rtti
LOCAL_LDLIBS           := -llog -ldl
LOCAL_LDFLAGS          := -Wl,--hash-style=both
include $(BUILD_SHARED_LIBRARY)
