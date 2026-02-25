// conversion.cpp: test suite for interval type conversions
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

// Test conversion from scalar to interval
template<typename Scalar>
int VerifyScalarConversion(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// Test degenerate interval from scalar
	{
		Scalar s(3.14159f);
		Interval a(s);
		if (a.lo() != s || a.hi() != s) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: scalar conversion " << s << " -> " << a << '\n';
		}
	}

	// Test assignment from scalar
	{
		Scalar s(2.71828f);
		Interval a;
		a = s;
		if (a.lo() != s || a.hi() != s) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: scalar assignment " << s << " -> " << a << '\n';
		}
	}

	// Test interval from two scalars
	{
		Scalar lo(1.0f), hi(2.0f);
		Interval a(lo, hi);
		if (a.lo() != lo || a.hi() != hi) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: two scalar construction [" << lo << ", " << hi << "] -> " << a << '\n';
		}
	}

	// Test interval auto-ordering (swap if lo > hi)
	{
		Scalar lo(5.0f), hi(2.0f);  // intentionally reversed
		Interval a(lo, hi);
		if (a.lo() != hi || a.hi() != lo) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: auto-ordering [" << lo << ", " << hi << "] -> " << a << '\n';
		}
	}

	return nrOfFailedTestCases;
}

// Test explicit conversions to native types
template<typename Scalar>
int VerifyExplicitConversions(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// Conversion to float (returns midpoint)
	{
		Interval a(Scalar(1.0f), Scalar(3.0f));
		float f = static_cast<float>(a);
		float expected = 2.0f;  // midpoint
		if (std::abs(f - expected) > 1e-6f) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: float conversion " << a << " -> " << f << " (expected " << expected << ")\n";
		}
	}

	// Conversion to double
	{
		Interval a(Scalar(2.0f), Scalar(4.0f));
		double d = static_cast<double>(a);
		double expected = 3.0;  // midpoint
		if (std::abs(d - expected) > 1e-10) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: double conversion " << a << " -> " << d << " (expected " << expected << ")\n";
		}
	}

	return nrOfFailedTestCases;
}

// Test special value assignments
template<typename Scalar>
int VerifySpecialValues(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// Zero
	{
		Interval a;
		a.setzero();
		if (!a.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: setzero() " << a << '\n';
		}
	}

	// Infinity
	{
		Interval a;
		a.setinf(false);  // +inf
		if (!a.isinf()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: setinf(false) " << a << '\n';
		}
	}

	// NaN
	{
		Interval a;
		a.setnan();
		if (!a.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: setnan() " << a << '\n';
		}
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "interval conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual testing code here

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	std::cout << "Scalar conversion tests (float)\n";
	nrOfFailedTestCases += VerifyScalarConversion<float>(reportTestCases);
	std::cout << "Scalar conversion tests (double)\n";
	nrOfFailedTestCases += VerifyScalarConversion<double>(reportTestCases);
#endif

#if REGRESSION_LEVEL_2
	std::cout << "Explicit conversion tests (float)\n";
	nrOfFailedTestCases += VerifyExplicitConversions<float>(reportTestCases);
	std::cout << "Explicit conversion tests (double)\n";
	nrOfFailedTestCases += VerifyExplicitConversions<double>(reportTestCases);
#endif

#if REGRESSION_LEVEL_3
	std::cout << "Special value tests (float)\n";
	nrOfFailedTestCases += VerifySpecialValues<float>(reportTestCases);
	std::cout << "Special value tests (double)\n";
	nrOfFailedTestCases += VerifySpecialValues<double>(reportTestCases);
#endif

#if REGRESSION_LEVEL_4
	// Test with Universal cfloat type
	std::cout << "Scalar conversion tests (cfloat<16,5>)\n";
	nrOfFailedTestCases += VerifyScalarConversion<cfloat<16, 5, uint16_t>>(reportTestCases);
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
