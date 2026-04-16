cmake_minimum_required(VERSION 3.18)
project(universal_missing_component_consumer LANGUAGES CXX)

# Universal currently ships no optional components, so requesting a
# REQUIRED component by name is a misuse of find_package. The installed
# universal-config.cmake calls check_required_components() which flips
# universal_FOUND back to FALSE; the REQUIRED keyword then turns the
# miss into a fatal configure error. This configure step is expected
# to fail -- if it succeeds, the negative install-smoke test driver
# reports a regression in check_required_components handling.
find_package(universal CONFIG REQUIRED COMPONENTS bogus_component_xyz)

message(FATAL_ERROR
    "find_package(universal ... REQUIRED COMPONENTS bogus_component_xyz) "
    "should have failed before reaching this message().")
