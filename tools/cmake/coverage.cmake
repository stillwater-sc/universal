if(NOT DEFINED MODE OR NOT DEFINED BINARY_DIR OR NOT DEFINED CTEST_EXECUTABLE OR NOT DEFINED COVERAGE_BACKEND)
  message(FATAL_ERROR "MODE, BINARY_DIR, CTEST_EXECUTABLE, and COVERAGE_BACKEND are required.")
endif()

if(NOT DEFINED CONFIG)
  set(CONFIG "")
endif()

set(COVERAGE_HTML_DIR "${BINARY_DIR}/coverage-html")

execute_process(
  COMMAND "${CTEST_EXECUTABLE}" -N -C "${CONFIG}"
  WORKING_DIRECTORY "${BINARY_DIR}"
  OUTPUT_VARIABLE _ctest_list
  RESULT_VARIABLE _ctest_list_result
)
if(NOT _ctest_list_result EQUAL 0)
  message(FATAL_ERROR "ctest -N failed in ${BINARY_DIR}.")
endif()
if(_ctest_list MATCHES "Total Tests: 0")
  message(FATAL_ERROR "No tests were found in ${BINARY_DIR}. Configure with test-bearing targets enabled.")
endif()

if(MODE STREQUAL "check")
  if(COVERAGE_BACKEND STREQUAL "llvm")
    file(GLOB_RECURSE _old_profraw "${BINARY_DIR}/*.profraw")
    if(_old_profraw)
      file(REMOVE ${_old_profraw})
    endif()
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E env "LLVM_PROFILE_FILE=universal-%p-%m.profraw"
              "${CTEST_EXECUTABLE}" --output-on-failure -C "${CONFIG}"
      WORKING_DIRECTORY "${BINARY_DIR}"
      RESULT_VARIABLE _ctest_result
    )
  else()
    execute_process(
      COMMAND "${CTEST_EXECUTABLE}" --output-on-failure -C "${CONFIG}"
      WORKING_DIRECTORY "${BINARY_DIR}"
      RESULT_VARIABLE _ctest_result
    )
  endif()
  if(NOT _ctest_result EQUAL 0)
    message(FATAL_ERROR "ctest failed during coverage run.")
  endif()
  return()
endif()

if(NOT MODE STREQUAL "report")
  message(FATAL_ERROR "Unknown coverage mode: ${MODE}")
endif()

if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR is required for coverage report generation.")
endif()

if(COVERAGE_BACKEND STREQUAL "lcov")
  # GCC/gcov coverage uses lcov + genhtml and keeps the existing HTML flow.
  if(NOT DEFINED LCOV_EXECUTABLE OR NOT DEFINED GENHTML_EXECUTABLE)
    message(FATAL_ERROR "lcov backend requires LCOV_EXECUTABLE and GENHTML_EXECUTABLE.")
  endif()

  file(REMOVE_RECURSE "${COVERAGE_HTML_DIR}")

  execute_process(
    COMMAND "${LCOV_EXECUTABLE}" --capture --directory "${BINARY_DIR}" --output-file "${BINARY_DIR}/coverage.info"
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _lcov_capture_result
  )
  if(NOT _lcov_capture_result EQUAL 0)
    message(FATAL_ERROR "lcov capture failed.")
  endif()

  execute_process(
    COMMAND "${LCOV_EXECUTABLE}" --remove "${BINARY_DIR}/coverage.info" "/usr/*" "${SOURCE_DIR}/third_party/*" "${BINARY_DIR}/*" --output-file "${BINARY_DIR}/coverage.filtered.info"
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _lcov_filter_result
  )
  if(NOT _lcov_filter_result EQUAL 0)
    message(FATAL_ERROR "lcov filtering failed.")
  endif()

  execute_process(
    COMMAND "${GENHTML_EXECUTABLE}" "${BINARY_DIR}/coverage.filtered.info" --output-directory "${COVERAGE_HTML_DIR}"
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _genhtml_result
  )
  if(NOT _genhtml_result EQUAL 0)
    message(FATAL_ERROR "genhtml failed.")
  endif()

  message(STATUS "Coverage report: ${COVERAGE_HTML_DIR}/index.html")
