// ieee754.cpp: takum_log conversion to and from double, float and the integers, against a quad-double reference
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A takum_log stores l = c + M/2^p and denotes sqrt(e)^l = e^(l/2).  The conversions are (#2079):
//   - from a double or an integer: l = 2 ln|x|, rounded once onto the layout, at every width.  A
//     double carried ~|l| 2^-53 of absolute error, which takum_log<64,3>, with 59 fraction bits of
//     l, could not absorb; and integers above 2^53 went through a double first.
//   - to a double or a float: e^(l/2) correctly rounded where l is not exact in a double
//     (takum_log<57,3> and up), within an ulp elsewhere, where std::exp is kept for speed.
//   - to an integer: truncated toward zero, saturated to [-max, max] ([0, max - 1] unsigned), NaR ->
//     the integer NaR, min (max unsigned) (posit's rule, #2073, amended by #2104).
//
// The reference is qd, ~212 bits, with its own log and exp: nothing shared with the double-double
// the conversions use.  A takum_log's l is exact in a qd.  A "nearest" check compares a result
// against its neighbours on the target's grid; ties cannot occur (ln x is irrational for a dyadic
// x != 1, and e^(l/2) for a dyadic l != 0).
#include <universal/utility/directives.hpp>
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/takum/takum_log.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

#include <climits>
#include <cstring>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>

namespace {

using sw::universal::qd;

qd qd_from_uint64(std::uint64_t v) {
	return sw::universal::ldexp(qd(static_cast<double>(v >> 32)), 32) + qd(static_cast<double>(v & 0xFFFFFFFFull));
}

// l of a finite non-zero takum_log, exactly
template<typename TL>
qd exact_l(const TL& t) {
	const auto d = TL::Codec::decode(t.magnitude_bits());
	qd l(static_cast<double>(d.c));
	if (d.p > 0) l += sw::universal::ldexp(qd_from_uint64(d.M_bits), -static_cast<int>(d.p));
	return l;
}

qd absq(const qd& a) { return (a < qd(0.0)) ? -a : a; }

// Is t the encoding nearest to lref?  Compared with both neighbours that are on the same side of
// zero; past the ends, saturation to minpos / maxpos is the nearest-encoding rule.
template<typename TL>
bool NearestInL(const TL& t, const qd& lref, bool negative) {
	if (t.iszero() || t.isnar() || t.sign() != negative) return false;
	const qd d0 = absq(lref - exact_l(t));
	for (int step : { -1, 1 }) {
		TL n;
		n.setbits(t.raw_bits() + static_cast<std::uint64_t>(static_cast<std::int64_t>(step)));
		if (n.iszero() || n.isnar() || n.sign() != negative) continue;
		if (absq(lref - exact_l(n)) < d0) return false;
	}
	return true;
}

// double -> takum_log
template<typename TL>
int VerifyFromDouble(std::size_t nrSamples, bool reportTestCases) {
	using namespace sw::universal;
	std::mt19937_64 rng(2079);
	int fails = 0;
	auto check = [&](double x) {
		if (x == 0.0 || !std::isfinite(x)) return;
		const TL t(x);
		const qd lref = 2.0 * log(qd(std::fabs(x)));
		if (!NearestInL(t, lref, x < 0.0)) {
			if (fails < 4 && reportTestCases) std::cerr << "FAIL: " << type_tag(t) << "(" << std::setprecision(17) << x << ") = 0x"
			                                              << std::hex << t.raw_bits() << std::dec << " is not the nearest encoding\n";
			++fails;
		}
	};
	for (double x : { 1.0, -1.0, 2.0, 0.5, 3.0, 1e-300, 1e300, 4.9406564584124654e-324, 2.2250738585072014e-308,
	                  1.7976931348623157e308, 1.0 + 0x1p-52, 1.0 - 0x1p-53, 1.6487212707001282 /* ~sqrt(e) */ }) {
		check(x);
	}
	for (std::size_t i = 0; i < nrSamples; ++i) {
		// across the whole double range, subnormals included
		const std::uint64_t bits = rng() & 0x7FFFFFFFFFFFFFFFull;
		double x;
		std::memcpy(&x, &bits, sizeof(x));
		check((rng() & 1u) ? x : -x);
		// near 1, where l is small and the layouts are finest
		check(1.0 + std::ldexp(static_cast<double>(rng() >> 11), -53 - static_cast<int>(rng() % 40u)));
		// on the encodings themselves, nudged by an ulp: the values next to a rounding boundary
		TL t;
		t.setbits(rng());
		if (!t.iszero() && !t.isnar()) {
			const double v = double(t);
			check(v);
			check(std::nextafter(v, 0.0));
		}
	}
	return fails;
}

// integer -> takum_log
template<typename TL>
int VerifyFromInteger(std::size_t nrSamples, bool reportTestCases) {
	using namespace sw::universal;
	std::mt19937_64 rng(2079);
	int fails = 0;
	auto check = [&](std::uint64_t v, bool negative) {
		if (v == 0) return;
		TL t;
		if (negative) t = static_cast<long long>(std::uint64_t(0) - v);   // modular in C++20: LLONG_MIN for v = 2^63
		else t = static_cast<unsigned long long>(v);
		const qd lref = 2.0 * log(qd_from_uint64(v));
		if (!NearestInL(t, lref, negative)) {
			if (fails < 4 && reportTestCases) std::cerr << "FAIL: " << type_tag(t) << "(" << (negative ? "-" : "") << v << ") = 0x"
			                                              << std::hex << t.raw_bits() << std::dec << " is not the nearest encoding\n";
			++fails;
		}
	};
	for (std::uint64_t v : { std::uint64_t(1), std::uint64_t(2), std::uint64_t(3), std::uint64_t(1000),
	                         (std::uint64_t(1) << 53) + 1u, (std::uint64_t(1) << 54) + 2u,
	                         std::uint64_t(LLONG_MAX), std::numeric_limits<std::uint64_t>::max() }) {
		check(v, false);
		if (v <= std::uint64_t(LLONG_MAX)) check(v, true);
	}
	check(std::uint64_t(1) << 63, true);   // LLONG_MIN
	for (std::size_t i = 0; i < nrSamples; ++i) {
		const unsigned width = 1u + static_cast<unsigned>(rng() % 64u);
		const std::uint64_t v = (width == 64) ? rng() : (rng() & ((std::uint64_t(1) << width) - 1u));
		check(v, false);
		if (v <= std::uint64_t(LLONG_MAX)) check(v, true);
	}
	return fails;
}

// e^(l/2) * 2^K in qd, scaled so that a double's subnormal and overflow regions stay well inside qd's range
qd reference_value(const qd& l, int K) {
	using namespace sw::universal;
	return exp(0.5 * l + qd(static_cast<double>(K)) * qd_ln2);
}

// takum_log -> Real: correctly rounded (correct == true) or within an ulp
template<typename TL, typename Real>
int VerifyToNative(std::size_t nrSamples, bool correct, bool reportTestCases) {
	using namespace sw::universal;
	using Limits = std::numeric_limits<Real>;
	std::mt19937_64 rng(2079);
	int fails = 0;
	auto check = [&](const TL& t) {
		if (t.iszero() || t.isnar()) return;
		const Real r = static_cast<Real>(t);
		const bool negative = t.sign();
		if ((r < Real(0)) != negative && r != Real(0)) { ++fails; return; }
		const qd l = exact_l(t);
		// scale: the value's binary exponent is about l / (2 ln 2)
		const double log2v = double(l) * 0.7213475204444817;
		const int K = (log2v < -900.0) ? 1000 : ((log2v > 900.0) ? -200 : 0);
		const qd R = reference_value(l, K);
		const Real a = negative ? -r : r;
		bool ok;
		if (std::isinf(a)) {
			// overflow: the value is at least max + half an ulp
			const qd threshold = ldexp(qd(static_cast<double>(Limits::max())) + ldexp(qd(1.0), Limits::max_exponent - Limits::digits - 1), K);
			ok = correct ? (R >= threshold) : (R >= ldexp(qd(static_cast<double>(Limits::max())), K));
		}
		else {
			const Real below = std::nextafter(a, Real(0));
			const Real above = std::nextafter(a, Limits::infinity());
			const qd qa = ldexp(qd(static_cast<double>(a)), K);
			const qd qb = ldexp(qd(static_cast<double>(below)), K);
			// past max, the next grid point is 2^max_exponent: a result of max is nearest only below
			// max + half an ulp, the same threshold the overflow branch uses
			const qd qc = std::isinf(above) ? ldexp(qd(1.0), Limits::max_exponent + K) : ldexp(qd(static_cast<double>(above)), K);
			if (correct) {
				const qd d0 = absq(R - qa);
				ok = !(absq(R - qb) < d0) && !(absq(R - qc) < d0);
			}
			else {
				ok = (qb < R || a == Real(0)) && R < qc;   // R lies within an ulp of the result
			}
		}
		if (!ok) {
			if (fails < 4 && reportTestCases) std::cerr << "FAIL: " << type_tag(t) << " 0x" << std::hex << t.raw_bits() << std::dec << " -> "
			                                              << std::setprecision(Limits::max_digits10) << r << " is not "
			                                              << (correct ? "correctly rounded" : "within an ulp") << '\n';
			++fails;
		}
	};
	constexpr unsigned nbits = TL::nbits;
	for (std::size_t i = 0; i < nrSamples; ++i) {
		TL t;
		t.setbits(nbits == 64 ? rng() : (rng() & ((std::uint64_t(1) << nbits) - 1u)));
		check(t);
	}
	check(TL(sw::universal::SpecificValue::maxpos));
	check(TL(sw::universal::SpecificValue::minpos));
	check(TL(sw::universal::SpecificValue::maxneg));
	check(TL(sw::universal::SpecificValue::minneg));
	return fails;
}

// takum_log -> integer: truncate, saturate to [-max, max] ([0, max - 1] unsigned), NaR -> min (max unsigned)
template<typename Integer, typename TL>
bool CastIsRight(const TL& t) {
	using namespace sw::universal;
	using Limits = std::numeric_limits<Integer>;
	const Integer got = static_cast<Integer>(t);
	if (t.iszero()) return got == 0;
	if (t.isnar()) return got == (Limits::is_signed ? Limits::min() : Limits::max());
	const bool negative = t.sign();
	if (negative && !Limits::is_signed) return got == 0;      // x < 0 truncates or saturates to 0
	const qd R = reference_value(exact_l(t), 0);            // |x|
	// the largest magnitude a finite value casts to; floor(|x|) past it saturates
	const std::uint64_t cap = static_cast<std::uint64_t>(Limits::max()) - (Limits::is_signed ? 0u : 1u);
	const qd limit = qd_from_uint64(cap) + qd(1.0);
	if (R >= limit) return got == (negative ? static_cast<Integer>(-static_cast<Integer>(cap)) : static_cast<Integer>(cap));
	// |got| = floor(|x|)
	const std::uint64_t mag = negative ? (std::uint64_t(0) - static_cast<std::uint64_t>(static_cast<long long>(got)))
	                                   : static_cast<std::uint64_t>(static_cast<long long>(got));
	if (negative && got > 0) return false;
	if (!negative && got < 0) return false;
	const qd q = qd_from_uint64(mag);
	return (q <= R) && (R < q + qd(1.0));
}

template<typename TL>
int VerifyIntegerCasts(std::size_t nrSamples, bool reportTestCases) {
	using namespace sw::universal;
	std::mt19937_64 rng(2079);
	int fails = 0;
	auto check = [&](const TL& t) {
		if (!CastIsRight<int>(t) || !CastIsRight<long>(t) || !CastIsRight<long long>(t) ||
		    !CastIsRight<unsigned int>(t) || !CastIsRight<unsigned long>(t) || !CastIsRight<unsigned long long>(t)) {
			if (fails < 4 && reportTestCases) std::cerr << "FAIL: integer casts of " << type_tag(t) << " 0x" << std::hex << t.raw_bits() << std::dec
			                                              << ": " << int(t) << ' ' << (long long)t << '\n';
			++fails;
		}
	};
	for (SpecificValue code : { SpecificValue::nar, SpecificValue::maxpos, SpecificValue::maxneg,
	                            SpecificValue::minpos, SpecificValue::minneg, SpecificValue::zero }) {
		check(TL(code));
	}
	for (double d : { 0.5, 1.0, 1.5, 2147483647.0, 2147483648.0, 9.2e18, 9.3e18, 1e30 }) { check(TL(d)); check(TL(-d)); }
	constexpr unsigned nbits = TL::nbits;
	for (std::size_t i = 0; i < nrSamples; ++i) {
		TL t;
		t.setbits(nbits == 64 ? rng() : (rng() & ((std::uint64_t(1) << nbits) - 1u)));
		check(t);
		// values in the integer range, where truncation has fraction bits to drop
		check(TL(std::ldexp(static_cast<double>(rng() >> 11), -static_cast<int>(rng() % 53u)) * ((rng() & 1u) ? 1.0 : -1.0)));
	}
	return fails;
}

// A constant evaluation gives the run-time encoding (#2098).  It went through a double and the
// constexpr log, so (1LL << 53) + 1 became 2^53, and even 3 or 0.1 missed the correct rounding on
// takum_log<64,3>, whose l carries 59 fraction bits.
template<typename TL>
int VerifyConstexprConversions(bool reportTestCases) {
	int fails = 0;
	auto check = [&](const TL& compile_time, const TL& run_time, const char* what) {
		if (compile_time.raw_bits() == run_time.raw_bits()) return;
		if (reportTestCases) std::cerr << "FAIL: " << type_tag(run_time) << ' ' << what << ": constexpr 0x" << std::hex << compile_time.raw_bits()
		                               << " run time 0x" << run_time.raw_bits() << std::dec << '\n';
		++fails;
	};
	volatile long long vll = 0;
	volatile unsigned long long vull = 0;
	volatile int vi = 0;
	volatile double vd = 0.0;
	{ constexpr TL c((1LL << 53) + 1);              vll = (1LL << 53) + 1;              check(c, TL(vll), "(1LL << 53) + 1"); }
	{ constexpr TL c(-123456789012345678LL);        vll = -123456789012345678LL;        check(c, TL(vll), "-123456789012345678"); }
	{ constexpr TL c(LLONG_MIN);                    vll = LLONG_MIN;                    check(c, TL(vll), "LLONG_MIN"); }
	{ constexpr TL c(18446744073709551615ULL);      vull = 18446744073709551615ULL;     check(c, TL(vull), "ULLONG_MAX"); }
	{ constexpr TL c(3);                            vi = 3;                             check(c, TL(vi), "3"); }
	{ constexpr TL c(1);                            vi = 1;                             check(c, TL(vi), "1"); }
	{ constexpr TL c(0.1);                          vd = 0.1;                           check(c, TL(vd), "0.1"); }
	{ constexpr TL c(1e300);                        vd = 1e300;                         check(c, TL(vd), "1e300"); }
	{ constexpr TL c(4.9406564584124654e-324);      vd = 4.9406564584124654e-324;       check(c, TL(vd), "denorm_min"); }
	// and back: the integer casts and the double of a value
	{
		constexpr TL c(-123456789012345678LL);
		constexpr long long ll = static_cast<long long>(c);
		constexpr unsigned long long ull = static_cast<unsigned long long>(TL(18446744073709551615ULL));
		constexpr double d = double(TL(0.1));
		vll = -123456789012345678LL;
		vull = 18446744073709551615ULL;
		vd = 0.1;
		if (ll != static_cast<long long>(TL(vll))) { ++fails; if (reportTestCases) std::cerr << "FAIL: constexpr long long cast\n"; }
		if (ull != static_cast<unsigned long long>(TL(vull))) { ++fails; if (reportTestCases) std::cerr << "FAIL: constexpr unsigned long long cast\n"; }
		// a layout whose l is exact in a double keeps std::exp at run time, which is within an ulp but
		// not correctly rounded; the others take the double-double on both sides
		if constexpr (!TL::l_exact_in_double) {
			if (d != double(TL(vd))) { ++fails; if (reportTestCases) std::cerr << "FAIL: constexpr double\n"; }
		}
	}
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

	std::string test_suite  = "takum_log conversion to and from native types";
	std::string test_tag    = "ieee754";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	takum_log<64, 3> a(3.0);
	std::cout << std::hex << a.raw_bits() << std::dec << ' ' << double(a) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	{
		// the sweep's repros: an integer past 2^53 is not the double it rounds to, and the casts of NaR
		// and of the range ends are defined
		using TL = takum_log<64, 3>;
		int fails = 0;
		if (TL((1LL << 53) + 1) == TL(9007199254740992.0)) ++fails;
		if (int(TL(SpecificValue::nar)) != INT_MIN) ++fails;
		if ((long long)TL(SpecificValue::maxpos) != LLONG_MAX) ++fails;
		if ((long long)TL(SpecificValue::maxneg) != -LLONG_MAX) ++fails;   // LLONG_MIN is NaR's (#2104)
		if ((unsigned)TL(SpecificValue::nar) != UINT_MAX) ++fails;
		if ((unsigned)TL(SpecificValue::maxpos) != UINT_MAX - 1u) ++fails;
		if (int(TL(-3.99)) != -3) ++fails;
		if (int(TL(1.0)) != 1) ++fails;
		{
			// operator=(char) must follow char's own signedness, which the platform decides
			char c = static_cast<char>(-5);
			TL t;
			t = c;
			const double expected = std::is_signed_v<char> ? -5.0 : 251.0;
			if (double(t) / expected > 1.001 || double(t) / expected < 0.999) ++fails;
			signed char sc = -5;
			t = sc;
			if (double(t) > -4.99 || double(t) < -5.01) ++fails;
		}
		nrOfFailedTestCases += ReportTestResult(fails, "takum_log<64,3>", "sweep repros");
	}
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<takum_log<64, 3>>(2000, reportTestCases), "takum_log<64,3>", "double -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<takum_log<60, 3>>(1000, reportTestCases), "takum_log<60,3>", "double -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<takum_log<48, 3>>(1000, reportTestCases), "takum_log<48,3>", "double -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<takum_log<32, 3>>(2000, reportTestCases), "takum_log<32,3>", "double -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<takum_log<16, 3>>(2000, reportTestCases), "takum_log<16,3>", "double -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<takum_log< 8, 3>>(1000, reportTestCases), "takum_log<8,3>",  "double -> takum_log");

	nrOfFailedTestCases += ReportTestResult(VerifyFromInteger<takum_log<64, 3>>(2000, reportTestCases), "takum_log<64,3>", "integer -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyFromInteger<takum_log<32, 3>>(1000, reportTestCases), "takum_log<32,3>", "integer -> takum_log");

	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<64, 3>, double>(2000, true, reportTestCases), "takum_log<64,3>", "takum_log -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<64, 3>, float>(2000, true, reportTestCases), "takum_log<64,3>", "takum_log -> float");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<60, 3>, double>(1000, true, reportTestCases), "takum_log<60,3>", "takum_log -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<57, 3>, double>(1000, true, reportTestCases), "takum_log<57,3>", "takum_log -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<48, 3>, double>(1000, false, reportTestCases), "takum_log<48,3>", "takum_log -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<32, 3>, double>(2000, false, reportTestCases), "takum_log<32,3>", "takum_log -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<16, 3>, double>(2000, false, reportTestCases), "takum_log<16,3>", "takum_log -> double");

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum_log<64, 3>>(1000, reportTestCases), "takum_log<64,3>", "integer casts");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum_log<32, 3>>(1000, reportTestCases), "takum_log<32,3>", "integer casts");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum_log<16, 3>>(1000, reportTestCases), "takum_log<16,3>", "integer casts");
#if BIT_CAST_IS_CONSTEXPR
	nrOfFailedTestCases += ReportTestResult(VerifyConstexprConversions<takum_log<64, 3>>(reportTestCases), "takum_log<64,3>", "constexpr conversions");
	nrOfFailedTestCases += ReportTestResult(VerifyConstexprConversions<takum_log<32, 3>>(reportTestCases), "takum_log<32,3>", "constexpr conversions");
	nrOfFailedTestCases += ReportTestResult(VerifyConstexprConversions<takum_log<16, 2>>(reportTestCases), "takum_log<16,2>", "constexpr conversions");
#endif
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<takum_log<64, 3>>(20000, reportTestCases), "takum_log<64,3>", "double -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<takum_log<57, 3>>(10000, reportTestCases), "takum_log<57,3>", "double -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyFromInteger<takum_log<64, 3>>(20000, reportTestCases), "takum_log<64,3>", "integer -> takum_log");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<64, 3>, double>(20000, true, reportTestCases), "takum_log<64,3>", "takum_log -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<takum_log<57, 3>, double>(10000, true, reportTestCases), "takum_log<57,3>", "takum_log -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum_log<64, 3>>(20000, reportTestCases), "takum_log<64,3>", "integer casts");
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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
