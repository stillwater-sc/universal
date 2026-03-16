SHELL := /bin/sh

###############################################################################
## @file Makefile
## @brief Thin convenience wrapper around the Universal CMake build.
##
## CMake remains the canonical build interface for this project. This Makefile
## exists to make common local workflows easier to discover and run.
##
## High-level workflows:
##
##   make            Run the default workflow (currently: test + build)
##   make build      Configure and build core production assets (prod_core).
##   make test       Configure, build, and run all tests (debug_all).
##   make sanitize   Configure, build, and run all tests with ASan and
##                   UBSan enabled.
##   make coverage   Configure, build, and run all tests with code coverage
##                   tracking enabled to generate a code coverage report.
##
## Internal stage targets are organized as:
##
##   configure__<suffix>
##   build__<suffix>
##   test__<suffix>
##   coverage__<suffix>
##
## Supported build suffix grammar:
##
##   <kind>_<scope>[_cov][_san]
##
## where:
##
##   <kind>  = prod | debug
##   <scope> = core | all
##
## There are 16 supported suffixes in total: four one-bit choices
## (prod/debug, core/all, cov/no-cov, san/no-san).
##
## Examples:
##
##   prod_core
##   prod_all
##   debug_all_cov
##   debug_core_cov_san
##
## Notes:
##
## - "core" vs "all" controls UNIVERSAL_BUILD_ALL.
## - "_cov" enables coverage instrumentation.
## - "_san" enables ASan + UBSan. (Address Sanitizer and Undefined Behavior Sanitizer)
## - CMake already owns the internal coverage pipeline. Its "coverage" target
##   depends on its preceding "check" step, so this Makefile does not try to
##   reimplement those sub-stages.
###############################################################################

###############################################################################
## @section Small utility functions
##
## Small GNU Make helper functions used throughout this file.
###############################################################################

## Function: $(call or_default,preferred_value,default_value)
## @fn or_default(preferred_value,default_value)
## @brief Return $(1) if it is non-empty after stripping whitespace;
##        otherwise return $(2).
## @param 1 Preferred value.
## @param 2 Default value.
or_default = $(if $(strip $(1)),$(strip $(1)),$(strip $(2)))

## Function: $(call find_tool,command_or_path)
## @fn find_tool(command_or_path)
## @brief Resolve a command name or explicit path using `command -v`.
## @param 1 Command name or path-like value to resolve.
## @return The resolved executable path, or the empty string if not found.
find_tool = $(strip $(shell command -v '$(1)' 2>/dev/null))

empty :=
space := $(empty) $(empty)

###############################################################################
## @section Tool selection
##
## Resolve the main external tools used by the public workflows.
###############################################################################

## @brief Resolved cmake executable path, or empty if unavailable.
CMAKE := $(call find_tool,$(call or_default,$(CMAKE),cmake))

## @brief Resolved ctest executable path, or empty if unavailable.
CTEST := $(call find_tool,$(call or_default,$(CTEST),ctest))

## @brief Resolved ninja executable path, or empty if unavailable.
NINJA := $(call find_tool,$(call or_default,$(NINJA),ninja))

## @brief Requested generator.
##
## If Ninja is available, prefer it. Otherwise fall back to Unix Makefiles.
GEN ?= $(if $(NINJA),Ninja,Unix Makefiles)

## @brief Optional compiler-family override.
##
## Allowed values:
##   default  -> do not force the compiler in Make; let CMake choose
##   clang    -> request clang / clang++
##   gcc      -> request gcc / g++
COMPILER ?= default

## @brief Extra arguments appended to the CMake configure invocation.
CMAKE_ARGS ?=

###############################################################################
## @section Compiler validation
###############################################################################

VALID_COMPILERS := default clang gcc

ifeq ($(filter $(COMPILER),$(VALID_COMPILERS)),)
$(error COMPILER must be one of [$(VALID_COMPILERS)], not '$(COMPILER)')
endif

