add_executable(ucalc ucalc.cpp)
set_target_properties(ucalc PROPERTIES FOLDER "Tools/ucalc")

# Optional readline support for line editing, tab completion, and history
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(READLINE QUIET readline)
endif()
if(NOT READLINE_FOUND)
    find_library(READLINE_LIB readline)
    find_path(READLINE_INCLUDE readline/readline.h)
    if(READLINE_LIB AND READLINE_INCLUDE)
        set(READLINE_FOUND TRUE)
        set(READLINE_LIBRARIES ${READLINE_LIB})
        set(READLINE_INCLUDE_DIRS ${READLINE_INCLUDE})
    endif()
endif()

if(READLINE_FOUND)
    target_compile_definitions(ucalc PRIVATE UCALC_USE_READLINE)
    target_include_directories(ucalc PRIVATE ${READLINE_INCLUDE_DIRS})
    target_link_libraries(ucalc PRIVATE ${READLINE_LIBRARIES})
    message(STATUS "ucalc: readline support enabled")
else()
    message(STATUS "ucalc: readline not found, using basic line input")
endif()

install(TARGETS ucalc DESTINATION bin)

if(UNIVERSAL_CMAKE_TRACE)
    message(STATUS "ucalc: Universal Mixed-Precision REPL Calculator")
endif()

# Smoke test: basic expression evaluation
add_test(ucalc_smoke ${RUNTIME_OUTPUT_DIRECTORY}/ucalc "type posit32; 1/3 + 1/3 + 1/3")

# Regression test: validates expression evaluation, type dispatch, precision rendering
add_executable(ucalc_regression regression.cpp)
set_target_properties(ucalc_regression PROPERTIES FOLDER "Tools/ucalc")
add_test(ucalc_regression ${RUNTIME_OUTPUT_DIRECTORY}/ucalc_regression)

# Tier 1.4: CLI flag tests
add_test(NAME ucalc_flag_type
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/ucalc -t posit32 "1/3 + 1/3 + 1/3")
add_test(NAME ucalc_flag_quiet
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/ucalc --quiet -t posit32 "sin(0.1)")
add_test(NAME ucalc_flag_json
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/ucalc --json "show 3.14")
add_test(NAME ucalc_flag_csv
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/ucalc --csv "types")
add_test(NAME ucalc_flag_help
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/ucalc --help)
add_test(NAME ucalc_flag_version
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/ucalc --version)

# Tier 1.4: Batch file mode tests (run each example script)
set(UCALC_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
file(GLOB UCALC_SCRIPTS "${UCALC_SCRIPTS_DIR}/*.ucalc")
foreach(script ${UCALC_SCRIPTS})
    get_filename_component(script_name ${script} NAME_WE)
    add_test(NAME ucalc_script_${script_name}
        COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/ucalc -f ${script})
endforeach()

# Tier 1.4: Exit code tests
add_test(NAME ucalc_exit_unknown_type
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/ucalc -t bogus_type "1+1")
set_tests_properties(ucalc_exit_unknown_type PROPERTIES WILL_FAIL TRUE)
