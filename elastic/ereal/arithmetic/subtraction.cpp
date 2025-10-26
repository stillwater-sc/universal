// subtraction.cpp: Test ereal subtraction using expansion operations
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

	std::cout << "ereal Subtraction Tests\n";
	std::cout << "=======================\n\n";

	int nrOfFailedTests = 0;

	// Test 1: Basic subtraction
	{
		ereal<16> a(10.0);
		ereal<16> b(3.0);
		ereal<16> c = a - b;

		double expected = 7.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-14) {
			std::cout << "FAIL: 10 - 3 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 2: Subtraction resulting in negative
	{
		ereal<16> a(3.0);
		ereal<16> b(10.0);
		ereal<16> c = a - b;

		double expected = -7.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-14) {
			std::cout << "FAIL: 3 - 10 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 3: Complete cancellation
	{
		ereal<16> a(10.0);
		ereal<16> b(10.0);
		ereal<16> c = a - b;

		double expected = 0.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-14) {
			std::cout << "FAIL: 10 - 10 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 4: Subtract zero
	{
		ereal<16> a(42.0);
		ereal<16> zero(0.0);
		ereal<16> c = a - zero;

		double expected = 42.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-14) {
			std::cout << "FAIL: 42 - 0 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 5: Subtraction with scalar
	{
		ereal<16> a(10.0);
		ereal<16> c = a - 3.0;

		double expected = 7.0;
		double result = double(c);

		if (std::abs(result - expected) > 1.0e-14) {
			std::cout << "FAIL: 10 - 3.0 = " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	// Test 6: Identity (a - b) + b = a
	{
		ereal<16> a(15.5);
		ereal<16> b(7.25);
		ereal<16> diff = a - b;
		ereal<16> recovered = diff + b;

		double expected = 15.5;
		double result = double(recovered);

		if (std::abs(result - expected) > 1.0e-13) {
			std::cout << "FAIL: (a - b) + b identity: " << result << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	}

	std::cout << "\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All ereal subtraction tests passed\n";
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
