// fractional.cpp: test suite runner for fractional functions for ereal adaptive-precision
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

	std::string test_suite  = "ereal mathlib fractional function validation";
	std::string test_tag    = "fmod/remainder";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 2: fmod and remainder implementation using expansion quotient

	std::cout << "Phase 2: Testing fractional functions (fmod, remainder)\n\n";

	// Test 1: fmod - basic positive values
	{
		std::cout << "Test 1: fmod (basic positive)\n";
		ereal<> x(5.3), y(2.0);
		ereal<> result = fmod(x, y);
		// fmod(5.3, 2.0) = 5.3 - trunc(5.3/2.0)*2.0 = 5.3 - 2*2.0 = 1.3
		ereal<> n = trunc(x / y);
		ereal<> expected = x - (n * y);

		bool test1_pass = (result == expected);
		std::cout << "  fmod(5.3, 2.0) correct: " << (test1_pass ? "PASS" : "FAIL") << "\n";
		if (!test1_pass) ++nrOfFailedTestCases;
	}

	// Test 2: fmod - negative dividend
	{
		std::cout << "\nTest 2: fmod (negative dividend)\n";
		ereal<> x(-5.3), y(2.0);
		ereal<> result = fmod(x, y);
		ereal<> n = trunc(x / y);
		ereal<> expected = x - (n * y);

		bool test2_pass = (result == expected);
		std::cout << "  fmod(-5.3, 2.0) correct: " << (test2_pass ? "PASS" : "FAIL") << "\n";
		if (!test2_pass) ++nrOfFailedTestCases;
	}

	// Test 3: fmod - result has same sign as dividend
	{
		std::cout << "\nTest 3: fmod sign property\n";
		ereal<> pos_x(5.3), neg_x(-5.3), y(2.0);
		ereal<> result_pos = fmod(pos_x, y);
		ereal<> result_neg = fmod(neg_x, y);

		bool test3_pass = result_pos.ispos() && result_neg.isneg();
		std::cout << "  fmod(+5.3, 2.0) > 0: " << (result_pos.ispos() ? "PASS" : "FAIL") << "\n";
		std::cout << "  fmod(-5.3, 2.0) < 0: " << (result_neg.isneg() ? "PASS" : "FAIL") << "\n";
		if (!test3_pass) ++nrOfFailedTestCases;
	}

	// Test 4: remainder - basic positive values
	{
		std::cout << "\nTest 4: remainder (basic positive)\n";
		ereal<> x(5.3), y(2.0);
		ereal<> result = remainder(x, y);
		// remainder(5.3, 2.0) = 5.3 - round(5.3/2.0)*2.0 = 5.3 - 3*2.0 = -0.7
		ereal<> n = round(x / y);
		ereal<> expected = x - (n * y);

		bool test4_pass = (result == expected);
		std::cout << "  remainder(5.3, 2.0) correct: " << (test4_pass ? "PASS" : "FAIL") << "\n";
		if (!test4_pass) ++nrOfFailedTestCases;
	}

	// Test 5: remainder vs fmod difference
	{
		std::cout << "\nTest 5: remainder vs fmod\n";
		ereal<> x(5.3), y(2.0);
		ereal<> fmod_result = fmod(x, y);
		ereal<> remainder_result = remainder(x, y);

		// fmod uses trunc (toward zero), remainder uses round (nearest)
		// For 5.3/2.0 = 2.65:
		//   trunc(2.65) = 2, so fmod = 5.3 - 4.0 = 1.3
		//   round(2.65) = 3, so remainder = 5.3 - 6.0 = -0.7
		bool different = !(fmod_result == remainder_result);
		std::cout << "  fmod(5.3, 2.0) != remainder(5.3, 2.0): " << (different ? "PASS" : "FAIL") << "\n";
		if (!different) ++nrOfFailedTestCases;
	}

	// Test 6: exact division
	{
		std::cout << "\nTest 6: exact division\n";
		ereal<> x(6.0), y(2.0), zero(0.0);
		ereal<> result = remainder(x, y);

		bool test6_pass = (result == zero);
		std::cout << "  remainder(6.0, 2.0) == 0.0: " << (test6_pass ? "PASS" : "FAIL") << "\n";
		if (!test6_pass) ++nrOfFailedTestCases;
	}

	std::cout << "\nPhase 2: fractional functions - "
	          << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "Note: fmod uses trunc (round toward zero)\n";
	std::cout << "Note: remainder uses round (round to nearest)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

	// Phase 0: No automated tests yet
	// TODO Phase 1: Add REGRESSION_LEVEL_1 tests (basic fmod/remainder)
	// TODO Phase 1: Add REGRESSION_LEVEL_2 tests (edge cases and negative values)
	// TODO Phase 2: Add REGRESSION_LEVEL_3 tests (precision validation)
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
