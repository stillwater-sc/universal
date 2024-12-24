#file (GLOB ALL_SRCS "./*.cpp")

set(COMPLEX_SRCS complex.cpp)

set(CONCURRENCY_SRCS concurrency.cpp)

set(EXAMPLE_SRCS efunc_posits.cpp 
	 efunc_valids.cpp 
	 gismo_test.cpp 
	 serialization.cpp 
	 skeleton.cpp 
	 type_test.cpp
	 float_to_decimal_string.cpp
	 conversion.cpp
)

set(ET_SRCS expression_templates.cpp
	 lazy_evaluation.cpp
	 #lazy_evaluation_2.cpp
)

compile_all("true" "play" "Playground/Integration" "${EXAMPLE_SRCS}")
compile_all("true" "play" "Playground/Expression Templates" "${ET_SRCS}")

# NOTE: AppleClang as XCode14 and Xcode15 have std::complex libs that do not support user defined types5
if(UNIVRSL_BUILD_COMPLEX)
    message(STATUS "Adding playground complex experiment")
    compile_all("true" "play" "Playground/Complex" "${COMPLEX_SRCS}")
else(UNIVRSL_BUILD_COMPLEX)
    message(STATUS "Removing complex environment experiment in the Playground")
endif(UNIVRSL_BUILD_COMPLEX)

if(UNIVRSL_BUILD_CONCURRENCY)
    message(STATUS "Adding playground concurrency experiment")
    compile_all("true" "play" "Playground/Concurrency" "${COMPLEX_SRCS}")
else(UNIVRSL_BUILD_CONCURRENCY)
    message(STATUS "Removing concurrency experiment in the Playground")
endif(UNIVRSL_BUILD_CONCURRENCY)
