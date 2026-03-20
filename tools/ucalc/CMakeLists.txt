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

add_test(ucalc_smoke ${RUNTIME_OUTPUT_DIRECTORY}/ucalc "type posit32; 1/3 + 1/3 + 1/3")

# Regression test: validates expression evaluation, type dispatch, precision rendering
add_executable(ucalc_regression regression.cpp)
set_target_properties(ucalc_regression PROPERTIES FOLDER "Tools/ucalc")
add_test(ucalc_regression ${RUNTIME_OUTPUT_DIRECTORY}/ucalc_regression)
