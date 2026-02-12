# CMake toolchain file for IBM POWER 64-bit little-endian cross-compilation
# Uses powerpc64le-linux-gnu GCC cross-compiler and QEMU user-mode emulation
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/ppc64le-linux-gnu.cmake ..
#
# Prerequisites (Ubuntu/Debian):
#   sudo apt-get install g++-powerpc64le-linux-gnu qemu-user-static

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ppc64le)

set(CMAKE_C_COMPILER powerpc64le-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER powerpc64le-linux-gnu-g++)

# QEMU user-mode emulation for running cross-compiled test binaries
set(CMAKE_CROSSCOMPILING_EMULATOR "qemu-ppc64le-static;-L;/usr/powerpc64le-linux-gnu")

# Search paths for cross-compiled libraries
set(CMAKE_FIND_ROOT_PATH /usr/powerpc64le-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
