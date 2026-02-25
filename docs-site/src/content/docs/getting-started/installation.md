---
title: Installation
description: Build and install the Universal Numbers Library from source
sidebar:
  order: 1
---

Universal is a header-only C++ template library with no external dependencies.

## Requirements

- **C++20** compiler (GCC 11+, Clang 14+, MSVC 2022+)
- **CMake** 3.22 or later

## Install CMake

**Ubuntu/Debian:**

```bash
sudo snap install cmake --classic
```

**Portable Linux:**

```bash
wget https://github.com/Kitware/CMake/releases/download/v4.2.1/cmake-4.2.1-Linux-x86_64.sh
sudo sh cmake-4.2.1-Linux-x86_64.sh --prefix=/usr/local --exclude-subdir
```

**macOS/Windows:** Download the installer from [cmake.org/download](https://cmake.org/download).

## Build from Source

```bash
git clone https://github.com/stillwater-sc/universal
cd universal
mkdir build && cd build
cmake ..
make -j4
make test
```

## CMake Build Options

Use `cmake-gui` or `ccmake` to explore interactively, or set options on the command line:

```bash
# Build everything
cmake -DUNIVERSAL_BUILD_ALL=ON ..

# Build a specific number system
cmake -DUNIVERSAL_BUILD_NUMBER_POSITS=ON ..

# Enable AVX2 optimizations
cmake -DUSE_AVX2=ON ..

# Build educational examples
cmake -DUNIVERSAL_BUILD_EDUCATION=ON ..
```

## Using Universal in Your Project

### Option 1: CMake find_package

After installing Universal (`make install`):

```cmake
project("my-numerical-experiment")
find_package(UNIVERSAL CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} universal::universal)
```

### Option 2: Header-only include

Since Universal is header-only, you can simply add the include path:

```bash
g++ -std=c++20 -I/path/to/universal/include/sw my_program.cpp -o my_program
```

## Install / Uninstall

```bash
# Install (default: /usr/local)
cmake -DCMAKE_INSTALL_PREFIX=/your/path ..
make install

# Uninstall
make uninstall
```
