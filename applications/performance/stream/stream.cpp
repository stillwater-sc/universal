// stream.cpp: stream benchmarks of vector operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <math.h>
#include <stdint.h>
#include <iostream>
#include <chrono>
#include <cmath>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>

// Configure the cfloat template environment
// first: enable general or specialized cfloat configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>

// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

#include <universal/verification/test_suite.hpp>
#include <universal/benchmark/performance_runner.hpp>

template<typename Scalar>
void Copy(std::vector<Scalar>& c, const std::vector<Scalar>& a, size_t start, size_t end) {
	for (size_t i = start; i < end; ++i) {
		c[i] = a[i];
	}
}

template<typename Scalar>
void Sum(std::vector<Scalar>& c, const std::vector<Scalar>& a, const std::vector<Scalar>& b, size_t start, size_t end) {
	for (size_t i = start; i < end; ++i) {
		c[i] = a[i] + b[i];
	}
}

template<typename Scalar>
void Scale(std::vector<Scalar>& c, const Scalar& a, const std::vector<Scalar>& b, size_t start, size_t end) {
	for (size_t i = start; i < end; ++i) {
		c[i] = a * b[i];
	}
}

template<typename Scalar>
void Triad(std::vector<Scalar>& c, const std::vector<Scalar>& a, const std::vector<Scalar>& b, size_t start, size_t end) {
	constexpr double pi = 3.14159265358979323846;
	Scalar alpha(static_cast<Scalar>(pi));
	for (size_t i = start; i < end; ++i) {
		c[i] = a[i] + alpha*b[i];
	}
}

void ClearCache() {
	constexpr size_t SIZE = (1ull << 27); // 128MB element array of 8byte doubles = 1GB data set
	std::vector<double> a(SIZE);
	for (size_t i = 0; i < SIZE; ++i) {
		a[i] = INFINITY;
	}
}
template<typename Scalar>
void Reset(std::vector<Scalar>& v, Scalar resetValue) {
	for (size_t i = 0; i < v.size(); ++i) {
		v[i] = resetValue;
	}
}

