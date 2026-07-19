// numerics.cpp: regression tests for efloat numeric support functions (frexp, ldexp).
//
// ldexp(x, n) = x * 2^n and frexp(x, &e) split x into m * 2^e with m in [0.5,1).
// Both only move efloat's binary exponent, so they are exact at any precision.
// (Issue #1164)
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

int VerifyEfloatNumerics(bool reportTestCases) {
	using namespace sw::universal;
	using E      = efloat<8>;   // 256-bit
	using EH     = efloat<16>;  // 512-bit
	int failures = 0;

	// ---------------------------------------------------------------------
	// 1. ldexp: x * 2^n against std for double-range values
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying ldexp...\n";
	{
		for (double v : {1.0, 3.0, 0.75, -2.5, 100.0}) {
			for (int n : {-10, -1, 0, 1, 5, 30}) {
				if (double(ldexp(E(v), n)) != std::ldexp(v, n)) {
					if (reportTestCases)
						std::cout << "    FAIL: ldexp(" << v << "," << n << ")\n";
					++failures;
				}
			}
		}
		// exactness at high precision: ldexp only shifts the exponent, no rounding
		EH x(1.0);
		x = x + ldexp(EH(1.0), -400);  // 1 + 2^-400, representable at 512-bit
		EH one(1.0);
		EH d = x - one;
		d.setsign(false);
		if (d.iszero() || d.scale() != -400) {
			if (reportTestCases)
				std::cout << "    FAIL: ldexp exactness, scale=" << (d.iszero() ? 0 : d.scale()) << "\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 2. frexp: mantissa in [0.5,1) and x == m * 2^e
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying frexp...\n";
	{
		for (double v : {1.0, 3.0, 0.75, -2.5, 100.0, 0.1}) {
			int    e  = 0;
			E      m  = frexp(E(v), &e);
			double md = std::abs(double(m));
			if (!(md >= 0.5 && md < 1.0)) {
				if (reportTestCases)
					std::cout << "    FAIL: frexp(" << v << ") mantissa " << double(m) << " not in [0.5,1)\n";
				++failures;
			}
			// matches std::frexp
			int    se = 0;
			double sm = std::frexp(v, &se);
			if (e != se || double(m) != sm) {
				if (reportTestCases)
					std::cout << "    FAIL: frexp(" << v << ") efloat(m=" << double(m) << ",e=" << e << ") std(m=" << sm
					          << ",e=" << se << ")\n";
				++failures;
			}
		}
	}

	// ---------------------------------------------------------------------
	// 3. round-trip: ldexp(frexp(x)) == x, exactly, at full precision
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying ldexp(frexp(x)) == x...\n";
	{
		for (double v : {1.0, 3.0, 0.75, -2.5, 100.0, 6.022e23, -1e-9}) {
			int e = 0;
			EH  m = frexp(EH(v), &e);
			if (ldexp(m, e) != EH(v)) {
				if (reportTestCases)
					std::cout << "    FAIL: ldexp(frexp(" << v << ")) != " << v << "\n";
				++failures;
			}
		}
		// a beyond-double value stays exact through the round-trip
		EH x(1.0);
		x     = x + ldexp(EH(1.0), -300);  // 1 + 2^-300
		int e = 0;
		EH  m = frexp(x, &e);
		if (ldexp(m, e) != x) {
			if (reportTestCases)
				std::cout << "    FAIL: high-precision frexp/ldexp round-trip\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 4. special values
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying special values...\n";
	{
		int e = 99;
		E   z = frexp(E(0.0), &e);
		if (!z.iszero() || e != 0) {
			if (reportTestCases)
				std::cout << "    FAIL: frexp(0) e=" << e << "\n";
			++failures;
		}

		E pinf;
		pinf.setinf(false);
		E nan;
		nan.setnan();
		e = 99;
		if (!frexp(pinf, &e).isinf() || e != 0) {
			if (reportTestCases)
				std::cout << "    FAIL: frexp(inf)\n";
			++failures;
		}
		if (!frexp(nan, &e).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: frexp(nan)\n";
			++failures;
		}

		if (!ldexp(E(0.0), 5).iszero()) {
			if (reportTestCases)
				std::cout << "    FAIL: ldexp(0,5)\n";
			++failures;
		}
		if (!ldexp(pinf, 5).isinf()) {
			if (reportTestCases)
				std::cout << "    FAIL: ldexp(inf,5)\n";
			++failures;
		}
		if (!ldexp(nan, 5).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: ldexp(nan,5)\n";
			++failures;
		}
		if (ldexp(E(3.5), 0) != E(3.5)) {
			if (reportTestCases)
				std::cout << "    FAIL: ldexp(x,0) != x\n";
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

	std::string test_suite          = "efloat numeric support library";
	std::string test_tag            = "numerics";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatNumerics(reportTestCases), "efloat", "numerics");
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
