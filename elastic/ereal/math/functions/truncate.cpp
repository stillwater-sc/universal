// truncate.cpp: test suite runner for truncate functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "ereal mathlib truncate function validation";
	std::string test_tag    = "floor/ceil/trunc/round";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 1: Component-wise floor/ceil implementation
	// Note: trunc and round still use stubs (deferred to Phase 2)

	std::cout << "Phase 1: Testing truncation functions with component-wise operations\n\n";

	// Test 1: floor - positive values
	{
		std::cout << "Test 1: floor (positive values)\n";
		ereal<> x(2.7), expected(2.0);
		ereal<> result = floor(x);

		bool test1_pass = (result == expected);
		std::cout << "  floor(2.7) == 2.0: " << (test1_pass ? "PASS" : "FAIL") << "\n";
		if (!test1_pass) ++nrOfFailedTestCases;
	}

	// Test 2: floor - negative values
	{
		std::cout << "\nTest 2: floor (negative values)\n";
		ereal<> x(-2.3), expected(-3.0);
		ereal<> result = floor(x);

		bool test2_pass = (result == expected);
		std::cout << "  floor(-2.3) == -3.0: " << (test2_pass ? "PASS" : "FAIL") << "\n";
		if (!test2_pass) ++nrOfFailedTestCases;
	}

	// Test 3: floor - already integer
	{
		std::cout << "\nTest 3: floor (integer values)\n";
		ereal<> x(5.0), expected(5.0);
		ereal<> result = floor(x);

		bool test3_pass = (result == expected);
		std::cout << "  floor(5.0) == 5.0: " << (test3_pass ? "PASS" : "FAIL") << "\n";
		if (!test3_pass) ++nrOfFailedTestCases;
	}

	// Test 4: ceil - positive values
	{
		std::cout << "\nTest 4: ceil (positive values)\n";
		ereal<> x(2.3), expected(3.0);
		ereal<> result = ceil(x);

		bool test4_pass = (result == expected);
		std::cout << "  ceil(2.3) == 3.0: " << (test4_pass ? "PASS" : "FAIL") << "\n";
		if (!test4_pass) ++nrOfFailedTestCases;
	}

	// Test 5: ceil - negative values
	{
		std::cout << "\nTest 5: ceil (negative values)\n";
		ereal<> x(-2.7), expected(-2.0);
		ereal<> result = ceil(x);

		bool test5_pass = (result == expected);
		std::cout << "  ceil(-2.7) == -2.0: " << (test5_pass ? "PASS" : "FAIL") << "\n";
		if (!test5_pass) ++nrOfFailedTestCases;
	}

	// Test 6: ceil - already integer
	{
		std::cout << "\nTest 6: ceil (integer values)\n";
		ereal<> x(5.0), expected(5.0);
		ereal<> result = ceil(x);

		bool test6_pass = (result == expected);
		std::cout << "  ceil(5.0) == 5.0: " << (test6_pass ? "PASS" : "FAIL") << "\n";
		if (!test6_pass) ++nrOfFailedTestCases;
	}

	// Test 7: zero handling
	{
		std::cout << "\nTest 7: zero handling\n";
		ereal<> zero(0.0);
		ereal<> result_floor = floor(zero);
		ereal<> result_ceil = ceil(zero);

		bool test7_pass = (result_floor == zero) && (result_ceil == zero);
		std::cout << "  floor(0.0) == 0.0: " << ((result_floor == zero) ? "PASS" : "FAIL") << "\n";
		std::cout << "  ceil(0.0) == 0.0: " << ((result_ceil == zero) ? "PASS" : "FAIL") << "\n";
		if (!test7_pass) ++nrOfFailedTestCases;
	}

	std::cout << "\nPhase 1: Component-wise floor/ceil - "
	          << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "Note: floor/ceil use component-wise operations on expansion\n";
	std::cout << "Note: trunc/round still use stubs (deferred to Phase 2)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

	// Phase 0: No automated tests yet
	// TODO Phase 1: Add REGRESSION_LEVEL_1 tests (basic truncation functionality)
	// TODO Phase 1: Add REGRESSION_LEVEL_2 tests (edge cases and negative values)
	// TODO Phase 2: Add REGRESSION_LEVEL_3 tests (multi-component precision)
	// TODO Phase 2: Add REGRESSION_LEVEL_4 tests (stress testing)

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
