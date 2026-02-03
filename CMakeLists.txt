########################################################################################
# CMakeList.txt
########################################################################################
include(tools/cmake/banners.cmake)
print_header()

####
# Set minimum version of CMake. 
cmake_minimum_required(VERSION 3.22)

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
  set(UNIVERSAL_VERSION_MINOR 94)
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
# Set the path to the data directory
set(TEST_MATRIX_DATA_DIR "${CMAKE_SOURCE_DIR}/data/matrices")
# Configure a header file that will contain the data directory path

configure_file(
    "${CMAKE_SOURCE_DIR}/config/TestMatrixDataDirConfig.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/TestMatrixDataDirConfig.hpp"
)

# Include the binary directory to find the generated header file
include_directories("${CMAKE_BINARY_DIR}/generated")


####
# Set build options
# MSVC generates SSE/SSE2 code by default. No support for SSE3 as of 7/15/2018
option(UNIVERSAL_USE_SSE3                          "Build code with SSE3 ISA support"                    OFF)
option(UNIVERSAL_USE_AVX                           "Build code with AVX ISA support"                     OFF)
option(UNIVERSAL_USE_AVX2                          "Build code with AVX2 ISA support"                    OFF)

# compiler configuration messages
option(UNIVERSAL_VERBOSE_BUILD                     "Enable verbose compiler specialization messages"     OFF)

# control which projects get enabled
# Continuous Integration override to build all
option(UNIVERSAL_BUILD_ALL                         "Set to ON to build all components"                   OFF)
option(UNIVERSAL_BUILD_CI                          "Set to ON to build ci components"                    OFF)

# or subsets of the components
option(UNIVERSAL_BUILD_DEMONSTRATION               "Set to ON to build all demonstration components"      ON)
option(UNIVERSAL_BUILD_NUMBERS                     "Set to ON to build all the number systems"           OFF)
option(UNIVERSAL_BUILD_NUMBER_CASCADES             "Set to ON to build all cascade tests"                OFF)
option(UNIVERSAL_BUILD_NUMERICS                    "Set to ON to build all the numeric components"       OFF)
option(UNIVERSAL_BUILD_BENCHMARKS                  "Set to ON to build all the benchmark components"     OFF)
option(UNIVERSAL_BUILD_MIXEDPRECISION_SDK          "Set to ON to build the mixed-precision SDK"          OFF)
option(UNIVERSAL_BUILD_COMPLEX                     "Set to ON to build complex support components"       OFF)
option(UNIVERSAL_BUILD_CONCURRENCY                 "Set to ON to build concurrent components"            OFF)

# or, build individual components
# utilities and educational examples
option(UNIVERSAL_BUILD_TOOLS_CMD_LINE              "Set to ON to build cmd line tools"                   OFF)
option(UNIVERSAL_BUILD_TOOLS_UTILS                 "Set to ON to build utilities"                        OFF)
option(UNIVERSAL_BUILD_TOOLS_CLOSURE               "Set to ON to build closure plot tools"               OFF)
option(UNIVERSAL_BUILD_PLAYGROUND                  "Set to ON to build experimentation playground"       OFF)
option(UNIVERSAL_BUILD_EDUCATION                   "Set to ON to build educational examples"             OFF)
option(UNIVERSAL_BUILD_EDUCATION_INTERACTIVE       "Set to ON to build interactive tutorials"            OFF)
option(UNIVERSAL_BUILD_APPLICATIONS                "Set to ON to build application examples"             OFF)

# numerical and special math functions
option(UNIVERSAL_BUILD_NUMERIC_FUNCTIONS           "Set to ON to build special function programs"        OFF)
option(UNIVERSAL_BUILD_NUMERIC_FAITHFUL            "Set to ON to build faithful arithmetic tests"        OFF)
option(UNIVERSAL_BUILD_NUMERIC_QUIRES              "Set to ON to build quire programs"                   OFF)
option(UNIVERSAL_BUILD_NUMERIC_CHALLENGES          "Set to ON to build numerical challenge programs"     OFF)
option(UNIVERSAL_BUILD_NUMERIC_UTILS               "Set to ON to build numerical utilities"              OFF)
option(UNIVERSAL_BUILD_NUMERIC_FPBENCH             "Set to ON to build fpbench benchmarks"               OFF)

# type hierachy, conversions, casting, coercions, and juggling
option(UNIVERSAL_BUILD_TYPE_HIERARCHY              "Set to ON to build type hierarchy tests"             OFF)

# application environment tests
option(UNIVERSAL_BUILD_APP_ENVIRONMENT             "Set to ON to build applicatoin environment tests"    OFF)

# C API library and test programs
option(UNIVERSAL_BUILD_C_API_PURE_LIB              "Set to ON to build C API native library"             OFF)
option(UNIVERSAL_BUILD_C_API_SHIM_LIB              "Set to ON to build C API shim library"               OFF)
option(UNIVERSAL_BUILD_C_API_LIB_PIC               "Set to ON to compile C API library with -fPIC"       OFF)

# number systems and their verification suites
option(UNIVERSAL_BUILD_NUMBER_INTERNALS            "Set to ON to build internal arithmetic type tests"   OFF)
option(UNIVERSAL_BUILD_NUMBER_NATIVE_TYPES         "Set to ON to build native type tests"                OFF)

option(UNIVERSAL_BUILD_NUMBER_ELASTICS             "Set to ON to build arbitrary precision tests"        OFF)
option(UNIVERSAL_BUILD_NUMBER_EINTEGERS            "Set to ON to build elastic einteger tests"           OFF)
option(UNIVERSAL_BUILD_NUMBER_EFLOATS              "Set to ON to build elastic efloat tests"             OFF)
option(UNIVERSAL_BUILD_NUMBER_EDECIMALS            "Set to ON to build elastic edecimal tests"           OFF)
option(UNIVERSAL_BUILD_NUMBER_ERATIONALS           "Set to ON to build elastic erational tests"          OFF)
option(UNIVERSAL_BUILD_NUMBER_EREALS               "Set to ON to build elastic ereals tests"             OFF)
option(UNIVERSAL_BUILD_NUMBER_ELREALS              "Set to ON to build exact lazy reals tests"           OFF)

