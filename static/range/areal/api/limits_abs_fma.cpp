// limits_abs_fma.cpp: numeric_limits, abs and fma for areal, and what the ubit promises
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep in #1962 found (#1970):
//   - numeric_limits<areal> disagreed with the encoding: min() was minpos, denorm_min() was 1,
//     epsilon() was 0 with the ubit set, digits counted the ubit, the exponents were off, and
//     has_denorm and is_bounded denied the subnormals and the finite range;
//   - abs(areal) could not be called: its definition returned a different type than its
//     declaration, through a constructor areal does not have;
//   - fma ignored the inputs' ubits.
// It also asked what a single areal's ubit promises. The answer, documented in areal_impl.hpp and
// docs/number-systems/areal.md, is a flag: an inexact operand makes the result inexact, but the
// result's interval need not enclose the exact interval result. VerifyFlagSemantics pins that.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include <universal/number/areal/areal.hpp>
#include <universal/number/integer/integer.hpp>   // the exact oracle of the sampled fma
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::areal;

// The limits against the encoding, derived from the format alone (#2010): a sign bit at nbits-1,
// es exponent bits with bias 2^(es-1) - 1 and an exponent field of 0 for the subnormals, fbits =
// nbits - 2 - es fraction bits, and the ubit at bit 0. Every exponent field is a binade; the
// all-ones fraction in the top binade is inf (ubit 0) and NaN (ubit 1), quiet with the sign bit
// clear, signalling with it set. The expected encodings are built bit by bit from that, and the
// values are checked with exact areal arithmetic (doubling and halving by 2, Sterbenz
// subtraction), so the wide configurations are checked without going through double.
template<typename A>
int VerifyLimits(bool reportTestCases) {
	using L = std::numeric_limits<A>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << sw::universal::type_tag(A()) << ' ' << what << '\n'; } };
	auto sameBits = [](const A& x, const A& y) { for (unsigned i = 0; i < A::nbits; ++i) if (x.at(i) != y.at(i)) return false; return true; };
	constexpr unsigned nbits = A::nbits, es = A::es;
	constexpr unsigned fbits = nbits - 2u - es;
	constexpr int bias = (1 << (es - 1u)) - 1;
	constexpr int minScale = 1 - bias;                        // the scale of exponent field 1
	constexpr int maxScale = ((1 << es) - 1) - bias;          // the scale of the all-ones exponent field
	static_assert(fbits == A::fbits, "the fraction is what the sign, the exponent and the ubit leave");

	// they are constant expressions, and exact values
	constexpr A eps = L::epsilon(), mn = L::min(), dmin = L::denorm_min();
	static_assert(!eps.at(0) && !mn.at(0) && !dmin.at(0), "the limits are exact values");

	// the expected encodings
	A expMin, expDmin, expMax, expInf, expQnan, expSnan;
	expMin.set(fbits + 1u);                                   // exponent field 1, fraction 0
	expDmin.set(1u);                                          // exponent field 0, fraction lsb
	for (unsigned i = 2; i + 1 < nbits; ++i) expMax.set(i);   // exponent all ones, fraction all ones but its lsb
	for (unsigned i = 1; i + 1 < nbits; ++i) expInf.set(i);   // exponent and fraction all ones, ubit 0
	for (unsigned i = 0; i + 1 < nbits; ++i) expQnan.set(i);  // all ones but the sign
	expSnan = expQnan; expSnan.set(nbits - 1u);               // all ones
	A expLowest = expMax; expLowest.set(nbits - 1u);

	check(sameBits(mn, expMin), "min is exponent field 1 with a zero fraction");
	check(sameBits(dmin, expDmin), "denorm_min is the fraction lsb alone");
	check(sameBits(dmin, A(sw::universal::SpecificValue::minpos)), "denorm_min is minpos");
	check(sameBits(L::max(), expMax), "max is the largest finite encoding");
	check(sameBits(L::lowest(), expLowest), "lowest is max with the sign set");
	check(sameBits(L::infinity(), expInf) && L::infinity().isinf(sw::universal::INF_TYPE_POSITIVE), "infinity is +inf");
	check(sameBits(L::quiet_NaN(), expQnan) && L::quiet_NaN().isnan(sw::universal::NAN_TYPE_QUIET), "quiet_NaN is the quiet NaN");
	check(sameBits(L::signaling_NaN(), expSnan) && L::signaling_NaN().isnan(sw::universal::NAN_TYPE_SIGNALLING), "signaling_NaN is the signalling NaN");
	check(L::lowest() == -L::max(), "lowest is -max");

	// the values, by exact arithmetic
	const A one(1.0f), two(2.0f), half(0.5f);
	{
		A x = one;   // min is 2^minScale
		if constexpr (minScale >= 0) { for (int k = 0; k < minScale; ++k) x *= two; }
		else { for (int k = 0; k < -minScale; ++k) x *= half; }
		check(sameBits(x, mn), "min is 2^(1 - bias)");
	}
	{
		A x = mn;    // the subnormals are min's binade stepped down by fbits halvings
		for (unsigned k = 0; k < fbits; ++k) x *= half;
		check(sameBits(x, dmin), "denorm_min is min * 2^-fbits");
		A below;     // exponent field 0, every fraction bit set: the largest subnormal
		for (unsigned i = 1; i <= fbits; ++i) below.set(i);
		check(sameBits(below + dmin, mn), "the largest subnormal plus denorm_min is min");
	}
	{
		A next = one;
		next.set(1);   // the fraction's least significant bit: the value after 1
		const A step = next - one;
		check(!step.at(0) && sameBits(step, eps), "epsilon is the distance from 1 to the next value");
		if constexpr (es == 1) {
			// bias 0: 1.0 is the subnormal 0.1 * 2^1, so the step is the subnormal spacing
			check(sameBits(eps, dmin), "es = 1: epsilon is denorm_min, 2^(1 - fbits)");
		}
		else {
			A x = eps;
			for (unsigned k = 0; k < fbits; ++k) x *= two;
			check(sameBits(x, one), "epsilon is 2^-fbits");
		}
	}
	if constexpr (maxScale < 1000 && minScale - static_cast<int>(fbits) > -1074) {
		// within double's range: the same values as doubles
		check(double(mn) == std::ldexp(1.0, minScale), "double(min)");
		check(double(dmin) == std::ldexp(1.0, minScale - static_cast<int>(fbits)), "double(denorm_min)");
		check(double(L::max()) == std::ldexp(2.0 - std::ldexp(1.0, 1 - static_cast<int>(fbits)), maxScale), "double(max) is (2 - 2^(1-fbits)) * 2^maxScale");
		check(double(L::lowest()) == -double(L::max()), "double(lowest)");
	}

	// the traits
	const double log10of2 = std::log10(2.0);
	check(L::digits == static_cast<int>(fbits) + 1, "digits is the significand of an exact value, without the ubit");
	check(L::digits10 == static_cast<int>(std::floor((L::digits - 1) * log10of2)), "digits10 = floor((digits - 1) log10 2)");
	check(L::max_digits10 == static_cast<int>(std::ceil(L::digits * log10of2)) + 1, "max_digits10 = ceil(digits log10 2) + 1");
	check(L::min_exponent == minScale + 1, "2^(min_exponent - 1) is min()");
	// max = (1 - 2^-fbits) * 2^(maxScale + 1) lies in [2^(max_exponent - 1), 2^max_exponent)
	check(L::max_exponent == maxScale + 1, "max lies in [2^(max_exponent - 1), 2^max_exponent)");
	// the smallest n with 10^n >= min, and the largest n with 10^n <= max
	check(L::min_exponent10 == static_cast<int>(std::ceil(minScale * log10of2)), "min_exponent10 = ceil(log10 min)");
	const double log10max = (maxScale + 1) * log10of2 + std::log1p(-std::ldexp(1.0, -static_cast<int>(fbits))) / std::log(10.0);
	check(L::max_exponent10 == static_cast<int>(std::floor(log10max)), "max_exponent10 = floor(log10 max)");
	check(L::has_infinity && L::has_quiet_NaN && L::has_signaling_NaN, "has inf and both NaNs");
	check(L::has_denorm == std::denorm_present && L::is_bounded, "has_denorm and is_bounded");
	check(L::round_style == std::round_toward_zero && double(L::round_error()) == 1.0, "truncation: round_toward_zero, an error of up to one ulp");
	return fails;
}

