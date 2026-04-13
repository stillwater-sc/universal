// performance.cpp: test suite runner for measuring performance off adaptive precision decimal integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/benchmark/performance_runner.hpp>

namespace sw::universal::internal {

	// Generic set of adds and subtracts for a given number system type
	template<typename Scalar>
	void AdditionSubtractionWorkload(size_t NR_OPS) {
	Scalar a{0}, b{0}, c{0}, d{0};
	    a = "1234567890123456789012345";
	    b = a;
		// we are doing two adds per loop, so half the NR_OPS
		NR_OPS >>= 1;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a + b;
			d = c - a;
		}
		//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (d == a)
			std::cout << " ";
		else
			std::cout << "-";
	}

	// Generic set of multiplies for a given number system type
	template<typename Scalar>
	void MultiplicationWorkload(size_t NR_OPS) {
		Scalar a{0}, b{0}, c{0}, d{0};
	    a = "1234567890123456789012345";
	    b = 10;
	    d = a * b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a * b;
		}
		//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d)
			std::cout << " ";
		else
			std::cout << "-";
	}

	// Generic set of divides for a given number system type
	template<typename Scalar>
	void DivisionWorkload(size_t NR_OPS) {
		Scalar a{0}, b{0}, c{0}, d{0};
	    a = "1234567890123456789012345";
	    b = 5;
	    d = a / b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a / b;
		}
		//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d)
			std::cout << " ";
		else
			std::cout << "-";
	}

	// Generic set of remainders for a given number system type
	template<typename Scalar>
	void RemainderWorkload(size_t NR_OPS) {
		Scalar a{0}, b{0}, c{0}, d{0};
	    a = "1234567890123456789012345";
	    b = 5;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a % b;
		}
		//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == 0)
			std::cout << " ";
		else
			std::cout << "-";
	}

	void TestArithmeticOperatorPerformance() {
	    using namespace sw::universal;
	    std::cout << "\nArithmetic operator performance\n";

	    size_t NR_OPS = 128ull * 1024ull * 4ull;
	    PerformanceRunner("edecimal 25 digits   add/subtract  ", AdditionSubtractionWorkload<sw::universal::edecimal>, NR_OPS);

	    NR_OPS = 64ull * 1024ull;
	    PerformanceRunner("edecimal 25 digits   multiplication", MultiplicationWorkload<sw::universal::edecimal>, NR_OPS);

	    NR_OPS = 16ull * 1024ull;
	    PerformanceRunner("edecimal 25 digits   division      ", DivisionWorkload<sw::universal::edecimal>, NR_OPS);

	    NR_OPS = 16ull * 1024ull;
	    PerformanceRunner("edecimal 25 digits   remainder     ", RemainderWorkload<sw::universal::edecimal>, NR_OPS);
    }

}  // namespace sw::universal::internal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "edecimal performance testing";
	std::string test_tag    = "edecimal performance";
	int nrOfFailedTestCases = 0;


	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	internal::TestArithmeticOperatorPerformance();

#else


#if REGRESSION_LEVEL_1

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
