####
# Set minimum version of CMake. Since we are using the VERSION option of the
# project command, we need at least 3.0. To honor the amazing work that
# the folks at KitWare have done for the open source community, we are
# going to specify a recent version.
cmake_minimum_required(VERSION 3.7)

####
# Set project variables
project (universal VERSION 1.0.0 LANGUAGES CXX)

####
## Enable project() command to manage VERSION variables
if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif(POLICY CMP0048)

####
# Change default build type to Release
#
# The CACHE STRING logic here and elsewhere is needed to force CMake
# to pay attention to the value of these variables.
if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No default build type specified: setting CMAKE_BUILD_TYPE=Release")
	set(CMAKE_BUILD_TYPE Release CACHE string
		"Choose the build type: options are: Debug Release RelWithDebInfo MinSizeRel"
		FORCE)
else(NOT CMAKE_BUILD_TYPE)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		message("====================================================================================")
		message(STATUS "Build type is set to Debug: Performance will be negatively impacted")
		message(STATUS "Add -DCMAKE_BUILD_TYPE=Release to the CMake command line to get an optimized build")
		message("====================================================================================")
	endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif(NOT CMAKE_BUILD_TYPE)

####
# Set build options
option(USE_SSE                           "Build code with SSE ISA support"                  ON)
option(USE_AVX                           "Build code with AVX ISA support"                  ON)
option(USE_AVX2                          "Build code with AVX2 ISA support"                 ON)
# control which projects get enabled
option(BUILD_UNUM_TYPE_1                 "Set to ON to build UNUM Type 1 arithmetic"        OFF)
option(BUILD_UNUM_TYPE_2                 "Set to ON to build UNUM Type 2 arithmetic"        OFF)
option(BUILD_UNUM_TYPE_3_POSIT           "Set to ON to build UNUM Type 3(posit) arithmetic" ON)
option(BUILD_UNUM_TYPE_3_VALID           "Set to ON to build UNUM Type 3(valid) arithmetic" OFF)
option(BUILD_EDUCATION_EXAMPLES          "Set to ON to build educational examples"          ON)
option(BUILD_APPLICATION_EXAMPLES        "Set to ON to build epplication examples"          ON)
option(BUILD_VERIFICATION_SUITE          "Set to ON to build verification tests"            OFF)
option(BUILD_CMD_LINE_TOOLS              "Set to ON to build cmd line tools"                OFF)
option(BUILD_PLAYGROUND                  "Set to ON to build experimentation playground"    ON)
option(BUILD_IEEE_FLOAT_QUIRES           "Set to ON to build reproducible IEEE floats"      OFF)
option(BUILD_PERFORMANCE_BENCHMARKS      "Set to ON to build performance benchmarking"      OFF)

####
# Create the library target
set(project_library_target_name ${PROJECT_NAME})
set(PACKAGE_NAME Universal)

####
# Set environmental options for tracing, testing, and verbosity
option(UNIVERSAL_CMAKE_TRACE "Tracing CMake results, i.e. printing variable settings." OFF)
option(UNIVERSAL_ENABLE_TESTS "Enable the build and run of tests." ON)
option(UNIVERSAL_VERBOSE_TESTS "Always print test output, otherwise only errors. Only relevant when tests enabled." OFF)

macro(trace_variable variable)
    if (UNIVERSAL_CMAKE_TRACE)
        message(STATUS "${variable} = ${${variable}}")
    endif()
endmacro()
        

# Use cmake scripts and modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake") 
trace_variable(CMAKE_MODULE_PATH)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake") 
trace_variable(CMAKE_PREFIX_PATH)

# Must be located in root dir, doesn't work in tests
if (UNIVERSAL_ENABLE_TESTS)
    enable_testing()
    # include(Dart)
endif()

####
# Configure the compiler options
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++14 support has been enabled by default")

# Compiler specific environments
if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	include(CheckCXXComilerFlag)
	check_cxx_compiler_flag("-msse3" COMPILER_HAS_SSE_FLAG)
	check_cxx_compiler_flag("-mavx"  COMPILER_HAS_AVX_FLAG)
	check_cxx_compiler_flag("-mavx2" COMPILER_HAS_AVX2_FLAG)
	
	# Streaming SIMD Extension (SSE) ISA
	if (USE_SSE AND COMPILER_HAS_SSE_FLAG)
		add_definitions(-DUNUM_USE_SSE)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -msse3")
	endif(USE_SSE AND COMPILER_HAS_SSE_FLAG)
	# Advanced Vector Extensions (AVX) ISA
	if (USE_AVX AND COMPILER_HAS_AVX_FLAG)
		add_definitions(-DUNUM_USE_AVX)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -mavx")
	endif(USE_AVX AND COMPILER_HAS_AVX_FLAG)
	# Advanced Vector Extensions 2 (AVX2) ISA
	if (USE_AVX2 AND COMPILER_HAS_AVX2_FLAG)
		add_definitions(-DUNUM_USE_AVX2)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -mavx2")
	endif(USE_AVX2 AND COMPILER_HAS_AVX2_FLAG)

	# include code quality flags
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wall -Wpedantic -Wno-narrowing -Wno-deprecated")
	# specific flags for debug and release builds
	set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -O3")
	set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} -g3 -pthread")
