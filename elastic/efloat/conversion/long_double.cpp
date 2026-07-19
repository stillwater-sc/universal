// long_double.cpp: regression tests for efloat <-> long double conversion.
//
// Before the fix, efloat's convert_ieee754 only built limbs for sizeof(Real) of
// 4 or 8, so a wide long double (sizeof 16 on x86-64) produced 0. The conversion
// now decomposes the value into an exact sum of doubles (scaled via frexp/ldexp),
// preserving the full significand and the extended exponent range in both
// directions. On platforms where long double aliases double the extended-
// precision/range checks are skipped. (Issue #1160)
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

#if LONG_DOUBLE_SUPPORT

int VerifyEfloatLongDouble(bool reportTestCases) {
	using namespace sw::universal;
	using E      = efloat<8>;  // 256-bit
	int failures = 0;

	// -----------------------------------------------------------------
	// 1. the reported bug: the constructor no longer yields 0
	// -----------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying long double constructor (no longer 0)...\n";
	{
		for (long double v : {1.0L, 2.0L, 3.5L, 100.0L, -7.25L, 0.5L}) {
			E x(v);
			if (double(x) != static_cast<double>(v)) {
				if (reportTestCases)
					std::cout << "    FAIL: efloat(" << double(v) << "L) -> " << double(x) << "\n";
				++failures;
			}
		}
	}

	// -----------------------------------------------------------------
	// 2. round-trip exactness through both directions
	// -----------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying long double round-trip...\n";
	{
		for (long double v : {1.0L, 3.5L, 0.1L, -2.75L, 3.14159265358979323846L, 100.0L}) {
			if (static_cast<long double>(E(v)) != v) {
				if (reportTestCases)
					std::cout << "    FAIL: round-trip " << double(v) << "L\n";
				++failures;
			}
		}
	}

	// -----------------------------------------------------------------
	// 3. special values
	// -----------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying special values...\n";
	{
		E negzero(-0.0L);
		if (!negzero.iszero()) {
			if (reportTestCases)
				std::cout << "    FAIL: -0.0L not zero\n";
			++failures;
		}

		long double inf = std::numeric_limits<long double>::infinity();
		E           pinf(inf), ninf(-inf);
		if (!pinf.isinf() || pinf.sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: +inf in\n";
			++failures;
		}
		if (!ninf.isinf() || ninf.sign() != -1) {
			if (reportTestCases)
				std::cout << "    FAIL: -inf in\n";
			++failures;
		}
		if (static_cast<long double>(pinf) != inf) {
			if (reportTestCases)
				std::cout << "    FAIL: +inf out\n";
			++failures;
		}

		E nan(std::numeric_limits<long double>::quiet_NaN());
		if (!nan.isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: nan in\n";
			++failures;
		}
		if (!std::isnan(static_cast<long double>(nan))) {
			if (reportTestCases)
				std::cout << "    FAIL: nan out\n";
			++failures;
		}
	}

	// -----------------------------------------------------------------
	// 4. wide long double only: precision and exponent range beyond double
	// -----------------------------------------------------------------
	if constexpr (sizeof(long double) > sizeof(double)) {
		if (reportTestCases)
			std::cout << "  Verifying beyond-double precision + range...\n";

		// significand bit below double's 53: 1 + 2^-60 must survive
		{
			E x(1.0L + std::ldexp(1.0L, -60));
			E one(1.0);
			E d = x - one;
			d.setsign(false);
			if (d.iszero() || d.scale() != -60) {
				if (reportTestCases)
					std::cout << "    FAIL: 1+2^-60 lost, scale=" << (d.iszero() ? 0 : d.scale()) << "\n";
				++failures;
			}
			if (static_cast<long double>(x) != 1.0L + std::ldexp(1.0L, -60)) {
				if (reportTestCases)
					std::cout << "    FAIL: 1+2^-60 round-trip\n";
				++failures;
			}
		}

		// exponent range beyond double (double tops out near 2^1023 / 2^-1074).
		// Use exact powers of two so the round-trip comparison is exact and to
		// dodge a cppcheck parser limitation on huge decimal long double literals.
		for (long double v : {std::ldexp(1.0L, 1100), std::ldexp(-3.0L, 1200), std::ldexp(1.0L, -1100),
		                      std::ldexp(1.0L, 16000), std::ldexp(1.0L, -16000)}) {
			E x(v);
			if (x.iszero() || x.isinf()) {
				if (reportTestCases)
					std::cout << "    FAIL: range value collapsed\n";
				++failures;
			}
			if (static_cast<long double>(x) != v) {
				if (reportTestCases)
					std::cout << "    FAIL: extended-range round-trip\n";
				++failures;
			}
		}
	}

	return failures;
}

#endif  // LONG_DOUBLE_SUPPORT

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

	std::string test_suite          = "efloat long double conversion library";
	std::string test_tag            = "long double";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if LONG_DOUBLE_SUPPORT
#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatLongDouble(reportTestCases), "efloat", "long double");
#	endif
#else
	if (reportTestCases)
		std::cout << "  long double aliases double on this platform; nothing wide to test\n";
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
