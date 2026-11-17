// rounding_and_stepping.cpp: microfloat numeric_limits against the encoding, one rounding from double,
// long double and the integers, and ++ / -- at -0 and at the top of the range (#2096)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each check here failed before #2096:
//   - numeric_limits::max_exponent was the top exponent code minus the bias, one short for the formats
//     without an infinity, whose top code holds maxpos: e4m3 reported 8 for 9, e2m1 2 for 3.
//   - a double, a long double and every integer went through a cast to float, which rounds to nearest
//     before the microfloat rounds again: e4m3(1.0625 + 1e-12) was 1.0, not 1.125.
//   - ++(-0) stayed -0, and ++ from maxpos walked into the inf and NaN encodings (e5m2 0x7B -> 0x7C).
#include <universal/utility/directives.hpp>
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>

namespace {

	// max_exponent is one more than the scale of maxpos, min_exponent one more than the smallest normal's
	template<typename Mf>
	int VerifyExponentLimits(const std::string& tag, bool reportTestCases) {
		using Limits = std::numeric_limits<Mf>;
		int fails = 0;
		const float maxpos = float(Mf(sw::universal::SpecificValue::maxpos));
		const int maxScale = std::ilogb(maxpos);
		if (Limits::max_exponent != maxScale + 1) {
			if (reportTestCases) std::cerr << "FAIL " << tag << ": max_exponent " << Limits::max_exponent << ", maxpos " << maxpos
			                               << " is 2^" << maxScale << '\n';
			++fails;
		}
		if (float(Limits::max()) != maxpos) {
			if (reportTestCases) std::cerr << "FAIL " << tag << ": max() is not maxpos\n";
			++fails;
		}
		return fails;
	}

	// the microfloat value just above the tie between two neighbours, from a double past float's precision
	template<typename Mf>
	int VerifyNearTie(const std::string& tag, double lo, double hi, bool reportTestCases) {
		int fails = 0;
		const double tie = (lo + hi) / 2.0;
		const double above = tie + std::ldexp(tie, -40);   // rounds to the tie in float
		const double below = tie - std::ldexp(tie, -40);
		const Mf a(above), b(below), na(-above), nb(-below);
		if (double(a) != hi || double(b) != lo || double(na) != -hi || double(nb) != -lo) {
			if (reportTestCases) std::cerr << "FAIL " << tag << ": around the tie " << tie << " between " << lo << " and " << hi << ": "
			                               << double(b) << ' ' << double(a) << ' ' << double(nb) << ' ' << double(na) << '\n';
			++fails;
		}
#if LONG_DOUBLE_SUPPORT
		if (std::numeric_limits<long double>::digits > 53) {
			const long double t = (static_cast<long double>(lo) + static_cast<long double>(hi)) / 2.0L;
			const long double up = t + std::ldexp(t, -60);   // rounds to the tie in double and in float
			if (double(Mf(up)) != hi || double(Mf(-up)) != -hi) {
				if (reportTestCases) std::cerr << "FAIL " << tag << ": long double just above the tie " << double(t) << '\n';
				++fails;
			}
		}
#endif
		return fails;
	}

	// an integer past float's 24 bits, just above a tie of a microfloat with a range that holds it
	int VerifyIntegerRounding(bool reportTestCases) {
		using namespace sw::universal;
		using E7 = microfloat<8, 7, false, false, true>;     // e7m0: powers of two up to 2^64
		int fails = 0;
		// 3 * 2^40 is the tie between 2^41 and 2^42 in a format with no fraction bits, and ties go to
		// the even significand, 2^41. 3 * 2^40 + 1 is above the tie: once, it rounds to 2^42; through
		// a float, it first became the tie itself, and then 2^41.
		const long long above = 3LL * (1LL << 40) + 1;
		const E7 a(above), na(-above);
		if (double(a) != std::ldexp(1.0, 42) || double(na) != -std::ldexp(1.0, 42)) {
			if (reportTestCases) std::cerr << "FAIL e7m0(3 * 2^40 + 1) = " << double(a) << ", expected 2^42\n";
			++fails;
		}
		const unsigned long long uabove = 3ULL * (1ULL << 40) + 1;
		if (double(E7(uabove)) != std::ldexp(1.0, 42)) {
			if (reportTestCases) std::cerr << "FAIL e7m0(3 * 2^40 + 1, unsigned) = " << double(E7(uabove)) << ", expected 2^42\n";
			++fails;
		}
		if (double(E7(3LL * (1LL << 40))) != std::ldexp(1.0, 41)) {
			if (reportTestCases) std::cerr << "FAIL e7m0(3 * 2^40), the tie, should be 2^41\n";
			++fails;
		}
		// small integers are exact or round once, as before
		if (double(e4m3(17)) != 16.0 || double(e4m3(19)) != 20.0 || double(e4m3(-19)) != -20.0) {
			if (reportTestCases) std::cerr << "FAIL e4m3 of 17, 19, -19\n";
			++fails;
		}
		return fails;
	}

