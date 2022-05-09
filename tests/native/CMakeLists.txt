file (GLOB FLOAT_SRCS "./float/*.cpp")
file (GLOB INTEGER_SRCS "./integer/*.cpp")

compile_all("true" "native_float" "Number Systems/native/floating-point/IEEE-754" "${FLOAT_SRCS}")
compile_all("true" "native_int" "Number Systems/native/integer/integer" "${INTEGER_SRCS}")
