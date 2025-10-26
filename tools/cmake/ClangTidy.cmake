# ClangTidy.cmake
# Adds targets for static analysis with clang-tidy

# Find clang-tidy executable
find_program(CLANG_TIDY_EXECUTABLE
    NAMES
        clang-tidy-18
        clang-tidy-17
        clang-tidy-16
        clang-tidy-15
        clang-tidy
    DOC "Path to clang-tidy executable"
)

if(CLANG_TIDY_EXECUTABLE)
    # Get clang-tidy version
    execute_process(
        COMMAND ${CLANG_TIDY_EXECUTABLE} --version
        OUTPUT_VARIABLE CLANG_TIDY_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # Sanitize version string for use in Makefiles (remove newlines)
    string(REPLACE "\n" " " CLANG_TIDY_VERSION_SINGLE "${CLANG_TIDY_VERSION}")
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXECUTABLE}")
    message(STATUS "  Version: ${CLANG_TIDY_VERSION}")

    # Check if compile_commands.json exists
    set(COMPILE_COMMANDS_FILE "${CMAKE_BINARY_DIR}/compile_commands.json")

    # Collect all header files (main analysis target)
    file(GLOB_RECURSE ALL_HEADER_FILES
        ${CMAKE_SOURCE_DIR}/include/*.hpp
        ${CMAKE_SOURCE_DIR}/include/*.h
    )

    # Collect sample test files for analysis (too many to analyze all at once)
    file(GLOB_RECURSE SAMPLE_TEST_FILES
        ${CMAKE_SOURCE_DIR}/static/*/api/*.cpp
        ${CMAKE_SOURCE_DIR}/elastic/*/api/*.cpp
    )

    # Filter out build directories
    list(FILTER ALL_HEADER_FILES EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")
    list(FILTER SAMPLE_TEST_FILES EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")

    # Count files
    list(LENGTH ALL_HEADER_FILES NUM_HEADER_FILES)
    list(LENGTH SAMPLE_TEST_FILES NUM_SAMPLE_FILES)
    message(STATUS "  Header files for analysis: ${NUM_HEADER_FILES}")
    message(STATUS "  Sample test files for analysis: ${NUM_SAMPLE_FILES}")

    # Write file lists
    set(HEADERS_FILE_LIST "${CMAKE_BINARY_DIR}/clang-tidy-headers.txt")
    set(SAMPLES_FILE_LIST "${CMAKE_BINARY_DIR}/clang-tidy-samples.txt")

    string(REPLACE ";" "\n" HEADERS_CONTENT "${ALL_HEADER_FILES}")
    string(REPLACE ";" "\n" SAMPLES_CONTENT "${SAMPLE_TEST_FILES}")

    file(WRITE ${HEADERS_FILE_LIST} "${HEADERS_CONTENT}\n")
    file(WRITE ${SAMPLES_FILE_LIST} "${SAMPLES_CONTENT}\n")

    # Helper script for running clang-tidy with compile_commands.json
    set(RUN_CLANG_TIDY_SCRIPT "${CMAKE_BINARY_DIR}/run-clang-tidy.sh")
    file(WRITE ${RUN_CLANG_TIDY_SCRIPT}
"#!/bin/bash
# Auto-generated script to run clang-tidy on Universal library
# Usage: ./run-clang-tidy.sh [options]
#   --fix           Apply suggested fixes
#   --headers-only  Only analyze headers
#   --samples       Analyze sample test files
#   --files FILE    Analyze specific files (one per line in FILE)
#   --checks CHECKS Override checks (e.g., 'bugprone-*,performance-*')

set -e

SCRIPT_DIR=\"\$( cd \"\$( dirname \"\${BASH_SOURCE[0]}\" )\" && pwd )\"
SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"
CLANG_TIDY=\"${CLANG_TIDY_EXECUTABLE}\"

FIX_FLAG=\"\"
FILE_LIST=\"${HEADERS_FILE_LIST}\"
CHECKS=\"\"
PARALLEL_JOBS=\$(nproc)

while [[ \$# -gt 0 ]]; do
    case \$1 in
        --fix)
            FIX_FLAG=\"--fix\"
            shift
            ;;
        --headers-only)
            FILE_LIST=\"${HEADERS_FILE_LIST}\"
            shift
            ;;
        --samples)
            FILE_LIST=\"${SAMPLES_FILE_LIST}\"
            shift
            ;;
        --files)
            FILE_LIST=\"\$2\"
            shift 2
            ;;
        --checks)
            CHECKS=\"--checks=\$2\"
            shift 2
            ;;
        --jobs|-j)
            PARALLEL_JOBS=\"\$2\"
            shift 2
            ;;
        *)
            echo \"Unknown option: \$1\"
            exit 1
            ;;
    esac
done

if [ ! -f \"${COMPILE_COMMANDS_FILE}\" ]; then
    echo \"Error: compile_commands.json not found at ${COMPILE_COMMANDS_FILE}\"
    echo \"Please build the project first with: mkdir build && cd build && cmake .. && make\"
    exit 1
fi

echo \"Running clang-tidy analysis...\"
echo \"  File list: \$FILE_LIST\"
echo \"  Parallel jobs: \$PARALLEL_JOBS\"
echo \"  Fix mode: \${FIX_FLAG:-disabled}\"
[ -n \"\$CHECKS\" ] && echo \"  Custom checks: \$CHECKS\"

# Function to run clang-tidy on a single file
run_tidy() {
    local file=\"\$1\"
    \"\$CLANG_TIDY\" \$FIX_FLAG \$CHECKS -p=\"${CMAKE_BINARY_DIR}\" \"\$file\" 2>&1 | head -n 100
}

export -f run_tidy
export CLANG_TIDY FIX_FLAG CHECKS

# Run in parallel using xargs
cat \"\$FILE_LIST\" | xargs -I {} -P \$PARALLEL_JOBS bash -c 'run_tidy \"\$@\"' _ {}

echo \"Analysis complete!\"
"
    )

    # Make script executable
    file(CHMOD ${RUN_CLANG_TIDY_SCRIPT}
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )

    # ===== Target: tidy-check =====
    # Quick check on headers (read-only, first 100 issues)
    add_custom_target(tidy-check
        COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy check on ${NUM_HEADER_FILES} header files (first 100 issues)..."
        COMMAND bash ${RUN_CLANG_TIDY_SCRIPT} --headers-only --jobs 1 | head -n 100
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy analysis on headers"
        VERBATIM
    )

    # ===== Target: tidy-check-samples =====
    # Check sample test files
    add_custom_target(tidy-check-samples
        COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy check on ${NUM_SAMPLE_FILES} sample files..."
        COMMAND bash ${RUN_CLANG_TIDY_SCRIPT} --samples
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy analysis on sample test files"
        VERBATIM
    )

    # ===== Target: tidy-fix =====
    # Apply fixes to headers
    add_custom_target(tidy-fix
        COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy with auto-fix on ${NUM_HEADER_FILES} header files..."
        COMMAND bash ${RUN_CLANG_TIDY_SCRIPT} --headers-only --fix
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Applying clang-tidy fixes to headers"
        VERBATIM
    )

    # ===== Target: tidy-info =====
    # Show information about configuration
    add_custom_target(tidy-info
        COMMAND ${CMAKE_COMMAND} -E echo "==== Clang-Tidy Configuration ===="
        COMMAND ${CMAKE_COMMAND} -E echo "Executable: ${CLANG_TIDY_EXECUTABLE}"
        COMMAND ${CMAKE_COMMAND} -E echo "Version: ${CLANG_TIDY_VERSION_SINGLE}"
        COMMAND ${CMAKE_COMMAND} -E echo "Compile commands: ${COMPILE_COMMANDS_FILE}"
        COMMAND ${CMAKE_COMMAND} -E echo "Header files: ${NUM_HEADER_FILES}"
        COMMAND ${CMAKE_COMMAND} -E echo "Sample test files: ${NUM_SAMPLE_FILES}"
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Usage:"
        COMMAND ${CMAKE_COMMAND} -E echo "  make tidy-check         - Check headers (first 100 issues)"
        COMMAND ${CMAKE_COMMAND} -E echo "  make tidy-check-samples - Check sample test files"
        COMMAND ${CMAKE_COMMAND} -E echo "  make tidy-fix           - Apply fixes to headers"
        COMMAND ${CMAKE_COMMAND} -E echo "  make tidy-info          - Show this information"
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Or use the script directly:"
        COMMAND ${CMAKE_COMMAND} -E echo "  ${RUN_CLANG_TIDY_SCRIPT}"
        COMMENT "Displaying clang-tidy information"
        VERBATIM
    )

    message(STATUS "")
    message(STATUS "Clang-tidy targets configured:")
    message(STATUS "  make tidy-check         - Run clang-tidy on headers (read-only)")
    message(STATUS "  make tidy-check-samples - Run clang-tidy on sample test files")
    message(STATUS "  make tidy-fix           - Apply clang-tidy fixes to headers")
    message(STATUS "  make tidy-info          - Show detailed configuration")
    message(STATUS "  Script: ${RUN_CLANG_TIDY_SCRIPT}")

else()
    message(STATUS "clang-tidy not found - static analysis targets will not be available")
    message(STATUS "  Install clang-tidy to enable: sudo apt-get install clang-tidy")
endif()
