# integration: tests of the library as an application uses it, across static and elastic
# number systems

# multifile: several number systems linked into one program, so that a header-defined
# function without `inline` shows up as a duplicate symbol (#1424)
file (GLOB MULTIFILE_SOURCES "./multifile/*.cpp")
compile_multifile_target("true" "multifile" "Integration/Multifile" ${MULTIFILE_SOURCES})

# noexceptions: every number system built with exceptions disabled, the way firmware and
# other bare-metal targets build (#1925). MSVC has no equivalent of -fno-exceptions that
# its standard library supports, so the test is for gcc and clang.
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang") AND NOT MSVC)
	file (GLOB NOEXCEPTIONS_SOURCES "./noexceptions/*.cpp")
	compile_multifile_target("true" "noexceptions" "Integration/NoExceptions" ${NOEXCEPTIONS_SOURCES})
	target_compile_options(noexceptions PRIVATE -fno-exceptions -Werror)
else()
	message(STATUS "noexceptions: skipped, it needs gcc or clang for -fno-exceptions")
endif()
