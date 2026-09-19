// coexistence.cpp: ereal and the floatcascade types in one translation unit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// expansion_ops.hpp (ereal's arithmetic) and floatcascade.hpp (dd_cascade, td_cascade,
// qd_cascade) both declare their error-free transformations in
// sw::universal::expansion_ops. While both were plain functions taking double, any
// program that used ereal next to a cascade type failed to compile -- two_sum, fast_two_sum
// and two_prod were redefined. expansion_ops.hpp's are templates on the limb type now
// (#1563), and the two sets coexist: for double arguments the non-template cascade
// overloads are chosen, and they compute the same error-free results.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal and the cascade types in one translation unit";
	std::string test_tag    = "coexistence";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	{
		// 1/3 in each type: all three carry far more than a double's 16 digits, and all
		// three round to the same double
		ereal<8>   e = ereal<8>(1.0) / ereal<8>(3.0);
		dd_cascade d = dd_cascade(1.0) / dd_cascade(3.0);
		qd_cascade q = qd_cascade(1.0) / qd_cascade(3.0);
		int fails = 0;
		if (double(e) != 1.0 / 3.0 || double(d) != 1.0 / 3.0 || double(q) != 1.0 / 3.0) ++fails;
		// and each has a non-zero second component: the extra precision is really there
		if (e.limbs().size() < 2 || d[1] == 0.0 || q[1] == 0.0) ++fails;
		if (fails && reportTestCases) std::cout << "    FAIL 1/3: ereal " << double(e) << ", dd_cascade " << double(d) << ", qd_cascade " << double(q) << '\n';
		nrOfFailedTestCases += ReportTestResult(fails, test_tag, "ereal, dd_cascade, qd_cascade together");
	}
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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
