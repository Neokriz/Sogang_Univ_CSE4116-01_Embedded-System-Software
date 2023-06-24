LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := simple-sim
LOCAL_SRC_FILES := FPGA_device.c
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)