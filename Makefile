SHELL := /bin/sh

# Optional GNU Make wrapper. CMake remains the canonical build interface.
CMAKE ?= cmake
CTEST ?= ctest
NINJA ?= $(shell command -v ninja 2>/dev/null)
GEN ?= $(if $(NINJA),Ninja,Unix Makefiles)
CONFIG ?= Release
TOOLCHAIN ?= default
CMAKE_ARGS ?=

empty :=
space := $(empty) $(empty)

# Prefer platform-native core detection and fall back to 2 total jobs.
JOBS ?= $(shell /bin/sh -c 'n=""; \
  if command -v sysctl >/dev/null 2>&1; then n=$$(sysctl -n hw.ncpu 2>/dev/null); fi; \
  if [ -z "$$n" ] && command -v getconf >/dev/null 2>&1; then n=$$(getconf _NPROCESSORS_ONLN 2>/dev/null); fi; \
  if [ -z "$$n" ] && command -v nproc >/dev/null 2>&1; then n=$$(nproc 2>/dev/null); fi; \
  if [ -z "$$n" ]; then n=1; fi; \
  expr "$$n" + 0 >/dev/null 2>&1 || n=1; \
  expr $$n + 1')
CTEST_JOBS ?= $(JOBS)

GEN_TAG := $(subst $(space),_,$(GEN))
TOOLCHAIN_TAG := $(if $(strip $(TOOLCHAIN)),$(TOOLCHAIN),default)

DEFAULT_BUILD_DIR ?= build/$(GEN_TAG)_$(TOOLCHAIN_TAG)_$(CONFIG)_a0u0c0
TEST_BUILD_DIR := build/$(GEN_TAG)_$(TOOLCHAIN_TAG)_$(CONFIG)_all_a0u0c0
SANITIZE_BUILD_DIR := build/$(GEN_TAG)_$(TOOLCHAIN_TAG)_$(CONFIG)_all_a1u1c0
COVERAGE_BUILD_DIR := build/$(GEN_TAG)_$(TOOLCHAIN_TAG)_$(CONFIG)_all_a0u0c1
BUILD_DIR ?= $(DEFAULT_BUILD_DIR)

ifeq ($(TOOLCHAIN),clang)
TOOLCHAIN_ARGS := -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
else ifeq ($(TOOLCHAIN),gcc)
TOOLCHAIN_ARGS := -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
else
TOOLCHAIN_ARGS :=
endif

COMMON_CONFIGURE_ARGS = -S . -G "$(GEN)" -DCMAKE_BUILD_TYPE=$(CONFIG) $(TOOLCHAIN_ARGS) $(CMAKE_ARGS)
DEFAULT_CONFIGURE_ARGS = $(COMMON_CONFIGURE_ARGS) -B $(BUILD_DIR) -DUNIVERSAL_ENABLE_TESTS=ON -DUNIVERSAL_ENABLE_ASAN=OFF -DUNIVERSAL_ENABLE_UBSAN=OFF -DUNIVERSAL_ENABLE_COVERAGE=OFF
# This repo registers the large CTest suite only when the build-bearing subtrees are enabled.
TEST_CONFIGURE_ARGS = $(COMMON_CONFIGURE_ARGS) -B $(TEST_BUILD_DIR) -DUNIVERSAL_ENABLE_TESTS=ON -DUNIVERSAL_BUILD_ALL=ON -DUNIVERSAL_ENABLE_ASAN=OFF -DUNIVERSAL_ENABLE_UBSAN=OFF -DUNIVERSAL_ENABLE_COVERAGE=OFF
SANITIZE_CONFIGURE_ARGS = $(COMMON_CONFIGURE_ARGS) -B $(SANITIZE_BUILD_DIR) -DUNIVERSAL_ENABLE_TESTS=ON -DUNIVERSAL_BUILD_ALL=ON -DUNIVERSAL_ENABLE_ASAN=ON -DUNIVERSAL_ENABLE_UBSAN=ON -DUNIVERSAL_ENABLE_COVERAGE=OFF
COVERAGE_CONFIGURE_ARGS = $(COMMON_CONFIGURE_ARGS) -B $(COVERAGE_BUILD_DIR) -DUNIVERSAL_ENABLE_TESTS=ON -DUNIVERSAL_BUILD_ALL=ON -DUNIVERSAL_ENABLE_ASAN=OFF -DUNIVERSAL_ENABLE_UBSAN=OFF -DUNIVERSAL_ENABLE_COVERAGE=ON

