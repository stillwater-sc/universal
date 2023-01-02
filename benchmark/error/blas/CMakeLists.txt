file (GLOB SOURCES "./*.cpp")

compile_all("true" "error" "Benchmarks/Error/BLAS" "${SOURCES}")
