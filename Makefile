APP_NAME := GLApp
PKG_NAME := com.termux.glapp
NDK := $(HOME)/ndk
AAPT := aapt

ARCH := arm64-v8a
BUILD_DIR := build
LIB_DIR := $(BUILD_DIR)/lib/$(ARCH)
APK := $(BUILD_DIR)/$(APP_NAME).apk
UNSIGNED_APK := $(BUILD_DIR)/$(APP_NAME)-unsigned.apk

all: $(APK)

$(LIB_DIR)/libmain.so: jni/main.cpp Android.mk Application.mk
	$(NDK)/ndk-build NDK_PROJECT_PATH=. \
		APP_BUILD_SCRIPT=./Android.mk \
		NDK_APPLICATION_MK=./Application.mk \
		APP_PLATFORM=android-21

$(UNSIGNED_APK): AndroidManifest.xml $(LIB_DIR)/libmain.so
	mkdir -p $(BUILD_DIR)/res
	$(AAPT) package -F $@ -M AndroidManifest.xml -S $(BUILD_DIR)/res -I $(NDK)/platforms/android-21/arch-arm64/usr/include
	$(AAPT) add $@ lib/$(ARCH)/libmain.so

$(APK): $(UNSIGNED_APK)
	cp $< $@

clean:
	rm -rf $(BUILD_DIR) libs obj

.PHONY: all clean
