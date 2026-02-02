if(NOT DEFINED BINARY_DIR)
  message(FATAL_ERROR "BINARY_DIR is required. Invoke with -DBINARY_DIR=<path>.")
endif()

get_filename_component(BINARY_DIR "${BINARY_DIR}" ABSOLUTE)
set(cache_file "${BINARY_DIR}/CMakeCache.txt")
if(NOT EXISTS "${cache_file}")
  message(FATAL_ERROR "CMakeCache.txt not found in ${BINARY_DIR}. Configure the build first.")
endif()

file(READ "${cache_file}" _cache_contents)

set(_compiler_id "")
string(REGEX MATCH "CMAKE_CXX_COMPILER_ID:STRING=([^\n\r]+)" _match "${_cache_contents}")
if(_match)
  set(_compiler_id "${CMAKE_MATCH_1}")
endif()

set(_compiler_path "")
string(REGEX MATCH "CMAKE_CXX_COMPILER:FILEPATH=([^\n\r]+)" _match "${_cache_contents}")
if(_match)
  set(_compiler_path "${CMAKE_MATCH_1}")
endif()

set(_source_dir "")
string(REGEX MATCH "CMAKE_HOME_DIRECTORY:INTERNAL=([^\n\r]+)" _match "${_cache_contents}")
if(_match)
  set(_source_dir "${CMAKE_MATCH_1}")
endif()

set(_generator "")
string(REGEX MATCH "CMAKE_GENERATOR:INTERNAL=([^\n\r]+)" _match "${_cache_contents}")
if(_match)
  set(_generator "${CMAKE_MATCH_1}")
endif()

set(_is_vs_generator FALSE)
if(_generator MATCHES "^Visual Studio")
  set(_is_vs_generator TRUE)
endif()

if(_compiler_id STREQUAL "" AND NOT _compiler_path STREQUAL "")
  string(TOLOWER "${_compiler_path}" _compiler_path_lc)
  if(_compiler_path_lc MATCHES "clang")
    set(_compiler_id "Clang")
  elseif(_compiler_path_lc MATCHES "gcc" OR _compiler_path_lc MATCHES "g\+\+")
    set(_compiler_id "GNU")
  endif()
endif()

if(_compiler_id STREQUAL "")
  message(FATAL_ERROR "Unable to determine compiler id from ${cache_file}.")
endif()

if(_compiler_id STREQUAL "MSVC")
  if(_is_vs_generator)
    message(FATAL_ERROR
      "MSVC coverage-report is not implemented by this script. If you want an HTML report, use clang/gcc "
      "coverage mode. Visual Studio itself may have coverage tooling; use the VS UI if desired.")
  else()
    message(FATAL_ERROR
      "MSVC coverage-report requires a Visual Studio generator at minimum (and is still not implemented). "
      "Use clang/gcc for this repository's portable coverage-html workflow.")
  endif()
endif()

set(_ctest_args_list "")
if(DEFINED CTEST_ARGS AND NOT CTEST_ARGS STREQUAL "")
  separate_arguments(_ctest_args_list NATIVE_COMMAND "${CTEST_ARGS}")
endif()

if(NOT DEFINED JOBS OR JOBS STREQUAL "")
  cmake_host_system_information(RESULT _cores QUERY NUMBER_OF_LOGICAL_CORES)
  if(NOT _cores OR _cores LESS 1)
    set(_cores 1)
  endif()
  math(EXPR JOBS "${_cores}+1")
endif()

find_program(CTEST_EXECUTABLE NAMES ctest)
if(NOT CTEST_EXECUTABLE)
  message(FATAL_ERROR "ctest not found. Install CMake or ensure it is on PATH.")
endif()

