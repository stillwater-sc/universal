file (GLOB API_SRC "./api/*.cpp")
file (GLOB CONVERSION_SRC "./conversion/*.cpp")
file (GLOB LOGIC_SRC "./logic/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")

compile_all("true" "pxl" "Number Systems/static/floating-point/tapered/poxel/api" "${API_SRC}")
compile_all("true" "pxl" "Number Systems/static/floating-point/tapered/poxel/conversion" "${CONVERSION_SRC}")
compile_all("true" "pxl" "Number Systems/static/floating-point/tapered/poxel/logic" "${LOGIC_SRC}")
compile_all("true" "pxl" "Number Systems/static/floating-point/tapered/poxel/arithmetic" "${ARITHMETIC_SRC}")
