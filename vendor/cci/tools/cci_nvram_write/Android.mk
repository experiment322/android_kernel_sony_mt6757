#
# Copyright (C) 2014 Sony Mobile Communications AB.
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#  LOCAL_ARM_MODE := arm

LOCAL_CFLAGS += -DMT6757

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
	${shell find $(MTK_PATH_SOURCE)/custom/$(MYMTK_PROJECT)/cgen -type d} \
	${shell find $(MTK_PATH_SOURCE)/custom/common/cgen -type d}
	

LOCAL_SRC_FILES := cci_nvram_write.c

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libnvram

#LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_MODULE := nvram_write

LOCAL_MODULE_PATH := $(TARGET_OUT_BIN)

include $(BUILD_EXECUTABLE)
