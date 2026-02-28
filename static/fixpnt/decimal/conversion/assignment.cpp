// assignment.cpp: test suite runner for dfixpnt conversion/assignment tests
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

	std::string test_suite  = "dfixpnt assignment tests";
	std::string test_tag    = "dfixpnt assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	{
		using Dfp = dfixpnt<8, 2>;

		// integer assignment
		{
			Dfp a;
			a = 42;
			if (static_cast<int>(a) != 42) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "int assign 42", test_tag);
			}
		}

		// negative integer
		{
			Dfp a;
			a = -15;
			if (static_cast<int>(a) != -15) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "int assign -15", test_tag);
			}
		}

		// zero
		{
			Dfp a;
			a = 0;
			if (!a.iszero()) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "int assign 0", test_tag);
			}
		}

		// double assignment
		{
			Dfp a;
			a = 1.25;
			if (a.to_string() != "1.25") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "double assign 1.25", test_tag);
			}
		}

		// double to int conversion
		{
			Dfp a(99.50);
			int v = static_cast<int>(a);
			if (v != 99) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "double->int truncation 99.50", test_tag);
			}
		}

		// float assignment
		{
			Dfp a;
			a = 3.14f;
			// float 3.14 rounds to 3.14 with 2 fraction digits
			if (a.to_string() != "3.14") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "float assign 3.14", test_tag);
			}
		}

		// string assign
		{
			Dfp a;
			a.assign("-123.45");
			if (a.to_string() != "-123.45") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "string assign -123.45", test_tag);
			}
		}

		// unsigned assignment
		{
			Dfp a;
			a = 255u;
			if (static_cast<int>(a) != 255) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "unsigned assign 255", test_tag);
			}
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
