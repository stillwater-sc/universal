// multiplication.cpp: test suite for interval multiplication
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
int VerifyIntervalMultiplication(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// [a,b] * [c,d] = [min(ac,ad,bc,bd), max(ac,ad,bc,bd)]

	// Test 1: positive intervals
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(3), Scalar(4));
		Interval c = a * b;
		Interval expected(Scalar(3), Scalar(8));  // [1*3, 2*4]
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " * " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 2: negative intervals
	{
		Interval a(Scalar(-3), Scalar(-1));
		Interval b(Scalar(-4), Scalar(-2));
		Interval c = a * b;
		Interval expected(Scalar(2), Scalar(12));  // [(-1)*(-2), (-3)*(-4)]
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " * " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 3: positive * negative
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(-4), Scalar(-3));
		Interval c = a * b;
		Interval expected(Scalar(-8), Scalar(-3));  // [2*(-4), 1*(-3)]
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " * " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 4: interval containing zero * positive
	{
		Interval a(Scalar(-1), Scalar(2));
		Interval b(Scalar(3), Scalar(4));
		Interval c = a * b;
		Interval expected(Scalar(-4), Scalar(8));  // [(-1)*4, 2*4]
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " * " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 5: both intervals contain zero
	{
		Interval a(Scalar(-1), Scalar(2));
		Interval b(Scalar(-3), Scalar(4));
		Interval c = a * b;
		// Products: (-1)*(-3)=3, (-1)*4=-4, 2*(-3)=-6, 2*4=8
		Interval expected(Scalar(-6), Scalar(8));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " * " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 6: *= operator
	{
		Interval a(Scalar(2), Scalar(3));
		Interval b(Scalar(4), Scalar(5));
		a *= b;
		Interval expected(Scalar(8), Scalar(15));
		if (a != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: *= operator, result = " << a << " (expected " << expected << ")\n";
		}
	}

	// Test 7: multiply by scalar
	{
		Interval a(Scalar(1), Scalar(2));
		Interval c = a * Scalar(3);
		Interval expected(Scalar(3), Scalar(6));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " * 3 = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 8: multiply by negative scalar
	{
		Interval a(Scalar(1), Scalar(2));
		Interval c = a * Scalar(-3);
		Interval expected(Scalar(-6), Scalar(-3));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " * (-3) = " << c << " (expected " << expected << ")\n";
		}
	}

	// Test 9: degenerate intervals
	{
		Interval a(Scalar(3));
		Interval b(Scalar(4));
		Interval c = a * b;
		Interval expected(Scalar(12));
		if (c != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " * " << b << " = " << c << " (expected " << expected << ")\n";
		}
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "interval multiplication validation";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual testing

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += VerifyIntervalMultiplication<float>(reportTestCases);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += VerifyIntervalMultiplication<double>(reportTestCases);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += VerifyIntervalMultiplication<cfloat<16, 5, uint16_t>>(reportTestCases);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += VerifyIntervalMultiplication<cfloat<32, 8, uint32_t>>(reportTestCases);
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
