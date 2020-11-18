LOCAL_PATH := $(call my-dir)

########################
local_target_dir := $(TARGET_OUT)/lib/modules
include $(CLEAR_VARS)
LOCAL_MODULE := texfat.ko
LOCAL_MODULE_TAGS := optinal
LOCAL_MODULE_CLASS := MODULE
LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_SRC_FILES := texfat_$(TARGET_BUILD_VARIANT).ko
include $(BUILD_PREBUILT)

########################
local_target_dir := $(TARGET_OUT)/bin
include $(CLEAR_VARS)
LOCAL_MODULE := exfatck
LOCAL_MODULE_TAGS := optinal
LOCAL_MODULE_CLASS := MODULE
LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_SRC_FILES := exfatck
include $(BUILD_PREBUILT)

########################
local_target_dir := $(TARGET_OUT)/bin
include $(CLEAR_VARS)
LOCAL_MODULE := exfatdebug
LOCAL_MODULE_TAGS := optinal
LOCAL_MODULE_CLASS := MODULE
LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_SRC_FILES := exfatdebug
include $(BUILD_PREBUILT)

########################
local_target_dir := $(TARGET_OUT)/bin
include $(CLEAR_VARS)
LOCAL_MODULE := exfatinfo
LOCAL_MODULE_TAGS := optinal
LOCAL_MODULE_CLASS := MODULE
LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_SRC_FILES := exfatinfo
include $(BUILD_PREBUILT)

########################
local_target_dir := $(TARGET_OUT)/bin
include $(CLEAR_VARS)
LOCAL_MODULE := exfatlabel
LOCAL_MODULE_TAGS := optinal
LOCAL_MODULE_CLASS := MODULE
LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_SRC_FILES := exfatlabel
include $(BUILD_PREBUILT)

########################
local_target_dir := $(TARGET_OUT)/bin
include $(CLEAR_VARS)
LOCAL_MODULE := exfatvsn
LOCAL_MODULE_TAGS := optinal
LOCAL_MODULE_CLASS := MODULE
LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_SRC_FILES := exfatvsn
include $(BUILD_PREBUILT)

########################
local_target_dir := $(TARGET_OUT)/bin
include $(CLEAR_VARS)
LOCAL_MODULE := mkexfat
LOCAL_MODULE_TAGS := optinal
LOCAL_MODULE_CLASS := MODULE
LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_SRC_FILES := mkexfat
include $(BUILD_PREBUILT)
