file (GLOB API_SRC "api/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")

compile_all("true" "e8m0" "Number Systems/static/floating-point/binary/e8m0/api" "${API_SRC}")
compile_all("true" "e8m0" "Number Systems/static/floating-point/binary/e8m0/conversion" "${CONVERSION_SRC}")
