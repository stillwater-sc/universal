# Universal instrumentation options and flags

option(UNIVERSAL_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)
option(UNIVERSAL_INSTRUMENT_ALL_TARGETS "Instrument all targets, not just tests" OFF)

function(universal_add_instrumentation_flags)
  get_directory_property(_universal_instrumentation_applied UNIVERSAL_INSTRUMENTATION_APPLIED)
  if(_universal_instrumentation_applied)
    return()
  endif()
  set_property(DIRECTORY PROPERTY UNIVERSAL_INSTRUMENTATION_APPLIED TRUE)

  if(UNIVERSAL_ENABLE_COVERAGE)
    if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      message(WARNING
        "Coverage is best-effort for MSVC. The coverage-report pipeline in this project does not support MSVC. "
        "Use Visual Studio tooling or clang/gcc for HTML reports if possible.")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:--coverage>)
      add_link_options(--coverage)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:-fprofile-instr-generate;-fcoverage-mapping>)
      add_link_options(-fprofile-instr-generate)
    endif()
  endif()
endfunction()