// sweep vector operators for different vector sizes.
// The sweep selected is defined by startSample and endSample
// and read through vectors sizes defined by 2^startSample, 2^(startSample+1), ... 2^(endSample-1)
template<typename Scalar>
void Sweep(size_t startSample, size_t endSample) {   // 13, 28
	using namespace std;
	using namespace std::chrono;
	constexpr double pi = 3.14159265358979323846;
	Scalar alpha(static_cast<Scalar>(pi));

	std::cout << "STREAM benchmark for Universal type : " << type_tag(alpha) << '\n';

	// create storage
	size_t leftShift = endSample;
	size_t SIZE = (1ull << leftShift);
	std::vector<Scalar> a(SIZE), b(SIZE), c(SIZE);
	for (size_t i = 0; i < SIZE; ++i) {
		a[i] = Scalar(1.0f);
		b[i] = Scalar(0.5f);
		c[i] = Scalar(0.0f);
	}

	// benchmark different vector sizes
	for (size_t i = startSample; i < endSample; ++i) {
		size_t start = 0;
		size_t stop  = (1ull << i);
		Reset(c, Scalar(0));
		ClearCache();

		steady_clock::time_point begin = steady_clock::now();
		Copy(c, a, start, stop);
		steady_clock::time_point end = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>> (end - begin);
		double elapsed_time = time_span.count();
		size_t NR_OPS = (stop - start);
		cout << setw(10) << NR_OPS << " copies per " << setw(15) << elapsed_time << "sec -> " << toPowerOfTen(double(NR_OPS) / elapsed_time) << "ops/sec" << endl;

	}

	for (size_t i = startSample; i < endSample; ++i) {
		size_t start = 0;
		size_t stop = (1ull << i);
		Reset(c, Scalar(0));
		ClearCache();

		steady_clock::time_point begin = steady_clock::now();
		Sum(c, a, b, start, stop);
		steady_clock::time_point end = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>> (end - begin);
		double elapsed_time = time_span.count();
		size_t NR_OPS = (stop - start);
		cout << setw(10) << NR_OPS << " adds   per " << setw(15) << elapsed_time << "sec -> " << toPowerOfTen(double(NR_OPS) / elapsed_time) << "ops/sec" << endl;

	}

	for (size_t i = startSample; i < endSample; ++i) {
		size_t start = 0;
		size_t stop = (1ull << i);
		Reset(c, Scalar(0));
		ClearCache();

		steady_clock::time_point begin = steady_clock::now();
		Scale(c, alpha, b, start, stop);
		steady_clock::time_point end = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>> (end - begin);
		double elapsed_time = time_span.count();
		size_t NR_OPS = (stop - start);
		cout << setw(10) << NR_OPS << " muls   per " << setw(15) << elapsed_time << "sec -> " << toPowerOfTen(double(NR_OPS) / elapsed_time) << "ops/sec" << endl;

	}

	for (size_t i = startSample; i < endSample; ++i) {
		size_t start = 0;
		size_t stop = (1ull << i);
		Reset(c, Scalar(0));
		ClearCache();

		steady_clock::time_point begin = steady_clock::now();
		Triad(c, a, b, start, stop);
		steady_clock::time_point end = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>> (end - begin);
		double elapsed_time = time_span.count();
		size_t NR_OPS = (stop - start);
		cout << setw(10) << NR_OPS << " triads per " << setw(15) << elapsed_time << "sec -> " << toPowerOfTen(double(NR_OPS) / elapsed_time) << "ops/sec" << endl;

	}
}

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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main() 
try {
	using namespace sw::universal;

	std::string test_suite  = "STREAM performance measurement";
	std::string test_tag    = "stream";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	size_t startSample = 11;
	size_t endSample = 13;
#if MANUAL_TESTING
	Sweep<float>(startSample, endSample);
	Sweep < fixpnt<8, 4> >(startSample, endSample);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// benchmark testing is a LEVEL_4 activity
	// here we just do a sanity check that the operators work
	std::vector<float> a(16, 1.0f), b(16, 1.0f), c(16, 0.0f);
	Copy(c, a, 0, 16);
	Sum(c, a, c, 0, 16);
	Scale(c, 1.0f, b, 0, 16);
	Triad(c, a, c, 0, 16);
#endif

#if REGRESSION_LEVEL_2
	startSample = 10;
	endSample   = 11;  // just one pass through the operators
	Sweep<float>(startSample, endSample);
	Sweep<cfloat<32, 8, std::uint32_t, true, false, false>>(startSample, endSample);
#endif

#if REGRESSION_LEVEL_3
	startSample = 10;
	endSample   = 11;  // just one pass through the operators
	Sweep<int>(startSample, endSample);
	Sweep<float>(startSample, endSample);
	Sweep<double>(startSample, endSample);
#endif

#if REGRESSION_LEVEL_4
	startSample = 10;
	endSample   = 11;  // just one pass through the operators
	Sweep<int>(startSample, endSample);
	Sweep<float>(startSample, endSample);
	Sweep<double>(startSample, endSample);
	Sweep<fixpnt<8, 4, Modulo, std::uint8_t>>(startSample, endSample);
	Sweep<fixpnt<8, 4, Saturate, std::uint8_t>>(startSample, endSample);
	Sweep<cfloat<32, 8, std::uint32_t, true, false, false>>(startSample, endSample);

	Sweep<float>(10, 28);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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

/*

Ryzen
	  1024 copies per           9e-07sec ->   1 Gops/sec
	  2048 copies per         1.3e-06sec ->   1 Gops/sec
	  4096 copies per         2.6e-06sec ->   1 Gops/sec
	  8192 copies per         4.5e-06sec ->   1 Gops/sec
	 16384 copies per         9.5e-06sec ->   1 Gops/sec
	  1024 adds   per           1e-06sec ->   1 Gops/sec
	  2048 adds   per         2.1e-06sec -> 975 Mops/sec
	  4096 adds   per         3.4e-06sec ->   1 Gops/sec
	  8192 adds   per           5e-06sec ->   1 Gops/sec
	 16384 adds   per        1.11e-05sec ->   1 Gops/sec
	  1024 muls   per         1.1e-06sec -> 930 Mops/sec
	  2048 muls   per         1.2e-06sec ->   1 Gops/sec
	  4096 muls   per         2.2e-06sec ->   1 Gops/sec
	  8192 muls   per         5.9e-06sec ->   1 Gops/sec
	 16384 muls   per         1.3e-05sec ->   1 Gops/sec
	  1024 triads per         1.8e-06sec -> 568 Mops/sec
	  2048 triads per         3.5e-06sec -> 585 Mops/sec
	  4096 triads per         4.5e-06sec -> 910 Mops/sec
	  8192 triads per         6.8e-06sec ->   1 Gops/sec
	 16384 triads per        1.96e-05sec -> 835 Mops/sec


Intel Xeon
STREAM performance measurement
	  1024 copies per       2.236e-06sec -> 457 Mops/sec
	  2048 copies per       3.635e-06sec -> 563 Mops/sec
	  4096 copies per       5.084e-06sec -> 805 Mops/sec
	  8192 copies per       9.865e-06sec -> 830 Mops/sec
	 16384 copies per      1.9088e-05sec -> 858 Mops/sec
	  1024 adds   per       1.952e-06sec -> 524 Mops/sec
	  2048 adds   per       3.485e-06sec -> 587 Mops/sec
	  4096 adds   per       6.618e-06sec -> 618 Mops/sec
	  8192 adds   per      1.2515e-05sec -> 654 Mops/sec
	 16384 adds   per      2.6708e-05sec -> 613 Mops/sec
	  1024 muls   per       1.188e-06sec -> 861 Mops/sec
	  2048 muls   per       2.613e-06sec -> 783 Mops/sec
	  4096 muls   per        4.25e-06sec -> 963 Mops/sec
	  8192 muls   per       8.506e-06sec -> 963 Mops/sec
	 16384 muls   per       1.766e-05sec -> 927 Mops/sec
	  1024 triads per       1.985e-06sec -> 515 Mops/sec
	  2048 triads per       5.009e-06sec -> 408 Mops/sec
	  4096 triads per      8.8654e-05sec ->  46 Mops/sec
	  8192 triads per      8.5967e-05sec ->  95 Mops/sec
	 16384 triads per     0.000158844sec -> 103 Mops/sec

*/
