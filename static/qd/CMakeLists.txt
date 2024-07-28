file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")
#file (GLOB PERFORMANCE_SRC "./performance/*.cpp")

compile_all("true" "dd" "Number Systems/static/floating-point/binary/dd/api" "${API_SRC}")
compile_all("true" "dd" "Number Systems/static/floating-point/binary/dd/logic" "${LOGIC_SRC}")
compile_all("true" "dd" "Number Systems/static/floating-point/binary/dd/conversion" "${CONVERSION_SRC}")
compile_all("true" "dd" "Number Systems/static/floating-point/binary/dd/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "dd" "Number Systems/static/floating-point/binary/dd/math" "${MATH_SRC}")
#compile_all("true" "dd" "Number Systems/static/floating-point/binary/dd/performance" "${PERFORMANCE_SRC}")