// abs over every encoding: the sign bit off, the value bits and the ubit kept, NaN as is
template<typename A>
int VerifyAbs(bool reportTestCases) {
	int fails = 0, reported = 0;
	A x;
	for (std::uint64_t raw = 0; raw < (std::uint64_t(1) << A::nbits); ++raw) {
		x.setbits(raw);
		const A a = abs(x);
		bool ok;
		if (x.isnan()) {
			ok = a.isnan();
		}
		else {
			ok = !a.at(A::nbits - 1) && !std::signbit(double(a));   // the sign bit off
			for (unsigned i = 0; i + 1 < A::nbits; ++i) ok = ok && (a.at(i) == x.at(i));   // the rest, ubit included, kept
			ok = ok && (x.isinf() ? a.isinf() : double(a) == std::fabs(double(x)));
		}
		if (!ok) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL abs(" << sw::universal::to_binary(x) << ") = " << sw::universal::to_binary(a) << '\n';
		}
	}
	return fails;
}

// The flag rule fma follows (#2010), as + - * / do: the result is flagged inexact when the fused
// value of the represented values was truncated, or when an inexact operand contributed to it. A
// factor that is an exact zero makes the product exactly 0, so the other factor's ubit drops
// nothing; the addend's ubit always carries. (The (0, minpos) encoding, the ubit alone, is not
// an exact zero.)
template<typename A>
bool FmaCarriesFlag(const A& a, const A& b, const A& c) {
	const bool exactZeroProduct = (a.iszero() || b.iszero());
	return (!exactZeroProduct && (a.at(0) || b.at(0))) || c.at(0);
}

