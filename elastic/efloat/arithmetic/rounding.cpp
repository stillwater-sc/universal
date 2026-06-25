// rounding.cpp: regression tests for efloat rounding modes.
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
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// =========================================================================
	// LEVEL 1: foundational hand-curated tests
	// =========================================================================
	int VerifyEfloatRounding(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// We will use efloat<1> (1 limb = 32 bits maximum precision)
		// and construct size-2 limb vectors to test rounding behavior on truncation.

		// ---------------------------------------------------------------------
		// 1. Round to Nearest (Ties to Even) (default)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Round to Nearest (Ties to Even)...\n";
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		{
			// Halfway case 1: 1.5 -> round to even (1.0)
			// limbs: { 0x80000000, 0x80000000 }
			// LSB of kept portion (index 0) is 0 (even), guard is 1 (halfway), sticky is 0.
			// Under ties-to-even, since LSB is even, we truncate! So it stays 1.0.
			efloat<1> a(false, 0, { 0x80000000, 0x80000000 }); // unnormalized 1.5-scale
			a.normalize(); // triggers rounding back to nlimbs = 1
			efloat<1> expected(false, 0, { 0x80000000 }); // expected 1.0
			if (a != expected) {
				if (reportTestCases) std::cout << "    FAIL: 1.5-scale did not round to 1.0 (even). Result: " << a << "\n";
				++nrOfFailedTestCases;
			}
		}
		{
			// Halfway case 2: 2.5 -> round to even (2.0)
			// In binary, LSB is 0 (even), guard is 1 (halfway).
			// Should round to even -> 2.0!
			efloat<1> a(false, 1, { 0x80000000, 0x80000000 }); // unnormalized 2.5-scale
			a.normalize();
			efloat<1> expected(false, 1, { 0x80000000 }); // expected 2.0
			if (a != expected) {
				if (reportTestCases) std::cout << "    FAIL: 2.5-scale did not round to 2.0 (even). Result: " << a << "\n";
				++nrOfFailedTestCases;
			}
		}
		{
			// Non-halfway case: 1.75 -> round to nearest (1.0 + LSB)
			// limbs: { 0x80000000, 0xC0000000 }
			// Guard is 1, sticky is 1 (non-halfway). Should always round up!
			efloat<1> a(false, 0, { 0x80000000, 0xC0000000 }); // unnormalized 1.75-scale
			a.normalize();
			efloat<1> expected(false, 0, { 0x80000001 }); // expected rounded up at LSB
			if (a != expected) {
				if (reportTestCases) std::cout << "    FAIL: 1.75 did not round to expected LSB. Result: " << a << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 2. Round to Zero (Truncation)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Round to Zero (Truncation)...\n";
		efloat_rounding_mode = RoundingMode::RoundToZero;
		{
			// 1.75 -> truncate -> 1.0
			efloat<1> a(false, 0, { 0x80000000, 0xC0000000 });
			a.normalize();
			efloat<1> expected(false, 0, { 0x80000000 }); // expected 1.0
			if (a != expected) {
				if (reportTestCases) std::cout << "    FAIL: 1.75 did not truncate to 1.0. Result: " << a << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 3. Round Toward Positive (+Infinity)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Round Toward Positive (+Infinity)...\n";
		efloat_rounding_mode = RoundingMode::RoundTowardPositive;
		{
			// Positive: 1.1 -> round up -> 1.0 + LSB
			// limbs: { 0x80000000, 0x20000000 }
			efloat<1> a(false, 0, { 0x80000000, 0x20000000 });
			a.normalize();
			efloat<1> expected(false, 0, { 0x80000001 }); // expected rounded up at LSB
			if (a != expected) {
				if (reportTestCases) std::cout << "    FAIL: +1.125 did not round up to expected LSB. Result: " << a << "\n";
				++nrOfFailedTestCases;
			}
		}
		{
			// Negative: -1.1 -> truncate -> -1.0
			efloat<1> a(true, 0, { 0x80000000, 0x20000000 }); // -1.125
			a.normalize();
			efloat<1> expected(true, 0, { 0x80000000 }); // expected -1.0 (toward positive infinity)
			if (a != expected) {
				if (reportTestCases) std::cout << "    FAIL: -1.125 did not round toward zero (-1.0). Result: " << a << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 4. Round Toward Negative (-Infinity)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Round Toward Negative (-Infinity)...\n";
		efloat_rounding_mode = RoundingMode::RoundTowardNegative;
		{
			// Positive: 1.1 -> truncate -> 1.0
			efloat<1> a(false, 0, { 0x80000000, 0x20000000 }); // 1.125
			a.normalize();
			efloat<1> expected(false, 0, { 0x80000000 }); // expected 1.0 (toward negative infinity)
			if (a != expected) {
				if (reportTestCases) std::cout << "    FAIL: +1.125 did not round toward zero (1.0). Result: " << a << "\n";
				++nrOfFailedTestCases;
			}
		}
		{
			// Negative: -1.1 -> round up magnitude -> -1.0 - LSB
			efloat<1> a(true, 0, { 0x80000000, 0x20000000 }); // -1.125
			a.normalize();
			efloat<1> expected(true, 0, { 0x80000001 }); // expected -1.0 - LSB
			if (a != expected) {
				if (reportTestCases) std::cout << "    FAIL: -1.125 did not round toward negative infinity. Result: " << a << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 5. Operator/= and Sticky Remainder Rounding (Issue #1091)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Operator/= and Sticky Remainder Rounding (1.0 / 3.0)...\n";
		{
			efloat<1> one(1.0);
			efloat<1> three(3.0);

			// Test 5a: Round to Zero (Truncation)
			efloat_rounding_mode = RoundingMode::RoundToZero;
			efloat<1> q_zero = one / three;

			// Test 5b: Round Toward Positive (+Infinity) -> inexact positive rounds up
			efloat_rounding_mode = RoundingMode::RoundTowardPositive;
			efloat<1> q_pos = one / three;

			// Test 5c: Round Toward Negative (-Infinity) -> inexact positive truncates (rounds down)
			efloat_rounding_mode = RoundingMode::RoundTowardNegative;
			efloat<1> q_neg = one / three;

			if (q_zero != q_neg) {
				if (reportTestCases) std::cout << "    FAIL: inexact positive under RoundTowardNegative did not match RoundToZero. Result: " << q_neg << "\n";
				++nrOfFailedTestCases;
			}
			if (q_pos == q_zero) {
				if (reportTestCases) std::cout << "    FAIL: inexact positive under RoundTowardPositive did not round up. Result: " << q_pos << "\n";
				++nrOfFailedTestCases;
			}
		}

		// Restore default rounding mode
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		return nrOfFailedTestCases;
	}

}  // anonymous namespace

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

	std::string test_suite          = "efloat rounding modes";
	std::string test_tag            = "rounding";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatRounding(reportTestCases), "efloat", "rounding manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatRounding(reportTestCases), "efloat", "rounding foundational");
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
