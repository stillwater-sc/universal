# ClangFormat.cmake
# Adds targets for code formatting with clang-format

# Find clang-format executable
find_program(CLANG_FORMAT_EXECUTABLE
    NAMES
        clang-format-18
        clang-format-17
        clang-format-16
        clang-format-15
        clang-format
    DOC "Path to clang-format executable"
)

if(CLANG_FORMAT_EXECUTABLE)
    # Get clang-format version
    execute_process(
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --version
        OUTPUT_VARIABLE CLANG_FORMAT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Found clang-format: ${CLANG_FORMAT_EXECUTABLE}")
    message(STATUS "  Version: ${CLANG_FORMAT_VERSION}")

    # Collect all source files
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/include/*.hpp
        ${CMAKE_SOURCE_DIR}/include/*.h
    )

    # Collect all test/application source files
    file(GLOB_RECURSE ALL_TEST_FILES
        ${CMAKE_SOURCE_DIR}/static/*.cpp
        ${CMAKE_SOURCE_DIR}/elastic/*.cpp
        ${CMAKE_SOURCE_DIR}/internal/*.cpp
        ${CMAKE_SOURCE_DIR}/numeric/*.cpp
        ${CMAKE_SOURCE_DIR}/linalg/*.cpp
        ${CMAKE_SOURCE_DIR}/mixedprecision/*.cpp
        ${CMAKE_SOURCE_DIR}/benchmark/*.cpp
        ${CMAKE_SOURCE_DIR}/applications/*.cpp
        ${CMAKE_SOURCE_DIR}/education/*.cpp
        ${CMAKE_SOURCE_DIR}/playground/*.cpp
        ${CMAKE_SOURCE_DIR}/tools/cmd/*.cpp
        ${CMAKE_SOURCE_DIR}/tools/utils/*.cpp
    )

    # Combine all files
    set(ALL_FORMAT_FILES ${ALL_SOURCE_FILES} ${ALL_TEST_FILES})

    # Remove files in build directories and matrix data files
    list(FILTER ALL_FORMAT_FILES EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")
    list(FILTER ALL_FORMAT_FILES EXCLUDE REGEX ".*/blas/matrices/.*")

    # Count files
    list(LENGTH ALL_FORMAT_FILES NUM_FORMAT_FILES)
    message(STATUS "  Files to format: ${NUM_FORMAT_FILES}")

    # Create separate file lists for each major directory
    set(HEADER_FILES ${ALL_SOURCE_FILES})
    set(TEST_FILES ${ALL_TEST_FILES})

    # Write file lists to separate files
    set(HEADERS_FILE_LIST "${CMAKE_BINARY_DIR}/clang-format-headers.txt")
    set(TESTS_FILE_LIST "${CMAKE_BINARY_DIR}/clang-format-tests.txt")
    set(ALL_FILES_LIST "${CMAKE_BINARY_DIR}/clang-format-all.txt")

    string(REPLACE ";" "\n" HEADERS_CONTENT "${HEADER_FILES}")
    string(REPLACE ";" "\n" TESTS_CONTENT "${TEST_FILES}")
    string(REPLACE ";" "\n" ALL_CONTENT "${ALL_FORMAT_FILES}")

    file(WRITE ${HEADERS_FILE_LIST} "${HEADERS_CONTENT}\n")
    file(WRITE ${TESTS_FILE_LIST} "${TESTS_CONTENT}\n")
    file(WRITE ${ALL_FILES_LIST} "${ALL_CONTENT}\n")

    list(LENGTH HEADER_FILES NUM_HEADERS)
    list(LENGTH TEST_FILES NUM_TESTS)

    # ===== Format Headers Only =====
    add_custom_target(format-headers
        COMMAND ${CMAKE_COMMAND} -E echo "Formatting ${NUM_HEADERS} header files..."
        COMMAND xargs -a ${HEADERS_FILE_LIST} ${CLANG_FORMAT_EXECUTABLE} -i -style=file
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting header files"
        VERBATIM
    )

    add_custom_target(format-headers-check
        COMMAND ${CMAKE_COMMAND} -E echo "Checking format of ${NUM_HEADERS} header files..."
        COMMAND xargs -a ${HEADERS_FILE_LIST} ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror -style=file 2>&1 | head -n 50
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking header file formatting (first 50 issues)"
        VERBATIM
    )

    # ===== Format Tests Only =====
    add_custom_target(format-tests
        COMMAND ${CMAKE_COMMAND} -E echo "Formatting ${NUM_TESTS} test/application files..."
        COMMAND xargs -a ${TESTS_FILE_LIST} ${CLANG_FORMAT_EXECUTABLE} -i -style=file
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting test files"
        VERBATIM
    )

    add_custom_target(format-tests-check
        COMMAND ${CMAKE_COMMAND} -E echo "Checking format of ${NUM_TESTS} test/application files..."
        COMMAND xargs -a ${TESTS_FILE_LIST} ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror -style=file 2>&1 | head -n 50
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking test file formatting (first 50 issues)"
        VERBATIM
    )

    # ===== Format All Files =====
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo "Formatting ${NUM_FORMAT_FILES} C++ source files..."
        COMMAND xargs -a ${ALL_FILES_LIST} ${CLANG_FORMAT_EXECUTABLE} -i -style=file
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting all C++ source files"
        VERBATIM
    )

    add_custom_target(format-check
        COMMAND ${CMAKE_COMMAND} -E echo "Checking format of ${NUM_FORMAT_FILES} C++ source files..."
        COMMAND xargs -a ${ALL_FILES_LIST} ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror -style=file 2>&1 | head -n 100
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking all file formatting (first 100 issues)"
        VERBATIM
    )

    add_custom_target(format-diff
        COMMAND ${CMAKE_COMMAND} -E echo "Files that would be reformatted (first 50):"
        COMMAND xargs -a ${ALL_FILES_LIST} ${CLANG_FORMAT_EXECUTABLE} --dry-run -style=file 2>&1 | head -n 50
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Showing formatting differences"
        VERBATIM
    )

else()
    message(STATUS "clang-format not found - formatting targets will not be available")
    message(STATUS "  Install clang-format to enable: sudo apt-get install clang-format")
endif()
