########################################################################################
# CMakeList.txt
#
# Authors: 
#          Matthias Moller, William Zorn
#          Theodore Omtzigt, Peter Gottschling
#          Universal Numerical Analysis Community
########################################################################################
include(tools/cmake/banners.cmake)
print_header()

####
# Set minimum version of CMake. Since we are using the VERSION option of the
# project command, we need at least 3.0. To honor the amazing work that
# the folks at KitWare have done for the open-source community, we are
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
  set(UNIVERSAL_VERSION_MINOR 43)
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
# Continuous Integration override to build all
option(BUILD_ALL                         "Set to ON to build all components"                   OFF)
# or subsets of the components
option(BUILD_DEMONSTRATION               "Set to ON to build all demonstration components"      ON)
option(BUILD_NUMERICAL_CHECKS            "Set to ON to build all the numerical components"     OFF)
option(BUILD_BENCHMARKS                  "Set to ON to build all the benchmark components"     OFF)
option(BUILD_REGRESSION_SUITES           "Set to ON to build all the regression suites"        OFF)
option(BUILD_MIXEDPRECISION_SDK          "Set to ON to build the mixed-precision SDK"          OFF)

# or, build individual components
# utilities and educational examples
option(BUILD_CMD_LINE_TOOLS              "Set to ON to build cmd line tools"                   OFF)
option(BUILD_PLAYGROUND                  "Set to ON to build experimentation playground"       OFF)
option(BUILD_EDUCATION                   "Set to ON to build educational examples"             OFF)
option(BUILD_APPLICATIONS                "Set to ON to build application examples"             OFF)

# numerical and special math functions
option(BUILD_NUMERICAL_CHALLENGES        "Set to ON to build numerical challenge programs"     OFF)
option(BUILD_NUMERICAL_UTILS             "Set to ON to build numerical utilities"              OFF)
option(BUILD_FPBENCH                     "Set to ON to build fpbench benchmarks"               OFF)
option(BUILD_FUNCTIONS                   "Set to ON to build special functions programs"       OFF)
# reproducible IEEE-754
option(BUILD_IEEE_FLOAT_QUIRES           "Set to ON to build reproducible IEEE floats"         OFF)


# C API library and test programs
option(BUILD_C_API_PURE_LIB              "Set to ON to build C API native library"             OFF)
option(BUILD_C_API_SHIM_LIB              "Set to ON to build C API shim library"               OFF)
option(BUILD_C_API_LIB_PIC               "Set to ON to compile C API library with -fPIC"       OFF)

# number systems and their verification suites
option(BUILD_ARITHMETIC_CLASSES          "Set to ON to build arithmetic class tests"           OFF)
option(BUILD_NATIVE_TYPES                "Set to ON to build native type tests"                OFF)
option(BUILD_INTEGERS                    "Set to ON to build integer tests"                    OFF)
option(BUILD_DECIMALS                    "Set to ON to build decimal tests"                    OFF)
option(BUILD_FIXPNTS                     "Set to ON to build fixed-point tests"                OFF)
option(BUILD_CFLOATS                     "Set to ON to build cfloat tests"                     OFF)
option(BUILD_AREALS                      "Set to ON to build areal tests"                      OFF)
option(BUILD_UNUM_TYPE_1                 "Set to ON to build unum Type 1 tests"                OFF)
option(BUILD_UNUM_TYPE_2                 "Set to ON to build unum Type 2 tests"                OFF)
option(BUILD_POSITS                      "Set to ON to build unum Type 3 posit tests"          OFF)
option(BUILD_VALIDS                      "Set to ON to build unum Type 3 valid tests"          OFF)
option(BUILD_REALS                       "Set to ON to build real tests"                       OFF)
option(BUILD_LNS                         "Set to ON to build LNS tests"                        OFF)
# conversion test suites
option(BUILD_CONVERSIONS                 "Set to ON to build conversion test suites"           OFF)

# Basic Linear Algebra tests
option(BUILD_BLAS                        "Set to ON to build the BLAS tests"                   OFF)
option(BUILD_VMATH                       "Set to ON to build the BLAS vector math lib"         OFF)

# benchmarking
option(BUILD_BENCHMARK_PERFORMANCE       "Set to ON to build performance benchmarks"           OFF)
option(BUILD_BENCHMARK_ENERGY            "Set to ON to build mixed-prec energy benchmarks"     OFF)
option(BUILD_BENCHMARK_ACCURACY          "Set to ON to build accuracy benchmarks"              OFF)

