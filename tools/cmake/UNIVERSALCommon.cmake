############################
#This configuration file defines some cmake variables:
# UNIVERSAL_INCLUDE_DIRS: list of include directories for the universal library
# UNIVERSAL_LIBRARIES: libraries needed for interfaces like umfpack and arprec, see below
# UNIVERSAL_CXX_DEFINITIONS: definitions to enable the requested interfaces
# UNIVERSAL_VERSION: version (current: 1)
# UNIVERSAL_MINOR_VERSION: minor version 
#
#supported components:
#

unset(UNIVERSAL_LIBRARIES )
unset(UNIVERSAL_CXX_DEFINITIONS )
unset(UNIVERSAL_INCLUDE_DIRS )

if (MSVC)
    add_definitions(/wd4522) # multiple assignment ops for single type, to be investigated further if avoidable
endif()

if (USE_ASSERTS)
  list(APPEND UNIVERSAL_CXX_DEFINITIONS "-DUNIVERSAL_ASSERT_FOR_THROW")
endif()

# quick but weak check to see if we have a real universal library we are pointing to
if(EXISTS ${UNIVERSAL_DIR}/include/universal/number/posit/posit.hpp)
  list(APPEND UNIVERSAL_INCLUDE_DIRS "${UNIVERSAL_DIR}/include")
else()
  message(STATUS "Couldn't find the posit include directory at ${UNIVERSAL_DIR}/include")
endif(EXISTS ${UNIVERSAL_DIR}/include/universal/number/posit/posit.hpp)

macro(unum_check_cxx_compiler_flag FLAG RESULT)
  # counts entirely on compiler's return code, maybe better to combine it with check_cxx_compiler_flag
  file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx" "int main() { return 0;}\n")
  try_compile(${RESULT}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx
    COMPILE_DEFINITIONS ${FLAG})  
endmacro()

