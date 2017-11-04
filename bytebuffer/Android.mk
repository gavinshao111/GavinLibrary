LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bytebuffer
LOCAL_CPPFLAGS := -std=c++14 -fexceptions
LOCAL_LDFLAGS := -Wl,--build-id

LOCAL_SRC_FILES := src/ByteBuffer.cpp \

LOCAL_C_INCLUDES +=  \
src

NDK_APP_DST_DIR := ../lib/$(TARGET_ARCH_ABI)
include $(BUILD_SHARED_LIBRARY)

