####
# Set minimum version of CMake. Since we are using the VERSION option of the
# project command, we need at least 3.0. To honor the amazing work that
# the folks at KitWare have done for the open source community, we are
# going to specify a recent version. 
# As of UNIVERSAL v3.0 December 2019
# Ubuntu 16.04 LTS runs cmake 3.5.1
# Ubuntu 18.04 LTS runs cmake 3.10.2
# container runs cmake 3.7.1
# gcc5 container runs cmake 3.0.2
cmake_minimum_required(VERSION 3.16)

####
## Enable project() command to manage VERSION variables
if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif(POLICY CMP0048)

####
# Set project variables
if(NOT DEFINED UNIVERSAL_VERSION_MAJOR)
  set(UNIVERSAL_VERSION_MAJOR 3)
endif()
if(NOT DEFINED UNIVERSAL_VERSION_MINOR)
  set(UNIVERSAL_VERSION_MINOR 1)
endif()
if(NOT DEFINED UNIVERSAL_VERSION_PATCH)
  set(UNIVERSAL_VERSION_PATCH 1)
endif()
project(universal 
	DESCRIPTION "A header only C++ library for plug-in replacement number systems for native types"
	VERSION "${UNIVERSAL_VERSION_MAJOR}.${UNIVERSAL_VERSION_MINOR}.${UNIVERSAL_VERSION_PATCH}" 
	LANGUAGES C CXX ASM
	HOMEPAGE_URL "https://github.com/stillwater-sc/universal")

# universal is a header-only library
add_library(${PROJECT_NAME} INTERFACE)

