file(GLOB API_SRC         "api/*.cpp")
file(GLOB CONVERSION_SRC  "conversion/*.cpp")
file(GLOB LOGIC_SRC       "logic/*.cpp")
file(GLOB ARITHMETIC_SRC  "arithmetic/*.cpp")

compile_all("true" "hint" "Number Systems/static/integer/hexadecimal/positional/api" "${API_SRC}")
compile_all("true" "hint" "Number Systems/static/integer/hexadecimal/positional/conversion" "${CONVERSION_SRC}")
compile_all("true" "hint" "Number Systems/static/integer/hexadecimal/positional/logic" "${LOGIC_SRC}")
compile_all("true" "hint" "Number Systems/static/integer/hexadecimal/positional/arithmetic" "${ARITHMETIC_SRC}")
