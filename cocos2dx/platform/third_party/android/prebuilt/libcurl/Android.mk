LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cocos_curl_static
LOCAL_MODULE_FILENAME := curl
LOCAL_SRC_FILES := libraries/$(TARGET_ARCH_ABI)/libcurl.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)
