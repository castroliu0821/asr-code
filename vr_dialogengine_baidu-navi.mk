LOCAL_PATH := $(my-dir)

#clean local vars
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libvr_dialogengine_baidu-navi

include $(BUILD_NAVI_TARGET_SHARED_LIBRARY)


