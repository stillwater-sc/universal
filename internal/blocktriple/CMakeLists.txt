file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB STANDARD_SRC "standard/*.cpp")
file (GLOB PERFORMANCE_SRC "performance/*.cpp")

compile_all("true" "bt" "Internal/block-level/blocktriple/api" "${API_SRC}")
compile_all("true" "bt" "Internal/block-level/blocktriple/logic" "${LOGIC_SRC}")
compile_all("true" "bt" "Internal/block-level/blocktriple/conversion" "${CONVERSION_SRC}")
compile_all("true" "bt" "Internal/block-level/blocktriple/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "bt" "Internal/block-level/blocktriple/standard" "${STANDARD_SRC}")
compile_all("true" "bt" "Internal/block-level/blocktriple/performance" "${PERFORMANCE_SRC}")
