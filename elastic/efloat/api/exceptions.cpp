// exceptions.cpp: regression tests for efloat exception and status flags.
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
	int VerifyEfloatExceptions(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// ---------------------------------------------------------------------
		// 1. Initial State & Clearing Flags
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Query and Clear interface...\n";
		{
			clear_efloat_exceptions();
			if (get_efloat_exceptions() != 0) {
				if (reportTestCases) std::cout << "    FAIL: exceptions not cleared initially\n";
				++nrOfFailedTestCases;
			}
			efloat_exception_flags.set(ExceptionFlag::Inexact);
			if (!has_efloat_exception(ExceptionFlag::Inexact)) {
				if (reportTestCases) std::cout << "    FAIL: efloat_exception_flags.has failed\n";
				++nrOfFailedTestCases;
			}
			clear_efloat_exceptions();
			if (get_efloat_exceptions() != 0) {
				if (reportTestCases) std::cout << "    FAIL: exceptions did not clear\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 2. DivisionByZero Exception
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  DivisionByZero Exception...\n";
		{
			clear_efloat_exceptions();
			efloat<4> a(1.0);
			efloat<4> zero(0.0);
			a /= zero; // finite / 0 -> DivByZero
			if (!has_efloat_exception(ExceptionFlag::DivisionByZero)) {
				if (reportTestCases) std::cout << "    FAIL: 1.0 / 0.0 did not raise DivisionByZero\n";
				++nrOfFailedTestCases;
			}
			if (has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: 1.0 / 0.0 erroneously raised InvalidOperation\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 3. InvalidOperation Exception
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  InvalidOperation Exception...\n";
		{
			// Test 3a: 0 / 0 -> NaN + InvalidOperation
			clear_efloat_exceptions();
			efloat<4> a(0.0);
			efloat<4> zero(0.0);
			a /= zero;
			if (!has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: 0.0 / 0.0 did not raise InvalidOperation\n";
				++nrOfFailedTestCases;
			}

			// Test 3b: Inf - Inf -> NaN + InvalidOperation
			clear_efloat_exceptions();
			efloat<4> inf(std::numeric_limits<double>::infinity());
			efloat<4> neg_inf(-std::numeric_limits<double>::infinity());
			inf += neg_inf;
			if (!has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: +Inf + -Inf did not raise InvalidOperation\n";
				++nrOfFailedTestCases;
			}

			// Test 3c: Inf * 0 -> NaN + InvalidOperation
			clear_efloat_exceptions();
			efloat<4> inf2(std::numeric_limits<double>::infinity());
			efloat<4> zero2(0.0);
			inf2 *= zero2;
			if (!has_efloat_exception(ExceptionFlag::InvalidOperation)) {
				if (reportTestCases) std::cout << "    FAIL: Inf * 0.0 did not raise InvalidOperation\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 4. Inexact Exception during Arithmetic
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Inexact Exception during Arithmetic...\n";
		{
			clear_efloat_exceptions();
			efloat<1> a(1.0);
			efloat<1> three(3.0);
			a /= three; // 1 / 3 is inexact (recurring binary)
			if (!has_efloat_exception(ExceptionFlag::Inexact)) {
				if (reportTestCases) std::cout << "    FAIL: 1.0 / 3.0 did not raise Inexact\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 5. Underflow Exception during Conversion
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Underflow Exception during Conversion...\n";
		{
			clear_efloat_exceptions();
			// Float underflow range: -127 subnormal space with discarded non-zero bits.
			// efloat exponent -135, significand has non-zero bits (unnormalized).
			// Converting to float must be inexact and underflow!
			efloat<4> a(false, -135, { 0x80000080 }); 
			float f = float(a);
			(void)f;
			if (!has_efloat_exception(ExceptionFlag::Underflow)) {
				if (reportTestCases) std::cout << "    FAIL: subnormal float conversion did not raise Underflow\n";
				++nrOfFailedTestCases;
			}
			if (!has_efloat_exception(ExceptionFlag::Inexact)) {
				if (reportTestCases) std::cout << "    FAIL: subnormal float conversion did not raise Inexact\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 6. Overflow Exception during Conversion
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Overflow Exception during Conversion...\n";
		{
			clear_efloat_exceptions();
			// Float overflow: exponent 128 exceeds max normal exponent of 127
			efloat<4> a(false, 128, { 0x80000000 });
			float f = float(a);
			(void)f;
			if (!has_efloat_exception(ExceptionFlag::Overflow)) {
				if (reportTestCases) std::cout << "    FAIL: overflow float conversion did not raise Overflow\n";
				++nrOfFailedTestCases;
			}
			if (!has_efloat_exception(ExceptionFlag::Inexact)) {
				if (reportTestCases) std::cout << "    FAIL: overflow float conversion did not raise Inexact\n";
				++nrOfFailedTestCases;
			}
		}

		clear_efloat_exceptions();
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

	std::string test_suite          = "efloat exception and status flags";
	std::string test_tag            = "exceptions";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatExceptions(reportTestCases), "efloat", "exceptions manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatExceptions(reportTestCases), "efloat", "exceptions foundational");
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
