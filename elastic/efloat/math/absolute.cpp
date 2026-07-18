// absolute.cpp: regression tests for efloat abs() / fabs().
//
// Guards against the prior stub that returned its argument unchanged (so
// abs(-x) == -x). Verifies sign clearing across normal, zero, inf, and NaN.
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

	int VerifyEfloatAbsolute(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. Normal values
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying abs() on normal values...\n";
		{
			efloat<8> neg(-3.25);
			if (abs(neg) != 3.25 || abs(neg).isneg())  { if (reportTestCases) std::cout << "    FAIL: abs(-3.25) != 3.25\n"; ++failures; }

			efloat<8> pos(3.25);
			if (abs(pos) != 3.25 || abs(pos).isneg())  { if (reportTestCases) std::cout << "    FAIL: abs(3.25) != 3.25\n"; ++failures; }

			// abs must NOT return the argument unchanged for negatives (the old-stub bug)
			if (abs(neg) == neg)                       { if (reportTestCases) std::cout << "    FAIL: abs(-3.25) returned the argument unchanged\n"; ++failures; }

			// fabs is the same as abs
			if (fabs(neg) != abs(neg))                 { if (reportTestCases) std::cout << "    FAIL: fabs(-3.25) != abs(-3.25)\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 2. Zero
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying abs() on zero...\n";
		{
			efloat<4> zero(0.0);
			if (abs(zero) != 0.0 || abs(zero).isneg())  { if (reportTestCases) std::cout << "    FAIL: abs(0) != +0\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 3. Infinity: abs(-inf) == +inf
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying abs() on infinity...\n";
		{
			efloat<4> neg_inf; neg_inf.setinf(true);
			efloat<4> r = abs(neg_inf);
			if (!r.isinf() || r.isneg())                { if (reportTestCases) std::cout << "    FAIL: abs(-inf) != +inf\n"; ++failures; }

			efloat<4> pos_inf; pos_inf.setinf(false);
			efloat<4> rp = abs(pos_inf);
			if (!rp.isinf() || rp.isneg())              { if (reportTestCases) std::cout << "    FAIL: abs(+inf) != +inf\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 4. NaN stays NaN
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying abs() on NaN...\n";
		{
			efloat<4> nan; nan.setnan();
			if (!abs(nan).isnan())                      { if (reportTestCases) std::cout << "    FAIL: abs(nan) not NaN\n"; ++failures; }
			if (!fabs(nan).isnan())                     { if (reportTestCases) std::cout << "    FAIL: fabs(nan) not NaN\n"; ++failures; }
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

	std::string test_suite          = "efloat absolute value library";
	std::string test_tag            = "absolute";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatAbsolute(reportTestCases), "efloat", "absolute manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatAbsolute(reportTestCases), "efloat", "absolute foundational");
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
