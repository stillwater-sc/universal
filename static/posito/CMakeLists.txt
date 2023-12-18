file (GLOB API_SRC "./api/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")

compile_all("true" "posito" "Number Systems/static/floating-point/tapered/posito/api" "${API_SRC}")
compile_all("true" "posito" "Number Systems/static/floating-point/tapered/posito/arithmetic" "${ARITHMETIC_SRC}")
