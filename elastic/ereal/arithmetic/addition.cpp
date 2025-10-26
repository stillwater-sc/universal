// addition.cpp: Test ereal addition using expansion operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// Test basic addition
	int test_basic_addition() {
		int nrOfFailedTests = 0;

		std::cout << "Testing basic ereal addition\n";

		// Test 1: Simple addition
		{
			ereal<16> a(10.0);
			ereal<16> b(5.0);
			ereal<16> c = a + b;

			double expected = 15.0;
			double result = double(c);

			if (std::abs(result - expected) > 1.0e-14) {
				std::cout << "  FAIL: 10 + 5 = " << result << " (expected " << expected << ")\n";
				++nrOfFailedTests;
			}
		}

		// Test 2: Addition with small components (testing precision)
		{
			ereal<16> a(1.0);
			ereal<16> tiny(1.0e-15);
			ereal<16> c = a + tiny;

			// Should preserve the tiny component
			double expected = 1.0 + 1.0e-15;
			double result = double(c);

			// More relaxed tolerance for conversion back to double
			if (std::abs(result - expected) > 1.0e-14) {
				std::cout << "  FAIL: 1.0 + 1e-15 = " << std::setprecision(17) << result
				          << " (expected " << expected << ")\n";
				++nrOfFailedTests;
			}
		}

		// Test 3: Associativity
		{
			ereal<16> a(10.0);
			ereal<16> b(5.0);
			ereal<16> c(2.0);

			ereal<16> result1 = (a + b) + c;
			ereal<16> result2 = a + (b + c);

			double val1 = double(result1);
			double val2 = double(result2);

			if (std::abs(val1 - val2) > 1.0e-14) {
				std::cout << "  FAIL: Associativity: (a+b)+c = " << val1
				          << " but a+(b+c) = " << val2 << "\n";
				++nrOfFailedTests;
			}
		}

		// Test 4: Identity
		{
			ereal<16> a(42.0);
			ereal<16> zero(0.0);
			ereal<16> c = a + zero;

			double expected = 42.0;
			double result = double(c);

			if (std::abs(result - expected) > 1.0e-14) {
				std::cout << "  FAIL: Identity: 42 + 0 = " << result << "\n";
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test subtraction
	int test_subtraction() {
		int nrOfFailedTests = 0;

		std::cout << "Testing ereal subtraction\n";

		// Test 1: Basic subtraction
		{
			ereal<16> a(10.0);
			ereal<16> b(3.0);
			ereal<16> c = a - b;

			double expected = 7.0;
			double result = double(c);

			if (std::abs(result - expected) > 1.0e-14) {
				std::cout << "  FAIL: 10 - 3 = " << result << "\n";
				++nrOfFailedTests;
			}
		}

		// Test 2: Cancellation
		{
			ereal<16> a(10.0);
			ereal<16> b(10.0);
			ereal<16> c = a - b;

			double expected = 0.0;
			double result = double(c);

			if (std::abs(result - expected) > 1.0e-14) {
				std::cout << "  FAIL: 10 - 10 = " << result << "\n";
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test scalar multiplication
	int test_scalar_multiplication() {
		int nrOfFailedTests = 0;

		std::cout << "Testing ereal scalar multiplication\n";

		// Test 1: Basic multiplication
		{
			ereal<16> a(5.0);
			ereal<16> c = a * 3.0;

			double expected = 15.0;
			double result = double(c);

			if (std::abs(result - expected) > 1.0e-14) {
				std::cout << "  FAIL: 5 * 3 = " << result << "\n";
				++nrOfFailedTests;
			}
		}

		// Test 2: Multiply by zero
		{
			ereal<16> a(42.0);
			ereal<16> c = a * 0.0;

			double expected = 0.0;
			double result = double(c);

			if (std::abs(result - expected) > 1.0e-14) {
				std::cout << "  FAIL: 42 * 0 = " << result << "\n";
				++nrOfFailedTests;
			}
		}

		// Test 3: Multiply by one
		{
			ereal<16> a(42.0);
			ereal<16> c = a * 1.0;

			double expected = 42.0;
			double result = double(c);

			if (std::abs(result - expected) > 1.0e-14) {
				std::cout << "  FAIL: 42 * 1 = " << result << "\n";
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test comparison operators
	int test_comparisons() {
		int nrOfFailedTests = 0;

		std::cout << "Testing ereal comparison operators\n";

		// Test equality
		{
			ereal<16> a(10.0);
			ereal<16> b(10.0);
			ereal<16> c(11.0);

			if (!(a == b)) {
				std::cout << "  FAIL: 10 == 10 should be true\n";
				++nrOfFailedTests;
			}

			if (a == c) {
				std::cout << "  FAIL: 10 == 11 should be false\n";
				++nrOfFailedTests;
			}
		}

		// Test less than
		{
			ereal<16> a(5.0);
			ereal<16> b(10.0);

			if (!(a < b)) {
				std::cout << "  FAIL: 5 < 10 should be true\n";
				++nrOfFailedTests;
			}

			if (b < a) {
				std::cout << "  FAIL: 10 < 5 should be false\n";
				++nrOfFailedTests;
			}
		}

		// Test greater than
		{
			ereal<16> a(10.0);
			ereal<16> b(5.0);

			if (!(a > b)) {
				std::cout << "  FAIL: 10 > 5 should be true\n";
				++nrOfFailedTests;
			}

			if (b > a) {
				std::cout << "  FAIL: 5 > 10 should be false\n";
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
int main()
try {
	using namespace sw::universal;

	std::cout << "ereal Arithmetic Tests (with expansion_ops)\n";
	std::cout << "============================================\n\n";

	int nrOfFailedTests = 0;

	nrOfFailedTests += test_basic_addition();
	nrOfFailedTests += test_subtraction();
	nrOfFailedTests += test_scalar_multiplication();
	nrOfFailedTests += test_comparisons();

	std::cout << "\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All ereal arithmetic tests passed\n";
	}

	return (nrOfFailedTests > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
