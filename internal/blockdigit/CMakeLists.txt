file (GLOB API_SRC "api/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")

compile_all("true" "blockdigit" "Internal/block-level/blockdigit/api" "${API_SRC}")
compile_all("true" "blockdigit" "Internal/block-level/blockdigit/conversion" "${CONVERSION_SRC}")
compile_all("true" "blockdigit" "Internal/block-level/blockdigit/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "blockdigit" "Internal/block-level/blockdigit/logic" "${LOGIC_SRC}")
