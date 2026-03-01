// multiplication.cpp: test suite runner for dfixpnt multiplication tests
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

	std::string test_suite  = "dfixpnt multiplication tests";
	std::string test_tag    = "dfixpnt multiplication";
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

		// integer multiplication
		{
			Dfp a(6), b(7);
			Dfp c = a * b;
			if (c.to_string() != "42.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "6 * 7 = 42", test_tag);
			}
		}

		// multiplication by zero
		{
			Dfp a(123), b(0);
			Dfp c = a * b;
			if (!c.iszero()) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "123 * 0 = 0", test_tag);
			}
		}

		// multiplication by one
		{
			Dfp a;
			a.assign("12.345");
			Dfp b(1);
			Dfp c = a * b;
			if (c.to_string() != "12.345") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "12.345 * 1 = 12.345", test_tag);
			}
		}

		// sign handling
		{
			Dfp a(3), b(-4);
			Dfp c = a * b;
			if (c.to_string() != "-12.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "3 * (-4) = -12", test_tag);
			}
		}
		{
			Dfp a(-3), b(-4);
			Dfp c = a * b;
			if (c.to_string() != "12.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "(-3) * (-4) = 12", test_tag);
			}
		}

		// fractional multiplication
		{
			Dfp a, b;
			a.assign("2.500");
			b.assign("4.000");
			Dfp c = a * b;
			if (c.to_string() != "10.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "2.500 * 4.000 = 10.000", test_tag);
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
