// subtraction.cpp: Identity-based tests for expansion subtraction operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/internal/expansion/expansion_ops.hpp>

namespace sw { namespace universal {

	// Helper: Print expansion for debugging
	void print_expansion(const std::string& name, const std::vector<double>& e) {
		std::cout << "  " << name << " = [";
		for (size_t i = 0; i < e.size(); ++i) {
			if (i > 0) std::cout << ", ";
			std::cout << std::setprecision(17) << e[i];
		}
		std::cout << "]  (" << e.size() << " components)\n";
	}

	// Helper: Sum expansion components
	double sum_expansion(const std::vector<double>& e) {
		double sum = 0.0;
		for (auto v : e) sum += v;
		return sum;
	}

	// Helper: Negate expansion
	std::vector<double> negate_expansion(const std::vector<double>& e) {
		std::vector<double> result = e;
		for (auto& v : result) v = -v;
		return result;
	}

	// Helper: Subtract expansions (a - b)
	std::vector<double> subtract_expansion(const std::vector<double>& a, const std::vector<double>& b) {
		using namespace expansion_ops;
		std::vector<double> neg_b = negate_expansion(b);
		return linear_expansion_sum(a, neg_b);
	}

	// ===================================================================
	// SUBTRACTION IDENTITY TESTS
	// ===================================================================

	// Test exact cancellation: a - a = [0]
	int test_subtraction_exact_cancellation() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: a - a = [0] (exact cancellation)\n";

		// Test case 1: Simple value
		{
			std::vector<double> a = { 42.0 };
			std::vector<double> result = subtract_expansion(a, a);

			double result_val = sum_expansion(result);

			if (result_val != 0.0) {
				std::cout << "  FAIL: [42] - [42] != [0]\n";
				std::cout << "    Result: " << std::setprecision(17) << result_val << "\n";
				print_expansion("result", result);
				++nrOfFailedTests;
			}
		}