option(UNIVERSAL_BUILD_NUMBER_STATICS              "Set to ON to build static arithmetic type tests"     OFF)
option(UNIVERSAL_BUILD_NUMBER_INTEGERS             "Set to ON to build static integer tests"             OFF)
option(UNIVERSAL_BUILD_NUMBER_DECIMALS             "Set to ON to build static decimal tests"             OFF)
option(UNIVERSAL_BUILD_NUMBER_RATIONALS            "Set to ON to build static rational tests"            OFF)
option(UNIVERSAL_BUILD_NUMBER_FIXPNTS              "Set to ON to build static fixed-point tests"         OFF)
option(UNIVERSAL_BUILD_NUMBER_BFLOATS              "Set to ON to build static bfloat tests"              OFF)
option(UNIVERSAL_BUILD_NUMBER_CFLOATS              "Set to ON to build static cfloat tests"              OFF)
option(UNIVERSAL_BUILD_NUMBER_DFLOATS              "Set to ON to build static dfloat tests"              OFF)
option(UNIVERSAL_BUILD_NUMBER_DOUBLE_DOUBLE        "Set to ON to build static dd tests"                  OFF)
option(UNIVERSAL_BUILD_NUMBER_QUAD_DOUBLE          "Set to ON to build static qd tests"                  OFF)
option(UNIVERSAL_BUILD_NUMBER_DD_CASCADE           "Set to ON to build static dd cascade tests"          OFF)
option(UNIVERSAL_BUILD_NUMBER_TD_CASCADE           "Set to ON to build static td cascade tests"          OFF)
option(UNIVERSAL_BUILD_NUMBER_QD_CASCADE           "Set to ON to build static qd cascade tests"          OFF)
option(UNIVERSAL_BUILD_NUMBER_AREALS               "Set to ON to build static areal tests"               OFF)
option(UNIVERSAL_BUILD_NUMBER_UNUM1S               "Set to ON to build static unum type 1 tests"         OFF)
option(UNIVERSAL_BUILD_NUMBER_UNUM2S               "Set to ON to build static unum type 2 tests"         OFF)
option(UNIVERSAL_BUILD_NUMBER_POSITS               "Set to ON to build static unum type 3 posit tests"   OFF)
option(UNIVERSAL_BUILD_NUMBER_POSITOS              "Set to ON to build static unum type 3 posito tests"  OFF)
option(UNIVERSAL_BUILD_NUMBER_VALIDS               "Set to ON to build static unum type 3 valid tests"   OFF)
option(UNIVERSAL_BUILD_NUMBER_TAKUMS               "Set to ON to build static takum tests"               OFF)
option(UNIVERSAL_BUILD_NUMBER_LNS                  "Set to ON to build static lns tests"                 OFF)
option(UNIVERSAL_BUILD_NUMBER_DBNS                 "Set to ON to build static dbns tests"                OFF)
option(UNIVERSAL_BUILD_NUMBER_SORNS                "Set to ON to build static SORN tests"                OFF)
# conversion test suites
option(UNIVERSAL_BUILD_NUMBER_CONVERSIONS          "Set to ON to build conversion test suites"           OFF)

# Basic Linear Algebra tests
option(UNIVERSAL_BUILD_LINEAR_ALGEBRA_BLAS         "Set to ON to build the BLAS tests"                   OFF)
option(UNIVERSAL_BUILD_LINEAR_ALGEBRA_VMATH        "Set to ON to build the vector math lib"              OFF)
option(UNIVERSAL_BUILD_LINEAR_ALGEBRA_DATA         "Set to ON to build the data prep math lib"           OFF)

# benchmarking
option(UNIVERSAL_BUILD_BENCHMARK_ERROR             "Set to ON to build error benchmarks"                 OFF)
option(UNIVERSAL_BUILD_BENCHMARK_ACCURACY          "Set to ON to build accuracy benchmarks"              OFF)
option(UNIVERSAL_BUILD_BENCHMARK_RANGE             "Set to ON to build dynamic range benchmarks"         OFF)
option(UNIVERSAL_BUILD_BENCHMARK_REPRODUCIBILITY   "Set to ON to build reproducibility benchmarks"       OFF)
option(UNIVERSAL_BUILD_BENCHMARK_PERFORMANCE       "Set to ON to build performance benchmarks"           OFF)
option(UNIVERSAL_BUILD_BENCHMARK_ENERGY            "Set to ON to build energy efficiency benchmarks"     OFF)

# mixed-precision algorithm design and optimization
option(UNIVERSAL_BUILD_MIXEDPRECISION_ROOTS         "Set to ON to build mixed-precision root finders"    OFF)
option(UNIVERSAL_BUILD_MIXEDPRECISION_APPROXIMATE   "Set to ON to build mixed-precision approximation"   OFF)
option(UNIVERSAL_BUILD_MIXEDPRECISION_INTEGRATE     "Set to ON to build mixed-precision integration"     OFF)
option(UNIVERSAL_BUILD_MIXEDPRECISION_INTERPOLATE   "Set to ON to build mixed-precision interpolation"   OFF)
option(UNIVERSAL_BUILD_MIXEDPRECISION_OPTIMIZE      "Set to ON to build mixed-precision optimization"    OFF)
option(UNIVERSAL_BUILD_MIXEDPRECISION_TENSOR        "Set to ON to build mixed-precision tensor algebra"  OFF)

# validation
option(UNIVERSAL_BUILD_VALIDATION_MATH             "Set to ON to build math validation testbenches"      OFF)
option(UNIVERSAL_BUILD_VALIDATION_HW               "Set to ON to build hw validation testbenches"        OFF)

