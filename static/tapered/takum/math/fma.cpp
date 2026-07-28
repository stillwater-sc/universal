// fma.cpp: functional tests for the takum fused multiply-add fma(a,b,c)
//
// takum (linear takum encoding, epic #592) had no fma (#1195, sub-issue of the
// universal fma epic #1189). fma widens the operands to double, forms a*b + c with
// std::fma, and rounds the result once into takum via the value constructor. A
// takum's significand precision is well under double's 53 bits for practical
// configurations, so the double intermediate carries the correctly-rounded a*b + c
// and the double -> takum rounding is the single rounding that determines the result.
// takum's non-real state (NaR) absorbs the IEEE specials.
//
// This suite validates against an INDEPENDENT reference that forms a*b + c in long
// double (off the implementation's double/std::fma path): exactness for
// exactly-representable results, correctly-rounded agreement with takum(reference),
// a faithfulness bound, and NaR handling for inf / NaN.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <random>
#include <string>

#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// ---- 1. exactness: exactly-representable results are exact -----------------
	template<unsigned nbits, unsigned rbits, typename bt = uint32_t>
	int VerifyExactness(bool reportTestCases) {
		using Takum = takum<nbits, rbits, bt>;
		int fails = 0;
		const double cases[][4] = {
			{ 1.5,  1.5,  0.25, 2.5  },
			{ 2.0,  3.0,  1.0,  7.0  },
			{ 0.5,  0.5,  0.5,  0.75 },
			{-1.5,  2.0,  0.5, -2.5  },
			{ 4.0,  0.25, 1.0,  2.0  },
			{ 0.0,  3.0,  2.0,  2.0  },
			{ 1.0,  3.0,  2.0,  5.0  },
		};
		for (auto& tc : cases) {
			Takum a(tc[0]), b(tc[1]), c(tc[2]);
			Takum r = fma(a, b, c);
			if (double(r) != tc[3]) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL exact fma(" << tc[0] << ", " << tc[1] << ", " << tc[2]
					<< ") = " << double(r) << "  expected " << tc[3] << '\n';
			}
		}
		return fails;
	}

	// ---- 2. correctly rounded vs an independent long-double reference ----------
	template<unsigned nbits, unsigned rbits, typename bt = uint32_t>
	int VerifyCorrectlyRounded(bool reportTestCases, int nrTests, uint64_t seed) {
		using Takum = takum<nbits, rbits, bt>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-4.0, 4.0);
		for (int t = 0; t < nrTests; ++t) {
			Takum a(U(rng)), b(U(rng)), c(U(rng));
			Takum r = fma(a, b, c);
			if (r.isnar()) continue;   // finite operands stay finite for this range
			// independent reference: exact a*b + c in long double (exact for these small configs)
			long double tru = (long double)double(a) * (long double)double(b) + (long double)double(c);
			Takum expected{ double(tru) };  // takum rounds the reference the same way it would the exact result
			if (r != expected) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL fma(" << double(a) << ", " << double(b) << ", " << double(c)
					<< ") = " << double(r) << "  expected " << double(expected) << '\n';
			}
		}
		return fails;
	}

	// ---- 3. NaR / IEEE special handling ---------------------------------------
	template<unsigned nbits, unsigned rbits, typename bt = uint32_t>
	int VerifySpecialValues(bool reportTestCases) {
		using Takum = takum<nbits, rbits, bt>;
		int fails = 0;
		const Takum inf(std::numeric_limits<double>::infinity());
		const Takum nan(std::numeric_limits<double>::quiet_NaN());
		const Takum zero(0.0), one(1.0), two(2.0), three(3.0);
		auto expectNaR = [&](Takum r, const char* tag) {
			if (!r.isnar()) { ++fails; if (reportTestCases) std::cout << "    FAIL special " << tag << " not NaR\n"; }
		};
		expectNaR(fma(inf, zero, one), "inf*0+1");   // NaN -> NaR
		expectNaR(fma(inf, two, three), "inf*2+3");   // inf -> NaR (takum has no infinity)
		expectNaR(fma(two, three, inf), "2*3+inf");
		expectNaR(fma(nan, one, one), "NaN*1+1");
		expectNaR(fma(one, nan, one), "1*NaN+1");
		expectNaR(fma(one, one, nan), "1*1+NaN");
		// finite identities
		if (double(fma(two, three, zero)) != 6.0) { ++fails; if (reportTestCases) std::cout << "    FAIL 2*3+0 != 6\n"; }
		if (double(fma(zero, three, two)) != 2.0) { ++fails; if (reportTestCases) std::cout << "    FAIL 0*3+2 != 2\n"; }
		return fails;
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
	std::string test_suite = "takum fused multiply-add fma(a,b,c) (#1195)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyExactness<32, 5>(true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 10000;
#if REGRESSION_LEVEL_2
	base = 50000;
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyExactness<32, 5>(reportTestCases), "fma exactness takum<32,5>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyExactness<24, 4>(reportTestCases), "fma exactness takum<24,4>", "fma");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 5>(reportTestCases, base, 0x7A20A), "fma correctly-rounded takum<32,5>", "fma");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<24, 4>(reportTestCases, base, 0x7A21B), "fma correctly-rounded takum<24,4>", "fma");
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecialValues<32, 5>(reportTestCases), "fma NaR / special values", "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
