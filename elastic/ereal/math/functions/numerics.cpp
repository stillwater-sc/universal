// numerics.cpp: test suite runner for numeric support functions for ereal adaptive-precision
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

	std::string test_suite  = "ereal mathlib numeric support function validation";
	std::string test_tag    = "frexp/ldexp/copysign";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 1: copysign implementation (complete)
	// Phase 2: frexp and ldexp implementation (complete)

	std::cout << "Phase 1 & 2: Testing numeric functions\n\n";

	// Test 1: copysign - positive to negative
	{
		std::cout << "Test 1: copysign (positive magnitude, negative sign)\n";
		ereal<> x(5.0), y(-3.0), expected(-5.0);
		ereal<> result = copysign(x, y);

		bool test1_pass = (result == expected);
		std::cout << "  copysign(5.0, -3.0) == -5.0: " << (test1_pass ? "PASS" : "FAIL") << "\n";
		if (!test1_pass) ++nrOfFailedTestCases;
	}

	// Test 2: copysign - negative to positive
	{
		std::cout << "\nTest 2: copysign (negative magnitude, positive sign)\n";
		ereal<> x(-5.0), y(3.0), expected(5.0);
		ereal<> result = copysign(x, y);

		bool test2_pass = (result == expected);
		std::cout << "  copysign(-5.0, 3.0) == 5.0: " << (test2_pass ? "PASS" : "FAIL") << "\n";
		std::cout << "  (magnitude of |-5|=5, sign of +3 = +5)\n";
		if (!test2_pass) ++nrOfFailedTestCases;
	}

	// Test 3: copysign - same sign (positive)
	{
		std::cout << "\nTest 3: copysign (both positive)\n";
		ereal<> x(5.0), y(3.0), expected(5.0);
		ereal<> result = copysign(x, y);

		bool test3_pass = (result == expected);
		std::cout << "  copysign(5.0, 3.0) == 5.0: " << (test3_pass ? "PASS" : "FAIL") << "\n";
		if (!test3_pass) ++nrOfFailedTestCases;
	}

	// Test 4: copysign - same sign (negative)
	{
		std::cout << "\nTest 4: copysign (both negative)\n";
		ereal<> x(-5.0), y(-3.0), expected(-5.0);
		ereal<> result = copysign(x, y);

		bool test4_pass = (result == expected);
		std::cout << "  copysign(-5.0, -3.0) == -5.0: " << (test4_pass ? "PASS" : "FAIL") << "\n";
		if (!test4_pass) ++nrOfFailedTestCases;
	}

	// Test 5: copysign with zero
	{
		std::cout << "\nTest 5: copysign with zero\n";
		ereal<> zero(0.0), pos(1.0), neg(-1.0);

		ereal<> result1 = copysign(zero, neg);
		ereal<> result2 = copysign(pos, zero);

		// Zero stays zero regardless of sign copy
		bool test5_pass = (result1 == zero) && (result2 == pos);
		std::cout << "  copysign(0.0, -1.0) == 0.0: " << ((result1 == zero) ? "PASS" : "FAIL") << "\n";
		std::cout << "  copysign(1.0, 0.0) == 1.0: " << ((result2 == pos) ? "PASS" : "FAIL") << "\n";
		if (!test5_pass) ++nrOfFailedTestCases;
	}

	// Test 6: ldexp - positive exponent
	{
		std::cout << "\nTest 6: ldexp (positive exponent)\n";
		ereal<> x(1.0), expected(8.0);
		ereal<> result = ldexp(x, 3);  // 1.0 * 2^3 = 8.0

		bool test6_pass = (result == expected);
		std::cout << "  ldexp(1.0, 3) == 8.0: " << (test6_pass ? "PASS" : "FAIL") << "\n";
		if (!test6_pass) ++nrOfFailedTestCases;
	}

	// Test 7: ldexp - negative exponent
	{
		std::cout << "\nTest 7: ldexp (negative exponent)\n";
		ereal<> x(1.0), expected(0.25);
		ereal<> result = ldexp(x, -2);  // 1.0 * 2^-2 = 0.25

		bool test7_pass = (result == expected);
		std::cout << "  ldexp(1.0, -2) == 0.25: " << (test7_pass ? "PASS" : "FAIL") << "\n";
		if (!test7_pass) ++nrOfFailedTestCases;
	}

	// Test 8: frexp - basic test
	{
		std::cout << "\nTest 8: frexp (basic)\n";
		ereal<> x(8.0);
		int exp;
		ereal<> mantissa = frexp(x, &exp);  // 8.0 = 0.5 * 2^4
		ereal<> expected_mantissa(0.5);
		int expected_exp = 4;

		bool test8_pass = (mantissa == expected_mantissa) && (exp == expected_exp);
		std::cout << "  frexp(8.0) mantissa == 0.5: " << ((mantissa == expected_mantissa) ? "PASS" : "FAIL") << "\n";
		std::cout << "  frexp(8.0) exponent == 4: " << ((exp == expected_exp) ? "PASS" : "FAIL") << "\n";
		if (!test8_pass) ++nrOfFailedTestCases;
	}

	// Test 9: frexp/ldexp roundtrip
	{
		std::cout << "\nTest 9: frexp/ldexp roundtrip\n";
		ereal<> x(6.0);
		int exp;
		ereal<> mantissa = frexp(x, &exp);
		ereal<> reconstructed = ldexp(mantissa, exp);

		bool test9_pass = (reconstructed == x);
		std::cout << "  ldexp(frexp(6.0)) == 6.0: " << (test9_pass ? "PASS" : "FAIL") << "\n";
		if (!test9_pass) ++nrOfFailedTestCases;
	}

	std::cout << "\nPhase 1 & 2: All numeric functions - "
	          << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "Note: copysign uses sign() method (Phase 1)\n";
	std::cout << "Note: frexp/ldexp use component scaling (Phase 2)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

	// Phase 0: No automated tests yet
	// TODO Phase 1: Add REGRESSION_LEVEL_1 tests (basic numeric functionality)
	// TODO Phase 1: Add REGRESSION_LEVEL_2 tests (edge cases and power-of-2 scaling)
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
