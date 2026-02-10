SHELL := /bin/sh

CMAKE ?= cmake
CTEST ?= ctest

empty :=
space := $(empty) $(empty)

NINJA_DETECTED := $(strip $(shell $(CMAKE) -DQUERY=NINJA -P tools/cmake/host_info.cmake))
GEN ?= $(if $(NINJA_DETECTED),Ninja,Unix Makefiles)
GEN_DIR := $(subst $(space),_,$(GEN))

TOOLCHAIN ?= default
BUILD_TYPE ?= Debug
MODE ?= normal
UNITY ?= 0
BUILD_ALL_AND_CAPI ?= 1
VS_CLANGCL ?= 0
VS_ARCH ?= x64
ARCH ?= $(strip $(shell $(CMAKE) -DQUERY=ARCH -P tools/cmake/host_info.cmake))
JOBS ?= $(strip $(shell $(CMAKE) -DQUERY=JOBS -P tools/cmake/host_info.cmake))
CTEST_ARGS ?=
CMAKE_LOG_LEVEL ?= VERBOSE
CMAKE_DEFINES_EXTRA ?=
HOST_OS = $(strip $(shell $(CMAKE) -DQUERY=HOST_OS -P tools/cmake/host_info.cmake))
CTEST_DETECTED = $(strip $(shell $(CMAKE) -DQUERY=PROG -DPROG_NAME=ctest -P tools/cmake/host_info.cmake))
CLANG_DETECTED = $(strip $(shell $(CMAKE) -DQUERY=PROG -DPROG_NAME=clang -P tools/cmake/host_info.cmake))
GCC_DETECTED = $(strip $(shell $(CMAKE) -DQUERY=PROG -DPROG_NAME=gcc -P tools/cmake/host_info.cmake))
CL_DETECTED = $(strip $(shell $(CMAKE) -DQUERY=PROG -DPROG_NAME=cl -P tools/cmake/host_info.cmake))
LLVM_PROFDATA_DETECTED = $(strip $(shell $(CMAKE) -DQUERY=PROG -DPROG_NAME=llvm-profdata -P tools/cmake/host_info.cmake))
LLVM_COV_DETECTED = $(strip $(shell $(CMAKE) -DQUERY=PROG -DPROG_NAME=llvm-cov -P tools/cmake/host_info.cmake))
GCOVR_DETECTED = $(strip $(shell $(CMAKE) -DQUERY=PROG -DPROG_NAME=gcovr -P tools/cmake/host_info.cmake))
DETECTED_TOOL_ANY = $(strip $(NINJA_DETECTED)$(CTEST_DETECTED)$(CLANG_DETECTED)$(GCC_DETECTED)$(CL_DETECTED)$(LLVM_PROFDATA_DETECTED)$(LLVM_COV_DETECTED)$(GCOVR_DETECTED))

SILENT_MODE := $(filter -s --silent --quiet,$(MAKEFLAGS))
ifneq ($(SILENT_MODE),)
  ifeq ($(origin CMAKE_LOG_LEVEL), default)
    CMAKE_LOG_LEVEL := NOTICE
  endif
  BUILD_VERBOSE :=
else
  BUILD_VERBOSE := --verbose
endif

ifeq ($(TOOLCHAIN),clang)
  TOOLCHAIN_ARGS := -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
else ifeq ($(TOOLCHAIN),gcc)
  TOOLCHAIN_ARGS := -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
else
  TOOLCHAIN_ARGS :=
endif

ifeq ($(UNITY),1)
  UNITY_ARG := -DCMAKE_UNITY_BUILD=ON
else
  UNITY_ARG := -DCMAKE_UNITY_BUILD=OFF
endif

ifneq ($(BUILD_ALL_AND_CAPI),0)
  BUILD_ALL_ARGS := -DUNIVERSAL_BUILD_ALL=ON -DUNIVERSAL_BUILD_C_API_SHIM_LIB=ON
else
  BUILD_ALL_ARGS := -DUNIVERSAL_BUILD_ALL=OFF -DUNIVERSAL_BUILD_C_API_SHIM_LIB=OFF
endif

