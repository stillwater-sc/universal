// mathlib.cpp: elementary function verification for the logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// takum_log stores |value| = sqrt(e)^l.  Functions that act on l by a rational
// factor are exact in that domain, and this suite pins which ones are and what
// that buys:
//
//   sqrt   l/2      exact, then one rounding
//   sqr    2l       exact, then one rounding
//   rsqrt  -l/2     exact, then one rounding
//   pow    n*l      exact for integer n, then one rounding
//   1/x    -l       exact, no rounding at all (Prop. 7)
//
// The alternative is a decode / libm / encode round trip, which rounds three
// times and cannot carry more than a double's 53 significand bits.  That ceiling
// is not hypothetical: takum_log<64,3> reaches p = 59, and the round trip is
// then wrong for essentially every input.  VerifyCorrectlyRoundedSqrt measures
// exactly that, against an oracle built from exact integer arithmetic.
//
// The transcendental functions have no such shortcut and are evaluated as reals;
// they are checked for agreement with libm rather than for exactness.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <universal/number/takum/takum_log.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// ---------------------------------------------------------------------------
// The exactness properties.  These are the reason the specializations exist.
// ---------------------------------------------------------------------------

// sqrt(x)^2 == x whenever the halving and doubling both land exactly, and
// sqrt is always within one ulp of the encoding otherwise.
template<unsigned nbits, unsigned rbits>
int VerifySqrtRoundTrip(bool reportTestCases, uint64_t stride = 1) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	const uint64_t limit = 1ull << (nbits - 1);
	for (uint64_t b = 1; b < limit; b += stride) {
		TL x; x.setbits(b);
		if (x.iszero() || x.isnar()) continue;
		TL r = sqrt(x);
		if (r.isnar() || r.iszero()) continue;
		TL back = sqr(r);
		// squaring the root must return to within one encoding step
		int64_t d = static_cast<int64_t>(back.magnitude_bits()) - static_cast<int64_t>(x.magnitude_bits());
		if (d < -1 || d > 1) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL sqr(sqrt(x)) bits=" << b << " -> " << back.magnitude_bits()
				          << " (off by " << d << ")\n";
			}
		}
	}
	return nrOfFailedTests;
}