if(_compiler_id MATCHES "Clang")
  find_program(LLVM_PROFDATA_EXECUTABLE NAMES llvm-profdata)
  find_program(LLVM_COV_EXECUTABLE NAMES llvm-cov)
  if(NOT LLVM_PROFDATA_EXECUTABLE)
    message(FATAL_ERROR
      "llvm-profdata not found. Install LLVM tools (llvm-profdata) and ensure it is on PATH (Windows: LLVM/Clang "
      "install includes these binaries).")
  endif()
  if(NOT LLVM_COV_EXECUTABLE)
    message(FATAL_ERROR
      "llvm-cov not found. Install LLVM tools (llvm-cov) and ensure it is on PATH (Windows: LLVM/Clang "
      "install includes these binaries).")
  endif()

  file(GLOB_RECURSE _old_profraw "${BINARY_DIR}/*.profraw")
  if(_old_profraw)
    file(REMOVE ${_old_profraw})
  endif()

  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "LLVM_PROFILE_FILE=universal-%p-%m.profraw"
            "${CTEST_EXECUTABLE}" --output-on-failure -j "${JOBS}" ${_ctest_args_list}
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _ctest_result
  )
  if(NOT _ctest_result EQUAL 0)
    message(FATAL_ERROR "ctest failed during coverage run.")
  endif()

  file(GLOB_RECURSE _profraw_files "${BINARY_DIR}/*.profraw")
  if(NOT _profraw_files)
    message(FATAL_ERROR "No .profraw files found in ${BINARY_DIR}. Ensure coverage instrumentation is enabled.")
  endif()

  set(_profraw_rsp "${BINARY_DIR}/profraw_files.rsp")
  file(WRITE "${_profraw_rsp}" "")
  foreach(p IN LISTS _profraw_files)
    file(APPEND "${_profraw_rsp}" "\"${p}\"\n")
  endforeach()

  execute_process(
    COMMAND "${LLVM_PROFDATA_EXECUTABLE}" merge -sparse @"${_profraw_rsp}" -o "${BINARY_DIR}/universal.profdata"
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _profdata_result
  )
  if(NOT _profdata_result EQUAL 0)
    message(FATAL_ERROR "llvm-profdata merge failed.")
  endif()

  set(_json_required_ver "3.19")
  set(_allow_json_parse FALSE)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "${_json_required_ver}")
    set(_allow_json_parse TRUE)
  endif()

  set(_objects_file "${BINARY_DIR}/test_executables.txt")
  set(_objects_from_json FALSE)
  if(_allow_json_parse)
    execute_process(
      COMMAND "${CTEST_EXECUTABLE}" --show-only=json-v1 ${_ctest_args_list}
      WORKING_DIRECTORY "${BINARY_DIR}"
      OUTPUT_VARIABLE _ctest_json
      RESULT_VARIABLE _ctest_json_result
    )
    string(STRIP "${_ctest_json}" _ctest_json)
    if(_ctest_json_result EQUAL 0
       AND _ctest_json MATCHES "^[ \t\r\n]*\\{"
       AND _ctest_json MATCHES "\"tests\"[ \t\r\n]*:[ \t\r\n]*\\[")
      set(_objects_count 0)
      file(WRITE "${_objects_file}" "")
      string(JSON _tests_len ERROR_VARIABLE _json_error LENGTH "${_ctest_json}" tests)
      if(NOT _json_error AND _tests_len MATCHES "^[0-9]+$")
        set(_objects_seen "")
        set(_idx 0)
        while(_idx LESS _tests_len)
          set(_test_json "")
          set(_json_error "")
          string(JSON _test_json ERROR_VARIABLE _json_error GET "${_ctest_json}" tests ${_idx})
          if(NOT _json_error AND _test_json MATCHES "\"command\"[ \t\r\n]*:[ \t\r\n]*\\[")
            set(_cmd_len 0)
            set(_json_error "")
            string(JSON _cmd_len ERROR_VARIABLE _json_error LENGTH "${_test_json}" command)
            if(NOT _json_error AND _cmd_len MATCHES "^[0-9]+$" AND _cmd_len GREATER 0)
              set(_exe "")
              set(_json_error "")
              string(JSON _exe ERROR_VARIABLE _json_error GET "${_test_json}" command 0)
              if(NOT _json_error AND NOT _exe STREQUAL "")
                list(FIND _objects_seen "${_exe}" _seen_index)
                if(_seen_index EQUAL -1)
                  list(APPEND _objects_seen "${_exe}")
                  string(REPLACE "\"" "\\\"" _exe_escaped "${_exe}")
                  if(_exe_escaped MATCHES "[ \t]")
                    file(APPEND "${_objects_file}" "-object \"${_exe_escaped}\"\n")
                  else()
                    file(APPEND "${_objects_file}" "-object ${_exe_escaped}\n")
                  endif()
                  math(EXPR _objects_count "${_objects_count}+1")
                endif()
              endif()
            endif()
          endif()
          math(EXPR _idx "${_idx}+1")
        endwhile()
      endif()
      if(_objects_count GREATER 0)
        set(_objects_from_json TRUE)
      endif()
    endif()
  endif()

  if(NOT _objects_from_json)
    execute_process(
      COMMAND "${CTEST_EXECUTABLE}" -N -V ${_ctest_args_list}
      WORKING_DIRECTORY "${BINARY_DIR}"
      OUTPUT_VARIABLE _ctest_list
      RESULT_VARIABLE _ctest_list_result
    )
    if(NOT _ctest_list_result EQUAL 0)
      message(FATAL_ERROR "ctest -N -V failed while gathering test executables.")
    endif()

    string(REPLACE "\r\n" "\n" _ctest_list "${_ctest_list}")
    string(REPLACE "\r" "\n" _ctest_list "${_ctest_list}")
    string(REPLACE "\n" ";" _ctest_lines "${_ctest_list}")

    file(WRITE "${_objects_file}" "")
    foreach(line IN LISTS _ctest_lines)
      if(line MATCHES "Test command:(.*)")
        set(_cmd "${CMAKE_MATCH_1}")
        string(STRIP "${_cmd}" _cmd)
        if(NOT _cmd STREQUAL "")
          file(APPEND "${_objects_file}" "-object ${_cmd}\n")
        endif()
      endif()
    endforeach()
  endif()

  file(READ "${_objects_file}" _objects_content)
  if(_objects_content STREQUAL "")
    message(FATAL_ERROR "No test executables found. Ensure tests are configured and built.")
  endif()

  set(_coverage_dir "${BINARY_DIR}/coverage-html")
  file(MAKE_DIRECTORY "${_coverage_dir}")

  execute_process(
    COMMAND "${LLVM_COV_EXECUTABLE}" show
            -instr-profile="${BINARY_DIR}/universal.profdata"
            -format=html
            -output-dir="${_coverage_dir}"
            -show-line-counts-or-regions
            "@${_objects_file}"
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _cov_result
  )
  if(NOT _cov_result EQUAL 0)
    message(FATAL_ERROR "llvm-cov show failed.")
  endif()