// fma over every triple of finite values of an 8-bit areal. With at most 6 significand bits and
// scales within 2^-18 .. 2^16, a*b + c is exact in double, which is the reference. The result,
// ubit cleared, must be the truncation of the exact value toward zero, and the ubit must be set
// exactly when that dropped bits or an operand's flag carries.
template<typename A>
int VerifyFmaExhaustive(bool reportTestCases) {
	static_assert(A::nbits <= 8, "exhaustive over triples");
	int fails = 0, reported = 0;
	std::vector<A> values;
	A v;
	for (unsigned raw = 0; raw < (1u << A::nbits); ++raw) {
		v.setbits(raw);
		if (!v.isnan() && !v.isinf()) values.push_back(v);
	}
	const double maxpos = double(A(sw::universal::SpecificValue::maxpos));
	for (const A& a : values) for (const A& b : values) for (const A& c : values) {
		const A r = fma(a, b, c);
		const double exact = double(a) * double(b) + double(c);
		bool ok;
		if (std::fabs(exact) > maxpos) {
			ok = !r.isinf() && !r.isnan() && r.at(0) && std::fabs(double(r)) == maxpos;   // saturates to (maxpos, inf)
		}
		else {
			A t(r);
			t.reset(0);
			const double tv = double(t);
			const bool truncated = (tv != exact);
			ok = !r.isinf() && !r.isnan()
				&& std::fabs(tv) <= std::fabs(exact) && (tv == 0.0 || std::signbit(tv) == std::signbit(exact))
				&& r.at(0) == (truncated || FmaCarriesFlag(a, b, c));
			if (ok && truncated) {
				// and the exact value lies in the open interval the encoding names
				A nxt(t);   // the next encoding in magnitude bounds the interval
				unsigned i = 1;
				while (i + 1 < A::nbits && nxt.at(i)) { nxt.reset(i); ++i; }
				nxt.set(i);
				ok = std::fabs(exact) < std::fabs(double(nxt));
			}
		}
		if (!ok) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL " << sw::universal::type_tag(A()) << " fma(" << double(a) << (a.at(0) ? "~" : "") << ", " << double(b) << (b.at(0) ? "~" : "") << ", " << double(c) << (c.at(0) ? "~" : "") << ") = " << sw::universal::to_binary(r) << ", exact " << exact << '\n';
		}
	}
	return fails;
}

