// logic.cpp: test suite runner for dfixpnt comparison/logic tests
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define DFIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/dfixpnt/dfixpnt.hpp>

#include <universal/verification/test_suite.hpp>

#define MANUAL_TESTING 0
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

	std::string test_suite  = "dfixpnt logic tests";
	std::string test_tag    = "dfixpnt logic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	{
		using Dfp = dfixpnt<8, 3>;

		Dfp a, b;

		// equality
		a.assign("1.500");
		b.assign("1.500");
		if (!(a == b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "1.500 == 1.500", test_tag);
		}

		// inequality
		a.assign("1.500");
		b.assign("2.500");
		if (!(a != b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "1.500 != 2.500", test_tag);
		}

		// less than (positive)
		a.assign("1.000");
		b.assign("2.000");
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "1.000 < 2.000", test_tag);
		}

		// greater than
		if (!(b > a)) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "2.000 > 1.000", test_tag);
		}

		// less than or equal
		a.assign("5.000");
		b.assign("5.000");
		if (!(a <= b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "5.000 <= 5.000", test_tag);
		}

		// greater than or equal
		if (!(a >= b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "5.000 >= 5.000", test_tag);
		}

		// negative comparisons
		a = -3;
		b = -1;
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "-3 < -1", test_tag);
		}

		// negative vs positive
		a = -1;
		b = 1;
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "-1 < 1", test_tag);
		}

		// +0 == -0
		Dfp pos_zero(SpecificValue::zero);
		Dfp neg_zero(SpecificValue::zero);
		neg_zero.setsign(true);
		if (pos_zero != neg_zero) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "+0 == -0", test_tag);
		}
		if (pos_zero < neg_zero || neg_zero < pos_zero) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportTestResult(1, "zero ordering", test_tag);
		}
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
