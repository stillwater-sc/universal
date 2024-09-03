file (GLOB API_SRC "./api/*.cpp")
file (GLOB CONVERSION_SRC "./conversion/*.cpp")
file (GLOB LOGIC_SRC "./logic/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")
file (GLOB COMPLEX_ARITHMETIC_SRC "./complex/arithmetic/*.cpp")
file (GLOB COMPLEX_MATH_SRC "./complex/math/*.cpp")

compile_all("true" "posit" "Number Systems/static/floating-point/tapered/posit/api" "${API_SRC}")
compile_all("true" "posit" "Number Systems/static/floating-point/tapered/posit/conversion" "${CONVERSION_SRC}")
compile_all("true" "posit" "Number Systems/static/floating-point/tapered/posit/logic" "${LOGIC_SRC}")
compile_all("true" "posit" "Number Systems/static/floating-point/tapered/posit/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "posit" "Number Systems/static/floating-point/tapered/posit/math" "${MATH_SRC}")


if(BUILD_COMPLEX)

	message(STATUS "Adding regression suite for complex arithmetic using posits")
	compile_all("true" "posit_cmplx" "Complex/Number Systems/static/floating-point/tapered/posit/arithmetic" "${COMPLEX_ARITHMETIC_SRC}")

	message(STATUS "Adding regression suite for complex math functions using posits")
	compile_all("true" "posit_cmplx" "Complex/Number Systems/static/floating-point/tapered/posit/math" "${COMPLEX_MATH_SRC}")

else(BUILD_COMPLEX)
	message(STATUS "Removing regression suite for complex arithmetic using posits")
	message(STATUS "Removing regression suite for complex math functions using posits")
endif(BUILD_COMPLEX)