// The reciprocal is exact (Prop. 7), so rsqrt must equal the reciprocal of sqrt
// whenever that reciprocal is itself representable.
template<unsigned nbits, unsigned rbits>
int VerifyRsqrt(bool reportTestCases, uint64_t stride = 1) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	const uint64_t limit = 1ull << (nbits - 1);
	for (uint64_t b = 1; b < limit; b += stride) {
		TL x; x.setbits(b);
		if (x.iszero() || x.isnar()) continue;
		TL r = sqrt(x);
		if (r.isnar() || r.iszero()) continue;
		TL want = r.reciprocal();
		TL got  = rsqrt(x);
		if (want.isnar()) continue;
		if (got.raw_bits() != want.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL rsqrt bits=" << b << " got=" << got.raw_bits()
				          << " want=" << want.raw_bits() << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// pow(x, n) must agree with repeated multiplication in the logarithmic domain.
// Checked against sqr for n == 2 and against the reciprocal for n == -1, both of
// which are independently exact.
template<unsigned nbits, unsigned rbits>
int VerifyIntegerPow(bool reportTestCases, uint64_t stride = 1) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	const uint64_t limit = 1ull << (nbits - 1);
	for (uint64_t b = 1; b < limit; b += stride) {
		TL x; x.setbits(b);
		if (x.iszero() || x.isnar()) continue;

		TL p1 = pow(x, 1);
		if (p1.raw_bits() != x.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL pow(x,1) != x at bits=" << b << '\n';
		}
		TL p2 = pow(x, 2), s = sqr(x);
		if (!p2.isnar() && !s.isnar() && p2.raw_bits() != s.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL pow(x,2) != sqr(x) at bits=" << b
				          << " " << p2.raw_bits() << " vs " << s.raw_bits() << '\n';
			}
		}
		TL pm1 = pow(x, -1), inv = x.reciprocal();
		if (!pm1.isnar() && !inv.isnar() && pm1.raw_bits() != inv.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL pow(x,-1) != reciprocal at bits=" << b
				          << " " << pm1.raw_bits() << " vs " << inv.raw_bits() << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// The headline property: sqrt is CORRECTLY ROUNDED at every width.
//
// The correctly rounded square root is the representable encoding whose
// logarithmic value l' lies nearest to l/2.  Both are exact binary fractions, so
// the comparison is done in exact integers -- no double or long double, because
// the whole point is that those are too narrow above p = 53.
//
// The intermediates need more than 64 bits at the widest layouts: takum_log<64,3>
// reaches p = 59, and a numerator scaled to a common denominator of 2^60 does not
// fit an int64_t.  __int128 is used where the compiler has it, following
// blockbinary and blocktype/carry.hpp.  Where it does not (MSVC), the wide
// configurations are SKIPPED rather than silently passing, and the caller is told
// so -- an oracle that quietly evaluates nothing is worse than no oracle.
#if defined(__SIZEOF_INT128__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
using sqrt_oracle_int = __int128;
#pragma GCC diagnostic pop
#define TAKUM_LOG_HAVE_WIDE_ORACLE 1
#else
using sqrt_oracle_int = int64_t;
#define TAKUM_LOG_HAVE_WIDE_ORACLE 0
#endif

template<unsigned nbits, unsigned rbits>
int VerifyCorrectlyRoundedSqrt(bool reportTestCases, uint64_t stride = 1) {
	using TL    = sw::universal::takum_log<nbits, rbits, std::uint64_t>;
	using Codec = typename TL::Codec;
	using wide  = sqrt_oracle_int;
	int nrOfFailedTests = 0;
	const uint64_t span = (1ull << (nbits - 1)) - 1;

	// Widest common denominator this oracle can carry exactly.  A numerator is
	// c * 2^Q + M with |c| < 2^32, so Q + 32 must stay inside the integer type.
	const unsigned width_limit = (sizeof(wide) >= 16) ? 90u : 60u;

	long exercised = 0, skipped = 0;
	for (uint64_t b = 1; b < span; b += stride) {
		TL x; x.setbits(b);
		if (x.iszero() || x.isnar()) continue;
		auto d = Codec::decode(b);
		TL got = sqrt(x);
		if (got.iszero() || got.isnar()) continue;
		uint64_t gm = got.magnitude_bits();

		// l/2 == (c*2^p + M) / 2^(p+1); a candidate l' == (c'*2^p' + M') / 2^p'.
		// Scale both to a common denominator 2^Q and compare numerators.
		unsigned Q = d.p + 1;
		const int64_t lo = (static_cast<int64_t>(gm) > 2) ? static_cast<int64_t>(gm) - 2 : 1;
		const int64_t hi = static_cast<int64_t>(gm) + 2;
		for (int64_t m = lo; m <= hi; ++m) {
			if (m < 1 || static_cast<uint64_t>(m) > span) continue;
			auto e = Codec::decode(static_cast<uint64_t>(m));
			if (e.p > Q) Q = e.p;
		}
		if (Q > width_limit) { ++skipped; continue; }

		// A value c + M/2^p is the fraction (c*2^p + M) / 2^p.  Halving it keeps the
		// SAME numerator and moves the denominator to 2^(p+1) -- scaling c by 2^(p+1)
		// instead is the easy mistake, and it silently compares against a different
		// number.  Numerator and denominator exponent are therefore passed separately.
		auto scaled = [&](wide num, unsigned dexp) -> wide {
			return num * (static_cast<wide>(1) << (Q - dexp));
		};
		const wide half_num = static_cast<wide>(d.c) * (static_cast<wide>(1) << d.p)
		                    + static_cast<wide>(d.M_bits);
		const wide half     = scaled(half_num, d.p + 1);

		wide best = 0; bool have = false; wide mine = 0;
		for (int64_t m = lo; m <= hi; ++m) {
			if (m < 1 || static_cast<uint64_t>(m) > span) continue;
			auto e = Codec::decode(static_cast<uint64_t>(m));
			wide cand_num = static_cast<wide>(e.c) * (static_cast<wide>(1) << e.p)
			              + static_cast<wide>(e.M_bits);
			wide diff = scaled(cand_num, e.p) - half;
			if (diff < 0) diff = -diff;
			if (!have || diff < best) { best = diff; have = true; }
			if (static_cast<uint64_t>(m) == gm) mine = diff;
		}
		++exercised;
		if (have && mine != best) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL sqrt not correctly rounded at bits=" << b << '\n';
			}
		}
	}
	// A run that compared nothing must not report success.
	if (exercised == 0) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cout << "FAIL oracle evaluated no inputs (" << skipped << " skipped for width)\n";
		}
	}
	else if (skipped > 0 && reportTestCases) {
		std::cout << "note: " << skipped << " inputs skipped, oracle width limit " << width_limit << '\n';
	}
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// The transcendental surface: no shortcut, so check agreement with libm.
// ---------------------------------------------------------------------------

