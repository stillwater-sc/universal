file(GLOB MODULO_SRC "./mod_*.cpp")
file(GLOB SATURATING_SRC "./sat_*.cpp")
file(GLOB COMPLEX_SRC "./complex/*.cpp")
set(SOURCES api.cpp constexpr.cpp complex.cpp tables.cpp)

compile_all("true" "fixpnt" "Number Systems/fixed-point" "${SOURCES}")
compile_all("true" "fixpnt" "Number Systems/fixed-point/complex" "${COMPLEX_SRC}")
compile_all("true" "fixpnt" "Number Systems/fixed-point/modulo" "${MODULO_SRC}")
compile_all("true" "fixpnt" "Number Systems/fixed-point/saturating" "${SATURATING_SRC}")
