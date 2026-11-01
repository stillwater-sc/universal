// numeric_limits.cpp: std::numeric_limits of the multi-double types (#1882, #1888, #1889, #1891)
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
//   traits    has_denorm = denorm_present (below min() precision falls off gradually down to
//             denorm_min()), has_denorm_loss as double's, is_bounded, round_to_nearest.
//   specials  infinity(), quiet_NaN() and signaling_NaN() carry the code in the leading limb
//             and zeros in the rest, as setinf()/setnan() and SpecificValue build them.
// Before #1882 dd's max() rounded to inf, the cascades' min() was denorm_min(), min_exponent was
// double's in every type, and the cascades' epsilon() came from truncated decimal literals.
// Before #1888 dd/qd reported denorm_absent, unbounded and round_toward_zero, and before #1889
// dd_cascade and qd_cascade put the special value in every limb.
#include <universal/utility/directives.hpp>
#include <bit>
#include <cfloat>
#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>
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

	// the traits are the same in all five types (#1888)
	template<typename Real>
	constexpr bool shared_traits() {
		using L = std::numeric_limits<Real>;
		return L::has_denorm == std::denorm_present && L::has_denorm_loss == std::numeric_limits<double>::has_denorm_loss
		    && L::is_bounded && L::round_style == std::round_to_nearest;
	}
	static_assert(shared_traits<dd>(),         "dd traits");
	static_assert(shared_traits<qd>(),         "qd traits");
	static_assert(shared_traits<dd_cascade>(), "dd_cascade traits");
	static_assert(shared_traits<td_cascade>(), "td_cascade traits");
	static_assert(shared_traits<qd_cascade>(), "qd_cascade traits");
	// special values carry the code in the leading limb only (#1889)
	static_assert(std::numeric_limits<dd_cascade>::infinity()[1] == 0.0, "dd_cascade infinity() tail");
	static_assert(std::numeric_limits<qd_cascade>::infinity()[3] == 0.0, "qd_cascade infinity() tail");

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

		// traits, the same in all five types (#1888)
		if (L::has_denorm != std::denorm_present) fail("has_denorm is not denorm_present");
		if (L::has_denorm_loss != std::numeric_limits<double>::has_denorm_loss) fail("has_denorm_loss is not double's");
		if (!L::is_bounded) fail("is_bounded is false");
		if (L::round_style != std::round_to_nearest) fail("round_style is not round_to_nearest");
		// below min() the value degrades gradually: min() / 2 is not zero, and denorm_min() is held
		if ((mn * Real(0.5))[0] != std::ldexp(mn[0], -1)) fail("min() / 2 is not held");
		if (L::denorm_min()[0] != std::numeric_limits<double>::denorm_min() || !tail_is_zero(L::denorm_min())) fail("denorm_min() is not the double subnormal");

		// special values: the code in the leading limb, zeros in the rest (#1889)
		auto bits = [](double d) { return std::bit_cast<std::uint64_t>(d); };
		auto same_bits = [&](const Real& a, const Real& b) {
			for (unsigned i = 0; i < N; ++i) if (bits(a[i]) != bits(b[i])) return false;
			return true;
		};
		const Real inf = L::infinity();
		if (inf[0] != std::numeric_limits<double>::infinity() || !tail_is_zero(inf)) fail("infinity() is not (inf, 0, ...)");
		if (!inf.isinf() || inf.isnan()) fail("infinity() is not isinf()");
		if (!same_bits(inf, Real(SpecificValue::infpos))) fail("infinity() != SpecificValue::infpos");
		Real ninf = -inf;
		if (ninf[0] != -std::numeric_limits<double>::infinity() || !tail_is_zero(ninf) || !ninf.isinf()) fail("-infinity() is not (-inf, 0, ...)");
		const Real qnan = L::quiet_NaN();
		if (!std::isnan(qnan[0]) || !tail_is_zero(qnan) || !qnan.isnan()) fail("quiet_NaN() is not (nan, 0, ...)");
		if (!same_bits(qnan, Real(SpecificValue::qnan))) fail("quiet_NaN() != SpecificValue::qnan");
		const Real snan = L::signaling_NaN();
		if (!std::isnan(snan[0]) || !tail_is_zero(snan) || !snan.isnan()) fail("signaling_NaN() is not (nan, 0, ...)");
		if (!same_bits(snan, Real(SpecificValue::snan))) fail("signaling_NaN() != SpecificValue::snan");
		// infinity stays infinity through arithmetic with a finite value
		if (!(inf + Real(1.0)).isinf() || (inf + Real(1.0)).isnan()) fail("infinity() + 1 is not inf");
		if (!(inf * Real(2.0)).isinf() || (inf * Real(2.0)).isnan()) fail("infinity() * 2 is not inf");
		if (!(Real(1.0) - inf).isinf() || (Real(1.0) - inf).isnan()) fail("1 - infinity() is not -inf");
		if (double(inf) != std::numeric_limits<double>::infinity()) fail("double(infinity()) is not inf");
		// the cascades' addition used to return NaN for any infinite operand (#1889)
		if (!(inf + inf).isinf() || (inf + inf).isnan()) fail("infinity() + infinity() is not inf");
		if (!(inf - inf).isnan()) fail("infinity() - infinity() is not NaN");
		if (!(qnan + Real(1.0)).isnan()) fail("NaN + 1 is not NaN");
		const Real ovf = mx + mx;
		if (!ovf.isinf() || ovf.isneg() || !tail_is_zero(ovf)) fail("max() + max() is not (inf, 0, ...)");
		Real acc = Real(1.0);
		acc += inf;
		if (!acc.isinf() || !tail_is_zero(acc)) fail("1 += infinity() is not (inf, 0, ...)");
		acc = Real(1.0);
		acc -= inf;
		if (!acc.isinf() || !acc.isneg() || !tail_is_zero(acc)) fail("1 -= infinity() is not (-inf, 0, ...)");

		return nrOfFailures;
	}

	// a multi-double value with the leading limbs (x0, x1) and zeros below
	template<typename Real, unsigned N>
	Real RangeLimbs(double x0, double x1) {
		if constexpr (std::is_same_v<Real, dd> || std::is_same_v<Real, qd>) {
			return Real(x0, x1);
		}
		else {
			floatcascade<N> f;
			f.clear();
			f[0] = x0;
			f[1] = x1;
			return Real(f);
		}
	}

	template<typename Real, unsigned N>
	bool RangeLimbsEqual(const Real& a, const Real& b) {
		for (unsigned i = 0; i < N; ++i) if (a[i] != b[i]) return false;
		return true;
	}

	// addition at the top of the range, where an intermediate two_sum can overflow although the
	// complete sum is finite, or the tail can carry a sum past max() (#1889 cascades, #1891 dd/qd)
	template<typename Real, unsigned N>
	int VerifyRangeCarry(bool reportTestCases, const std::string& type) {
		int nrOfFailures = 0;
		auto fail = [&](const std::string& what) {
			++nrOfFailures;
			if (reportTestCases) std::cerr << "FAIL: " << type << ' ' << what << '\n';
		};
		auto limbs = [](double x0, double x1) { return RangeLimbs<Real, N>(x0, x1); };
		auto is = [&](const Real& r, double x0, double x1) { return RangeLimbsEqual<Real, N>(r, limbs(x0, x1)); };
		// the leading limbs DBL_MAX + 2^970 overflow, the complete sum DBL_MAX + 2^969 does not
		const Real a = limbs(DBL_MAX, -0x1p+969), b = limbs(0x1p+970, 0.0);
		if (!is(a + b, DBL_MAX, 0x1p+969)) fail("(DBL_MAX, -2^969) + 2^970 is not (DBL_MAX, 2^969)");
		if (!is(b + a, DBL_MAX, 0x1p+969)) fail("2^970 + (DBL_MAX, -2^969) is not (DBL_MAX, 2^969)");
		if (!is(a - (-b), DBL_MAX, 0x1p+969)) fail("(DBL_MAX, -2^969) - (-2^970) is not (DBL_MAX, 2^969)");
		if (!is(-a - b, -DBL_MAX, -0x1p+969)) fail("-(DBL_MAX, -2^969) - 2^970 is not -(DBL_MAX, 2^969)");
		Real acc = a;
		acc += b;
		if (!is(acc, DBL_MAX, 0x1p+969)) fail("(DBL_MAX, -2^969) += 2^970 is not (DBL_MAX, 2^969)");
		acc = a;
		acc -= -b;
		if (!is(acc, DBL_MAX, 0x1p+969)) fail("(DBL_MAX, -2^969) -= -2^970 is not (DBL_MAX, 2^969)");
		// the leading limbs DBL_MAX + 2^969 round to DBL_MAX, the complete sum is past max()
		const Real mx = std::numeric_limits<Real>::max();
		const Real over = mx + limbs(0x1p+969, 0.0);
		if (!over.isinf() || over.isneg() || !is(over, over[0], 0.0)) fail("max() + 2^969 is not (inf, 0, ...)");
		const Real under = -mx - limbs(0x1p+969, 0.0);
		if (!under.isinf() || !under.isneg() || !is(under, under[0], 0.0)) fail("-max() - 2^969 is not (-inf, 0, ...)");
		// a finite sum on the half-scale path is exact
		if (!is(limbs(0x1.8p+1022, 0.0) + limbs(0x1p+960, 0.0), 0x1.8p+1022, 0x1p+960)) fail("1.5*2^1022 + 2^960 is not exact");
		if (!is(mx - mx, 0.0, 0.0)) fail("max() - max() is not 0");
		return nrOfFailures;
	}

	template<typename Real, unsigned N>
	int VerifyRangeSubnormalTail(bool reportTestCases, const std::string& type) {
		int nrOfFailures = 0;
		auto fail = [&](const std::string& what) {
			++nrOfFailures;
			if (reportTestCases) std::cerr << "FAIL: " << type << ' ' << what << '\n';
		};
		auto limbs = [](double x0, double x1) { return RangeLimbs<Real, N>(x0, x1); };
		auto is = [&](const Real& r, double x0, double x1) { return RangeLimbsEqual<Real, N>(r, limbs(x0, x1)); };
		// a tail limb quarter scaling would round is added back unscaled: when the leading limbs
		// cancel it is the whole answer
		const Real p = limbs(0x1p+1022, 0x1p-1074), m = limbs(-0x1p+1022, 0x1p-1074);
		if (!is(p + m, 0x1p-1073, 0.0)) fail("(2^1022, 2^-1074) + (-2^1022, 2^-1074) is not 2^-1073");
		if (!is(p - p, 0.0, 0.0)) fail("(2^1022, 2^-1074) - itself is not 0");
		if (!is(limbs(DBL_MAX, 0x1p-1074) - limbs(DBL_MAX, 0.0), 0x1p-1074, 0.0)) fail("(DBL_MAX, 2^-1074) - DBL_MAX is not 2^-1074");
		if (!is(limbs(0x1p+1023, 0.0) + limbs(0x1p-1074, 0.0), 0x1p+1023, 0x1p-1074)) fail("2^1023 + 2^-1074 is not (2^1023, 2^-1074)");
		return nrOfFailures;
	}

	template<typename Real, unsigned N>
	int VerifyRangeTie(bool reportTestCases, const std::string& type) {
		int nrOfFailures = 0;
		auto fail = [&](const std::string& what) {
			++nrOfFailures;
			if (reportTestCases) std::cerr << "FAIL: " << type << ' ' << what << '\n';
		};
		auto limbs = [](double x0, double x1) { return RangeLimbs<Real, N>(x0, x1); };
		auto is = [&](const Real& r, double x0, double x1) { return RangeLimbsEqual<Real, N>(r, limbs(x0, x1)); };
		// a sum whose leading limbs land on the tie DBL_MAX + 2^970: the rest of the sum decides.
		// Below the threshold the value is above max() and rounds to it (#1892 review)
		auto is_max = [&](const Real& r, bool negative) {
			const Real m = std::numeric_limits<Real>::max();
			return negative ? RangeLimbsEqual<Real, N>(r, -m) : RangeLimbsEqual<Real, N>(r, m);
		};
		const Real dmax = limbs(DBL_MAX, 0.0);
		if (!is_max(dmax + limbs(0x1p+970, -0x1p-1074), false)) fail("DBL_MAX + (2^970, -2^-1074) is not max()");
		if (!is_max(-dmax - limbs(0x1p+970, -0x1p-1074), true)) fail("-DBL_MAX - (2^970, -2^-1074) is not -max()");
		const Real tie_up = dmax + limbs(0x1p+970, 0x1p-1074);
		if (!tie_up.isinf() || tie_up.isneg() || !is(tie_up, tie_up[0], 0.0)) fail("DBL_MAX + (2^970, 2^-1074) is not (inf, 0, ...)");
		const Real tie = dmax + limbs(0x1p+970, 0.0);
		if (!tie.isinf() || tie.isneg() || !is(tie, tie[0], 0.0)) fail("DBL_MAX + 2^970 is not (inf, 0, ...)");
		// DBL_MAX + 2^970 - 2^900: dd's 106 bits round it to the threshold, three or more limbs keep it
		const Real near = dmax + limbs(0x1p+970, -0x1p+900);
		if constexpr (N == 2) {
			if (!near.isinf() || near.isneg()) fail("DBL_MAX + (2^970, -2^900) is not inf at two limbs");
		}
		else {
			if (!is_max(near, false)) fail("DBL_MAX + (2^970, -2^900) is not max()");
		}
		return nrOfFailures;
	}

	template<typename Real, unsigned N>
	int VerifyRangeBoundary(bool reportTestCases, const std::string& type) {
		return VerifyRangeCarry<Real, N>(reportTestCases, type)
			+ VerifyRangeSubnormalTail<Real, N>(reportTestCases, type)
			+ VerifyRangeTie<Real, N>(reportTestCases, type);
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
	nrOfFailedTestCases += ReportTestResult(VerifyRangeBoundary<dd, 2>(reportTestCases, "dd"), "dd", "range boundary");
	nrOfFailedTestCases += ReportTestResult(VerifyRangeBoundary<qd, 4>(reportTestCases, "qd"), "qd", "range boundary");
	nrOfFailedTestCases += ReportTestResult(VerifyRangeBoundary<dd_cascade, 2>(reportTestCases, "dd_cascade"), "dd_cascade", "range boundary");
	nrOfFailedTestCases += ReportTestResult(VerifyRangeBoundary<td_cascade, 3>(reportTestCases, "td_cascade"), "td_cascade", "range boundary");
	nrOfFailedTestCases += ReportTestResult(VerifyRangeBoundary<qd_cascade, 4>(reportTestCases, "qd_cascade"), "qd_cascade", "range boundary");
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
