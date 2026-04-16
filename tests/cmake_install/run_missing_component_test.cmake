# Negative install-smoke driver: verify that find_package(universal ...
# REQUIRED COMPONENTS <bogus>) fails cleanly at the config site.
#
# Rationale: universal currently defines no optional components. A caller
# that requests one by name is misusing find_package. The installed
# universal-config.cmake calls check_required_components() to flip
# universal_FOUND back to FALSE so CMake's find_package(REQUIRED) machinery
# produces a clear configure-time error. Without that call the package
# silently appears found and the misuse ricochets into a later build-step
# failure with much less diagnostic value.

foreach(var IN ITEMS UNIVERSAL_SOURCE_DIR WORK_DIR CONSUMER_SOURCE)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "missing-component smoke: required argument -D${var}=... is missing")
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
  message(FATAL_ERROR "missing-component smoke: Universal configure failed (rc=${rc})")
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} --install "${UBUILD}" --config ${TEST_CONFIG}
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "missing-component smoke: cmake --install failed (rc=${rc})")
endif()

file(GLOB_RECURSE found_configs "${PREFIX}/*/universal-config.cmake")
list(LENGTH found_configs num_configs)
if(NOT num_configs EQUAL 1)
  message(FATAL_ERROR
    "missing-component smoke: expected exactly one universal-config.cmake, "
    "got ${num_configs}: ${found_configs}")
endif()
list(GET found_configs 0 config_path)
get_filename_component(config_dir "${config_path}" DIRECTORY)

message(STATUS "missing-component smoke: configure consumer that requests a bogus REQUIRED component (must fail)")
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -S "${CONSUMER_SOURCE}"
    -B "${CBUILD}"
    -DCMAKE_BUILD_TYPE=${TEST_CONFIG}
    -Duniversal_DIR=${config_dir}
  RESULT_VARIABLE  rc
  OUTPUT_VARIABLE  stdout_text
  ERROR_VARIABLE   stderr_text
)
if(rc EQUAL 0)
  message(FATAL_ERROR
    "missing-component smoke: consumer configure succeeded but should have failed.\n"
    "stdout:\n${stdout_text}\nstderr:\n${stderr_text}")
endif()

set(combined "${stdout_text}${stderr_text}")
# CMake's standard error text for find_package when _FOUND is flipped back
# to FALSE is: "but it set <PackageName>_FOUND to FALSE so package
# <PackageName> is considered to be NOT FOUND." Matching on universal_FOUND
# is stable across CMake versions.
if(NOT combined MATCHES "universal_FOUND")
  message(FATAL_ERROR
    "missing-component smoke: consumer failed, but not with the expected\n"
    "check_required_components() error -- 'universal_FOUND' was not in the output.\n"
    "stdout:\n${stdout_text}\nstderr:\n${stderr_text}")
endif()

message(STATUS "missing-component smoke: PASS - find_package failed with expected message")
