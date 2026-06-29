// fractional.cpp: regression tests for efloat mathematical fractional and remainder functions.
//
// Categories tested:
//   - fmod, remainder, drem, frac, modf, copysign, nextafter, nexttoward
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

	int VerifyEfloatFractional(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. fmod Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying fmod...\n";
		{
			// fmod(5.5, 2.0) == 1.5
			efloat<4> x(5.5);
			efloat<4> y(2.0);
			double d_fmod = double(fmod(x, y));
			if (std::abs(d_fmod - 1.5) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: fmod(5.5, 2.0) is not 1.5. Result: " << d_fmod << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. remainder & drem Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying remainder and drem...\n";
		{
			// remainder(5.5, 2.0) == -0.5 (nearest even of 5.5/2.0 is 3, 5.5 - 3*2 = -0.5)
			efloat<4> x(5.5);
			efloat<4> y(2.0);
			double d_rem = double(remainder(x, y));
			if (std::abs(d_rem - (-0.5)) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: remainder(5.5, 2.0) is not -0.5. Result: " << d_rem << "\n";
				++failures;
			}

			double d_drem = double(drem(x, y));
			if (std::abs(d_drem - (-0.5)) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: drem(5.5, 2.0) is not -0.5\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. frac Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying frac...\n";
		{
			// frac(5.5) == 0.5
			efloat<4> x(5.5);
			double d_frac = double(frac(x));
			if (std::abs(d_frac - 0.5) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: frac(5.5) is not 0.5. Result: " << d_frac << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. modf Decompositions
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying modf...\n";
		{
			efloat<4> x(5.75);
			efloat<4> iptr;
			double d_frac = double(modf(x, &iptr));
			if (std::abs(d_frac - 0.75) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: modf(5.75) frac part is not 0.75\n";
				++failures;
			}
			if (iptr != 5.0) {
				if (reportTestCases) std::cout << "    FAIL: modf(5.75) int part is not 5.0. Result: " << double(iptr) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 5. copysign Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying copysign...\n";
		{
			efloat<4> x(5.5);
			efloat<4> y(-2.0);
			efloat<4> res = copysign(x, y);
			if (res != -5.5) {
				if (reportTestCases) std::cout << "    FAIL: copysign(5.5, -2.0) is not -5.5\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 6. nextafter & nexttoward Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying nextafter and nexttoward...\n";
		{
			// nextafter(1.0, 2.0) > 1.0
			efloat<4> x(1.0);
			efloat<4> target_up(2.0);
			efloat<4> next_up = nextafter(x, target_up);
			if (next_up <= efloat<4>(1.0)) {
				if (reportTestCases) std::cout << "    FAIL: nextafter(1.0, 2.0) is not > 1.0. Result: " << double(next_up) << "\n";
				++failures;
			}

			// nextafter(1.0, 0.0) < 1.0
			efloat<4> target_down(0.0);
			efloat<4> next_down = nextafter(x, target_down);
			if (next_down >= efloat<4>(1.0)) {
				if (reportTestCases) std::cout << "    FAIL: nextafter(1.0, 0.0) is not < 1.0. Result: " << double(next_down) << "\n";
				++failures;
			}

			// nextafter(0.0, 1.0) > 0.0
			efloat<4> zero(0.0);
			efloat<4> next_zero_up = nextafter(zero, target_up);
			if (next_zero_up <= efloat<4>(0.0)) {
				if (reportTestCases) std::cout << "    FAIL: nextafter(0.0, 2.0) is not > 0.0. Result: " << double(next_zero_up) << "\n";
				++failures;
			}

			// nexttoward(1.0, 2.0) > 1.0
			efloat<4> next_tow = nexttoward(x, 2.0);
			if (next_tow <= efloat<4>(1.0)) {
				if (reportTestCases) std::cout << "    FAIL: nexttoward(1.0, 2.0) is not > 1.0\n";
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

	std::string test_suite          = "efloat mathematical fractional library";
	std::string test_tag            = "fractional";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatFractional(reportTestCases), "efloat", "fractional manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatFractional(reportTestCases), "efloat", "fractional foundational");
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
