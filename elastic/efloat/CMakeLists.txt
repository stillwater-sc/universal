file (GLOB API_SRCS "./api/*.cpp")
file (GLOB CONVERSION_SRCS "./conversion/*.cpp")
file (GLOB ARITHMETIC_SRCS "./arithmetic/*.cpp")
file (GLOB LOGIC_SRCS "./logic/*.cpp")
file (GLOB MATHLIB_SRCS "./math/*.cpp")
file (GLOB PERFORMANCE_SRCS "./performance/*.cpp")

compile_all("true" "efloat" "Number Systems/elastic/floating-point/binary/efloat/api" "${API_SRCS}")
compile_all("true" "efloat" "Number Systems/elastic/floating-point/binary/efloat/conversion" "${CONVERSION_SRCS}")
compile_all("true" "efloat" "Number Systems/elastic/floating-point/binary/efloat/arithmetic" "${ARITHMETIC_SRCS}")
compile_all("true" "efloat" "Number Systems/elastic/floating-point/binary/efloat/logic" "${LOGIC_SRCS}")
compile_all("true" "efloat" "Number Systems/elastic/floating-point/binary/efloat/mathlib" "${MATHLIB_SRCS}")
compile_all("true" "efloat" "Number Systems/elastic/floating-point/binary/efloat/performance" "${PERFORMANCE_SRCS}")

# The math library and parse() in a constant evaluation (#1819): lgamma and tgamma sum Stirling's
# series from coefficients a constant evaluation cannot cache, and pi, e and phi past a few hundred
# bits read a long literal, both past clang's and MSVC's default step limits. gcc's default
# operation limit holds the math library; the 3300-bit constants need twice it.
foreach(target efloat_constexpr_mathlib efloat_constexpr_parse efloat_constexpr_constant)
	if(TARGET ${target})
		if(MSVC)
			target_compile_options(${target} PRIVATE /constexpr:steps33554432)
		elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			target_compile_options(${target} PRIVATE -fconstexpr-steps=33554432)
		elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			target_compile_options(${target} PRIVATE -fconstexpr-ops-limit=67108864)
		endif()
	endif()
endforeach()
