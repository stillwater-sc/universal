// saturation.cpp: sorn arithmetic past the ends of a lattice without infinity
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A value outside a lattice that has no infinity saturates to the outermost interval on its
// side (#1972). Conversion did, but arithmetic clamped only the bound that crossed the end:
// on sorn<0,4,8,...,no inf> 3.75 + 3.75 gave the inverted interval (7,4], and 3.75 * 3 gave
// (8.75,4] (#2054). Every arithmetic result past an end must now equal the conversion of a
// value past that end, and every result must be a well-formed interval.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

template<typename S>
bool same_interval(const S& a, const S& b) {
	const auto& x = a.sornIntVal;
	const auto& y = b.sornIntVal;
	return x.lowerBound == y.lowerBound && x.upperBound == y.upperBound && x.lowerIsOpen == y.lowerIsOpen && x.upperIsOpen == y.upperIsOpen;
}

template<typename S>
bool well_formed(const S& a) {
	const auto& x = a.sornIntVal;
	if (x.lowerBound < x.upperBound) return true;
	return x.lowerBound == x.upperBound && !x.lowerIsOpen && !x.upperIsOpen;   // a closed point
}

// results entirely past the top or the bottom of the lattice, for + - * / with sorn and scalar operands
template<typename S>
int VerifySaturatedResults(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const S top(1.0e6), bottom(-1.0e6);   // the conversion saturates to the outermost interval
	auto check = [&](const S& result, const S& expected, const char* label) {
		if (!same_interval(result, expected)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << result << " expected " << expected << '\n';
		}
	};
	const S a(3.75), b(3.0), m(-3.75), q(0.25);
	check(a + a, top, "3.75 + 3.75");
	check(a * b, top, "3.75 * 3");
	check(a - m, top, "3.75 - -3.75");
	check(a / q, top, "3.75 / 0.25");
	check(m * m, top, "-3.75 * -3.75");
	check(m + m, bottom, "-3.75 + -3.75");
	check(m * b, bottom, "-3.75 * 3");
	check(m - a, bottom, "-3.75 - 3.75");
	check(m / q, bottom, "-3.75 / 0.25");
	check(a * m, bottom, "3.75 * -3.75");
	{ S t(a); t += 10.0;  check(t, top, "3.75 + 10.0"); }
	{ S t(a); t *= 10.0;  check(t, top, "3.75 * 10.0"); }
	{ S t(m); t -= 10.0;  check(t, bottom, "-3.75 - 10.0"); }
	{ S t(m); t *= 10.0;  check(t, bottom, "-3.75 * 10.0"); }
	return nrOfFailedTests;
}

// every result of every pair of lattice intervals is a well-formed interval
template<typename S>
int VerifyWellFormed(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const auto& dt = S().datatype();
	for (const auto& x : dt) {
		for (const auto& y : dt) {
			S a, b;
			a.sornIntVal = x;
			b.sornIntVal = y;
			const S results[] = { a + b, a - b, a * b };
			const char* names[] = { "+", "-", "*" };
			for (int k = 0; k < 3; ++k) {
				if (!results[k].isnan() && !well_formed(results[k])) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << names[k] << ' ' << b << " = " << results[k] << " is not an interval\n";
				}
			}
		}
	}
	return nrOfFailedTests;
}

} // namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "sorn arithmetic saturation";
	std::string test_tag    = "saturation";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using NoInf    = sorn<0, 4, 8, true, true, true, false, true>;   // linear, no infinity
	using NoInfLog = sorn<-2, 2, 1, false, true, true, false, true>;  // logarithmic, no infinity

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifySaturatedResults<NoInf>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySaturatedResults<NoInf>(reportTestCases), "sorn<0,4,8,..,no inf>", "past the ends");
	nrOfFailedTestCases += ReportTestResult(VerifyWellFormed<NoInf>(reportTestCases), "sorn<0,4,8,..,no inf>", "well-formed results");
	nrOfFailedTestCases += ReportTestResult(VerifyWellFormed<NoInfLog>(reportTestCases), "sorn<-2,2,1,log,..,no inf>", "well-formed results");
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
