// numeric_limits.cpp: regression tests for std::numeric_limits<efloat> (#1786)
//
// efloat had no specialization, so std::numeric_limits<efloat<N>> was the primary template:
// is_specialized false and every limit a value-initialized efloat, zero. infinity() was 0.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- every limit at efloat<1>, <4>, <16> and the default <1024>, checked against the
//              arithmetic it describes, plus generic code written against numeric_limits.
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
		using sw::universal::efloat_detail::max_scale;
		using sw::universal::efloat_detail::min_scale;
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
