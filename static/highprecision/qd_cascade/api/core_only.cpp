// core_only.cpp: the qd_cascade arithmetic core, with no other qd_cascade header included
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// #1334 split qd_cascade into core.hpp / manipulators.hpp / iostream.hpp, and a compute kernel is
// meant to be able to include core.hpp alone. For a while it could include it but not
// LINK against it: ulp() in qd_cascade_impl.hpp calls ldexp(qd_cascade, int), which the impl declared
// and only the mathlib defined (#1462). -fsyntax-only cannot see that, and every other
// test includes the qd_cascade.hpp umbrella, which does bring the definition. This one does not:
// it builds a real executable from core.hpp, so the link step is the check.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>
// the only qd_cascade header in this translation unit
#include <universal/number/qd_cascade/core.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// ulp, frexp, ldexp and copysign are the core's own: each must be defined, not just declared
inline int VerifyCoreFunctionsLink(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
		}
	};

	// ulp(a) = 2^(scale(a) - 159): the weight of the last bit of the fourth limb
	check(double(ulp(qd_cascade(1.0))) == std::ldexp(1.0, -159), "ulp(1)");
	check(double(ulp(qd_cascade(1024.0))) == std::ldexp(1.0, 10 - 159), "ulp(1024)");

	int e = 0;
	qd_cascade f = frexp(qd_cascade(12.0), &e);
	check(double(f) == 0.75 && e == 4, "frexp(12) = 0.75 * 2^4");
	check(ldexp(qd_cascade(0.75), 4) == qd_cascade(12.0), "ldexp(0.75, 4) = 12");
	check(copysign(qd_cascade(3.0), qd_cascade(-1.0)) == qd_cascade(-3.0), "copysign(3, -1) = -3");
	return nrOfFailedTests;
}

}} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "qd_cascade core.hpp without the umbrella (#1462)";
	std::string test_tag    = "core only";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyCoreFunctionsLink(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCoreFunctionsLink(reportTestCases), "ulp/frexp/ldexp/copysign", test_tag);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
