# Positive install-smoke driver.
#
# Args (pass via -D on the outer `cmake -P` invocation):
#   UNIVERSAL_SOURCE_DIR - absolute path to the Universal source tree
#   WORK_DIR             - unique scratch directory for this test run
#   CONSUMER_SOURCE      - source directory of the consumer_positive project
#   TEST_CONFIG          - CMake build config (Debug/Release/..., default Release)
#
# Sequence:
#   1. Configure Universal into ${WORK_DIR}/universal_build with a minimal
#      feature set. The header-only interface target and its install rules
#      are available regardless of the UNIVERSAL_BUILD_* options, so no
#      Universal compile step is required.
#   2. `cmake --install` into ${WORK_DIR}/staging_prefix.
#   3. Locate the installed universal-config.cmake (Linux/macOS put it at
#      share/universal/, Windows at CMake/).
#   4. Configure + build + run the consumer against the staging prefix.
#   5. Check the consumer's stdout matches the expected arithmetic output.
#   6. `cmake --build . --target uninstall` on the Universal build.
#   7. Assert the staging prefix contains zero files.

foreach(var IN ITEMS UNIVERSAL_SOURCE_DIR WORK_DIR CONSUMER_SOURCE)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "install smoke: required argument -D${var}=... is missing")
  endif()
endforeach()

if(NOT DEFINED TEST_CONFIG OR TEST_CONFIG STREQUAL "")
  set(TEST_CONFIG "Release")
endif()

set(UBUILD "${WORK_DIR}/universal_build")
set(PREFIX "${WORK_DIR}/staging_prefix")
set(CBUILD "${WORK_DIR}/consumer_build")

foreach(dir IN ITEMS "${UBUILD}" "${PREFIX}" "${CBUILD}")
  file(REMOVE_RECURSE "${dir}")
  file(MAKE_DIRECTORY "${dir}")
endforeach()

message(STATUS "install smoke: configure Universal in ${UBUILD}")
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -S "${UNIVERSAL_SOURCE_DIR}"
    -B "${UBUILD}"
    -DCMAKE_BUILD_TYPE=${TEST_CONFIG}
    -DCMAKE_INSTALL_PREFIX=${PREFIX}
    -DUNIVERSAL_ENABLE_TESTS=OFF
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "install smoke: Universal configure failed (rc=${rc})")
endif()

message(STATUS "install smoke: install Universal to ${PREFIX}")
execute_process(
  COMMAND ${CMAKE_COMMAND} --install "${UBUILD}" --config ${TEST_CONFIG}
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "install smoke: cmake --install failed (rc=${rc})")
endif()

file(GLOB_RECURSE found_configs "${PREFIX}/*/universal-config.cmake")
list(LENGTH found_configs num_configs)
if(NOT num_configs EQUAL 1)
  message(FATAL_ERROR
    "install smoke: expected exactly one universal-config.cmake under ${PREFIX}, "
    "found ${num_configs}: ${found_configs}")
endif()
list(GET found_configs 0 config_path)
get_filename_component(config_dir "${config_path}" DIRECTORY)
message(STATUS "install smoke: found config at ${config_path}")

foreach(sibling IN ITEMS universal-config-version.cmake universal-targets.cmake)
  if(NOT EXISTS "${config_dir}/${sibling}")
    message(FATAL_ERROR "install smoke: missing ${config_dir}/${sibling}")
  endif()
endforeach()

message(STATUS "install smoke: configure consumer (universal_DIR=${config_dir})")
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -S "${CONSUMER_SOURCE}"
    -B "${CBUILD}"
    -DCMAKE_BUILD_TYPE=${TEST_CONFIG}
    -Duniversal_DIR=${config_dir}
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "install smoke: consumer configure failed (rc=${rc})")
endif()

message(STATUS "install smoke: build consumer")
execute_process(
  COMMAND ${CMAKE_COMMAND} --build "${CBUILD}" --config ${TEST_CONFIG}
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "install smoke: consumer build failed (rc=${rc})")
endif()

set(consumer_exe "")
foreach(candidate IN ITEMS
    "${CBUILD}/consumer"
    "${CBUILD}/consumer.exe"
    "${CBUILD}/${TEST_CONFIG}/consumer"
    "${CBUILD}/${TEST_CONFIG}/consumer.exe"
    "${CBUILD}/Release/consumer.exe"
    "${CBUILD}/Debug/consumer.exe")
  if(EXISTS "${candidate}")
    set(consumer_exe "${candidate}")
    break()
  endif()
endforeach()
if(consumer_exe STREQUAL "")
  file(GLOB_RECURSE found_any "${CBUILD}/*consumer*")
  message(FATAL_ERROR "install smoke: could not locate consumer binary; found: ${found_any}")
endif()

message(STATUS "install smoke: run ${consumer_exe}")
execute_process(
  COMMAND "${consumer_exe}"
  RESULT_VARIABLE  rc
  OUTPUT_VARIABLE  stdout_text
  ERROR_VARIABLE   stderr_text
)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR
    "install smoke: consumer run failed (rc=${rc})\n"
    "stdout:\n${stdout_text}\nstderr:\n${stderr_text}")
endif()
if(NOT stdout_text MATCHES "posit32 sum = 3\\.75")
  message(FATAL_ERROR
    "install smoke: consumer output missing expected 'posit32 sum = 3.75...' line\n"
    "Got:\n${stdout_text}")
endif()
message(STATUS "install smoke: consumer produced expected arithmetic")

message(STATUS "install smoke: run uninstall target")
execute_process(
  COMMAND ${CMAKE_COMMAND} --build "${UBUILD}" --target uninstall --config ${TEST_CONFIG}
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "install smoke: uninstall target failed (rc=${rc})")
endif()

file(GLOB_RECURSE remaining "${PREFIX}/*")
list(LENGTH remaining n_remaining)
if(NOT n_remaining EQUAL 0)
  message(FATAL_ERROR
    "install smoke: ${n_remaining} files remain under ${PREFIX} after uninstall:\n"
    "  ${remaining}")
endif()

message(STATUS "install smoke: PASS")
