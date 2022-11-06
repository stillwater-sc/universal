file (GLOB API_SRC "./api/*.cpp")
file (GLOB CONVERSION_SRC "./conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")
file (GLOB PERFORMANCE_SRC "./performance/*.cpp")

compile_all("true" "bs" "Internal/block-level/blocksignificant/api" "${API_SRC}")
compile_all("true" "bs" "Internal/block-level/blocksignificant/conversion" "${CONVERSION_SRC}")
compile_all("true" "bs" "Internal/block-level/blocksignificant/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "bs" "Internal/block-level/blocksignificant/performance" "${PERFORMANCE_SRC}")
