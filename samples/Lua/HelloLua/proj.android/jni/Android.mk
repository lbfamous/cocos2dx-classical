# macros to include all files with same extension
define all-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find $(1) -name "$(2)" -and -not -name ".*") \
 )
endef

define all-cpp-files-under
$(call all-files-under,$(1),*.cpp)
endef

# module
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := hellolua
LOCAL_SRC_FILES := $(call all-cpp-files-under,.) \
	$(call all-cpp-files-under,../../Classes)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes
LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static \
	cocosdenshion_static \
	cocos_extension_static \
	cocos_lua_static
include $(BUILD_SHARED_LIBRARY)
$(call import-module,cocos2dx)
$(call import-module,cocos2dx/platform/third_party/android/prebuilt/libcurl)
$(call import-module,scripting/lua)
$(call import-module,CocosDenshion/android)
$(call import-module,extensions)
