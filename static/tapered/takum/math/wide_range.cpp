// wide_range.cpp: elementary functions past double's range, on the takum formats that reach there
//
// rbits = 4 and 5 span 2^+/-65535 and 2^+/-2^32 against double's 2^+/-1024.  The math
// functions evaluate in a double, so before #1626 a result past double's range came
// back NaR or zero (exp(800)), an argument past it made the result NaR (log(2^2000)),
// and a tiny argument became zero (sin(2^-2000), floor(-2^-2000) = 0).
//
// Most results here are far outside double's range, so they are checked in LOG space:
// the result must be FAITHFUL -- the true value lies strictly between the logs of the
// result's two neighbouring encodings.  The references come from dd_cascade by routes
// that never form the huge value and, where possible, by different algebra than the
// implementation:
//   exp(x)       ln = x, by definition
//   exp2 / exp10 ln = x ln2, x ln10
//   pow(x, y)    ln = y ln|x|, ln|x| from the decoded fields
//   sinh, cosh   ln = |x| - ln2
//   erfc(x)      a continued fraction, where the implementation sums the asymptotic series
//   erf(tiny)    ln = ln|x| + ln(2/sqrt(pi))
//   log family, asinh, hypot: ordinary-sized results, judged against dd_cascade directly
// fmod and remainder are exact, so they are checked exactly with 1024-bit integers.
// The deliberate limits are asserted too: sin of a huge argument and takum_log fmod
// with a huge quotient stay NaR.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

#include <universal/number/takum/takum.hpp>
#include <universal/number/takum/mathlib.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::dd_cascade;
using sw::universal::is_takum_log;

const dd_cascade LN2    = sw::universal::log(dd_cascade(2.0));
const dd_cascade LN10   = sw::universal::log(dd_cascade(10.0));
const dd_cascade PI     = sw::universal::atan(dd_cascade(1.0)) * dd_cascade(4.0);
const dd_cascade LNSQPI = sw::universal::log(PI) * dd_cascade(0.5);

// ln|x| of an encoding, independently of the implementation's ln_abs
template<typename T>
dd_cascade ln_of(const T& x) {
	auto d = T::Codec::decode(x.magnitude_bits());
	dd_cascade f(0.0);
	if (d.p > 0 && d.M_bits != 0) {
		const double hi = static_cast<double>(d.M_bits >> 26), lo = static_cast<double>(d.M_bits & ((1ull << 26) - 1));
		f = dd_cascade(std::ldexp(hi, 26 - static_cast<int>(d.p))) + dd_cascade(std::ldexp(lo, -static_cast<int>(d.p)));
	}
	if constexpr (is_takum_log<T>) return (dd_cascade(static_cast<double>(d.c)) + f) * dd_cascade(0.5);
	else return dd_cascade(static_cast<double>(d.c)) * LN2 + sw::universal::log1p(f);
}

template<typename T>
T neighbour(const T& x, int k) { T r(x); if (k > 0) ++r; else --r; return r; }

struct Tally {
	int fails = 0; long checked = 0;
	bool report; std::string tag;
	void fail(const std::string& what) { ++fails; if (report) std::cout << "FAIL " << tag << ' ' << what << '\n'; }
};

// The true value has ln = Lref and sign negative.  Faithful means the result is one of
// the two encodings bracketing it; past either end of the range it must saturate.
template<typename T>
void expect_ln(Tally& t, const T& got, const dd_cascade& Lref, bool negative, const std::string& what) {
	using sw::universal::SpecificValue;
	++t.checked;
	if (got.isnar() || got.iszero()) { t.fail(what + ": got " + (got.isnar() ? "NaR" : "0")); return; }
	if (got.sign() != negative)      { t.fail(what + ": wrong sign"); return; }
	const T maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
	const T a = got.sign() ? -got : got;
	if (a.raw_bits() == maxpos.raw_bits()) { if (Lref < ln_of(neighbour(a, -1))) t.fail(what + ": saturated to maxpos below the range"); return; }
	if (a.raw_bits() == minpos.raw_bits()) { if (ln_of(neighbour(a, 1)) < Lref)  t.fail(what + ": saturated to minpos above the range"); return; }
	const dd_cascade lo = ln_of(neighbour(a, -1)), hi = ln_of(neighbour(a, 1));
	if (!(lo < Lref && Lref < hi)) t.fail(what + ": not faithful");
}