# mixed-precision algorithm design and optimization
option(BUILD_MIXEDPRECISION_ROOTS        "Set to ON to build mixed-precision root finders"     OFF)
option(BUILD_MIXEDPRECISION_APPROXIMATE  "Set to ON to build mixed-precision approximation"    OFF)
option(BUILD_MIXEDPRECISION_INTEGRATE    "Set to ON to build mixed-precision integration"      OFF)
option(BUILD_MIXEDPRECISION_INTERPOLATE  "Set to ON to build mixed-precision interpolation"    OFF)
option(BUILD_MIXEDPRECISION_OPTIMIZE     "Set to ON to build mixed-precision optimization"     OFF)
option(BUILD_MIXEDPRECISION_CONJUGATE    "Set to ON to build mixed-precision CG"               OFF)

# validation
option(BUILD_VALIDATION_MATH             "Set to ON to build math validation testbenches"      OFF)
option(BUILD_VALIDATION_HW               "Set to ON to build hw validation testbenches"        OFF)

# regression test intensity
# sanity is level 1 (=min) regression testing
option(BUILD_REGRESSION_SANITY           "Set to ON to enable regression level sanity"         ON)
# stress is level 4 (= max) regression testing
option(BUILD_REGRESSION_STRESS           "Set to ON to enable regression level stress"         OFF)
# four levels of intensity
option(BUILD_REGRESSION_LEVEL_1          "Set to ON to enable regression level 1"              OFF)
option(BUILD_REGRESSION_LEVEL_2          "Set to ON to enable regression level 2"              OFF)
option(BUILD_REGRESSION_LEVEL_3          "Set to ON to enable regression level 3"              OFF)
option(BUILD_REGRESSION_LEVEL_4          "Set to ON to enable regression level 4"              OFF)

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

# enable the regression testing level of the build
if(BUILD_REGRESSION_SANITY OR BUILD_REGRESSION_LEVEL_1)
	add_definitions(-D REGRESSION_LEVEL_OVERRIDE)
	add_definitions(-D REGRESSION_LEVEL_1)
endif(BUILD_REGRESSION_SANITY OR BUILD_REGRESSION_LEVEL_1)
if(BUILD_REGRESSION_LEVEL_2)
	add_definitions(-D REGRESSION_LEVEL_OVERRIDE)
	add_definitions(-D REGRESSION_LEVEL_2)
endif(BUILD_REGRESSION_LEVEL_2)
if(BUILD_REGRESSION_LEVEL_3)
	add_definitions(-D REGRESSION_LEVEL_OVERRIDE)
	add_definitions(-D REGRESSION_LEVEL_3)
endif(BUILD_REGRESSION_LEVEL_3)
if(BUILD_REGRESSION_STRESS OR BUILD_REGRESSION_LEVEL_4)
	add_definitions(-D REGRESSION_LEVEL_OVERRIDE)
	add_definitions(-D REGRESSION_LEVEL_1)
	add_definitions(-D REGRESSION_LEVEL_2)
	add_definitions(-D REGRESSION_LEVEL_3)
	add_definitions(-D REGRESSION_LEVEL_4)
endif(BUILD_REGRESSION_STRESS OR BUILD_REGRESSION_LEVEL_4)

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
#	add_definitions(-D _CRT_SECURE_NO_WARNINGS)
#	add_definitions(-D _SCL_SECURE_NO_WARNINGS)

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
	# set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} /Qspectre")
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
        #message(STATUS "Add test ${test_name} from source ${new_source}.")
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
# in case the project name changes
# message(STATUS "${PROJECT_NAME} -> ${PROJECT_NAME_NOSPACES}")
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