###############################################################################
## @section Parallelism
###############################################################################

## @brief Preferred build parallelism.
##
## Try a few platform-native mechanisms, then fall back to 1, and finally use
## cores+1 to keep local builds reasonably busy by default.
JOBS ?= $(shell /bin/sh -c 'n=""; \
	if command -v sysctl >/dev/null 2>&1; then n=$$(sysctl -n hw.ncpu 2>/dev/null); fi; \
	if [ -z "$$n" ] && command -v getconf >/dev/null 2>&1; then n=$$(getconf _NPROCESSORS_ONLN 2>/dev/null); fi; \
	if [ -z "$$n" ] && command -v nproc >/dev/null 2>&1; then n=$$(nproc 2>/dev/null); fi; \
	if [ -z "$$n" ]; then n=1; fi; \
	expr "$$n" + 0 >/dev/null 2>&1 || n=1; \
	expr $$n + 1')
CTEST_JOBS ?= $(JOBS)

###############################################################################
## @section Suffix grammar
##
## Helpers for recognizing, validating, tokenizing, and querying supported
## build suffixes.
###############################################################################

## @brief All 16 supported build suffixes.
##
## The supported matrix is:
##
##   prod_all         prod_core         debug_all         debug_core
##   prod_all_cov     prod_core_cov     debug_all_cov     debug_core_cov
##   prod_all_san     prod_core_san     debug_all_san     debug_core_san
##   prod_all_cov_san prod_core_cov_san debug_all_cov_san debug_core_cov_san
##
ALL_BUILD_SUFFIXES := $(strip $(foreach \
  build,$\
  prod debug,$\
  $(foreach \
    subset,$\
    all core,$\
    $(build)_$(subset)         \
    $(build)_$(subset)_cov     \
    $(build)_$(subset)_san     \
    $(build)_$(subset)_cov_san \
  )$\
))

## @name Suffix parsing and query helpers
## @{

## Function: $(call get_suffix,suffix_or_symbol_with_suffix)
## @fn get_suffix(suffix_or_symbol_with_suffix)
## @brief Recover every matching legal build suffix from the input string.
##
## This function is intentionally tolerant: it accepts either:
##
## - a suffix by itself, such as `prod_all_cov`
## - a larger underscore-delimited string ending in that suffix, such as
##   `configure_prod_all_cov`
##
## Valid usage is expected to produce exactly one suffix.
##
## @param 1 Candidate string that is or ends with a legal build suffix.
## @return Zero or more matching legal suffixes.
get_suffix = $(strip \
  $(foreach suffix,$(ALL_BUILD_SUFFIXES),\
    $(if $(filter $(suffix) %_$(suffix),$1),$(suffix),)))

## Function: $(call valid_suffix,suffix_or_symbol_with_suffix)
## @fn valid_suffix(suffix_or_symbol_with_suffix)
## @brief Return the one valid legal suffix from the input, or empty if invalid.
## @param 1 Candidate string to normalize to a suffix.
valid_suffix = $(if $(filter 1,$(words $(call get_suffix,$1))),$(call get_suffix,$1),)

## Function: $(call has_suffix_token,suffix_or_symbol_with_suffix,token)
## @fn has_suffix_token(suffix_or_symbol_with_suffix,token)
## @brief Return token $(2) if validated suffix $(1) contains that token.
## @param 1 Candidate string containing a suffix.
## @param 2 Token to search for.
has_suffix_token = $(filter $(2),$(subst _,$(space),$(call valid_suffix,$1)))

## Function: $(call is_debug,suffix_or_symbol_with_suffix)
## @fn is_debug(suffix_or_symbol_with_suffix)
## @brief Return `debug` if the validated suffix is a debug build, else empty.
## @param 1 Candidate string containing a suffix.
is_debug = $(call has_suffix_token,$1,debug)

