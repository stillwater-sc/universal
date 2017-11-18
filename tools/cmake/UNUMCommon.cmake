############################
#This configuration file defines some cmake variables:
#UNUM_INCLUDE_DIRS: list of include directories for the universal library
#UNUM_LIBRARIES: libraries needed for interfaces like umfpack and arprec, see below
#UNUM_CXX_DEFINITIONS: definitions to enable the requested interfaces
#UNUM_VERSION: version (current: 1)
#UNUM_MINOR_VERSION: minor version 
#
#supported components:
#

unset(UNUM_LIBRARIES )
unset(UNUM_CXX_DEFINITIONS )
unset(UNUM_INCLUDE_DIRS )

if (MSVC)
    add_definitions(/wd4522) # multiple assignment ops for single type, to be investigated further if avoidable
endif()

if (USE_ASSERTS)
  list(APPEND UNUM_CXX_DEFINITIONS "-DUNUM_ASSERT_FOR_THROW")
endif()

if(EXISTS ${UNUM_DIR}/posit/posit.hpp)
	list(APPEND UNUM_INCLUDE_DIRS "${UNUM_DIR}/posit")
else()
	list(APPEND UNUM_INCLUDE_DIRS "${UNUM_DIR}/../../include")
endif(EXISTS ${UNUM_DIR}/posit/posit.hpp)

macro(unum_check_cxx_compiler_flag FLAG RESULT)
  # counts entirely on compiler's return code, maybe better to combine it with check_cxx_compiler_flag
  file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx" "int main() { return 0;}\n")
  try_compile(${RESULT}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx
    COMPILE_DEFINITIONS ${FLAG})  
endmacro()

