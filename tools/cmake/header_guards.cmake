################################################################################################
# Header compile guards: C++ macros that Universal's headers test, typically
#   #ifndef UNIVERSAL_X
#   #define UNIVERSAL_X <default>
#   #endif
# Each one is declared here as a CMake cache variable. Setting it adds a DIRECTORY-SCOPE compile
# definition for Universal's own build: its tests, tools and applications.
#
# Directory scope is deliberate. It is never written into the installed UNIVERSALTargets.cmake,
# so a project that consumes Universal through find_package(UNIVERSAL) makes its own choice by
# defining the macro itself; an installer's choice does not leak into it. The library is
# header-only and these are compile-time choices of the code that includes the headers.
#
# Declaring a guard here also registers it with the configuration summary
# (tools/cmake/summary.cmake), so a new guard cannot be added without being reported.
#
# Usage, from the top-level CMakeLists.txt before any add_subdirectory():
#   universal_header_guard_tristate(NAME "<doc>" "<summary default text>")
#       AUTO (the header decides) | ON | OFF  ->  NAME=1 | NAME=0
#   universal_header_guard_choice(NAME "<doc>" "<summary default text>" LABEL=VALUE ...)
#       DEFAULT (the header decides) | LABEL | VALUE  ->  NAME=VALUE
#   universal_header_guard_flag(NAME "<doc>" "<summary default text>")
#       OFF | ON  ->  NAME defined

# record a guard for the summary: what to print when the header default applies, and what to
# print when the build has set it (empty when it has not)
function(_universal_register_header_guard name default_text set_text)
  set_property(GLOBAL APPEND PROPERTY UNIVERSAL_HEADER_GUARDS ${name})
  set_property(GLOBAL PROPERTY UNIVERSAL_HEADER_GUARD_DEFAULT_${name} "${default_text}")
  set_property(GLOBAL PROPERTY UNIVERSAL_HEADER_GUARD_SET_${name} "${set_text}")
endfunction()

