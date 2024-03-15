// matrix_ops.cpp: matrix API for sw::universal::blas
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// pull in the number systems you would like to use
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/edecimal/edecimal.hpp>

#define BLAS_POSIT_FDP_ENABLED 0
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>

/*
 * In the posit number system, the quire is used to create a reproducible fused dot product.
 * In the Universal BLAS library, the fused dot product is conditionally compiled for posits.
 * The compilation guard is BLAS_POSIT_FDP_ENABLED.
 * 
 * For comparison studies, a non-fused dot product matrix multiplication is also provided.
 */
template<typename Scalar>
void FdpTest() {
	using namespace sw::universal::blas;
	std::cout << "Fused DOT product BLAS when posits are used\n";
	using Matrix = sw::universal::blas::matrix<Scalar>;
	using Vector = sw::universal::blas::vector<Scalar>;
	constexpr unsigned M = 15;
	constexpr unsigned N = M;
	Matrix A = row_order_index<Scalar>(M, N);
	Vector x(M, 1);
	
	auto b = A * x;
	std::cout << b << '\n';
}

int main(int argc, char* argv[])
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;
	using namespace sw::universal;

	// generate a test matrix
	unsigned M = 5, N = 5;
	double mean = 0.0, stddev = 1.0;
	matrix<double> testA = gaussian_random_matrix<double>(M, N, mean, stddev);
	vector<double> testx(M, 1);
	vector<double> testb = testA * testx;

	std::cout << "Matrix A\n" << testA << '\n';
	std::cout << "Vector x\n" << testx << '\n';
	std::cout << "Vector b\n" << testb << '\n';

	{
		using Scalar = posit<16, 2>;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		using Vector = sw::universal::blas::vector<Scalar>;
		Matrix A(testA);
		Vector x(testx);

		auto b = A * x;  // optionally use the fused dot product when compiled with BLAS_POSIT_FDP_ENABLED
		std::cout << "Vector b\n" << b << '\n';
		auto c = fmv(A, x);
		std::cout << "Vector c\n" << c << '\n';
		auto d = norm(b - c, 2);  // 2-norm of the difference
		std::cout << "norm(b - c, 2) = " << d << '\n';
	}


	{
		using Scalar = float;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(23, 57);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			std::cout << "transpose FAIL\n";
		}
		else {
			std::cout << "transpose PASS\n";
		}
	}

	{
		using Scalar = sw::universal::posit<256,5>;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(117, 253);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			std::cout << "transpose FAIL\n";
		}
		else {
			std::cout << "transpose PASS\n";
		}
	}

	{
		using Scalar = sw::universal::integer<8192>;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(253, 771);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			std::cout << "transpose FAIL\n";
		}
		else {
			std::cout << "transpose PASS\n";
		}
	}

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
