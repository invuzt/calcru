ANDROID_SDK ?= /usr/local/lib/android/sdk
NDK_PATH ?= $(ANDROID_SDK)/ndk/26.1.10909125
BUILD_TOOLS_PATH := $(shell ls -d $(ANDROID_SDK)/build-tools/* | tail -1)
PLATFORM_JAR := $(shell ls -d $(ANDROID_SDK)/platforms/android-* | tail -1)/android.jar

AAPT := $(BUILD_TOOLS_PATH)/aapt
D8 := $(BUILD_TOOLS_PATH)/d8
ZIPALIGN := $(BUILD_TOOLS_PATH)/zipalign
APKSIGNER := $(BUILD_TOOLS_PATH)/apksigner

build:
	@echo "Building native code..."
	$(NDK_PATH)/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=jni/Android.mk NDK_APPLICATION_MK=jni/Application.mk
	@echo "Compiling Java..."
	javac Dummy.java
	$(D8) --output . Dummy.class
	@echo "Packaging APK..."
	$(AAPT) package -f -M AndroidManifest.xml -S res -I $(PLATFORM_JAR) -F cakru.unaligned.apk
	mkdir -p lib && cp -r libs/* lib/
	zip -u cakru.unaligned.apk lib/*/libhello.so classes.dex
	$(ZIPALIGN) -f 4 cakru.unaligned.apk cakru.apk
	$(APKSIGNER) sign --ks debug.keystore --ks-key-alias androiddebugkey --ks-pass pass:android --key-pass pass:android --out cakru-final.apk cakru.apk
	@echo "Done: cakru-final.apk"
	rm -rf libs obj lib classes.dex Dummy.class cakru.unaligned.apk cakru.apk

clean:
	rm -rf libs obj lib classes.dex Dummy.class *.apk