	template<typename Mf>
	int VerifyStepping(const std::string& tag, bool reportTestCases) {
		using namespace sw::universal;
		int fails = 0;
		auto check = [&](bool ok, const char* what) {
			if (ok) return;
			if (reportTestCases) std::cerr << "FAIL " << tag << ": " << what << '\n';
			++fails;
		};
		const Mf minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
		Mf negzero;
		negzero.setbits(Mf::sign_mask);
		Mf a(negzero);
		++a;
		check(a.bits() == minpos.bits(), "++(-0) is +minpos");
		a = negzero;
		--a;
		check(a.bits() == (-minpos).bits(), "--(-0) is -minpos");
		a.setzero();
		--a;
		check(a.bits() == (-minpos).bits(), "--(+0) is -minpos");
		a = maxpos;
		++a;
		check(a.bits() == maxpos.bits(), "++maxpos stays maxpos");
		a = -maxpos;
		--a;
		check(a.bits() == (-maxpos).bits(), "--maxneg stays maxneg");
		// every finite encoding steps to its neighbour and back
		for (unsigned bits = 0; bits < (1u << Mf::nbits); ++bits) {
			Mf x;
			x.setbits(bits);
			if (x.isnan() || x.isinf()) {
				Mf y(x);
				++y;
				check(y.bits() == x.bits(), "++ leaves inf and NaN where they are");
				continue;
			}
			if (x.bits() == maxpos.bits() || x.iszero()) continue;
			Mf y(x);
			++y;
			check(float(y) > float(x), "++ increases a finite value");
			--y;
			check(y.bits() == x.bits(), "-- undoes ++");
		}
		return fails;
	}