// An ordinary-sized result: faithful against a dd_cascade reference.
template<typename T>
void expect_value(Tally& t, const T& got, const dd_cascade& ref, const std::string& what) {
	++t.checked;
	if (got.isnar()) { t.fail(what + ": got NaR"); return; }
	const dd_cascade lo(double(neighbour(got, -1))), hi(double(neighbour(got, 1)));
	if (!(lo < ref && ref < hi)) t.fail(what + ": not faithful (got " + std::to_string(double(got)) + ", want " + std::to_string(double(ref)) + ")");
}

template<typename T>
void expect_bits(Tally& t, const T& got, const T& want, const std::string& what) {
	++t.checked;
	if (got.raw_bits() != want.raw_bits()) t.fail(what + ": got " + std::to_string(double(got)));
}

// 2^k, exactly, from the codec -- independently of the code under test
template<typename T>
T pow2(int64_t k) { T r; r.setbits(T::Codec::encode_exact(k, 0ull).magnitude); return r; }

// R * 2^g, rounded once, R < 2^62 -- through the exact arithmetic path's rounding tail
template<typename T>
T dyadic(std::uint64_t R, int64_t g, bool negative) {
	T r; r.setzero();
	if (R == 0) return r;
	return r.assign_wide(sw::universal::takum_wide::wide_value{ sw::universal::takum_wide::make_u128(R), g, negative, false });
}

// the largest |L| whose e^L is inside the range, with a margin
template<typename T>
double ln_maxpos() { return double(ln_of(T(sw::universal::SpecificValue::maxpos))); }

// erfc(x) for large x by its continued fraction (Lentz, evaluated backward):
// erfc(x) = e^(-x^2) / sqrt(pi) / (x + (1/2)/(x + 1/(x + (3/2)/(x + ...))))
dd_cascade ln_erfc_cf(double x) {
	const dd_cascade X(x);
	dd_cascade K = X;
	for (int n = 80; n >= 1; --n) K = X + dd_cascade(0.5 * n) / K;
	return dd_cascade(0.0) - X * X - LNSQPI - sw::universal::log(K);
}

template<typename T>
int VerifyExponentials(const std::string& tag, bool report) {
	Tally t{ 0, 0, report, tag };
	const double Lmax = ln_maxpos<T>();
	for (double x = 709.5; x < 0.98 * Lmax; x *= 1.37) {
		const dd_cascade X(x);
		expect_ln(t, exp(T(x)),    dd_cascade(double(T(x))),                    false, "exp(" + std::to_string(x) + ")");
		expect_ln(t, exp(T(-x)),   dd_cascade(0.0) - dd_cascade(double(T(x))),  false, "exp(-" + std::to_string(x) + ")");
		expect_ln(t, expm1(T(x)),  dd_cascade(double(T(x))),                    false, "expm1(" + std::to_string(x) + ")");
		expect_ln(t, sinh(T(x)),   dd_cascade(double(T(x))) - LN2,              false, "sinh(" + std::to_string(x) + ")");
		expect_ln(t, sinh(T(-x)),  dd_cascade(double(T(x))) - LN2,              true,  "sinh(-" + std::to_string(x) + ")");
		expect_ln(t, cosh(T(-x)),  dd_cascade(double(T(x))) - LN2,              false, "cosh(-" + std::to_string(x) + ")");
		const double x2 = x / 0.6931471805599453, x10 = x / 2.302585092994046;
		expect_ln(t, exp2(T(x2)),  dd_cascade(double(T(x2))) * LN2,             false, "exp2(" + std::to_string(x2) + ")");
		expect_ln(t, exp10(T(x10)), dd_cascade(double(T(x10))) * LN10,          false, "exp10(" + std::to_string(x10) + ")");
	}
	// past either end: exactly saturated
	using sw::universal::SpecificValue;
	const T maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
	expect_bits(t, exp(T(1.5 * Lmax)),  maxpos, "exp past the top");
	expect_bits(t, exp(T(-1.5 * Lmax)), minpos, "exp past the bottom");
	expect_bits(t, exp(pow2<T>(3000)),  maxpos, "exp(2^3000)");
	expect_bits(t, exp(-pow2<T>(3000)), minpos, "exp(-2^3000)");
	return t.fails;
}

