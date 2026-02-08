file (GLOB API_SRCS "./api/*.cpp")
file (GLOB CONVERSION_SRCS "./conversion/*.cpp")
file (GLOB ARITHMETIC_SRCS "./arithmetic/*.cpp")
file (GLOB LOGIC_SRCS "./logic/*.cpp")

compile_all("true" "interval" "Number Systems/static/interval/api" "${API_SRCS}")
compile_all("true" "interval" "Number Systems/static/interval/conversion" "${CONVERSION_SRCS}")
compile_all("true" "interval" "Number Systems/static/interval/arithmetic" "${ARITHMETIC_SRCS}")
compile_all("true" "interval" "Number Systems/static/interval/logic" "${LOGIC_SRCS}")