.PHONY: all configure build test sanitize coverage clean help

all: build

configure:
	@command -v $(CMAKE) >/dev/null 2>&1 || { echo "cmake not found"; exit 1; }
	@$(CMAKE) $(DEFAULT_CONFIGURE_ARGS)

build: configure
	@$(CMAKE) --build $(BUILD_DIR) --config $(CONFIG) --parallel $(JOBS)

test:
	@command -v $(CMAKE) >/dev/null 2>&1 || { echo "cmake not found"; exit 1; }
	@command -v $(CTEST) >/dev/null 2>&1 || { echo "ctest not found"; exit 1; }
	@$(CMAKE) $(TEST_CONFIGURE_ARGS)
	@$(CMAKE) --build $(TEST_BUILD_DIR) --config $(CONFIG) --parallel $(JOBS)
	@$(CTEST) --test-dir $(TEST_BUILD_DIR) --output-on-failure -C $(CONFIG) -j$(CTEST_JOBS)

sanitize:
	@command -v $(CMAKE) >/dev/null 2>&1 || { echo "cmake not found"; exit 1; }
	@command -v $(CTEST) >/dev/null 2>&1 || { echo "ctest not found"; exit 1; }
	@$(CMAKE) $(SANITIZE_CONFIGURE_ARGS)
	@$(CMAKE) --build $(SANITIZE_BUILD_DIR) --config $(CONFIG) --parallel $(JOBS)
	@$(CTEST) --test-dir $(SANITIZE_BUILD_DIR) --output-on-failure -C $(CONFIG) -j$(CTEST_JOBS)

coverage:
	@command -v $(CMAKE) >/dev/null 2>&1 || { echo "cmake not found"; exit 1; }
	@$(CMAKE) $(COVERAGE_CONFIGURE_ARGS)
	@$(CMAKE) --build $(COVERAGE_BUILD_DIR) --config $(CONFIG) --parallel $(JOBS)
	@$(CMAKE) --build $(COVERAGE_BUILD_DIR) --config $(CONFIG) --target coverage

clean:
	@rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR) $(SANITIZE_BUILD_DIR) $(COVERAGE_BUILD_DIR)

help:
	@echo "Targets:"
	@echo "  all        Configure and build the default out-of-tree build"
	@echo "  configure  Configure the default out-of-tree build"
	@echo "  build      Build the default out-of-tree build"
	@echo "  test       Configure, build, and run the full CTest suite"
	@echo "  sanitize   Configure, build, and run the full CTest suite with ASan+UBSan"
	@echo "  coverage   Configure a dedicated coverage tree and build the CMake coverage target"
	@echo "  clean      Remove all wrapper-managed build trees"
	@echo "  help       Show this help text"
	@echo ""
	@echo "Variables:"
	@echo "  GEN=<generator>     CMake generator (default: Ninja if available, else Unix Makefiles)"
	@echo "  CONFIG=<type>       CMake build type (default: Release)"
	@echo "  TOOLCHAIN=<name>    Compiler selection: default, clang, or gcc"
	@echo "  JOBS=<n>            Parallelism for build and ctest (default: cores + 1)"
	@echo "  CTEST_JOBS=<n>      Parallelism for ctest only (default: JOBS)"
	@echo "  CMAKE_ARGS='...'    Extra arguments passed to CMake configure"
	@echo ""
	@echo "Build directories:"
	@echo "  default   $(BUILD_DIR)"
	@echo "  test      $(TEST_BUILD_DIR)"
	@echo "  sanitize  $(SANITIZE_BUILD_DIR)"
	@echo "  coverage  $(COVERAGE_BUILD_DIR)"
