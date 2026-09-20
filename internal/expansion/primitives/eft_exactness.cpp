// eft_exactness.cpp: prove the error-free transformations are exact (epic #987, Layer 1)
//
// The expansion algorithms rest on three error-free transformations (Shewchuk
// 1997; Knuth; Dekker). Their defining theorems are exact identities:
//
//   two_sum(a,b)      -> (s,e):  s == fl(a+b)  and  s + e == a + b   exactly
//   fast_two_sum(a,b) -> (s,e):  same, under the precondition |a| >= |b|
//   two_prod(a,b)     -> (x,y):  x == fl(a*b)  and  x + y == a * b   exactly
//
// We verify these against an independent exact dyadic-rational oracle
// (dyadic_exact.hpp, backed by einteger), which shares no code with the
// transformations under test. The identities hold when the rounded result
// stays finite (sums) and in the normal range (products): two_prod's error
// term y is only representable when a*b neither overflows nor underflows into
// the subnormal range, so the product fuzz stays within a normal-result band.
//
// Precondition audit: fast_two_sum's |a| >= |b| requirement is guaranteed at
// its sole call site, priest_renormalize's cumulative sweep in
// expansion_ops.hpp, where it is applied to a magnitude-sorted expansion. The
// fuzz below only feeds |a| >= |b| pairs and additionally checks that the
// identity breaks when the precondition is violated (so the precondition is
// load-bearing, not incidental). two_prod uses std::fma where the platform's fma
// is correctly rounded, so VerifyTwoProd doubles as a check that it is -- that is
// the gap that would silently break every product, and mingw-w64 without -mfma
// falls into it (5333 of 50000 products wrong). Such a build takes Dekker's
// product instead, which VerifyDekkerTwoProd exercises directly so that the
// fallback is covered on every platform rather than only the one that needs it
// (#1578).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <random>
#include <universal/internal/expansion/expansion_ops.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;
	using sw::universal::expansion_ops::two_sum;
	using sw::universal::expansion_ops::fast_two_sum;
	using sw::universal::expansion_ops::two_prod;

	// random normal double with exponent in [elo, ehi], full 52-bit mantissa, random sign
	double rdbl(std::mt19937_64& rng, int elo, int ehi) {
		std::uniform_int_distribution<int> ed(elo, ehi);
		std::uniform_int_distribution<std::uint64_t> md(0, (1ULL << 52) - 1);
		std::uniform_int_distribution<int> sd(0, 1);
		double frac = std::ldexp(static_cast<double>(md(rng)), -52);  // [0,1)
		double v = std::ldexp(1.0 + frac, ed(rng));
		return sd(rng) ? -v : v;
	}

	// s + e == a + b  exactly, and s == fl(a+b)
	bool two_sum_is_exact(double a, double b) {
		double s, e; two_sum(a, b, s, e);
		if (s != a + b) return false;
		return dyadic::from_double(a) + dyadic::from_double(b)
		    == dyadic::from_double(s) + dyadic::from_double(e);
	}
	// x + y == a * b  exactly, and x == fl(a*b)
	bool two_prod_is_exact(double a, double b) {
		double x, y; two_prod(a, b, x, y);
		if (x != a * b) return false;
		return dyadic::from_double(a) * dyadic::from_double(b)
		    == dyadic::from_double(x) + dyadic::from_double(y);
	}

	// the same identity for Dekker's product, called directly
	bool dekker_is_exact(double a, double b) {
		double x, y; expansion_ops::detail_eft::dekker_two_prod<double>(a, b, x, y);
		if (x != a * b) return false;
		return dyadic::from_double(a) * dyadic::from_double(b)
		    == dyadic::from_double(x) + dyadic::from_double(y);
	}

	// Independent self-check of the oracle on exactly representable values, so a
	// failure elsewhere is attributable to the EFT, not the reference.
	int VerifyOracleSanity(bool reportTestCases) {
		int fails = 0;
		auto chk = [&](bool c, const char* w) { if (!c) { if (reportTestCases) std::cout << "    FAIL oracle " << w << '\n'; ++fails; } };
		chk(dyadic::from_double(2.0) + dyadic::from_double(3.0) == dyadic::from_double(5.0), "2+3");
		chk(dyadic::from_double(0.5) * dyadic::from_double(0.25) == dyadic::from_double(0.125), "0.5*0.25");
		chk(dyadic::from_double(-4.0) + dyadic::from_double(4.0) == dyadic::from_double(0.0), "-4+4");
		chk(dyadic::from_double(1e16) + dyadic::from_double(1.0) != dyadic::from_double(1e16), "1e16+1");
		chk(dyadic::from_double(DBL_TRUE_MIN) * dyadic::from_double(2.0) == dyadic::from_double(2.0 * DBL_TRUE_MIN), "subnormal*2");
		return fails;
	}

	// two_sum is exact for all finite a, b whose sum is finite (subnormals
	// included). Structured edges + random fuzz.
	int VerifyTwoSum(bool reportTestCases, unsigned nrIterations) {
		int fails = 0;
		auto probe = [&](double a, double b, const char* tag) {
			if (!two_sum_is_exact(a, b)) { if (reportTestCases) std::cout << "    FAIL two_sum " << tag << " a=" << a << " b=" << b << '\n'; ++fails; }
		};
		// edges
		probe(1.0, DBL_TRUE_MIN, "1+true_min");
		probe(1.0, -DBL_TRUE_MIN, "1-true_min");
		probe(DBL_MAX, -DBL_MAX, "max-max");
		probe(1.0, 1.0, "1+1");
		probe(1.0, std::ldexp(1.0, -53), "tie");
		probe(0.0, -0.0, "+0-0");
		probe(std::ldexp(1.0, 1000), std::ldexp(1.0, -1000), "huge gap");
		probe(DBL_MIN, DBL_TRUE_MIN, "min+true_min");
		std::mt19937_64 rng(0x7500ull);
		for (unsigned i = 0; i < nrIterations; ++i) {
			double a = rdbl(rng, -1000, 1000), b = rdbl(rng, -1000, 1000);
			if (!std::isfinite(a + b)) continue;
			if (!two_sum_is_exact(a, b)) { if (reportTestCases) std::cout << "    FAIL two_sum fuzz a=" << a << " b=" << b << " (iter " << i << ")\n"; ++fails; }
		}
		return fails;
	}

	// fast_two_sum is exact under |a| >= |b|; verify on ordered pairs, and
	// confirm the precondition is load-bearing (unordered pairs can break it).
	int VerifyFastTwoSum(bool reportTestCases, unsigned nrIterations) {
		int fails = 0;
		std::mt19937_64 rng(0xFA57ull);
		for (unsigned i = 0; i < nrIterations; ++i) {
			double a = rdbl(rng, -1000, 1000), b = rdbl(rng, -1000, 1000);
			if (std::abs(a) < std::abs(b)) std::swap(a, b);   // enforce |a| >= |b|
			if (!std::isfinite(a + b)) continue;
			double s, e; fast_two_sum(a, b, s, e);
			bool ok = (s == a + b) &&
			          (dyadic::from_double(a) + dyadic::from_double(b)
			           == dyadic::from_double(s) + dyadic::from_double(e));
			if (!ok) { if (reportTestCases) std::cout << "    FAIL fast_two_sum a=" << a << " b=" << b << " (iter " << i << ")\n"; ++fails; }
		}
		// precondition is load-bearing: a known case where |a| < |b| is NOT exact
		{
			double a = 1.0, b = std::ldexp(1.0, 60);  // |a| < |b|
			double s, e; fast_two_sum(a, b, s, e);
			bool exact = (dyadic::from_double(a) + dyadic::from_double(b)
			              == dyadic::from_double(s) + dyadic::from_double(e));
			if (exact) {
				if (reportTestCases) std::cout << "    FAIL fast_two_sum stayed exact with |a|<|b| (precondition not load-bearing)\n";
				++fails;
			}
		}
		return fails;
	}

	// two_prod is exact when the product is a normal double (no overflow, no
	// subnormal underflow of the error term).
	int VerifyTwoProd(bool reportTestCases, unsigned nrIterations) {
		int fails = 0;
		auto probe = [&](double a, double b, const char* tag) {
			if (!two_prod_is_exact(a, b)) { if (reportTestCases) std::cout << "    FAIL two_prod " << tag << " a=" << a << " b=" << b << '\n'; ++fails; }
		};
		probe(1.5, 1.5, "1.5*1.5");
		probe(3.0, 7.0, "3*7");
		probe(1.0, 1.0, "1*1");
		probe(-2.0, 0.5, "-2*0.5");
		probe(std::ldexp(1.0, 200), std::ldexp(1.0, -100), "pow2 spread");
		std::mt19937_64 rng(0x9A0Dull);
		for (unsigned i = 0; i < nrIterations; ++i) {
			double a = rdbl(rng, -450, 450), b = rdbl(rng, -450, 450);  // product exp in [-900,900], normal
			if (!two_prod_is_exact(a, b)) { if (reportTestCases) std::cout << "    FAIL two_prod fuzz a=" << a << " b=" << b << " (iter " << i << ")\n"; ++fails; }
		}
		return fails;
	}

	// Dekker's product, exercised directly on double.
	//
	// two_prod only selects it for a limb wider than double, or on a build whose fma is not
	// correctly rounded -- mingw-w64 without -mfma (#1578). No CI platform is in either
	// case for double, so without this the double fallback is code that ships and is never
	// run, free to rot until the one configuration that needs it finds out. The identity is
	// the same one two_prod must satisfy, against the same independent oracle.
	int VerifyDekkerTwoProd(bool reportTestCases, unsigned nrIterations) {
		int fails = 0;
		auto probe = [&](double a, double b, const char* tag) {
			if (!dekker_is_exact(a, b)) { if (reportTestCases) std::cout << "    FAIL dekker_two_prod " << tag << " a=" << a << " b=" << b << '\n'; ++fails; }
		};
		probe(1.5, 1.5, "1.5*1.5");
		probe(3.0, 7.0, "3*7");
		probe(-2.0, 0.5, "-2*0.5");
		probe(std::ldexp(1.0, 200), std::ldexp(1.0, -100), "pow2 spread");
		probe(0.0, 1.0, "zero operand");
		std::mt19937_64 rng(0xDEC6E12ull);
		for (unsigned i = 0; i < nrIterations; ++i) {
			double a = rdbl(rng, -450, 450), b = rdbl(rng, -450, 450);
			if (!dekker_is_exact(a, b)) { if (reportTestCases) std::cout << "    FAIL dekker_two_prod fuzz a=" << a << " b=" << b << " (iter " << i << ")\n"; ++fails; }
		}
		// and it must agree with two_prod bit for bit wherever both are exact, since the
		// two are alternatives for the same operation
		for (unsigned i = 0; i < nrIterations; ++i) {
			double a = rdbl(rng, -450, 450), b = rdbl(rng, -450, 450);
			double x1, y1, x2, y2;
			two_prod(a, b, x1, y1);
			expansion_ops::detail_eft::dekker_two_prod<double>(a, b, x2, y2);
			if (x1 != x2 || y1 != y2) {
				if (reportTestCases) std::cout << "    FAIL two_prod vs dekker disagree a=" << a << " b=" << b << " (iter " << i << ")\n";
				++fails;
			}
		}
		return fails;
	}

}  // anonymous namespace

// Regression testing guards
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

	std::string test_suite          = "error-free transformation exactness (two_sum / fast_two_sum / two_prod)";
	std::string test_tag            = "eft exactness";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyOracleSanity(reportTestCases), "oracle", "dyadic self-check");

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyTwoSum(reportTestCases, 10000), "two_sum", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFastTwoSum(reportTestCases, 10000), "fast_two_sum", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTwoProd(reportTestCases, 10000), "two_prod", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTwoSum(reportTestCases, 50000),     "two_sum",      test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFastTwoSum(reportTestCases, 50000), "fast_two_sum", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTwoProd(reportTestCases, 50000),    "two_prod",     test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDekkerTwoProd(reportTestCases, 50000), "dekker_two_prod", test_tag);
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyTwoSum(reportTestCases, 200000),     "two_sum x2",      test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTwoProd(reportTestCases, 200000),    "two_prod x2",     test_tag);
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyTwoSum(reportTestCases, 1000000),  "two_sum x3",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTwoProd(reportTestCases, 1000000), "two_prod x3", test_tag);
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyTwoSum(reportTestCases, 5000000),  "two_sum x4",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTwoProd(reportTestCases, 5000000), "two_prod x4", test_tag);
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
