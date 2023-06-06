file (GLOB SOURCES "./*.cpp")
file (GLOB INCLUDES "../../include/universal/blas/vmath/*.hpp")

compile_all("true" "vmath" "Linear Algebra/vmath" "${SOURCES}")
