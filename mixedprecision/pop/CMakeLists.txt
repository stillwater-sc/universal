file (GLOB SOURCES "./*.cpp")

# UNIVERSAL_USE_GLPK (tools/cmake/header_guards.cmake) found GLPK: these are the only targets
# that include the GLPK solver, so the definition and the library are scoped to them
if(UNIVERSAL_USE_GLPK)
	add_compile_definitions(UNIVERSAL_HAS_GLPK)
	include_directories(${GLPK_INCLUDE_DIR})
	link_libraries(${GLPK_LIBRARY})
endif()

compile_all("true" "pop" "Mixed-Precision/POP" "${SOURCES}")
