// division.cpp: floatcascade division accuracy and special values, with addition and subtraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2054) found that floatcascade<N>::operator/= took two
// quotient digits, so floatcascade<3> and <4> divided at double-double accuracy; that x / +0
// was -inf, because sign() of a zero is 0; that an overflowing quotient left NaN in the tail;
// and that to_string printed garbage for |x| below ~1e-308. The reference for the arithmetic
// is exact dyadic-rational arithmetic on the components.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <universal/internal/floatcascade/floatcascade.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

template<size_t N>
dyadic exact_value(const floatcascade<N>& a) {
	dyadic acc;
	for (size_t i = 0; i < N; ++i) acc = acc + dyadic::from_double(a[i]);
	return acc;
}

// |error| <= |reference| * 2^-bits
bool within(const dyadic& error, const dyadic& reference, int bits) {
	return !less_in_magnitude(reference, dyadic(error.numerator, error.scale + bits));
}

// the quotient q of a / b satisfies |q b - a| <= |a| 2^-(53 N - 4): N components, less a few bits
template<size_t N>
int VerifyDivisionAccuracy(bool reportTestCases) {
	constexpr int bits = 53 * static_cast<int>(N) - 4;
	int nrOfFailedTests = 0;
	std::mt19937_64 rng(2054 + N);
	std::uniform_real_distribution<double> mantissa(0.5, 1.0);
	std::uniform_int_distribution<int> exponent(-30, 30);
	auto random_cascade = [&]() {
		floatcascade<N> c;
		c[0] = std::ldexp(mantissa(rng), exponent(rng));
		for (size_t i = 1; i < N; ++i) c[i] = std::ldexp(c[i - 1], -60) * mantissa(rng);
		return c;
	};
	auto check = [&](const floatcascade<N>& a, const floatcascade<N>& b) {
		floatcascade<N> q(a);
		q /= b;
		const dyadic A = exact_value(a);
		const dyadic residual = exact_value(q) * exact_value(b) - A;
		if (!within(residual, A, bits)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: floatcascade<" << N << "> " << a[0] << " / " << b[0] << " is short of " << bits << " bits\n";
		}
	};
	check(floatcascade<N>(1.0), floatcascade<N>(3.0));
	check(floatcascade<N>(2.0), floatcascade<N>(7.0));
	check(floatcascade<N>(-10.0), floatcascade<N>(3.0));
	for (int t = 0; t < 200 && nrOfFailedTests < 10; ++t) check(random_cascade(), random_cascade());
	return nrOfFailedTests;
}

// sums whose exact value fits the N components come out exact; the others within 2^-(53 N - 4)
template<size_t N>
int VerifyAddSubtract(bool reportTestCases) {
	constexpr int bits = 53 * static_cast<int>(N) - 4;
	int nrOfFailedTests = 0;
	auto check = [&](const floatcascade<N>& a, const floatcascade<N>& b, bool exact) {
		floatcascade<N> s(a), d(a);
		s += b;
		d -= b;
		const dyadic S = exact_value(a) + exact_value(b), D = exact_value(a) - exact_value(b);
		const bool ok = exact ? (exact_value(s) == S && exact_value(d) == D)
		                      : (within(exact_value(s) - S, S, bits) && within(exact_value(d) - D, D, bits));
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: floatcascade<" << N << "> " << a[0] << " +- " << b[0] << '\n';
		}
	};
	floatcascade<N> a(1.0), b(std::ldexp(1.0, -60));
	check(a, b, true);                                  // 1 + 2^-60: two components
	check(floatcascade<N>(3.0), floatcascade<N>(-1.0), true);
	floatcascade<N> third(1.0);
	third /= floatcascade<N>(3.0);
	check(third, floatcascade<N>(1.0), false);
	check(third, third, false);
	return nrOfFailedTests;
}

template<size_t N>
int VerifySpecialValues(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const double inf = std::numeric_limits<double>::infinity();
	auto expect = [&](const floatcascade<N>& r, double lead, const char* label) {
		bool ok = (std::isnan(lead) ? std::isnan(r[0]) : (r[0] == lead && std::signbit(r[0]) == std::signbit(lead)));
		for (size_t i = 1; i < N; ++i) ok = ok && (r[i] == 0.0);   // a clean tail: no NaN, no residue
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: floatcascade<" << N << "> " << label << " = (" << r[0] << ", " << r[1] << ", ...)\n";
		}
	};
	auto div = [](double x, double y) { floatcascade<N> a(x); a /= floatcascade<N>(y); return a; };
	expect(div(1.0, 0.0), inf, "1 / +0");
	expect(div(1.0, -0.0), -inf, "1 / -0");
	expect(div(-1.0, 0.0), -inf, "-1 / +0");
	expect(div(0.0, 0.0), std::numeric_limits<double>::quiet_NaN(), "0 / 0");
	expect(div(1e308, 1e-10), inf, "1e308 / 1e-10");
	expect(div(-1e308, 1e-10), -inf, "-1e308 / 1e-10");
	expect(div(inf, 3.0), inf, "inf / 3");
	expect(div(3.0, inf), 0.0, "3 / inf");
	expect(div(-3.0, inf), -0.0, "-3 / inf");
	return nrOfFailedTests;
}

// to_string of values below double's normal range
int VerifyTinyToString(bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (double v : { 1e-310, 4.9e-324, -2.5e-315, 1e-300 }) {
		const floatcascade<4> a(v);
		const std::string s = a.to_string(10, 0, false, true);
		const double back = std::strtod(s.c_str(), nullptr);   // std::stod throws out_of_range on a subnormal
		if (!(std::abs(back - v) <= std::abs(v) * 1e-9)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: to_string(" << v << ") = \"" << s << "\"\n";
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

	std::string test_suite  = "floatcascade division, addition and special values";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyDivisionAccuracy<4>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAccuracy<2>(reportTestCases), "floatcascade<2>", "division accuracy");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAccuracy<3>(reportTestCases), "floatcascade<3>", "division accuracy");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionAccuracy<4>(reportTestCases), "floatcascade<4>", "division accuracy");
	nrOfFailedTestCases += ReportTestResult(VerifyAddSubtract<2>(reportTestCases), "floatcascade<2>", "addition/subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyAddSubtract<3>(reportTestCases), "floatcascade<3>", "addition/subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyAddSubtract<4>(reportTestCases), "floatcascade<4>", "addition/subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<2>(reportTestCases), "floatcascade<2>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<3>(reportTestCases), "floatcascade<3>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<4>(reportTestCases), "floatcascade<4>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifyTinyToString(reportTestCases), "floatcascade<4>", "to_string below 1e-308");
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