elseif(COVERAGE_BACKEND STREQUAL "llvm")
  # Clang/AppleClang coverage uses LLVM profile data plus llvm-cov HTML output.
  if(NOT DEFINED LLVM_PROFDATA_EXECUTABLE OR NOT DEFINED LLVM_COV_EXECUTABLE)
    message(FATAL_ERROR "llvm backend requires LLVM_PROFDATA_EXECUTABLE and LLVM_COV_EXECUTABLE.")
  endif()

  file(REMOVE_RECURSE "${COVERAGE_HTML_DIR}")
  file(GLOB_RECURSE _profraw_files "${BINARY_DIR}/*.profraw")
  if(NOT _profraw_files)
    message(FATAL_ERROR "No .profraw files were produced in ${BINARY_DIR}.")
  endif()

  execute_process(
    COMMAND "${LLVM_PROFDATA_EXECUTABLE}" merge -sparse ${_profraw_files} -o "${BINARY_DIR}/universal.profdata"
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _profdata_result
  )
  if(NOT _profdata_result EQUAL 0)
    message(FATAL_ERROR "llvm-profdata merge failed.")
  endif()

  execute_process(
    COMMAND "${CTEST_EXECUTABLE}" -N -V -C "${CONFIG}"
    WORKING_DIRECTORY "${BINARY_DIR}"
    OUTPUT_VARIABLE _ctest_verbose
    RESULT_VARIABLE _ctest_verbose_result
  )
  if(NOT _ctest_verbose_result EQUAL 0)
    message(FATAL_ERROR "ctest -N -V failed while collecting test executables.")
  endif()

  string(REPLACE "\r\n" "\n" _ctest_verbose "${_ctest_verbose}")
  string(REPLACE "\r" "\n" _ctest_verbose "${_ctest_verbose}")
  string(REPLACE "\n" ";" _ctest_lines "${_ctest_verbose}")

  set(_test_executables "")
  foreach(_line IN LISTS _ctest_lines)
    if(NOT _line MATCHES "Test command: ")
      continue()
    endif()
    string(REGEX REPLACE "^.*Test command: " "" _command "${_line}")
    if(WIN32)
      separate_arguments(_args WINDOWS_COMMAND "${_command}")
    else()
      separate_arguments(_args UNIX_COMMAND "${_command}")
    endif()

    set(_candidate "")
    foreach(_arg IN LISTS _args)
      if(_arg STREQUAL "-E" OR _arg STREQUAL "env")
        continue()
      endif()
      if(_arg MATCHES "^[A-Za-z_][A-Za-z0-9_]*=.*$")
        continue()
      endif()
      get_filename_component(_arg_name "${_arg}" NAME)
      if(_arg_name STREQUAL "cmake" OR _arg_name STREQUAL "cmake.exe")
        continue()
      endif()
      if(IS_ABSOLUTE "${_arg}" AND EXISTS "${_arg}")
        set(_candidate "${_arg}")
        break()
      endif()
      if(EXISTS "${BINARY_DIR}/${_arg}")
        get_filename_component(_candidate "${BINARY_DIR}/${_arg}" ABSOLUTE)
        break()
      endif()
    endforeach()

    if(_candidate)
      list(FIND _test_executables "${_candidate}" _candidate_index)
      if(_candidate_index EQUAL -1)
        list(APPEND _test_executables "${_candidate}")
      endif()
    endif()
  endforeach()

  if(NOT _test_executables)
    message(FATAL_ERROR "Could not determine test executables from ctest -N -V output.")
  endif()

  list(GET _test_executables 0 _primary_executable)
  list(REMOVE_AT _test_executables 0)

  set(_response_file "${BINARY_DIR}/test_executables.txt")
  file(WRITE "${_response_file}" "")
  foreach(_exe IN LISTS _test_executables)
    file(APPEND "${_response_file}" "-object\n\"${_exe}\"\n")
  endforeach()

  execute_process(
    COMMAND "${LLVM_COV_EXECUTABLE}" show "${_primary_executable}"
            "@${_response_file}"
            -instr-profile="${BINARY_DIR}/universal.profdata"
            -format=html
            -output-dir="${COVERAGE_HTML_DIR}"
            -show-line-counts-or-regions
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _llvm_cov_result
  )
  if(NOT _llvm_cov_result EQUAL 0)
    message(FATAL_ERROR "llvm-cov show failed.")
  endif()

  message(STATUS "Coverage report: ${COVERAGE_HTML_DIR}/index.html")
else()
  message(FATAL_ERROR "Unknown coverage backend: ${COVERAGE_BACKEND}")
endif()
