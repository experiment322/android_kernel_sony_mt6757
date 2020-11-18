LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES:= \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
	$(MTK_PATH_SOURCE)/custom/hinoki/cgen/inc \
	$(MTK_PATH_SOURCE)/custom/common/cgen/inc \
	$(MTK_PATH_SOURCE)/custom/hinoki/cgen/cfgdefault \
	$(MTK_PATH_SOURCE)/custom/hinoki/cgen/cfgfileinc

LOCAL_SRC_FILES := g_read_miscta.c
LOCAL_SHARED_LIBRARIES := libcutils libnvram libmiscta liblog

LOCAL_MODULE := g_read_miscta
LOCAL_MODULE_PATH := $(TARGET_OUT_BIN)

include $(BUILD_EXECUTABLE)
