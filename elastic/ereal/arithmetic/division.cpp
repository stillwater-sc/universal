// division.cpp: Test ereal division using expansion operations
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

	std::cout << "ereal Division Tests\n";
	std::cout << "====================\n\n";

	int nrOfFailedTests = 0;

	// Test 1: Basic division
	{
		ereal<16> a(15.0);
		ereal<16> b(3.0);
		ereal<16> c = a / b;

		double expected = 5.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-12) {
			std::cout << "FAIL: 15 / 3 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 2: Scalar division
	{
		ereal<16> a(15.0);
		ereal<16> c = a / 3.0;

		double expected = 5.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-12) {
			std::cout << "FAIL: 15 / 3.0 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 3: Divide by one (identity)
	{
		ereal<16> a(42.0);
		ereal<16> one(1.0);
		ereal<16> c = a / one;

		double expected = 42.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-12) {
			std::cout << "FAIL: 42 / 1 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 4: Self-division (a / a = 1)
	{
		ereal<16> a(42.0);
		ereal<16> c = a / a;

		double expected = 1.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-12) {
			std::cout << "FAIL: 42 / 42 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 5: Division resulting in fraction
	{
		ereal<16> a(1.0);
		ereal<16> b(3.0);
		ereal<16> c = a / b;

		double expected = 1.0 / 3.0;
		double result = double(c);

		// More relaxed tolerance for repeating decimal
		if (std::abs(result - expected) > 1.0e-11) {
			std::cout << "FAIL: 1 / 3 = " << std::setprecision(15) << result
			          << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 6: Identity (a / b) * b ≈ a
	{
		ereal<16> a(15.5);
		ereal<16> b(3.5);
		ereal<16> quotient = a / b;
		ereal<16> recovered = quotient * b;

		double expected = 15.5;
		double result = double(recovered);

		// Relaxed tolerance due to Newton iteration approximation
		if (std::abs(result - expected) > 1.0e-10) {
			std::cout << "FAIL: (a / b) * b identity: " << std::setprecision(15) << result
			          << " (expected " << expected << ")\n";
			std::cout << "  Difference: " << std::abs(result - expected) << "\n";
			++nrOfFailedTests;
		}
	}

	// Test 7: Reciprocal
	{
		ereal<16> a(4.0);
		ereal<16> one(1.0);
		ereal<16> reciprocal = one / a;

		double expected = 0.25;
		double result = double(reciprocal);

		if (std::abs(result - expected) > 1.0e-12) {
			std::cout << "FAIL: 1 / 4 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	std::cout << "\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All ereal division tests passed\n";
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