####
# Change default build type to Release
#
# The CACHE STRING logic here and elsewhere is needed to force CMake
# to pay attention to the value of these variables.
if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No default build type specified: setting CMAKE_BUILD_TYPE=Release")
	set(CMAKE_BUILD_TYPE Release CACHE STRING
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
# MSVC generates SSE/SSE2 code by default. No support for SSE3 as of 7/15/2018
option(USE_SSE3                          "Build code with SSE3 ISA support"                    OFF)
option(USE_AVX                           "Build code with AVX ISA support"                     OFF)
option(USE_AVX2                          "Build code with AVX2 ISA support"                    OFF)
# control which projects get enabled
# Continuous Integration override to build all components
option(BUILD_CI_CHECK                    "Set to ON to build all components"                   OFF)

# utilities and educational examples
option(BUILD_CMD_LINE_TOOLS              "Set to ON to build cmd line tools"                   ON)
option(BUILD_PLAYGROUND                  "Set to ON to build experimentation playground"       ON)
option(BUILD_EDUCATION                   "Set to ON to build educational examples"             ON)
option(BUILD_APPLICATIONS                "Set to ON to build epplication examples"             ON)
option(BUILD_NUMERICAL_TESTS             "Set to ON to build numerical test programs"          OFF)
option(BUILD_FUNCTIONS                   "Set to ON to build special functions programs"       OFF)

# C API library and test programs
option(BUILD_C_API_PURE_LIB              "Set to ON to build C API native library"             OFF)
option(BUILD_C_API_SHIM_LIB              "Set to ON to build C API shim library"               OFF)
option(BUILD_C_API_LIB_PIC               "Set to ON to compile C API library with -fPIC"       OFF)

# number systems and their verification suites
option(BUILD_STORAGE_CLASSES             "Set to ON to build storage class tests"              OFF)
option(BUILD_NATIVE_TYPES                "Set to ON to build native type tests"                OFF)
option(BUILD_INTEGERS                    "Set to ON to build integer tests"                    OFF)
option(BUILD_DECIMALS                    "Set to ON to build decimal tests"                    OFF)
option(BUILD_FIXPNTS                     "Set to ON to build fixed-point tests"                OFF)
option(BUILD_BFLOATS                     "Set to ON to build bfloat tests"                     OFF)
option(BUILD_AREALS                      "Set to ON to build areal tests"                      OFF)
option(BUILD_UNUM_TYPE_1                 "Set to ON to build unum Type 1 tests"                OFF)
option(BUILD_UNUM_TYPE_2                 "Set to ON to build unum Type 2 tests"                OFF)
option(BUILD_POSITS                      "Set to ON to build unum Type 3 posit tests"          OFF)
option(BUILD_VALIDS                      "Set to ON to build unum Type 3 valid tests"          OFF)
option(BUILD_REALS                       "Set to ON to build real tests"                       OFF)
option(BUILD_LNS                         "Set to ON to build LNS tests"                        OFF)
# Basic Linear Algebra 
option(BUILD_BLAS                        "Set to ON to build the BLAS tests"                   OFF)
# conversion test suites
option(BUILD_CONVERSION_TESTS            "Set to ON to build conversion test suites"           OFF)
# performance benchmarking
option(BUILD_PERFORMANCE_TESTS           "Set to ON to build performance benchmarks"           OFF)
option(BUILD_IEEE_FLOAT_QUIRES           "Set to ON to build reproducible IEEE floats"         OFF)
# documentation
option(BUILD_DOCS                        "Set to ON to build documentation"                    OFF)

option(UNIVERSAL_USE_FOLDERS "Enable solution folders in Visual Studio, disable for Express"   ON)
if (UNIVERSAL_USE_FOLDERS) 
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

####
# Create the library target
set(project_library_target_name ${PROJECT_NAME})
set(PACKAGE_NAME ${PROJECT_NAME})

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
if(MSVC)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++20 has been enabled by default")
else()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++20 has been enabled by default: but bit_cast<> requires gcc 11")
endif()

# Compiler specific environments
if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag("-msse3" COMPILER_HAS_SSE3_FLAG)
	check_cxx_compiler_flag("-mavx"  COMPILER_HAS_AVX_FLAG)
	check_cxx_compiler_flag("-mavx2" COMPILER_HAS_AVX2_FLAG)

	# Streaming SIMD Extension (SSE) ISA
	if (USE_SSE3 AND COMPILER_HAS_SSE_FLAG)
		add_definitions(-DLIB_USE_SSE)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -msse3")
	endif(USE_SSE3 AND COMPILER_HAS_SSE_FLAG)
	# Advanced Vector Extensions (AVX) ISA
	if (USE_AVX AND COMPILER_HAS_AVX_FLAG)
		add_definitions(-DLIB_USE_AVX)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -mavx")
	endif(USE_AVX AND COMPILER_HAS_AVX_FLAG)
	# Advanced Vector Extensions 2 (AVX2) ISA
	if (USE_AVX2 AND COMPILER_HAS_AVX2_FLAG)
		add_definitions(-DLIB_USE_AVX2)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -mavx2")
	endif(USE_AVX2 AND COMPILER_HAS_AVX2_FLAG)

	# include code quality flags
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wall -Wpedantic -Wno-narrowing -Wno-deprecated")
	# specific flags for debug and release builds
	#set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -O3 -DNDEBUG")
	#set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} -g3 -pthread")

	#set(EXTRA_CXX_FLAGS "-std=c++20") # isn't this done by CMAKE_CXX_STANDARD?
elseif(MSVC)
	# Streaming SIMD Extension (SSE) ISA
	if (USE_SSE3)
		add_definitions(-DLIB_USE_SSE3)
		set(COMPILER_HAS_SSE3_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:SSE3")
	endif(USE_SSE3)
	# Advanced Vector Extensions (AVX) ISA
	if (USE_AVX)
		add_definitions(-DLIB_USE_AVX)
		set(COMPILER_HAS_AVX_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:AVX")
	endif(USE_AVX)
	# Advanced Vector Extensions 2 (AVX2) ISA
	if (USE_AVX2)
		add_definitions(-DLIB_USE_AVX2)
		set(COMPILER_HAS_AVX2_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:AVX2")
	endif(USE_AVX2)

	# include code quality flags
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /MP")
	add_definitions(-D _CRT_SECURE_NO_WARNINGS)
	add_definitions(-D _SCL_SECURE_NO_WARNINGS)

	# specific flags for debug and release builds

	# correct __cplusplus variable setting
    # /Zc:__cplusplus
    # You need to compile with the /Zc:__cplusplus switch to see the updated value of the __cplusplus macro.
	# Microsoft tried updating the macro by default and discovered that a lot of code doesn't compile correctly
	# when they changed the value of __cplusplus.
	# They'll continue to require use of the /Zc:__cplusplus switch for all minor versions of MSVC in the 19.xx family.
	#
	# The version reported by the __cplusplus macro also depends upon the standard version switch used.
	# If you're compiling in C++14 mode the macro will be set to '201402L'. If you compile in C++17 mode
	# the macro will be set to '201703L'. And the /std:c++latest switch, used to enable features from the
	# Standard currently in development, sets a value that is more than the current Standard.
	# This chart shows the values of the __cplusplus macro with different switch combinations:
	#
	# /Zc:__cplusplus switch		/std:c++ switch				__cplusplus value
	# Zc:__cplusplus			Currently defaults to C++14			201402L
	# Zc:__cplusplus			/std:c++14					201402L
	# Zc:__cplusplus			/std:c++17					201703L
	# Zc:__cplusplus			/std:c++latest					201704L
	# Zc:__cplusplus- (disabled)		Any value					199711L
	# Zc:__cplusplus not specified		Any value					199711L
	# Note that the MSVC compiler does not, and never will, support a C++11, C++03, or C++98 standards version switch.
	# Also, the value of the __cplusplus macro is not affected by the /permissive- switch.
	#set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /Zc:__cplusplus /std:c++14")
	# for the moment turn off since we are getting: cl : Command line warning D9002: ignoring unknown option '/Zc:__cplusplus`
	#set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} /std:c++17")

	# Optimization
	# Option	Purpose
	# /O1		Creates small code.
	# /O2		Creates fast code.
	# /Ob		Controls inline expansion.
	# /Od		Disables optimization.
	# /Og		Deprecated. Uses global optimizations.
	# /Oi		Generates intrinsic functions.
	# /Os		Favors small code.
	# /Ot		Favors fast code.
	# /Ox		Uses maximum optimization (/Ob2gity /Gs).
	# /Oy		Omits frame pointer. (x86 only)
	# /favor	Produces code that is optimized for a specified architecture, or for a range of architectures.
	# /fp:fast  Floating Point Model: options are fast, strict, precise
	# /GS-      Disable Security Checks

	# be forewarned that /fp:fast alters the float logic functions:
	# \fp:fast  floating point model set to fast
	# NaN ==  NaN: IEEE=true    Posit=true
	# NaN == real: IEEE=true    Posit=false
	# INF ==  INF: IEEE=true    Posit=true
	# NaN !=  NaN: IEEE=false   Posit=false
	# \fp:strict floating point model set to strict, \fp:precise yields the same results
	# NaN ==  NaN: IEEE=false   Posit=true
	# NaN == real: IEEE=false   Posit=false
	# INF ==  INF: IEEE=true    Posit=true
	# NaN !=  NaN: IEEE=true    Posit=false
	set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} /Oi /Ot /Ox /Oy /fp:fast /GS-")
	# /LTCG specified but no code generation required; remove /LTCG from the link command line to improve linker performance
	# set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
	set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} /Wall /bigobj")

	# include Spectre mitigation flags
	# https://docs.microsoft.com/en-us/cpp/build/reference/qspectre
	# Microsoft Visual C++ libraries are also available in versions with Spectre mitigation. 
	# The Spectre-mitigated libraries for Visual Studio 2017 and later can be downloaded 
	# in the Visual Studio Installer. They're found in the Individual Components tab under 
	# Compilers, build tools, and runtimes, and have "Libs for Spectre" in the name.
	set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} /Qspectre")
	# /Qspectre is very noise, so do not include it for debug builds
	# set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} /Qspectre")

endif()

####
# set the aggregated compiler options

if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# clear nonsensical defaults, like "-O3 -NDEBUG" for release
	#set(CMAKE_C_FLAGS           "")
	#set(CMAKE_C_FLAGS_RELEASE   "")
	#set(CMAKE_C_FLAGS_DEBUG     "")
	#set(CMAKE_CXX_FLAGS         "")
	#set(CMAKE_CXX_FLAGS_RELEASE "")
	#set(CMAKE_CXX_FLAGS_DEBUG   "")
endif()

# bring in the flags set above
set(EXTRA_CXX_FLAGS         "${EXTRA_CXX_FLAGS} ${EXTRA_C_FLAGS}")
set(EXTRA_CXX_FLAGS_RELEASE "${EXTRA_CXX_FLAGS} ${EXTRA_C_FLAGS_RELEASE}")
set(EXTRA_CXX_FLAGS_DEBUG   "${EXTRA_CXX_FLAGS} ${EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS} ${EXTRA_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} ${EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} ${EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${EXTRA_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_CXX_FLAGS_DEBUG}")

if(PROFILE AND (CMAKE_COMPILER_IS_GNUCXX OR MINGW OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -pg")
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} /EHsc")
endif()

####
# Set UNIVERSAL include directory that contains all the different number systems
# universal/native
# universal/bitblock
# universal/blockbin
# universal/integer
# universal/decimal
# universal/fixpnt
# universal/lns
# universal/unum
# universal/unum2
# universal/posit
# universal/valid
# universal/areal
# universal/adaptiveint
# universal/adaptivefloat
include_directories("./include")

####
# macro to read all cpp files in a directory
# and create a test target for that cpp file
macro (compile_all testing prefix folder)
    # cycle through the sources
    # For the according directories, we assume that each cpp file is a separate test
    # so, create a executable target and an associated test target
    foreach (source ${ARGN})
        get_filename_component (test ${source} NAME_WE)
        string(REPLACE " " ";" new_source ${source})
        set(test_name ${prefix}_${test})
        message(STATUS "Add test ${test_name} from source ${new_source}.")
        add_executable (${test_name} ${new_source})

        #add_custom_target(valid SOURCES ${SOURCES})
        set_target_properties(${test_name} PROPERTIES FOLDER ${folder})
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
string(REGEX REPLACE "_" "" PROJECT_NAME_NOSPACES ${PROJECT_NAME})
message(STATUS "${PROJECT_NAME} -> ${PROJECT_NAME_NOSPACES}")
set(cmake_conf_file         "${PROJECT_NAME_NOSPACES}-config.cmake")
set(cmake_conf_version_file "${PROJECT_NAME_NOSPACES}-config-version.cmake")
set(cmake_targets_file      "${PROJECT_NAME_NOSPACES}-targets.cmake")

set(targets_export_name "${PROJECT_NAME_NOSPACES}-targets")
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

message(STATUS "include_install_dir         = ${include_install_dir}")
message(STATUS "include_install_dir_full    = ${include_install_dir_full}")
message(STATUS "config_install_dir          = ${config_install_dir}")
message(STATUS "include_install_dir_postfix = ${include_install_dir_postfix}")
message(STATUS "PROJECT_SOURCE_DIR          = ${PROJECT_SOURCE_DIR}")
message(STATUS "PROJECT_VERSION             = ${PROJECT_VERSION}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR    = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR    = ${CMAKE_CURRENT_BINARY_DIR}")

# configure the library target
target_include_directories(${project_library_target_name} 
    INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    	      $<INSTALL_INTERFACE:${include_install_dir_full}>)

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

# Create *-targets.cmake file for build directory
install(TARGETS ${project_library_target_name}
        EXPORT  ${targets_export_name}
        INCLUDES DESTINATION ${include_install_dir})

export(EXPORT ${targets_export_name}
       FILE   ${CMAKE_CURRENT_BINARY_DIR}/${cmake_targets_file})

# Install *-targets.cmake file
install(EXPORT      ${targets_export_name}
        NAMESPACE   ${namespace}
        DESTINATION ${config_install_dir})

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_file}"
    "${CMAKE_CURRENT_BINARY_DIR}/${cmake_conf_version_file}"
    DESTINATION ${config_install_dir} COMPONENT cmake)