template<unsigned nbits, unsigned rbits>
int VerifyElementaryAgreement(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;

	// A relative tolerance a little looser than the format's own resolution: the
	// point is that the function was evaluated, not that it beat the encoding.
	const double tol = 1e-3;
	auto check = [&](const char* name, double got, double want) {
		double err = (want == 0.0) ? std::fabs(got) : std::fabs((got - want) / want);
		if (!(err < tol)) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << name << " got=" << got << " want=" << want
				          << " relerr=" << err << '\n';
			}
		}
	};

	// The reference is the function applied to the value the type actually STORES,
	// not to the literal it was built from.  takum_log represents no integer
	// exactly -- an integer k is sqrt(e)^l only for irrational l -- so
	// takum_log<32,3>(3) holds 2.99999999, and floor of it is honestly 2.  Using
	// the literal as the reference would score correct results as failures for
	// every rounding function.
	for (double v : { 0.125, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 7.0 }) {
		TL x(v);
		const double s = double(x);   // the stored value
		check("sin",   double(sin(x)),   std::sin(s));
		check("cos",   double(cos(x)),   std::cos(s));
		check("tan",   double(tan(x)),   std::tan(s));
		check("sinh",  double(sinh(x)),  std::sinh(s));
		check("cosh",  double(cosh(x)),  std::cosh(s));
		check("tanh",  double(tanh(x)),  std::tanh(s));
		check("exp",   double(exp(x)),   std::exp(s));
		check("exp2",  double(exp2(x)),  std::exp2(s));
		check("log",   double(log(x)),   std::log(s));
		check("log2",  double(log2(x)),  std::log2(s));
		check("log10", double(log10(x)), std::log10(s));
		check("erf",   double(erf(x)),   std::erf(s));
		check("sqrt",  double(sqrt(x)),  std::sqrt(s));
		check("trunc", double(trunc(x)), std::trunc(s));
		check("floor", double(floor(x)), std::floor(s));
		check("ceil",  double(ceil(x)),  std::ceil(s));
	}
	for (double v : { 0.25, 0.5, 0.75 }) {
		TL x(v);
		const double s = double(x);
		check("asin",  double(asin(x)),  std::asin(s));
		check("acos",  double(acos(x)),  std::acos(s));
		check("atan",  double(atan(x)),  std::atan(s));
		check("atanh", double(atanh(x)), std::atanh(s));
	}
	{
		TL a(3.0), b(4.0), c(0.5);
		check("hypot", double(hypot(a, b)), std::hypot(double(a), double(b)));
		check("fma",   double(fma(a, b, c)), std::fma(double(a), double(b), double(c)));
		check("fmod",  double(fmod(TL(7.0), TL(3.0))), std::fmod(double(TL(7.0)), double(TL(3.0))));
		check("atan2", double(atan2(a, b)), std::atan2(double(a), double(b)));
		check("pow",   double(pow(a, b)),   std::pow(double(a), double(b)));
	}
	return nrOfFailedTests;
}