	// the gaps of #2113: odd_float's clamps, subnormal near-ties, integer sources of every kind and
	// the saturating e5m2 at the microfloat level
	int VerifyConversionEdges(bool reportTestCases) {
		using namespace sw::universal;
		int fails = 0;
		auto check = [&](bool ok, const char* what) {
			if (ok) return;
			if (reportTestCases) std::cerr << "FAIL " << what << '\n';
			++fails;
		};
		// odd_float clamps a double past 2^127 to 2^127 and one below float's normal range to 0
		check(e5m2(1e300).isinf() && !e5m2(1e300).sign(), "e5m2(1e300) is +inf");
		check(e5m2(-1e300).isinf() && e5m2(-1e300).sign(), "e5m2(-1e300) is -inf");
		check(e4m3(1e300).isnan(), "e4m3(1e300) is NaN, OCP e4m3fn has no inf");
		check(e4m3_saturating(1e300).bits() == e4m3_saturating(SpecificValue::maxpos).bits(), "e4m3_saturating(1e300) is maxpos");
		check(e2m1(-1e300).bits() == e2m1(SpecificValue::maxneg).bits(), "e2m1(-1e300) is maxneg");
		check(e4m3(1e-300).bits() == 0u, "e4m3(1e-300) is +0");
		check(e4m3(-1e-300).bits() == e4m3::sign_mask, "e4m3(-1e-300) is -0");
		check(e5m2(std::ldexp(1.0, -130)).bits() == 0u, "e5m2(2^-130), below float's normals, is +0");
		// integer sources: bool, char of either signedness, and LLONG_MIN, whose magnitude is not a long long
		check(double(e4m3(true)) == 1.0 && e4m3(false).bits() == 0u, "e4m3(true) is 1, e4m3(false) is 0");
		{
			e4m3 c;
			c = static_cast<char>(-5);
			check(double(c) == (std::is_signed_v<char> ? -5.0 : 256.0), "e4m3 = char(-5) follows char's signedness");
			c = static_cast<signed char>(-128);
			check(double(c) == -128.0, "e4m3 = signed char(-128)");
			c = static_cast<unsigned char>(255);
			check(double(c) == 256.0, "e4m3 = unsigned char(255) rounds to 256");
		}
		check(e5m2(std::numeric_limits<long long>::min()).isinf() && e5m2(std::numeric_limits<long long>::min()).sign(), "e5m2(LLONG_MIN) is -inf");
		check(e4m3(std::numeric_limits<long long>::min()).isnan(), "e4m3(LLONG_MIN) is NaN");
		check(e2m1(std::numeric_limits<long long>::min()).bits() == e2m1(SpecificValue::maxneg).bits(), "e2m1(LLONG_MIN) is maxneg");
		check(double(e5m2(std::numeric_limits<unsigned long long>::max())) != 0.0 && e5m2(std::numeric_limits<unsigned long long>::max()).isinf(), "e5m2(ULLONG_MAX) is +inf");
		check(double(e5m2(-(1LL << 14))) == -std::ldexp(1.0, 14) && e5m2(-(1LL << 40)).isinf(), "e5m2(-2^14) is exact, e5m2(-2^40) is -inf");
		// e5m2_saturating: a finite source past maxpos saturates; an infinite one keeps inf (#2096)
		const e5m2_saturating maxpos(SpecificValue::maxpos);
		check(e5m2_saturating(1e300).bits() == maxpos.bits(), "e5m2_saturating(1e300) is maxpos");
		check(e5m2_saturating(1e39).bits() == maxpos.bits(), "e5m2_saturating(1e39) is maxpos");
		check(e5m2_saturating(-1e39).bits() == (-maxpos).bits(), "e5m2_saturating(-1e39) is maxneg");
		check(e5m2_saturating(std::numeric_limits<float>::infinity()).isinf(), "e5m2_saturating(inf) keeps inf");
		check(e5m2_saturating(100000).bits() == maxpos.bits(), "e5m2_saturating(100000) is maxpos");
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

	std::string test_suite  = "microfloat rounding, limits and stepping (#2096)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExponentLimits<e2m1>("e2m1", reportTestCases), "e2m1", "max_exponent");
	nrOfFailedTestCases += ReportTestResult(VerifyExponentLimits<e2m3>("e2m3", reportTestCases), "e2m3", "max_exponent");
	nrOfFailedTestCases += ReportTestResult(VerifyExponentLimits<e3m2>("e3m2", reportTestCases), "e3m2", "max_exponent");
	nrOfFailedTestCases += ReportTestResult(VerifyExponentLimits<e4m3>("e4m3", reportTestCases), "e4m3", "max_exponent");
	nrOfFailedTestCases += ReportTestResult(VerifyExponentLimits<e5m2>("e5m2", reportTestCases), "e5m2", "max_exponent");
	static_assert(std::numeric_limits<e4m3>::max_exponent == 9, "e4m3 maxpos is 448 = 1.75 * 2^8");
	static_assert(std::numeric_limits<e2m1>::max_exponent == 3, "e2m1 maxpos is 6 = 1.5 * 2^2");
	static_assert(std::numeric_limits<e5m2>::max_exponent == 16, "e5m2 maxpos is 57344 = 1.75 * 2^15");

	// the sweep's value: 1.0625 is the tie between 1 and 1.125 in e4m3, and 1.0625 + 1e-12 is above it
	nrOfFailedTestCases += ReportTestResult((double(e4m3(1.0625 + 1e-12)) == 1.125) ? 0 : 1, "e4m3", "1.0625 + 1e-12 rounds up");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e4m3>("e4m3", 1.0, 1.125, reportTestCases), "e4m3", "near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e4m3>("e4m3", 256.0, 288.0, reportTestCases), "e4m3", "near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e5m2>("e5m2", 0.75, 0.875, reportTestCases), "e5m2", "near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e2m1>("e2m1", 4.0, 6.0, reportTestCases), "e2m1", "near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e3m2>("e3m2", 0.25, 0.3125, reportTestCases), "e3m2", "near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerRounding(reportTestCases), "microfloat", "integers round once");
	// subnormal near-ties from double: between 0 and minpos, and between the first two subnormals
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e4m3>("e4m3", 0.0, std::ldexp(1.0, -9), reportTestCases), "e4m3", "subnormal near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e4m3>("e4m3", std::ldexp(1.0, -9), std::ldexp(1.0, -8), reportTestCases), "e4m3", "subnormal near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e5m2>("e5m2", std::ldexp(2.0, -16), std::ldexp(3.0, -16), reportTestCases), "e5m2", "subnormal near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTie<e2m1>("e2m1", 0.0, 0.5, reportTestCases), "e2m1", "subnormal near ties from double");
	nrOfFailedTestCases += ReportTestResult(VerifyConversionEdges(reportTestCases), "microfloat", "clamps, integer sources, e5m2_saturating");
	{
		// the narrowing keeps the sign of zero: e4m3(-0.0) is -0, as the direct float cast gave
		const e4m3 nz(-0.0), pz(0.0);
		int fails = (nz.bits() == e4m3::sign_mask && pz.bits() == 0u) ? 0 : 1;
#if LONG_DOUBLE_SUPPORT
		fails += (e5m2(-0.0L).bits() == e5m2::sign_mask) ? 0 : 1;
#endif
		nrOfFailedTestCases += ReportTestResult(fails, "e4m3", "-0.0 stays -0");
	}

	nrOfFailedTestCases += ReportTestResult(VerifyStepping<e2m1>("e2m1", reportTestCases), "e2m1", "++ and --");
	nrOfFailedTestCases += ReportTestResult(VerifyStepping<e3m2>("e3m2", reportTestCases), "e3m2", "++ and --");
	nrOfFailedTestCases += ReportTestResult(VerifyStepping<e4m3>("e4m3", reportTestCases), "e4m3", "++ and --");
	nrOfFailedTestCases += ReportTestResult(VerifyStepping<e4m3_saturating>("e4m3_saturating", reportTestCases), "e4m3_saturating", "++ and --");
	nrOfFailedTestCases += ReportTestResult(VerifyStepping<e5m2>("e5m2", reportTestCases), "e5m2", "++ and --");
	nrOfFailedTestCases += ReportTestResult(VerifyStepping<e5m2_saturating>("e5m2_saturating", reportTestCases), "e5m2_saturating", "++ and --");
	{
		e5m2 x;
		x.setbits(0x7Bu);   // maxpos
		++x;
		nrOfFailedTestCases += ReportTestResult((x.bits() == 0x7Bu) ? 0 : 1, "e5m2", "++0x7B stays 0x7B, not inf");
	}
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