template<typename T>
int VerifyPow(const std::string& tag, bool report) {
	Tally t{ 0, 0, report, tag };
	const double Lmax = ln_maxpos<T>();
	for (int64_t e = 600; e < static_cast<int64_t>(0.9 * Lmax / 0.6931471805599453); e = e * 3 / 2) {
		const T x = pow2<T>(e) * T(1.375);
		for (double y : { 0.5, -0.5, 0.25, 1.5, -1.0 / 3.0 }) {
			const T Y(y);
			expect_ln(t, pow(x, Y), ln_of(x) * dd_cascade(double(Y)), false, "pow(1.375*2^" + std::to_string(e) + ", " + std::to_string(y) + ")");
		}
	}
	for (double y = 400.0; y < 0.9 * Lmax / 2.302585092994046; y *= 1.9) {
		const T Y(y), ten(10.0);
		expect_ln(t, pow(ten, Y), ln_of(ten) * dd_cascade(double(Y)), false, "pow(10, " + std::to_string(y) + ")");
		expect_ln(t, pow(ten, -Y), ln_of(ten) * dd_cascade(-double(Y)), false, "pow(10, -" + std::to_string(y) + ")");
	}
	if constexpr (!is_takum_log<T>) {   // the exponent must be an exact integer for a negative base
		expect_ln(t, pow(T(-2.0), T(3001.0)), LN2 * dd_cascade(3001.0), true,  "pow(-2, 3001)");
		expect_ln(t, pow(T(-2.0), T(3000.0)), LN2 * dd_cascade(3000.0), false, "pow(-2, 3000)");
	}
	++t.checked;
	if (!pow(T(-2.0), T(0.5)).isnar()) t.fail("pow(-2, 0.5) must be NaR");
	return t.fails;
}

template<typename T>
int VerifyLogarithms(const std::string& tag, bool report) {
	Tally t{ 0, 0, report, tag };
	for (int64_t e : { 600, 2000, 20000, -600, -2000, -20000 }) {
		if (std::fabs(double(e)) * 0.6931471805599453 > 0.9 * ln_maxpos<T>()) continue;
		const T x = pow2<T>(e) * T(1.375);
		const dd_cascade L = ln_of(x);
		expect_value(t, log(x),   L,        "log(1.375*2^" + std::to_string(e) + ")");
		expect_value(t, log2(x),  L / LN2,  "log2(1.375*2^" + std::to_string(e) + ")");
		expect_value(t, log10(x), L / LN10, "log10(1.375*2^" + std::to_string(e) + ")");
		if (e > 0) {
			expect_value(t, log1p(x), L,       "log1p(1.375*2^" + std::to_string(e) + ")");
			expect_value(t, asinh(x), L + LN2, "asinh(1.375*2^" + std::to_string(e) + ")");
			expect_value(t, acosh(x), L + LN2, "acosh(1.375*2^" + std::to_string(e) + ")");
		}
		++t.checked;
		if (!log(-x).isnar()) t.fail("log of a negative must be NaR");
	}
	return t.fails;
}

template<typename T>
int VerifyErrorFunctions(const std::string& tag, bool report) {
	Tally t{ 0, 0, report, tag };
	const double Lmax = ln_maxpos<T>();
	for (double x = 26.6; x * x < 0.95 * Lmax; x *= 1.29) {
		const T X(x);
		expect_ln(t, erfc(X), ln_erfc_cf(double(X)), false, "erfc(" + std::to_string(x) + ")");
	}
	const T tiny = pow2<T>(-2000) * T(1.375);
	expect_ln(t, erf(tiny),  ln_of(tiny) + sw::universal::log(dd_cascade(2.0) / sw::universal::sqrt(PI)), false, "erf(tiny)");
	expect_ln(t, erf(-tiny), ln_of(tiny) + sw::universal::log(dd_cascade(2.0) / sw::universal::sqrt(PI)), true,  "erf(-tiny)");
	expect_bits(t, erfc(tiny), T(1.0), "erfc(tiny)");
	return t.fails;
}

