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

#include <blas/blas.hpp>
#include <blas/generators.hpp>
// enable/disable type specific BLAS algorithm overloads
// for this compilation unit
#define BLAS_POSIT_FDP_OVERRIDE_ENABLED 0
#if BLAS_POSIT_FDP_OVERRIDE_ENABLED
// overload operator*() to use reproducible algorithms that leverage the quire
#include <blas/modifiers/posit_linalg_operator_overload.hpp>
#endif
#include <blas/ext/posit_fused_blas.hpp>   // addition of fdp, fmv, and fmm functions

/*
 * In the posit number system, the quire is used to create a reproducible fused dot product.
 * For the Universal BLAS library, by including <universal/blas/modifiers/posit_fdp.hpp>,
 * the default non-reproducible dot, matvec, and matmul operators are overloaded with
 * reproducible algorithms. Expressions, such as a matrix-vector product expressed 
 * through operator*(),
 * 
 * auto b = A * x;
 * 
 * will change to be reproducible computations when the BLAS modifier header is included.
 * 
 * The posit API also provides explicit reproducible algorithms:
 *   fdp - fused dot product
 *   fmv - fused matrix-vector product
 *   fmm - fused matrix-matrix product
 */

template<typename Scalar>
void TestReproducibleMatvec(sw::universal::blas::matrix<double>& testA, sw::universal::blas::vector<double>& testx) 
{
	using Matrix = sw::numeric::containers::matrix<Scalar>;
	using Vector = sw::numeric::containers::vector<Scalar>;
	Matrix A(testA);
	Vector x(testx);

	auto b = A * x;  // optionally use the fused dot product when compiled with BLAS_POSIT_FDP_ENABLED
	std::cout << "Matrix-Vector product b\n" << b << '\n';
	auto c = sw::blas::fmv(A, x);
	std::cout << "Reproducible Matrix-Vector c\n" << c << '\n';
	auto d = sw::blas::norm(b - c, 2);  // 2-norm of the difference: if we enable the overload, the difference becomes 0
	std::cout << "norm(b - c, 2) = " << d << '\n';
}

template<typename Scalar>
void TestReproducibleMatmul(sw::universal::blas::matrix<double>& testA, sw::universal::blas::matrix<double>& testB)
{
	using Matrix = sw::numeric::containers::matrix<Scalar>;
	//using Vector = sw::numeric::containers::vector<Scalar>;
	Matrix A(testA), B(testB);

	auto C = A * B;  // optionally use the fused dot product when compiled with BLAS_POSIT_FDP_ENABLED
	std::cout << "Matrix product C\n" << C << '\n';
	auto C2 = sw::blas::fmm(A, B);
	std::cout << "Reproducible matmul C2\n" << C2 << '\n';
	// TDB: we need matrix norm functionality here
	auto d = C(0, 0) - C2(0, 0);  // 2-norm of the difference: if we enable the overload, the difference becomes 0
	std::cout << "C(0,0) - C2(0,0) = " << d << '\n';
}

template<typename Scalar = float>
void TestTranspose(unsigned M, unsigned N) {
	using Matrix = sw::numeric::containers::matrix<Scalar>;
	Matrix A = sw::blas::row_order_index<Scalar>(M, N);
	Matrix B(A);
	A.transpose().transpose();
	if (A != B) {
		std::cout << "transpose FAIL\n";
	}
	else {
		std::cout << "transpose PASS\n";
	}
}

int main(int argc, char* argv[])
try {
	using namespace sw::universal;
	using namespace sw::blas;

	// generate a test matrix
	unsigned M = 5, N = 5;
	double mean = 0.0, stddev = 1.0;
	matrix<double> testA = gaussian_random_matrix<double>(M, N, mean, stddev);
	matrix<double> testB = gaussian_random_matrix<double>(M, N, mean, stddev);
	vector<double> testx(M, 1);
	vector<double> testb = testA * testx;

	std::cout << "Matrix A\n" << testA << '\n';
	std::cout << "Vector x\n" << testx << '\n';
	std::cout << "Vector b\n" << testb << '\n';

	TestReproducibleMatvec< posit<16, 2> >(testA, testx);
	TestReproducibleMatmul< posit<16, 2> >(testA, testB);

	TestTranspose(23, 57);  // default type is native float
	TestTranspose< posit<256, 5> >(117, 253);
	TestTranspose< integer<8192> >(253, 771);

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
