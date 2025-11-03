// classify.cpp: test suite runner for classification functions for ereal adaptive-precision
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

	std::string test_suite  = "ereal mathlib classification function validation";
	std::string test_tag    = "fpclassify/isnan/isinf/isfinite/isnormal/signbit";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 1: Full precision implementation using ereal's native methods

	std::cout << "Phase 1: Testing classification with ereal's native methods\n\n";

	// Test 1: isfinite - should be true for normal values
	{
		std::cout << "Test 1: isfinite\n";
		ereal<> x(2.0), y(-1.0), z(0.0);

		bool test1_pass = isfinite(x) && isfinite(y) && isfinite(z);
		std::cout << "  isfinite(2.0): " << (isfinite(x) ? "PASS" : "FAIL") << "\n";
		std::cout << "  isfinite(-1.0): " << (isfinite(y) ? "PASS" : "FAIL") << "\n";
		std::cout << "  isfinite(0.0): " << (isfinite(z) ? "PASS" : "FAIL") << "\n";
		if (!test1_pass) ++nrOfFailedTestCases;
	}

	// Test 2: isnan - should be false for normal values
	{
		std::cout << "\nTest 2: isnan\n";
		ereal<> x(2.0);

		bool test2_pass = !isnan(x);
		std::cout << "  isnan(2.0) == false: " << (test2_pass ? "PASS" : "FAIL") << "\n";
		if (!test2_pass) ++nrOfFailedTestCases;
	}

	// Test 3: isinf - should be false for normal values
	{
		std::cout << "\nTest 3: isinf\n";
		ereal<> x(2.0);

		bool test3_pass = !isinf(x);
		std::cout << "  isinf(2.0) == false: " << (test3_pass ? "PASS" : "FAIL") << "\n";
		if (!test3_pass) ++nrOfFailedTestCases;
	}

	// Test 4: isnormal - non-zero finite values are normal
	{
		std::cout << "\nTest 4: isnormal\n";
		ereal<> x(2.0), y(-1.0), z(0.0);

		bool test4_pass = isnormal(x) && isnormal(y) && !isnormal(z);
		std::cout << "  isnormal(2.0): " << (isnormal(x) ? "PASS" : "FAIL") << "\n";
		std::cout << "  isnormal(-1.0): " << (isnormal(y) ? "PASS" : "FAIL") << "\n";
		std::cout << "  isnormal(0.0) == false: " << (!isnormal(z) ? "PASS" : "FAIL") << "\n";
		if (!test4_pass) ++nrOfFailedTestCases;
	}

	// Test 5: signbit - test sign detection
	{
		std::cout << "\nTest 5: signbit\n";
		ereal<> pos(2.0), neg(-1.0), zero(0.0);

		bool test5_pass = !signbit(pos) && signbit(neg) && !signbit(zero);
		std::cout << "  signbit(2.0) == false: " << (!signbit(pos) ? "PASS" : "FAIL") << "\n";
		std::cout << "  signbit(-1.0) == true: " << (signbit(neg) ? "PASS" : "FAIL") << "\n";
		std::cout << "  signbit(0.0) == false: " << (!signbit(zero) ? "PASS" : "FAIL") << "\n";
		if (!test5_pass) ++nrOfFailedTestCases;
	}

	// Test 6: fpclassify
	{
		std::cout << "\nTest 6: fpclassify\n";
		ereal<> normal(2.0), zero(0.0);

		bool test6_pass = (fpclassify(normal) == FP_NORMAL) && (fpclassify(zero) == FP_ZERO);
		std::cout << "  fpclassify(2.0) == FP_NORMAL: " << ((fpclassify(normal) == FP_NORMAL) ? "PASS" : "FAIL") << "\n";
		std::cout << "  fpclassify(0.0) == FP_ZERO: " << ((fpclassify(zero) == FP_ZERO) ? "PASS" : "FAIL") << "\n";
		if (!test6_pass) ++nrOfFailedTestCases;
	}

	std::cout << "\nPhase 1: Full precision implementation - "
	          << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "Note: Classification uses ereal's native methods (isnan, isinf, iszero, isneg)\n";
	std::cout << "Note: ereal has no subnormal representation (expansion arithmetic)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

	// Phase 0: No automated tests yet
	// TODO Phase 1: Add REGRESSION_LEVEL_1 tests (basic classification)
	// TODO Phase 1: Add REGRESSION_LEVEL_2 tests (special values if supported)
	// TODO Phase 2: Add REGRESSION_LEVEL_3 tests (edge cases)
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
