# CMake toolchain file for RISC-V 64-bit cross-compilation
# Uses riscv64-linux-gnu GCC cross-compiler and QEMU user-mode emulation
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/riscv64-linux-gnu.cmake ..
#
# Prerequisites (Ubuntu/Debian):
#   sudo apt-get install g++-riscv64-linux-gnu qemu-user-static

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(CMAKE_C_COMPILER riscv64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER riscv64-linux-gnu-g++)

# QEMU user-mode emulation for running cross-compiled test binaries
set(CMAKE_CROSSCOMPILING_EMULATOR "qemu-riscv64-static;-L;/usr/riscv64-linux-gnu")

# Search paths for cross-compiled libraries
set(CMAKE_FIND_ROOT_PATH /usr/riscv64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
