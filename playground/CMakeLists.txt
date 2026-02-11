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
	 closure_of_8bit_systems.cpp
	 generate_a_closure_map.cpp 
	 evaluate_small_number_systems.cpp
	)

set(ET_SRCS expression_templates.cpp
	 lazy_evaluation.cpp
	 #lazy_evaluation_2.cpp
)

compile_all("true" "play" "Playground/Integration" "${EXAMPLE_SRCS}")
compile_all("true" "play" "Playground/Expression Templates" "${ET_SRCS}")

# NOTE: AppleClang as XCode14 and Xcode15 have std::complex libs that do not support user defined types5
if(UNIVERSAL_BUILD_COMPLEX)
    message(STATUS "Adding playground complex experiment")
    compile_all("true" "play" "Playground/Complex" "${COMPLEX_SRCS}")
else(UNIVERSAL_BUILD_COMPLEX)
    message(STATUS "Removing complex environment experiment in the Playground")
endif(UNIVERSAL_BUILD_COMPLEX)

if(UNIVERSAL_BUILD_CONCURRENCY)
    message(STATUS "Adding playground concurrency experiment")
    if(MSVC)
        # MSVC has built-in support for C++17 parallel execution policies
        message(STATUS "MSVC: using built-in parallel execution support")
        compile_all("true" "play" "Playground/Concurrency" "${CONCURRENCY_SRCS}")
    else()
        # GCC and Clang require TBB for parallel execution policies
        find_package(TBB QUIET)
        if(TBB_FOUND)
            message(STATUS "TBB found: linking to concurrency experiment")
            compile_all("true" "play" "Playground/Concurrency" "${CONCURRENCY_SRCS}")
            target_link_libraries(play_concurrency TBB::tbb)
        else()
            message(WARNING "TBB not found: concurrency experiment requires TBB for parallel execution policies")
            message(STATUS "Install TBB: sudo apt install libtbb-dev (Debian/Ubuntu) or brew install tbb (macOS)")
        endif()
    endif()
else(UNIVERSAL_BUILD_CONCURRENCY)
    message(STATUS "Removing concurrency experiment in the Playground")
endif(UNIVERSAL_BUILD_CONCURRENCY)
