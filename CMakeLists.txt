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

# Set posit include directory
include_directories("./posit")

# Possibly not under Windows
add_compile_options ( -std=c++11 )

macro (compile_all testing prefix)
    # cycle through the sources
    # For the according directories, we assume that each cpp file is a separate test
    # so, create a executable target and an associated test target
    foreach (source ${ARGN})
        get_filename_component (test ${source} NAME_WE)
        string(REPLACE " " ";" new_source ${source})
        set(test_name ${prefix}_${test})
        # message(STATUS "Add test ${test_name} from source ${new_source}.")
        add_executable (${test_name} ${new_source})
        if (${testing} STREQUAL "true")
            if (UNIVERSAL_CMAKE_TRACE)
                message(STATUS "testing: ${test_name} ${RUNTIME_OUTPUT_DIRECTORY}/${test_name}")
            endif()
            add_test(${test_name} ${RUNTIME_OUTPUT_DIRECTORY}/${test_name})
        endif()
    endforeach (source)
endmacro (compile_all)

add_subdirectory("tests/bitblock")
add_subdirectory("tests/float")
add_subdirectory("tests/unum")
add_subdirectory("tests/unum2")
add_subdirectory("tests/posit")
add_subdirectory("tests/valid")

add_subdirectory("perf")
add_subdirectory("tools/cmd")
add_subdirectory("education/posit")
add_subdirectory("examples/blas")
add_subdirectory("examples/dsp")
add_subdirectory("examples/playground")
