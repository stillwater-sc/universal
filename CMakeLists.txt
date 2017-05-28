project (universal)

cmake_minimum_required(VERSION 2.8)

option(UNIVERSAL_CMAKE_TRACE "Tracing CMake results, i.e. printing variable settings." OFF)
option(UNIVERSAL_ENABLE_TESTS "Enable the build and run of tests." ON)
option(UNIVERSAL_VERBOSE_TESTS "Always print test output, otherwise only errors. Only relevant when tests enabled." OFF)

macro(trace_variable variable)
    if (UNIVERSAL_CMAKE_TRACE)
        message(STATUS "${variable} = ${${variable}}")
    endif()
endmacro()
        

# TODO: Check whether this is needed for old CMake versions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake") 
trace_variable(CMAKE_MODULE_PATH)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake") 
trace_variable(CMAKE_PREFIX_PATH)

  
# Must be located in root dir, doesn't work in tests
if (UNIVERSAL_ENABLE_TESTS)
    enable_testing()
    # include(Dart)
endif()

# Possibly not under Windows
add_compile_options ( -std=c++11 )


add_subdirectory(unum)


