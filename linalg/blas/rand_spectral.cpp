// rand_spectral.cpp: random matrix with a given spectrum
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
#include <blas/generators.hpp>


int main(int argc, char* argv[])
try {
	using namespace sw::blas;

	if (argc == 1) std::cout << argv[0] << std::endl;

	// Av = lambda * v
	// AQ = Q * Sigma
	// A  = Q * Sigma * Q^-1
	
	// generate a random base matrix for Q
	using Scalar = float;
	using Matrix = matrix<Scalar>;
	constexpr size_t M = 5;
	constexpr size_t N = 2;
	Matrix Qbase = uniform_random_matrix<Scalar>(M, N);
	std::cout << Qbase << '\n';

	// normalize the column vectors
	auto total = sumOfElements(Qbase); // default is dim = 0
	std::cout << "Total    : " << total << '\n';
	auto rowSums = sumOfElements(Qbase, 1);
	std::cout << "Row sums : " << rowSums << '\n';
	auto colSums = sumOfElements(Qbase, 2);
	std::cout << "Col sums : " << colSums << '\n';

	normalize(Qbase, 2);  // normalize columns so they are unit length
	std::cout << Qbase << '\n';
	auto colNorms = matrixNorm(Qbase, 2);
	std::cout << "Col norms: " << colNorms << '\n';

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