// the identities a tiny argument reduces to, and rounding to an integer
template<typename T>
int VerifyTinyAndIntegral(const std::string& tag, bool report) {
	Tally t{ 0, 0, report, tag };
	const T one(1.0), zero(0.0), mone(-1.0);
	for (int64_t e : { -600, -2000, -20000 }) {
		if (std::fabs(double(e)) * 0.6931471805599453 > 0.9 * ln_maxpos<T>()) continue;
		for (bool negative : { false, true }) {
			const T x = negative ? -(pow2<T>(e) * T(1.375)) : pow2<T>(e) * T(1.375);
			const std::string s = std::string(negative ? "-" : "") + "1.375*2^" + std::to_string(e);
			expect_bits(t, sin(x), x, "sin(" + s + ")");    expect_bits(t, tan(x), x, "tan(" + s + ")");
			expect_bits(t, asin(x), x, "asin(" + s + ")");  expect_bits(t, atan(x), x, "atan(" + s + ")");
			expect_bits(t, sinh(x), x, "sinh(" + s + ")");  expect_bits(t, tanh(x), x, "tanh(" + s + ")");
			expect_bits(t, asinh(x), x, "asinh(" + s + ")"); expect_bits(t, atanh(x), x, "atanh(" + s + ")");
			expect_bits(t, expm1(x), x, "expm1(" + s + ")"); expect_bits(t, log1p(x), x, "log1p(" + s + ")");
			expect_bits(t, cos(x), one, "cos(" + s + ")");  expect_bits(t, cosh(x), one, "cosh(" + s + ")");
			expect_bits(t, exp(x), one, "exp(" + s + ")");  expect_bits(t, sec(x), one, "sec(" + s + ")");
			expect_bits(t, cot(x), one / x, "cot(" + s + ")"); expect_bits(t, csc(x), one / x, "csc(" + s + ")");
			expect_bits(t, frac(x), x, "frac(" + s + ")");
			expect_bits(t, trunc(x), zero, "trunc(" + s + ")"); expect_bits(t, round(x), zero, "round(" + s + ")");
			expect_bits(t, floor(x), negative ? mone : zero, "floor(" + s + ")");
			expect_bits(t, ceil(x),  negative ? zero : one,  "ceil(" + s + ")");
		}
	}
	for (int64_t e : { 600, 2000 }) {
		const T x = pow2<T>(e) * T(1.375);
		expect_bits(t, trunc(x), x, "trunc(huge)"); expect_bits(t, floor(-x), -x, "floor(-huge)");
		expect_bits(t, frac(x), zero, "frac(huge)");
	}
	// Past double's range sin stays NaR: no exact reduction modulo pi.  (Below 2^1024
	// the argument is an exact double, and libm reduces that exactly.)
	++t.checked;
	if (!sin(pow2<T>(2000)).isnar()) t.fail("sin(2^2000) stays NaR (no exact reduction modulo pi)");
	// hypot and atan2 across the range
	const T a = pow2<T>(2000) * T(1.375), b = pow2<T>(2000) * T(1.125);
	const dd_cascade La = ln_of(a), Lb = ln_of(b);
	const dd_cascade ratio = sw::universal::exp(Lb - La);
	expect_ln(t, hypot(a, b), La + sw::universal::log1p(ratio * ratio) * dd_cascade(0.5), false, "hypot(huge, huge)");
	// 1/b is the larger, and (1/a) / (1/b) = b/a = ratio
	const T ia = T(1.0) / a, ib = T(1.0) / b;
	const dd_cascade iratio = sw::universal::exp(ln_of(ia) - ln_of(ib));   // |ia| / |ib| < 1, from the encodings
	expect_ln(t, hypot(ia, ib), ln_of(ib) + sw::universal::log1p(iratio * iratio) * dd_cascade(0.5), false, "hypot(tiny, tiny)");
	expect_value(t, atan2(b, a), sw::universal::atan(ratio), "atan2(huge, huge)");
	const T tiny = T(1.0) / a;
	expect_bits(t, atan2(tiny, a), tiny / a, "atan2(tiny, huge)");
	return t.fails;
}