## Function: $(call is_all,suffix_or_symbol_with_suffix)
## @fn is_all(suffix_or_symbol_with_suffix)
## @brief Return `all` if the validated suffix enables UNIVERSAL_BUILD_ALL,
##        else empty.
## @param 1 Candidate string containing a suffix.
is_all = $(call has_suffix_token,$1,all)

## Function: $(call is_coverage,suffix_or_symbol_with_suffix)
## @fn is_coverage(suffix_or_symbol_with_suffix)
## @brief Return `coverage` if the validated suffix enables coverage,
##        else empty.
## @param 1 Candidate string containing a suffix.
is_coverage = $(if $(call has_suffix_token,$1,cov),coverage,$(empty))

## Function: $(call is_sanitize,suffix_or_symbol_with_suffix)
## @fn is_sanitize(suffix_or_symbol_with_suffix)
## @brief Return `sanitize` if the validated suffix enables sanitizers,
##        else empty.
## @param 1 Candidate string containing a suffix.
is_sanitize = $(if $(call has_suffix_token,$1,san),sanitize,$(empty))

## Function: $(call profile_build_type,suffix_or_symbol_with_suffix)
## @fn profile_build_type(suffix_or_symbol_with_suffix)
## @brief Return the CMake build type for the validated suffix.
## @param 1 Candidate string containing a suffix.
profile_build_type = $(if $(call is_debug,$1),Debug,Release)

## Function: $(call predicate_on_off,predicate,suffix_or_symbol_with_suffix)
## @fn predicate_on_off(predicate,suffix_or_symbol_with_suffix)
## @brief Return `ON` or `OFF` for a predicate applied to a suffix.
## @param 1 Predicate function name.
## @param 2 Candidate string containing a suffix.
predicate_on_off = $(if $(call $1,$2),ON,OFF)

## @}

###############################################################################
## @section Generator / compiler naming
###############################################################################

## @name Compiler selection helpers
## @{

## The makefile allows compiler toolchain selection using the
## `$(COMPILER)` variable, which may be `gcc`, `clang`, or `default`.
## `default` leaves the CMake C and C++ compiler settings unset, so
## CMake can select the correct default. When `gcc` or `clang` is
## selected, `gcc` and `g++` or `clang` and `clang++` are specified
## to CMake as the C and C++ compilers. On most platforms, this is fine,
## but on macOS, Clang is usually installed as both `clang` **and**
## `gcc` (presumably for backward compatibility); but this means that
## CMake may be told to use Clang when the user requested GCC.

## To remedy this, this makefile asks each compiler to identify itself,
## so the user will get the compiler they requested, instead of the
## first match in the path.

## These functions are to facilitate that verification:

## Function: $(call escape_spaces,in_string)
## @fn escape_spaces(in_string)
## @brief Escape spaces and dollar signs.
## @param 1 the string to escape.
escape_spaces = $(subst $(space),$$(space),$(subst $$,$$(dollars),$1))

## Function: $(call unescape_spaces,in_string)
## @fn unescape_spaces(in_string)
## @brief Unescape spaces and dollar signs.
## @param 1 the string to unescape.
unescape_spaces = $(subst $$(dollars),$$,$(subst $$(space),$(space),$1))

## Function: $(call split_on_colon,in_string)
## @fn split_on_colon(in_string)
## @brief Split colon delimited string.
## @param 1 the string to split.
split_on_colon = $(subst :,$(space),$1)

## Function: $(call safe_wildcard,path_patterns)
## @fn safe_wildcard(path_patterns)
## @brief glob expand a space delimited list of escaped path patterns.
## @param 1 the list of escaped path patterns.
safe_wildcard = $(foreach w,$1,$(shell ls -1 $(call unescape_spaces,$(subst $$(dollar),'$$(dollar)',$(subst $$(space),"$$(space)",$(w)))) 2> /dev/null | sed -e 's,\$$,\$$(dollar),g' -e 's, ,\$$(space),g'))