// an exact integer image of a value on the grid of 2^-S: v * 2^S
template<typename I>
I ScaledInteger(double v, int S) {
	int e = 0;
	const double m = std::frexp(v, &e);                                 // v = m * 2^e, 0.5 <= |m| < 1
	const long long mi = static_cast<long long>(std::ldexp(std::fabs(m), 53));   // |v| = mi * 2^(e - 53)
	I x(mi);
	const int sh = e - 53 + S;
	if (sh >= 0) x <<= sh; else x >>= -sh;                               // exact: v is a multiple of 2^-S
	return (v < 0.0 ? -x : x);
}

// fma over random encodings of a wider areal, against an exact integer oracle: every value is a
// multiple of minpos = 2^-S, so (a*b + c) * 2^(2S) is an integer, and so is every grid value. The
// operands are drawn as raw encodings, so tiny products meet large addends -- where a*b + c needs
// more bits than double has. The draws are sequenced (argument evaluation order is unspecified).
template<typename A, unsigned obits>
int VerifyFmaSampled(bool reportTestCases, int nrSamples, std::uint64_t seed) {
	using I = sw::universal::integer<obits, std::uint32_t>;
	constexpr int S = -A::MIN_EXP_SUBNORMAL;
	int fails = 0, reported = 0;
	std::mt19937_64 rng(seed);
	const A maxpos(sw::universal::SpecificValue::maxpos);
	const I maxposI = ScaledInteger<I>(double(maxpos), 2 * S);
	auto draw = [&]() {
		A x;
		do { x.setbits(rng()); } while (x.isnan() || x.isinf());
		return x;
	};
	for (int t = 0; t < nrSamples; ++t) {
		const A a = draw();
		const A b = draw();
		const A c = draw();
		const A r = fma(a, b, c);
		const I exact = ScaledInteger<I>(double(a), S) * ScaledInteger<I>(double(b), S) + ScaledInteger<I>(double(c), 2 * S);
		const I absExact = (exact < I(0) ? -exact : exact);
		bool ok;
		if (absExact > maxposI) {
			A tr(abs(r));
			tr.reset(0);
			ok = !r.isinf() && !r.isnan() && r.at(0) && tr == maxpos;   // saturates to (maxpos, inf)
		}
		else {
			A tr(r);
			tr.reset(0);
			const I trI = ScaledInteger<I>(double(tr), 2 * S);
			const I absTr = (trI < I(0) ? -trI : trI);
			const bool truncated = (trI != exact);
			ok = !r.isinf() && !r.isnan() && absTr <= absExact && (tr.iszero() || (tr.sign() == (exact < I(0))))
				&& r.at(0) == (truncated || FmaCarriesFlag(a, b, c));
			if (ok && truncated && !(abs(tr) == maxpos)) {
				A nxt(tr);   // the next encoding in magnitude bounds the interval
				unsigned i = 1;
				while (i + 1 < A::nbits && nxt.at(i)) { nxt.reset(i); ++i; }
				nxt.set(i);
				const I nI = ScaledInteger<I>(double(nxt), 2 * S);
				ok = absExact < (nI < I(0) ? -nI : nI);
			}
		}
		if (!ok) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL " << sw::universal::type_tag(A()) << " fma(" << sw::universal::to_binary(a) << ", " << sw::universal::to_binary(b) << ", " << sw::universal::to_binary(c) << ") = " << sw::universal::to_binary(r) << '\n';
		}
	}
	return fails;
}

