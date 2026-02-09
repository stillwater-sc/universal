file (GLOB API_SRC "api/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB ACCURACY_SRC "accuracy/*.cpp")

compile_all("true" "mxfloat" "Number Systems/static/floating-point/binary/mxfloat/api" "${API_SRC}")
compile_all("true" "mxfloat" "Number Systems/static/floating-point/binary/mxfloat/conversion" "${CONVERSION_SRC}")
compile_all("true" "mxfloat" "Number Systems/static/floating-point/binary/mxfloat/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "mxfloat" "Number Systems/static/floating-point/binary/mxfloat/accuracy" "${ACCURACY_SRC}")