## Function: $(call egrep_escaped,regex,in_strings)
## @fn egrep_escaped(regex,in_strings)
## @brief perform egrep on escaped input strings.
## @param 1 the enhanced regular expression.
## @param 2 the list of escaped input strings.
egrep_escaped = $(foreach w,$2,$(shell echo '$(call unescape_spaces,$(w))' | grep -E '$1' | sed -e 's,\$$,\$$(dollar),g' -e 's, ,\$$(space),g'))

## Function: $(call search_path_for_tool_candidates,path,tool_name)
## @fn search_path_for_tool_candidates(path,tool_name)
## @brief Search provided colon delimited "path-like" list of directories for
## tools beginning with provided name (that is whole-word delimited).
## @param 1 the colon delimited search path.
## @param 2 the whole word tool name.
search_path_for_tool_candidates = $(strip $(call egrep_escaped,\b$2\b(([^/][^/+]|[^/+])[^/]*)?$$,$(call safe_wildcard,$(foreach each,$(call split_on_colon,$(call escape_spaces,$1)),$(each)/$2*))))

## Function: $(call filter_version_compatible,tool_paths)
## @fn filter_version_compatible(tool_paths)
## @brief Filter out tools that don't return 0 when executed with
## `--version` command line switch.
## @param 1 space-delimited, escaped tool path names.
filter_version_compatible = $(strip $(foreach t,$1,$(if $(shell '$(call unescape_spaces,$(t))' --version > /dev/null 2> /dev/null && echo ok),$(t),)))

## Function: $(call scrape_compiler_defines,tool_path)
## @fn scrape_compiler_defines(tool_path)
## @brief Treat tool as a C compiler, and ask it to dump all its
## predefined CPP macros. We then scrape the result for the two macros
## we care about to differentiate Clang, GCC and other compilers.
## The two macros we care about are `__GNUC__` and `__clang__`
## @param 1 escaped tool path name.
scrape_compiler_defines = $(filter __%,$(shell echo | '$(call unescape_spaces,$(1))' -dM -E -x c - 2> /dev/null | grep -E '\b__(GNUC|clang)__\b'))

## Function: $(call identify_compiler_by_defines,list_of_macros)
## @fn identify_compiler_by_defines(list_of_macros)
## @brief Based on the result of $(call scrape_compiler_defines,...), identify the compiler.
## if __clang__ is present, ignore __GNUC__, it's `clang`
## if __clang__ is absent and __GNUC__ is present, it's `gcc`
## otherwise it's `unknown`.
## @param 1 space delimited list of relevant macros.
identify_compiler_by_defines = $(if $(filter __clang__,$1),clang,$(if $(filter __GNUC__,$1),gcc,unknown))

## Function: $(call search_path_for_specified_compiler,path,compiler_name)
## @fn search_path_for_specified_compiler(path,compiler_name)
## @brief Use the functions above to find the correct compiler.
## @param 1 the colon delimited search path.
## @param 2 the compiler name. (only gcc and clang supported)
search_path_for_specified_compiler = $(call unescape_spaces,$(firstword $(foreach c,$(call filter_version_compatible,$(call search_path_for_tool_candidates,$1,$2)),$(if $(filter $2,$(call identify_compiler_by_defines,$(call scrape_compiler_defines,$c))),$c,))))

## Function: $(call get_cpp_compiler_path,c_compiler_path)
## @fn get_cpp_compiler_path(c_compiler_path)
## @brief Substitute name of C++ compiler for C compiler name.
## @param 1 path of gcc or clang compiler.
get_cpp_compiler_path = $(if $1,$(call unescape_spaces,$(dir $(call escape_spaces,$1))$(subst gcc,g++,$(subst clang,clang++,$(notdir $(call escape_spaces,$1))))),)

## @}

## @brief  `Ninja` if `$(GEN)` is set to `Ninja`.
IS_NINJA = $(if $(filter Ninja,$(GEN)),Ninja,$(empty))

## @brief Generator name normalized for filesystem use.
GEN_TAG := $(subst $(space),_,$(GEN))