# Install headers
install(DIRECTORY   ${PROJECT_SOURCE_DIR}/include/${project_library_target_name}
        DESTINATION ${include_install_dir})

if(BUILD_CI_CHECK)
	# core example applications that use the library
	set(BUILD_EDUCATION ON)
	set(BUILD_APPLICATIONS ON)
	set(BUILD_CMD_LINE_TOOLS ON)
	set(BUILD_NUMERICAL_TESTS ON)
	set(BUILD_FUNCTIONS ON)
	set(BUILD_PLAYGROUND ON)
	# build the different test/verification suites for each number system
	set(BUILD_STORAGE_CLASSES ON)
	set(BUILD_NATIVE_TYPES ON)
	set(BUILD_REALS ON)
	set(BUILD_INTEGERS ON)
	set(BUILD_DECIMALS ON)
	set(BUILD_FIXPNTS ON)
	set(BUILD_BFLOATS ON)
	set(BUILD_AREALS ON)
	set(BUILD_UNUM_TYPE_1 ON)
	set(BUILD_UNUM_TYPE_2 ON)
	set(BUILD_POSITS ON)
	set(BUILD_VALIDS ON)
	set(BUILD_LNS ON)
	# build the BLAS test/verification suites
	set(BUILD_BLAS ON)
	set(BUILD_VMATH ON)
	# build the conversion test suites
	set(BUILD_CONVERSION_TESTS ON)
	# build the performance suites
	set(BUILD_PERFORMANCE_TESTS ON)
	# build the C API library
	set(BUILD_C_API_PURE_LIB ON)
	set(BUILD_C_API_SHIM_LIB ON)
	# build IEEE float/double quire capability
	set(BUILD_IEEE_FLOAT_QUIRES ON)
	#
