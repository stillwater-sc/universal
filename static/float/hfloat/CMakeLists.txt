file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB STANDARD_SRC "standard/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")
file (GLOB PERFORMANCE_SRC "./performance/*.cpp")

# hfloat API test suites
compile_all("true" "hfloat" "Number Systems/static/floating-point/hexadecimal/hfloat/api" "${API_SRC}")

# logic test suites
compile_all("true" "hfloat" "Number Systems/static/floating-point/hexadecimal/hfloat/logic" "${LOGIC_SRC}")

# conversion test suites
compile_all("true" "hfloat" "Number Systems/static/floating-point/hexadecimal/hfloat/conversion" "${CONVERSION_SRC}")

# arithmetic test suites
compile_all("true" "hfloat" "Number Systems/static/floating-point/hexadecimal/hfloat/arithmetic" "${ARITHMETIC_SRC}")

# standard formats test suites
compile_all("true" "hfloat" "Number Systems/static/floating-point/hexadecimal/hfloat/standard" "${STANDARD_SRC}")

# math library test suites
compile_all("true" "hfloat" "Number Systems/static/floating-point/hexadecimal/hfloat/math" "${MATH_SRC}")

# performance test suites
compile_all("true" "hfloat" "Number Systems/static/floating-point/hexadecimal/hfloat/performance" "${PERFORMANCE_SRC}")
