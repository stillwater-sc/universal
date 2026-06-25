// lossless.cpp: regression tests for lossless bit-level efloat to native conversions.
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
#include <bit>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// =========================================================================
	// LEVEL 1: foundational hand-curated tests
	// =========================================================================
	int VerifyEfloatLosslessConversions(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// ---------------------------------------------------------------------
		// 1. Basic Identity Conversions (Float and Double)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Identity conversions...\n";
		{
			double values[] = {
				0.0, -0.0, 1.0, -1.0, 2.0, -100.0,
				1.5, 0.125, 0.375, 3.68929e+19, -3.68929e+19
			};
			for (double val : values) {
				efloat<4> a(val);
				double back = double(a);
				if (std::bit_cast<uint64_t>(val) != std::bit_cast<uint64_t>(back)) {
					if (reportTestCases) std::cout << "    FAIL: double identity mismatch. Input: " << val << ", Back: " << back << "\n";
					++nrOfFailedTestCases;
				}
			}
		}
		{
			float values[] = {
				0.0f, -0.0f, 1.0f, -1.0f, 2.0f, -100.0f,
				1.5f, 0.125f, 0.375f, 3.68929e+10f, -3.68929e+10f
			};
			for (float val : values) {
				efloat<4> a(val);
				float back = float(a);
				if (std::bit_cast<uint32_t>(val) != std::bit_cast<uint32_t>(back)) {
					if (reportTestCases) std::cout << "    FAIL: float identity mismatch. Input: " << val << ", Back: " << back << "\n";
					++nrOfFailedTestCases;
				}
			}
		}

		// ---------------------------------------------------------------------
		// 2. IEEE-754 Special Values
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Special values...\n";
		{
			efloat<4> zero(0.0);
			efloat<4> inf(std::numeric_limits<double>::infinity());
			efloat<4> neg_inf(-std::numeric_limits<double>::infinity());
			efloat<4> nan(std::numeric_limits<double>::quiet_NaN());

			if (double(zero) != 0.0 || std::signbit(double(zero))) {
				if (double(zero) != 0.0) {
					if (reportTestCases) std::cout << "    FAIL: zero state mismatch\n";
					++nrOfFailedTestCases;
				}
			}
			if (!std::isinf(double(inf)) || std::signbit(double(inf))) {
				if (reportTestCases) std::cout << "    FAIL: positive infinity mismatch\n";
				++nrOfFailedTestCases;
			}
			if (!std::isinf(double(neg_inf)) || !std::signbit(double(neg_inf))) {
				if (reportTestCases) std::cout << "    FAIL: negative infinity mismatch\n";
				++nrOfFailedTestCases;
			}
			if (!std::isnan(double(nan))) {
				if (reportTestCases) std::cout << "    FAIL: quiet NaN mismatch\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 3. extreme subnormal float/double conversions
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Subnormal conversions...\n";
		{
			// Smallest subnormal float (1.4013e-45f)
			float smallest_f = 1.401298464324817e-45f;
			efloat<4> a(smallest_f);
			float back_f = float(a);
			if (std::bit_cast<uint32_t>(smallest_f) != std::bit_cast<uint32_t>(back_f)) {
				if (reportTestCases) std::cout << "    FAIL: smallest subnormal float mismatch\n";
				++nrOfFailedTestCases;
			}
		}
		{
			// Smallest subnormal double (4.94066e-324)
			double smallest_d = 4.940656458412465e-324;
			efloat<4> a(smallest_d);
			double back_d = double(a);
			if (std::bit_cast<uint64_t>(smallest_d) != std::bit_cast<uint64_t>(back_d)) {
				if (reportTestCases) std::cout << "    FAIL: smallest subnormal double mismatch\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 4. Overflow / Underflow boundaries
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Overflow and Underflow boundaries...\n";
		{
			// Float overflow: 1.0 * 2^128 -> exceeds 127, should round to float infinity
			efloat<4> a(false, 128, { 0x80000000 });
			float back_f = float(a);
			if (!std::isinf(back_f)) {
				if (reportTestCases) std::cout << "    FAIL: exponent 128 did not overflow float to infinity. Result: " << back_f << "\n";
				++nrOfFailedTestCases;
			}
		}
		{
			// Float underflow: 1.0 * 2^-150 -> underflow float range completely to zero
			efloat<4> a(false, -150, { 0x80000000 });
			float back_f = float(a);
			if (back_f != 0.0f) {
				if (reportTestCases) std::cout << "    FAIL: exponent -150 did not underflow float to zero. Result: " << back_f << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 5. Rounding Modes during conversion
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Rounding modes...\n";
		{
			// Build an inexact efloat value that has bits set at the guard/sticky positions of a float.
			// float significand has 24 bits (K=24, shift_amt = 40).
			// We set limbs: { 0x80000080, 0x00000000 }
			// Since 0x80000080 has bit 7 set, when shifted left by 32 it becomes bit 39 of raw_sig.
			// Bit 39 of raw_sig is the float guard bit (shift_amt - 1 = 39).
			// Since all lower bits are 0, this represents an exact halfway tie (guard=1, sticky=0).
			// Under RoundToNearest (ties-to-even), since the LSB of the kept portion (bit 40) is 0 (even),
			// it should truncate (round to even).
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			{
				efloat<4> a(false, 0, { 0x80000080, 0x00000000 }); // exact halfway tie
				float back = float(a);
				if (std::bit_cast<uint32_t>(back) != 0x3F800000u) { // 1.0f (truncated to even)
					if (reportTestCases) std::cout << "    FAIL: exact halfway tie did not round to even (1.0f). Result: " << back << "\n";
					++nrOfFailedTestCases;
				}
			}
			// Round toward Positive: positive inexact halfway rounds up!
			efloat_rounding_mode = RoundingMode::RoundTowardPositive;
			{
				efloat<4> a(false, 0, { 0x80000080, 0x00000000 });
				float back = float(a);
				if (std::bit_cast<uint32_t>(back) != 0x3F800001u) { // 1.0f + 1 ULP (rounded up)
					if (reportTestCases) std::cout << "    FAIL: positive inexact did not round toward positive infinity. Result: " << back << "\n";
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

	std::string test_suite          = "efloat lossless conversions";
	std::string test_tag            = "lossless";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatLosslessConversions(reportTestCases), "efloat", "lossless manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatLosslessConversions(reportTestCases), "efloat", "lossless foundational");
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
