LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := main
LOCAL_SRC_FILES := jni/main.cpp
LOCAL_LDLIBS    := -landroid -llog -lEGL -lGLESv2
include $(BUILD_SHARED_LIBRARY)

