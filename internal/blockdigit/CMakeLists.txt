file (GLOB API_SRC "api/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")

compile_all("true" "blockdigit" "Number Systems/static/blockdigit/api" "${API_SRC}")
compile_all("true" "blockdigit" "Number Systems/static/blockdigit/conversion" "${CONVERSION_SRC}")
compile_all("true" "blockdigit" "Number Systems/static/blockdigit/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "blockdigit" "Number Systems/static/blockdigit/logic" "${LOGIC_SRC}")
