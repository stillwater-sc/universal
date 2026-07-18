// trigonometry.cpp: regression tests for efloat trigonometric functions.
//
// Categories tested:
//   - sin, cos, tan
//   - asin, acos, atan, atan2
//   - argument reduction, identities, special values (Issue #1115)
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

	// Helper to check if two values are close
	template<unsigned nlimbs>
	bool IsClose(const sw::universal::efloat<nlimbs>& a, double target_val, double tolerance = 1e-12) {
		double diff = std::abs(double(a) - target_val);
		return diff <= tolerance;
	}

	int VerifyEfloatTrigonometry(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		const double PI = std::acos(-1.0);

		// ---------------------------------------------------------------------
		// 1. sine (sin)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying sine (sin)...\n";
		{
			clear_efloat_exceptions();

			if (sin(efloat<8>(0.0)) != 0.0)                           { if (reportTestCases) std::cout << "    FAIL: sin(0) != 0\n"; ++failures; }
			if (!IsClose(sin(efloat<8>(PI / 2.0)),  1.0))             { if (reportTestCases) std::cout << "    FAIL: sin(pi/2) != 1\n"; ++failures; }
			if (!IsClose(sin(efloat<8>(PI / 6.0)),  0.5))             { if (reportTestCases) std::cout << "    FAIL: sin(pi/6) != 0.5\n"; ++failures; }
			if (!IsClose(sin(efloat<8>(1.0)), std::sin(1.0)))         { if (reportTestCases) std::cout << "    FAIL: sin(1) inaccurate\n"; ++failures; }
			if (!IsClose(sin(efloat<8>(-1.0)), std::sin(-1.0)))       { if (reportTestCases) std::cout << "    FAIL: sin(-1) inaccurate\n"; ++failures; }
			// argument reduction: sin(10*pi + pi/6) == sin(pi/6) == 0.5
			if (!IsClose(sin(efloat<8>(10.0 * PI + PI / 6.0)), 0.5, 1e-10)) { if (reportTestCases) std::cout << "    FAIL: sin argument reduction\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 2. cosine (cos)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying cosine (cos)...\n";
		{
			if (cos(efloat<8>(0.0)) != 1.0)                          { if (reportTestCases) std::cout << "    FAIL: cos(0) != 1\n"; ++failures; }
			if (!IsClose(cos(efloat<8>(PI / 3.0)), 0.5))             { if (reportTestCases) std::cout << "    FAIL: cos(pi/3) != 0.5\n"; ++failures; }
			if (!IsClose(cos(efloat<8>(PI)), -1.0))                  { if (reportTestCases) std::cout << "    FAIL: cos(pi) != -1\n"; ++failures; }
			if (!IsClose(cos(efloat<8>(2.0)), std::cos(2.0)))        { if (reportTestCases) std::cout << "    FAIL: cos(2) inaccurate\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 3. Pythagorean identity sin^2 + cos^2 == 1 (internal consistency)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying sin^2 + cos^2 == 1...\n";
		{
			efloat<8> x(0.7);
			efloat<8> s = sin(x), c = cos(x);
			efloat<8> id = s * s + c * c;
			if (!IsClose(id, 1.0, 1e-14))                            { if (reportTestCases) std::cout << "    FAIL: sin^2+cos^2 != 1, got " << double(id) << "\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 4. tangent (tan)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying tangent (tan)...\n";
		{
			if (tan(efloat<8>(0.0)) != 0.0)                          { if (reportTestCases) std::cout << "    FAIL: tan(0) != 0\n"; ++failures; }
			if (!IsClose(tan(efloat<8>(PI / 4.0)), 1.0))             { if (reportTestCases) std::cout << "    FAIL: tan(pi/4) != 1\n"; ++failures; }
			if (!IsClose(tan(efloat<8>(0.5)), std::tan(0.5)))        { if (reportTestCases) std::cout << "    FAIL: tan(0.5) inaccurate\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 5. arcsine (asin)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying arcsine (asin)...\n";
		{
			if (asin(efloat<8>(0.0)) != 0.0)                         { if (reportTestCases) std::cout << "    FAIL: asin(0) != 0\n"; ++failures; }
			if (!IsClose(asin(efloat<8>(1.0)), PI / 2.0))            { if (reportTestCases) std::cout << "    FAIL: asin(1) != pi/2\n"; ++failures; }
			if (!IsClose(asin(efloat<8>(0.5)), PI / 6.0))            { if (reportTestCases) std::cout << "    FAIL: asin(0.5) != pi/6\n"; ++failures; }
			if (!IsClose(asin(efloat<8>(-0.5)), -PI / 6.0))          { if (reportTestCases) std::cout << "    FAIL: asin(-0.5) != -pi/6\n"; ++failures; }
			if (!IsClose(asin(efloat<8>(0.9)), std::asin(0.9), 1e-10)) { if (reportTestCases) std::cout << "    FAIL: asin(0.9) inaccurate (reduction)\n"; ++failures; }
			// domain: |x| > 1 -> NaN
			if (!asin(efloat<8>(2.0)).isnan())                      { if (reportTestCases) std::cout << "    FAIL: asin(2) not NaN\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 6. arccosine (acos)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying arccosine (acos)...\n";
		{
			if (!IsClose(acos(efloat<8>(1.0)), 0.0))                 { if (reportTestCases) std::cout << "    FAIL: acos(1) != 0\n"; ++failures; }
			if (!IsClose(acos(efloat<8>(0.0)), PI / 2.0))            { if (reportTestCases) std::cout << "    FAIL: acos(0) != pi/2\n"; ++failures; }
			if (!IsClose(acos(efloat<8>(-1.0)), PI))                 { if (reportTestCases) std::cout << "    FAIL: acos(-1) != pi\n"; ++failures; }
			if (!acos(efloat<8>(2.0)).isnan())                      { if (reportTestCases) std::cout << "    FAIL: acos(2) not NaN\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 7. arctangent (atan)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying arctangent (atan)...\n";
		{
			if (atan(efloat<8>(0.0)) != 0.0)                         { if (reportTestCases) std::cout << "    FAIL: atan(0) != 0\n"; ++failures; }
			if (!IsClose(atan(efloat<8>(1.0)), PI / 4.0))            { if (reportTestCases) std::cout << "    FAIL: atan(1) != pi/4\n"; ++failures; }
			if (!IsClose(atan(efloat<8>(-1.0)), -PI / 4.0))          { if (reportTestCases) std::cout << "    FAIL: atan(-1) != -pi/4\n"; ++failures; }
			if (!IsClose(atan(efloat<8>(3.0)), std::atan(3.0)))      { if (reportTestCases) std::cout << "    FAIL: atan(3) inaccurate (|x|>1)\n"; ++failures; }
			if (!IsClose(atan(efloat<8>(0.75)), std::atan(0.75)))    { if (reportTestCases) std::cout << "    FAIL: atan(0.75) inaccurate (addition formula)\n"; ++failures; }
			// atan(+/-inf) == +/- pi/2
			efloat<8> pinf; pinf.setinf(false);
			efloat<8> ninf; ninf.setinf(true);
			if (!IsClose(atan(pinf),  PI / 2.0))                    { if (reportTestCases) std::cout << "    FAIL: atan(+inf) != pi/2\n"; ++failures; }
			if (!IsClose(atan(ninf), -PI / 2.0))                    { if (reportTestCases) std::cout << "    FAIL: atan(-inf) != -pi/2\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 8. atan2 (all quadrants)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying atan2 (quadrants)...\n";
		{
			if (!IsClose(atan2(efloat<8>(1.0),  efloat<8>(1.0)),  PI / 4.0))         { if (reportTestCases) std::cout << "    FAIL: atan2(1,1)\n"; ++failures; }   // QI
			if (!IsClose(atan2(efloat<8>(1.0),  efloat<8>(-1.0)), 3.0 * PI / 4.0))   { if (reportTestCases) std::cout << "    FAIL: atan2(1,-1)\n"; ++failures; }  // QII
			if (!IsClose(atan2(efloat<8>(-1.0), efloat<8>(-1.0)), -3.0 * PI / 4.0))  { if (reportTestCases) std::cout << "    FAIL: atan2(-1,-1)\n"; ++failures; } // QIII
			if (!IsClose(atan2(efloat<8>(-1.0), efloat<8>(1.0)),  -PI / 4.0))        { if (reportTestCases) std::cout << "    FAIL: atan2(-1,1)\n"; ++failures; }  // QIV
			if (!IsClose(atan2(efloat<8>(1.0),  efloat<8>(0.0)),  PI / 2.0))         { if (reportTestCases) std::cout << "    FAIL: atan2(1,0)\n"; ++failures; }
			if (!IsClose(atan2(efloat<8>(0.0),  efloat<8>(-1.0)), PI))               { if (reportTestCases) std::cout << "    FAIL: atan2(0,-1)\n"; ++failures; }
			// both arguments infinite -> diagonal directions (+/- pi/4, +/- 3pi/4)
			{
				efloat<8> pinf; pinf.setinf(false);
				efloat<8> ninf; ninf.setinf(true);
				if (!IsClose(atan2(pinf, pinf),        PI / 4.0))       { if (reportTestCases) std::cout << "    FAIL: atan2(+inf,+inf) != pi/4\n"; ++failures; }
				if (!IsClose(atan2(pinf, ninf),  3.0 * PI / 4.0))       { if (reportTestCases) std::cout << "    FAIL: atan2(+inf,-inf) != 3pi/4\n"; ++failures; }
				if (!IsClose(atan2(ninf, pinf),       -PI / 4.0))       { if (reportTestCases) std::cout << "    FAIL: atan2(-inf,+inf) != -pi/4\n"; ++failures; }
				if (!IsClose(atan2(ninf, ninf), -3.0 * PI / 4.0))       { if (reportTestCases) std::cout << "    FAIL: atan2(-inf,-inf) != -3pi/4\n"; ++failures; }
				// single infinite argument (negative operands exercise the sign()-vs-isneg() fix)
				if (!IsClose(atan2(efloat<8>(1.0), ninf),  PI))          { if (reportTestCases) std::cout << "    FAIL: atan2(1,-inf) != pi\n"; ++failures; }
				if (!IsClose(atan2(ninf, efloat<8>(1.0)), -PI / 2.0))    { if (reportTestCases) std::cout << "    FAIL: atan2(-inf,1) != -pi/2\n"; ++failures; }
			}
		}

		// ---------------------------------------------------------------------
		// 9. round-trip: asin(sin(x)) == x, atan(tan(x)) == x  (x in principal range)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying inverse round-trips...\n";
		{
			efloat<8> x(0.3);
			if (!IsClose(asin(sin(x)), 0.3, 1e-12))                 { if (reportTestCases) std::cout << "    FAIL: asin(sin(0.3)) != 0.3\n"; ++failures; }
			if (!IsClose(atan(tan(x)), 0.3, 1e-12))                 { if (reportTestCases) std::cout << "    FAIL: atan(tan(0.3)) != 0.3\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 10. special values (NaN / Inf propagation)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying special values...\n";
		{
			efloat<4> nan; nan.setnan();
			if (!sin(nan).isnan())  { if (reportTestCases) std::cout << "    FAIL: sin(nan) not NaN\n"; ++failures; }
			if (!cos(nan).isnan())  { if (reportTestCases) std::cout << "    FAIL: cos(nan) not NaN\n"; ++failures; }
			if (!atan(nan).isnan()) { if (reportTestCases) std::cout << "    FAIL: atan(nan) not NaN\n"; ++failures; }

			efloat<4> inf; inf.setinf(false);
			if (!sin(inf).isnan())  { if (reportTestCases) std::cout << "    FAIL: sin(+inf) not NaN\n"; ++failures; }
			if (!cos(inf).isnan())  { if (reportTestCases) std::cout << "    FAIL: cos(+inf) not NaN\n"; ++failures; }
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

	std::string test_suite          = "efloat mathematical trigonometry library";
	std::string test_tag            = "trigonometry";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatTrigonometry(reportTestCases), "efloat", "trigonometry manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatTrigonometry(reportTestCases), "efloat", "trigonometry foundational");
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
