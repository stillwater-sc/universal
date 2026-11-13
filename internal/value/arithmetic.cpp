// arithmetic.cpp: value<fbits> arithmetic, special values, rounding and integer conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2053) found that value<fbits> + and - were wrong for
// every input (abits = fbits + 5 against module_add's fbits + 4 layout: 1 + 0.5 = 1.75), that a
// zero addend was read as 1.0 * 2^0, that x / 0 was 0, that negation, abs and round_to dropped
// NaN, that every infinity was negative, that -0.0 lost its sign, that integer assignment went
// through long long and integer conversion through float, and that round_to OR-ed the dropped
// bits into the last kept bit instead of rounding to nearest. The quotient also lacked a
// sticky bit. The reference here is the double result rounded once to fbits + 1 bits.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <climits>
#include <iostream>
#include <limits>
#include <string>
#include <universal/internal/value/value.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using namespace sw::universal::internal;

// x rounded to fbits + 1 significant bits, to nearest even (the default rounding mode)
double rne(double x, unsigned fbits) {
	if (x == 0.0 || !std::isfinite(x)) return x;
	int e = 0;
	const double m = std::frexp(x, &e);
	return std::ldexp(std::nearbyint(std::ldexp(m, static_cast<int>(fbits) + 1)), e - static_cast<int>(fbits) - 1);
}

// + - * / on a grid of operands, each against the once-rounded double result. With 10
// fraction bits the sums and products are exact in double, and the quotient is rounded
// twice only where its double lands exactly on a tie, which this grid does not reach.
template<unsigned fbits>
int VerifyArithmetic(bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (int i = -24; i <= 24; ++i) {
		for (int j = -24; j <= 24; ++j) {
			const double a = i * 0.375, b = j * 1.25;
			const value<fbits> x(a), y(b);
			const double got[4]      = { double(x + y), double(x - y), double(x * y), (b != 0.0) ? double(x / y) : 0.0 };
			const double expected[4] = { rne(a + b, fbits), rne(a - b, fbits), rne(a * b, fbits), (b != 0.0) ? rne(a / b, fbits) : 0.0 };
			const char* ops = "+-*/";
			for (int k = 0; k < 4; ++k) {
				if (got[k] != expected[k]) {
					++nrOfFailedTests;
					if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: value<" << fbits << "> " << a << ' ' << ops[k] << ' ' << b << " = " << got[k] << " expected " << expected[k] << '\n';
				}
			}
		}
	}
	value<fbits> p(1.0);
	++p;
	if (double(p) != 2.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: ++value(1) = " << double(p) << '\n';
	}
	return nrOfFailedTests;
}

