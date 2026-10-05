file(GLOB API_SRC "api/*.cpp")
file(GLOB LOGIC_SRC "logic/*.cpp")
file(GLOB ARITHMETIC_SRC "arithmetic/*.cpp")

compile_all("true" "valid" "Number Systems/static/floating-point/tapered/valid/api" "${API_SRC}")
compile_all("true" "valid" "Number Systems/static/floating-point/tapered/valid/logic" "${LOGIC_SRC}")
compile_all("true" "valid" "Number Systems/static/floating-point/tapered/valid/arithmetic" "${ARITHMETIC_SRC}")
