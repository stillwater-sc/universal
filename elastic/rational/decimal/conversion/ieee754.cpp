// ieee754.cpp: regression tests for double/float <-> erational conversion (#986)
//
// erational stores an exact rational (edecimal numerator / edecimal
// denominator). A binary float is the exact dyadic rational
//   (-1)^s * significand * 2^(e - bias - fbits),
// so conversion in is exact, and conversion out is the correctly-rounded
// (round-half-to-even) nearest float. Before #986 the inbound conversion
// dropped the exponent (collapsing every value into [1,2)) and the outbound
// conversion ignored the sign and overflowed for large numerator/denominator;
// there was no test covering ieee754 conversion at all.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <random>
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// A finite double round-trips exactly: (double)erational(d) == d. erational
	// represents every double exactly, and the nearest-float of an exactly
	// representable value is itself.
	int VerifyDoubleRoundTrip(bool reportTestCases, unsigned nrIterations) {
		std::mt19937_64 rng(0xE4A11Aull);
		std::uniform_int_distribution<std::uint64_t> bits;
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < nrIterations; ++i) {
			double d;
			std::uint64_t u = bits(rng);
			std::memcpy(&d, &u, sizeof(d));
			if (!std::isfinite(d)) continue;          // inf/nan are out of scope
			double r = double(erational(d));
			if (r != d) {
				if (reportTestCases) std::cout << "    FAIL roundtrip d=" << d << " got " << r << '\n';
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// Hand-picked values across the whole range: integers, fractions, signs,
	// subnormals, the extremes, and exact-but-awkward decimals.
	int VerifySpecificConversions(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const double cases[] = {
			0.0, -0.0, 1.0, 2.0, 3.0, 0.5, -4.0, 0.1, -0.1, 0.25, 1.0 / 3.0,
			123456.789, -987.654, std::ldexp(1.0, 60), std::ldexp(1.0, -60),
			DBL_MIN, DBL_TRUE_MIN, 2.0 * DBL_TRUE_MIN, DBL_MAX, -DBL_MAX,
			1e-300, 1e300, -1e-300
		};
		for (double d : cases) {
			double r = double(erational(d));
			// Exact value round-trip. erational is a rational type and has no
			// signed zero: -0.0 canonicalizes to 0 (0 == -0 mathematically), so
			// the value compare r == d is the correct criterion (it treats
			// +0 and -0 as equal and still distinguishes sign for non-zero).
			if (r != d) {
				if (reportTestCases) std::cout << "    FAIL specific d=" << d << " got " << r << '\n';
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// Arithmetic on exactly representable integer operands stays exact and
	// round-trips through double.
	int VerifyArithmeticRoundTrip(bool reportTestCases, unsigned nrIterations) {
		std::mt19937_64 rng(0xA21CEull);
		std::uniform_int_distribution<int> d(-100000, 100000);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < nrIterations; ++i) {
			int a = d(rng), b = d(rng);
			if (double(erational(a) + erational(b)) != double(a) + double(b)) {
				if (reportTestCases) std::cout << "    FAIL add " << a << '+' << b << '\n';
				++nrOfFailedTestCases;
			}
			if (double(erational(a) - erational(b)) != double(a) - double(b)) {
				if (reportTestCases) std::cout << "    FAIL sub " << a << '-' << b << '\n';
				++nrOfFailedTestCases;
			}
			if (double(erational(a) * erational(b)) != double(a) * double(b)) {
				if (reportTestCases) std::cout << "    FAIL mul " << a << '*' << b << '\n';
				++nrOfFailedTestCases;
			}
			// (a*b)/b == a exactly (exact rational division)
			if (b != 0) {
				long long prod = static_cast<long long>(a) * static_cast<long long>(b);
				if (double(erational(prod) / erational(b)) != double(a)) {
					if (reportTestCases) std::cout << "    FAIL div (" << a << '*' << b << ")/" << b << '\n';
					++nrOfFailedTestCases;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// float round-trip (narrower mantissa exercises a different fbits/bias).
	int VerifyFloatRoundTrip(bool reportTestCases, unsigned nrIterations) {
		std::mt19937_64 rng(0xF10A7ull);
		std::uniform_int_distribution<std::uint32_t> bits;
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < nrIterations; ++i) {
			float f;
			std::uint32_t u = bits(rng);
			std::memcpy(&f, &u, sizeof(f));
			if (!std::isfinite(f)) continue;
			float r = float(erational(f));
			if (r != f) {
				if (reportTestCases) std::cout << "    FAIL float roundtrip f=" << f << " got " << r << '\n';
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// Degenerate denominators (reachable via the (n, d) ctor and other paths)
	// must resolve to IEEE infinity/NaN in to_ieee754, not spin in the exponent
	// search. Regression guard for the CodeRabbit critical finding on #993.
	int VerifyDegenerateDenominator(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		double pinf = double(erational(static_cast<std::int64_t>(1),  static_cast<std::uint64_t>(0)));
		double ninf = double(erational(static_cast<std::int64_t>(-1), static_cast<std::uint64_t>(0)));
		double nan0 = double(erational(static_cast<std::int64_t>(0),  static_cast<std::uint64_t>(0)));
		if (!(std::isinf(pinf) && pinf > 0)) { if (reportTestCases) std::cout << "    FAIL 1/0 != +inf (" << pinf << ")\n"; ++nrOfFailedTestCases; }
		if (!(std::isinf(ninf) && ninf < 0)) { if (reportTestCases) std::cout << "    FAIL -1/0 != -inf (" << ninf << ")\n"; ++nrOfFailedTestCases; }
		if (!std::isnan(nan0))               { if (reportTestCases) std::cout << "    FAIL 0/0 != nan (" << nan0 << ")\n"; ++nrOfFailedTestCases; }
		return nrOfFailedTestCases;
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
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "erational ieee754 conversion";
	std::string test_tag            = "conversion";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifySpecificConversions(reportTestCases), "erational", "specific values");
	nrOfFailedTestCases += ReportTestResult(VerifyDegenerateDenominator(reportTestCases), "erational", "degenerate denominator");

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleRoundTrip(reportTestCases, 2000), "erational", "double roundtrip");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleRoundTrip(reportTestCases, 2000),     "erational", "double roundtrip x2k");
	nrOfFailedTestCases += ReportTestResult(VerifyFloatRoundTrip(reportTestCases, 2000),      "erational", "float roundtrip x2k");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRoundTrip(reportTestCases, 2000), "erational", "arithmetic roundtrip x2k");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleRoundTrip(reportTestCases, 20000),     "erational", "double roundtrip x20k");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticRoundTrip(reportTestCases, 20000), "erational", "arithmetic roundtrip x20k");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleRoundTrip(reportTestCases, 100000), "erational", "double roundtrip x100k");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleRoundTrip(reportTestCases, 500000), "erational", "double roundtrip x500k");
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
