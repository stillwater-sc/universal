file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")

compile_all("true" "microfloat" "Number Systems/static/floating-point/binary/microfloat/api" "${API_SRC}")
compile_all("true" "microfloat" "Number Systems/static/floating-point/binary/microfloat/logic" "${LOGIC_SRC}")
compile_all("true" "microfloat" "Number Systems/static/floating-point/binary/microfloat/conversion" "${CONVERSION_SRC}")
compile_all("true" "microfloat" "Number Systems/static/floating-point/binary/microfloat/arithmetic" "${ARITHMETIC_SRC}")