# regression test intensity
# sanity is level 1 (=min) regression testing
option(UNIVERSAL_BUILD_REGRESSION_SANITY           "Set to ON to enable regression level sanity"         ON)
# stress is level 4 (= max) regression testing
option(UNIVERSAL_BUILD_REGRESSION_STRESS           "Set to ON to enable regression level stress"         OFF)
# four levels of intensity
option(UNIVERSAL_BUILD_REGRESSION_LEVEL_1          "Set to ON to enable regression level 1"              OFF)
option(UNIVERSAL_BUILD_REGRESSION_LEVEL_2          "Set to ON to enable regression level 2"              OFF)
option(UNIVERSAL_BUILD_REGRESSION_LEVEL_3          "Set to ON to enable regression level 3"              OFF)
option(UNIVERSAL_BUILD_REGRESSION_LEVEL_4          "Set to ON to enable regression level 4"              OFF)

# Sanitizer options (GCC/Clang only)
option(UNIVERSAL_ENABLE_ASAN                       "Enable AddressSanitizer"                             OFF)
option(UNIVERSAL_ENABLE_UBSAN                      "Enable UndefinedBehaviorSanitizer"                   OFF)

# documentation
option(UNIVERSAL_BUILD_DOCS                        "Set to ON to build documentation"                    OFF)

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
# Generate compile_commands.json for clang-tidy and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

####
# Configure the compiler options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#message(STATUS "C++17 has been enabled by default")
message(STATUS "C++20 has been enabled by default")

# enable the regression testing level of the build
if(UNIVERSAL_BUILD_REGRESSION_STRESS)
	message(STATUS "UNIVERSAL_BUILD_REGRESSION_STRESS   ${UNIVERSAL_BUILD_REGRESSION_STRESS}")
	set(UNIVERSAL_BUILD_REGRESSION_LEVEL_4 ON)
	set(UNIVERSAL_BUILD_REGRESSION_SANITY OFF)
elseif(UNIVERSAL_BUILD_REGRESSION_SANITY)
	message(STATUS "UNIVERSAL_BUILD_REGRESSION_SANITY   ${UNIVERSAL_BUILD_REGRESSION_SANITY}")
	set(UNIVERSAL_BUILD_REGRESSION_LEVEL_1 ON)
endif(UNIVERSAL_BUILD_REGRESSION_STRESS)

if(UNIVERSAL_BUILD_REGRESSION_LEVEL_4)
	message(STATUS "UNIVERSAL_BUILD_REGRESSION_LEVEL_4  ${UNIVERSAL_BUILD_REGRESSION_LEVEL_4}")
	add_definitions(-D REGRESSION_LEVEL_OVERRIDE)
	add_definitions(-D REGRESSION_LEVEL_1=1)
	add_definitions(-D REGRESSION_LEVEL_2=1)
	add_definitions(-D REGRESSION_LEVEL_3=1)
	add_definitions(-D REGRESSION_LEVEL_4=1)
elseif(UNIVERSAL_BUILD_REGRESSION_LEVEL_3)
	message(STATUS "UNIVERSAL_BUILD_REGRESSION_LEVEL_3  ${UNIVERSAL_BUILD_REGRESSION_LEVEL_3}")
	add_definitions(-D REGRESSION_LEVEL_OVERRIDE)
	add_definitions(-D REGRESSION_LEVEL_1=1)
	add_definitions(-D REGRESSION_LEVEL_2=1)
	add_definitions(-D REGRESSION_LEVEL_3=1)
	add_definitions(-D REGRESSION_LEVEL_4=0)
elseif(UNIVERSAL_BUILD_REGRESSION_LEVEL_2)
	message(STATUS "UNIVERSAL_BUILD_REGRESSION_LEVEL_2  ${UNIVERSAL_BUILD_REGRESSION_LEVEL_2}")
	add_definitions(-D REGRESSION_LEVEL_OVERRIDE)
	add_definitions(-D REGRESSION_LEVEL_1=1)
	add_definitions(-D REGRESSION_LEVEL_2=1)
	add_definitions(-D REGRESSION_LEVEL_3=0)
	add_definitions(-D REGRESSION_LEVEL_4=0)
elseif(UNIVERSAL_BUILD_REGRESSION_LEVEL_1)
	message(STATUS "UNIVERSAL_BUILD_REGRESSION_LEVEL_1  ${UNIVERSAL_BUILD_REGRESSION_LEVEL_1}")
	add_definitions(-D REGRESSION_LEVEL_OVERRIDE)
	add_definitions(-D REGRESSION_LEVEL_1=1)
	add_definitions(-D REGRESSION_LEVEL_2=0)
	add_definitions(-D REGRESSION_LEVEL_3=0)
	add_definitions(-D REGRESSION_LEVEL_4=0)
else(UNIVERSAL_BUILD_REGRESSION_LEVEL_4)
	message(STATUS "Nothing has been set")
endif(UNIVERSAL_BUILD_REGRESSION_LEVEL_4)

# Enable verbose compiler specialization messages
if(UNIVERSAL_VERBOSE_BUILD)
	message(STATUS "UNIVERSAL_VERBOSE_BUILD         ${UNIVERSAL_VERBOSE_BUILD}")
	add_definitions(-DUNIVERSAL_VERBOSE_BUILD)
endif(UNIVERSAL_VERBOSE_BUILD)

# Compiler specific environments
if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag("-msse3" COMPILER_HAS_SSE3_FLAG)
	check_cxx_compiler_flag("-mavx"  COMPILER_HAS_AVX_FLAG)
	check_cxx_compiler_flag("-mavx2" COMPILER_HAS_AVX2_FLAG)

	# Streaming SIMD Extension (SSE) ISA
	if (UNIVERSAL_USE_SSE3 AND COMPILER_HAS_SSE_FLAG)
		add_definitions(-DLIB_USE_SSE)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -msse3")
	endif(UNIVERSAL_USE_SSE3 AND COMPILER_HAS_SSE_FLAG)
	# Advanced Vector Extensions (AVX) ISA
	if (UNIVERSAL_USE_AVX AND COMPILER_HAS_AVX_FLAG)
		add_definitions(-DLIB_USE_AVX)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -mavx")
	endif(UNIVERSAL_USE_AVX AND COMPILER_HAS_AVX_FLAG)
	# Advanced Vector Extensions 2 (AVX2) ISA
	if (UNIVERSAL_USE_AVX2 AND COMPILER_HAS_AVX2_FLAG)
		add_definitions(-DLIB_USE_AVX2)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -mavx2")
	endif(UNIVERSAL_USE_AVX2 AND COMPILER_HAS_AVX2_FLAG)

	# include code quality flags
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wall -Wpedantic -Wno-narrowing -Wno-deprecated")
	# specific flags for debug and release builds
	#set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -O3 -DNDEBUG")
	#set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} -g3 -pthread")

	# Clang specific: disable floating-point contraction to match MSVC /fp:precise behavior
	# Clang defaults to -ffp-contract=fast which enables FMA contractions that change
	# intermediate rounding, causing test failures on x86 with -O3 optimization.
	# GCC defaults to -ffp-contract=off, and MSVC uses /fp:precise.
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -ffp-contract=off")
	endif()

	# Sanitizer support (GCC/Clang only)
	if(UNIVERSAL_ENABLE_ASAN)
		message(STATUS "AddressSanitizer enabled")
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
	endif()

	if(UNIVERSAL_ENABLE_UBSAN)
		message(STATUS "UndefinedBehaviorSanitizer enabled")
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
	endif()