endif(BUILD_CI_CHECK)

# Build the tests for the underlying storage classes
if(BUILD_STORAGE_CLASSES)
add_subdirectory("tests/bitblock")
add_subdirectory("tests/blockbinary")
add_subdirectory("tests/blocktriple")
endif(BUILD_STORAGE_CLASSES)

if(BUILD_NATIVE_TYPES)
add_subdirectory("tests/native")
endif(BUILD_NATIVE_TYPES)

# Arbitrary integers tests
if(BUILD_INTEGERS)
add_subdirectory("tests/integer")
add_subdirectory("tests/adaptiveint")
endif(BUILD_INTEGERS)

if(BUILD_DECIMALS)
add_subdirectory("tests/decimal")
endif(BUILD_DECIMALS)

# Arbitrary fixed-point tests
if(BUILD_FIXPNTS)
add_subdirectory("tests/fixpnt")
endif(BUILD_FIXPNTS)

# Arbitrary reals tests
if(BUILD_REALS)
add_subdirectory("tests/adaptivefloat")
add_subdirectory("tests/value")
endif(BUILD_REALS)

if(BUILD_LNS)
add_subdirectory("tests/lns")
endif(BUILD_LNS)

if(BUILD_UNUM_TYPE_1)
add_subdirectory("tests/unum")
endif(BUILD_UNUM_TYPE_1)

