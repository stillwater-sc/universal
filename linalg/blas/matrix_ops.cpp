// matrix_ops.cpp: regression tests for matrix operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
// pull in the number systems you would like to use
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/integer/integer.hpp>

#include <blas/blas.hpp>
#include <blas/generators.hpp>
#include <blas/ext/posit_fused_blas.hpp>
#include <blas/utes/matnorm.hpp>
#include <blas/vmath/power.hpp>
#include <blas/vmath/sqrt.hpp>
#include <blas/vmath/square.hpp>
#include <blas/vmath/trigonometry.hpp>
#include <universal/verification/test_suite.hpp>

using namespace sw::numeric::containers;

namespace sw { namespace blas {

	////////////////////////////////////////////////////////////////////////
	// Test matrix transpose
	template<typename Scalar>
	int VerifyTranspose(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test square matrix transpose
		{
			matrix<Scalar> A = row_order_index<Scalar>(4, 4);
			matrix<Scalar> B(A);
			A.transpose().transpose();
			if (A != B) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: double transpose should return original matrix\n";
			}
		}

		// Test rectangular matrix transpose
		{
			matrix<Scalar> A = row_order_index<Scalar>(3, 5);
			matrix<Scalar> At = A;
			At.transpose();

			if (num_rows(At) != 5 || num_cols(At) != 3) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: transpose dimensions incorrect\n";
			}

			// Verify A[i][j] == At[j][i]
			for (size_t i = 0; i < num_rows(A); ++i) {
				for (size_t j = 0; j < num_cols(A); ++j) {
					if (A[i][j] != At[j][i]) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: transpose value mismatch at [" << i << "][" << j << "]\n";
						break;
					}
				}
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test matrix-vector product
	template<typename Scalar>
	int VerifyMatrixVectorProduct(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test identity matrix * vector = vector
		{
			matrix<Scalar> I(4, 4);
			I = 1;  // Set to identity
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4) };
			vector<Scalar> b = I * x;

