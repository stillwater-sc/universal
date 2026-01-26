// generators.cpp: regression tests for matrix generators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>

// configure posit environment
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>

#include <blas/blas.hpp>
#include <blas/generators.hpp>
#include <universal/verification/test_suite.hpp>

using namespace sw::numeric::containers;

namespace sw { namespace blas {

	////////////////////////////////////////////////////////////////////////
	// Test identity matrix generator (matrix = 1)
	template<typename Scalar>
	int VerifyIdentityGenerator(bool reportTestCases) {
		int nrOfFailedTests = 0;

		matrix<Scalar> A(4, 4);
		A = 1;  // Set to identity

		for (size_t i = 0; i < 4; ++i) {
			for (size_t j = 0; j < 4; ++j) {
				Scalar expected = (i == j) ? Scalar(1) : Scalar(0);
				if (A[i][j] != expected) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: identity at [" << i << "][" << j << "]\n";
				}
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test row_order_index generator
	template<typename Scalar>
	int VerifyRowOrderIndex(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test square matrix (default start=1)
		{
			matrix<Scalar> A = row_order_index<Scalar>(3, 3);
			// Expected: [[1,2,3], [4,5,6], [7,8,9]] (starts at 1)
			int expected = 1;
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					if (double(A[i][j]) != double(expected)) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: row_order_index at [" << i << "][" << j << "] = " << A[i][j] << " (expected " << expected << ")\n";
					}
					++expected;
				}
			}
		}

		// Test rectangular matrix
		{
			matrix<Scalar> A = row_order_index<Scalar>(2, 4);
			if (num_rows(A) != 2 || num_cols(A) != 4) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: row_order_index dimensions\n";
			}
			// First row should be [1,2,3,4] (starts at 1)
			if (double(A[0][0]) != 1.0 || double(A[0][3]) != 4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: row_order_index rectangular values\n";
			}
		}