// fmod and remainder against 1024-bit integers: x = Sx 2^ex with ex in [511, 900]
// is beyond double's range yet small enough to hold exactly.
template<typename T>
int VerifyFmodExact(const std::string& tag, bool report) {
	using BigInt = sw::universal::integer<1024, std::uint64_t>;
	Tally t{ 0, 0, report, tag };
	for (int64_t e = 511; e < 900; e += 37) {
		for (double m : { 1.0, 1.375, 1.9921875 }) {
			const T x = pow2<T>(e) * T(m);
			for (double yv : { 3.0, 7.5, 1.0e10, 0.0009765625 }) {
				const T y(yv);
				const auto dx = T::Codec::decode(x.magnitude_bits()), dy = T::Codec::decode(y.magnitude_bits());
				const int64_t ex = dx.c - int64_t(dx.p), ey = dy.c - int64_t(dy.p), g = std::min(ex, ey);
				BigInt X((1ull << dx.p) | dx.M_bits), Y((1ull << dy.p) | dy.M_bits);
				X <<= static_cast<int>(ex - g); Y <<= static_cast<int>(ey - g);
				BigInt R = X % Y;
				// fmod: R 2^g, sign of x (positive here)
				const T want_fmod = dyadic<T>(static_cast<std::uint64_t>(static_cast<long long>(R)), g, false);
				expect_bits(t, fmod(x, y), want_fmod, "fmod(" + std::to_string(m) + "*2^" + std::to_string(e) + ", " + std::to_string(yv) + ")");
				// remainder: subtract Y when R > Y/2, or at the tie when the quotient is odd
				const BigInt Q = X / Y;
				const bool odd = (Q % BigInt(2)) != BigInt(0);
				BigInt twoR = R + R;
				T want_rem = want_fmod;
				if (twoR > Y || (twoR == Y && odd)) {
					BigInt D = Y - R;
					want_rem = dyadic<T>(static_cast<std::uint64_t>(static_cast<long long>(D)), g, true);
				}
				expect_bits(t, remainder(x, y), want_rem, "remainder(" + std::to_string(m) + "*2^" + std::to_string(e) + ", " + std::to_string(yv) + ")");
			}
		}
	}
	// past 1024 bits, by number theory: 2^2000 = (2^2)^1000 = 1 (mod 3)
	expect_bits(t, fmod(pow2<T>(2000), T(3.0)), T(1.0), "fmod(2^2000, 3)");
	expect_bits(t, remainder(pow2<T>(2001), T(3.0)), T(-1.0), "remainder(2^2001, 3)");
	expect_bits(t, fmod(pow2<T>(-2000), T(1.0)), pow2<T>(-2000), "fmod(2^-2000, 1)");
	return t.fails;
}

template<typename T>
int VerifyLogFmodLimits(const std::string& tag, bool report) {
	Tally t{ 0, 0, report, tag };
	const T big = pow2<T>(2000), tiny = pow2<T>(-2000);
	expect_bits(t, fmod(tiny, T(1.0)), tiny, "fmod(tiny, 1)");
	++t.checked;
	if (!fmod(big, T(3.0)).isnar()) t.fail("takum_log fmod with a huge quotient stays NaR");
	return t.fails;
}

