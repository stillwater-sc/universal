file (GLOB SOURCES "./*.cpp")

# compute.cpp uses sw::universal::complex<T>, not std::complex<T> (#2033)
if(UNIVERSAL_BUILD_COMPLEX)
	message(STATUS "Adding complex test for Application environment")
	compile_all("true" "complex" "Complex/Applications/Performance" "${SOURCES}")
else(UNIVERSAL_BUILD_COMPLEX)
	message(STATUS "Ignoring complex tests for application/performance")
endif(UNIVERSAL_BUILD_COMPLEX)
