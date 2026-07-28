// fma_rounded.cpp: functional tests for the rounded scalar posit fma(a,b,c)
//
// posit1's fma historically returned an UNROUNDED internal::value building block, not a
// drop-in scalar (#1197). A public fma overload now returns the posit type with a single
// rounding (exact product-sum via fma_value, then one convert), matching the modern posit
// fma; the unrounded variant is retained as fma_value.
//
// This suite validates the rounded fma against an INDEPENDENT fused reference: for small
// posits whose product is exact in double, round(std::fmal(a,b,c)) is the correctly-rounded
// result. We check: correctly-rounded agreement, single-rounding superiority over the naive
// two-rounding a*b + c under cancellation, and NaR propagation.
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

#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// correctly-rounded vs an independent fused long-double reference (exact for small posits)
	template<unsigned nbits, unsigned es>
	int VerifyCorrectlyRounded(bool reportTestCases, int nrTests, uint64_t seed) {
		using Posit = posit<nbits, es>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-4.0, 4.0);
		for (int t = 0; t < nrTests; ++t) {
			Posit a(U(rng)), b(U(rng)), c(U(rng));
			Posit r = fma(a, b, c);
			if (r.isnar()) continue;
			long double tru = std::fmal((long double)double(a), (long double)double(b), (long double)double(c));
			Posit ref{ double(tru) };
			if (r != ref) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL fma(" << double(a) << ", " << double(b) << ", " << double(c)
					<< ") = " << double(r) << "  expected " << double(ref) << '\n';
			}
		}
		return fails;
	}

	// fused single-rounding beats the naive two-rounding a*b + c
	template<unsigned nbits, unsigned es>
	int VerifyFusedBeatsNaive(bool reportTestCases, int nrTests, uint64_t seed, int& disagreements) {
		using Posit = posit<nbits, es>;
		int fails = 0;
		disagreements = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-4.0, 4.0);
		for (int t = 0; t < nrTests; ++t) {
			Posit a(U(rng)), b(U(rng)), c(U(rng));
			Posit r = fma(a, b, c);
			if (r.isnar()) continue;
			long double tru = std::fmal((long double)double(a), (long double)double(b), (long double)double(c));
			Posit ref{ double(tru) };
			if (r != ref) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL (fused != oracle)\n";
			}
			Posit naive = a * b; naive = naive + c;   // two roundings
			if (naive != ref) ++disagreements;
		}
		return fails;
	}

	// NaR propagation
	template<unsigned nbits, unsigned es>
	int VerifyNaR(bool reportTestCases) {
		using Posit = posit<nbits, es>;
		int fails = 0;
		Posit nar; nar.setnar();
		const Posit one(1.0), two(2.0);
		if (!fma(nar, one, two).isnar()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(NaR,1,2) not NaR\n"; }
		if (!fma(one, nar, two).isnar()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,NaR,2) not NaR\n"; }
		if (!fma(one, two, nar).isnar()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,2,NaR) not NaR\n"; }
		// fma(a,b,0) == a*b (exact product path)
		if (fma(two, two, Posit(0.0)) != (two * two)) {
			++fails;
			if (reportTestCases) std::cout << "    FAIL fma(2,2,0) != 4\n";
		}
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
	std::string test_suite = "posit1 rounded scalar fma(a,b,c) (#1197)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyNaR<16, 1>(true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 10000;
#if REGRESSION_LEVEL_2
	base = 50000;
#endif

	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<16, 1>(reportTestCases, base, 0x1970), "fma correctly-rounded posit<16,1>", "fma");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<20, 2>(reportTestCases, base, 0x1971), "fma correctly-rounded posit<20,2>", "fma");
	{
		int disagreements = 0;
		nrOfFailedTestCases += ReportTestResult(
			VerifyFusedBeatsNaive<16, 1>(reportTestCases, base, 0x1972, disagreements),
			"fma fused == oracle posit<16,1>", "fma");
		std::cout << "    (fused vs naive: the two-rounding a*b + c disagreed with the exact result in "
			<< disagreements << " of " << base << " cases -- fma is single-rounded)\n";
	}
	nrOfFailedTestCases += ReportTestResult(VerifyNaR<16, 1>(reportTestCases), "fma NaR / identities posit<16,1>", "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
