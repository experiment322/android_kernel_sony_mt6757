LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := moddir.c

LOCAL_SHARED_LIBRARIES := libc libcutils liblog 

LOCAL_MODULE := moddir

LOCAL_MODULE_PATH := $(TARGET_OUT_BIN)

include $(BUILD_EXECUTABLE)
