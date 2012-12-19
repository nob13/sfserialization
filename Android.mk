LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := sfserialization
LOCAL_SRC_FILES := \
sfserialization/Deserialization.cpp \
sfserialization/Serialization.cpp \
sfserialization/JSONParser.cpp

include $(BUILD_SHARED_LIBRARY)
