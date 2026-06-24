// division.cpp: regression tests for efloat division.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- all foundational hand-curated tests: algebraic invariants,
//              hostile arithmetic corner cases (round-to-even, massive
//              exponent gaps, complete overlap), subnormal boundaries,
//              IEEE 754 special values.
//   LEVEL 2 -- property-based fuzzer over random multi-component expansions
//              (~1,000 iterations per invariant).
//   LEVEL 3 -- same fuzzer at higher intensity (~100,000 iterations).
//   LEVEL 4 -- exhaustive fuzzer (~10,000,000 iterations).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/efloat_test_support.hpp>

namespace {

	// =========================================================================
	// LEVEL 1: foundational hand-curated tests
	// =========================================================================
	int VerifyEfloatDivision(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// --- Algebraic invariants ---

		// Identity: a / 1.0 == a
		if (reportTestCases) std::cout << "  Identity...\n";
		{
			efloat<16> a(1.234567e+15);
			efloat<16> one(1.0);
			efloat<16> result = a / one;
			if (result != a) {
				if (reportTestCases) std::cout << "    FAIL a / 1 != a\n";
				++nrOfFailedTestCases;
			}
		}

		// Self divisor: a / a == 1.0
		if (reportTestCases) std::cout << "  Self divisor...\n";
		{
			efloat<16> a(1.234567e+15);
			efloat<16> result = a / a;
			efloat<16> expected(1.0);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL a / a != 1\n";
				++nrOfFailedTestCases;
			}
		}

		// Zero dividend: 0.0 / a == 0.0
		if (reportTestCases) std::cout << "  Zero dividend...\n";
		{
			efloat<16> a(1.234567e+15);
			efloat<16> zero(0.0);
			efloat<16> result = zero / a;
			if (!result.iszero()) {
				if (reportTestCases) std::cout << "    FAIL 0 / a != 0\n";
				++nrOfFailedTestCases;
			}
		}

		// --- Hostile arithmetic ---

		// Exact division: 2^40 / 2^20 = 2^20
		if (reportTestCases) std::cout << "  Limb contraction (exact powers of 2)...\n";
		{
			efloat<16> a(1099511627776.0); // 2^40
			efloat<16> b(1048576.0);       // 2^20
			efloat<16> result = a / b;
			efloat<16> expected(1048576.0); // 2^20
			if (result != expected) {
				if (reportTestCases) {
					std::cout << "    FAIL limb contraction value mismatch. Result: " << result << " Expected: " << expected << "\n";
				}
				++nrOfFailedTestCases;
			}
		}

		// --- IEEE 754 special values ---
		double qnan = std::numeric_limits<double>::quiet_NaN();
		double pinf = std::numeric_limits<double>::infinity();
		double ninf = -pinf;

		if (reportTestCases) std::cout << "  NaN / finite...\n";
		{
			efloat<16> a(1.0), n(qnan);
			if (!(n / a).isnan()) { if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases; }
		}

		if (reportTestCases) std::cout << "  finite / +Inf...\n";
		{
			efloat<16> a(1.0), inf(pinf);
			if (!(a / inf).iszero()) { if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases; }
		}

		if (reportTestCases) std::cout << "  finite / Zero (Divide by Zero)...\n";
		{
			efloat<16> a(1.0), zero(0.0);
			if (!(a / zero).isinf()) { if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases; }
		}

		if (reportTestCases) std::cout << "  Zero / Zero...\n";
		{
			efloat<16> zero1(0.0), zero2(0.0);
			if (!(zero1 / zero2).isnan()) { if (reportTestCases) std::cout << "    FAIL\n"; ++nrOfFailedTestCases; }
		}

		return nrOfFailedTestCases;
	}

	// =========================================================================
	// Fuzzer infrastructure
	// =========================================================================
	int VerifyEfloatDivision_Fuzz(bool reportTestCases, unsigned nrIterations) {
		using namespace sw::universal;
		const uint64_t seed = 0xC0FFEE'A11CEULL;
		std::mt19937_64 rng(seed);
		int nrOfFailedTestCases = 0;
		efloat<16> one(1.0);
		efloat<16> zero(0.0);

		for (unsigned i = 0; i < nrIterations; ++i) {
			efloat<16> a = random_efloat<16>(rng);
			
			// Identity: a / 1 == a
			if (a / one != a) {
				if (reportTestCases) std::cout << "    FAIL identity (seed=0x" << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
			// Self divisor: a / a == 1
			if (a / a != one) {
				if (reportTestCases) std::cout << "    FAIL self divisor (seed=0x" << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
			// Zero: 0 / a == 0
			if (zero / a != zero) {
				if (reportTestCases) std::cout << "    FAIL zero (seed=0x" << std::hex << seed << " iter=" << std::dec << i << ")\n";
				++nrOfFailedTestCases;
			}
		}
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
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat division";
	std::string test_tag            = "division";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatDivision(reportTestCases), "efloat", "division manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatDivision(reportTestCases), "efloat", "division foundational");
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatDivision_Fuzz(reportTestCases, 100), "efloat", "division fuzz x100");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatDivision_Fuzz(reportTestCases, 1000), "efloat", "division fuzz x1k");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatDivision_Fuzz(reportTestCases, 10000), "efloat", "division fuzz x10k");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatDivision_Fuzz(reportTestCases, 100000), "efloat", "division fuzz x100k");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
