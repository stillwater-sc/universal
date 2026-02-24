file(GLOB API_SRC         "api/*.cpp")
file(GLOB CONVERSION_SRC  "conversion/*.cpp")
file(GLOB LOGIC_SRC       "logic/*.cpp")
file(GLOB ARITHMETIC_SRC  "arithmetic/*.cpp")

compile_all("true" "oint" "Number Systems/static/integer/octal/positional/api" "${API_SRC}")
compile_all("true" "oint" "Number Systems/static/integer/octal/positional/conversion" "${CONVERSION_SRC}")
compile_all("true" "oint" "Number Systems/static/integer/octal/positional/logic" "${LOGIC_SRC}")
compile_all("true" "oint" "Number Systems/static/integer/octal/positional/arithmetic" "${ARITHMETIC_SRC}")
