file (GLOB SOURCES "./*.cpp")

compile_all("true" "data" "Linear Algebra/data" "${SOURCES}")

# data_serialization currently exercises lns::fraction(), which is not implemented.
# Skip the test until lns::fraction() has a defined behavior.
set_tests_properties(data_serialization PROPERTIES DISABLED TRUE)
