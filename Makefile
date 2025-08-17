APP_NAME := GLApp
PKG_NAME := com.termux.glapp
BUILD_DIR := build
LIB_DIR := ${PREFIX}/lib
SRC := jni/main.cpp
NDK_CC := aarch64-linux-android-clang
NDK_CXX := aarch64-linux-android-clang++
NDK_SYSROOT := ${PREFIX}

APK := ${BUILD_DIR}/${APP_NAME}.apk
UNSIGNED_APK := ${BUILD_DIR}/${APP_NAME}-unsigned.apk
AAPT := aapt

all: $(APK)

$(LIB_DIR)/libmain.so: ${SRC}
	$(NDK_CXX) -fPIC -shared -o $@ $< \
		-I${NDK_SYSROOT}/include \
		-L${NDK_SYSROOT}/lib -llog -landroid -lGLESv2 -lEGL

$(UNSIGNED_APK): AndroidManifest.xml $(LIB_DIR)/libmain.so
	mkdir -p $(BUILD_DIR)/res
	$(AAPT) package -F $@ -M AndroidManifest.xml -S $(BUILD_DIR)/res
	$(AAPT) add $@ lib/arm64-v8a/libmain.so

$(APK): $(UNSIGNED_APK)
	cp $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

