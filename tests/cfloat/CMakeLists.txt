file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB STANDARD_SRC "standard/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")
file (GLOB PERFORMANCE_SRC "./performance/*.cpp")

# cfloat API test suites
compile_all("true" "cfloat" "Number Systems/fixed size/floating-point/linear/cfloat/api" "${API_SRC}")

# logic test suites
compile_all("true" "cfloat" "Number Systems/fixed size/floating-point/linear/cfloat/logic" "${LOGIC_SRC}")

# conversion test suites
compile_all("true" "cfloat" "Number Systems/fixed size/floating-point/linear/cfloat/conversion" "${CONVERSION_SRC}")

# arithmetic test suites
compile_all("true" "cfloat" "Number Systems/fixed size/floating-point/linear/cfloat/arithmetic" "${ARITHMETIC_SRC}")

# standard formats test suites
compile_all("true" "cfloat" "Number Systems/fixed size/floating-point/linear/cfloat/standard" "${STANDARD_SRC}")

# math library test suites
compile_all("true" "cfloat" "Number Systems/fixed size/floating-point/linear/cfloat/math" "${MATH_SRC}")

# performance test suites
compile_all("true" "cfloat" "Number Systems/fixed size/floating-point/linear/cfloat/performance" "${PERFORMANCE_SRC}")

# non-saturating varieties
# conversion
add_subdirectory("conversion/nonsaturating/normal")
add_subdirectory("conversion/nonsaturating/subnormal")
add_subdirectory("conversion/nonsaturating/supernormal")
add_subdirectory("conversion/nonsaturating/subsuper")
# arithmetic
add_subdirectory("arithmetic/nonsaturating/normal")
add_subdirectory("arithmetic/nonsaturating/subnormal")
add_subdirectory("arithmetic/nonsaturating/supernormal")
add_subdirectory("arithmetic/nonsaturating/subsuper")

# saturating varieties
# conversion
add_subdirectory("conversion/saturating/normal")
add_subdirectory("conversion/saturating/subnormal")
add_subdirectory("conversion/saturating/supernormal")
add_subdirectory("conversion/saturating/subsuper")
# arithmetic
add_subdirectory("arithmetic/saturating/normal")
add_subdirectory("arithmetic/saturating/subnormal")
add_subdirectory("arithmetic/saturating/supernormal")
add_subdirectory("arithmetic/saturating/subsuper")
