// generators.cpp: matrix generator examples
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

template<typename Scalar>
void generateMatrices() {
	using namespace sw::blas;
	using Matrix = sw::numeric::containers::matrix<Scalar>;

	Matrix A(5, 5);
	// create an Identity matrix
	A = 1;
	std::cout << "Identity\n" << A << std::endl;

	// create a 2D Laplacian
	laplace2D(A, 5, 5);
	std::cout << "Laplace-2D\n" << A << '\n';

	// create a row order index matrix
	Matrix roi = row_order_index<Scalar>(5, 6);
	std::cout << "Row order index\n" << roi << '\n';

	// create a column order index matrix
	Matrix coi = column_order_index<Scalar>(6,5);
	std::cout << "Column order index\n" << coi << '\n';

	// create a min-ij matrix
	Matrix mij(9, 9);
	minij(mij);
	std::cout << "Min-ij\n" << mij << '\n';

	// create a magic square matrix
	Matrix ms = magic<Scalar>(5);
	std::cout << "Magic Square\n" << ms << '\n';

	// create a uniform random matrix
	Matrix uniform(10, 10);
	uniform_random(uniform, -1.0, 1.0);
	std::cout << "Uniform random\n" << std::setprecision(5) << std::setw(10) << uniform << '\n';

	// create a uniform random matrix
	Matrix gaussian(10, 10);
	gaussian_random(gaussian, -1.0, 1.0);
	std::cout << "Gaussian Random\n" << std::setprecision(5) << std::setw(10) << gaussian << '\n';
}

int main(int argc, char* argv[])
try {
	using namespace sw::universal;
	using namespace sw::blas;

	if (argc > 0) std::cout << argv[0] << std::endl;

	generateMatrices< posit< 8, 0> >();
	generateMatrices< posit<16, 1> >();
	generateMatrices< posit<32, 2> >();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
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
