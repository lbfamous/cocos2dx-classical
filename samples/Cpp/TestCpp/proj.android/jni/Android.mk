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
LOCAL_MODULE := testcpp_shared
LOCAL_MODULE_FILENAME := libtestcpp
LOCAL_SRC_FILES := $(call all-cpp-files-under,testcpp)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes
LOCAL_WHOLE_STATIC_LIBRARIES := cocos_testcpp_common \
	cocos2dx_static \
	cocosdenshion_static \
	box2d_static \
	chipmunk_static \
	cocos_extension_static \
	cocos_lua_static
include $(BUILD_SHARED_LIBRARY)
$(call import-module,samples/Cpp/TestCpp)
$(call import-module,cocos2dx)
$(call import-module,cocos2dx/platform/third_party/android/prebuilt/libcurl)
$(call import-module,CocosDenshion/android)
$(call import-module,extensions)
$(call import-module,external/Box2D)
$(call import-module,external/chipmunk)
$(call import-module,scripting/lua)