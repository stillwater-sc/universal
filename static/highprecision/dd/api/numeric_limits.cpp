// numeric_limits.cpp: std::numeric_limits of the multi-double types (#1882)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// dd, qd and the floatcascade types dd_cascade, td_cascade and qd_cascade share one convention,
// for an expansion of N doubles:
//   max()     the largest normalized expansion: DBL_MAX, then each limb DBL_MAX * 2^-54k, the
//             largest double below half an ulp of the limb before it. A fixed point of * 1.
//   min()     the smallest value at full precision, 2^(-1022 + 53(N-1)): below it the last limb is
//             subnormal. min() == 2^(min_exponent - 1), as std::numeric_limits requires.
//   epsilon() eps(double)^N / 2 = 2^-(52N + 1), an exact power of two.
// Before #1882 dd's max() rounded to inf, the cascades' min() was denorm_min(), min_exponent was
// double's in every type, and the cascades' epsilon() came from truncated decimal literals.
#include <universal/utility/directives.hpp>
#include <cfloat>
#include <cmath>
#include <limits>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

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

namespace sw { namespace universal {

	// the limits are constexpr: check the integral members and the leading limbs at compile time
	static_assert(std::numeric_limits<dd>::min_exponent         == -968, "dd min_exponent");
	static_assert(std::numeric_limits<dd_cascade>::min_exponent == -968, "dd_cascade min_exponent");
	static_assert(std::numeric_limits<td_cascade>::min_exponent == -915, "td_cascade min_exponent");
	static_assert(std::numeric_limits<qd>::min_exponent         == -862, "qd min_exponent");
	static_assert(std::numeric_limits<qd_cascade>::min_exponent == -862, "qd_cascade min_exponent");
	static_assert(std::numeric_limits<dd>::min()[0] == 0x1p-969 && std::numeric_limits<dd>::max()[1] == 0x1.fffffffffffffp+969, "dd min/max");
	static_assert(std::numeric_limits<qd>::min()[0] == 0x1p-863 && std::numeric_limits<qd>::max()[3] == 0x1.fffffffffffffp+861, "qd min/max");
	static_assert(std::numeric_limits<dd>::epsilon()[0] == std::numeric_limits<dd_cascade>::epsilon()[0], "dd and dd_cascade epsilon agree");
	static_assert(std::numeric_limits<qd>::epsilon()[0] == std::numeric_limits<qd_cascade>::epsilon()[0], "qd and qd_cascade epsilon agree");

	template<typename Real, unsigned N>
	int VerifyNumericLimits(bool reportTestCases, const std::string& type) {
		using L = std::numeric_limits<Real>;
		constexpr int p = std::numeric_limits<double>::digits;
		int nrOfFailures = 0;
		auto fail = [&](const std::string& what) {
			++nrOfFailures;
			if (reportTestCases) std::cerr << "FAIL: " << type << ' ' << what << '\n';
		};
		auto same = [](const Real& a, const Real& b) {
			for (unsigned i = 0; i < N; ++i) if (!(a[i] == b[i])) return false;
			return true;
		};
		auto tail_is_zero = [](const Real& a) {
			for (unsigned i = 1; i < N; ++i) if (a[i] != 0.0) return false;
			return true;
		};

		// max(): DBL_MAX, then DBL_MAX * 2^-54k, each strictly below half an ulp of the limb before it
		const Real mx = L::max();
		for (unsigned i = 0; i < N; ++i) {
			if (mx[i] != std::ldexp(DBL_MAX, -(p + 1) * static_cast<int>(i))) fail("max() limb " + std::to_string(i) + " is " + to_hex(mx[i]));
			if (i > 0) {
				double half_ulp = std::ldexp(mx[i - 1] - std::nextafter(mx[i - 1], 0.0), -1);
				if (!(mx[i] < half_ulp)) fail("max() limb " + std::to_string(i) + " is not below half an ulp of limb " + std::to_string(i - 1));
			}
		}
		if (double(mx) != DBL_MAX) fail("double(max()) is not DBL_MAX");
		if (!same(mx * Real(1.0), mx)) fail("max() * 1 != max()");
		if (!same(Real(1.0) * mx, mx)) fail("1 * max() != max()");
		if (!same(mx / Real(1.0), mx)) fail("max() / 1 != max()");
		if (!same(mx + Real(0.0), mx)) fail("max() + 0 != max()");
		if (!same((mx * Real(0.5)) * Real(2.0), mx)) fail("max() * 0.5 * 2 != max()");
		if (!same(L::lowest(), -mx)) fail("lowest() != -max()");
		if (!same(Real(SpecificValue::maxpos), mx)) fail("SpecificValue::maxpos != max()");
		if (!same(Real(SpecificValue::maxneg), -mx)) fail("SpecificValue::maxneg != -max()");

		// min(): 2^(min_exponent - 1), the smallest value at full precision
		const Real mn = L::min();
		const int expected_min_exponent = std::numeric_limits<double>::min_exponent + p * static_cast<int>(N - 1);
		if (L::min_exponent != expected_min_exponent) fail("min_exponent is " + std::to_string(L::min_exponent));
		if (mn[0] != std::ldexp(1.0, L::min_exponent - 1) || !tail_is_zero(mn)) fail("min() is not 2^(min_exponent - 1)");
		if (!(mn > L::denorm_min())) fail("min() is not above denorm_min()");
		// min_exponent10: the smallest n with 10^n >= min()
		if (!(std::pow(10.0, L::min_exponent10) >= mn[0] && std::pow(10.0, L::min_exponent10 - 1) < mn[0])) fail("min_exponent10 is " + std::to_string(L::min_exponent10));
		// at min() an increment of one epsilon is still held exactly
		const Real eps = L::epsilon();
		Real bumped = mn + mn * eps;
		if (!(bumped - mn == mn * eps) || (mn * eps)[0] == 0.0) fail("min() * (1 + epsilon) is not exact");

		// epsilon(): eps(double)^N / 2 = 2^-(52N + 1)
		if (eps[0] != std::ldexp(1.0, -(p - 1) * static_cast<int>(N) - 1) || !tail_is_zero(eps)) fail("epsilon() is " + to_hex(eps[0]));
		if (!(Real(1.0) + eps > Real(1.0))) fail("1 + epsilon() is not above 1");

		return nrOfFailures;
	}

} }

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "multi-double numeric_limits";
	std::string test_tag    = "numeric_limits";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<dd, 2>(reportTestCases, "dd"), "dd", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<dd, 2>(reportTestCases, "dd"), "dd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<qd, 4>(reportTestCases, "qd"), "qd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<dd_cascade, 2>(reportTestCases, "dd_cascade"), "dd_cascade", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<td_cascade, 3>(reportTestCases, "td_cascade"), "td_cascade", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<qd_cascade, 4>(reportTestCases, "qd_cascade"), "qd_cascade", test_tag);
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
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception." << std::endl;
	return EXIT_FAILURE;
}
