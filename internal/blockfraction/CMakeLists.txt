file (GLOB API_SRC "./api/*.cpp")
file (GLOB CONVERSION_SRC "./conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")
file (GLOB PERFORMANCE_SRC "./performance/*.cpp")

compile_all("true" "bf" "Internal/block-level/blockfraction/api" "${API_SRC}")
compile_all("true" "bf" "Internal/block-level/blockfraction/conversion" "${CONVERSION_SRC}")
compile_all("true" "bf" "Internal/block-level/blockfraction/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "bf" "Internal/block-level/blockfraction/performance" "${PERFORMANCE_SRC}")
