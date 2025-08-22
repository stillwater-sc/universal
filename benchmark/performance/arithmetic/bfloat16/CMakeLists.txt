file (GLOB SOURCES "./*.cpp")

compile_all("true" "bfloat16" "Benchmarks/Performance/Arithmetic/bfloat16" "${SOURCES}")
