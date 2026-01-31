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
MODE ?= dev
UNITY ?= 0
BUILD_ALL_AND_CAPI ?= 0
ARCH ?= $(strip $(shell $(CMAKE) -DQUERY=ARCH -P tools/cmake/host_info.cmake))
JOBS ?= $(strip $(shell $(CMAKE) -DQUERY=JOBS -P tools/cmake/host_info.cmake))
CTEST_ARGS ?=
CMAKE_LOG_LEVEL ?= VERBOSE
CMAKE_DEFINES_EXTRA ?=

SILENT_MODE := $(filter s --silent --quiet,$(MAKEFLAGS))
ifneq ($(SILENT_MODE),)
  ifeq ($(origin CMAKE_LOG_LEVEL), default)
    CMAKE_LOG_LEVEL := WARNING
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

CMAKE_DEFINES_MODE ?=

BUILD_DIR := build/$(GEN_DIR)/$(TOOLCHAIN)/$(BUILD_TYPE)/$(MODE)/unity$(UNITY)/allcapi$(BUILD_ALL_AND_CAPI)/$(ARCH)

CONFIGURE_ARGS := -S . -B $(BUILD_DIR) -G "$(GEN)" --log-level $(CMAKE_LOG_LEVEL) \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	$(TOOLCHAIN_ARGS) \
	$(UNITY_ARG) \
	$(BUILD_ALL_ARGS) \
	-DUNIVERSAL_COVERAGE_JOBS=$(JOBS) \
	-DUNIVERSAL_COVERAGE_CTEST_ARGS:STRING="$(CTEST_ARGS)" \
	$(CMAKE_DEFINES_MODE) \
	$(CMAKE_DEFINES_EXTRA)

.PHONY: all configure build test sanitize coverage coverage-report clean distclean print-config help doctor

all: build

configure:
	@$(CMAKE) $(CONFIGURE_ARGS)

build: configure
	@$(CMAKE) --build $(BUILD_DIR) $(BUILD_VERBOSE) -j $(JOBS)

test: build
	@$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure -j $(JOBS) $(CTEST_ARGS)

sanitize: MODE := san
sanitize: CMAKE_DEFINES_MODE := -DUNIVERSAL_ENABLE_SANITIZERS=ON
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
	@$(CMAKE) -E echo "GEN=$(GEN)"
	@$(CMAKE) -E echo "TOOLCHAIN=$(TOOLCHAIN)"
	@$(CMAKE) -E echo "BUILD_TYPE=$(BUILD_TYPE)"
	@$(CMAKE) -E echo "MODE=$(MODE)"
	@$(CMAKE) -E echo "UNITY=$(UNITY)"
	@$(CMAKE) -E echo "BUILD_ALL_AND_CAPI=$(BUILD_ALL_AND_CAPI)"
	@$(CMAKE) -E echo "ARCH=$(ARCH)"
	@$(CMAKE) -E echo "JOBS=$(JOBS)"
	@$(CMAKE) -E echo "CTEST_ARGS=$(CTEST_ARGS)"
	@$(CMAKE) -E echo "CMAKE_LOG_LEVEL=$(CMAKE_LOG_LEVEL)"
	@$(CMAKE) -E echo "CMAKE_DEFINES_EXTRA=$(CMAKE_DEFINES_EXTRA)"
	@$(CMAKE) -E echo "BUILD_DIR=$(BUILD_DIR)"

help:
	@$(CMAKE) -E echo "Targets: build (default), configure, test, sanitize, coverage, clean, distclean, print-config, doctor"
	@$(CMAKE) -E echo "Knobs: TOOLCHAIN=default|clang|gcc BUILD_TYPE=Debug|Release MODE=dev|san|cov UNITY=0|1 JOBS=N"
	@$(CMAKE) -E echo "       CTEST_ARGS=\"...\" BUILD_ALL_AND_CAPI=0|1 CMAKE_LOG_LEVEL=VERBOSE CMAKE_DEFINES_EXTRA=..."
	@$(CMAKE) -E echo "Quiet: make -s (reduces verbosity and log level if not explicitly set)"

doctor:
	@$(CMAKE) -P tools/cmake/doctor.cmake
