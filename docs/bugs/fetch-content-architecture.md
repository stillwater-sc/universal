# FetchContent Design

## CMAKE_SOURCE_DIR

Here's what the bug in the design of the Universal CMakeLists.txt is about

What ${CMAKE_SOURCE_DIR} means

In CMake, ${CMAKE_SOURCE_DIR} is a built-in variable that always points to the top-level source directory — the directory where the outermost
CMakeLists.txt lives (the one CMake was originally pointed at). It does not change when CMake descends into nested directories.

There's a sibling variable, ${CMAKE_CURRENT_SOURCE_DIR}, which points to the directory of the CMakeLists.txt that is currently being processed. 
That one DOES change as CMake descends.

A third one, ${PROJECT_SOURCE_DIR}, points to the source directory of the nearest enclosing project() call. 
It also changes as you descend, but only at project() boundaries.

## How FetchContent enters the picture

When you write:

```cmake
FetchContent_MakeAvailable(universal)
```

CMake clones Universal into <build>/_deps/universal-src/ and then effectively does add_subdirectory(<build>/_deps/universal-src ...) — it runs
Universal's CMakeLists.txt as a sub-project of your build.

Inside Universal's CMakeLists, when its code refers to ${CMAKE_SOURCE_DIR}, that variable still points to the 
outermost source — your cortex repo root, not Universal's source dir. This is by design: CMAKE_SOURCE_DIR is 
a "where did the user invoke CMake" variable, not a "where am I right now" variable.

## The specific bug in Universal

Universal's CMakeLists.txt (around line 95–100) has:

```cmake
set(TEST_MATRIX_DATA_DIR "${CMAKE_SOURCE_DIR}/data/matrices")
configure_file(
    "${CMAKE_SOURCE_DIR}/config/TestMatrixDataDirConfig.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/TestMatrixDataDirConfig.hpp"
)
```

When Universal is the standalone top-level project (someone cloning Universal directly and building it), 
CMAKE_SOURCE_DIR is Universal's root, so ${CMAKE_SOURCE_DIR}/config/TestMatrixDataDirConfig.hpp.in resolves correctly.

When Universal is pulled in via FetchContent from cortex, CMAKE_SOURCE_DIR is /home/stillwater/dev/branes/clones/cortex/, 
so the configure_file call looks for /.../cortex/config/TestMatrixDataDirConfig.hpp.in — which doesn't exist in cortex. 
The configure step fails:

```bash
CMake Error: File /home/stillwater/.../cortex/config/TestMatrixDataDirConfig.hpp.in does not exist.
```

The bug is that Universal should use ${CMAKE_CURRENT_SOURCE_DIR} or ${PROJECT_SOURCE_DIR} in those calls — those would 
correctly point at Universal's own source dir regardless of how it was included.

## Why MTL5 doesn't have the same problem

MTL5's CMakeLists uses ${CMAKE_CURRENT_SOURCE_DIR} for its include directories:

```cmake
target_include_directories(mtl5 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ...
```

So MTL5 works correctly whether built standalone or via FetchContent — CMAKE_CURRENT_SOURCE_DIR always points to MTL5's 
own source dir while MTL5's CMakeLists is being processed.

## The workaround in cortex

Since Universal is header-only, cortex doesn't actually need Universal's CMakeLists machinery — only its 
include directory (<src>/include/sw/). The workaround:

1. FetchContent_Declare Universal but don't put it in FetchContent_MakeAvailable(...).
2. Manually populate via FetchContent_GetProperties + FetchContent_Populate (which downloads but skips the add_subdirectory).
3. Create a cortex-owned INTERFACE library and add Universal's include dir to it.

That bypasses Universal's broken configure_file entirely while still getting the headers.

The proper fix is upstream — file a PR on stillwater-sc/universal changing ${CMAKE_SOURCE_DIR} to ${PROJECT_SOURCE_DIR} 
in those calls. Once that lands and we bump the pinned tag, the manual-populate workaround in cmake/deps.cmake can be 
retired and Universal can go back into the normal FetchContent_MakeAvailable call.

