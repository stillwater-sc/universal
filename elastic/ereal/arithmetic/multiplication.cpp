// multiplication.cpp: Test ereal multiplication using expansion operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::cout << "ereal Multiplication Tests\n";
	std::cout << "==========================\n\n";

	int nrOfFailedTests = 0;

	// Test 1: Basic ereal * ereal multiplication
	{
		ereal<16> a(5.0);
		ereal<16> b(3.0);
		ereal<16> c = a * b;

		double expected = 15.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-13) {
			std::cout << "FAIL: 5 * 3 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 2: Scalar multiplication
	{
		ereal<16> a(5.0);
		ereal<16> c = a * 3.0;

		double expected = 15.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-13) {
			std::cout << "FAIL: 5 * 3.0 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 3: Multiply by zero
	{
		ereal<16> a(42.0);
		ereal<16> zero(0.0);
		ereal<16> c = a * zero;

		double expected = 0.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-14) {
			std::cout << "FAIL: 42 * 0 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 4: Multiply by one (identity)
	{
		ereal<16> a(42.0);
		ereal<16> one(1.0);
		ereal<16> c = a * one;

		double expected = 42.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-13) {
			std::cout << "FAIL: 42 * 1 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 5: Multiply by negative one
	{
		ereal<16> a(42.0);
		ereal<16> c = a * -1.0;

		double expected = -42.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-13) {
			std::cout << "FAIL: 42 * -1 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 6: Commutivity a * b = b * a
	{
		ereal<16> a(7.0);
		ereal<16> b(3.0);
		ereal<16> c1 = a * b;
		ereal<16> c2 = b * a;

		double result1 = double(c1);
		double result2 = double(c2);

		if (std::abs(result1 - result2) > 1.0e-13) {
			std::cout << "FAIL: Commutivity: a*b = " << result1 << " but b*a = " << result2 << "\n";
			++nrOfFailedTests;
		}
	}

	// Test 7: Associativity (a * b) * c ≈ a * (b * c)
	{
		ereal<16> a(2.0);
		ereal<16> b(3.0);
		ereal<16> c(5.0);

		ereal<16> result1 = (a * b) * c;
		ereal<16> result2 = a * (b * c);

		double val1 = double(result1);
		double val2 = double(result2);

		if (std::abs(val1 - val2) > 1.0e-12) {
			std::cout << "FAIL: Associativity: (a*b)*c = " << val1 << " but a*(b*c) = " << val2 << "\n";
			++nrOfFailedTests;
		}
	}

	// Test 8: Distributive property a * (b + c) ≈ a*b + a*c
	{
		ereal<16> a(2.0);
		ereal<16> b(3.0);
		ereal<16> c(5.0);

		ereal<16> left = a * (b + c);
		ereal<16> right = (a * b) + (a * c);

		double val_left = double(left);
		double val_right = double(right);

		if (std::abs(val_left - val_right) > 1.0e-12) {
			std::cout << "FAIL: Distributive: a*(b+c) = " << val_left << " but a*b + a*c = " << val_right << "\n";
			++nrOfFailedTests;
		}
	}

	std::cout << "\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All ereal multiplication tests passed\n";
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
