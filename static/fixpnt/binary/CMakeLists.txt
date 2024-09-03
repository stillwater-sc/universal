file(GLOB API_SRC        "api/*.cpp")
file(GLOB CONVERSION_SRC "conversion/*.cpp")
file(GLOB LOGIC_SRC      "logic/*.cpp")
file(GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file(GLOB MATH_SRC       "math/*.cpp")

file(GLOB COMPLEX_ARITHMETIC_SRC    "complex/arithmetic/*.cpp")
file(GLOB COMPLEX_MATH_SRC    "complex/math/*.cpp")

compile_all("true" "fixpnt" "Number Systems/static/fixed-point/binary/fixpnt/api" "${API_SRC}")
compile_all("true" "fixpnt" "Number Systems/static/fixed-point/binary/fixpnt/conversion" "${CONVERSION_SRC}")
compile_all("true" "fixpnt" "Number Systems/static/fixed-point/binary/fixpnt/logic" "${LOGIC_SRC}")
compile_all("true" "fixpnt" "Number Systems/static/fixed-point/binary/fixpnt/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "fixpnt" "Number Systems/static/fixed-point/binary/fixpnt/math" "${MATH_SRC}")

# Compiler specific environments
message(STATUS "CMAKE_CXX_COMPILER ID is -${CMAKE_CXX_COMPILER_ID}-")

if(BUILD_COMPLEX)

    message(STATUS "Adding regression suite for complex arithmetic using fixpnt")
    compile_all("true" "fixpnt_cmplx" "Complex/Number Systems/static/fixed-point/binary/fixpnt/arithmetic" "${COMPLEX_ARITHMETIC_SRC}")

    message(STATUS "Adding regression suite for complex math functions using fixpnt")
    compile_all("true" "fixpnt_cmplx" "Complex/Number Systems/static/fixed-point/binary/fixpnt/math" "${COMPLEX_MATH_SRC}")

else(BUILD_COMPLEX)
    message(STATUS "Removing regression suite for complex arithmetic using fixpnt")
endif(BUILD_COMPLEX)