#####  Gather git repo related information
# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# propagate the git information to any executables that might want to use it
add_definitions("-DGIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
add_definitions("-DGIT_BRANCH=${GIT_BRANCH}")

set(PROJECT_VERSION "${PROJECT_VERSION}.${GIT_COMMIT_HASH}")

message(STATUS "")
message(STATUS "PROJECT_NAME                = ${PROJECT_NAME}")
message(STATUS "PROJECT_NAME_NOSPACES       = ${PROJECT_NAME_NOSPACES}")
message(STATUS "PROJECT_SOURCE_DIR          = ${PROJECT_SOURCE_DIR}")
message(STATUS "PROJECT_VERSION             = ${PROJECT_VERSION}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR    = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR    = ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "GIT_COMMIT_HASH             = ${GIT_COMMIT_HASH}")
message(STATUS "GIT_BRANCH                  = ${GIT_BRANCH}")
message(STATUS "include_install_dir         = ${include_install_dir}")
message(STATUS "include_install_dir_full    = ${include_install_dir_full}")
message(STATUS "config_install_dir          = ${config_install_dir}")
message(STATUS "include_install_dir_postfix = ${include_install_dir_postfix}")

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

if(BUILD_ALL)
	# set the grouped components to build (will trigger builds when tested)
	set(BUILD_DEMONSTRATION ON)
	set(BUILD_NUMERICAL_CHECKS ON)
	set(BUILD_BENCHMARKS ON)
	set(BUILD_REGRESSION_SUITES ON)
	set(BUILD_MIXEDPRECISION_SDK ON)

	# build the BLAS test/verification suites
	set(BUILD_BLAS ON)
	set(BUILD_VMATH ON)

	# build the C API library
	set(BUILD_C_API_PURE_LIB ON)
	set(BUILD_C_API_SHIM_LIB ON)

	# build IEEE float/double quire capability
	set(BUILD_IEEE_FLOAT_QUIRES ON)

	# build the HW validation environment
	set(BUILD_VALIDATION_HW ON)
endif(BUILD_ALL)

if(BUILD_DEMONSTRATION)
	# core demonstration example applications that use the library
	set(BUILD_EDUCATION ON)
	set(BUILD_APPLICATIONS ON)
	set(BUILD_CMD_LINE_TOOLS ON)

	set(BUILD_PLAYGROUND ON)
endif(BUILD_DEMONSTRATION)

if(BUILD_NUMERICAL_CHECKS)
	set(BUILD_NUMERICAL_CHALLENGES ON)
	set(BUILD_NUMERICAL_UTILS ON)
	set(BUILD_FUNCTIONS ON)
endif(BUILD_NUMERICAL_CHECKS)

if(BUILD_BENCHMARKS)
	# build the benchmark suites
	set(BUILD_BENCHMARK_PERFORMANCE ON)
	set(BUILD_BENCHMARK_ACCURACY ON)
	set(BUILD_BENCHMARK_ENERGY ON)
endif(BUILD_BENCHMARKS)

if(BUILD_REGRESSION_SUITES)
	# build the different test/verification suites for each number system
	set(BUILD_ARITHMETIC_CLASSES ON)
	set(BUILD_NATIVE_TYPES ON)
	set(BUILD_REALS ON)
	set(BUILD_INTEGERS ON)
	set(BUILD_DECIMALS ON)
	set(BUILD_FIXPNTS ON)
	set(BUILD_CFLOATS ON)
	set(BUILD_AREALS ON)
	set(BUILD_UNUM_TYPE_1 ON)
	set(BUILD_UNUM_TYPE_2 ON)
	set(BUILD_POSITS ON)
	set(BUILD_VALIDS ON)
	set(BUILD_LNS ON)
	
	# build the conversion test suites
	set(BUILD_CONVERSIONS ON)
endif(BUILD_REGRESSION_SUITES)

if(BUILD_MIXEDPRECISION_SDK)
	set(BUILD_MIXEDPRECISION_ROOTS ON)
	set(BUILD_MIXEDPRECISION_APPROXIMATE ON)
	set(BUILD_MIXEDPRECISION_INTEGRATE ON)
	set(BUILD_MIXEDPRECISION_INTERPOLATE ON)
	set(BUILD_MIXEDPRECISION_OPTIMIZE ON)
	set(BUILD_MIXEDPRECISION_CONJUGATE ON)
endif(BUILD_MIXEDPRECISION_SDK)

# Build the tests for the underlying storage classes
if(BUILD_ARITHMETIC_CLASSES)
add_subdirectory("tests/bitblock")
add_subdirectory("tests/value")
add_subdirectory("tests/blockbinary")
add_subdirectory("tests/blockfraction")
add_subdirectory("tests/blocktriple")
endif(BUILD_ARITHMETIC_CLASSES)

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

# cfloats
if(BUILD_CFLOATS)
add_subdirectory("tests/cfloat")
endif(BUILD_CFLOATS)

# conversion tests suites
if(BUILD_CONVERSION_TESTS)
add_subdirectory("tests/conversions")
endif(BUILD_CONVERSION_TESTS)


if(BUILD_MIXEDPRECISION_ROOTS)
add_subdirectory("mixedprecision/roots")
endif(BUILD_MIXEDPRECISION_ROOTS)

if(BUILD_MIXEDPRECISION_APPROXIMATE)
add_subdirectory("mixedprecision/approximation")
endif(BUILD_MIXEDPRECISION_APPROXIMATE)

if(BUILD_MIXEDPRECISION_INTEGRATE)
add_subdirectory("mixedprecision/integration")
endif(BUILD_MIXEDPRECISION_INTEGRATE)

if(BUILD_MIXEDPRECISION_INTERPOLATE)
add_subdirectory("mixedprecision/interpolation")
endif(BUILD_MIXEDPRECISION_INTERPOLATE)

if(BUILD_MIXEDPRECISION_OPTIMIZE)
add_subdirectory("mixedprecision/optimization")
endif(BUILD_MIXEDPRECISION_OPTIMIZE)

if(BUILD_MIXEDPRECISION_CONJUGATE)
add_subdirectory("mixedprecision/linearalgebra/cg")
endif(BUILD_MIXEDPRECISION_CONJUGATE)

# performance benchmarks
if(BUILD_BENCHMARK_PERFORMANCE)
add_subdirectory("benchmark/performance/blas")
add_subdirectory("benchmark/performance/arithmetic/decimal")
add_subdirectory("benchmark/performance/arithmetic/integer")
add_subdirectory("benchmark/performance/arithmetic/fixpnt")
add_subdirectory("benchmark/performance/arithmetic/cfloat")
add_subdirectory("benchmark/performance/arithmetic/areal")
add_subdirectory("benchmark/performance/arithmetic/lns")
add_subdirectory("benchmark/performance/arithmetic/posit")
add_subdirectory("benchmark/performance/arithmetic/valid")
add_subdirectory("benchmark/performance/arithmetic/unum")
endif(BUILD_BENCHMARK_PERFORMANCE)

# accuracy benchmarks
if(BUILD_BENCHMARK_ACCURACY)
add_subdirectory("benchmark/accuracy/blas")
endif(BUILD_BENCHMARK_ACCURACY)

# energy benchmarks
if(BUILD_BENCHMARK_ENERGY)
add_subdirectory("benchmark/energy/blas")
endif(BUILD_BENCHMARK_ENERGY)

# command line tools and utilities 
if(BUILD_CMD_LINE_TOOLS)
add_subdirectory("tools/cmd")
endif(BUILD_CMD_LINE_TOOLS)

# educational examples
if(BUILD_EDUCATION)
add_subdirectory("education/tables")
add_subdirectory("education/number")
add_subdirectory("education/quire")
endif(BUILD_EDUCATION)

# application examples
if(BUILD_APPLICATIONS)
add_subdirectory("applications/blas")
add_subdirectory("applications/chaos")
add_subdirectory("applications/chebyshev")
add_subdirectory("applications/complex")
add_subdirectory("applications/cryptography")
add_subdirectory("applications/dsp")
add_subdirectory("applications/engineering")
add_subdirectory("applications/math")
add_subdirectory("applications/numeric")
add_subdirectory("applications/ode")
add_subdirectory("applications/pde")
add_subdirectory("applications/roots")
add_subdirectory("applications/science")
add_subdirectory("applications/sequences")
add_subdirectory("applications/stl")
add_subdirectory("applications/trigonometry")
add_subdirectory("applications/weather")
endif(BUILD_APPLICATIONS)

# numerical benchmarks and challenge problems
if(BUILD_NUMERICAL_CHALLENGES)
add_subdirectory("tests/numerical/properties")
endif(BUILD_NUMERICAL_CHALLENGES)
if(BUILD_NUMERICAL_UTILS)
add_subdirectory("tests/numerical/utils")
endif(BUILD_NUMERICAL_UTILS)
if(BUILD_FPBENCH)
add_subdirectory("tests/numerical/fpbench")
endif(BUILD_FPBENCH)

if(BUILD_VALIDATION_HW)
add_subdirectory("validation/hw")
endif(BUILD_VALIDATION_HW)

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

# Basic Linear Algebra library
if(BUILD_BLAS)
add_subdirectory("tests/blas")
add_subdirectory("tests/vmath")
endif(BUILD_BLAS)

####
# Configuration summary
include(tools/cmake/summary.cmake)
universal_print_configuration_summary()
print_footer()