		// Test case 2: Multi-component
		{
			std::vector<double> a = { 15.5, 7.75e-16, 3.875e-32 };
			std::vector<double> result = subtract_expansion(a, a);

			double result_val = sum_expansion(result);

			if (std::abs(result_val) > 1.0e-40) {
				std::cout << "  FAIL: Multi-component self-subtraction != [0]\n";
				std::cout << "    Result: " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Large value
		{
			std::vector<double> a = { 1.0e100 };
			std::vector<double> result = subtract_expansion(a, a);

			double result_val = sum_expansion(result);

			if (result_val != 0.0) {
				std::cout << "  FAIL: [1e100] - [1e100] != [0]\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Exact cancellation produces zero\n";
		}

		return nrOfFailedTests;
	}

	// Test subtraction identity: a - 0 = a
	int test_subtraction_zero_identity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: a - [0] = a (zero identity)\n";

		// Test case 1: Simple value
		{
			std::vector<double> a = { 15.0 };
			std::vector<double> zero = { 0.0 };
			std::vector<double> result = subtract_expansion(a, zero);

			double a_val = sum_expansion(a);
			double result_val = sum_expansion(result);

			if (std::abs(a_val - result_val) > 1.0e-14) {
				std::cout << "  FAIL: a - [0] != a\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Multi-component
		{
			std::vector<double> a = { 42.0, 2.1e-15, 1.05e-31 };
			std::vector<double> zero = { 0.0 };
			std::vector<double> result = subtract_expansion(a, zero);

			double a_val = sum_expansion(a);
			double result_val = sum_expansion(result);

			if (std::abs(a_val - result_val) > 1.0e-14) {
				std::cout << "  FAIL: Multi-component - [0] != original\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Zero identity holds\n";
		}

		return nrOfFailedTests;
	}

	// Test subtraction from zero: 0 - a = -a
	int test_subtraction_from_zero() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: [0] - a = -a (negation)\n";

		// Test case 1: Simple value
		{
			std::vector<double> zero = { 0.0 };
			std::vector<double> a = { 15.0 };
			std::vector<double> result = subtract_expansion(zero, a);

			double result_val = sum_expansion(result);
			double expected = -15.0;

			if (std::abs(result_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: [0] - [15] != [-15]\n";
				std::cout << "    Result: " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Multi-component
		{
			std::vector<double> zero = { 0.0 };
			std::vector<double> a = { 7.0, 3.5e-16 };
			std::vector<double> result = subtract_expansion(zero, a);
			std::vector<double> neg_a = negate_expansion(a);

			double result_val = sum_expansion(result);
			double neg_a_val = sum_expansion(neg_a);

			if (std::abs(result_val - neg_a_val) > 1.0e-14) {
				std::cout << "  FAIL: [0] - a != -a (multi-component)\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Subtraction from zero produces negation\n";
		}

		return nrOfFailedTests;
	}

	// Test inverse addition: (a + b) - b = a
	int test_subtraction_inverse_addition() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: (a + b) - b = a (inverse addition)\n";

		// Test case 1: Simple values
		{
			std::vector<double> a = { 10.0 };
			std::vector<double> b = { 5.0 };

			std::vector<double> sum = linear_expansion_sum(a, b);
			std::vector<double> result = subtract_expansion(sum, b);

			double a_val = sum_expansion(a);
			double result_val = sum_expansion(result);

			if (std::abs(a_val - result_val) > 1.0e-14) {
				std::cout << "  FAIL: ([10] + [5]) - [5] != [10]\n";
				std::cout << "    Expected: " << std::setprecision(17) << a_val << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: With precision components
		{
			std::vector<double> a = { 15.5, 7.75e-16 };
			std::vector<double> b = { 3.5, 1.75e-16 };

			std::vector<double> sum = linear_expansion_sum(a, b);
			std::vector<double> result = subtract_expansion(sum, b);

			double a_val = sum_expansion(a);
			double result_val = sum_expansion(result);

			if (std::abs(a_val - result_val) > 1.0e-14) {
				std::cout << "  FAIL: Multi-component inverse addition\n";
				print_expansion("a", a);
				print_expansion("sum", sum);
				print_expansion("result", result);
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Inverse addition property holds\n";
		}

		return nrOfFailedTests;
	}

	// Test catastrophic cancellation avoidance: (large + small) - large = small
	int test_subtraction_catastrophic_cancellation() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: catastrophic cancellation avoidance\n";

		// Test case 1: The bug that Phase 1 found - (1e20 + 1) - 1e20 = 1
		{
			std::vector<double> large = { 1.0e20 };
			std::vector<double> small = { 1.0 };

			std::vector<double> sum = linear_expansion_sum(large, small);
			std::vector<double> result = subtract_expansion(sum, large);

			double result_val = sum_expansion(result);
			double expected = 1.0;

			if (std::abs(result_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: (1e20 + 1) - 1e20 != 1\n";
				std::cout << "    Expected: " << std::setprecision(17) << expected << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << result_val << "\n";
				print_expansion("sum", sum);
				print_expansion("result", result);
				++nrOfFailedTests;
			}
			else {
				std::cout << "  (1e20 + 1) - 1e20 = 1 preserved! ✓\n";
			}
		}

		// Test case 2: Even more extreme - (1e100 + 1) - 1e100 = 1
		{
			std::vector<double> large = { 1.0e100 };
			std::vector<double> small = { 1.0 };

			std::vector<double> sum = linear_expansion_sum(large, small);
			std::vector<double> result = subtract_expansion(sum, large);

			double result_val = sum_expansion(result);
			double expected = 1.0;

			if (std::abs(result_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: (1e100 + 1) - 1e100 != 1\n";
				std::cout << "    Result: " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  (1e100 + 1) - 1e100 = 1 preserved! ✓\n";
			}
		}

		// Test case 3: Tiny component preservation - (1.0 + 1e-30) - 1.0 = 1e-30
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> tiny = { 1.0e-30 };

			std::vector<double> sum = linear_expansion_sum(one, tiny);
			std::vector<double> result = subtract_expansion(sum, one);

			double result_val = sum_expansion(result);
			double expected = 1.0e-30;

			if (std::abs(result_val - expected) > 1.0e-44) {
				std::cout << "  FAIL: (1 + 1e-30) - 1 != 1e-30\n";
				std::cout << "    Expected: " << std::setprecision(17) << expected << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  (1 + 1e-30) - 1 = 1e-30 preserved! ✓\n";
			}
		}

		// Test case 4: Multiple small components - (1e20 + 1e-5 + 1e-10) - 1e20 = 1e-5 + 1e-10
		{
			std::vector<double> large = { 1.0e20 };
			std::vector<double> small1 = { 1.0e-5 };
			std::vector<double> small2 = { 1.0e-10 };

			std::vector<double> sum1 = linear_expansion_sum(large, small1);
			std::vector<double> sum2 = linear_expansion_sum(sum1, small2);
			std::vector<double> result = subtract_expansion(sum2, large);

			double result_val = sum_expansion(result);
			double expected = 1.0e-5 + 1.0e-10;

			// Relative tolerance for very small values
			if (std::abs(result_val - expected) / expected > 1.0e-12) {
				std::cout << "  FAIL: Multiple small components not preserved\n";
				std::cout << "    Expected: " << std::setprecision(17) << expected << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  Multiple small components preserved! ✓\n";
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Catastrophic cancellation avoided\n";
		}

		return nrOfFailedTests;
	}

	// Test near-cancellation: (a + small) - a = small
	int test_subtraction_near_cancellation() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: near-cancellation (a + ε) - a = ε\n";

		// Test case 1: a=1, ε=1e-15
		{
			std::vector<double> a = { 1.0 };
			std::vector<double> epsilon = { 1.0e-15 };

			std::vector<double> sum = linear_expansion_sum(a, epsilon);
			std::vector<double> result = subtract_expansion(sum, a);

			double epsilon_val = sum_expansion(epsilon);
			double result_val = sum_expansion(result);

			// Note: Due to representation, we check value equality, not structural
			if (std::abs(result_val - epsilon_val) > 1.0e-29) {
				std::cout << "  FAIL: (1 + 1e-15) - 1 != 1e-15\n";
				std::cout << "    Expected: " << std::setprecision(17) << epsilon_val << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << result_val << "\n";
				print_expansion("epsilon", epsilon);
				print_expansion("sum", sum);
				print_expansion("result", result);
				++nrOfFailedTests;
			}
		}

		// Test case 2: a=100, ε=1e-10
		{
			std::vector<double> a = { 100.0 };
			std::vector<double> epsilon = { 1.0e-10 };

			std::vector<double> sum = linear_expansion_sum(a, epsilon);
			std::vector<double> result = subtract_expansion(sum, a);

			double epsilon_val = sum_expansion(epsilon);
			double result_val = sum_expansion(result);

			if (std::abs(result_val - epsilon_val) > 1.0e-24) {
				std::cout << "  FAIL: (100 + 1e-10) - 100 != 1e-10\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Multi-component a with small epsilon
		{
			std::vector<double> a = { 42.0, 2.1e-15 };
			std::vector<double> epsilon = { 1.0e-20 };

			std::vector<double> sum = linear_expansion_sum(a, epsilon);
			std::vector<double> result = subtract_expansion(sum, a);

			double epsilon_val = sum_expansion(epsilon);
			double result_val = sum_expansion(result);

			if (std::abs(result_val - epsilon_val) > 1.0e-34) {
				std::cout << "  FAIL: Multi-component near-cancellation\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Near-cancellation preserves small components\n";
		}

		return nrOfFailedTests;
	}

	// Test sign change: a - b where b > a gives negative result
	int test_subtraction_sign_change() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: a - b where b > a (sign change)\n";

		// Test case 1: [5] - [10] = [-5]
		{
			std::vector<double> a = { 5.0 };
			std::vector<double> b = { 10.0 };

			std::vector<double> result = subtract_expansion(a, b);
			double result_val = sum_expansion(result);
			double expected = -5.0;

			if (std::abs(result_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: [5] - [10] != [-5]\n";
				std::cout << "    Result: " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: [3.5] - [7.5] = [-4.0]
		{
			std::vector<double> a = { 3.5 };
			std::vector<double> b = { 7.5 };

			std::vector<double> result = subtract_expansion(a, b);
			double result_val = sum_expansion(result);
			double expected = -4.0;

			if (std::abs(result_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: [3.5] - [7.5] != [-4.0]\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Verify result is truly negative
		{
			std::vector<double> a = { 1.0 };
			std::vector<double> b = { 100.0 };

			std::vector<double> result = subtract_expansion(a, b);
			double result_val = sum_expansion(result);

			if (result_val >= 0.0) {
				std::cout << "  FAIL: [1] - [100] should be negative\n";
				std::cout << "    Result: " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Sign change handled correctly\n";
		}

		return nrOfFailedTests;
	}

	// Test associativity variations
	int test_subtraction_associativity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: associativity patterns\n";

		// Test case 1: (a - b) - c vs a - (b + c)
		{
			std::vector<double> a = { 20.0 };
			std::vector<double> b = { 5.0 };
			std::vector<double> c = { 3.0 };

			// Compute (a - b) - c
			std::vector<double> ab = subtract_expansion(a, b);
			std::vector<double> left = subtract_expansion(ab, c);

			// Compute a - (b + c)
			std::vector<double> bc = linear_expansion_sum(b, c);
			std::vector<double> right = subtract_expansion(a, bc);

			double left_val = sum_expansion(left);
			double right_val = sum_expansion(right);

			if (std::abs(left_val - right_val) > 1.0e-14) {
				std::cout << "  FAIL: (a - b) - c != a - (b + c)\n";
				std::cout << "    (20 - 5) - 3 = " << std::setprecision(17) << left_val << "\n";
				std::cout << "    20 - (5 + 3) = " << std::setprecision(17) << right_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: With precision components
		{
			std::vector<double> a = { 100.0, 5.0e-15 };
			std::vector<double> b = { 30.0, 1.5e-15 };
			std::vector<double> c = { 20.0, 1.0e-15 };

			std::vector<double> ab = subtract_expansion(a, b);
			std::vector<double> left = subtract_expansion(ab, c);

			std::vector<double> bc = linear_expansion_sum(b, c);
			std::vector<double> right = subtract_expansion(a, bc);

			double left_val = sum_expansion(left);
			double right_val = sum_expansion(right);

			if (std::abs(left_val - right_val) > 1.0e-13) {
				std::cout << "  FAIL: Multi-component associativity\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Associativity patterns hold\n";
		}

		return nrOfFailedTests;
	}

	// Test extreme scale differences
	int test_subtraction_extreme_scales() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing subtraction: extreme scale differences\n";

		// Test case 1: 1e100 - 1 (large - small)
		{
			std::vector<double> large = { 1.0e100 };
			std::vector<double> small = { 1.0 };

			std::vector<double> result = subtract_expansion(large, small);
			double result_val = sum_expansion(result);

			// Result should be very close to 1e100 (small doesn't affect it much)
			double expected = 1.0e100;
			if (std::abs(result_val - expected) / expected > 1.0e-14) {
				std::cout << "  FAIL: 1e100 - 1 computation error\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: 1.0 - 1e-100 (large - tiny)
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> tiny = { 1.0e-100 };

			std::vector<double> result = subtract_expansion(one, tiny);
			double result_val = sum_expansion(result);

			// Result should be very close to 1.0
			if (std::abs(result_val - 1.0) > 1.0e-14) {
				std::cout << "  FAIL: 1 - 1e-100 != 1\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Extreme scale differences handled correctly\n";
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
int main()
try {
	using namespace sw::universal;

	std::cout << "========================================================\n";
	std::cout << "Expansion Subtraction Tests (Identity-Based)\n";
	std::cout << "========================================================\n\n";

	int nrOfFailedTests = 0;

	nrOfFailedTests += test_subtraction_exact_cancellation();
	nrOfFailedTests += test_subtraction_zero_identity();
	nrOfFailedTests += test_subtraction_from_zero();
	nrOfFailedTests += test_subtraction_inverse_addition();
	nrOfFailedTests += test_subtraction_catastrophic_cancellation();
	nrOfFailedTests += test_subtraction_near_cancellation();
	nrOfFailedTests += test_subtraction_sign_change();
	nrOfFailedTests += test_subtraction_associativity();
	nrOfFailedTests += test_subtraction_extreme_scales();

	std::cout << "\n========================================================\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All subtraction tests passed\n";
	}
	std::cout << "========================================================\n";

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