IS_VS_GEN := $(findstring Visual Studio,$(GEN))
VS_SUFFIX :=
ifneq ($(IS_VS_GEN),)
  VS_SUFFIX := _vs$(VS_ARCH)
endif
ifneq ($(IS_VS_GEN),)
  ifeq ($(TOOLCHAIN),clang)
    ifeq ($(origin VS_CLANGCL), default)
      VS_CLANGCL := 1
    endif
  endif
endif
VS_GEN_ARGS :=
ifneq ($(IS_VS_GEN),)
  VS_GEN_ARGS += -A $(VS_ARCH)
  ifeq ($(VS_CLANGCL),1)
    VS_GEN_ARGS += -T ClangCL
  endif
endif
ifneq ($(IS_VS_GEN),)
  ifeq ($(TOOLCHAIN),clang)
    # Clang-cl is selected via the VS generator toolset; avoid forcing C/C++ compiler paths.
    TOOLCHAIN_ARGS :=
  endif
endif

CMAKE_DEFINES_MODE ?=

BUILD_DIR = build/$(GEN_DIR)_$(TOOLCHAIN)_$(BUILD_TYPE)_$(MODE)_u$(UNITY)_c$(BUILD_ALL_AND_CAPI)_$(ARCH)$(VS_SUFFIX)

CONFIGURE_ARGS = -S . -B $(BUILD_DIR) -G "$(GEN)" $(VS_GEN_ARGS) --log-level $(CMAKE_LOG_LEVEL) \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	$(TOOLCHAIN_ARGS) \
	$(UNITY_ARG) \
	$(BUILD_ALL_ARGS) \
	-DUNIVERSAL_COVERAGE_JOBS=$(JOBS) \
	-DUNIVERSAL_COVERAGE_CTEST_ARGS:STRING="$(CTEST_ARGS)" \
	$(CMAKE_DEFINES_MODE) \
	$(CMAKE_DEFINES_EXTRA)

.PHONY: all configure build test sanitize coverage coverage-report clean distclean print-config help check-tools

all: build

configure:
	@$(CMAKE) $(CONFIGURE_ARGS)

build: configure
	@$(CMAKE) --build $(BUILD_DIR) $(BUILD_VERBOSE) --parallel $(JOBS)

test: build
	@$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure -j $(JOBS) $(CTEST_ARGS)

sanitize: MODE := san
sanitize: BUILD_ALL_AND_CAPI := 1
sanitize: CMAKE_DEFINES_MODE := -DUNIVERSAL_ENABLE_ASAN=ON -DUNIVERSAL_ENABLE_UBSAN=ON -DUNIVERSAL_BUILD_ALL=ON -DUNIVERSAL_BUILD_C_API_PURE_LIB=ON -DUNIVERSAL_BUILD_C_API_SHIM_LIB=ON -DUNIVERSAL_BUILD_C_API_LIB_PIC=ON
sanitize: test

coverage: MODE := cov
coverage: CMAKE_DEFINES_MODE := -DUNIVERSAL_ENABLE_COVERAGE=ON
coverage: build coverage-report

coverage-report: configure
	@$(CMAKE) --build $(BUILD_DIR) --target coverage-report

clean:
	@$(CMAKE) --build $(BUILD_DIR) --target clean

distclean:
	@$(CMAKE) -E rm -rf build

print-config:
	@echo "GEN=$(GEN)"
	@echo "TOOLCHAIN=$(TOOLCHAIN)"
	@echo "BUILD_TYPE=$(BUILD_TYPE)"
	@echo "MODE=$(MODE)"
	@echo "UNITY=$(UNITY)"
	@echo "BUILD_ALL_AND_CAPI=$(BUILD_ALL_AND_CAPI)"
	@echo "ARCH=$(ARCH)"
	@echo "JOBS=$(JOBS)"
	@echo "CTEST_ARGS=$(CTEST_ARGS)"
	@echo "CMAKE_LOG_LEVEL=$(CMAKE_LOG_LEVEL)"
	@echo "CMAKE_DEFINES_EXTRA=$(CMAKE_DEFINES_EXTRA)"
	@echo "BUILD_DIR=$(BUILD_DIR)"

