#file (GLOB SOURCES "./*.cpp")

set(SECANT_SRC "./secant.cpp")
set(RPOLY_SRC "./rpoly.cpp")

compile_all("true" "mp" "Mixed-Precision/Root Finding" "${SECANT_SRC}")

if(UNIVERSAL_BUILD_COMPLEX)
    message(STATUS "adding rpoly test suite")
    compile_all("true" "mp" "Mixed-Precision/Root Finding" "${RPOLY_SRC}")
else(UNIVERSAL_BUILD_COMPLEX)
    message(STATUS "removing rpoly test suite from regression due to dependency on std::complex<>")
endif(UNIVERSAL_BUILD_COMPLEX)

