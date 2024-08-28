file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB STANDARD_SRC "standard/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")
file (GLOB PERFORMANCE_SRC "./performance/*.cpp")
file (GLOB COMPLEX_ARITHMETIC_SRC "./complex/arithmetic/*.cpp")
file (GLOB COMPLEX_MATH_SRC "./complex/math/*.cpp")

# cfloat test suites
compile_all("true" "cfloat" "Number Systems/static/floating-point/binary/cfloat/api" "${API_SRC}")
compile_all("true" "cfloat" "Number Systems/static/floating-point/binary/cfloat/conversion" "${CONVERSION_SRC}")
compile_all("true" "cfloat" "Number Systems/static/floating-point/binary/cfloat/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "cfloat" "Number Systems/static/floating-point/binary/cfloat/logic" "${LOGIC_SRC}")
compile_all("true" "cfloat" "Number Systems/static/floating-point/binary/cfloat/standard" "${STANDARD_SRC}")
compile_all("true" "cfloat" "Number Systems/static/floating-point/binary/cfloat/math" "${MATH_SRC}")
compile_all("true" "cfloat" "Number Systems/static/floating-point/binary/cfloat/performance" "${PERFORMANCE_SRC}")

# non-saturating varieties clip towards a sticky INFINITY
# conversion

file (GLOB NONSAT_CONV_NORMAL_SRC    "conversion/nonsaturating/normal/*.cpp")
file (GLOB NONSAT_CONV_SUBNORMAL_SRC "conversion/nonsaturating/subnormal/*.cpp")
file (GLOB NONSAT_CONV_SUPNORMAL_SRC "conversion/nonsaturating/supernormal/*.cpp")
file (GLOB NONSAT_CONV_SUBSUP_SRC    "conversion/nonsaturating/subsuper/*.cpp")

compile_all("true" "cfloat_fff" "Number Systems/static/floating-point/binary/cfloat/conversion/nonsaturating/normal" "${NONSAT_CONV_NORMAL_SRC}")
compile_all("true" "cfloat_tff" "Number Systems/static/floating-point/binary/cfloat/conversion/nonsaturating/subnormal" "${NONSAT_CONV_SUBNORMAL_SRC}")
compile_all("true" "cfloat_ftf" "Number Systems/static/floating-point/binary/cfloat/conversion/nonsaturating/supernormal" "${NONSAT_CONV_SUPNORMAL_SRC}")
compile_all("true" "cfloat_ttf" "Number Systems/static/floating-point/binary/cfloat/conversion/nonsaturating/subsuper" "${NONSAT_CONV_SUBSUP_SRC}")

# arithmetic
file (GLOB NONSAT_ARITH_NORMAL_SRC    "arithmetic/nonsaturating/normal/*.cpp")
file (GLOB NONSAT_ARITH_SUBNORMAL_SRC "arithmetic/nonsaturating/subnormal/*.cpp")
file (GLOB NONSAT_ARITH_SUPNORMAL_SRC "arithmetic/nonsaturating/supernormal/*.cpp")
file (GLOB NONSAT_ARITH_SUBSUP_SRC    "arithmetic/nonsaturating/subsuper/*.cpp")

compile_all("true" "cfloat_fff" "Number Systems/static/floating-point/binary/cfloat/arithmetic/nonsaturating/normal" "${NONSAT_ARITH_NORMAL_SRC}")
compile_all("true" "cfloat_tff" "Number Systems/static/floating-point/binary/cfloat/arithmetic/nonsaturating/subnormal" "${NONSAT_ARITH_SUBNORMAL_SRC}")
compile_all("true" "cfloat_ftf" "Number Systems/static/floating-point/binary/cfloat/arithmetic/nonsaturating/supernormal" "${NONSAT_ARITH_SUPNORMAL_SRC}")
compile_all("true" "cfloat_ttf" "Number Systems/static/floating-point/binary/cfloat/arithmetic/nonsaturating/subsuper" "${NONSAT_ARITH_SUBSUP_SRC}")


# saturating varieties saturate to maximum value on overflow that is not-sticky
# conversion

file (GLOB SAT_CONV_NORMAL_SRC    "conversion/saturating/normal/*.cpp")
file (GLOB SAT_CONV_SUBNORMAL_SRC "conversion/saturating/subnormal/*.cpp")
file (GLOB SAT_CONV_SUPNORMAL_SRC "conversion/saturating/supernormal/*.cpp")
file (GLOB SAT_CONV_SUBSUP_SRC    "conversion/saturating/subsuper/*.cpp")

compile_all("true" "cfloat_fft" "Number Systems/static/floating-point/binary/cfloat/conversion/saturating/normal" "${SAT_CONV_NORMAL_SRC}")
compile_all("true" "cfloat_tft" "Number Systems/static/floating-point/binary/cfloat/conversion/saturating/subnormal" "${SAT_CONV_SUBNORMAL_SRC}")
compile_all("true" "cfloat_ftt" "Number Systems/static/floating-point/binary/cfloat/conversion/saturating/supernormal" "${SAT_CONV_SUPNORMAL_SRC}")
compile_all("true" "cfloat_ttt" "Number Systems/static/floating-point/binary/cfloat/conversion/saturating/subsuper" "${SAT_CONV_SUBSUP_SRC}")

# arithmetic
file (GLOB SAT_ARITH_NORMAL_SRC    "arithmetic/saturating/normal/*.cpp")
file (GLOB SAT_ARITH_SUBNORMAL_SRC "arithmetic/saturating/subnormal/*.cpp")
file (GLOB SAT_ARITH_SUPNORMAL_SRC "arithmetic/saturating/supernormal/*.cpp")
file (GLOB SAT_ARITH_SUBSUP_SRC    "arithmetic/saturating/subsuper/*.cpp")

compile_all("true" "cfloat_fft" "Number Systems/static/floating-point/binary/cfloat/arithmetic/saturating/normal" "${SAT_ARITH_NORMAL_SRC}")
compile_all("true" "cfloat_tft" "Number Systems/static/floating-point/binary/cfloat/arithmetic/saturating/subnormal" "${SAT_ARITH_SUBNORMAL_SRC}")
compile_all("true" "cfloat_ftt" "Number Systems/static/floating-point/binary/cfloat/arithmetic/saturating/supernormal" "${SAT_ARITH_SUPNORMAL_SRC}")
compile_all("true" "cfloat_ttt" "Number Systems/static/floating-point/binary/cfloat/arithmetic/saturating/subsuper" "${SAT_ARITH_SUBSUP_SRC}")


if(BUILD_COMPLEX)

	message(STATUS "Adding regression suite for complex arithmetic using cfloats")
	compile_all("true" "cfloat_cmplx" "Complex/Number Systems/static/floating-point/binary/cfloat/arithmetic" "${COMPLEX_ARITHMETIC_SRC}")

	message(STATUS "Adding regression suite for complex math functions using cfloats")
	compile_all("true" "cfloat_cmplx" "Complex/Number Systems/static/floating-point/binary/cfloat/math" "${COMPLEX_MATH_SRC}")

else(BUILD_COMPLEX)
	message(STATUS "Removing regression suite for complex arithmetic using cfloats")
	message(STATUS "Removing regression suite for complex math functions using cfloats")
endif(BUILD_COMPLEX)

