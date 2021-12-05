// stream.cpp: stream benchmarks of vector operations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
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

#include <universal/verification/performance_runner.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

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
	Scalar alpha(pi);
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

template<typename Scalar>
void Sweep(size_t startSample = 13, size_t endSample = 28) {
	using namespace std;
	using namespace std::chrono;
	constexpr double pi = 3.14159265358979323846;
	Scalar alpha(pi);

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
	//bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING
	Sweep<float>();
	Sweep < fixpnt<8, 4> >();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	Sweep<float>(13, 15);
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
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
