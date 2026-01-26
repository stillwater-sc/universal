// blas_operations.cpp: regression tests for BLAS Level 1, 2, and 3 operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <cmath>

// pull in the number systems
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>

#include <blas/blas.hpp>
#include <blas/generators.hpp>
#include <universal/verification/test_suite.hpp>

using namespace sw::numeric::containers;

namespace sw { namespace blas {

	////////////////////////////////////////////////////////////////////////
	// BLAS Level 1 Tests
	////////////////////////////////////////////////////////////////////////

	// Test asum: sum of absolute values
	template<typename Scalar>
	int VerifyAsum(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test with positive values
		{
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4) };
			Scalar result = asum(4, x, 1);
			if (std::abs(double(result) - 10.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: asum([1,2,3,4]) = " << result << " (expected 10)\n";
			}
		}

		// Test with negative values
		{
			vector<Scalar> x = { Scalar(-1), Scalar(-2), Scalar(3), Scalar(-4) };
			Scalar result = asum(4, x, 1);
			// |−1| + |−2| + |3| + |−4| = 10
			if (std::abs(double(result) - 10.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: asum([-1,-2,3,-4]) = " << result << " (expected 10)\n";
			}
		}

		// Test with stride - note: library asum uses ix < n, so n must account for stride
		{
			vector<Scalar> x = { Scalar(1), Scalar(100), Scalar(2), Scalar(100), Scalar(3) };
			// With stride 2, we access indices 0, 2, 4 - need n=5 to reach index 4
			Scalar result = asum(5, x, 2);
			// |1| + |2| + |3| = 6
			if (std::abs(double(result) - 6.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: asum with stride 2 = " << result << " (expected 6)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test sum: sum of elements
	template<typename Scalar>
	int VerifySum(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test basic sum
		{
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4), Scalar(5) };
			Scalar result = sum(x);
			if (std::abs(double(result) - 15.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: sum([1,2,3,4,5]) = " << result << " (expected 15)\n";
			}
		}

		// Test with negative values (should cancel)
		{
			vector<Scalar> x = { Scalar(5), Scalar(-3), Scalar(2), Scalar(-4) };
			Scalar result = sum(x);
			// 5 - 3 + 2 - 4 = 0
			if (std::abs(double(result) - 0.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: sum([5,-3,2,-4]) = " << result << " (expected 0)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test axpy: a*x + y
	template<typename Scalar>
	int VerifyAxpy(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Basic axpy test
		{
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> y = { Scalar(10), Scalar(20), Scalar(30) };
			Scalar a = Scalar(2);
			axpy(3, a, x, 1, y, 1);
			// y = 2*[1,2,3] + [10,20,30] = [12, 24, 36]
			if (std::abs(double(y[0]) - 12.0) > 0.001 ||
			    std::abs(double(y[1]) - 24.0) > 0.001 ||
			    std::abs(double(y[2]) - 36.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: axpy result incorrect\n";
			}
		}

		// Test with stride
		{
			vector<Scalar> x = { Scalar(1), Scalar(0), Scalar(2), Scalar(0), Scalar(3) };
			vector<Scalar> y = { Scalar(10), Scalar(0), Scalar(20), Scalar(0), Scalar(30) };
			Scalar a = Scalar(3);
			axpy(3, a, x, 2, y, 2);
			// y[0] = 3*1 + 10 = 13, y[2] = 3*2 + 20 = 26, y[4] = 3*3 + 30 = 39
			if (std::abs(double(y[0]) - 13.0) > 0.001 ||
			    std::abs(double(y[2]) - 26.0) > 0.001 ||
			    std::abs(double(y[4]) - 39.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: axpy with stride incorrect\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test copy: vector copy with stride
	template<typename Scalar>
	int VerifyCopy(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Basic copy
		{
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4) };
			vector<Scalar> y(4);
			copy(4, x, 1, y, 1);
			for (size_t i = 0; i < 4; ++i) {
				if (x[i] != y[i]) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: copy mismatch at index " << i << "\n";
					break;
				}
			}
		}

		// Copy with stride
		{
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4), Scalar(5), Scalar(6) };
			vector<Scalar> y(6);
			y = 0;
			copy(3, x, 2, y, 2);  // Copy every other element
			// y should be [1, 0, 3, 0, 5, 0]
			if (std::abs(double(y[0]) - 1.0) > 0.001 ||
			    std::abs(double(y[2]) - 3.0) > 0.001 ||
			    std::abs(double(y[4]) - 5.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: copy with stride incorrect\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test dot: dot product
	template<typename Scalar>
	int VerifyDot(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Basic dot product
		{
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> y = { Scalar(4), Scalar(5), Scalar(6) };
			Scalar result = dot(x, y);
			// 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
			if (std::abs(double(result) - 32.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: dot([1,2,3], [4,5,6]) = " << result << " (expected 32)\n";
			}
		}

		// Dot product with stride
		{
			vector<Scalar> x = { Scalar(1), Scalar(0), Scalar(2), Scalar(0), Scalar(3) };
			vector<Scalar> y = { Scalar(4), Scalar(0), Scalar(5), Scalar(0), Scalar(6) };
			Scalar result = dot(3, x, 2, y, 2);
			// 1*4 + 2*5 + 3*6 = 32
			if (std::abs(double(result) - 32.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: dot with stride = " << result << " (expected 32)\n";
			}
		}

		// Orthogonal vectors
		{
			vector<Scalar> x = { Scalar(1), Scalar(0), Scalar(0) };
			vector<Scalar> y = { Scalar(0), Scalar(1), Scalar(0) };
			Scalar result = dot(x, y);
			if (std::abs(double(result)) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: dot of orthogonal vectors = " << result << " (expected 0)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test scale: scale vector by constant
	template<typename Scalar>
	int VerifyScale(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Basic scaling
		{
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4) };
			scale(4, Scalar(2), x, 1);
			// x should be [2, 4, 6, 8]
			if (std::abs(double(x[0]) - 2.0) > 0.001 ||
			    std::abs(double(x[1]) - 4.0) > 0.001 ||
			    std::abs(double(x[2]) - 6.0) > 0.001 ||
			    std::abs(double(x[3]) - 8.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: scale result incorrect\n";
			}
		}

		// Scale by zero
		{
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3) };
			scale(3, Scalar(0), x, 1);
			for (size_t i = 0; i < 3; ++i) {
				if (std::abs(double(x[i])) > 0.001) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: scale by 0 should give zeros\n";
					break;
				}
			}
		}

		return nrOfFailedTests;
	}

	// Test swap: swap two vectors
	template<typename Scalar>
	int VerifySwap(bool reportTestCases) {
		int nrOfFailedTests = 0;

		vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3) };
		vector<Scalar> y = { Scalar(10), Scalar(20), Scalar(30) };
		swap(3, x, 1, y, 1);

		// x should now be [10, 20, 30], y should be [1, 2, 3]
		if (std::abs(double(x[0]) - 10.0) > 0.001 ||
		    std::abs(double(x[1]) - 20.0) > 0.001 ||
		    std::abs(double(x[2]) - 30.0) > 0.001) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: swap x values incorrect\n";
		}
		if (std::abs(double(y[0]) - 1.0) > 0.001 ||
		    std::abs(double(y[1]) - 2.0) > 0.001 ||
		    std::abs(double(y[2]) - 3.0) > 0.001) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: swap y values incorrect\n";
		}

		return nrOfFailedTests;
	}

	// Test amax/amin: find index of max/min absolute value
	template<typename Scalar>
	int VerifyAmaxAmin(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test amax
		{
			vector<Scalar> x = { Scalar(1), Scalar(-5), Scalar(3), Scalar(2) };
			size_t idx = amax(4, x, 1);
			// max abs value is |-5| = 5 at index 1
			if (idx != 1) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: amax index = " << idx << " (expected 1)\n";
			}
		}

		// Test amin
		{
			vector<Scalar> x = { Scalar(5), Scalar(-1), Scalar(3), Scalar(2) };
			size_t idx = amin(4, x, 1);
			// min abs value is |-1| = 1 at index 1
			if (idx != 1) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: amin index = " << idx << " (expected 1)\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test rot: Givens rotation
	template<typename Scalar>
	int VerifyRot(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test 90-degree rotation (c=0, s=1)
		{
			vector<Scalar> x = { Scalar(1), Scalar(0) };
			vector<Scalar> y = { Scalar(0), Scalar(1) };
			Scalar c = Scalar(0);  // cos(90°)
			Scalar s = Scalar(1);  // sin(90°)
			rot(2, x, 1, y, 1, c, s);
			// After rotation: x_new = c*x + s*y = 0*1 + 1*0 = 0 (first element)
			//                 y_new = c*y - s*x = 0*0 - 1*1 = -1 (first element)
			if (std::abs(double(x[0]) - 0.0) > 0.001 ||
			    std::abs(double(y[0]) - (-1.0)) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: rot 90 degrees incorrect\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test L1, L2, Linf norms
	template<typename Scalar>
	int VerifyNorms(bool reportTestCases) {
		int nrOfFailedTests = 0;

		vector<Scalar> v = { Scalar(3), Scalar(-4) };

		// L1 norm: |3| + |-4| = 7
		{
			Scalar n = normL1(v);
			if (std::abs(double(n) - 7.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: normL1 = " << n << " (expected 7)\n";
			}
		}

		// L2 norm: sqrt(9 + 16) = 5
		{
			Scalar n = normL2(v);
			if (std::abs(double(n) - 5.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: normL2 = " << n << " (expected 5)\n";
			}
		}

		// Linf norm: max(|3|, |-4|) = 4
		{
			Scalar n = normLinf(v);
			if (std::abs(double(n) - 4.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: normLinf = " << n << " (expected 4)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// BLAS Level 2 Tests
	////////////////////////////////////////////////////////////////////////

	// Test matvec: matrix-vector product
	template<typename Scalar>
	int VerifyMatvec(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test identity matrix
		{
			matrix<Scalar> I(3, 3);
			I = 1;  // Identity
			vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3) };
			vector<Scalar> b(3);
			matvec(b, I, x);
			// b should equal x
			for (size_t i = 0; i < 3; ++i) {
				if (std::abs(double(b[i]) - double(x[i])) > 0.001) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: matvec(I, x) != x\n";
					break;
				}
			}
		}

		// Test general matrix
		{
			matrix<Scalar> A(2, 3);
			A[0][0] = Scalar(1); A[0][1] = Scalar(2); A[0][2] = Scalar(3);
			A[1][0] = Scalar(4); A[1][1] = Scalar(5); A[1][2] = Scalar(6);
			vector<Scalar> x = { Scalar(1), Scalar(1), Scalar(1) };
			vector<Scalar> b(2);
			matvec(b, A, x);
			// b[0] = 1+2+3 = 6, b[1] = 4+5+6 = 15
			if (std::abs(double(b[0]) - 6.0) > 0.001 ||
			    std::abs(double(b[1]) - 15.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: matvec general case incorrect\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// BLAS Level 3 Tests
	////////////////////////////////////////////////////////////////////////

	// Test sumOfElements
	template<typename Scalar>
	int VerifySumOfElements(bool reportTestCases) {
		int nrOfFailedTests = 0;

		matrix<Scalar> A(2, 3);
		A[0][0] = Scalar(1); A[0][1] = Scalar(2); A[0][2] = Scalar(3);
		A[1][0] = Scalar(4); A[1][1] = Scalar(5); A[1][2] = Scalar(6);

		// Total sum (dim=0)
		{
			auto result = sumOfElements(A, 0);
			// 1+2+3+4+5+6 = 21
			if (std::abs(double(result[0]) - 21.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: sumOfElements total = " << result[0] << " (expected 21)\n";
			}
		}

		// Row sums (dim=1)
		{
			auto result = sumOfElements(A, 1);
			// Row 0: 1+2+3=6, Row 1: 4+5+6=15
			if (std::abs(double(result[0]) - 6.0) > 0.001 ||
			    std::abs(double(result[1]) - 15.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: sumOfElements row sums incorrect\n";
			}
		}

		// Column sums (dim=2)
		{
			auto result = sumOfElements(A, 2);
			// Col 0: 1+4=5, Col 1: 2+5=7, Col 2: 3+6=9
			if (std::abs(double(result[0]) - 5.0) > 0.001 ||
			    std::abs(double(result[1]) - 7.0) > 0.001 ||
			    std::abs(double(result[2]) - 9.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: sumOfElements column sums incorrect\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test normalize
	template<typename Scalar>
	int VerifyNormalize(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test matrix normalization (dim=0)
		{
			matrix<Scalar> A(2, 2);
			A[0][0] = Scalar(3); A[0][1] = Scalar(0);
			A[1][0] = Scalar(0); A[1][1] = Scalar(4);
			// Frobenius norm = sqrt(9 + 16) = 5
			normalize(A, 0);
			// After normalization: A[0][0] = 3/5 = 0.6, A[1][1] = 4/5 = 0.8
			if (std::abs(double(A[0][0]) - 0.6) > 0.001 ||
			    std::abs(double(A[1][1]) - 0.8) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: normalize matrix incorrect\n";
			}
		}

		return nrOfFailedTests;
	}

	// Test matrixNorm
	template<typename Scalar>
	int VerifyMatrixNorm(bool reportTestCases) {
		int nrOfFailedTests = 0;

		matrix<Scalar> A(2, 2);
		A[0][0] = Scalar(3); A[0][1] = Scalar(0);
		A[1][0] = Scalar(0); A[1][1] = Scalar(4);

		// Total Frobenius norm (dim=0): sqrt(9 + 16) = 5
		{
			auto result = matrixNorm(A, 0);
			if (std::abs(double(result[0]) - 5.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: matrixNorm total = " << result[0] << " (expected 5)\n";
			}
		}

		// Row norms (dim=1): row 0 = 3, row 1 = 4
		{
			auto result = matrixNorm(A, 1);
			if (std::abs(double(result[0]) - 3.0) > 0.001 ||
			    std::abs(double(result[1]) - 4.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: matrixNorm row norms incorrect\n";
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

	std::string test_suite  = "BLAS Level 1/2/3 operations";
	std::string test_tag    = "blas_operations";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	reportTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyDot<double>(reportTestCases), "double", "dot");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// BLAS Level 1 tests with double
	nrOfFailedTestCases += ReportTestResult(VerifyAsum<double>(reportTestCases), "double", "asum");
	nrOfFailedTestCases += ReportTestResult(VerifySum<double>(reportTestCases), "double", "sum");
	nrOfFailedTestCases += ReportTestResult(VerifyAxpy<double>(reportTestCases), "double", "axpy");
	nrOfFailedTestCases += ReportTestResult(VerifyCopy<double>(reportTestCases), "double", "copy");
	nrOfFailedTestCases += ReportTestResult(VerifyDot<double>(reportTestCases), "double", "dot");
	nrOfFailedTestCases += ReportTestResult(VerifyScale<double>(reportTestCases), "double", "scale");
	nrOfFailedTestCases += ReportTestResult(VerifySwap<double>(reportTestCases), "double", "swap");
	nrOfFailedTestCases += ReportTestResult(VerifyAmaxAmin<double>(reportTestCases), "double", "amax/amin");
	nrOfFailedTestCases += ReportTestResult(VerifyRot<double>(reportTestCases), "double", "rot");
	nrOfFailedTestCases += ReportTestResult(VerifyNorms<double>(reportTestCases), "double", "norms");

	// BLAS Level 2 tests with double
	nrOfFailedTestCases += ReportTestResult(VerifyMatvec<double>(reportTestCases), "double", "matvec");

	// BLAS Level 3 tests with double
	nrOfFailedTestCases += ReportTestResult(VerifySumOfElements<double>(reportTestCases), "double", "sumOfElements");
	nrOfFailedTestCases += ReportTestResult(VerifyNormalize<double>(reportTestCases), "double", "normalize");
	nrOfFailedTestCases += ReportTestResult(VerifyMatrixNorm<double>(reportTestCases), "double", "matrixNorm");
#endif

#if REGRESSION_LEVEL_2
	// BLAS Level 1 tests with float
	nrOfFailedTestCases += ReportTestResult(VerifyAsum<float>(reportTestCases), "float", "asum");
	nrOfFailedTestCases += ReportTestResult(VerifyDot<float>(reportTestCases), "float", "dot");
	nrOfFailedTestCases += ReportTestResult(VerifyAxpy<float>(reportTestCases), "float", "axpy");
	nrOfFailedTestCases += ReportTestResult(VerifyNorms<float>(reportTestCases), "float", "norms");

	// BLAS Level 2 tests with float
	nrOfFailedTestCases += ReportTestResult(VerifyMatvec<float>(reportTestCases), "float", "matvec");

	// BLAS Level 3 tests with float
	nrOfFailedTestCases += ReportTestResult(VerifySumOfElements<float>(reportTestCases), "float", "sumOfElements");
#endif

#if REGRESSION_LEVEL_3
	// BLAS tests with posit
	nrOfFailedTestCases += ReportTestResult(VerifyAsum<posit<32, 2>>(reportTestCases), "posit<32,2>", "asum");
	nrOfFailedTestCases += ReportTestResult(VerifyDot<posit<32, 2>>(reportTestCases), "posit<32,2>", "dot");
	nrOfFailedTestCases += ReportTestResult(VerifyAxpy<posit<32, 2>>(reportTestCases), "posit<32,2>", "axpy");
	nrOfFailedTestCases += ReportTestResult(VerifyMatvec<posit<32, 2>>(reportTestCases), "posit<32,2>", "matvec");
#endif

#if REGRESSION_LEVEL_4
	// Stress tests with different types
	using bfloat_t = cfloat<16, 8, uint16_t, true, true, false>;
	nrOfFailedTestCases += ReportTestResult(VerifyDot<bfloat_t>(reportTestCases), "bfloat16", "dot");
	nrOfFailedTestCases += ReportTestResult(VerifyMatvec<bfloat_t>(reportTestCases), "bfloat16", "matvec");
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