## @brief Compiler override name used only for naming wrapper-managed build dirs.
##
## This reflects the requested override from Make, not necessarily the actual
## compiler family CMake chooses when COMPILER=default.
COMPILER_TAG := $(COMPILER)

## If COMPILER is NOT "default" or empty, search `$(PATH)`
## for requested compiler.
## (At this point, `$(COMPILER)` should only be `gcc`, `clang`
## or `default`. Empty should not be possible)
ifneq ($(filter-out default,$(COMPILER)),)


## @brief (Using functions from above:) Search system `$(PATH)` for
## requested compiler, verify that it has a name we expect, understands
## `--version`, and reports CPP macros indicating that it's the requested
## compiler family.
CC_BIN := $(call search_path_for_specified_compiler,$(PATH),$(COMPILER))

## @brief Derive C++ compiler filename from C compiler filename. While
## we don't verify this here, we verify it later with `tool__CC_BIN`
## and `tool__CXX_BIN` prerequisites.
CXX_BIN := $(call get_cpp_compiler_path,$(CC_BIN))

ifeq ($(strip $(CC_BIN)),)
$(error $(COMPILER) not found in PATH)
endif

COMPILER_ARGS := -DCMAKE_C_COMPILER='$(CC_BIN)' -DCMAKE_CXX_COMPILER='$(CXX_BIN)'

else
## default compiler
COMPILER_ARGS :=
endif

###############################################################################
## @section Build-directory and configure-argument helpers
##
## Helpers for turning a validated suffix into a concrete CMake configure
## invocation.
###############################################################################

## @name Build-directory and configure helpers
## @{

## Function: $(call build_dir,suffix_or_symbol_with_suffix)
## @fn build_dir(suffix_or_symbol_with_suffix)
## @brief Return the out-of-tree build directory for the suffix.
## @param 1 Candidate string containing a suffix.
build_dir = build/$(GEN_TAG)_$(COMPILER_TAG)_$(call valid_suffix,$1)

## Function: $(call cmake_args_for_suffix,suffix_or_symbol_with_suffix)
## @fn cmake_args_for_suffix(suffix_or_symbol_with_suffix)
## @brief Return the full CMake configure argument set for the suffix.
##
## Most suffix-specific behavior is derived directly from the suffix grammar.
##
## @param 1 Candidate string containing a suffix.
cmake_args_for_suffix = \
	-S . \
	-B "$(call build_dir,$1)" \
	-G "$(GEN)" \
	$(COMPILER_ARGS) \
	$(CMAKE_ARGS) \
	-DCMAKE_BUILD_TYPE=$(call profile_build_type,$1) \
	-DUNIVERSAL_ENABLE_TESTS=ON \
	-DUNIVERSAL_BUILD_ALL=$(call predicate_on_off,is_all,$1) \
	-DUNIVERSAL_ENABLE_ASAN=$(call predicate_on_off,is_sanitize,$1) \
	-DUNIVERSAL_ENABLE_UBSAN=$(call predicate_on_off,is_sanitize,$1) \
	-DUNIVERSAL_ENABLE_COVERAGE=$(call predicate_on_off,is_coverage,$1)

## @}

###############################################################################
## @section Default target
###############################################################################

default: all

###############################################################################
## @section Concrete .PHONY target catalog
###############################################################################

.PHONY: default all build test sanitize coverage clean help more_help

###############################################################################
## @section Validation targets
###############################################################################

## @brief Validate that required tool variable $* is non-empty.
##
## The tool paths are resolved earlier using find_tool. At this point the only
## thing we need to know is whether the variable resolved successfully.
tool__%:
	$(if $(call find_tool,$($*)),@#,\
		$(error Required tool '$*' is missing))

## @brief Validate that stage suffix $* is exactly one legal suffix.
validate_suffix__%:
	$(if $(call valid_suffix,$*),@#,\
		$(error Invalid build suffix '$*'))

