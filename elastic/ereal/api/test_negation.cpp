// test_negation.cpp: test ereal negation operator
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

	std::cout << "Testing ereal negation operator\n";
	std::cout << "================================\n\n";

	// Test 1: Simple negation
	{
		std::cout << "Test 1: Simple negation\n";
		ereal<64> a(1000.0);
		ereal<64> neg_a = -a;

		std::cout << "  a     = " << double(a) << "\n";
		std::cout << "  -a    = " << double(neg_a) << "\n";
		std::cout << "  Expected: -1000.0\n";

		if (std::abs(double(neg_a) - (-1000.0)) < 1e-10) {
			std::cout << "  PASS\n\n";
		} else {
			std::cout << "  FAIL\n\n";
		}
	}

	// Test 2: Negation in expression
	{
		std::cout << "Test 2: Negation in expression\n";
		ereal<64> b(1000.0);
		ereal<64> result = -b + ereal<64>(500.0);

		std::cout << "  b           = " << double(b) << "\n";
		std::cout << "  -b + 500    = " << double(result) << "\n";
		std::cout << "  Expected: -500.0\n";

		if (std::abs(double(result) - (-500.0)) < 1e-10) {
			std::cout << "  PASS\n\n";
		} else {
			std::cout << "  FAIL\n\n";
		}
	}

	// Test 3: Subtraction from zero
	{
		std::cout << "Test 3: Subtraction from zero\n";
		ereal<64> zero(0.0);
		ereal<64> b(1000.0);
		ereal<64> result = zero - b;

		std::cout << "  0 - b  = " << double(result) << "\n";
		std::cout << "  Expected: -1000.0\n";

		if (std::abs(double(result) - (-1000.0)) < 1e-10) {
			std::cout << "  PASS\n\n";
		} else {
			std::cout << "  FAIL\n\n";
		}
	}

	// Test 4: The actual quadratic case
	{
		std::cout << "Test 4: Quadratic formula case\n";
		ereal<64> b(1000.0);
		ereal<64> sqrt_disc(999.998);

		ereal<64> neg_b = -b;
		ereal<64> term1 = neg_b - sqrt_disc;  // Should be -1000 - 999.998 = -1999.998
		ereal<64> two(2.0);
		ereal<64> x1 = term1 / two;

		std::cout << "  b           = " << double(b) << "\n";
		std::cout << "  -b          = " << double(neg_b) << "\n";
		std::cout << "  sqrt_disc   = " << double(sqrt_disc) << "\n";
		std::cout << "  -b - sqrt   = " << double(term1) << "\n";
		std::cout << "  x1 = term/2 = " << double(x1) << "\n";
		std::cout << "  Expected x1: -999.999\n";

		if (std::abs(double(x1) - (-999.999)) < 0.01) {
			std::cout << "  PASS\n\n";
		} else {
			std::cout << "  FAIL\n\n";
		}
	}

	// Test 5: Check limbs directly
	{
		std::cout << "Test 5: Check limbs directly\n";
		ereal<64> a(1000.0);
		ereal<64> neg_a = -a;

		std::cout << "  a limbs: ";
		for (auto limb : a.limbs()) {
			std::cout << limb << " ";
		}
		std::cout << "\n";

		std::cout << "  -a limbs: ";
		for (auto limb : neg_a.limbs()) {
			std::cout << limb << " ";
		}
		std::cout << "\n";
		std::cout << "  Expected: negative of each limb\n\n";
	}

	return EXIT_SUCCESS;
}
catch (const std::exception& ex) {
	std::cerr << "Caught exception: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
