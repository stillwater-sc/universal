# CMake toolchain file for Android aarch64 cross-compilation
# Uses the Android NDK Clang toolchain (long double is 128-bit IEEE binary128)
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-linux-android.cmake ..
#
# Prerequisites:
#   Android NDK installed; set ANDROID_NDK_HOME or ANDROID_NDK environment variable
#   On GitHub Actions ubuntu-latest, $ANDROID_NDK_HOME is pre-installed.
#
# Note: Android cross-compiled binaries require an Android runtime and cannot
# be executed with QEMU, so CI is compile-only (no ctest).

set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Locate the NDK: prefer ANDROID_NDK_HOME, fall back to ANDROID_NDK
if(DEFINED ENV{ANDROID_NDK_HOME})
    set(CMAKE_ANDROID_NDK "$ENV{ANDROID_NDK_HOME}")
elseif(DEFINED ENV{ANDROID_NDK})
    set(CMAKE_ANDROID_NDK "$ENV{ANDROID_NDK}")
endif()

set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_STL_TYPE c++_static)

# Minimum API level (Android 9 / Pie)
set(CMAKE_SYSTEM_VERSION 28)
