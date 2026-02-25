file (GLOB API_SRC "api/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB ACCURACY_SRC "accuracy/*.cpp")

compile_all("true" "nvblock" "Number Systems/static/floating-point/binary/nvblock/api" "${API_SRC}")
compile_all("true" "nvblock" "Number Systems/static/floating-point/binary/nvblock/conversion" "${CONVERSION_SRC}")
compile_all("true" "nvblock" "Number Systems/static/floating-point/binary/nvblock/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "nvblock" "Number Systems/static/floating-point/binary/nvblock/accuracy" "${ACCURACY_SRC}")
