file(GLOB MODULO_SRC "./mod_*.cpp")
file(GLOB SATURATING_SRC "./sat_*.cpp")
file(GLOB COMPLEX_SRC "./complex/*.cpp")
set(SOURCES api.cpp constexpr.cpp complex.cpp tables.cpp)

compile_all("true" "fixpnt" "Number Systems/fixed size/fixed-point/api" "${SOURCES}")
compile_all("true" "fixpnt" "Number Systems/fixed size/fixed-point/complex" "${COMPLEX_SRC}")
compile_all("true" "fixpnt" "Number Systems/fixed size/fixed-point/modulo" "${MODULO_SRC}")
compile_all("true" "fixpnt" "Number Systems/fixed size/fixed-point/saturating" "${SATURATING_SRC}")
