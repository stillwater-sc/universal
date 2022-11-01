file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB STANDARD_SRC "standard/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")
file (GLOB PERFORMANCE_SRC "./performance/*.cpp")

# cfloat API test suites
compile_all("true" "cfloat_clip" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/api" "${API_SRC}")

# logic test suites
compile_all("true" "cfloat_clip" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/logic" "${LOGIC_SRC}")

# conversion test suites
compile_all("true" "cfloat_clip" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/conversion" "${CONVERSION_SRC}")

# arithmetic test suites
compile_all("true" "cfloat_clip" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/arithmetic" "${ARITHMETIC_SRC}")

# standard formats test suites
compile_all("true" "cfloat_clip" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/standard" "${STANDARD_SRC}")

# math library test suites
compile_all("true" "cfloat_clip" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/math" "${MATH_SRC}")

# performance test suites
compile_all("true" "cfloat_clip" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/performance" "${PERFORMANCE_SRC}")

# non-saturating varieties
# conversion

file (GLOB NONSAT_CONV_NORMAL_SRC    "conversion/nonsaturating/normal/*.cpp")
file (GLOB NONSAT_CONV_SUBNORMAL_SRC "conversion/nonsaturating/subnormal/*.cpp")
file (GLOB NONSAT_CONV_SUPNORMAL_SRC "conversion/nonsaturating/supernormal/*.cpp")
file (GLOB NONSAT_CONV_SUBSUP_SRC    "conversion/nonsaturating/subsuper/*.cpp")

compile_all("true" "cfloat_fff" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/conversion/normal" "${NONSAT_CONV_NORMAL_SRC}")
compile_all("true" "cfloat_tff" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/conversion/subnormal" "${NONSAT_CONV_SUBNORMAL_SRC}")
compile_all("true" "cfloat_ftf" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/conversion/supernormal" "${NONSAT_CONV_SUPNORMAL_SRC}")
compile_all("true" "cfloat_ttf" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/conversion/subsuper" "${NONSAT_CONV_SUBSUP_SRC}")

# arithmetic
file (GLOB NONSAT_ARITH_NORMAL_SRC    "arithmetic/nonsaturating/normal/*.cpp")
file (GLOB NONSAT_ARITH_SUBNORMAL_SRC "arithmetic/nonsaturating/subnormal/*.cpp")
file (GLOB NONSAT_ARITH_SUPNORMAL_SRC "arithmetic/nonsaturating/supernormal/*.cpp")
file (GLOB NONSAT_ARITH_SUBSUP_SRC    "arithmetic/nonsaturating/subsuper/*.cpp")

compile_all("true" "cfloat_fff" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/arithmetic/normal" "${NONSAT_ARITH_NORMAL_SRC}")
compile_all("true" "cfloat_tff" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/arithmetic/subnormal" "${NONSAT_ARITH_SUBNORMAL_SRC}")
compile_all("true" "cfloat_ftf" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/arithmetic/supernormal" "${NONSAT_ARITH_SUPNORMAL_SRC}")
compile_all("true" "cfloat_ttf" "Number Systems/fixed size/floating-point/classic/cfloat/clipping/arithmetic/subsuper" "${NONSAT_ARITH_SUBSUP_SRC}")


# saturating varieties
# conversion

file (GLOB SAT_CONV_NORMAL_SRC    "conversion/saturating/normal/*.cpp")
file (GLOB SAT_CONV_SUBNORMAL_SRC "conversion/saturating/subnormal/*.cpp")
file (GLOB SAT_CONV_SUPNORMAL_SRC "conversion/saturating/supernormal/*.cpp")
file (GLOB SAT_CONV_SUBSUP_SRC    "conversion/saturating/subsuper/*.cpp")

compile_all("true" "cfloat_fft" "Number Systems/fixed size/floating-point/classic/cfloat/saturating/conversion/normal" "${SAT_CONV_NORMAL_SRC}")
compile_all("true" "cfloat_tft" "Number Systems/fixed size/floating-point/classic/cfloat/saturating/conversion/subnormal" "${SAT_CONV_SUBNORMAL_SRC}")
compile_all("true" "cfloat_ftt" "Number Systems/fixed size/floating-point/classic/cfloat/saturating/conversion/supernormal" "${SAT_CONV_SUPNORMAL_SRC}")
compile_all("true" "cfloat_ttt" "Number Systems/fixed size/floating-point/classic/cfloat/saturating/conversion/subsuper" "${SAT_CONV_SUBSUP_SRC}")

# arithmetic
file (GLOB SAT_ARITH_NORMAL_SRC    "arithmetic/saturating/normal/*.cpp")
file (GLOB SAT_ARITH_SUBNORMAL_SRC "arithmetic/saturating/subnormal/*.cpp")
file (GLOB SAT_ARITH_SUPNORMAL_SRC "arithmetic/saturating/supernormal/*.cpp")
file (GLOB SAT_ARITH_SUBSUP_SRC    "arithmetic/saturating/subsuper/*.cpp")

compile_all("true" "cfloat_fft" "Number Systems/fixed size/floating-point/classic/cfloat/saturating/arithmetic/normal" "${SAT_ARITH_NORMAL_SRC}")
compile_all("true" "cfloat_tft" "Number Systems/fixed size/floating-point/classic/cfloat/saturating/arithmetic/subnormal" "${SAT_ARITH_SUBNORMAL_SRC}")
compile_all("true" "cfloat_ftt" "Number Systems/fixed size/floating-point/classic/cfloat/saturating/arithmetic/supernormal" "${SAT_ARITH_SUPNORMAL_SRC}")
compile_all("true" "cfloat_ttt" "Number Systems/fixed size/floating-point/classic/cfloat/saturating/arithmetic/subsuper" "${SAT_ARITH_SUBSUP_SRC}")
