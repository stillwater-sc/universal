file (GLOB SOURCES "./*.cpp")

compile_all("true" "quantization" "Benchmarks/Error/QSNR" "${SOURCES}")
