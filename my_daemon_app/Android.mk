LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := app_daemon
LOCAL_SRC_FILES := app_daemon.c
include $(BUILD_EXECUTABLE)
