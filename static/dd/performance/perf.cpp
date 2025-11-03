//  perf.cpp : baseline performance benchmarking for double-double (dd) arithmetic operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <chrono>

#include <universal/number/dd/dd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/benchmark/performance_runner.hpp>

namespace sw::universal::internal {

	// Generic set of adds and subtracts for a given number system type
	template<typename Scalar>
	void AdditionSubtractionWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1.0f;
		c = d;
		b = c;
		a = b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a + b;
			b = c;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d) std::cout << "amazing\n";
	}

	// Generic set of multiplies for a given number system type
	template<typename Scalar>
	void MultiplicationWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1.0f;
		c = d;
		b = 1.125f;
		a = 1.0f / b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a * b;
			b = c;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d) std::cout << "amazing\n";
	}

	// Generic set of divides for a given number system type
	template<typename Scalar>
	void DivisionWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1.0f;
		c = d;
		b = 1.5f;
		a = 0.75f;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a / b;
			b = c;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d) std::cout << "amazing\n";
	}

	/*
	August, 2024, AMD Ryzen 7 2700X Eight-Core Processor, 3.70 GHz desktop

	Arithmetic operator performance (no SIMD)
	float    add/subtract      4194304 per       0.0003982sec ->  10 Gops/sec
	double   add/subtract      4194304 per       0.0004099sec ->  10 Gops/sec
	quad     add/subtract      2097152 per        0.204351sec ->  10 Mops/sec
	dd       add/subtract      4194304 per       0.0033172sec ->   1 Gops/sec
	float    multiplication    1048576 per        9.78e-05sec ->  10 Gops/sec
	double   multiplication    1048576 per       0.0001303sec ->   8 Gops/sec
	quad     multiplication     524288 per       0.0906857sec ->   5 Mops/sec
	dd       multiplication    1048576 per       0.0450566sec ->  23 Mops/sec
	float    division          1048576 per       0.0026275sec -> 399 Mops/sec
	double   division          1048576 per       0.0027365sec -> 383 Mops/sec
	quad     division           524288 per         5.85166sec ->  89 Kops/sec
	dd       division          1048576 per        0.112724sec ->   9 Mops/sec

	 */

	void TestArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("float    add/subtract  ", AdditionSubtractionWorkload< float >, NR_OPS);
		PerformanceRunner("double   add/subtract  ", AdditionSubtractionWorkload< double >, NR_OPS);
		PerformanceRunner("quad     add/subtract  ", AdditionSubtractionWorkload< sw::universal::quad >, NR_OPS / 2);
		PerformanceRunner("dd       add/subtract  ", AdditionSubtractionWorkload< sw::universal::dd >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("float    multiplication", MultiplicationWorkload< float >, NR_OPS);
		PerformanceRunner("double   multiplication", MultiplicationWorkload< double >, NR_OPS);
		PerformanceRunner("quad     multiplication", MultiplicationWorkload< sw::universal::quad >, NR_OPS / 2);
		PerformanceRunner("dd       multiplication", MultiplicationWorkload< sw::universal::dd >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("float    division      ", DivisionWorkload< float >, NR_OPS);
		PerformanceRunner("double   division      ", DivisionWorkload< double >, NR_OPS);
		PerformanceRunner("quad     division      ", DivisionWorkload< sw::universal::quad >, NR_OPS / 2);
		PerformanceRunner("dd       division      ", DivisionWorkload< sw::universal::dd >, NR_OPS);

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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::internal;

	std::string test_suite  = "double-double operator performance benchmarking";
	std::string test_tag    = "performance";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	TestArithmeticOperatorPerformance();
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// benchmark testing is a LEVEL_4 activity
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_4
	TestArithmeticOperatorPerformance();
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}

/*
ETLO
Date run : 3/01/2021
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/