## @brief Validate that stage suffix $* supports coverage.
fail_unless_cov__%: validate_suffix__%
	$(if $(call is_coverage,$*),@#,\
		$(error Coverage requires a *_cov suffix, not '$*'))

###############################################################################
## @section Stage rules
###############################################################################

## @brief Configure the build tree for suffix $*.
##
## If GEN is set to a Ninja generator while ninja is unavailable, CMake will
## report that error during configure. This Makefile does not duplicate that
## generator-specific tool check.
configure__%: validate_suffix__% tool__CMAKE $(if $(IS_NINJA),tool__NINJA,)
	$(CMAKE) $(call cmake_args_for_suffix,$*)

## @brief Build the configured tree for suffix $*.
build__%: validate_suffix__% tool__CMAKE $(if $(COMPILER_ARGS),tool__CC_BIN tool__CXX_BIN,) configure__%
	$(CMAKE) --build "$(call build_dir,$*)" \
		--config "$(call profile_build_type,$*)" \
		--parallel "$(JOBS)"

## @brief Run ctest for suffix $*.
##
## CMake owns test execution through its `test` target.
test__%: validate_suffix__% tool__CTEST configure__%
	$(CMAKE) --build "$(call build_dir,$*)" \
		--config "$(call profile_build_type,$*)" \
		--target test

## @brief Run CMake's coverage pipeline for suffix $*.
##
## CMake already wires its "coverage" target to depend on its preceding
## "check" step, so this Makefile does not duplicate that pipeline here.
coverage__%: validate_suffix__% fail_unless_cov__% tool__CMAKE tool__CTEST configure__%
	$(CMAKE) --build "$(call build_dir,$*)" \
		--config "$(call profile_build_type,$*)" \
		--target coverage

###############################################################################
## @section Public targets
###############################################################################

## @brief Configure and build core production assets (prod_core).
build: build__prod_core

## @brief Configure, build, and run all tests (debug_all).
test: test__debug_all

## @brief Configure, build, and run all tests with ASan and UBSan enabled.
sanitize: test__debug_all_san

## @brief Configure, build, and run all tests with code coverage tracking
##        enabled to generate a code coverage report.
coverage: coverage__prod_all_cov

## @brief Default workflow: run all tests (debug_all) and then build core
##        production assets (prod_core).
all: test build

## @brief Remove wrapper-managed build trees.
clean:
	rm -rf build

###############################################################################
## @section Help
###############################################################################

help:
	@echo "Common targets:"
	@echo "  build      Configure and build core production assets"
	@echo "  test       Configure, build, and run all tests"
	@echo "  sanitize   Configure, build, and run all tests with ASan and UBSan enabled"
	@echo "  coverage   Configure, build, and run all tests with code coverage tracking"
	@echo "                enabled to generate a code coverage report."
	@echo "  clean      Remove build trees"
	@echo "  help       Show this summary"
	@echo "  more_help  Show advanced usage, suffixes, and override variables"

more_help:
	@echo "Default workflows:"
	@echo "  build      -> build__prod_core"
	@echo "  test       -> test__debug_all"
	@echo "  sanitize   -> test__debug_all_san"
	@echo "  coverage   -> coverage__prod_all_cov"
	@echo "  all        -> test + build"
	@echo ""
	@echo "Supported suffixes (16 total):"
	@echo "  $(ALL_BUILD_SUFFIXES)"
	@echo ""
	@echo "Variables:"
	@echo "  GEN=<generator>"
	@echo "  COMPILER=<default|clang|gcc>"
	@echo "  JOBS=<n>"
	@echo "  CTEST_JOBS=<n>"
	@echo "  CMAKE_ARGS='...'"
	@echo "  CMAKE=<path>"
	@echo "  CTEST=<path>"
	@echo "  NINJA=<path>"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make build"
	@echo "  make test"
	@echo "  make sanitize"
	@echo "  make coverage"
	@echo "  make test__prod_all_cov_san"
