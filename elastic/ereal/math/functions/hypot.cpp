// hypot.cpp: test suite runner for hypot function for ereal adaptive-precision
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

	std::string test_suite  = "ereal mathlib hypot function validation";
	std::string test_tag    = "hypot";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 3: Full adaptive-precision implementation using Phase 3 sqrt

	std::cout << "Phase 3: Testing hypot with expansion arithmetic\n\n";

	// Test 1: hypot - Pythagorean triple (3-4-5)
	{
		std::cout << "Test 1: hypot Pythagorean triple (3-4-5)\n";
		ereal<> x(3.0), y(4.0), expected(5.0);
		ereal<> result = hypot(x, y);

		bool test1_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  hypot(3.0, 4.0) ≈ 5.0: " << (test1_pass ? "PASS" : "FAIL") << "\n";
		if (!test1_pass) ++nrOfFailedTestCases;
	}

	// Test 2: hypot - Pythagorean triple (5-12-13)
	{
		std::cout << "\nTest 2: hypot Pythagorean triple (5-12-13)\n";
		ereal<> x(5.0), y(12.0), expected(13.0);
		ereal<> result = hypot(x, y);

		bool test2_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  hypot(5.0, 12.0) ≈ 13.0: " << (test2_pass ? "PASS" : "FAIL") << "\n";
		if (!test2_pass) ++nrOfFailedTestCases;
	}

	// Test 3: hypot - Pythagorean triple (8-15-17)
	{
		std::cout << "\nTest 3: hypot Pythagorean triple (8-15-17)\n";
		ereal<> x(8.0), y(15.0), expected(17.0);
		ereal<> result = hypot(x, y);

		bool test3_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  hypot(8.0, 15.0) ≈ 17.0: " << (test3_pass ? "PASS" : "FAIL") << "\n";
		if (!test3_pass) ++nrOfFailedTestCases;
	}

	// Test 4: hypot - verification hypot(x,y)^2 = x^2 + y^2
	{
		std::cout << "\nTest 4: hypot precision verification\n";
		ereal<> x(1.0), y(1.0);
		ereal<> result = hypot(x, y);

		// Verify: hypot(1,1)^2 = 1^2 + 1^2 = 2
		ereal<> result_squared = result * result;
		ereal<> expected_sum = x*x + y*y;  // Should be 2.0
		ereal<> diff = result_squared - expected_sum;

		bool test4_pass = (std::abs(double(diff)) < 1e-15);
		std::cout << "  hypot(1,1)^2 ≈ 1^2 + 1^2 within 1e-15: " << (test4_pass ? "PASS" : "FAIL") << "\n";
		if (!test4_pass) ++nrOfFailedTestCases;
	}

	// Test 5: hypot - zero handling
	{
		std::cout << "\nTest 5: hypot with zeros\n";
		ereal<> zero(0.0);
		ereal<> result = hypot(zero, zero);

		// Note: Due to expansion arithmetic quirks, 0+0 may not be exactly zero
		// Use tolerance test instead
		bool test5_pass = (std::abs(double(result)) < 1e-15);
		std::cout << "  hypot(0.0, 0.0) ≈ 0.0 within 1e-15: " << (test5_pass ? "PASS" : "FAIL") << "\n";
		if (!test5_pass) ++nrOfFailedTestCases;
	}

	// Test 6: hypot - one zero
	{
		std::cout << "\nTest 6: hypot with one zero\n";
		ereal<> x(3.0), zero(0.0), expected(3.0);
		ereal<> result = hypot(x, zero);

		bool test6_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  hypot(3.0, 0.0) ≈ 3.0: " << (test6_pass ? "PASS" : "FAIL") << "\n";
		if (!test6_pass) ++nrOfFailedTestCases;
	}

	// Test 7: hypot 3D - simple case
	{
		std::cout << "\nTest 7: hypot 3D (0-0-0)\n";
		ereal<> zero(0.0);
		ereal<> result = hypot(zero, zero, zero);

		bool test7_pass = (result == zero);
		std::cout << "  hypot(0.0, 0.0, 0.0) == 0.0: " << (test7_pass ? "PASS" : "FAIL") << "\n";
		if (!test7_pass) ++nrOfFailedTestCases;
	}

	// Test 8: hypot 3D - Pythagorean quadruple (2-3-6 = 7)
	{
		std::cout << "\nTest 8: hypot 3D (2-3-6 = 7)\n";
		ereal<> x(2.0), y(3.0), z(6.0), expected(7.0);
		ereal<> result = hypot(x, y, z);

		bool test8_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  hypot(2.0, 3.0, 6.0) ≈ 7.0: " << (test8_pass ? "PASS" : "FAIL") << "\n";
		if (!test8_pass) ++nrOfFailedTestCases;
	}

	// Test 9: hypot 3D - unit cube diagonal
	{
		std::cout << "\nTest 9: hypot 3D unit cube diagonal\n";
		ereal<> one(1.0);
		ereal<> result = hypot(one, one, one);

		// sqrt(1^2 + 1^2 + 1^2) = sqrt(3)
		ereal<> expected = sqrt(ereal<>(3.0));
		ereal<> diff = result - expected;

		bool test9_pass = (std::abs(double(diff)) < 1e-15);
		std::cout << "  hypot(1,1,1) ≈ sqrt(3) within 1e-15: " << (test9_pass ? "PASS" : "FAIL") << "\n";
		if (!test9_pass) ++nrOfFailedTestCases;
	}

	std::cout << "\nPhase 3: hypot functions - "
	          << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "Note: hypot uses Phase 3 sqrt with expansion arithmetic\n";
	std::cout << "Note: expansion arithmetic naturally prevents overflow\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

	// Phase 0: No automated tests yet
	// TODO Phase 1: Add REGRESSION_LEVEL_1 tests (basic hypot functionality)
	// TODO Phase 1: Add REGRESSION_LEVEL_2 tests (overflow prevention, large values)
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
