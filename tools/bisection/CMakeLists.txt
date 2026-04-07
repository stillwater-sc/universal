add_executable(bisection bisection.cpp)
set_target_properties(bisection PROPERTIES FOLDER "Tools/bisection")

# Reuse type-erased dispatch / expression / output_format headers from
# tools/ucalc without rebuilding any ucalc source. We only need the
# include search path -- everything we touch is header-only.
target_include_directories(bisection PRIVATE
    ${CMAKE_SOURCE_DIR}/tools/ucalc)

install(TARGETS bisection DESTINATION bin)

if(UNIVERSAL_CMAKE_TRACE)
    message(STATUS "bisection: standalone REPL for the bisection coding framework")
endif()

# Smoke tests
add_test(NAME bisection_smoke_repl
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/bisection "type bisection_posit16; show pi")
add_test(NAME bisection_smoke_compare
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/bisection "compare 1/3")
add_test(NAME bisection_smoke_range
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/bisection "type bisection_posit16; range")
add_test(NAME bisection_smoke_types
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/bisection --types)
