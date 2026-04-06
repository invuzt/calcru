LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cakru_core
LOCAL_SRC_FILES := libcakru_core.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := hello
LOCAL_SHARED_LIBRARIES := cakru_core
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
