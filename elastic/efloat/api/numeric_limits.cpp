// numeric_limits.cpp: regression tests for std::numeric_limits<efloat> (#1786)
//
// efloat had no specialization, so std::numeric_limits<efloat<N>> was the primary template:
// is_specialized false and every limit a value-initialized efloat, zero. infinity() was 0.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- every limit at efloat<1>, <4>, <16> and the default <1024>, checked against the
//              arithmetic it describes; overflow and underflow at the range's ends in all four
//              rounding modes; generic code written against numeric_limits.
//   LEVELS 2-4 -- the same; the limits are compile-time facts with nothing to sweep.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	int report(bool ok, bool reportTestCases, const char* what, unsigned nlimbs) {
		if (!ok && reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> " << what << '\n';
		return ok ? 0 : 1;
	}

	// 2^e, exact and carrying no precision
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> power_of_2(int64_t e) {
		sw::universal::efloat<nlimbs> v(1);
		v.setexponent(e);
		return v;
	}

	// the traits: digits, the decimal digit counts by the float/double formulas, and the flags
	template<unsigned nlimbs>
	int VerifyTraits(bool reportTestCases) {
		using Limits = std::numeric_limits<sw::universal::efloat<nlimbs>>;
		int fails = 0;
		const long double log10_2 = std::log10(2.0L);
		const int digits10     = static_cast<int>(std::floor((Limits::digits - 1) * log10_2));
		const int max_digits10 = static_cast<int>(std::ceil(1 + Limits::digits * log10_2));
		fails += report(Limits::is_specialized, reportTestCases, "is_specialized", nlimbs);
		fails += report(Limits::digits == static_cast<int>(nlimbs * 32), reportTestCases, "digits", nlimbs);
		fails += report(Limits::digits10 == digits10, reportTestCases, "digits10", nlimbs);
		fails += report(Limits::max_digits10 == max_digits10, reportTestCases, "max_digits10", nlimbs);
		fails += report(Limits::radix == 2 && Limits::is_signed && !Limits::is_integer && !Limits::is_exact,
		                reportTestCases, "radix and kind", nlimbs);
		fails += report(Limits::has_infinity && Limits::has_quiet_NaN && Limits::has_signaling_NaN,
		                reportTestCases, "special values", nlimbs);
		fails += report(Limits::has_denorm == std::denorm_absent && !Limits::has_denorm_loss, reportTestCases, "denorm", nlimbs);
		fails += report(Limits::round_style == std::round_to_nearest, reportTestCases, "round_style", nlimbs);
		// the range does not fit in an int: every int power of 2 is inside it
		fails += report(Limits::max_exponent == std::numeric_limits<int>::max() && Limits::min_exponent == -std::numeric_limits<int>::max(),
		                reportTestCases, "exponent range", nlimbs);
		return fails;
	}

	template<unsigned nlimbs>
	int VerifySpecialValues(bool reportTestCases) {
		using Efloat = sw::universal::efloat<nlimbs>;
		using Limits = std::numeric_limits<Efloat>;
		int fails = 0;
		const Efloat inf = Limits::infinity();
		fails += report(inf.isinf() && inf.sign() == 1 && !inf.carries_precision(), reportTestCases, "infinity()", nlimbs);
		fails += report((2 * inf).isinf() && (-2 * inf).isinf() && (-2 * inf).sign() == -1, reportTestCases, "2 * infinity()", nlimbs);
		fails += report(-inf < Efloat(-1.0e300) && Efloat(1.0e300) < inf, reportTestCases, "infinity() order", nlimbs);
		fails += report(Limits::quiet_NaN().isqnan() && !Limits::quiet_NaN().issnan(), reportTestCases, "quiet_NaN()", nlimbs);
		fails += report(Limits::signaling_NaN().issnan(), reportTestCases, "signaling_NaN()", nlimbs);
		fails += report(Limits::quiet_NaN() != Limits::quiet_NaN(), reportTestCases, "NaN != NaN", nlimbs);
		return fails;
	}

	// epsilon() is the gap above 1 at `digits` bits, and carries no precision of its own
	template<unsigned nlimbs>
	int VerifyEpsilon(bool reportTestCases) {
		using Efloat = sw::universal::efloat<nlimbs>;
		using Limits = std::numeric_limits<Efloat>;
		int fails = 0;
		const Efloat eps = Limits::epsilon();
		fails += report(eps == power_of_2<nlimbs>(1 - Limits::digits) && !eps.carries_precision(), reportTestCases, "epsilon() value", nlimbs);
		Efloat one(1);
		one.set_precision(static_cast<unsigned>(Limits::digits));
		fails += report(sw::universal::nextafter(one, Efloat(2)) - one == eps, reportTestCases, "nextafter(1, 2) - 1", nlimbs);
		fails += report(one + eps != one, reportTestCases, "1 + epsilon()", nlimbs);
		fails += report(one + eps / 2 == one, reportTestCases, "1 + epsilon()/2 ties to even", nlimbs);
		// an exact epsilon does not widen a narrower operand
		if constexpr (nlimbs > 2) {
			Efloat x(1);
			x.set_precision(64);
			fails += report((x + eps).get_precision() == 64, reportTestCases, "x + epsilon() keeps x's precision", nlimbs);
		}
		fails += report(Limits::round_error() == Efloat(0.5) && !Limits::round_error().carries_precision(), reportTestCases, "round_error()", nlimbs);
		return fails;
	}

	// the range: min() and max() sit at the scales exp2 still reaches, one past them it does not
	template<unsigned nlimbs>
	int VerifyRange(bool reportTestCases) {
		using Efloat = sw::universal::efloat<nlimbs>;
		using Limits = std::numeric_limits<Efloat>;
		constexpr int64_t max_scale = Efloat::max_scale;
		constexpr int64_t min_scale = Efloat::min_scale;
		int fails = 0;
		const Efloat mn = Limits::min(), mx = Limits::max();
		fails += report(mn == power_of_2<nlimbs>(min_scale) && !mn.carries_precision(), reportTestCases, "min()", nlimbs);
		fails += report(Limits::denorm_min() == mn, reportTestCases, "denorm_min() == min()", nlimbs);
		// max() = (2 - 2^(1 - digits)) 2^max_scale: digits one bits, all carried
		bool all_ones = mx.limb_vector().size() == nlimbs;
		for (uint32_t limb : mx.limb_vector()) all_ones = all_ones && limb == 0xFFFF'FFFFu;
		fails += report(all_ones && mx.scale() == max_scale && mx.get_precision() == nlimbs * 32u && mx.sign() == 1,
		                reportTestCases, "max()", nlimbs);
		fails += report(mx > power_of_2<nlimbs>(max_scale) && mx < 2 * power_of_2<nlimbs>(max_scale), reportTestCases, "max() binade", nlimbs);
		fails += report(Limits::lowest() == -mx && Limits::lowest().sign() == -1, reportTestCases, "lowest()", nlimbs);
		// the math library's range is the same one: exp2 of an integer is exact inside it, and
		// returns 0 or inf outside it
		fails += report(sw::universal::exp2(Efloat(static_cast<long long>(max_scale))) == power_of_2<nlimbs>(max_scale),
		                reportTestCases, "exp2(max_scale)", nlimbs);
		fails += report(sw::universal::exp2(Efloat(static_cast<long long>(max_scale)) + 1).isinf(), reportTestCases, "exp2(max_scale + 1)", nlimbs);
		fails += report(sw::universal::exp2(Efloat(static_cast<long long>(min_scale))) == mn, reportTestCases, "exp2(min_scale)", nlimbs);
		fails += report(sw::universal::exp2(Efloat(static_cast<long long>(min_scale)) - 1).iszero(), reportTestCases, "exp2(min_scale - 1)", nlimbs);
		return fails;
	}

	// the arithmetic keeps to the range the limits report (#1786): a result past max() overflows
	// and one below min() underflows, in each mode as IEEE-754 does without subnormals. Before,
	// max() * 2 stayed finite and min() / 2 positive, and the int64_t exponent could wrap.
	template<unsigned nlimbs>
	int VerifyRangeEnforcement(bool reportTestCases) {
		using namespace sw::universal;
		using Efloat = efloat<nlimbs>;
		using Limits = std::numeric_limits<Efloat>;
		int fails = 0;
		const RoundingMode saved = efloat_rounding_mode;
		const Efloat mx = Limits::max(), mn = Limits::min(), inf = Limits::infinity();
		// the largest value at a narrower precision, 64 bits (32 for efloat<1>)
		constexpr unsigned narrow = (nlimbs > 2) ? 2u : nlimbs;
		Efloat max64(false, Efloat::max_scale, std::vector<uint32_t>(narrow, 0xFFFF'FFFFu));
		max64.set_precision(32u * narrow);
		const Efloat zero(0);
		struct Case { RoundingMode mode; bool pos_inf, neg_inf, pos_min, neg_min; };
		// overflow goes to inf, or to the largest value where the mode rounds toward zero from it;
		// underflow goes to 0, or to min() where the mode rounds away from zero
		const Case cases[] = {
			{ RoundingMode::RoundToNearest,      true,  true,  false, false },
			{ RoundingMode::RoundToZero,         false, false, false, false },
			{ RoundingMode::RoundTowardPositive, true,  false, true,  false },
			{ RoundingMode::RoundTowardNegative, false, true,  false, true  },
		};
		for (const Case& c : cases) {
			efloat_rounding_mode = c.mode;
			const char* m = (c.mode == RoundingMode::RoundToNearest) ? "RNE" : (c.mode == RoundingMode::RoundToZero) ? "RTZ"
			              : (c.mode == RoundingMode::RoundTowardPositive) ? "RTP" : "RTN";
			auto expect_over = [&](const Efloat& r, bool to_inf, bool negative, const Efloat& largest, const char* what) {
				const bool ok = to_inf ? (r.isinf() && (r.sign() == -1) == negative) : (r == (negative ? -largest : largest));
				if (!ok && reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> " << m << ' ' << what << '\n';
				return ok ? 0 : 1;
			};
			auto expect_under = [&](const Efloat& r, bool to_min, bool negative, const char* what) {
				const bool ok = to_min ? (r == (negative ? -mn : mn)) : (r.iszero() && (r.sign() == -1) == negative);
				if (!ok && reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> " << m << ' ' << what << '\n';
				return ok ? 0 : 1;
			};
			clear_efloat_exceptions();
			fails += expect_over(mx * 2, c.pos_inf, false, mx, "max() * 2");
			fails += report(has_efloat_exception(ExceptionFlag::Overflow), reportTestCases, "Overflow flag", nlimbs);
			fails += expect_over(-mx * 2, c.neg_inf, true, mx, "-max() * 2");
			fails += expect_over(mx + mx, c.pos_inf, false, mx, "max() + max()");
			fails += expect_over(mx / Limits::round_error(), c.pos_inf, false, mx, "max() / 0.5");
			fails += expect_over(mx * mx, c.pos_inf, false, mx, "max() * max()");
			fails += expect_over(ldexp(mx, 1), c.pos_inf, false, mx, "ldexp(max(), 1)");
			fails += expect_over(max64 * Efloat(1.5), c.pos_inf, false, max64, "max at 64 bits * 1.5");
			fails += expect_over(nextafter(mx, inf), true, false, mx, "nextafter(max(), inf)");   // in every mode
			clear_efloat_exceptions();
			fails += expect_under(mn / 2, c.pos_min, false, "min() / 2");   // the midpoint: to 0 at nearest
			fails += report(has_efloat_exception(ExceptionFlag::Underflow), reportTestCases, "Underflow flag", nlimbs);
			fails += expect_under(-mn / 2, c.neg_min, true, "-min() / 2");
			fails += expect_under(mn * mn, c.pos_min, false, "min() * min()");
			fails += expect_under(mn / mx, c.pos_min, false, "min() / max()");
			fails += expect_under(ldexp(mn, -1), c.pos_min, false, "ldexp(min(), -1)");
			// above the midpoint min() / 2, nearest rounds up to min()
			fails += expect_under(mn * Efloat(0.75), c.pos_min || c.mode == RoundingMode::RoundToNearest, false, "min() * 0.75");
			fails += expect_under(-mn * Efloat(0.75), c.neg_min || c.mode == RoundingMode::RoundToNearest, true, "-min() * 0.75");
			fails += expect_under(nextafter(mn, zero), false, false, "nextafter(min(), 0)");
			// inside the range nothing changes
			fails += report(mx / 2 * 2 == mx && mn * 2 / 2 == mn && mx - mx == zero, reportTestCases, "in range", nlimbs);
			// fma forms its product exactly though the product leaves the range
			fails += report(fma(mx, Efloat(2), -mx) == mx, reportTestCases, "fma(max(), 2, -max())", nlimbs);
			fails += report(fma(mn, Efloat(0.5), mn) == mn * Efloat(1.5), reportTestCases, "fma(min(), 0.5, min())", nlimbs);
			fails += report(fma(mx, mx, -inf).isinf() && fma(mx, mx, -inf).sign() == -1, reportTestCases, "fma(max(), max(), -inf)", nlimbs);
			fails += expect_over(fma(mx, Efloat(2), zero), c.pos_inf, false, mx, "fma(max(), 2, 0)");
			fails += expect_over(fma(mx, mx, mx), c.pos_inf, false, mx, "fma(max(), max(), max())");
			fails += expect_under(fma(mn, Efloat(0.75), zero), c.pos_min || c.mode == RoundingMode::RoundToNearest, false, "fma(min(), 0.75, 0)");
			fails += expect_under(fma(mn, mn, zero), c.pos_min, false, "fma(min(), min(), 0)");
			fails += expect_under(fma(mn, -mn, zero), c.neg_min, true, "fma(min(), -min(), 0)");
		}
		efloat_rounding_mode = saved;
		clear_efloat_exceptions();
		return fails;
	}

	// isone() compared the leading limb with 0x8000'000, 2^27, and was false for every value
	template<unsigned nlimbs>
	int VerifyIsOne(bool reportTestCases) {
		using Efloat = sw::universal::efloat<nlimbs>;
		int fails = 0;
		Efloat wide_one(1);
		wide_one.set_precision(nlimbs * 32u);
		fails += report(Efloat(1).isone() && Efloat(1.0).isone() && wide_one.isone(), reportTestCases, "isone(1)", nlimbs);
		fails += report(!Efloat(0).isone() && !Efloat(-1).isone() && !Efloat(2).isone() && !Efloat(0.5).isone() && !Efloat(1.5).isone(),
		                reportTestCases, "!isone(x != 1)", nlimbs);
		fails += report(!(Efloat(1) + std::numeric_limits<Efloat>::epsilon()).isone(), reportTestCases, "!isone(1 + epsilon())", nlimbs);
		return fails;
	}

	// generic code written against numeric_limits, as a caller would write it
	template<typename Real>
	Real heron_sqrt2() {   // iterate until the step falls within epsilon of the iterate
		Real x(1), two(2);
		for (int i = 0; i < 64; ++i) {
			const Real next = (x + two / x) / 2;
			const Real step = (next > x) ? next - x : x - next;
			x = next;
			if (step <= std::numeric_limits<Real>::epsilon() * x) break;
		}
		return x;
	}

	template<typename Real>
	Real smallest_above(const Real& x) {   // the cmath idiom
		using std::nextafter;
		return nextafter(x, std::numeric_limits<Real>::infinity());
	}

	template<unsigned nlimbs>
	int VerifyGenericUse(bool reportTestCases) {
		using Efloat = sw::universal::efloat<nlimbs>;
		using Limits = std::numeric_limits<Efloat>;
		int fails = 0;
		// Heron's iteration stops within an ulp of sqrt(2) at `digits` bits; with epsilon() == 0
		// it only stopped once two iterates were equal, or after the 64 steps
		Efloat two(2);
		two.set_precision(static_cast<unsigned>(Limits::digits));
		const Efloat root = heron_sqrt2<Efloat>();
		const Efloat error = root - sw::universal::sqrt(two);
		fails += report(sw::universal::abs(error) <= 2 * Limits::epsilon(), reportTestCases, "Heron sqrt(2) within epsilon", nlimbs);
		Efloat x(3);
		x.set_precision(static_cast<unsigned>(Limits::digits));
		const Efloat up = smallest_above(x);
		fails += report(up > x && up - x == 2 * Limits::epsilon(), reportTestCases, "nextafter(3, infinity())", nlimbs);
		return fails;
	}

	template<unsigned nlimbs>
	int VerifyAll(bool reportTestCases) {
		return VerifyTraits<nlimbs>(reportTestCases) + VerifySpecialValues<nlimbs>(reportTestCases)
		     + VerifyEpsilon<nlimbs>(reportTestCases) + VerifyRange<nlimbs>(reportTestCases)
		     + VerifyRangeEnforcement<nlimbs>(reportTestCases)
		     + VerifyIsOne<nlimbs>(reportTestCases) + VerifyGenericUse<nlimbs>(reportTestCases);
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat numeric_limits";
	std::string test_tag            = "numeric_limits";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyAll<4>(reportTestCases), "efloat<4>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAll<1>(reportTestCases), "efloat<1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAll<4>(reportTestCases), "efloat<4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAll<16>(reportTestCases), "efloat<16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTraits<1024>(reportTestCases) + VerifySpecialValues<1024>(reportTestCases)
	                                        + VerifyEpsilon<1024>(reportTestCases) + VerifyRange<1024>(reportTestCases)
	                                        + VerifyRangeEnforcement<1024>(reportTestCases)
	                                        + VerifyIsOne<1024>(reportTestCases), "efloat<1024>", test_tag);
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
