file (GLOB SOURCES "./*.cpp")

# Compiler specific environments
# message(STATUS "CMAKE_CXX_COMPILER ID is -${CMAKE_CXX_COMPILER_ID}-")

# exclude AppleClang as XCode14 and Xcode15 have std::complex libs that do not support user defined types5
if(CMAKE_CXX_COMPILER_ID MATCHES  "AppleClang")
    message(STATUS "Ignoring complex tests for AppleClang")

else()
    message(STATUS "Adding complex test for all other non-AppleClang environments")
    compile_all("true" "complex" "Applications/Performance/Complex" "${SOURCES}")

endif()
