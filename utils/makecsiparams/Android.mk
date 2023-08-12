LOCAL_PATH := $(call my-dir)

include $(CLEAR_VAR)

LOCAL_SRC_FILES := \
	makecsiparams.c \
	bcmwifi_channels.c
LOCAL_MODULE := makecsiparams
LOCAL_MODULE_FILENAME := makecsiparams

#NDK_TOOL_CHAIN_VERSION := 5.4
LOCAL_CFLAGS += -std=c99
LOCAL_CFLAGS += -fPIE
LOCAL_LDFLAGS += -fPIE -pie
LOCAL_C_INCLUDES += ./include

AL_MODEL_PATH := $(TARGET_OUT_OPTIONAL_EXCUABLES)

include $(BUILD_EXECUTABLE)
