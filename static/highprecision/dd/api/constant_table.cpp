// constant_table.cpp: the dd constant table, checked against dd itself (#1849)
//
//   generated   every entry is dd's own parse of its ~340-digit reference
//               (math/constants/reference_constants.hpp), component for component
//   nearest     every entry is the nearest expansion of its reference: each component the double
//               nearest what the ones before it leave, certified by efloat at 1024 bits
//   computed    dd's own arithmetic and math functions -- 4 atan(1), exp(1), log(2), sqrt(5), ...
//               -- compute every constant to within 2^-(106 - 4) of it
//
// tools/generators/multidouble_constants_gen.cpp prints the table the same way, with the parse.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- every entry of the table, all three checks.
//   LEVEL 2 -- as level 1.
//   LEVEL 3 -- as level 2.
//   LEVEL 4 -- as level 3.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <universal/verification/multidouble_constant_tables.hpp>
#include <universal/verification/test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
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

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::multidouble_verification;

	std::string test_suite          = "double-double constant table";
	std::string test_tag            = "dd constants";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// 2 doubles, 106 bits; the type's own functions may lose 4 bits on a constant
	constexpr int limbs = 2;
	constexpr int slack = 4;

#if MANUAL_TESTING

	double worst = 0.0;
	nrOfFailedTestCases += ReportTestResult(VerifyConstantTable("dd", dd_constant_table(), limbs, slack, reportTestCases, &worst), test_tag, "table");
	std::cout << "worst relative error of a constant dd computes: 2^" << worst << '\n';
	PrintConstantTable("dd", dd_constant_table(), limbs);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConstantTable("dd", dd_constant_table(), limbs, slack, reportTestCases), test_tag, "table");
#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