# the same macro given both as a cache variable and by hand (CMAKE_CXX_FLAGS,
# CMAKE_CXX_FLAGS_<CONFIG>, or a parent project's compile definitions or options, including
# a NAME=value inside a generator expression such as $<$<CONFIG:Debug>:NAME=0> or
# $<IF:$<CONFIG:Debug>,NAME=0,NAME=1>) would be defined twice,
# with the command-line order deciding which wins: refuse it.
# Usage:
#   _universal_header_guard_single_source(NAME ["<what sets it>"])   default: NAME=<cache value>
function(_universal_header_guard_single_source name)
  if(ARGC GREATER 1)
    set(source "${ARGV1}")
  else()
    set(source "a CMake cache variable (${name}=${${name}})")
  endif()
  get_property(multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(multi)
    set(configs ${CMAKE_CONFIGURATION_TYPES})
  else()
    set(configs "${CMAKE_BUILD_TYPE}")
  endif()
  set(haystack "${CMAKE_CXX_FLAGS}")
  foreach(c ${configs})
    string(TOUPPER "${c}" cu)
    string(APPEND haystack " ${CMAKE_CXX_FLAGS_${cu}}")
  endforeach()
  get_directory_property(defs COMPILE_DEFINITIONS)
  get_directory_property(opts COMPILE_OPTIONS)
  string(APPEND haystack " ${defs} ${opts}")
  # A plain token, -DNAME, NAME or NAME=value; or, inside a generator expression (after ':'
  # or ',', as in $<IF:cond,NAME=0,NAME=1>), only NAME=value. A bare name there can be an
  # operand rather than a definition ($<TARGET_PROPERTY:tgt,NAME>), and a bare definition can
  # only duplicate a flag guard's own, which is harmless.
  if(haystack MATCHES "(^|[ ;])(-D|/D)?${name}([= ;]|$)" OR haystack MATCHES "[:,](-D|/D)?${name}=")
    message(FATAL_ERROR "${name} is set both by ${source} "
                        "and by hand (CMAKE_CXX_FLAGS, CMAKE_CXX_FLAGS_<CONFIG>, or a compile "
                        "definition or option); set it one way only.")
  endif()
endfunction()

function(universal_header_guard_tristate name doc default_text)
  set(${name} AUTO CACHE STRING "${doc}")
  set_property(CACHE ${name} PROPERTY STRINGS AUTO ON OFF)
  string(TOUPPER "${${name}}" value)
  if(value STREQUAL "AUTO")
    _universal_register_header_guard(${name} "${default_text}" "")
    return()
  endif()
  if(value MATCHES "^(ON|TRUE|YES|Y|1)$")
    set(macro_value 1)
  elseif(value MATCHES "^(OFF|FALSE|NO|N|0)$")
    set(macro_value 0)
  else()
    message(FATAL_ERROR "${name}=${${name}} is not one of: AUTO, ON, OFF")
  endif()
  _universal_header_guard_single_source(${name})
  add_compile_definitions(${name}=${macro_value})
  _universal_register_header_guard(${name} "${default_text}" "${macro_value} (${name}=${${name}})")
endfunction()

function(universal_header_guard_choice name doc default_text)
  set(labels DEFAULT)
  foreach(pair ${ARGN})
    string(REPLACE "=" ";" kv "${pair}")
    list(GET kv 0 label)
    list(APPEND labels ${label})
  endforeach()
  set(${name} DEFAULT CACHE STRING "${doc}")
  set_property(CACHE ${name} PROPERTY STRINGS ${labels})
  set(value "${${name}}")
  if(value STREQUAL "DEFAULT")
    _universal_register_header_guard(${name} "${default_text}" "")
    return()
  endif()
  # accept the label (NEWTON_DIVISION) or the macro value it stands for (0)
  set(macro_value "")
  foreach(pair ${ARGN})
    string(REPLACE "=" ";" kv "${pair}")
    list(GET kv 0 label)
    list(GET kv 1 number)
    if(value STREQUAL label OR value STREQUAL number)
      set(macro_value ${number})
      set(macro_label ${label})
    endif()
  endforeach()
  if(macro_value STREQUAL "")
    string(REPLACE ";" ", " allowed "${labels}")
    message(FATAL_ERROR "${name}=${value} is not one of: ${allowed}")
  endif()
  _universal_header_guard_single_source(${name})
  add_compile_definitions(${name}=${macro_value})
  _universal_register_header_guard(${name} "${default_text}" "${macro_value}, ${macro_label} (${name}=${value})")
endfunction()

function(universal_header_guard_flag name doc default_text)
  option(${name} "${doc}" OFF)
  if(NOT ${name})
    _universal_register_header_guard(${name} "${default_text}" "")
    return()
  endif()
  _universal_header_guard_single_source(${name})
  add_compile_definitions(${name})
  _universal_register_header_guard(${name} "${default_text}" "defined (${name}=ON)")
endfunction()

################################################################################################
# The guards

# utility/directives.hpp: is std::fma correctly rounded? AUTO lets the header detect it
# (0 for mingw-w64 without -mfma, #1578); OFF forces Dekker's product everywhere.
universal_header_guard_tristate(UNIVERSAL_TRUSTED_FMA
  "Is std::fma correctly rounded: AUTO (header detects), ON, OFF (use Dekker's product)"
  "header detection")

# {dd,td,qd}_cascade/math/functions/sqrt.hpp: the square-root algorithm (#1333)
universal_header_guard_choice(UNIVERSAL_DD_CASCADE_SQRT_ALGORITHM
  "dd_cascade sqrt: DEFAULT, NEWTON_DIVISION, NEWTON_RECIPROCAL, KARP"
  "header default (0, NEWTON_DIVISION)"
  NEWTON_DIVISION=0 NEWTON_RECIPROCAL=1 KARP=2)
universal_header_guard_choice(UNIVERSAL_TD_CASCADE_SQRT_ALGORITHM
  "td_cascade sqrt: DEFAULT, NEWTON_DIVISION, NEWTON_RECIPROCAL"
  "header default (1, NEWTON_RECIPROCAL)"
  NEWTON_DIVISION=0 NEWTON_RECIPROCAL=1)
universal_header_guard_choice(UNIVERSAL_QD_CASCADE_SQRT_ALGORITHM
  "qd_cascade sqrt: DEFAULT, NEWTON_DIVISION, NEWTON_RECIPROCAL"
  "header default (0, NEWTON_DIVISION)"
  NEWTON_DIVISION=0 NEWTON_RECIPROCAL=1)

# internal/limbs/limb_kernels.hpp: compile the portable limb loops alone, without the
# x86-64 mulx/adcx/adox inline-assembly kernels (#1818)
universal_header_guard_flag(UNIVERSAL_LIMB_KERNELS_NO_ASM
  "Compile the portable limb kernels only, without the x86-64 inline assembly"
  "not defined (x86-64 asm kernels where available)")

# mixedprecision/pop_solver.hpp: solve POP's LP with GLPK instead of the built-in simplex.
# Unlike the guards above this one needs a library, so it is an option of its own:
# UNIVERSAL_USE_GLPK finds GLPK, and mixedprecision/pop defines UNIVERSAL_HAS_GLPK and links
# it for the targets there, the only code that includes the GLPK solver.
option(UNIVERSAL_USE_GLPK "Build the POP precision tuner with the GLPK LP solver (defines UNIVERSAL_HAS_GLPK)" OFF)
if(UNIVERSAL_USE_GLPK)
  _universal_header_guard_single_source(UNIVERSAL_HAS_GLPK "UNIVERSAL_USE_GLPK=ON")
  find_path(GLPK_INCLUDE_DIR glpk.h)
  find_library(GLPK_LIBRARY NAMES glpk)
  if(NOT GLPK_INCLUDE_DIR OR NOT GLPK_LIBRARY)
    message(FATAL_ERROR "UNIVERSAL_USE_GLPK=ON but GLPK was not found "
                        "(glpk.h: ${GLPK_INCLUDE_DIR}, libglpk: ${GLPK_LIBRARY}). "
                        "Install it, or point GLPK_INCLUDE_DIR and GLPK_LIBRARY at it.")
  endif()
  _universal_register_header_guard(UNIVERSAL_HAS_GLPK "not defined (POP solver without GLPK)"
    "defined in mixedprecision/pop (UNIVERSAL_USE_GLPK=ON, ${GLPK_LIBRARY})")
else()
  _universal_register_header_guard(UNIVERSAL_HAS_GLPK "not defined (POP solver without GLPK)" "")
endif()