elseif(MSVC)
	# Streaming SIMD Extension (SSE) ISA
	if (UNIVERSAL_USE_SSE3)
		add_definitions(-DLIB_USE_SSE3)
		set(COMPILER_HAS_SSE3_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:SSE3")
	endif(UNIVERSAL_USE_SSE3)
	# Advanced Vector Extensions (AVX) ISA
	if (UNIVERSAL_USE_AVX)
		add_definitions(-DLIB_USE_AVX)
		set(COMPILER_HAS_AVX_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:AVX")
	endif(UNIVERSAL_USE_AVX)
	# Advanced Vector Extensions 2 (AVX2) ISA
	if (UNIVERSAL_USE_AVX2)
		add_definitions(-DLIB_USE_AVX2)
		set(COMPILER_HAS_AVX2_FLAG true)
		set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /arch:AVX2")
	endif(UNIVERSAL_USE_AVX2)

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
      set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /Zc:__cplusplus")
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
	set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} /Oi /Ot /Ox /Oy /fp:precise /GS-")
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

	add_compile_options(/wd5045) # disable Spectre mitigation linker warning
	add_compile_options(/wd4514) # disable unreferenced function has been removed warning
	add_compile_options(/wd4515) # disable unreferenced inline function has been removed warning
	add_compile_options(/wd4710) # disable function not inlined warning
	add_compile_options(/wd4820) # disable padding added warning
	add_compile_options(/wd5262) # disable implicit fall-through warning

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
# Set include directory that contains all the different components
# universal: number systems
# blas: basic linear algebra
# numeric: containers
# functino: general functions
include_directories("./include/sw")

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
# macro to create an executable target consisting of all cpp files in a directory
# and create a test target for that cpp file
macro (compile_multifile_target testing test_name folder)
    message(STATUS "Add test ${test_name} from source folder ${folder}.")
    add_executable (${test_name} ${ARGN})

    #add_custom_target(valid SOURCES ${SOURCES})
    set_target_properties(${test_name} PROPERTIES FOLDER ${folder})
    if (${testing} STREQUAL "true")
        if (UNIVERSAL_CMAKE_TRACE)
            message(STATUS "testing: ${test_name} ${RUNTIME_OUTPUT_DIRECTORY}/${test_name}")
        endif()
        add_test(${test_name} ${RUNTIME_OUTPUT_DIRECTORY}/${test_name})
    endif()
endmacro (compile_multifile_target)

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
message(STATUS "CMAKE_C_COMPILER            = ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER          = ${CMAKE_CXX_COMPILER}")
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

if(UNIVERSAL_BUILD_ALL)
	# ALL and CI are exclusive
	set(UNIVERSAL_BUILD_CI OFF)
	# set the grouped components to build (will trigger builds when tested)
	set(UNIVERSAL_BUILD_DEMONSTRATION ON)
	set(UNIVERSAL_BUILD_NUMBERS ON)
	set(UNIVERSAL_BUILD_NUMBER_CASCADES ON)
	set(UNIVERSAL_BUILD_NUMERICS ON)
	set(UNIVERSAL_BUILD_BENCHMARKS ON)
	set(UNIVERSAL_BUILD_TYPE_HIERARCHY ON)
	set(UNIVERSAL_BUILD_MIXEDPRECISION_SDK ON)
	set(UNIVERSAL_BUILD_APP_ENVIRONMENT ON)
	# we now have a standalone complex<> solution
	set(UNIVERSAL_BUILD_COMPLEX ON)
	set(UNIVERSAL_BUILD_CONCURRENCY ON)

	# build the BLAS test/verification suites
	set(UNIVERSAL_BUILD_LINEAR_ALGEBRA_BLAS ON)
	set(UNIVERSAL_BUILD_LINEAR_ALGEBRA_VMATH ON)
	set(UNIVERSAL_BUILD_LINEAR_ALGEBRA_DATA ON)

	# build the C API library
	set(UNIVERSAL_BUILD_C_API_PURE_LIB ON)
	set(UNIVERSAL_BUILD_C_API_SHIM_LIB ON)
	set(UNIVERSAL_BUILD_C_API_LIB_PIC ON)

	# build the HW validation environment
	set(UNIVERSAL_BUILD_VALIDATION_HW ON)
endif(UNIVERSAL_BUILD_ALL)

# set the grouped components to build Continuous Integration regression suite
# we are disabling COMPLEX because we need a solution to the 
# disappearing support of complex<> for user-defined types
if(UNIVERSAL_BUILD_CI)
	set(UNIVERSAL_BUILD_NUMBERS ON)
	set(UNIVERSAL_BUILD_DEMONSTRATION OFF)
	set(UNIVERSAL_BUILD_NUMERICS OFF)
	set(UNIVERSAL_BUILD_MIXEDPRECISION_SDK OFF)
	set(UNIVERSAL_BUILD_APP_ENVIRONMENT ON)

	# we now have a standalone complex<> solution
	set(UNIVERSAL_BUILD_COMPLEX ON)
	set(UNIVERSAL_BUILD_CONCURRENCY ON)

	# build the C API library
	set(UNIVERSAL_BUILD_C_API_PURE_LIB ON)
	set(UNIVERSAL_BUILD_C_API_SHIM_LIB ON)

endif(UNIVERSAL_BUILD_CI)

# core demonstration example applications that use the library
if(UNIVERSAL_BUILD_DEMONSTRATION)
	set(UNIVERSAL_BUILD_EDUCATION ON)
	set(UNIVERSAL_BUILD_APPLICATIONS ON)
	set(UNIVERSAL_BUILD_TOOLS_CMD_LINE ON)
	set(UNIVERSAL_BUILD_TOOLS_UTILS ON)
	set(UNIVERSAL_BUILD_PLAYGROUND ON)
endif(UNIVERSAL_BUILD_DEMONSTRATION)

# enable complex environment components
if(UNIVERSAL_BUILD_COMPLEX)
	# sw::universal::complex<T> is a standalone complex number implementation
	# that works with user-defined floating-point types, bypassing the
	# ISO C++ 26.2/2 restriction that std::complex<T> is only valid for
	# float, double, and long double.
	set(UNIVERSAL_BUILD_APPLICATIONS ON)
	set(UNIVERSAL_BUILD_NUMBER_FIXPNTS ON)
	set(UNIVERSAL_BUILD_NUMBER_CFLOATS ON)
	set(UNIVERSAL_BUILD_NUMBER_POSITS ON)
endif(UNIVERSAL_BUILD_COMPLEX)

if(UNIVERSAL_BUILD_NUMBERS)
	# build the different test/verification suites for each number system
	set(UNIVERSAL_BUILD_NUMBER_INTERNALS ON)
	set(UNIVERSAL_BUILD_NUMBER_NATIVE_TYPES ON)
	set(UNIVERSAL_BUILD_NUMBER_ELASTICS ON)
	set(UNIVERSAL_BUILD_NUMBER_STATICS ON)
	
	# build the conversion test suites
	set(UNIVERSAL_BUILD_NUMBER_CONVERSIONS ON)
endif(UNIVERSAL_BUILD_NUMBERS)

if(UNIVERSAL_BUILD_NUMBER_ELASTICS)
	set(UNIVERSAL_BUILD_NUMBER_EINTEGERS ON)
	set(UNIVERSAL_BUILD_NUMBER_EDECIMALS ON)
	set(UNIVERSAL_BUILD_NUMBER_ERATIONALS ON)
	set(UNIVERSAL_BUILD_NUMBER_EFLOATS ON)
	set(UNIVERSAL_BUILD_NUMBER_EREALS ON)
endif(UNIVERSAL_BUILD_NUMBER_ELASTICS)

if(UNIVERSAL_BUILD_NUMBER_STATICS)
	set(UNIVERSAL_BUILD_NUMBER_INTEGERS ON)
	set(UNIVERSAL_BUILD_NUMBER_DECIMALS ON)
	set(UNIVERSAL_BUILD_NUMBER_RATIONALS ON)
	set(UNIVERSAL_BUILD_NUMBER_FIXPNTS ON)
	set(UNIVERSAL_BUILD_NUMBER_BFLOATS ON)
	set(UNIVERSAL_BUILD_NUMBER_CFLOATS ON)
	set(UNIVERSAL_BUILD_NUMBER_DFLOATS ON)
	set(UNIVERSAL_BUILD_NUMBER_DOUBLE_DOUBLE ON)
	set(UNIVERSAL_BUILD_NUMBER_QUAD_DOUBLE ON)
	set(UNIVERSAL_BUILD_NUMBER_DD_CASCADE ON)
	set(UNIVERSAL_BUILD_NUMBER_TD_CASCADE ON)
	set(UNIVERSAL_BUILD_NUMBER_QD_CASCADE ON)
	set(UNIVERSAL_BUILD_NUMBER_AREALS ON)
	set(UNIVERSAL_BUILD_NUMBER_UNUM1S ON)
	set(UNIVERSAL_BUILD_NUMBER_UNUM2S ON)
	set(UNIVERSAL_BUILD_NUMBER_POSITS ON)
	set(UNIVERSAL_BUILD_NUMBER_POSITOS ON)
	set(UNIVERSAL_BUILD_NUMBER_VALIDS ON)
	set(UNIVERSAL_BUILD_NUMBER_TAKUMS ON)
	set(UNIVERSAL_BUILD_NUMBER_LNS ON)
	set(UNIVERSAL_BUILD_NUMBER_DBNS ON)
	set(UNIVERSAL_BUILD_NUMBER_SORNS ON)
endif(UNIVERSAL_BUILD_NUMBER_STATICS)

if(UNIVERSAL_BUILD_NUMBER_CASCADES)
	set(UNIVERSAL_BUILD_NUMBER_DD_CASCADE ON)
	set(UNIVERSAL_BUILD_NUMBER_TD_CASCADE ON)
	set(UNIVERSAL_BUILD_NUMBER_QD_CASCADE ON)
endif(UNIVERSAL_BUILD_NUMBER_CASCADES)

# build numerical tools and tests
if(UNIVERSAL_BUILD_NUMERICS)
	set(UNIVERSAL_BUILD_NUMERIC_FUNCTIONS ON)
	set(UNIVERSAL_BUILD_NUMERIC_FAITHFUL ON)
	set(UNIVERSAL_BUILD_NUMERIC_QUIRES ON)
	set(UNIVERSAL_BUILD_NUMERIC_CHALLENGES ON)
	set(UNIVERSAL_BUILD_NUMERIC_UTILS ON)
	set(UNIVERSAL_BUILD_NUMERIC_FPBENCH ON)
endif(UNIVERSAL_BUILD_NUMERICS)

# build the benchmark suites
if(UNIVERSAL_BUILD_BENCHMARKS)
	set(UNIVERSAL_BUILD_BENCHMARK_ERROR ON)
	set(UNIVERSAL_BUILD_BENCHMARK_ACCURACY ON)
	set(UNIVERSAL_BUILD_BENCHMARK_RANGE ON)
	set(UNIVERSAL_BUILD_BENCHMARK_REPRODUCIBILITY ON)
	set(UNIVERSAL_BUILD_BENCHMARK_ENERGY ON)
	set(UNIVERSAL_BUILD_BENCHMARK_PERFORMANCE ON)
endif(UNIVERSAL_BUILD_BENCHMARKS)

if(UNIVERSAL_BUILD_MIXEDPRECISION_SDK)
	set(UNIVERSAL_BUILD_MIXEDPRECISION_ROOTS ON)
	set(UNIVERSAL_BUILD_MIXEDPRECISION_APPROXIMATE ON)
	set(UNIVERSAL_BUILD_MIXEDPRECISION_INTEGRATE ON)
	set(UNIVERSAL_BUILD_MIXEDPRECISION_INTERPOLATE ON)
	set(UNIVERSAL_BUILD_MIXEDPRECISION_OPTIMIZE ON)
	set(UNIVERSAL_BUILD_MIXEDPRECISION_TENSOR ON)
endif(UNIVERSAL_BUILD_MIXEDPRECISION_SDK)

##################################################################
###          multi-file application regression environment

if(UNIVERSAL_BUILD_APP_ENVIRONMENT)
add_subdirectory("static/appenv")
endif(UNIVERSAL_BUILD_APP_ENVIRONMENT)

##################################################################
###          arithmetic type regression environment

# Build the tests for the underlying storage classes
if(UNIVERSAL_BUILD_NUMBER_INTERNALS)
add_subdirectory("internal/bitblock")
add_subdirectory("internal/gfp")
add_subdirectory("internal/f2s")
add_subdirectory("internal/conversion")
add_subdirectory("internal/value")
add_subdirectory("internal/blockdecimal")
add_subdirectory("internal/blockbinary")
add_subdirectory("internal/blockfraction")
add_subdirectory("internal/blocksignificand")
add_subdirectory("internal/blocktriple")
add_subdirectory("internal/floatcascade")
add_subdirectory("internal/expansion")
endif(UNIVERSAL_BUILD_NUMBER_INTERNALS)

# native type int and float tests
if(UNIVERSAL_BUILD_NUMBER_NATIVE_TYPES)
add_subdirectory("static/native")
endif(UNIVERSAL_BUILD_NUMBER_NATIVE_TYPES)

###################################################################
###                  Arbitrary precision tests

if(UNIVERSAL_BUILD_NUMBER_EINTEGERS)
add_subdirectory("elastic/einteger")
endif(UNIVERSAL_BUILD_NUMBER_EINTEGERS)

if(UNIVERSAL_BUILD_NUMBER_EDECIMALS)
add_subdirectory("elastic/decimal")
endif(UNIVERSAL_BUILD_NUMBER_EDECIMALS)

if(UNIVERSAL_BUILD_NUMBER_ERATIONALS)
add_subdirectory("elastic/rational")
endif(UNIVERSAL_BUILD_NUMBER_ERATIONALS)

if(UNIVERSAL_BUILD_NUMBER_EFLOATS)
add_subdirectory("elastic/efloat")
endif(UNIVERSAL_BUILD_NUMBER_EFLOATS)

if(UNIVERSAL_BUILD_NUMBER_EREALS)
add_subdirectory("elastic/ereal")
endif(UNIVERSAL_BUILD_NUMBER_EREALS)

if(UNIVERSAL_BUILD_NUMBER_ELREALS)
add_subdirectory("elastic/elreal")
endif(UNIVERSAL_BUILD_NUMBER_ELREALS)


###################################################################
###                  fixed-size number system tests

if(UNIVERSAL_BUILD_NUMBER_INTEGERS)
add_subdirectory("static/integer")
endif(UNIVERSAL_BUILD_NUMBER_INTEGERS)

if(UNIVERSAL_BUILD_NUMBER_DECIMALS)
add_subdirectory("static/decimal")
endif(UNIVERSAL_BUILD_NUMBER_DECIMALS)

if(UNIVERSAL_BUILD_NUMBER_RATIONALS)
add_subdirectory("static/rational")
endif(UNIVERSAL_BUILD_NUMBER_RATIONALS)

if(UNIVERSAL_BUILD_NUMBER_FIXPNTS)
add_subdirectory("static/fixpnt")
endif(UNIVERSAL_BUILD_NUMBER_FIXPNTS)

if(UNIVERSAL_BUILD_NUMBER_LNS)
add_subdirectory("static/lns")
endif(UNIVERSAL_BUILD_NUMBER_LNS)

if(UNIVERSAL_BUILD_NUMBER_DBNS)
add_subdirectory("static/dbns")
endif(UNIVERSAL_BUILD_NUMBER_DBNS)

if(UNIVERSAL_BUILD_NUMBER_SORNS)
add_subdirectory("static/sorn")
endif(UNIVERSAL_BUILD_NUMBER_SORNS)

if(UNIVERSAL_BUILD_NUMBER_UNUM1S)
add_subdirectory("elastic/unum")
endif(UNIVERSAL_BUILD_NUMBER_UNUM1S)

if(UNIVERSAL_BUILD_NUMBER_UNUM2S)
add_subdirectory("static/unum2")
endif(UNIVERSAL_BUILD_NUMBER_UNUM2S)

if(UNIVERSAL_BUILD_NUMBER_POSITS)
add_subdirectory("static/posit")
add_subdirectory("static/posit/specialized")
add_subdirectory("static/posit2")
add_subdirectory("static/quire")
endif(UNIVERSAL_BUILD_NUMBER_POSITS)

if(UNIVERSAL_BUILD_NUMBER_POSITOS)
add_subdirectory("static/posito")
endif(UNIVERSAL_BUILD_NUMBER_POSITOS)

if(UNIVERSAL_BUILD_NUMBER_VALIDS)
add_subdirectory("static/valid")
endif(UNIVERSAL_BUILD_NUMBER_VALIDS)

if(UNIVERSAL_BUILD_NUMBER_TAKUMS)
add_subdirectory("static/takum")
endif(UNIVERSAL_BUILD_NUMBER_TAKUMS)

# reals with an uncertainty bit
if(UNIVERSAL_BUILD_NUMBER_AREALS)
add_subdirectory("static/areal")
endif(UNIVERSAL_BUILD_NUMBER_AREALS)

# brain floats
if(UNIVERSAL_BUILD_NUMBER_BFLOATS)
add_subdirectory("static/bfloat16")
endif(UNIVERSAL_BUILD_NUMBER_BFLOATS)

# classic floats
if(UNIVERSAL_BUILD_NUMBER_CFLOATS)
add_subdirectory("static/cfloat")
endif(UNIVERSAL_BUILD_NUMBER_CFLOATS)

# decimal floats
if(UNIVERSAL_BUILD_NUMBER_DFLOATS)
add_subdirectory("static/dfloat")
endif(UNIVERSAL_BUILD_NUMBER_DFLOATS)

# double-double floats
if(UNIVERSAL_BUILD_NUMBER_DOUBLE_DOUBLE)
add_subdirectory("static/dd")
endif(UNIVERSAL_BUILD_NUMBER_DOUBLE_DOUBLE)

# quad-double floats
if(UNIVERSAL_BUILD_NUMBER_QUAD_DOUBLE)
add_subdirectory("static/qd")
endif(UNIVERSAL_BUILD_NUMBER_QUAD_DOUBLE)

# double-double cascade floats
if(UNIVERSAL_BUILD_NUMBER_DD_CASCADE)
add_subdirectory("static/dd_cascade")
endif(UNIVERSAL_BUILD_NUMBER_DD_CASCADE)

# triple-double cascade floats
if(UNIVERSAL_BUILD_NUMBER_TD_CASCADE)
add_subdirectory("static/td_cascade")
endif(UNIVERSAL_BUILD_NUMBER_TD_CASCADE)

# quad-double cascade floats
if(UNIVERSAL_BUILD_NUMBER_QD_CASCADE)
add_subdirectory("static/qd_cascade")
endif(UNIVERSAL_BUILD_NUMBER_QD_CASCADE)

# conversion tests suites
if(UNIVERSAL_BUILD_NUMBER_CONVERSIONS)
add_subdirectory("static/conversions")
endif(UNIVERSAL_BUILD_NUMBER_CONVERSIONS)

# complex number tests (sw::universal::complex<T>)
if(UNIVERSAL_BUILD_COMPLEX)
add_subdirectory("static/complex")
endif(UNIVERSAL_BUILD_COMPLEX)

##################################################################
###          mixed-precision environment

if(UNIVERSAL_BUILD_MIXEDPRECISION_ROOTS)
add_subdirectory("mixedprecision/roots")
endif(UNIVERSAL_BUILD_MIXEDPRECISION_ROOTS)

if(UNIVERSAL_BUILD_MIXEDPRECISION_APPROXIMATE)
add_subdirectory("mixedprecision/approximation")
endif(UNIVERSAL_BUILD_MIXEDPRECISION_APPROXIMATE)

if(UNIVERSAL_BUILD_MIXEDPRECISION_INTEGRATE)
add_subdirectory("mixedprecision/integration")
endif(UNIVERSAL_BUILD_MIXEDPRECISION_INTEGRATE)

if(UNIVERSAL_BUILD_MIXEDPRECISION_INTERPOLATE)
add_subdirectory("mixedprecision/interpolation")
endif(UNIVERSAL_BUILD_MIXEDPRECISION_INTERPOLATE)

if(UNIVERSAL_BUILD_MIXEDPRECISION_OPTIMIZE)
add_subdirectory("mixedprecision/optimization")
endif(UNIVERSAL_BUILD_MIXEDPRECISION_OPTIMIZE)

if(UNIVERSAL_BUILD_MIXEDPRECISION_TENSOR)
add_subdirectory("mixedprecision/tensor")
endif(UNIVERSAL_BUILD_MIXEDPRECISION_TENSOR)

##################################################################
###          benchmark environment

# error benchmarks
if(UNIVERSAL_BUILD_BENCHMARK_ERROR)
add_subdirectory("benchmark/error/sampling")
add_subdirectory("benchmark/error/scaling")
add_subdirectory("benchmark/error/qsnr")
add_subdirectory("benchmark/error/blas")
endif(UNIVERSAL_BUILD_BENCHMARK_ERROR)

# accuracy benchmarks
if(UNIVERSAL_BUILD_BENCHMARK_ACCURACY)
add_subdirectory("benchmark/accuracy/blas")
add_subdirectory("benchmark/accuracy/quantization")
endif(UNIVERSAL_BUILD_BENCHMARK_ACCURACY)

# range benchmarks
if(UNIVERSAL_BUILD_BENCHMARK_RANGE)
add_subdirectory("benchmark/range/blas")
add_subdirectory("benchmark/range/arithmetic")
endif(UNIVERSAL_BUILD_BENCHMARK_RANGE)

# reproducibility benchmarks
if(UNIVERSAL_BUILD_BENCHMARK_REPRODUCIBILITY)
add_subdirectory("benchmark/reproducibility/blas")
endif(UNIVERSAL_BUILD_BENCHMARK_REPRODUCIBILITY)

# performance benchmarks
if(UNIVERSAL_BUILD_BENCHMARK_PERFORMANCE)
add_subdirectory("benchmark/performance/blas")
add_subdirectory("benchmark/performance/arithmetic")
endif(UNIVERSAL_BUILD_BENCHMARK_PERFORMANCE)

# energy benchmarks
if(UNIVERSAL_BUILD_BENCHMARK_ENERGY)
add_subdirectory("benchmark/energy/blas")
add_subdirectory("benchmark/energy/models")
add_subdirectory("benchmark/energy/hw_counters")
endif(UNIVERSAL_BUILD_BENCHMARK_ENERGY)

##################################################################
###          tools, utilities, education, playground environment

# command line tools and utilities
if(UNIVERSAL_BUILD_TOOLS_CMD_LINE)
add_subdirectory("tools/cmd")
endif(UNIVERSAL_BUILD_TOOLS_CMD_LINE)

if(UNIVERSAL_BUILD_TOOLS_UTILS)
add_subdirectory("tools/utils")
endif(UNIVERSAL_BUILD_TOOLS_UTILS)

if(UNIVERSAL_BUILD_TOOLS_CLOSURE)
add_subdirectory("tools/closure")
endif(UNIVERSAL_BUILD_TOOLS_CLOSURE)

# educational examples (CI-compatible, non-interactive)
if(UNIVERSAL_BUILD_EDUCATION)
add_subdirectory("education/internal")
add_subdirectory("education/number")
add_subdirectory("education/tables")
add_subdirectory("education/ranges")
add_subdirectory("education/quire")
endif(UNIVERSAL_BUILD_EDUCATION)

# interactive educational tutorials (NOT part of CI builds)
if(UNIVERSAL_BUILD_EDUCATION_INTERACTIVE)
add_subdirectory("education/interactive")
endif(UNIVERSAL_BUILD_EDUCATION_INTERACTIVE)

if(UNIVERSAL_BUILD_PLAYGROUND)
add_subdirectory("playground")
endif(UNIVERSAL_BUILD_PLAYGROUND)

##################################################################
###          application examples

# application examples
if(UNIVERSAL_BUILD_APPLICATIONS)
add_subdirectory("applications/accuracy/music")
add_subdirectory("applications/accuracy/mathematics")
add_subdirectory("applications/accuracy/science")
add_subdirectory("applications/accuracy/engineering")
add_subdirectory("applications/accuracy/ode")
add_subdirectory("applications/accuracy/pde")
add_subdirectory("applications/accuracy/roots")
add_subdirectory("applications/accuracy/optimization")
add_subdirectory("applications/accuracy/geometry")

add_subdirectory("applications/approximation/taylor")
add_subdirectory("applications/approximation/chebyshev")

add_subdirectory("applications/mixed-precision/dnn")
add_subdirectory("applications/mixed-precision/dsp")
add_subdirectory("applications/mixed-precision/imaging")

add_subdirectory("applications/performance/chaos")
add_subdirectory("applications/performance/complex")
add_subdirectory("applications/performance/stream")
add_subdirectory("applications/performance/weather")
add_subdirectory("applications/performance/ir")

add_subdirectory("applications/precision/constants")
add_subdirectory("applications/precision/floating-point")
add_subdirectory("applications/precision/math")
add_subdirectory("applications/precision/numeric")
add_subdirectory("applications/precision/tapered")

add_subdirectory("applications/reproducibility/blas")
add_subdirectory("applications/reproducibility/cryptography")
add_subdirectory("applications/reproducibility/sequences")

add_subdirectory("applications/stl")
endif(UNIVERSAL_BUILD_APPLICATIONS)

##################################################################
###          numerical utilities and challenges 

# quire capability
if(UNIVERSAL_BUILD_NUMERIC_FAITHFUL)
add_subdirectory("numeric/faithful")
endif(UNIVERSAL_BUILD_NUMERIC_FAITHFUL)
# quire capability
if(UNIVERSAL_BUILD_NUMERIC_QUIRES)
add_subdirectory("numeric/quire/ieee754")
endif(UNIVERSAL_BUILD_NUMERIC_QUIRES)
# numeric utilities
if(UNIVERSAL_BUILD_NUMERIC_UTILS)
add_subdirectory("numeric/utils")
endif(UNIVERSAL_BUILD_NUMERIC_UTILS)
# numerical benchmarks and challenge problems
if(UNIVERSAL_BUILD_NUMERIC_CHALLENGES)
add_subdirectory("numeric/properties")
endif(UNIVERSAL_BUILD_NUMERIC_CHALLENGES)
# numeric functions of interest
if(UNIVERSAL_BUILD_NUMERIC_FUNCTIONS)
add_subdirectory("numeric/functions")
endif(UNIVERSAL_BUILD_NUMERIC_FUNCTIONS)
# FPBench examples
if(UNIVERSAL_BUILD_NUMERIC_FPBENCH)
add_subdirectory("numeric/fpbench")
endif(UNIVERSAL_BUILD_NUMERIC_FPBENCH)

##################################################################
###          type hierarchy tests

# application examples
if(UNIVERSAL_BUILD_TYPE_HIERARCHY)
add_subdirectory("type_hierarchy")
endif(UNIVERSAL_BUILD_TYPE_HIERARCHY)

##################################################################
###          hardware validation environment

if(UNIVERSAL_BUILD_VALIDATION_HW)
add_subdirectory("validation/hw")
endif(UNIVERSAL_BUILD_VALIDATION_HW)

##################################################################
###          C api

# C API library and their tests
if(UNIVERSAL_BUILD_C_API_PURE_LIB)
add_subdirectory("c_api/pure_c/posit")
add_subdirectory("c_api/pure_c/test/posit")
endif(UNIVERSAL_BUILD_C_API_PURE_LIB)

if(UNIVERSAL_BUILD_C_API_SHIM_LIB)
add_subdirectory("c_api/shim/posit")
add_subdirectory("c_api/shim/test/posit")
endif(UNIVERSAL_BUILD_C_API_SHIM_LIB)

##################################################################
###          dense BLAS environment for experimentation

# Basic Linear Algebra library
if(UNIVERSAL_BUILD_LINEAR_ALGEBRA_BLAS)
add_subdirectory("linalg/blas")
endif(UNIVERSAL_BUILD_LINEAR_ALGEBRA_BLAS)
if(UNIVERSAL_BUILD_LINEAR_ALGEBRA_VMATH)
add_subdirectory("linalg/vmath")
endif(UNIVERSAL_BUILD_LINEAR_ALGEBRA_VMATH)
if(UNIVERSAL_BUILD_LINEAR_ALGEBRA_DATA)
add_subdirectory("linalg/data")
endif(UNIVERSAL_BUILD_LINEAR_ALGEBRA_DATA)

####
# Code formatting targets (clang-format)
include(tools/cmake/ClangFormat.cmake)

####
# Static analysis targets (clang-tidy)
include(tools/cmake/ClangTidy.cmake)

####
# Configuration summary
include(tools/cmake/summary.cmake)
universal_print_configuration_summary()
print_footer()