help:
	@echo "Targets:"
	@echo "  - build (default)   Configure and build using current settings"
	@echo "  - configure         Generate build files only"
	@echo "  - test              Run tests with CTest"
	@echo "  - sanitize          Configure, build, and run tests with sanitizers"
	@echo "  - coverage          Configure, build, and run tests with coverage"
	@echo "  - clean             Remove build outputs for current config"
	@echo "  - distclean         Remove all build directories"
	@echo "  - print-config      Show resolved build configuration"
	@echo "  - check-tools       Verify required tools are available"
	@echo "Environment variables:"
	@echo "  - GEN                           CMake generator (default: Ninja when found, else Unix Makefiles)"
	@echo "  - TOOLCHAIN=default|clang|gcc   Select compiler toolchain"
	@echo "  - BUILD_TYPE=Debug|Release      CMake build type"
	@echo "  - MODE=normal|san|cov           Build mode (normal, sanitizers, or coverage)"
	@echo "  - UNITY=0|1                     Toggle CMake unity build"
	@echo "  - BUILD_ALL_AND_CAPI=0|1        Build all targets and C API shim"
	@echo "  - ARCH                          Override detected host architecture"
	@echo "  - JOBS=N                        Parallel build and test jobs"
	@echo "  - CTEST_ARGS=\"...\"              Extra arguments for ctest"
	@echo "  - CMAKE_LOG_LEVEL=VERBOSE       CMake log verbosity"
	@echo "  - CMAKE_DEFINES_EXTRA=...       Extra CMake -D definitions"
	@$(if $(IS_VS_GEN),echo "  - VS_CLANGCL=0|1                Use clang-cl toolset with Visual Studio generator",#)
	@$(if $(IS_VS_GEN),echo "  - VS_ARCH=x64                   Visual Studio generator platform",#)
	@echo "Detected system:"
	@echo "  - OS          Host operating system: $(HOST_OS)"
	@echo "  - Arch        Host CPU architecture: $(ARCH)"
	@echo "  - Jobs        Default parallel jobs: $(JOBS)"
	@echo "  - Generator   Active CMake generator: $(GEN)"
	@echo "Detected tools:"
	@$(if $(NINJA_DETECTED),echo "  - Ninja           Ninja build tool: $(NINJA_DETECTED)",#)
	@$(if $(CTEST_DETECTED),echo "  - ctest           CTest runner for make test: $(CTEST_DETECTED)",#)
	@$(if $(CLANG_DETECTED),echo "  - clang           Clang C/C++ compiler: $(CLANG_DETECTED)",#)
	@$(if $(GCC_DETECTED),echo "  - gcc             GCC C/C++ compiler: $(GCC_DETECTED)",#)
	@$(if $(CL_DETECTED),echo "  - cl.exe          MSVC compiler: $(CL_DETECTED)",#)
	@$(if $(LLVM_PROFDATA_DETECTED),$(CMAKE) -E echo "  - llvm-profdata   LLVM coverage data tool: $(LLVM_PROFDATA_DETECTED)",#)
	@$(if $(LLVM_COV_DETECTED),echo "  - llvm-cov        LLVM coverage tool: $(LLVM_COV_DETECTED)",#)
	@$(if $(GCOVR_DETECTED),echo "  - gcovr           GCovr coverage tool: $(GCOVR_DETECTED)",#)
	@$(if $(DETECTED_TOOL_ANY),,echo "  - none            No optional tools detected; run 'make check-tools' for a full report")
	@echo "Notes:"
	@$(if $(CL_DETECTED),echo "  - Sanitizers      cl.exe supports best-effort ASan only; prefer clang/clang-cl/gcc",#)
	@$(if $(CL_DETECTED),echo "  - Coverage        cl.exe coverage is limited; prefer clang/clang-cl/gcc",#)
	@$(if $(IS_VS_GEN),echo "  - Visual Studio   With GEN=\"Visual Studio 17 2022\" and TOOLCHAIN=clang; default is clang-cl (set VS_CLANGCL=0 to keep MSVC)",#)
	@echo "  - Quiet           make -s reduces verbosity and log level if not explicitly set"

check-tools:
	@$(CMAKE) -P tools/cmake/check-tools.cmake
