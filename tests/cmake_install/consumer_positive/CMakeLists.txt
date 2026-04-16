cmake_minimum_required(VERSION 3.18)

# Deliberately do not set CMAKE_CXX_STANDARD. The Universal imported
# target must propagate cxx_std_20 on its own. If a future regression
# makes the installed universal-config.cmake mutate the consumer's
# global standard (the issue #708 failure mode) the C++20 concept in
# consumer.cpp is still compilable -- but the consumer's own code would
# be silently downgraded, which is the bug we want to catch here. The
# concept keeps the test honest about the standard being at least 20.
project(universal_install_smoke_consumer LANGUAGES CXX)

find_package(universal CONFIG REQUIRED)

add_executable(consumer consumer.cpp)
target_link_libraries(consumer PRIVATE universal::universal)
