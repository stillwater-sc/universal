// randsvd.cpp: Randsvd matrix
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
#include <blas/generators/randsvd.hpp>

template<typename Scalar>
void RandsvdMatrixTest(size_t N = 5) {
	using namespace sw::universal::blas;
	using Matrix = sw::universal::blas::matrix<Scalar>;
	Matrix A(N,N), S(N, N), V(N, N), D(N, N);
	std::cout << "RandsvdMatrixTest for type: " << typeid(Scalar).name() << '\n';
	std::tie(S, V, D) = randsvd(A);
	std::cout << S << '\n';
	std::cout << V << '\n';
	std::cout << D << '\n';
}

int main(int argc, char* argv[])
try {
	using namespace sw::universal;

	if (argc == 1) std::cout << argv[0] << '\n';

	RandsvdMatrixTest< float >();
//	RandsvdMatrixTest< posit< 8, 0> >();
//	RandsvdMatrixTest< posit<16, 1> >();
//	RandsvdMatrixTest< posit<32, 2> >();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}