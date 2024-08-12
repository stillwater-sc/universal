#file (GLOB ALL_SRCS "./*.cpp")

set(COMPLEX_SRCS complex.cpp)

set(REAL_SRCS efunc_posits.cpp 
	 efunc_valids.cpp 
	 gismo_test.cpp 
	 serialization.cpp 
	 skeleton.cpp 
	 type_test.cpp
	 float_to_decimal_string.cpp
)

compile_all("true" "playground" "Playground" "${REAL_SRCS}")

# exclude AppleClang as XCode14 and Xcode15 have std::complex libs that do not support user defined types5
if(CMAKE_CXX_COMPILER_ID MATCHES  "AppleClang")
    message(STATUS "Ignoring complex playground tests for AppleClang")

else()
    message(STATUS "Adding complex test for all other non-AppleClang environments")
    compile_all("true" "playground" "Playground" "${COMPLEX_SRCS}")

endif()
