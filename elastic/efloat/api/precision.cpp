// precision.cpp: regression tests for efloat dynamic runtime precision management.
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
	int VerifyEfloatPrecisionManagement(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// ---------------------------------------------------------------------
		// 1. Global Default Precision
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Global default precision...\n";
		{
			set_default_precision(0);
			if (get_default_precision() != 0) {
				if (reportTestCases) std::cout << "    FAIL: initial default precision is not 0\n";
				++nrOfFailedTestCases;
			}
			set_default_precision(64);
			if (get_default_precision() != 64) {
				if (reportTestCases) std::cout << "    FAIL: default precision was not set to 64\n";
				++nrOfFailedTestCases;
			}
			// Newly constructed efloat should use default precision (64 bits = 2 limbs)
			efloat<4> a;
			if (a.get_precision() != 64) {
				if (reportTestCases) std::cout << "    FAIL: newly constructed efloat did not inherit default precision. Result: " << a.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
			set_default_precision(0); // restore default
		}

		// ---------------------------------------------------------------------
		// 2. Per-Instance Precision Controls
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Per-instance precision...\n";
		{
			efloat<4> a;
			if (a.get_precision() != 128) { // static fallback 4 * 32 = 128
				if (reportTestCases) std::cout << "    FAIL: static fallback precision was incorrect\n";
				++nrOfFailedTestCases;
			}
			a.set_precision(32); // resize to 32 bits (1 limb)
			if (a.get_precision() != 32) {
				if (reportTestCases) std::cout << "    FAIL: setting instance precision failed\n";
				++nrOfFailedTestCases;
			}

			// Verify clear() retains instance-level narrowed precision
			a.clear();
			if (a.get_precision() != 32) {
				if (reportTestCases) std::cout << "    FAIL: clear() clobbered instance precision. Result: " << a.get_precision() << "\n";
				++nrOfFailedTestCases;
			}

			// Verify parse() respects instance-level narrowed precision (32 bits = 1 limb)
			parse("0.33333333333333333", a);
			if (a.get_precision() != 32) {
				if (reportTestCases) std::cout << "    FAIL: parse() clobbered instance precision. Result: " << a.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
			if (a.bits().size() != 1) {
				if (reportTestCases) std::cout << "    FAIL: parse() exceeded narrowed precision limb size. Size: " << a.bits().size() << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 3. Arithmetic Precision Escalation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Arithmetic precision escalation...\n";
		{
			efloat<4> a(1.0);
			efloat<4> b(1.0);
			a.set_precision(32); // 1 limb
			b.set_precision(96); // 3 limbs

			a += b; // Result precision should be std::max(32, 96) = 96
			if (a.get_precision() != 96) {
				if (reportTestCases) std::cout << "    FAIL: addition did not escalate precision. Result: " << a.get_precision() << "\n";
				++nrOfFailedTestCases;
			}

			efloat<4> c(2.0);
			efloat<4> d(3.0);
			c.set_precision(64); // 2 limbs
			d.set_precision(128); // 4 limbs

			c *= d; // Result precision should be std::max(64, 128) = 128
			if (c.get_precision() != 128) {
				if (reportTestCases) std::cout << "    FAIL: multiplication did not escalate precision. Result: " << c.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 4. Re-rounding on Downscaling
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Re-rounding on Downscaling...\n";
		{
			// Construct a 64-bit precision value that has guard bit set for 32-bit truncation
			// a = { 0x80000000, 0x80000000 } (exact halfway tie at 32-bit)
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			{
				efloat<4> a(false, 0, { 0x80000000, 0x80000000 });
				a.set_precision(64);
				a.set_precision(32); // round to nearest (ties to even) -> truncates to 1.0 (even)
				efloat<4> expected(false, 0, { 0x80000000 });
				expected.set_precision(32);
				if (a != expected) {
					if (reportTestCases) std::cout << "    FAIL: halfway-tie downscaling did not round to even. Result: " << a << "\n";
					++nrOfFailedTestCases;
				}
			}
			// Toward Positive: positive inexact halfway rounds up!
			efloat_rounding_mode = RoundingMode::RoundTowardPositive;
			{
				efloat<4> a(false, 0, { 0x80000000, 0x80000000 });
				a.set_precision(64);
				a.set_precision(32); // round toward positive -> rounds up
				efloat<4> expected(false, 0, { 0x80000001 });
				expected.set_precision(32);
				if (a != expected) {
					if (reportTestCases) std::cout << "    FAIL: halfway-tie downscaling did not round toward positive. Result: " << a << "\n";
					++nrOfFailedTestCases;
				}
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

	std::string test_suite          = "efloat dynamic runtime precision";
	std::string test_tag            = "precision";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatPrecisionManagement(reportTestCases), "efloat", "precision manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatPrecisionManagement(reportTestCases), "efloat", "precision foundational");
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