			for (size_t i = 0; i < size(x); ++i) {
				if (b[i] != x[i]) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: I * x != x at index " << i << "\n";
					break;
				}
			}
		}

		// Test known matrix-vector product
		{
			matrix<Scalar> A(2, 2);
			A[0][0] = Scalar(1); A[0][1] = Scalar(2);
			A[1][0] = Scalar(3); A[1][1] = Scalar(4);
			vector<Scalar> x = { Scalar(1), Scalar(1) };
			vector<Scalar> b = A * x;

			// Expected: b = [1+2, 3+4] = [3, 7]
			if (double(b[0]) != 3.0 || double(b[1]) != 7.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: A * x = [" << b[0] << ", " << b[1] << "] (expected [3, 7])\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test matrix-matrix product
	template<typename Scalar>
	int VerifyMatrixMatrixProduct(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test identity * A = A
		{
			matrix<Scalar> I(3, 3);
			I = 1;  // Set to identity
			matrix<Scalar> A = row_order_index<Scalar>(3, 3);
			matrix<Scalar> B = I * A;

			if (A != B) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: I * A != A\n";
			}
		}

		// Test A * I = A
		{
			matrix<Scalar> I(3, 3);
			I = 1;
			matrix<Scalar> A = row_order_index<Scalar>(3, 3);
			matrix<Scalar> B = A * I;

			if (A != B) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: A * I != A\n";
			}
		}

		// Test known 2x2 multiplication
		{
			matrix<Scalar> A(2, 2), B(2, 2);
			A[0][0] = Scalar(1); A[0][1] = Scalar(2);
			A[1][0] = Scalar(3); A[1][1] = Scalar(4);
			B[0][0] = Scalar(5); B[0][1] = Scalar(6);
			B[1][0] = Scalar(7); B[1][1] = Scalar(8);

			matrix<Scalar> C = A * B;

			// Expected: C[0][0] = 1*5 + 2*7 = 19, C[0][1] = 1*6 + 2*8 = 22
			//           C[1][0] = 3*5 + 4*7 = 43, C[1][1] = 3*6 + 4*8 = 50
			if (double(C[0][0]) != 19.0 || double(C[0][1]) != 22.0 ||
			    double(C[1][0]) != 43.0 || double(C[1][1]) != 50.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 2x2 matrix multiplication incorrect\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test fused matrix operations (posit-specific)
	template<unsigned nbits, unsigned es>
	int VerifyFusedMatrixOps(bool reportTestCases) {
		using Scalar = sw::universal::posit<nbits, es>;
		int nrOfFailedTests = 0;

		// Test fmv (fused matrix-vector)
		{
			matrix<Scalar> A(3, 3);
			A = 1;  // Identity
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> b = fmv(A, x);

			for (size_t i = 0; i < size(x); ++i) {
				if (b[i] != x[i]) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: fmv(I, x) != x\n";
					break;
				}
			}
		}

		// Test fmm (fused matrix-matrix)
		{
			matrix<Scalar> A(2, 2), B(2, 2);
			A = 1;  // Identity
			B[0][0] = Scalar(1); B[0][1] = Scalar(2);
			B[1][0] = Scalar(3); B[1][1] = Scalar(4);

			matrix<Scalar> C = fmm(A, B);

			if (C != B) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: fmm(I, B) != B\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test operators.hpp functions
	template<typename Scalar>
	int VerifyOperators(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test eye() - identity matrix
		{
			matrix<Scalar> I = eye<matrix<Scalar>>(4);
			for (size_t i = 0; i < 4; ++i) {
				for (size_t j = 0; j < 4; ++j) {
					Scalar expected = (i == j) ? Scalar(1) : Scalar(0);
					if (I[i][j] != expected) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: eye() incorrect at [" << i << "][" << j << "]\n";
					}
				}
			}
		}

		// Test diag() - extract diagonal
		{
			matrix<Scalar> A(3, 3);
			A[0][0] = Scalar(1); A[0][1] = Scalar(2); A[0][2] = Scalar(3);
			A[1][0] = Scalar(4); A[1][1] = Scalar(5); A[1][2] = Scalar(6);
			A[2][0] = Scalar(7); A[2][1] = Scalar(8); A[2][2] = Scalar(9);

			vector<Scalar> d = diag(A);
			if (double(d[0]) != 1.0 || double(d[1]) != 5.0 || double(d[2]) != 9.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: diag() extraction incorrect\n";
			}
		}

		// Test tril() - lower triangular
		{
			matrix<Scalar> A(3, 3);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					A[i][j] = Scalar(i * 3 + j + 1);
				}
			}

			matrix<Scalar> L = tril(A);
			// Check upper triangle is zero
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = i + 1; j < 3; ++j) {
					if (double(L[i][j]) != 0.0) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: tril() upper triangle not zero\n";
					}
				}
			}
			// Check lower triangle matches original
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j <= i; ++j) {
					if (L[i][j] != A[i][j]) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: tril() lower triangle mismatch\n";
					}
				}
			}
		}

		// Test triu() - upper triangular
		{
			matrix<Scalar> A(3, 3);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					A[i][j] = Scalar(i * 3 + j + 1);
				}
			}

			matrix<Scalar> U = triu(A);
			// Check lower triangle is zero
			for (size_t i = 1; i < 3; ++i) {
				for (size_t j = 0; j < i; ++j) {
					if (double(U[i][j]) != 0.0) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: triu() lower triangle not zero\n";
					}
				}
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test norm functions
	template<typename Scalar>
	int VerifyNorm(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test vector 2-norm
		{
			vector<Scalar> v = { Scalar(3), Scalar(4) };
			Scalar n = norm(v, 2);
			// sqrt(3^2 + 4^2) = sqrt(25) = 5
			if (std::abs(double(n) - 5.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 2-norm of [3,4] = " << n << " (expected 5)\n";
			}
		}

		// Test vector 1-norm
		{
			vector<Scalar> v = { Scalar(-3), Scalar(4) };
			Scalar n = norm(v, 1);
			// |−3| + |4| = 7
			if (std::abs(double(n) - 7.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 1-norm of [-3,4] = " << n << " (expected 7)\n";
			}
		}

		// Test vector inf-norm
		{
			vector<Scalar> v = { Scalar(-5), Scalar(3), Scalar(4) };
			Scalar n = norm(v, std::numeric_limits<int>::max());  // inf-norm
			// max(|−5|, |3|, |4|) = 5
			if (std::abs(double(n) - 5.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: inf-norm of [-5,3,4] = " << n << " (expected 5)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test matnorm function (matrix norms)
	template<typename Scalar>
	int VerifyMatnorm(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Create test matrix:
		// A = [[1, 2, 3],
		//      [4, 5, 6]]
		matrix<Scalar> A(2, 3);
		A[0][0] = Scalar(1); A[0][1] = Scalar(2); A[0][2] = Scalar(3);
		A[1][0] = Scalar(4); A[1][1] = Scalar(5); A[1][2] = Scalar(6);

		// Test 1-norm (max column sum of absolute values)
		// Col sums: |1|+|4|=5, |2|+|5|=7, |3|+|6|=9
		// Max = 9
		{
			Scalar n1 = matnorm(A, 1);
			if (std::abs(double(n1) - 9.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: matnorm(A, 1) = " << n1 << " (expected 9)\n";
			}
		}

		// Test inf-norm (max row sum of absolute values)
		// Row sums: |1|+|2|+|3|=6, |4|+|5|+|6|=15
		// Max = 15
		{
			Scalar ninf = matnorm(A, 2);  // p != 1 gives inf-norm
			if (std::abs(double(ninf) - 15.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: matnorm(A, inf) = " << ninf << " (expected 15)\n";
			}
		}

		// Test with negative values
		{
			matrix<Scalar> B(2, 2);
			B[0][0] = Scalar(-1); B[0][1] = Scalar(2);
			B[1][0] = Scalar(-3); B[1][1] = Scalar(4);

			// 1-norm: col sums = |-1|+|-3|=4, |2|+|4|=6, max = 6
			Scalar n1 = matnorm(B, 1);
			if (std::abs(double(n1) - 6.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: matnorm with negatives 1-norm = " << n1 << " (expected 6)\n";
			}

			// inf-norm: row sums = |-1|+|2|=3, |-3|+|4|=7, max = 7
			Scalar ninf = matnorm(B, 0);
			if (std::abs(double(ninf) - 7.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: matnorm with negatives inf-norm = " << ninf << " (expected 7)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test vmath functions (vectorized math operations)
	template<typename Scalar>
	int VerifyVmath(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test sqrt
		{
			vector<Scalar> v = { Scalar(1), Scalar(4), Scalar(9), Scalar(16) };
			vector<Scalar> result = sqrt(v);
			// Expected: [1, 2, 3, 4]
			if (std::abs(double(result[0]) - 1.0) > 0.001 ||
			    std::abs(double(result[1]) - 2.0) > 0.001 ||
			    std::abs(double(result[2]) - 3.0) > 0.001 ||
			    std::abs(double(result[3]) - 4.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: sqrt incorrect\n";
			}
		}

		// Test square
		{
			vector<Scalar> v = { Scalar(1), Scalar(2), Scalar(3), Scalar(4) };
			vector<Scalar> result = square(v);
			// Expected: [1, 4, 9, 16]
			if (std::abs(double(result[0]) - 1.0) > 0.001 ||
			    std::abs(double(result[1]) - 4.0) > 0.001 ||
			    std::abs(double(result[2]) - 9.0) > 0.001 ||
			    std::abs(double(result[3]) - 16.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: square incorrect\n";
			}
		}

		// Test power
		{
			Scalar base = Scalar(2);
			vector<Scalar> exponents = { Scalar(0), Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> result = power(base, exponents);
			// Expected: [1, 2, 4, 8]
			if (std::abs(double(result[0]) - 1.0) > 0.001 ||
			    std::abs(double(result[1]) - 2.0) > 0.001 ||
			    std::abs(double(result[2]) - 4.0) > 0.001 ||
			    std::abs(double(result[3]) - 8.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: power incorrect\n";
			}
		}

		// Test sin
		{
			const double pi = 3.14159265358979323846;
			vector<Scalar> radians = { Scalar(0), Scalar(pi/6), Scalar(pi/2), Scalar(pi) };
			vector<Scalar> result = sin(radians);
			// Expected: [0, 0.5, 1, 0]
			if (std::abs(double(result[0]) - 0.0) > 0.001 ||
			    std::abs(double(result[1]) - 0.5) > 0.001 ||
			    std::abs(double(result[2]) - 1.0) > 0.001 ||
			    std::abs(double(result[3]) - 0.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: sin incorrect\n";
			}
		}

		// Test cos
		{
			const double pi = 3.14159265358979323846;
			vector<Scalar> radians = { Scalar(0), Scalar(pi/3), Scalar(pi/2), Scalar(pi) };
			vector<Scalar> result = cos(radians);
			// Expected: [1, 0.5, 0, -1]
			if (std::abs(double(result[0]) - 1.0) > 0.001 ||
			    std::abs(double(result[1]) - 0.5) > 0.001 ||
			    std::abs(double(result[2]) - 0.0) > 0.001 ||
			    std::abs(double(result[3]) - (-1.0)) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: cos incorrect\n";
			}
		}

		// Test tan
		{
			const double pi = 3.14159265358979323846;
			vector<Scalar> radians = { Scalar(0), Scalar(pi/4) };
			vector<Scalar> result = tan(radians);
			// Expected: [0, 1]
			if (std::abs(double(result[0]) - 0.0) > 0.001 ||
			    std::abs(double(result[1]) - 1.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: tan incorrect\n";
			}
		}

		return nrOfFailedTests;
	}

}} // namespace sw::blas

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::blas;

	std::string test_suite  = "BLAS matrix operations";
	std::string test_tag    = "matrix_ops";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual testing code here
	reportTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyTranspose<double>(reportTestCases), "double", "transpose");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// Test transpose with different types
	nrOfFailedTestCases += ReportTestResult(VerifyTranspose<float>(reportTestCases), "float", "transpose");
	nrOfFailedTestCases += ReportTestResult(VerifyTranspose<double>(reportTestCases), "double", "transpose");

	// Test matrix-vector product
	nrOfFailedTestCases += ReportTestResult(VerifyMatrixVectorProduct<float>(reportTestCases), "float", "matrix-vector");
	nrOfFailedTestCases += ReportTestResult(VerifyMatrixVectorProduct<double>(reportTestCases), "double", "matrix-vector");

	// Test matrix-matrix product
	nrOfFailedTestCases += ReportTestResult(VerifyMatrixMatrixProduct<float>(reportTestCases), "float", "matrix-matrix");
	nrOfFailedTestCases += ReportTestResult(VerifyMatrixMatrixProduct<double>(reportTestCases), "double", "matrix-matrix");

	// Test operators.hpp functions
	nrOfFailedTestCases += ReportTestResult(VerifyOperators<float>(reportTestCases), "float", "operators");
	nrOfFailedTestCases += ReportTestResult(VerifyOperators<double>(reportTestCases), "double", "operators");

	// Test norm functions
	nrOfFailedTestCases += ReportTestResult(VerifyNorm<float>(reportTestCases), "float", "norm");
	nrOfFailedTestCases += ReportTestResult(VerifyNorm<double>(reportTestCases), "double", "norm");

	// Test matnorm functions
	nrOfFailedTestCases += ReportTestResult(VerifyMatnorm<float>(reportTestCases), "float", "matnorm");
	nrOfFailedTestCases += ReportTestResult(VerifyMatnorm<double>(reportTestCases), "double", "matnorm");

	// Test vmath functions
	nrOfFailedTestCases += ReportTestResult(VerifyVmath<float>(reportTestCases), "float", "vmath");
	nrOfFailedTestCases += ReportTestResult(VerifyVmath<double>(reportTestCases), "double", "vmath");
#endif

#if REGRESSION_LEVEL_2
	// Test with posit types
	nrOfFailedTestCases += ReportTestResult(VerifyTranspose<posit<32, 2>>(reportTestCases), "posit<32,2>", "transpose");
	nrOfFailedTestCases += ReportTestResult(VerifyMatrixVectorProduct<posit<32, 2>>(reportTestCases), "posit<32,2>", "matrix-vector");
	nrOfFailedTestCases += ReportTestResult(VerifyMatrixMatrixProduct<posit<32, 2>>(reportTestCases), "posit<32,2>", "matrix-matrix");
	nrOfFailedTestCases += ReportTestResult(VerifyOperators<posit<32, 2>>(reportTestCases), "posit<32,2>", "operators");

	// Test fused operations
	nrOfFailedTestCases += ReportTestResult(VerifyFusedMatrixOps<16, 2>(reportTestCases), "posit<16,2>", "fused matrix ops");
	nrOfFailedTestCases += ReportTestResult(VerifyFusedMatrixOps<32, 2>(reportTestCases), "posit<32,2>", "fused matrix ops");
#endif

#if REGRESSION_LEVEL_3
	// Test with larger matrices
	nrOfFailedTestCases += ReportTestResult(VerifyTranspose<posit<16, 1>>(reportTestCases), "posit<16,1>", "transpose");
	nrOfFailedTestCases += ReportTestResult(VerifyFusedMatrixOps<8, 0>(reportTestCases), "posit<8,0>", "fused matrix ops");
#endif

#if REGRESSION_LEVEL_4
	// Stress tests with bfloat16
	using bfloat_t = cfloat<16, 8, uint16_t, true, true, false>;
	nrOfFailedTestCases += ReportTestResult(VerifyTranspose<bfloat_t>(reportTestCases), "bfloat16", "transpose");
	nrOfFailedTestCases += ReportTestResult(VerifyMatrixVectorProduct<bfloat_t>(reportTestCases), "bfloat16", "matrix-vector");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
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
// LCOV_EXCL_STOP