// the cases the sweeps reported, the special operands, and the double-rounding edge
template<typename A>
int VerifyFmaCases(bool reportTestCases) {
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << sw::universal::type_tag(A()) << ' ' << what << '\n'; } };
	auto sameBits = [](const A& x, const A& y) { for (unsigned i = 0; i < A::nbits; ++i) if (x.at(i) != y.at(i)) return false; return true; };
	const A zero(0.0f), one(1.0f), two(2.0f), three(3.0f), half(0.5f), tenth(0.1f);
	const A inf(INFINITY), ninf(-INFINITY), nan(NAN);
	A ubitZero;
	ubitZero.setbits(0x1u);   // (0, minpos)
	check(tenth.at(0), "0.1 is inexact");

	// zero products (#2010): an exact zero factor drops nothing
	check(sameBits(fma(tenth, zero, zero), zero), "fma(0.1~, 0, 0) is an exact +0");
	check(sameBits(fma(zero, tenth, zero), zero), "fma(0, 0.1~, 0) is an exact +0");
	check(sameBits(fma(tenth, zero, zero), tenth * zero + zero), "fma(0.1~, 0, 0) agrees with 0.1~ * 0 + 0");
	check(sameBits(fma(tenth, zero, half), half), "fma(0.1~, 0, 0.5) is an exact 0.5");
	check(sameBits(fma(zero, zero, tenth), tenth), "fma(0, 0, 0.1~) is 0.1~, still inexact");
	check(fma(ubitZero, one, zero).at(0), "the (0, minpos) factor is not an exact zero");
	check(fma(tenth, one, zero).at(0) && (tenth * one + zero).at(0), "fma(0.1~, 1, 0) is as inexact as 0.1~ * 1 + 0");

	// inf and NaN operands
	check(fma(inf, two, three).isinf(sw::universal::INF_TYPE_POSITIVE), "inf * 2 + 3 = +inf");
	check(fma(ninf, two, three).isinf(sw::universal::INF_TYPE_NEGATIVE), "-inf * 2 + 3 = -inf");
	check(fma(inf, -two, three).isinf(sw::universal::INF_TYPE_NEGATIVE), "inf * -2 + 3 = -inf");
	check(fma(two, three, inf).isinf(sw::universal::INF_TYPE_POSITIVE), "2 * 3 + inf = +inf");
	check(fma(two, three, ninf).isinf(sw::universal::INF_TYPE_NEGATIVE), "2 * 3 - inf = -inf");
	check(fma(inf, one, inf).isinf(sw::universal::INF_TYPE_POSITIVE), "inf * 1 + inf = +inf");
	check(fma(tenth, inf, one).isinf(sw::universal::INF_TYPE_POSITIVE), "an inexact factor times inf is inf, not NaN");
	check(fma(tenth, one, inf).isinf(sw::universal::INF_TYPE_POSITIVE), "inf addend with an inexact factor is inf, not NaN");
	check(fma(inf, zero, one).isnan(), "inf * 0 + 1 is NaN");
	check(fma(zero, inf, one).isnan(), "0 * inf + 1 is NaN");
	check(fma(inf, one, ninf).isnan(), "inf * 1 - inf is NaN");
	check(fma(nan, one, one).isnan() && fma(one, nan, one).isnan() && fma(one, one, nan).isnan(), "a NaN operand gives NaN");
	check(fma(nan, zero, zero).isnan() && fma(zero, zero, nan).isnan(), "a NaN operand gives NaN with zero operands");
	{
		const A maxpos(sw::universal::SpecificValue::maxpos);
		const A r = fma(maxpos, two, zero);
		A t(r);
		t.reset(0);
		check(!r.isinf() && r.at(0) && t == maxpos, "maxpos * 2 saturates to (maxpos, inf)");
	}

	// a*b + c past double's precision: a tiny product meets a large addend. double rounds the sum
	// back onto the addend, and the areal must still see the bits it dropped (#2010).
	if constexpr (A::nbits == 16 && A::es == 5) {
		const A mp(sw::universal::SpecificValue::minpos);   // 2^-24: mp * mp = 2^-48
		const A big(32768.0f), k(1024.0f), below1024(512.0f);
		A exp1;   // 2^15 with its ubit: (2^15, 2^15 + ulp)
		exp1 = big; exp1.set(0);
		check(sameBits(fma(mp, mp, big), exp1), "fma(minpos, minpos, 2^15) = (2^15, 2^15 + ulp)");
		A exp2;   // the largest value below 2^10 with its ubit: (2^10 - ulp, 2^10)
		exp2 = below1024;
		for (unsigned i = 0; i <= A::fbits; ++i) exp2.set(i);
		check(sameBits(fma(-mp, mp, k), exp2), "fma(-minpos, minpos, 2^10) = (2^10 - ulp, 2^10)");
		check(sameBits(fma(mp, mp, -k), -exp2), "fma(minpos, minpos, -2^10) = (-2^10, -2^10 + ulp)");
		A exp3 = -k; exp3.set(0);
		check(sameBits(fma(-mp, mp, -k), exp3), "fma(-minpos, minpos, -2^10) = (-2^10 - ulp, -2^10)");
	}
	return fails;
}