// Inside double's range nothing may change: the far-range code must hand every such
// argument back to the plain double evaluation.
template<typename T>
int VerifyInRangeUnchanged(const std::string& tag, bool report) {
	Tally t{ 0, 0, report, tag };
	for (double v = -300.0; v <= 300.0; v += 1.737) {
		const T x(v);
		const double d = double(x);
		expect_bits(t, exp(x),   T(std::exp(d)),   "exp(" + std::to_string(v) + ")");
		expect_bits(t, sinh(x),  T(std::sinh(d)),  "sinh(" + std::to_string(v) + ")");
		expect_bits(t, sin(x),   T(std::sin(d)),   "sin(" + std::to_string(v) + ")");
		expect_bits(t, atan(x),  T(std::atan(d)),  "atan(" + std::to_string(v) + ")");
		expect_bits(t, floor(x), T(std::floor(d)), "floor(" + std::to_string(v) + ")");
		if (d < 26.0) expect_bits(t, erfc(x), T(std::erfc(d)), "erfc(" + std::to_string(v) + ")");   // beyond, the double underflows -- the case this fixes
		if (d > 0.0) {
			expect_bits(t, log(x),            T(std::log(d)),              "log(" + std::to_string(v) + ")");
			expect_bits(t, pow(x, T(1.5)),    T(std::pow(d, double(T(1.5)))), "pow(" + std::to_string(v) + ", 1.5)");
			expect_bits(t, fmod(x, T(7.0)),   T(std::fmod(d, double(T(7.0)))), "fmod(" + std::to_string(v) + ", 7)");
		}
	}
	return t.fails;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite = "takum math past double's range, rbits 4 and 5 (#1626)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyExponentials<takum<24, 4, std::uint32_t>>("takum<24,4>", true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	using L24 = takum<24, 4, std::uint32_t>;
	using L32 = takum<32, 4, std::uint32_t>;
	using L325 = takum<32, 5, std::uint32_t>;
	using G24 = takum_log<24, 4, std::uint32_t>;
	using G325 = takum_log<32, 5, std::uint32_t>;

	nrOfFailedTestCases += ReportTestResult(VerifyExponentials<L24>("takum<24,4>", reportTestCases),  "takum<24,4>", "exponentials");
	nrOfFailedTestCases += ReportTestResult(VerifyExponentials<L32>("takum<32,4>", reportTestCases),  "takum<32,4>", "exponentials");
	nrOfFailedTestCases += ReportTestResult(VerifyExponentials<L325>("takum<32,5>", reportTestCases), "takum<32,5>", "exponentials");
	nrOfFailedTestCases += ReportTestResult(VerifyExponentials<G24>("takum_log<24,4>", reportTestCases),  "takum_log<24,4>", "exponentials");
	nrOfFailedTestCases += ReportTestResult(VerifyExponentials<G325>("takum_log<32,5>", reportTestCases), "takum_log<32,5>", "exponentials");

	nrOfFailedTestCases += ReportTestResult(VerifyPow<L24>("takum<24,4>", reportTestCases),   "takum<24,4>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPow<L325>("takum<32,5>", reportTestCases),  "takum<32,5>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPow<G24>("takum_log<24,4>", reportTestCases), "takum_log<24,4>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyLogarithms<L24>("takum<24,4>", reportTestCases),   "takum<24,4>", "logarithms");
	nrOfFailedTestCases += ReportTestResult(VerifyLogarithms<L325>("takum<32,5>", reportTestCases),  "takum<32,5>", "logarithms");
	nrOfFailedTestCases += ReportTestResult(VerifyLogarithms<G24>("takum_log<24,4>", reportTestCases), "takum_log<24,4>", "logarithms");

	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunctions<L24>("takum<24,4>", reportTestCases),   "takum<24,4>", "erf, erfc");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunctions<L325>("takum<32,5>", reportTestCases),  "takum<32,5>", "erf, erfc");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunctions<G24>("takum_log<24,4>", reportTestCases), "takum_log<24,4>", "erf, erfc");

	nrOfFailedTestCases += ReportTestResult(VerifyTinyAndIntegral<L24>("takum<24,4>", reportTestCases),   "takum<24,4>", "tiny, integral, hypot, atan2");
	nrOfFailedTestCases += ReportTestResult(VerifyTinyAndIntegral<L325>("takum<32,5>", reportTestCases),  "takum<32,5>", "tiny, integral, hypot, atan2");
	nrOfFailedTestCases += ReportTestResult(VerifyTinyAndIntegral<G24>("takum_log<24,4>", reportTestCases), "takum_log<24,4>", "tiny, integral, hypot, atan2");

	nrOfFailedTestCases += ReportTestResult(VerifyFmodExact<L24>("takum<24,4>", reportTestCases),  "takum<24,4>", "fmod, remainder exact");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodExact<L325>("takum<32,5>", reportTestCases), "takum<32,5>", "fmod, remainder exact");
	nrOfFailedTestCases += ReportTestResult(VerifyLogFmodLimits<G24>("takum_log<24,4>", reportTestCases), "takum_log<24,4>", "fmod limits");
	nrOfFailedTestCases += ReportTestResult(VerifyInRangeUnchanged<L24>("takum<24,4>", reportTestCases), "takum<24,4>", "in range unchanged");
	nrOfFailedTestCases += ReportTestResult(VerifyInRangeUnchanged<G24>("takum_log<24,4>", reportTestCases), "takum_log<24,4>", "in range unchanged");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