if(BUILD_UNUM_TYPE_2)
add_subdirectory("tests/unum2")
endif(BUILD_UNUM_TYPE_2)

if(BUILD_POSITS)
add_subdirectory("tests/posit")
add_subdirectory("tests/posit/specialized")
add_subdirectory("tests/adaptiveposit")
endif(BUILD_POSITS)

if(BUILD_VALIDS)
add_subdirectory("tests/valid")
endif(BUILD_VALIDS)

# reals with an uncertainty bit
if(BUILD_AREALS)
add_subdirectory("tests/areal")
endif(BUILD_AREALS)

# bfloats
if(BUILD_BFLOATS)
add_subdirectory("tests/bfloat")
endif(BUILD_BFLOATS)

# Basic Linear Algebra library
if(BUILD_BLAS)
add_subdirectory("tests/blas")
add_subdirectory("tests/vmath")
endif(BUILD_BLAS)

# conversion tests suites
if(BUILD_CONVERSION_TESTS)
add_subdirectory("tests/conversions")
endif(BUILD_CONVERSION_TESTS)

# performance benchmarks
if(BUILD_PERFORMANCE_TESTS)
add_subdirectory("performance/decimal")
add_subdirectory("performance/integer")
add_subdirectory("performance/fixpnt")
add_subdirectory("performance/areal")
add_subdirectory("performance/lns")
add_subdirectory("performance/posit")
add_subdirectory("performance/valid")
add_subdirectory("performance/unum")
endif(BUILD_PERFORMANCE_TESTS)

