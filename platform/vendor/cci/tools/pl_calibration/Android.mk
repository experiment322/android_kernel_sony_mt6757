LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DMT6757

LOCAL_ARM_MODE := arm
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES:= \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
	$(MTK_PATH_SOURCE)/custom/hinoki/cgen/inc \
	$(MTK_PATH_SOURCE)/custom/common/cgen/inc \
	$(MTK_PATH_SOURCE)/custom/hinoki/cgen/cfgdefault \
	$(MTK_PATH_SOURCE)/custom/hinoki/cgen/cfgfileinc

LOCAL_SRC_FILES := \
	pl_calibration.c \
	pl_misc_ta.c \
	pl_nvram.c

LOCAL_STATIC_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES := \
	libnvram \
	libmiscta\
	liblog
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

LOCAL_MODULE := pl_calibration

LOCAL_MODULE_PATH := $(TARGET_OUT_BIN)

include $(BUILD_EXECUTABLE)
