file (GLOB API_SRC "./api/*.cpp")
file (GLOB CONVERSION_SRC "./conversion/*.cpp")
file (GLOB LOGIC_SRC "./logic/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")
file (GLOB COMPLEX_ARITHMETIC_SRC "./complex/arithmetic/*.cpp")
file (GLOB COMPLEX_MATH_SRC "./complex/math/*.cpp")

compile_all("true" "posit1" "Number Systems/static/floating-point/tapered/posit1/api" "${API_SRC}")
compile_all("true" "posit1" "Number Systems/static/floating-point/tapered/posit1/conversion" "${CONVERSION_SRC}")
compile_all("true" "posit1" "Number Systems/static/floating-point/tapered/posit1/logic" "${LOGIC_SRC}")
compile_all("true" "posit1" "Number Systems/static/floating-point/tapered/posit1/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "posit1" "Number Systems/static/floating-point/tapered/posit1/math" "${MATH_SRC}")


if(BUILD_COMPLEX)

	message(STATUS "Adding regression suite for complex arithmetic using posit1")
	compile_all("true" "posit1_cmplx" "Complex/Number Systems/static/floating-point/tapered/posit1/arithmetic" "${COMPLEX_ARITHMETIC_SRC}")

	message(STATUS "Adding regression suite for complex math functions using posit1")
	compile_all("true" "posit1_cmplx" "Complex/Number Systems/static/floating-point/tapered/posit1/math" "${COMPLEX_MATH_SRC}")

else(BUILD_COMPLEX)
	message(STATUS "Removing regression suite for complex arithmetic using posit1")
	message(STATUS "Removing regression suite for complex math functions using posit1")
endif(BUILD_COMPLEX)
