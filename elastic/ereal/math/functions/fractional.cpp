// fractional.cpp: test suite runner for fractional functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {

		// Verify fmod function
		template<typename Real>
		int VerifyFmod(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: fmod property x - fmod(x,y) = n*y where n = trunc(x/y)
			Real x(5.3), y(2.0);
			Real result = fmod(x, y);
			Real n = trunc(x / y);
			Real expected = x - (n * y);

			if (result != expected) {
				if (reportTestCases) std::cerr << "FAIL: fmod(5.3, 2.0) property violation\n";
				++nrOfFailedTestCases;
			}

			// Test: fmod sign property (same sign as dividend)
			Real pos_result = fmod(Real(5.3), Real(2.0));
			Real neg_result = fmod(Real(-5.3), Real(2.0));

			if (!pos_result.ispos()) {
				if (reportTestCases) std::cerr << "FAIL: fmod(5.3, 2.0) should be positive\n";
				++nrOfFailedTestCases;
			}
			if (!neg_result.isneg()) {
				if (reportTestCases) std::cerr << "FAIL: fmod(-5.3, 2.0) should be negative\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify remainder function with IEEE round-to-nearest-even
		template<typename Real>
		int VerifyRemainder(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test 1: Normal case (no tie)
			// 7/3 = 2.333... -> rounds to 2 -> 7 - 2*3 = 1
			{
				Real x(7.0), y(3.0);
				Real result = remainder(x, y);
				Real expected(1.0);
				if (result != expected) {
					if (reportTestCases) std::cerr << "FAIL: remainder(7.0, 3.0) = "
						<< double(result) << ", expected 1.0\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 2: IEEE round-to-nearest-even - tie case (floor_q is even)
			// 5/2 = 2.5 -> tie, floor=2 (even), ceil=3 (odd) -> choose 2 (even)
			// 5 - 2*2 = 1
			{
				Real x(5.0), y(2.0);
				Real result = remainder(x, y);
				Real expected(1.0);
				if (result != expected) {
					if (reportTestCases) std::cerr << "FAIL: remainder(5.0, 2.0) = "
						<< double(result) << ", expected 1.0 (rounds 2.5 to 2 even)\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 3: IEEE round-to-nearest-even - tie case (ceil_q is even)
			// 7/2 = 3.5 -> tie, floor=3 (odd), ceil=4 (even) -> choose 4 (even)
			// 7 - 4*2 = -1
			{
				Real x(7.0), y(2.0);
				Real result = remainder(x, y);
				Real expected(-1.0);
				if (result != expected) {
					if (reportTestCases) std::cerr << "FAIL: remainder(7.0, 2.0) = "
						<< double(result) << ", expected -1.0 (rounds 3.5 to 4 even)\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 4: Negative values
			// -7/3 = -2.333... -> rounds to -2 -> -7 - (-2)*3 = -1
			{
				Real x(-7.0), y(3.0);
				Real result = remainder(x, y);
				Real expected(-1.0);
				if (result != expected) {
					if (reportTestCases) std::cerr << "FAIL: remainder(-7.0, 3.0) = "
						<< double(result) << ", expected -1.0\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 5: Exact division should give zero
			// 9/3 = 3.0 (exact) -> 9 - 3*3 = 0
			{
				Real x(9.0), y(3.0);
				Real result = remainder(x, y);
				Real expected(0.0);
				if (result != expected) {
					if (reportTestCases) std::cerr << "FAIL: remainder(9.0, 3.0) = "
						<< double(result) << ", expected 0.0\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 6: Result in range [-|y|/2, |y|/2]
			// 10/3 = 3.333... -> rounds to 3 -> 10 - 3*3 = 1
			{
				Real x(10.0), y(3.0);
				Real result = remainder(x, y);
				Real expected(1.0);
				Real y_half = y / Real(2.0);

				if (result != expected) {
					if (reportTestCases) std::cerr << "FAIL: remainder(10.0, 3.0) = "
						<< double(result) << ", expected 1.0\n";
					++nrOfFailedTestCases;
				}

				// Verify result is in range [-1.5, 1.5]
				if (abs(result) > y_half) {
					if (reportTestCases) std::cerr << "FAIL: remainder(10.0, 3.0) out of range [-1.5, 1.5]\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 7: Another tie case with negative result
			// 9/4 = 2.25 -> rounds to 2 -> 9 - 2*4 = 1
			// 11/4 = 2.75 -> rounds to 3 -> 11 - 3*4 = -1
			{
				Real x(11.0), y(4.0);
				Real result = remainder(x, y);
				Real expected(-1.0);
				if (result != expected) {
					if (reportTestCases) std::cerr << "FAIL: remainder(11.0, 4.0) = "
						<< double(result) << ", expected -1.0\n";
					++nrOfFailedTestCases;
				}
			}

			return nrOfFailedTestCases;
		}

		// Verify division by zero exception handling
		template<typename Real>
		int VerifyDivisionByZeroExceptions(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: remainder(x, 0) should throw ereal_divide_by_zero
			{
				Real x(5.0), y(0.0);
				bool caught_exception = false;

				try {
					Real result = remainder(x, y);
					// If we get here, exception was not thrown
					if (reportTestCases) std::cerr << "FAIL: remainder(5.0, 0.0) should throw exception\n";
					++nrOfFailedTestCases;
				}
				catch (const ereal_divide_by_zero&) {
					caught_exception = true;
				}
				catch (...) {
					if (reportTestCases) std::cerr << "FAIL: remainder(5.0, 0.0) threw wrong exception type\n";
					++nrOfFailedTestCases;
				}

				if (!caught_exception) {
					++nrOfFailedTestCases;
				}
			}

			// Test: fmod(x, 0) should throw ereal_divide_by_zero
			{
				Real x(5.0), y(0.0);
				bool caught_exception = false;

				try {
					Real result = fmod(x, y);
					// If we get here, exception was not thrown
					if (reportTestCases) std::cerr << "FAIL: fmod(5.0, 0.0) should throw exception\n";
					++nrOfFailedTestCases;
				}
				catch (const ereal_divide_by_zero&) {
					caught_exception = true;
				}
				catch (...) {
					if (reportTestCases) std::cerr << "FAIL: fmod(5.0, 0.0) threw wrong exception type\n";
					++nrOfFailedTestCases;
				}

				if (!caught_exception) {
					++nrOfFailedTestCases;
				}
			}

			return nrOfFailedTestCases;
		}

		// Verify fmod vs remainder difference
		template<typename Real>
		int VerifyFmodVsRemainder(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: fmod and remainder should differ for certain inputs
			// Example: 5.3 / 2.0 = 2.65
			//   fmod: trunc(2.65) = 2, so 5.3 - 4.0 = 1.3
			//   remainder: round(2.65) = 3, so 5.3 - 6.0 = -0.7
			Real x(5.3), y(2.0);
			Real fmod_result = fmod(x, y);
			Real remainder_result = remainder(x, y);

			if (fmod_result == remainder_result) {
				if (reportTestCases) std::cerr << "FAIL: fmod and remainder should differ for 5.3/2.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

	}
}

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

	std::string test_suite  = "ereal mathlib fractional function validation";
	std::string test_tag    = "fractional";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of fractional functions:\n";

	std::cout << "\nBasic tests:\n";
	ereal<> x(5.3), y(2.0);
	std::cout << "fmod(5.3, 2.0) = " << double(fmod(x, y)) << " (expected: 1.3)\n";
	std::cout << "remainder(5.3, 2.0) = " << double(remainder(x, y)) << " (expected: -0.7)\n";

	std::cout << "\nIEEE round-to-nearest-even tie cases:\n";
	ereal<> x1(5.0), y1(2.0);
	std::cout << "remainder(5.0, 2.0) = " << double(remainder(x1, y1))
		<< " (expected: 1.0, rounds 2.5 to 2 even)\n";

	ereal<> x2(7.0), y2(2.0);
	std::cout << "remainder(7.0, 2.0) = " << double(remainder(x2, y2))
		<< " (expected: -1.0, rounds 3.5 to 4 even)\n";

	std::cout << "\nDivision by zero exception test:\n";
	try {
		ereal<> x3(5.0), y3(0.0);
		ereal<> result = remainder(x3, y3);
		std::cout << "ERROR: Should have thrown exception!\n";
	}
	catch (const ereal_divide_by_zero& e) {
		std::cout << "Caught expected exception: " << e.what() << "\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	test_tag = "fmod";
	nrOfFailedTestCases += ReportTestResult(VerifyFmod<ereal<>>(reportTestCases), "fmod(ereal)", test_tag);

	test_tag = "remainder";
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<ereal<>>(reportTestCases), "remainder(ereal)", test_tag);

	test_tag = "fmod vs remainder";
	nrOfFailedTestCases += ReportTestResult(VerifyFmodVsRemainder<ereal<>>(reportTestCases), "fmod vs remainder", test_tag);
#endif

#if REGRESSION_LEVEL_2
	test_tag = "division by zero exceptions";
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionByZeroExceptions<ereal<>>(reportTestCases), "division by zero", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// Future: Precision validation
#endif

#if REGRESSION_LEVEL_4
	// Future: Stress tests
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
