file(GLOB API_SRC        "api/*.cpp")
file(GLOB CONVERSION_SRC "conversion/*.cpp")
file(GLOB LOGIC_SRC      "logic/*.cpp")
file(GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file(GLOB MATH_SRC       "math/*.cpp")

compile_all("true" "erat" "Number Systems/elastic/rational/decimal/erational/api"        "${API_SRC}")
compile_all("true" "erat" "Number Systems/elastic/rational/decimal/erational/conversion" "${CONVERSION_SRC}")
compile_all("true" "erat" "Number Systems/elastic/rational/decimal/erational/logic"      "${LOGIC_SRC}")
compile_all("true" "erat" "Number Systems/elastic/rational/decimal/erational/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "erat" "Number Systems/elastic/rational/decimal/erational/math"       "${MATH_SRC}")
