file(GLOB API_SRC        "api/*.cpp")
file(GLOB CONVERSION_SRC "conversion/*.cpp")
file(GLOB LOGIC_SRC      "logic/*.cpp")
file(GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file(GLOB COMPLEX_SRC    "complex/*.cpp")
file(GLOB MATH_SRC       "math/*.cpp")

compile_all("true" "deci" "Number Systems/static/fixed-point/decimal/deci/api" "${API_SRC}")
compile_all("true" "deci" "Number Systems/static/fixed-point/decimal/deci/conversion" "${CONVERSION_SRC}")
compile_all("true" "deci" "Number Systems/static/fixed-point/decimal/deci/logic" "${LOGIC_SRC}")
compile_all("true" "deci" "Number Systems/static/fixed-point/decimal/deci/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "deci" "Number Systems/static/fixed-point/decimal/deci/complex" "${COMPLEX_SRC}")
compile_all("true" "deci" "Number Systems/static/fixed-point/decimal/deci/math" "${MATH_SRC}")
