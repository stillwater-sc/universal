file (GLOB API_SRC "./api/*.cpp")
file (GLOB CONVERSION_SRC "./conversion/*.cpp")
file (GLOB LOGIC_SRC "./logic/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")

compile_all("true" "bpst" "Number Systems/static/floating-point/tapered/bposit/api" "${API_SRC}")
compile_all("true" "bpst" "Number Systems/static/floating-point/tapered/bposit/conversion" "${CONVERSION_SRC}")
compile_all("true" "bpst" "Number Systems/static/floating-point/tapered/bposit/logic" "${LOGIC_SRC}")
compile_all("true" "bpst" "Number Systems/static/floating-point/tapered/bposit/arithmetic" "${ARITHMETIC_SRC}")
