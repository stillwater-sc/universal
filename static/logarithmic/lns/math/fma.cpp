// fma.cpp: functional tests for the lns (logarithmic number system) fused multiply-add
//
// lns had no fma (#1196, sub-issue of the universal fma epic #1189). In a logarithmic
// number system the multiply is (near-)exact (an exponent addition) while the ADD is
// the lossy step; fma forms a*b + c in a wide double intermediate and rounds the result
// once into lns via the value constructor, so the add-domain rounding happens exactly
// once. Log-domain encodings do not represent "nice" linear values exactly (e.g. 2.5),
// so this suite does not test exactness; it validates correctly-rounded agreement with
// an INDEPENDENT long-double reference (a*b + c formed off the implementation's
// double/std::fma path) and NaN propagation.
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

#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// correctly rounded vs an independent long-double reference
	template<unsigned nbits, unsigned rbits, typename bt = uint16_t>
	int VerifyCorrectlyRounded(bool reportTestCases, int nrTests, uint64_t seed) {
		using Lns = lns<nbits, rbits, bt>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-4.0, 4.0);
		for (int t = 0; t < nrTests; ++t) {
			Lns a(U(rng)), b(U(rng)), c(U(rng));
			Lns r = fma(a, b, c);
			// independent fused reference in long double: exact product then a single rounding
			// (a plain da*db + dc would double-round and amplify cancellation -- which is exactly
			// what fma avoids, so it must not be the oracle).
			long double tru = std::fmal((long double)double(a), (long double)double(b), (long double)double(c));
			Lns expected{ double(tru) };   // lns rounds the reference the same way it would the exact result
			if (r != expected) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL fma(" << double(a) << ", " << double(b) << ", " << double(c)
					<< ") = " << double(r) << "  expected " << double(expected) << '\n';
			}
		}
		return fails;
	}

	// NaN propagation (lns has no infinity: isinf() is always false)
	template<unsigned nbits, unsigned rbits, typename bt = uint16_t>
	int VerifyNaN(bool reportTestCases) {
		using Lns = lns<nbits, rbits, bt>;
		int fails = 0;
		const Lns nan(std::numeric_limits<double>::quiet_NaN());
		const Lns one(1.0), two(2.0);
		if (!fma(nan, one, two).isnan()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(NaN,1,2) not NaN\n"; }
		if (!fma(one, nan, two).isnan()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,NaN,2) not NaN\n"; }
		if (!fma(one, two, nan).isnan()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,2,NaN) not NaN\n"; }
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
	std::string test_suite = "lns fused multiply-add fma(a,b,c) (#1196)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyNaN<16, 8>(true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 10000;
#if REGRESSION_LEVEL_2
	base = 50000;
#endif

	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<16, 8>(reportTestCases, base, 0x1A20A), "fma correctly-rounded lns<16,8>", "fma");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<24, 12>(reportTestCases, base, 0x1A21B), "fma correctly-rounded lns<24,12>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyNaN<16, 8>(reportTestCases), "fma NaN propagation lns<16,8>", "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
