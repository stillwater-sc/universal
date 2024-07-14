file (GLOB ALL_SRCS "./*.cpp")

set(COMPLEX_SRC complex.cpp)

set(SRCS efunc_posits.cpp 
	 efunc_valids.cpp 
	 gismo_test.cpp 
	 meta_programming.cpp 
	 serialization.cpp 
	 skeleton.cpp 
	 type_test.cpp)

compile_all("true" "playground" "Playground" "${SRCS}")
