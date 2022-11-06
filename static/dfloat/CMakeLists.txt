file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB STANDARD_SRC "standard/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")
file (GLOB PERFORMANCE_SRC "./performance/*.cpp")

# dfloat API test suites
compile_all("true" "dfloat" "Number Systems/static/floating-point/decimal/dfloat/api" "${API_SRC}")

# logic test suites
compile_all("true" "dfloat" "Number Systems/static/floating-point/decimal/dfloat/logic" "${LOGIC_SRC}")

# conversion test suites
compile_all("true" "dfloat" "Number Systems/static/floating-point/decimal/dfloat/conversion" "${CONVERSION_SRC}")

# arithmetic test suites
compile_all("true" "dfloat" "Number Systems/static/floating-point/decimal/dfloat/arithmetic" "${ARITHMETIC_SRC}")

# standard formats test suites
compile_all("true" "dfloat" "Number Systems/static/floating-point/decimal/dfloat/standard" "${STANDARD_SRC}")

# math library test suites
compile_all("true" "dfloat" "Number Systems/static/floating-point/decimal/dfloat/math" "${MATH_SRC}")

# performance test suites
compile_all("true" "dfloat" "Number Systems/static/floating-point/decimal/dfloat/performance" "${PERFORMANCE_SRC}")

