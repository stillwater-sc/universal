#file (GLOB ALL_SRCS "./*.cpp")

set(COMPLEX_SRCS complex.cpp)

set(CONCURRENCY_SRCS concurrency.cpp)

set(REAL_SRCS efunc_posits.cpp 
	 efunc_valids.cpp 
	 gismo_test.cpp 
	 serialization.cpp 
	 skeleton.cpp 
	 type_test.cpp
	 float_to_decimal_string.cpp
	 lazy_evaluation.cpp
	 expression_templates.cpp
)

compile_all("true" "playground" "Playground" "${REAL_SRCS}")

# NOTE: AppleClang as XCode14 and Xcode15 have std::complex libs that do not support user defined types5
if(UNIVRSL_BUILD_COMPLEX)
    message(STATUS "Adding playground complex experiment")
    compile_all("true" "play_cmplx" "Complex/Playground" "${COMPLEX_SRCS}")
else(UNIVRSL_BUILD_COMPLEX)
    message(STATUS "Removing complex environment experiment in the Playground")
endif(UNIVRSL_BUILD_COMPLEX)

if(UNIVRSL_BUILD_CONCURRENCY)
    message(STATUS "Adding playground concurrency experiment")
    compile_all("true" "play_conc" "Concurrency/Playground" "${COMPLEX_SRCS}")
else(UNIVRSL_BUILD_CONCURRENCY)
    message(STATUS "Removing concurrency experiment in the Playground")
endif(UNIVRSL_BUILD_CONCURRENCY)
