// sqrt_precision.cpp: regression tests for efloat sqrt()/cbrt() accuracy scaling.
//
// Guards issue #1140: sqrt/cbrt used a fixed 7 Newton iterations from a ~1-bit
// exponent-only seed, freezing accuracy at ~323 bits (~97 digits) regardless of
// get_precision(). The iteration count is now precision-adaptive, so accuracy
// must scale with the working precision.
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

	// |a - b| binary scale (very negative == agreement to many bits; -1e6 == exact)
	template<unsigned nlimbs>
	int64_t err_scale(sw::universal::efloat<nlimbs> a, const sw::universal::efloat<nlimbs>& b) {
		a = a - b; a.setsign(false);
		return a.iszero() ? -1000000 : a.scale();
	}

	int VerifyEfloatSqrtPrecision(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. Basic values
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying basic sqrt/cbrt/hypot values...\n";
		{
			if (std::abs(double(sqrt(efloat<8>(4.0)))  - 2.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: sqrt(4)!=2\n"; ++failures; }
			if (std::abs(double(sqrt(efloat<8>(2.0)))  - std::sqrt(2.0)) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: sqrt(2)\n"; ++failures; }
			if (std::abs(double(cbrt(efloat<8>(27.0))) - 3.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: cbrt(27)!=3\n"; ++failures; }
			if (std::abs(double(cbrt(efloat<8>(-8.0))) + 2.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: cbrt(-8)!=-2\n"; ++failures; }
			if (std::abs(double(hypot(efloat<8>(3.0), efloat<8>(4.0))) - 5.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: hypot(3,4)!=5\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 2. Accuracy scales with precision (the #1140 guard).
		//    At capacity B bits, |sqrt(2)^2 - 2| must fall well below 2^-300
		//    (the old cap) -- we require agreement to at least B-8 bits.
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying sqrt accuracy scales with precision...\n";
		{
			// efloat<16> -> 512-bit capacity; require > 300-bit agreement (old cap was ~323)
			{
				efloat<16> two(2.0);
				efloat<16> r = sqrt(two);
				int64_t sc = err_scale<16>(r * r, two);
				if (sc > -500) { if (reportTestCases) std::cout << "    FAIL: sqrt@512 only " << -sc << " bits (want > 500)\n"; ++failures; }
			}
			// efloat<64> -> 2048-bit capacity; must far exceed the ~323-bit old cap
			{
				efloat<64> two(2.0);
				efloat<64> r = sqrt(two);
				int64_t sc = err_scale<64>(r * r, two);
				if (sc > -2000) { if (reportTestCases) std::cout << "    FAIL: sqrt@2048 only " << -sc << " bits (want > 2000; old cap ~323)\n"; ++failures; }
			}
			// cbrt at 2048-bit capacity
			{
				efloat<64> two(2.0);
				efloat<64> r = cbrt(two);
				int64_t sc = err_scale<64>(r * r * r, two);
				if (sc > -2000) { if (reportTestCases) std::cout << "    FAIL: cbrt@2048 only " << -sc << " bits (want > 2000)\n"; ++failures; }
			}
		}

		// ---------------------------------------------------------------------
		// 3. Special values
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying sqrt special values...\n";
		{
			if (!sqrt(efloat<8>(0.0)).iszero())      { if (reportTestCases) std::cout << "    FAIL: sqrt(0)\n"; ++failures; }
			if (!sqrt(efloat<8>(-1.0)).isnan())      { if (reportTestCases) std::cout << "    FAIL: sqrt(-1) not NaN\n"; ++failures; }
			efloat<8> inf; inf.setinf(false);
			if (!sqrt(inf).isinf())                  { if (reportTestCases) std::cout << "    FAIL: sqrt(inf)\n"; ++failures; }
			efloat<8> nan; nan.setnan();
			if (!sqrt(nan).isnan())                  { if (reportTestCases) std::cout << "    FAIL: sqrt(nan)\n"; ++failures; }
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

	std::string test_suite          = "efloat sqrt precision library";
	std::string test_tag            = "sqrt-precision";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatSqrtPrecision(reportTestCases), "efloat", "sqrt precision");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
