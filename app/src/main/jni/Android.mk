LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_LDLIBS    := -llog
LOCAL_MODULE := networkdetect
LOCAL_SRC_FILES := networkdetect.c
include $(BUILD_SHARED_LIBRARY)
LOCAL_CFLAGS=-g
LOCAL_CERTIFICATE := platform