		// Test with custom start value
		{
			matrix<Scalar> A = row_order_index<Scalar>(2, 2, Scalar(0));
			// With start=0: [[0,1], [2,3]]
			if (double(A[0][0]) != 0.0 || double(A[1][1]) != 3.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: row_order_index with start=0\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test column_order_index generator
	template<typename Scalar>
	int VerifyColumnOrderIndex(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test square matrix (default start=1)
		{
			matrix<Scalar> A = column_order_index<Scalar>(3, 3);
			// Expected (column-major fill, start=1): [[1,4,7], [2,5,8], [3,6,9]]
			int expected = 1;
			for (size_t j = 0; j < 3; ++j) {
				for (size_t i = 0; i < 3; ++i) {
					if (double(A[i][j]) != double(expected)) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: column_order_index at [" << i << "][" << j << "] = " << A[i][j] << " (expected " << expected << ")\n";
					}
					++expected;
				}
			}
		}

		// Test with custom start value
		{
			matrix<Scalar> A = column_order_index<Scalar>(2, 2, Scalar(0));
			// With start=0: [[0,2], [1,3]]
			if (double(A[0][0]) != 0.0 || double(A[1][0]) != 1.0 ||
			    double(A[0][1]) != 2.0 || double(A[1][1]) != 3.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: column_order_index with start=0\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test laplace2D generator
	template<typename Scalar>
	int VerifyLaplace2D(bool reportTestCases) {
		int nrOfFailedTests = 0;

		matrix<Scalar> A(4, 4);
		laplace2D(A, 4, 4);

		// Laplace2D should have:
		// - diagonal elements = 4 (for internal nodes) or smaller for boundary
		// - off-diagonal elements = -1 for neighbors, 0 otherwise

		// Check diagonal is non-negative
		for (size_t i = 0; i < 4; ++i) {
			if (double(A[i][i]) < 0.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: laplace2D diagonal should be non-negative\n";
			}
		}

		// Check symmetry
		for (size_t i = 0; i < 4; ++i) {
			for (size_t j = i + 1; j < 4; ++j) {
				if (A[i][j] != A[j][i]) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: laplace2D should be symmetric\n";
				}
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test minij generator
	template<typename Scalar>
	int VerifyMinij(bool reportTestCases) {
		int nrOfFailedTests = 0;

		matrix<Scalar> A(4, 4);
		minij(A);

		// minij: A[i][j] = min(i+1, j+1)
		for (size_t i = 0; i < 4; ++i) {
			for (size_t j = 0; j < 4; ++j) {
				int expected = std::min(i + 1, j + 1);
				if (double(A[i][j]) != double(expected)) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: minij at [" << i << "][" << j << "] = " << A[i][j] << " (expected " << expected << ")\n";
				}
			}
		}

		// Test minij factory function
		auto B = minij<Scalar>(5);
		if (num_rows(B) != 5 || num_cols(B) != 5) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: minij factory dimensions\n";
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test magic square generator
	template<typename Scalar>
	int VerifyMagicSquare(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test 3x3 magic square
		{
			matrix<Scalar> M = magic<Scalar>(3);

			// Magic constant for 3x3 = 15
			Scalar magicConstant = Scalar(15);

			// Check all rows sum to magic constant
			for (size_t i = 0; i < 3; ++i) {
				Scalar rowSum{0};
				for (size_t j = 0; j < 3; ++j) {
					rowSum += M[i][j];
				}
				if (rowSum != magicConstant) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: magic square row " << i << " sum = " << rowSum << " (expected 15)\n";
				}
			}

			// Check all columns sum to magic constant
			for (size_t j = 0; j < 3; ++j) {
				Scalar colSum{0};
				for (size_t i = 0; i < 3; ++i) {
					colSum += M[i][j];
				}
				if (colSum != magicConstant) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: magic square col " << j << " sum = " << colSum << " (expected 15)\n";
				}
			}
		}

		// Test 5x5 magic square
		{
			matrix<Scalar> M = magic<Scalar>(5);
			// Magic constant for 5x5 = 65
			Scalar magicConstant = Scalar(65);

			Scalar rowSum{0};
			for (size_t j = 0; j < 5; ++j) {
				rowSum += M[0][j];
			}
			if (rowSum != magicConstant) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 5x5 magic square row sum = " << rowSum << " (expected 65)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test uniform_random generator
	template<typename Scalar>
	int VerifyUniformRandom(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test that values are within range
		{
			matrix<Scalar> A(10, 10);
			uniform_random(A, -1.0, 1.0);

			for (size_t i = 0; i < 10; ++i) {
				for (size_t j = 0; j < 10; ++j) {
					double val = double(A[i][j]);
					if (val < -1.0 || val > 1.0) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: uniform_random value " << val << " out of range [-1,1]\n";
					}
				}
			}
		}

		// Test uniform_random_matrix factory
		{
			auto A = uniform_random_matrix<Scalar>(5, 5, 0.0, 10.0);
			if (num_rows(A) != 5 || num_cols(A) != 5) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: uniform_random_matrix dimensions\n";
			}

			// Check values are in range [0, 10]
			for (size_t i = 0; i < 5; ++i) {
				for (size_t j = 0; j < 5; ++j) {
					double val = double(A[i][j]);
					if (val < 0.0 || val > 10.0) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: uniform_random_matrix value out of range\n";
					}
				}
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test gaussian_random generator
	template<typename Scalar>
	int VerifyGaussianRandom(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Generate and check statistics
		{
			matrix<Scalar> A(50, 50);
			gaussian_random(A, 0.0, 1.0);

			// Compute mean
			Scalar sum{0};
			for (size_t i = 0; i < 50; ++i) {
				for (size_t j = 0; j < 50; ++j) {
					sum += A[i][j];
				}
			}
			Scalar mean = sum / Scalar(2500);

			// Mean should be close to 0 (within ~0.2 for 2500 samples)
			if (std::abs(double(mean)) > 0.3) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian_random mean = " << mean << " (expected ~0.0)\n";
			}
		}

		// Test gaussian_random_matrix factory
		{
			auto A = gaussian_random_matrix<Scalar>(10, 10, 100.0, 10.0);
			if (num_rows(A) != 10 || num_cols(A) != 10) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian_random_matrix dimensions\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test hilbert matrix generator
	template<typename Scalar>
	int VerifyHilbert(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test unscaled Hilbert matrix: H[i][j] = 1 / (i + j + 1) (0-indexed)
		{
			auto H = hilbert<Scalar>(4, false);  // bScale=false for standard Hilbert matrix

			// Check H[0][0] = 1
			if (std::abs(double(H[0][0]) - 1.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: hilbert[0][0] = " << H[0][0] << " (expected 1)\n";
			}

			// Check H[0][1] = 1/2
			if (std::abs(double(H[0][1]) - 0.5) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: hilbert[0][1] = " << H[0][1] << " (expected 0.5)\n";
			}

			// Check H[1][1] = 1/3
			if (std::abs(double(H[1][1]) - 1.0/3.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: hilbert[1][1] = " << H[1][1] << " (expected 0.333...)\n";
			}

			// Hilbert matrix should be symmetric
			for (size_t i = 0; i < 4; ++i) {
				for (size_t j = i + 1; j < 4; ++j) {
					if (H[i][j] != H[j][i]) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: hilbert matrix not symmetric\n";
					}
				}
			}
		}

		// Test scaled Hilbert matrix (default behavior)
		{
			auto H = hilbert<Scalar>(4);  // default bScale=true
			// Scaled matrix should have integer-representable values
			// For N=4, scale factor = LCM(2,3,4,5,6,7) = 420
			// H[0][0] = 420 * 1 = 420
			// H[0][1] = 420 * 1/2 = 210
			// H[1][1] = 420 * 1/3 = 140
			if (std::abs(double(H[0][0]) - 420.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: scaled hilbert[0][0] = " << H[0][0] << " (expected 420)\n";
			}
			if (std::abs(double(H[0][1]) - 210.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: scaled hilbert[0][1] = " << H[0][1] << " (expected 210)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test tridiag matrix generator
	template<typename Scalar>
	int VerifyTridiag(bool reportTestCases) {
		int nrOfFailedTests = 0;

		auto T = tridiag<Scalar>(5, Scalar(-1), Scalar(2), Scalar(-1));

		// Check diagonal = 2
		for (size_t i = 0; i < 5; ++i) {
			if (double(T[i][i]) != 2.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: tridiag diagonal = " << T[i][i] << " (expected 2)\n";
			}
		}

		// Check sub-diagonal = -1
		for (size_t i = 1; i < 5; ++i) {
			if (double(T[i][i-1]) != -1.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: tridiag sub-diagonal incorrect\n";
			}
		}

		// Check super-diagonal = -1
		for (size_t i = 0; i < 4; ++i) {
			if (double(T[i][i+1]) != -1.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: tridiag super-diagonal incorrect\n";
			}
		}

		// Check that other elements are zero
		if (double(T[0][2]) != 0.0 || double(T[0][3]) != 0.0 || double(T[2][0]) != 0.0) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: tridiag non-tridiagonal elements should be zero\n";
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

	std::string test_suite  = "BLAS matrix generators";
	std::string test_tag    = "generators";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	reportTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyMagicSquare<double>(reportTestCases), "double", "magic square");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// Test basic generators with double
	nrOfFailedTestCases += ReportTestResult(VerifyIdentityGenerator<double>(reportTestCases), "double", "identity");
	nrOfFailedTestCases += ReportTestResult(VerifyRowOrderIndex<double>(reportTestCases), "double", "row_order_index");
	nrOfFailedTestCases += ReportTestResult(VerifyColumnOrderIndex<double>(reportTestCases), "double", "column_order_index");
	nrOfFailedTestCases += ReportTestResult(VerifyMinij<double>(reportTestCases), "double", "minij");
	nrOfFailedTestCases += ReportTestResult(VerifyMagicSquare<double>(reportTestCases), "double", "magic");

	// Test structured generators
	nrOfFailedTestCases += ReportTestResult(VerifyLaplace2D<double>(reportTestCases), "double", "laplace2D");
	nrOfFailedTestCases += ReportTestResult(VerifyHilbert<double>(reportTestCases), "double", "hilbert");
	nrOfFailedTestCases += ReportTestResult(VerifyTridiag<double>(reportTestCases), "double", "tridiag");
#endif

#if REGRESSION_LEVEL_2
	// Test random generators
	nrOfFailedTestCases += ReportTestResult(VerifyUniformRandom<double>(reportTestCases), "double", "uniform_random");
	nrOfFailedTestCases += ReportTestResult(VerifyGaussianRandom<double>(reportTestCases), "double", "gaussian_random");

	// Test with float
	nrOfFailedTestCases += ReportTestResult(VerifyIdentityGenerator<float>(reportTestCases), "float", "identity");
	nrOfFailedTestCases += ReportTestResult(VerifyRowOrderIndex<float>(reportTestCases), "float", "row_order_index");
	nrOfFailedTestCases += ReportTestResult(VerifyMinij<float>(reportTestCases), "float", "minij");
#endif

#if REGRESSION_LEVEL_3
	// Test with posit types
	nrOfFailedTestCases += ReportTestResult(VerifyIdentityGenerator<posit<32, 2>>(reportTestCases), "posit<32,2>", "identity");
	nrOfFailedTestCases += ReportTestResult(VerifyRowOrderIndex<posit<32, 2>>(reportTestCases), "posit<32,2>", "row_order_index");
	nrOfFailedTestCases += ReportTestResult(VerifyMinij<posit<32, 2>>(reportTestCases), "posit<32,2>", "minij");
	nrOfFailedTestCases += ReportTestResult(VerifyHilbert<posit<32, 2>>(reportTestCases), "posit<32,2>", "hilbert");
	nrOfFailedTestCases += ReportTestResult(VerifyMagicSquare<posit<32, 2>>(reportTestCases), "posit<32,2>", "magic");
#endif

#if REGRESSION_LEVEL_4
	// Test with different posit configurations
	nrOfFailedTestCases += ReportTestResult(VerifyIdentityGenerator<posit<16, 1>>(reportTestCases), "posit<16,1>", "identity");
	nrOfFailedTestCases += ReportTestResult(VerifyTridiag<posit<16, 1>>(reportTestCases), "posit<16,1>", "tridiag");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
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
