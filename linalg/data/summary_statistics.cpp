// summary_statistics.cpp: test suite for summary statistics function for data preprocessing
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <sstream>
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

// Stillwater BLAS library
#include <blas/blas.hpp>
#include <universal/verification/test_suite.hpp>


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
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

namespace sw { namespace blas {

	////////////////////////////////////////////////////////////////////////
	// Test summaryStatistics() function
	template<typename Scalar>
	int VerifySummaryStatistics(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test with known data: [1, 2, 3, 4, 5]
		// mean = 3.0, stddev = sqrt(10/4) = 1.5811...
		{
			std::vector<Scalar> data = { Scalar(1), Scalar(2), Scalar(3), Scalar(4), Scalar(5) };
			auto stats = summaryStatistics(data);

			if (std::abs(double(stats.mean) - 3.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: mean of [1,2,3,4,5] = " << stats.mean << " (expected 3.0)\n";
			}

			// Sample stddev: sqrt(sum((x-mean)^2)/(N-1)) = sqrt(10/4) = 1.5811...
			double expectedStddev = std::sqrt(10.0 / 4.0);
			if (std::abs(double(stats.stddev) - expectedStddev) > 0.01) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: stddev of [1,2,3,4,5] = " << stats.stddev << " (expected " << expectedStddev << ")\n";
			}

			// Quantiles: min=1, q1=2, median=3, q3=4, max=5
			if (double(stats.quantiles.q[0]) != 1.0 || double(stats.quantiles.q[4]) != 5.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: quantiles min/max incorrect\n";
			}
		}

		// Test with two elements (edge case for sample stddev formula)
		{
			std::vector<Scalar> data = { Scalar(0), Scalar(2) };
			auto stats = summaryStatistics(data);

			if (std::abs(double(stats.mean) - 1.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: mean of [0,2] = " << stats.mean << " (expected 1.0)\n";
			}

			// Sample stddev: sqrt(2/(2-1)) = sqrt(2) = 1.4142...
			double expectedStddev = std::sqrt(2.0);
			if (std::abs(double(stats.stddev) - expectedStddev) > 0.01) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: stddev of [0,2] = " << stats.stddev << " (expected " << expectedStddev << ")\n";
			}
		}

		// Test with negative values
		{
			std::vector<Scalar> data = { Scalar(-2), Scalar(-1), Scalar(0), Scalar(1), Scalar(2) };
			auto stats = summaryStatistics(data);

			if (std::abs(double(stats.mean)) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: mean of [-2,-1,0,1,2] = " << stats.mean << " (expected 0.0)\n";
			}
		}

		// Test with uniform data (stddev should be 0)
		{
			std::vector<Scalar> data = { Scalar(5), Scalar(5), Scalar(5), Scalar(5) };
			auto stats = summaryStatistics(data);

			if (std::abs(double(stats.mean) - 5.0) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: mean of [5,5,5,5] = " << stats.mean << " (expected 5.0)\n";
			}

			if (std::abs(double(stats.stddev)) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: stddev of [5,5,5,5] = " << stats.stddev << " (expected 0.0)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test quantiles() function directly
	template<typename Scalar>
	int VerifyQuantiles(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test with 8 elements: [1,2,3,4,5,6,7,8]
		{
			std::vector<Scalar> data = { Scalar(1), Scalar(2), Scalar(3), Scalar(4),
			                              Scalar(5), Scalar(6), Scalar(7), Scalar(8) };
			auto q = quantiles(data);

			// min = 1, q1 = v[2] = 3, median = v[4] = 5, q3 = v[6] = 7, max = 8
			if (double(q.q[0]) != 1.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: quantiles min = " << q.q[0] << " (expected 1)\n";
			}
			if (double(q.q[4]) != 8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: quantiles max = " << q.q[4] << " (expected 8)\n";
			}
		}

		// Test with unsorted data
		{
			std::vector<Scalar> data = { Scalar(5), Scalar(1), Scalar(8), Scalar(3),
			                              Scalar(7), Scalar(2), Scalar(6), Scalar(4) };
			auto q = quantiles(data);

			// After sorting: [1,2,3,4,5,6,7,8]
			if (double(q.q[0]) != 1.0 || double(q.q[4]) != 8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: quantiles on unsorted data incorrect\n";
			}
		}

		// Test Quantiles struct constructors
		{
			Quantiles<Scalar> q1;  // default constructor
			Quantiles<Scalar> q2(Scalar(1), Scalar(2), Scalar(3), Scalar(4), Scalar(5));

			if (double(q2.q[0]) != 1.0 || double(q2.q[2]) != 3.0 || double(q2.q[4]) != 5.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: Quantiles parameterized constructor\n";
			}

			// Test set() method
			q1.set(Scalar(10), Scalar(20), Scalar(30), Scalar(40), Scalar(50));
			if (double(q1.q[0]) != 10.0 || double(q1.q[4]) != 50.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: Quantiles::set() method\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test gaussian_random() with std::vector
	template<typename Scalar>
	int VerifyGaussianRandomStdVector(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Generate gaussian random data and verify basic properties
		{
			size_t N = 10000;
			std::vector<Scalar> data(N);
			gaussian_random(data, 0.0, 1.0);

			auto stats = summaryStatistics(data);

			// With 10000 samples, mean should be close to 0 (within ~0.05 typically)
			if (std::abs(double(stats.mean)) > 0.1) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian mean = " << stats.mean << " (expected ~0.0)\n";
			}

			// stddev should be close to 1 (within ~0.05 typically)
			if (std::abs(double(stats.stddev) - 1.0) > 0.1) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian stddev = " << stats.stddev << " (expected ~1.0)\n";
			}
		}

		// Test with different mean and stddev
		{
			size_t N = 10000;
			std::vector<Scalar> data(N);
			gaussian_random(data, 100.0, 10.0);

			auto stats = summaryStatistics(data);

			if (std::abs(double(stats.mean) - 100.0) > 2.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian mean = " << stats.mean << " (expected ~100.0)\n";
			}

			if (std::abs(double(stats.stddev) - 10.0) > 1.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian stddev = " << stats.stddev << " (expected ~10.0)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test gaussian_random() with blas::vector
	template<typename Scalar>
	int VerifyGaussianRandomBlasVector(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Generate gaussian random data using blas::vector
		{
			size_t N = 1000;
			sw::numeric::containers::vector<Scalar> v(N);
			gaussian_random(v, 0.0, 1.0);

			// Compute mean manually
			Scalar sum{0};
			for (size_t i = 0; i < N; ++i) {
				sum += v[i];
			}
			Scalar mean = sum / Scalar(N);

			// Mean should be close to 0
			if (std::abs(double(mean)) > 0.2) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: blas::vector gaussian mean = " << mean << " (expected ~0.0)\n";
			}
		}

		// Test gaussian_random_vector factory function
		{
			auto v = gaussian_random_vector<Scalar>(1000, 50.0, 5.0);

			Scalar sum{0};
			for (size_t i = 0; i < size(v); ++i) {
				sum += v[i];
			}
			Scalar mean = sum / Scalar(size(v));

			if (std::abs(double(mean) - 50.0) > 2.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian_random_vector mean = " << mean << " (expected ~50.0)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test gaussian_random() with blas::matrix
	template<typename Scalar>
	int VerifyGaussianRandomMatrix(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Generate gaussian random matrix
		{
			sw::numeric::containers::matrix<Scalar> A(50, 50);
			gaussian_random(A, 0.0, 1.0);

			// Compute mean of all elements
			Scalar sum{0};
			for (size_t i = 0; i < num_rows(A); ++i) {
				for (size_t j = 0; j < num_cols(A); ++j) {
					sum += A[i][j];
				}
			}
			Scalar mean = sum / Scalar(num_rows(A) * num_cols(A));

			if (std::abs(double(mean)) > 0.2) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: matrix gaussian mean = " << mean << " (expected ~0.0)\n";
			}
		}

		// Test gaussian_random_matrix factory function
		{
			auto A = gaussian_random_matrix<Scalar>(30, 30, 100.0, 10.0);

			if (num_rows(A) != 30 || num_cols(A) != 30) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian_random_matrix dimensions incorrect\n";
			}

			Scalar sum{0};
			for (size_t i = 0; i < num_rows(A); ++i) {
				for (size_t j = 0; j < num_cols(A); ++j) {
					sum += A[i][j];
				}
			}
			Scalar mean = sum / Scalar(num_rows(A) * num_cols(A));

			if (std::abs(double(mean) - 100.0) > 5.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: gaussian_random_matrix mean = " << mean << " (expected ~100.0)\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test SummaryStats and Quantiles operator<<
	int VerifyStreamOperators(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test Quantiles operator<<
		{
			Quantiles<double> q(1.0, 2.0, 3.0, 4.0, 5.0);
			std::ostringstream oss;
			oss << q;
			std::string output = oss.str();

			if (output.find("quantiles:") == std::string::npos) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: Quantiles operator<< missing 'quantiles:'\n";
			}
			if (output.find("1") == std::string::npos || output.find("5") == std::string::npos) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: Quantiles operator<< missing values\n";
			}
		}

		// Test SummaryStats operator<<
		{
			SummaryStats<double> stats;
			stats.mean = 10.0;
			stats.stddev = 2.0;
			stats.quantiles.set(1.0, 5.0, 10.0, 15.0, 20.0);

			std::ostringstream oss;
			oss << stats;
			std::string output = oss.str();

			if (output.find("mean") == std::string::npos) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: SummaryStats operator<< missing 'mean'\n";
			}
			if (output.find("stddev") == std::string::npos) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: SummaryStats operator<< missing 'stddev'\n";
			}
		}

		return nrOfFailedTests;
	}

}} // namespace sw::blas

/*
 
 stats for a couple of 1M element runs:
 mean     : -0.000105222
 stddev   : 0.999774
 quartiles
 [ -4.40803, -0.673688, 0.00028514, 0.672469, 4.67264]
 mean     : -0.000603408
 stddev   : 1.00284
 quartiles
 [ -5.29692, -0.675401, -0.000193462, 0.674231, 4.90644]
 mean     : -0.0010701
 stddev   : 0.997858
 quartiles
 [ -4.99329, -0.674899, -0.00123088, 0.673464, 4.73132]

 */


int main()
try {
	using namespace sw::universal;
	using namespace sw::blas;

	std::string test_suite  = "summary statistics";
	std::string test_tag    = "sumstat";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	size_t N = 1024*1024;
	std::vector<double> data(N);
	gaussian_random(data, 0.0, 1.0);
	auto stats = summaryStatistics(data);

	std::cout << "Summary statistics:\n" << stats << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// Test summaryStatistics() with double
	nrOfFailedTestCases += ReportTestResult(VerifySummaryStatistics<double>(reportTestCases), "double", "summaryStatistics");

	// Test quantiles() function directly
	nrOfFailedTestCases += ReportTestResult(VerifyQuantiles<double>(reportTestCases), "double", "quantiles");

	// Test gaussian_random with std::vector<double>
	nrOfFailedTestCases += ReportTestResult(VerifyGaussianRandomStdVector<double>(reportTestCases), "std::vector<double>", "gaussian_random");

	// Test stream operators
	nrOfFailedTestCases += ReportTestResult(VerifyStreamOperators(reportTestCases), "SummaryStats/Quantiles", "operator<<");
#endif

#if REGRESSION_LEVEL_2
	// Test with float
	nrOfFailedTestCases += ReportTestResult(VerifySummaryStatistics<float>(reportTestCases), "float", "summaryStatistics");
	nrOfFailedTestCases += ReportTestResult(VerifyQuantiles<float>(reportTestCases), "float", "quantiles");
	nrOfFailedTestCases += ReportTestResult(VerifyGaussianRandomStdVector<float>(reportTestCases), "std::vector<float>", "gaussian_random");

	// Test gaussian_random with blas::vector
	nrOfFailedTestCases += ReportTestResult(VerifyGaussianRandomBlasVector<double>(reportTestCases), "blas::vector<double>", "gaussian_random");
#endif

#if REGRESSION_LEVEL_3
	// Test gaussian_random with blas::matrix
	nrOfFailedTestCases += ReportTestResult(VerifyGaussianRandomMatrix<double>(reportTestCases), "blas::matrix<double>", "gaussian_random");
	nrOfFailedTestCases += ReportTestResult(VerifyGaussianRandomMatrix<float>(reportTestCases), "blas::matrix<float>", "gaussian_random");
#endif

#if REGRESSION_LEVEL_4
	// Stress test with larger dataset
	{
		size_t N = 100000;
		std::vector<double> data(N);
		gaussian_random(data, 0.0, 1.0);
		auto stats = summaryStatistics(data);

		// With 100k samples, statistics should be very close to target
		if (std::abs(stats.mean) > 0.02) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: large dataset mean = " << stats.mean << "\n";
		}
		if (std::abs(stats.stddev - 1.0) > 0.02) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: large dataset stddev = " << stats.stddev << "\n";
		}
	}
	nrOfFailedTestCases += ReportTestResult(0, "stress test", "100k samples");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
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
