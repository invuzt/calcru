LOCAL_PATH := $(call my-dir)

# 1. Deklarasikan Pustaka Rust sebagai Prebuilt
include $(CLEAR_VARS)
LOCAL_MODULE := rust_logic
# Sesuaikan path ini dengan tempat Cargo menyimpan file .a (biasanya di target/...)
LOCAL_SRC_FILES := ../rust_logic/target/aarch64-linux-android/release/librust_logic.a
include $(PREBUILT_STATIC_LIBRARY)

# 2. Bangun Shared Library Utama
include $(CLEAR_VARS)
LOCAL_MODULE := hello
LOCAL_SRC_FILES := hello.c sound.c
LOCAL_LDLIBS := -llog -landroid -lGLESv2 -lEGL -lOpenSLES
LOCAL_STATIC_LIBRARIES := android_native_app_glue rust_logic
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
