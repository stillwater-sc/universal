file(GLOB API_SRC        "api/*.cpp")
file(GLOB CONVERSION_SRC "conversion/*.cpp")
file(GLOB LOGIC_SRC      "logic/*.cpp")
file(GLOB ARITHMETIC_SRC "arithmetic/*.cpp")

compile_all("true" "dfixpnt" "Number Systems/static/fixed-point/decimal/dfixpnt/api" "${API_SRC}")
compile_all("true" "dfixpnt" "Number Systems/static/fixed-point/decimal/dfixpnt/conversion" "${CONVERSION_SRC}")
compile_all("true" "dfixpnt" "Number Systems/static/fixed-point/decimal/dfixpnt/logic" "${LOGIC_SRC}")
compile_all("true" "dfixpnt" "Number Systems/static/fixed-point/decimal/dfixpnt/arithmetic" "${ARITHMETIC_SRC}")
