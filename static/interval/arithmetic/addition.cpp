// addition.cpp: test suite for interval addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/interval/interval.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

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

namespace sw { namespace universal {

template<typename Scalar>
int VerifyIntervalAddition(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// [a,b] + [c,d] = [a+c, b+d]

	// Test 1: positive intervals
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(3), Scalar(4));
		Interval c = a + b;
		Interval expected(Scalar(4), Scalar(6));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " + " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 2: negative intervals
	{
		Interval a(Scalar(-3), Scalar(-1));
		Interval b(Scalar(-5), Scalar(-2));
		Interval c = a + b;
		Interval expected(Scalar(-8), Scalar(-3));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " + " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 3: mixed sign intervals
	{
		Interval a(Scalar(-1), Scalar(2));
		Interval b(Scalar(1), Scalar(3));
		Interval c = a + b;
		Interval expected(Scalar(0), Scalar(5));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " + " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 4: degenerate interval (point)
	{
		Interval a(Scalar(2));
		Interval b(Scalar(3));
		Interval c = a + b;
		Interval expected(Scalar(5));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " + " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 5: += operator
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(3), Scalar(4));
		a += b;
		Interval expected(Scalar(4), Scalar(6));
		if (a != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: += operator, result = " << a << " (expected " << expected << ")\n";
		}
	}

	// Test 6: addition with scalar
	{
		Interval a(Scalar(1), Scalar(2));
		Interval c = a + Scalar(3);
		Interval expected(Scalar(4), Scalar(5));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " + 3 = " << c << " (expected " << expected << ")\n";
		}
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "interval addition validation";
	std::string test_tag    = "addition";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual testing

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += VerifyIntervalAddition<float>(reportTestCases);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += VerifyIntervalAddition<double>(reportTestCases);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += VerifyIntervalAddition<cfloat<16, 5, uint16_t>>(reportTestCases);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += VerifyIntervalAddition<cfloat<32, 8, uint32_t>>(reportTestCases);
#endif

#endif // MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