# command line tools and utilities 
if(BUILD_CMD_LINE_TOOLS)
add_subdirectory("tools/cmd")
add_subdirectory("tools/utils")
endif(BUILD_CMD_LINE_TOOLS)

if(BUILD_EDUCATION)
add_subdirectory("education/tables")
add_subdirectory("education/integer")
add_subdirectory("education/fixpnt")
add_subdirectory("education/areal")
add_subdirectory("education/unum")
add_subdirectory("education/posit")
add_subdirectory("education/quire")
add_subdirectory("education/valid")
add_subdirectory("education/lns")
endif(BUILD_EDUCATION)

if(BUILD_APPLICATIONS)
add_subdirectory("applications/stl")
add_subdirectory("applications/math")
add_subdirectory("applications/ode")
add_subdirectory("applications/pde")
add_subdirectory("applications/trigonometry")
add_subdirectory("applications/chebyshev")
add_subdirectory("applications/blas")
add_subdirectory("applications/dsp")
add_subdirectory("applications/roots")
add_subdirectory("applications/chaos")
add_subdirectory("applications/weather")
add_subdirectory("applications/engineering")
add_subdirectory("applications/science")
add_subdirectory("applications/cryptography")
add_subdirectory("applications/numeric")
add_subdirectory("applications/sequences")
add_subdirectory("applications/multiprecision")
endif(BUILD_APPLICATIONS)

if(BUILD_NUMERICAL_TESTS)
add_subdirectory("tests/numerical/properties")
add_subdirectory("tests/numerical/fpbench")
endif(BUILD_NUMERICAL_TESTS)

if(BUILD_FUNCTIONS)
add_subdirectory("tests/functions")
endif(BUILD_FUNCTIONS)

if(BUILD_PLAYGROUND)
add_subdirectory("playground")
endif(BUILD_PLAYGROUND)

# C API library and their tests
if(BUILD_C_API_PURE_LIB)
add_subdirectory("c_api/pure_c/posit")
add_subdirectory("c_api/pure_c/test/posit")
endif(BUILD_C_API_PURE_LIB)

if(BUILD_C_API_SHIM_LIB)
add_subdirectory("c_api/shim/posit")
add_subdirectory("c_api/shim/test/posit")
endif(BUILD_C_API_SHIM_LIB)

# IEEE float/double quire capability
if(BUILD_IEEE_FLOAT_QUIRES)
add_subdirectory("tests/float")
endif(BUILD_IEEE_FLOAT_QUIRES)

####
# Configuration summary
include(tools/cmake/summary.cmake)
universal_print_configuration_summary()
