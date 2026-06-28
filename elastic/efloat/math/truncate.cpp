// truncate.cpp: regression tests for efloat mathematical truncation and rounding functions.
//
// Categories tested:
//   - trunc, floor, ceil, round, rint, nearbyint, lrint, llrint
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

	int VerifyEfloatTruncation(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. trunc Validation (Round toward Zero)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying trunc...\n";
		{
			// Positive: trunc(1.75) == 1.0
			efloat<4> pos(1.75);
			if (trunc(pos) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: trunc(1.75) is not 1.0. Result: " << double(trunc(pos)) << "\n";
				++failures;
			}
			// Negative: trunc(-1.75) == -1.0
			efloat<4> neg(-1.75);
			if (trunc(neg) != -1.0) {
				if (reportTestCases) std::cout << "    FAIL: trunc(-1.75) is not -1.0. Result: " << double(trunc(neg)) << "\n";
				++failures;
			}
			// Inexact check
			clear_efloat_exceptions();
			trunc(pos);
			if (!has_efloat_exception(ExceptionFlag::Inexact)) {
				if (reportTestCases) std::cout << "    FAIL: trunc(1.75) did not raise Inexact\n";
				++failures;
			}
			// Negative Zero: trunc(-0.25) == -0.0 (preserves negative sign)
			efloat<4> small_neg(-0.25);
			efloat<4> res_trunc = trunc(small_neg);
			if (res_trunc != 0.0 || res_trunc.sign() != -1) {
				if (reportTestCases) std::cout << "    FAIL: trunc(-0.25) did not return negative zero. Result: " << double(res_trunc) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. floor Validation (Round toward -Infinity)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying floor...\n";
		{
			// Positive: floor(1.75) == 1.0
			efloat<4> pos(1.75);
			if (floor(pos) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: floor(1.75) is not 1.0\n";
				++failures;
			}
			// Negative: floor(-1.75) == -2.0
			efloat<4> neg(-1.75);
			if (floor(neg) != -2.0) {
				if (reportTestCases) std::cout << "    FAIL: floor(-1.75) is not -2.0. Result: " << double(floor(neg)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. ceil Validation (Round toward +Infinity)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying ceil...\n";
		{
			// Positive: ceil(1.75) == 2.0
			efloat<4> pos(1.75);
			if (ceil(pos) != 2.0) {
				if (reportTestCases) std::cout << "    FAIL: ceil(1.75) is not 2.0\n";
				++failures;
			}
			// Negative: ceil(-1.75) == -1.0
			efloat<4> neg(-1.75);
			if (ceil(neg) != -1.0) {
				if (reportTestCases) std::cout << "    FAIL: ceil(-1.75) is not -1.0\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. round Validation (Round to Nearest, Halfway away from Zero)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying round...\n";
		{
			// Halfway positive: round(1.5) == 2.0
			efloat<4> pos_half(1.5);
			if (round(pos_half) != 2.0) {
				if (reportTestCases) std::cout << "    FAIL: round(1.5) is not 2.0. Result: " << double(round(pos_half)) << "\n";
				++failures;
			}
			// Halfway negative: round(-1.5) == -2.0
			efloat<4> neg_half(-1.5);
			if (round(neg_half) != -2.0) {
				if (reportTestCases) std::cout << "    FAIL: round(-1.5) is not -2.0. Result: " << double(round(neg_half)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 5. rint Validation (Round to Nearest, Halfway to Even)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying rint...\n";
		{
			// Halfway to even 1: rint(1.5) == 2.0 (nearest even integer)
			efloat<4> half1(1.5);
			if (rint(half1) != 2.0) {
				if (reportTestCases) std::cout << "    FAIL: rint(1.5) is not 2.0\n";
				++failures;
			}
			// Halfway to even 2: rint(2.5) == 2.0 (nearest even integer)
			efloat<4> half2(2.5);
			if (rint(half2) != 2.0) {
				if (reportTestCases) std::cout << "    FAIL: rint(2.5) is not 2.0. Result: " << double(rint(half2)) << "\n";
				++failures;
			}

			// Verify dynamic rounding mode integration on rint
			// Toward Positive: positive inexact halfway rounds up!
			efloat_rounding_mode = RoundingMode::RoundTowardPositive;
			{
				efloat<4> half(1.5);
				if (rint(half) != 2.0) {
					if (reportTestCases) std::cout << "    FAIL: rint(1.5) under RoundTowardPositive did not round up to 2.0\n";
					++failures;
				}
				efloat<4> small(0.25);
				if (rint(small) != 1.0) {
					if (reportTestCases) std::cout << "    FAIL: rint(0.25) under RoundTowardPositive did not round up to 1.0\n";
					++failures;
				}
			}
			// Toward Negative: positive inexact halfway truncates!
			efloat_rounding_mode = RoundingMode::RoundTowardNegative;
			{
				efloat<4> half(1.5);
				if (rint(half) != 1.0) {
					if (reportTestCases) std::cout << "    FAIL: rint(1.5) under RoundTowardNegative did not truncate to 1.0. Result: " << double(rint(half)) << "\n";
					++failures;
				}
				efloat<4> small(-0.25);
				if (rint(small) != -1.0) {
					if (reportTestCases) std::cout << "    FAIL: rint(-0.25) under RoundTowardNegative did not round down to -1.0\n";
					++failures;
				}
			}
			// Restore default rounding
			efloat_rounding_mode = RoundingMode::RoundToNearest;
		}

		// ---------------------------------------------------------------------
		// 6. nearbyint Validation (Same as rint but suppresses Inexact)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying nearbyint...\n";
		{
			clear_efloat_exceptions();
			efloat<4> half(2.5); // test halfway-to-even: should yield exactly 2.0
			efloat<4> res_near = nearbyint(half);
			if (res_near != 2.0) {
				if (reportTestCases) std::cout << "    FAIL: nearbyint(2.5) did not return nearest-even 2.0. Result: " << double(res_near) << "\n";
				++failures;
			}
			if (has_efloat_exception(ExceptionFlag::Inexact)) {
				if (reportTestCases) std::cout << "    FAIL: nearbyint erroneously raised Inexact\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 7. lrint & llrint Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying lrint and llrint...\n";
		{
			efloat<4> val(1.5);
			if (lrint(val) != 2L) {
				if (reportTestCases) std::cout << "    FAIL: lrint(1.5) is not 2L\n";
				++failures;
			}
			if (llrint(val) != 2LL) {
				if (reportTestCases) std::cout << "    FAIL: llrint(1.5) is not 2LL\n";
				++failures;
			}

			// Verify high-precision exact integers above 2^53 do not get lost in lrint (CodeRabbit feedback)
			efloat<4> large(9007199254740993ULL); // 2^53 + 1
			if (llrint(large) != 9007199254740993LL) {
				if (reportTestCases) std::cout << "    FAIL: llrint(2^53 + 1) suffered from double-cast truncation. Result: " << llrint(large) << "\n";
				++failures;
			}
		}

		return failures;
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

	std::string test_suite          = "efloat mathematical truncation library";
	std::string test_tag            = "truncate";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatTruncation(reportTestCases), "efloat", "truncate manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatTruncation(reportTestCases), "efloat", "truncate foundational");
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
