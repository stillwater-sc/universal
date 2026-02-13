# CMake toolchain file for Windows x86-64 cross-compilation via MinGW-w64
# Uses x86_64-w64-mingw32 GCC cross-compiler and Wine for running test binaries
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/x86_64-w64-mingw32.cmake ..
#
# Prerequisites (Ubuntu/Debian):
#   sudo apt-get install g++-mingw-w64-x86-64 wine wine64

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# Wine for running cross-compiled Windows test binaries
set(CMAKE_CROSSCOMPILING_EMULATOR wine)

# Statically link GCC and C++ runtime libraries so Wine doesn't need
# libgcc_s_seh-1.dll and libstdc++-6.dll at runtime
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static")

# Work around two MinGW GCC bugs:
# 1. IPA ICF incorrectly folds different lns<nbits> template instantiation
#    fragments after function splitting (partial-inlining + ipa-icf interaction)
# 2. MinGW's software std::fma() has precision errors that break error-free
#    transformations in floatcascade; -mfma uses hardware FMA3 instead
set(CMAKE_CXX_FLAGS_INIT "-fno-ipa-icf -mfma")

# Search paths for cross-compiled libraries
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
