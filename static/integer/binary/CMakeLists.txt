file(GLOB API_SRC         "api/*.cpp")
file(GLOB CONVERSION_SRC  "conversion/*.cpp")
file(GLOB LOGIC_SRC       "logic/*.cpp")
file(GLOB ARITHMETIC_SRC  "arithmetic/*.cpp")
file(GLOB MATH_SRC        "math/*.cpp")
file(GLOB PERFORMANCE_SRC "performance/*.cpp")

compile_all("true" "bint" "Number Systems/static/integer/binary/integer/api" "${API_SRC}")
# native_and_limits.cpp checks that the double-double guards raise no FE_INVALID / FE_OVERFLOW:
# the compiler must not fold those operations away at compile time (clang's default exception
# behavior is "ignore"), so this source keeps the floating-point exception semantics
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set_source_files_properties(conversion/native_and_limits.cpp PROPERTIES COMPILE_OPTIONS "-ffp-exception-behavior=strict")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set_source_files_properties(conversion/native_and_limits.cpp PROPERTIES COMPILE_OPTIONS "-ftrapping-math;-frounding-math")
elseif(MSVC)
	set_source_files_properties(conversion/native_and_limits.cpp PROPERTIES COMPILE_OPTIONS "/fp:strict")
endif()
compile_all("true" "bint" "Number Systems/static/integer/binary/integer/conversion" "${CONVERSION_SRC}")
compile_all("true" "bint" "Number Systems/static/integer/binary/integer/logic" "${LOGIC_SRC}")
compile_all("true" "bint" "Number Systems/static/integer/binary/integer/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "bint" "Number Systems/static/integer/binary/integer/math" "${MATH_SRC}")
compile_all("true" "bint" "Number Systems/static/integer/binary/integer/performance" "${PERFORMANCE_SRC}")
