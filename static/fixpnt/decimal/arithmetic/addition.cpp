// addition.cpp: test suite runner for dfixpnt addition tests
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

	std::string test_suite  = "dfixpnt addition tests";
	std::string test_tag    = "dfixpnt addition";
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

		// same-sign addition
		{
			Dfp a, b, c;
			a.assign("1.500");
			b.assign("2.500");
			c = a + b;
			if (c.to_string() != "4.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "1.500 + 2.500 = 4.000", test_tag);
			}
		}

		// different-sign addition (positive result)
		{
			Dfp a(5), b(-3);
			Dfp c = a + b;
			if (c.to_string() != "2.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "5 + (-3) = 2", test_tag);
			}
		}

		// different-sign addition (negative result)
		{
			Dfp a(3), b(-5);
			Dfp c = a + b;
			if (c.to_string() != "-2.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "3 + (-5) = -2", test_tag);
			}
		}

		// addition to zero
		{
			Dfp a(7), b(-7);
			Dfp c = a + b;
			if (!c.iszero()) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "7 + (-7) = 0", test_tag);
			}
		}

		// fractional addition
		{
			Dfp a, b;
			a.assign("0.125");
			b.assign("0.875");
			Dfp c = a + b;
			if (c.to_string() != "1.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "0.125 + 0.875 = 1.000", test_tag);
			}
		}

		// negative addition
		{
			Dfp a(-10), b(-20);
			Dfp c = a + b;
			if (c.to_string() != "-30.000") {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "-10 + (-20) = -30", test_tag);
			}
		}

		// increment/decrement
		{
			Dfp a(5);
			++a;
			if (static_cast<int>(a) != 6) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "++5 = 6", test_tag);
			}
			--a;
			if (static_cast<int>(a) != 5) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportTestResult(1, "--6 = 5", test_tag);
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
