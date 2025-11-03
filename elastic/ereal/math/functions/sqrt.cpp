// sqrt.cpp: test suite runner for sqrt/cbrt functions for ereal adaptive-precision
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

	std::string test_suite  = "ereal mathlib sqrt/cbrt function validation";
	std::string test_tag    = "sqrt/cbrt";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 3: Newton-Raphson implementation with full adaptive precision

	std::cout << "Phase 3: Testing sqrt and cbrt with Newton-Raphson iteration\n\n";

	// Test 1: sqrt - exact values
	{
		std::cout << "Test 1: sqrt exact values\n";
		ereal<> a(4.0), expected(2.0);
		ereal<> result = sqrt(a);

		bool test1_pass = (result == expected);
		std::cout << "  sqrt(4.0) == 2.0: " << (test1_pass ? "PASS" : "FAIL") << "\n";
		if (!test1_pass) ++nrOfFailedTestCases;
	}

	// Test 2: sqrt - irrational value (sqrt(2))
	{
		std::cout << "\nTest 2: sqrt(2) precision verification\n";
		ereal<> a(2.0);
		ereal<> result = sqrt(a);

		// Verify: (sqrt(2))^2 ≈ 2.0
		ereal<> squared = result * result;
		ereal<> diff = squared - a;

		bool test2_pass = (std::abs(double(diff)) < 1e-15);
		std::cout << "  (sqrt(2))^2 ≈ 2.0 within 1e-15: " << (test2_pass ? "PASS" : "FAIL") << "\n";
		if (!test2_pass) ++nrOfFailedTestCases;
	}

	// Test 3: sqrt - another irrational (sqrt(3))
	{
		std::cout << "\nTest 3: sqrt(3) precision verification\n";
		ereal<> a(3.0);
		ereal<> result = sqrt(a);

		// Verify: (sqrt(3))^2 ≈ 3.0
		ereal<> squared = result * result;
		ereal<> diff = squared - a;

		bool test3_pass = (std::abs(double(diff)) < 1e-15);
		std::cout << "  (sqrt(3))^2 ≈ 3.0 within 1e-15: " << (test3_pass ? "PASS" : "FAIL") << "\n";
		if (!test3_pass) ++nrOfFailedTestCases;
	}

	// Test 4: sqrt - zero handling
	{
		std::cout << "\nTest 4: sqrt(0) == 0\n";
		ereal<> zero(0.0);
		ereal<> result = sqrt(zero);

		bool test4_pass = (result == zero);
		std::cout << "  sqrt(0.0) == 0.0: " << (test4_pass ? "PASS" : "FAIL") << "\n";
		if (!test4_pass) ++nrOfFailedTestCases;
	}

	// Test 5: cbrt - exact values
	{
		std::cout << "\nTest 5: cbrt exact values\n";
		ereal<> a(8.0), expected(2.0);
		ereal<> result = cbrt(a);

		bool test5_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  cbrt(8.0) ≈ 2.0: " << (test5_pass ? "PASS" : "FAIL") << "\n";
		if (!test5_pass) ++nrOfFailedTestCases;
	}

	// Test 6: cbrt - another exact value
	{
		std::cout << "\nTest 6: cbrt(27) == 3\n";
		ereal<> a(27.0), expected(3.0);
		ereal<> result = cbrt(a);

		bool test6_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  cbrt(27.0) ≈ 3.0: " << (test6_pass ? "PASS" : "FAIL") << "\n";
		if (!test6_pass) ++nrOfFailedTestCases;
	}

	// Test 7: cbrt - negative value (sign preservation)
	{
		std::cout << "\nTest 7: cbrt negative values (sign preservation)\n";
		ereal<> a(-8.0), expected(-2.0);
		ereal<> result = cbrt(a);

		bool test7_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  cbrt(-8.0) ≈ -2.0: " << (test7_pass ? "PASS" : "FAIL") << "\n";
		if (!test7_pass) ++nrOfFailedTestCases;
	}

	// Test 8: cbrt - another negative value
	{
		std::cout << "\nTest 8: cbrt(-27) == -3\n";
		ereal<> a(-27.0), expected(-3.0);
		ereal<> result = cbrt(a);

		bool test8_pass = (std::abs(double(result - expected)) < 1e-15);
		std::cout << "  cbrt(-27.0) ≈ -3.0: " << (test8_pass ? "PASS" : "FAIL") << "\n";
		if (!test8_pass) ++nrOfFailedTestCases;
	}

	// Test 9: cbrt - verification (cbrt(x))^3 = x
	{
		std::cout << "\nTest 9: cbrt(2) precision verification\n";
		ereal<> a(2.0);
		ereal<> result = cbrt(a);

		// Verify: (cbrt(2))^3 ≈ 2.0
		ereal<> cubed = result * result * result;
		ereal<> diff = cubed - a;

		bool test9_pass = (std::abs(double(diff)) < 1e-15);
		std::cout << "  (cbrt(2))^3 ≈ 2.0 within 1e-15: " << (test9_pass ? "PASS" : "FAIL") << "\n";
		if (!test9_pass) ++nrOfFailedTestCases;
	}

	// Test 10: cbrt - zero handling
	{
		std::cout << "\nTest 10: cbrt(0) == 0\n";
		ereal<> zero(0.0);
		ereal<> result = cbrt(zero);

		bool test10_pass = (result == zero);
		std::cout << "  cbrt(0.0) == 0.0: " << (test10_pass ? "PASS" : "FAIL") << "\n";
		if (!test10_pass) ++nrOfFailedTestCases;
	}

	std::cout << "\nPhase 3: sqrt/cbrt functions - "
	          << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "Note: sqrt uses Newton-Raphson: x' = (x + a/x) / 2\n";
	std::cout << "Note: cbrt uses frexp/ldexp range reduction + Newton-Raphson\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

	// Phase 0: No automated tests yet
	// TODO Phase 2: Add REGRESSION_LEVEL_1 tests (basic sqrt/cbrt at double precision)
	// TODO Phase 2: Add REGRESSION_LEVEL_2 tests (extended precision 100-200 bits)
	// TODO Phase 2: Add REGRESSION_LEVEL_3 tests (high precision 200-500 bits)
	// TODO Phase 2: Add REGRESSION_LEVEL_4 tests (extreme precision 500-1000 bits)

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