elseif(_compiler_id STREQUAL "GNU")
  find_program(GCOVR_EXECUTABLE NAMES gcovr)
  if(NOT GCOVR_EXECUTABLE)
    message(FATAL_ERROR "gcovr not found. Install gcovr (Python package) and ensure it is on PATH.")
  endif()

  execute_process(
    COMMAND "${CTEST_EXECUTABLE}" --output-on-failure -j "${JOBS}" ${_ctest_args_list}
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _ctest_result
  )
  if(NOT _ctest_result EQUAL 0)
    message(FATAL_ERROR "ctest failed during coverage run.")
  endif()

  if(_source_dir STREQUAL "")
    set(_source_dir "${BINARY_DIR}")
  endif()

  set(_coverage_dir "${BINARY_DIR}/coverage-html")
  file(MAKE_DIRECTORY "${_coverage_dir}")

  execute_process(
    COMMAND "${GCOVR_EXECUTABLE}"
            -r "${_source_dir}"
            --html
            --html-details
            -o "${_coverage_dir}/index.html"
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _gcovr_result
  )
  if(NOT _gcovr_result EQUAL 0)
    message(FATAL_ERROR "gcovr failed to generate HTML coverage report.")
  endif()
else()
  message(FATAL_ERROR "Coverage report generation not supported for compiler id: ${_compiler_id}.")
endif()

message(STATUS "Coverage report generated at ${BINARY_DIR}/coverage-html")
