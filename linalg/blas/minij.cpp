// minij.cpp: Minimum IJ matrix
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// enable the following define to show the intermediate steps in the fused-dot product
// #define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_MUL
#define QUIRE_TRACE_ADD
// configure posit environment
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <blas/blas.hpp>
#include <blas/generators/minij.hpp>

template<typename Scalar>
void MinIJMatrixTest(size_t N = 5) {
	using namespace sw::blas;

	std::cout << "MinIJ MatrixTest for type: " << typeid(Scalar).name() << '\n';
	auto M = minij<Scalar>(N);

	// normalize the column vectors
	auto total = sumOfElements(M);
	std::cout << "Total    : " << total << '\n';
	auto rowSums = sumOfElements(M, 1);
	std::cout << "Row sums : " << rowSums << '\n';
	auto colSums = sumOfElements(M, 2);
	std::cout << "Col sums : " << colSums << '\n';
}

int main(int argc, char* argv[])
try {
	using namespace sw::universal;

	if (argc == 1) std::cout << argv[0] << std::endl;

	MinIJMatrixTest< float >();
	MinIJMatrixTest< posit<32, 2> >();

	return EXIT_SUCCESS;
}
// LCOV_EXCL_START
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
// LCOV_EXCL_STOP
