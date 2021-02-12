file(GLOB MODULO_SRC "modulo/*.cpp")
file(GLOB SATURATING_SRC "saturating/*.cpp")
file(GLOB API_SRC "api/*.cpp")

compile_all("true" "fixpnt" "Number Systems/fixed size/fixed-point/api" "${API_SRC}")
compile_all("true" "fixpnt" "Number Systems/fixed size/fixed-point/modulo" "${MODULO_SRC}")
compile_all("true" "fixpnt" "Number Systems/fixed size/fixed-point/saturating" "${SATURATING_SRC}")
