# Install/uninstall regression tests.
#
# Opt-in via -DUNIVERSAL_TEST_INSTALL=ON on the outer configure. When OFF
# (the default) this subdirectory registers no tests, so enabling the tree
# has zero cost for normal development builds.
#
# The tests drive an isolated Universal configure + install into a staging
# prefix, then configure a synthetic consumer project against that prefix
# via find_package(universal CONFIG REQUIRED). They exist to catch two
# regression classes that escaped CI and broke a downstream consumer (see
# issue #708):
#
#   cmake_install_smoke_positive
#       - Installs, builds, and runs a consumer that relies on the
#         imported target to propagate cxx_std_20. Catches regressions
#         where the installed config pollutes the consumer's global scope
#         (e.g. a stray `set(CMAKE_CXX_STANDARD ...)`).
#       - Also runs `make uninstall` and asserts the staging prefix is
#         empty afterwards, catching uninstall-target breakage.
#
#   cmake_install_smoke_missing_component
#       - Configures a consumer that requests a bogus REQUIRED component.
#         Must fail at configure time with universal_FOUND=FALSE. Catches
#         regressions where the installed config forgets to call
#         check_required_components().

if(NOT UNIVERSAL_TEST_INSTALL)
  return()
endif()

set(_drv_dir   "${CMAKE_CURRENT_SOURCE_DIR}")
set(_work_root "${CMAKE_CURRENT_BINARY_DIR}/work")

add_test(
  NAME cmake_install_smoke_positive
  COMMAND ${CMAKE_COMMAND}
    -DUNIVERSAL_SOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DWORK_DIR=${_work_root}/positive
    -DCONSUMER_SOURCE=${_drv_dir}/consumer_positive
    -DTEST_CONFIG=$<CONFIG>
    -P ${_drv_dir}/run_install_test.cmake
)

add_test(
  NAME cmake_install_smoke_missing_component
  COMMAND ${CMAKE_COMMAND}
    -DUNIVERSAL_SOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DWORK_DIR=${_work_root}/missing_component
    -DCONSUMER_SOURCE=${_drv_dir}/consumer_missing_component
    -DTEST_CONFIG=$<CONFIG>
    -P ${_drv_dir}/run_missing_component_test.cmake
)

set_tests_properties(
    cmake_install_smoke_positive
    cmake_install_smoke_missing_component
  PROPERTIES
    LABELS "cmake_install"
    TIMEOUT 600
)