// Special values must propagate rather than produce a wrong number.
template<unsigned nbits, unsigned rbits>
int VerifySpecialValues(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	TL nar; nar.setnar();
	TL zero(0.0), neg(-4.0), one(1.0);

	auto expect_nar = [&](const char* what, const TL& v) {
		if (!v.isnar()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL " << what << " should be NaR, got " << double(v) << '\n';
		}
	};
	auto expect_zero = [&](const char* what, const TL& v) {
		if (!v.iszero()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL " << what << " should be zero, got " << double(v) << '\n';
		}
	};

	expect_nar("sqrt(NaR)",   sqrt(nar));
	expect_nar("sqrt(-4)",    sqrt(neg));
	expect_zero("sqrt(0)",    sqrt(zero));
	expect_nar("rsqrt(0)",    rsqrt(zero));
	expect_nar("rsqrt(-4)",   rsqrt(neg));
	expect_nar("log(0)",      log(zero));
	expect_nar("log(-4)",     log(neg));
	expect_nar("log2(-4)",    log2(neg));
	expect_nar("log10(0)",    log10(zero));
	expect_nar("exp(NaR)",    exp(nar));
	expect_nar("pow(0,-1)",   pow(zero, -1));
	expect_zero("pow(0,3)",   pow(zero, 3));
	expect_nar("sqr(NaR)",    sqr(nar));

	// pow(x, 0) is 1 for every finite x, including zero
	if (double(pow(zero, 0)) != 1.0 || double(pow(neg, 0)) != 1.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL pow(x,0) should be 1\n";
	}
	// a negative base keeps its sign only for an odd exponent
	if (!(double(pow(neg, 3)) < 0.0) || !(double(pow(neg, 2)) > 0.0)) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL pow sign parity for a negative base\n";
	}
	// exp(0) == 1
	if (double(exp(zero)) != 1.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL exp(0) should be 1\n";
	}
	// exp saturates in the right DIRECTION at the extremes.  The result's
	// logarithmic value is 2x, which tracks the value of x rather than its
	// logarithm and so leaves the characteristic range almost immediately; an
	// unguarded conversion to int64_t wrapped and inverted the answer, returning
	// zero for exp(maxpos) and maxpos for exp(maxneg).
	{
		TL big(sw::universal::SpecificValue::maxpos);
		TL small(sw::universal::SpecificValue::maxneg);
		if (!exp(big).isnar() && !(double(exp(big)) > 1.0)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL exp(maxpos) must saturate upward, got " << double(exp(big)) << '\n';
		}
		if (!exp(small).iszero()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL exp(maxneg) must underflow to zero, got " << double(exp(small)) << '\n';
			}
		}
	}
	// classification
	if (!isnan(nar) || isfinite(nar) || !isfinite(one) || isnormal(zero) || !isnormal(one)) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL classification of special values\n";
	}
	return nrOfFailedTests;
}

} // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "takum_log elementary function verification";
	std::string test_tag    = "mathlib";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRoundedSqrt<16, 3>(true), "takum_log<16,3>", "sqrt correctly rounded");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecialValues<16, 3>(reportTestCases), "takum_log<16,3>", "special values");
	nrOfFailedTestCases += ReportTestResult(
		VerifyElementaryAgreement<32, 3>(reportTestCases), "takum_log<32,3>", "libm agreement");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRoundedSqrt<16, 3>(reportTestCases), "takum_log<16,3>", "sqrt correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifySqrtRoundTrip<16, 3>(reportTestCases), "takum_log<16,3>", "sqr(sqrt(x)) round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRsqrt<16, 3>(reportTestCases), "takum_log<16,3>", "rsqrt == 1/sqrt exactly");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerPow<16, 3>(reportTestCases), "takum_log<16,3>", "integer pow identities");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecialValues<12, 3>(reportTestCases), "takum_log<12,3>", "special values");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRoundedSqrt<12, 3>(reportTestCases), "takum_log<12,3>", "sqrt correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRoundedSqrt<20, 3>(reportTestCases), "takum_log<20,3>", "sqrt correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerPow<20, 3>(reportTestCases), "takum_log<20,3>", "integer pow identities");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRoundedSqrt<32, 3>(reportTestCases, 4093ull), "takum_log<32,3>", "sqrt correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRsqrt<24, 3>(reportTestCases, 7ull), "takum_log<24,3>", "rsqrt == 1/sqrt exactly");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerPow<24, 3>(reportTestCases, 7ull), "takum_log<24,3>", "integer pow identities");
#endif

	// Level 4 reaches the widths where the double round trip actually fails.  At
	// nbits = 64 the trailing field runs to p = 59, past a double's 53 bits, so
	// the exact-integer oracle is the only way to check the result at all -- and
	// on a compiler without __int128 it says so rather than passing vacuously.
#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		VerifySqrtRoundTrip<32, 3>(reportTestCases, 1031ull), "takum_log<32,3>", "sqr(sqrt(x)) round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerPow<32, 3>(reportTestCases, 1031ull), "takum_log<32,3>", "integer pow identities");
#if TAKUM_LOG_HAVE_WIDE_ORACLE
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRoundedSqrt<48, 3>(reportTestCases, 0x2AAAAAAABull),
		"takum_log<48,3>", "sqrt correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRoundedSqrt<64, 3>(reportTestCases, 0xAAAAAAAAAAABull),
		"takum_log<64,3>", "sqrt correctly rounded");
#else
	std::cout << "\ntakum_log<48,3> / <64,3> sqrt oracle SKIPPED: no __int128 on this compiler\n";
#endif
	nrOfFailedTestCases += ReportTestResult(
		VerifySqrtRoundTrip<64, 3>(reportTestCases, 0xAAAAAAAAAAABull),
		"takum_log<64,3>", "sqr(sqrt(x)) round-trip");
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
