file (GLOB API_SRC "./api/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")
file (GLOB CONVERSION_SRC "./conversion/*.cpp")

compile_all("true" "posito" "Number Systems/static/floating-point/tapered/posito/api" "${API_SRC}")
compile_all("true" "posito" "Number Systems/static/floating-point/tapered/posito/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "posito" "Number Systems/static/floating-point/tapered/posito/conversion" "${CONVERSION_SRC}")
