file (GLOB SOURCES "./*.cpp")

# exclude AppleClang as XCode14 and Xcode15 have std::complex libs that do not support user defined types5
if(BUILD_COMPLEX)
	message(STATUS "Adding complex test for Application environment")
	compile_all("true" "complex" "Complex/Applications/Performance" "${SOURCES}")
else(BUILD_COMPLEX)
	message(STATUS "Ignoring complex tests for application/performance")
endif(BUILD_COMPLEX)