// A single areal's ubit is a flag (#1970): an inexact operand makes the result inexact, and a
// result from exact operands names the interval holding the exact value. It does not promise that
// the result's interval encloses the exact interval result: (0, minpos) - minpos is flagged, but
// the true range (-minpos, 0) is not the interval it names. tile_interval is the enclosure.
int VerifyFlagSemantics(bool reportTestCases) {
	using A = areal<16, 5>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	A zeroToMinpos;
	zeroToMinpos.setbits(0x0001u);   // the ubit alone: (0, minpos)
	const A minpos(sw::universal::SpecificValue::minpos);
	const A r = zeroToMinpos - minpos;
	check(r.at(0), "an inexact operand flags the result inexact");
	check(!((A(1.0f) - A(0.5f)).at(0)), "exact operands with a representable result stay exact");
	check((A(1.0f) / A(3.0f)).at(0), "exact operands with an unrepresentable result are flagged");
	return fails;
}

}  // namespace

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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal numeric_limits, abs, fma and ubit semantics";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << double(std::numeric_limits<areal<16, 5>>::epsilon()) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<8, 2>>(reportTestCases), "areal<8,2>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<16, 5>>(reportTestCases), "areal<16,5>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<32, 8>>(reportTestCases), "areal<32,8>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<64, 11>>(reportTestCases), "areal<64,11>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<16, 5, std::uint8_t>>(reportTestCases), "areal<16,5,uint8>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<8, 1>>(reportTestCases), "areal<8,1>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<16, 1>>(reportTestCases), "areal<16,1>", "numeric_limits");
	// three 32-bit limbs: fraction_ull shifted the top limb by 64, so min() by halving came out 0 under clang
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<72, 8, std::uint32_t>>(reportTestCases), "areal<72,8,uint32>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<100, 10, std::uint16_t>>(reportTestCases), "areal<100,10,uint16>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<80, 11, std::uint8_t>>(reportTestCases), "areal<80,11,uint8>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyAbs<areal<8, 2>>(reportTestCases), "areal<8,2>", "abs");
	nrOfFailedTestCases += ReportTestResult(VerifyAbs<areal<12, 4, std::uint8_t>>(reportTestCases), "areal<12,4,uint8>", "abs");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaExhaustive<areal<8, 2>>(reportTestCases), "areal<8,2>", "fma ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaCases<areal<16, 5>>(reportTestCases), "areal<16,5>", "fma cases");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaCases<areal<32, 8>>(reportTestCases), "areal<32,8>", "fma cases");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaCases<areal<32, 8, std::uint8_t>>(reportTestCases), "areal<32,8,uint8>", "fma cases");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaSampled<areal<16, 5>, 192>(reportTestCases, 20000, 0x2010u), "areal<16,5>", "fma sampled");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaSampled<areal<32, 8>, 640>(reportTestCases, 2000, 0x2010u), "areal<32,8>", "fma sampled");
	nrOfFailedTestCases += ReportTestResult(VerifyFlagSemantics(reportTestCases), "areal<16,5>", "ubit is a flag");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyFmaSampled<areal<32, 8>, 640>(reportTestCases, 20000, 0x2010u + 1u), "areal<32,8>", "fma sampled");
	// every triple is 16M fma calls per configuration
	nrOfFailedTestCases += ReportTestResult(VerifyFmaExhaustive<areal<8, 1>>(reportTestCases), "areal<8,1>", "fma ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaExhaustive<areal<8, 3>>(reportTestCases), "areal<8,3>", "fma ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaExhaustive<areal<8, 4>>(reportTestCases), "areal<8,4>", "fma ubit");
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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
