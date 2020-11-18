#
# Copyright (C) 2014 Sony Mobile Communications AB.
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#  LOCAL_ARM_MODE := arm

LOCAL_CFLAGS += -DMT6757

LOCAL_MODULE_TAGS := optional


LOCAL_SRC_FILES := clbrt_d.c

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
        libc

#LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_MODULE := clbrt_d


include $(BUILD_EXECUTABLE)
