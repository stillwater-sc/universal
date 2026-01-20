# CMakeLists.txt: test suite for sw::universal::complex<T>
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT

message(STATUS "Adding regression suite for sw::universal::complex<T>")

file (GLOB API_SRC "./api/*.cpp")
file (GLOB ARITHMETIC_SRC "./arithmetic/*.cpp")
file (GLOB MATH_SRC "./math/*.cpp")

compile_all("true" "complex" "Number Systems/static/complex/api" "${API_SRC}")
compile_all("true" "complex" "Number Systems/static/complex/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "complex" "Number Systems/static/complex/math" "${MATH_SRC}")