elsif(MSVC)
	# Streaming SIMD Extension (SSE) ISA
	if (USE_SSE)
		add_definitions(-DUNUM_USE_SSE)
		set(COMPILER_HAS_SSE_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:SSE2")
	endif(USE_SSE)
	# Advanced Vector Extensions (AVX) ISA
	if (USE_AVX)
		add_definitions(-DUNUM_USE_AVX)
		set(COMPILER_HAS_AVX_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:AVX")
	endif(USE_AVX)
	# Advanced Vector Extensions 2 (AVX2) ISA
	if (USE_AVX2)
		add_definitions(-DUNUM_USE_AVX2)
		set(COMPILER_HAS_AVX2_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:AVX2")
	endif(USE_AVX2)

	# include code quality flags
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /MP")
	add_definitions(-D _CRT_SECURE_NO_WARNINGS)
	add_definitions(-D _SCL_SECURE_NO_WARNINGS)
	# specific flags for debug and release builds
	set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} /Ox /Oi /Ot /Oy /OGL /fp:fast /GS-")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
	set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} /W4 /bigobj")
endif()

####
# set the aggregated compiler options
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${EXTRA_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_C_FLAGS_DEBUG}")

if(PROFILE AND (CMAKE_COMPILER_IS_GNUCXX OR MINGW OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -pg")
endif()

####
# Set posit include directory
include_directories("./posit")

# Possibly not under Windows
add_compile_options ( -std=c++11 )

####
# macro to read all cpp files in a directory 
# and create a test target for that cpp file
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

####
# Setup the cmake config files
string(REGEX REPLACE "_" "" PROJECT_NAME_JOINED ${PROJECT_NAME})
message(STATUS "${PROJECT_NAME} -> ${PROJECT_NAME_JOINED}")
set(cmake_conf_file         "${PROJECT_NAME_JOINED}-config.cmake")
set(cmake_conf_version_file "${PROJECT_NAME_JOINED}-config-version.cmake")
set(cmake_targets_file      "${PROJECT_NAME_JOINED}-targets.cmake")

set(targets_export_name "${PROJECT_NAME_JOINED}-targets")
set(namespace "${PACKAGE_NAME}::")

# Set up install directories. INCLUDE_INSTALL_DIR and
# CMAKECONFIG_INSTALL_DIR must not be absolute paths.
if(WIN32)
    set(include_install_dir Include)
    set(include_install_dir_full Include)
    set(config_install_dir CMake)
elseif(UNIX)
    set(include_install_dir include)
    set(include_install_dir_postfix "${project_library_target_name}")
    set(include_install_dir_full    "${include_install_dir}/${include_install_dir_postfix}")

    set(config_install_dir share/${PACKAGE_NAME})
else()
    message(FATAL_ERROR "Not supported system type. Options: UNIX or WIN32.")
endif()

# configure the library target
#target_include_directories(
#    ${project_library_target_name} INTERFACE
#    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
#    $<INSTALL_INTERFACE:${include_install_dir_full}>)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/Templates/cmake-uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake-uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake-uninstall.cmake)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/Templates/${cmake_conf_file}.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_file}"
    PATH_VARS include_install_dir_full
    INSTALL_DESTINATION ${config_install_dir})

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_version_file}
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${EXTRA_C_FLAGS}")
# Create *-targets.cmake file for build directory
#install(TARGETS ${project_library_target_name}
#        EXPORT  ${targets_export_name}
#        INCLUDES DESTINATION ${include_install_dir})

#export(EXPORT ${targets_export_name}
#       FILE   ${CMAKE_CURRENT_BINARY_DIR}/${cmake_targets_file})

# Install *-targets.cmake file
#install(EXPORT      ${targets_export_name}
#        NAMESPACE   ${namespace}
#        DESTINATION ${config_install_dir})

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_file}"
    "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_version_file}"
    DESTINATION ${config_install_dir} COMPONENT cmake)

# Install headers
install(DIRECTORY   ${PROJECT_SOURCE_DIR}/${project_library_target_name}
        DESTINATION ${include_install_dir})

if(BUILD_VERIFICATION_SUITE)
add_subdirectory("tests/bitblock")
add_subdirectory("tests/float")
add_subdirectory("tests/unum")
add_subdirectory("tests/unum2")
add_subdirectory("tests/posit")
add_subdirectory("tests/valid")
endif(BUILD_VERIFICATION_SUITE)

if(BUILD_CMD_LINE_TOOLS)
add_subdirectory("tools/cmd")
add_subdirectory("tools/utils")
endif(BUILD_CMD_LINE_TOOLS)

if(BUILD_EDUCATION_EXAMPLES)
add_subdirectory("education/posit")
endif(BUILD_EDUCATION_EXAMPLES)

if(BUILD_APPLICATION_EXAMPLES)
add_subdirectory("examples/blas")
add_subdirectory("examples/dsp")
endif(BUILD_APPLICATION_EXAMPLES)

if(BUILD_PLAYGROUND)
add_subdirectory("examples/playground")
endif(BUILD_PLAYGROUND)

if(BUILD_PERFORMANCE_BENCHMARKS)
add_subdirectory("perf")
endif(BUILD_PERFORMANCE_BENCHMARKS)

####
# Configuration summary
include(tools/cmake/summary.cmake)
universal_print_configuration_summary()
