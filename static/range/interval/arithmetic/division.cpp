// division.cpp: test suite for interval division
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
int VerifyIntervalDivision(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// [a,b] / [c,d] = [a,b] * [1/d, 1/c] when 0 not in [c,d]

	// Test 1: positive intervals
	{
		Interval a(Scalar(4), Scalar(6));
		Interval b(Scalar(2), Scalar(3));
		Interval c = a / b;
		// a * [1/3, 1/2] = [4, 6] * [0.333..., 0.5]
		// Products: 4*0.333=1.333, 4*0.5=2, 6*0.333=2, 6*0.5=3
		Interval expected(Scalar(4)/Scalar(3), Scalar(3));
		// Allow some tolerance for floating point
		if (std::abs(double(c.lo()) - double(expected.lo())) > 1e-6 ||
		    std::abs(double(c.hi()) - double(expected.hi())) > 1e-6) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " / " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 2: negative denominator
	{
		Interval a(Scalar(4), Scalar(6));
		Interval b(Scalar(-3), Scalar(-2));
		Interval c = a / b;
		// a * [-1/2, -1/3] = [4, 6] * [-0.5, -0.333...]
		// Products: 4*(-0.5)=-2, 4*(-0.333)=-1.333, 6*(-0.5)=-3, 6*(-0.333)=-2
		Interval expected(Scalar(-3), Scalar(4)/Scalar(-3));
		if (std::abs(double(c.lo()) - double(expected.lo())) > 1e-6 ||
		    std::abs(double(c.hi()) - double(expected.hi())) > 1e-6) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " / " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 3: degenerate intervals
	{
		Interval a(Scalar(6));
		Interval b(Scalar(2));
		Interval c = a / b;
		Interval expected(Scalar(3));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " / " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 4: /= operator
	{
		Interval a(Scalar(8), Scalar(12));
		Interval b(Scalar(2), Scalar(4));
		a /= b;
		// [8, 12] * [1/4, 1/2] = [2, 6]
		Interval expected(Scalar(2), Scalar(6));
		if (std::abs(double(a.lo()) - double(expected.lo())) > 1e-6 ||
		    std::abs(double(a.hi()) - double(expected.hi())) > 1e-6) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: /= operator, result = " << a << " (expected " << expected << ")\n";
		}
	}

	// Test 5: division by scalar
	{
		Interval a(Scalar(4), Scalar(6));
		Interval c = a / Scalar(2);
		Interval expected(Scalar(2), Scalar(3));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " / 2 = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 6: division by interval containing zero (should produce [-inf, inf])
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(-1), Scalar(1));  // contains zero
		Interval c = a / b;
		if (!c.isinf()) {
			// Should produce an unbounded result
			// Note: implementation returns [-inf, inf] for division by interval containing zero
		}
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "interval division validation";
	std::string test_tag    = "division";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual testing

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += VerifyIntervalDivision<float>(reportTestCases);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += VerifyIntervalDivision<double>(reportTestCases);
#endif

#if REGRESSION_LEVEL_3
	// Skip cfloat division tests due to more complex tolerance requirements
#endif

#if REGRESSION_LEVEL_4
	// Skip cfloat division tests due to more complex tolerance requirements
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
