// numerics.cpp: regression tests for efloat numeric support functions
//               (frexp, ldexp, scalbn, logb, ilogb, fma).
//
// ldexp/scalbn only move efloat's binary exponent (exact); frexp splits into
// m * 2^e with m in [0.5,1); logb/ilogb return the radix-2 exponent; fma computes
// x*y+z with no intermediate rounding. (Issues #1164, #1166)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <climits>
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

	// ---------------------------------------------------------------------
	// 5. scalbn, logb, ilogb, fma
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying scalbn/logb/ilogb/fma...\n";
	{
		for (double v : {1.0, 3.0, -2.5, 0.75, 100.0}) {
			for (int n : {-5, 0, 3, 20}) {
				if (double(scalbn(E(v), n)) != std::scalbn(v, n)) {
					if (reportTestCases)
						std::cout << "    FAIL: scalbn(" << v << "," << n << ")\n";
					++failures;
				}
			}
			if (double(logb(E(v))) != std::logb(v) || ilogb(E(v)) != std::ilogb(v)) {
				if (reportTestCases)
					std::cout << "    FAIL: logb/ilogb(" << v << ") got " << double(logb(E(v))) << "/" << ilogb(E(v))
					          << "\n";
				++failures;
			}
		}
		// fma is EXACT (no intermediate rounding): (2^30+1)*(2^30-1) = 2^60 - 1,
		// representable in high-precision efloat but NOT in double (rounds to 2^60).
		// Build the expected value independently of the product (scalbn/subtract),
		// so the check is not a tautology against x*y+z.
		EH x30(1073741825.0), y30(1073741823.0);  // 2^30 + 1, 2^30 - 1
		EH pm1 = scalbn(EH(1.0), 60) - EH(1.0);   // 2^60 - 1
		if (fma(x30, y30, EH(0.0)) != pm1) {
			if (reportTestCases)
				std::cout << "    FAIL: fma not exact (2^60-1)\n";
			++failures;
		}
		if (fma(x30, y30, EH(2.0)) != pm1 + EH(2.0)) {  // z folded in exactly
			if (reportTestCases)
				std::cout << "    FAIL: fma z addend\n";
			++failures;
		}
		// ilogb special values
		E z(0.0), pinf;
		pinf.setinf(false);
		E nan;
		nan.setnan();
		if (ilogb(z) != FP_ILOGB0 || ilogb(pinf) != INT_MAX || ilogb(nan) != FP_ILOGBNAN) {
			if (reportTestCases)
				std::cout << "    FAIL: ilogb special values\n";
			++failures;
		}
		if (!logb(z).isinf() || logb(z).sign() != -1) {
			if (reportTestCases)
				std::cout << "    FAIL: logb(0) != -inf\n";
			++failures;
		}
		if (!logb(nan).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: logb(nan)\n";
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
