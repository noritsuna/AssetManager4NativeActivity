LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := assetmanager
LOCAL_SRC_FILES := assetmanager.c unzip.c ioapi.c
LOCAL_LDLIBS    += -lz

include $(BUILD_STATIC_LIBRARY)