int VerifySpecialValues(bool reportTestCases) {
	using V = value<10>;
	const double inf = std::numeric_limits<double>::infinity();
	int nrOfFailedTests = 0;
	auto same = [](double a, double b) { return (std::isnan(a) && std::isnan(b)) || (a == b && std::signbit(a) == std::signbit(b)); };
	auto check = [&](double got, double expected, const char* label) {
		if (!same(got, expected)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	const V one(1.0), zero(0.0), mzero(-0.0), pinf(inf), ninf(-inf), nan(std::numeric_limits<double>::quiet_NaN());
	check(double(pinf), inf, "value(+inf)");
	check(double(ninf), -inf, "value(-inf)");
	check(double(mzero), -0.0, "value(-0.0)");
	check(double(one / zero), inf, "1 / 0");
	check(double(-one / zero), -inf, "-1 / 0");
	check(double(zero / zero), std::numeric_limits<double>::quiet_NaN(), "0 / 0");
	check(double(one / pinf), 0.0, "1 / inf");
	check(double(pinf + one), inf, "inf + 1");
	check(double(ninf - one), -inf, "-inf - 1");
	check(double(pinf + ninf), std::numeric_limits<double>::quiet_NaN(), "inf + -inf");
	check(double(pinf * zero), std::numeric_limits<double>::quiet_NaN(), "inf * 0");
	check(double(ninf * one), -inf, "-inf * 1");
	check(double(nan + one), std::numeric_limits<double>::quiet_NaN(), "NaN + 1");
	check(double(-nan), std::numeric_limits<double>::quiet_NaN(), "-NaN");
	check(double(abs(nan)), std::numeric_limits<double>::quiet_NaN(), "abs(NaN)");
	check(double(abs(ninf)), inf, "abs(-inf)");
	check(double(zero + V(1.5)), 1.5, "0 + 1.5");
	check(double(V(1.5) - zero), 1.5, "1.5 - 0");
	check(double(mzero + mzero), -0.0, "-0 + -0");
	// comparisons: NaN unordered, +0 == -0, -inf < x < +inf
	if (nan == nan || !(zero == mzero) || !(ninf < one) || !(one < pinf) || (pinf < one) || (nan < one) || (nan <= nan)) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: value comparisons with special values\n";
	}
	return nrOfFailedTests;
}

// round_to: round to nearest, ties to even, with the carry into the scale
int VerifyRoundTo(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](double v, double expected, const char* label) {
		const value<10> x(v);
		const double got = double(x.round_to<3>());
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: round_to<3>(" << v << ") " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	check(1.0625, 1.0, "guard 0");                 // 1.0001
	check(1.1875, 1.25, "guard 1, sticky");        // 1.0011
	check(1.0625 + 0.0625, 1.125, "exact");        // 1.001
	check(1.1875 - 0.0625 + 0.0, 1.125, "exact");  // 1.001
	check(1.4375, 1.5, "tie, odd: up");            // 1.0111
	check(1.5625, 1.5, "tie, even: down");         // 1.1001 -> 1.100
	check(1.9375, 2.0, "carry into the scale");    // 1.1111
	// no fraction bits: the kept significand is the hidden bit, which is odd, so a tie rounds up
	const value<10> t(1.5), u(1.25);
	if (double(t.round_to<0>()) != 2.0 || double(u.round_to<0>()) != 1.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: round_to<0>(1.5) = " << double(t.round_to<0>()) << ", round_to<0>(1.25) = " << double(u.round_to<0>()) << '\n';
	}
	// NaN and infinity survive rounding
	const value<10> nan(std::numeric_limits<double>::quiet_NaN()), ninf(-std::numeric_limits<double>::infinity());
	if (!nan.round_to<3>().isnan() || !ninf.round_to<3>().isinf() || !ninf.round_to<3>().sign()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: round_to dropped a special value\n";
	}
	return nrOfFailedTests;
}

int VerifyIntegers(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](long long got, long long expected, const std::string& label) {
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	check(value<62>(123456789012345678LL).to_long_long(), 123456789012345678LL, "value<62>(123456789012345678)");   // past double
	check(value<62>(LLONG_MIN).to_long_long(), LLONG_MIN, "value<62>(LLONG_MIN)");
	check(value<62>(-7).to_long_long(), -7, "value<62>(-7)");
	check(value<30>(static_cast<char>(-1)).to_long_long(), std::is_signed_v<char> ? -1 : 255, "value(char(-1))");
	check(value<64>(18000000000000000000ull).to_long_long(), LLONG_MAX, "value(1.8e19 unsigned long) saturates");
	if (double(value<64>(18000000000000000000ull)) != 18000000000000000000.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: value(18000000000000000000ul) is " << double(value<64>(18000000000000000000ull)) << '\n';
	}
	check(value<40>(16777217).to_int(), 16777217, "to_int(2^24 + 1)");   // through float it was 2^24
	check(value<10>(-2.75).to_long_long(), -2, "to_long_long(-2.75) truncates");
	check(value<10>(0.5).to_int(), 0, "to_int(0.5)");
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

	std::string test_suite  = "value arithmetic, special values, rounding and integers";
	std::string test_tag    = "value";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyArithmetic<10>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<10>(reportTestCases), "value<10>", "+ - * /");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<6>(reportTestCases), "value<6>", "+ - * /");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues(reportTestCases), "value<10>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTo(reportTestCases), "value<10>", "round_to");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegers(reportTestCases), "value", "integer conversion");
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
