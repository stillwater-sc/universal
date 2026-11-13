// range.cpp: elreal conversion to native types past their range, and significand() of large values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// approx<T>() converted each block to T on its own: past T's range a block saturated to +-inf,
// and opposite-signed infinities summed to NaN. double(exp(elreal(1e6))) and
// double(pow(elreal(2), elreal(2000.5))) were NaN, and significand(exp(elreal(800))), which
// divided the inf by 2^scale, was inf (#2054). The cases below build the large values from
// products, which exercise the same conversion. The blocks are now summed relative to the
// leading one and scaled once: a value past the range is +-inf, and significand() scales first.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// large values from products, which are cheap: the transcendental functions take seconds at
// the default precision (#2014), and the defect was in the conversion, not in them
int VerifyPastDoubleRange(bool reportTestCases) {
	using R = elreal<double>;
	int nrOfFailedTests = 0;
	const double inf = std::numeric_limits<double>::infinity();
	auto check = [&](double got, double expected, const std::string& label) {
		if (!(got == expected)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	const R big = R(1.0e300) * R(1.0e300);              // 1e600
	const R third = R(1.0) / R(3.0);
	check(double(big), inf, "double(1e300 * 1e300)");
	check(double(-big), -inf, "double(-(1e300 * 1e300))");
	check(double(big * third), inf, "double(1e600 / 3)");   // blocks of both signs past the range
	check(double(big - R(1.0)), inf, "double(1e600 - 1)");
	check(double(-(big * third)), -inf, "double(-1e600 / 3)");
	check(double((R(1.0e-300) * R(1.0e-300)) * third), 0.0, "double(1e-600 / 3)");
	// within range nothing changes
	check(double(third), 1.0 / 3.0, "double(1/3)");
	check(double(R(1.0e300) * R(-2.0)), -2.0e300, "double(1e300 * -2)");
	check(double(R(0x1.fffffffffffffp+1023)), 0x1.fffffffffffffp+1023, "double(DBL_MAX)");
	return nrOfFailedTests;
}

int VerifySignificandOfLargeValues(bool reportTestCases) {
	using R = elreal<double>;
	int nrOfFailedTests = 0;
	auto check = [&](const R& v, double expected, double tolerance, const std::string& label) {
		const double sig = significand(v);
		if (!(std::abs(sig - expected) <= tolerance)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: significand(" << label << ") = " << sig << " expected " << expected << '\n';
		}
	};
	const R big = R(1.0e300) * R(1.0e300);
	// 1e600 = 2^(600 log2 10) = 2^1993.1..., so its significand is 2^0.1...
	const double log2_1e600 = 600.0 * 3.3219280948873623;
	check(big, std::exp2(log2_1e600 - std::floor(log2_1e600)), 1e-12, "1e600");
	check(big * (R(1.0) / R(3.0)), std::exp2(log2_1e600 - std::log2(3.0) - std::floor(log2_1e600 - std::log2(3.0))), 1e-12, "1e600 / 3");
	check(R(3.0), 1.5, 0.0, "3");
	return nrOfFailedTests;
}

// a stream built directly from blocks need not be normalised: v * 2^exp with |v| well above 1.
// The scaling picks its leader and scales by the combined exponent scale_of_v() + exp, so
// a block v = DBL_MAX with a large tail stays finite (#2056 review)
int VerifyUnnormalisedStream(bool reportTestCases) {
	using R = elreal<double>;
	using B = block<double>;
	int nrOfFailedTests = 0;
	// value = DBL_MAX * 2^-1022 + 2^970 * 2^-1022 = 4 - 2^-52, held as two zero-overlap blocks
	// with equal raw exp; as a double it is a tie that rounds to even, 4. Summed at the raw exp,
	// DBL_MAX + 2^970 rounded to inf.
	const B lead{ std::numeric_limits<double>::max(), typename B::exp_t(-1022) };
	const B tail{ std::ldexp(1.0, 970), typename B::exp_t(-1022) };
	const R v(ZBCL<double>::cons(lead, ZBCL<double>::singleton(tail)));
	const double got = double(v);
	if (got != 4.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: unnormalised two-block stream = " << got << " expected 4\n";
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

	std::string test_suite  = "elreal conversion past the native range";
	std::string test_tag    = "range";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyPastDoubleRange(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyPastDoubleRange(reportTestCases), "elreal<double>", "past double's range");
	nrOfFailedTestCases += ReportTestResult(VerifySignificandOfLargeValues(reportTestCases), "elreal<double>", "significand of large values");
	nrOfFailedTestCases += ReportTestResult(VerifyUnnormalisedStream(reportTestCases), "elreal<double>", "unnormalised block stream");
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
