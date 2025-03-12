#file (GLOB ALL_SRCS "./*.cpp")

set(COMPLEX_SRCS complex.cpp)

set(REAL_SRCS efunc_posits.cpp 
	 efunc_valids.cpp 
	 gismo_test.cpp 
	 serialization.cpp 
	 skeleton.cpp 
	 type_test.cpp
	 float_to_decimal_string.cpp
	 generateClosurePlots.cpp

)

compile_all("true" "playground" "Playground" "${REAL_SRCS}")

# NOTE: AppleClang as XCode14 and Xcode15 have std::complex libs that do not support user defined types5
if(BUILD_COMPLEX)
    message(STATUS "Adding playground complex experiment")
    compile_all("true" "playground_cmplx" "Complex/Playground" "${COMPLEX_SRCS}")
else(BUILD_COMPLEX)
    message(STATUS "Removing complex environment experiment in the Playground")
endif(BUILD_COMPLEX)
