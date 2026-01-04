// addition.cpp: Tests for expansion addition operations
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
#include <iomanip>

namespace sw { namespace universal {

	// Test expansion addition with identity property
	int test_addition_identity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion addition identity: (a + b) - a = b\n";

		// Test case 1: Basic identity using properly constructed expansions
		{
			// Create valid expansion from TWO-SUM
			double a_hi, a_lo;
			two_sum(10.0, 1.0e-15, a_hi, a_lo);
			std::vector<double> a = { a_hi, a_lo };

			double b_hi, b_lo;
			two_sum(5.0, 5.0e-16, b_hi, b_lo);
			std::vector<double> b = { b_hi, b_lo };

			std::cout << "  a = {" << a[0] << ", " << a[1] << "}\n";
			std::cout << "  b = {" << b[0] << ", " << b[1] << "}\n";

			std::vector<double> sum = linear_expansion_sum(a, b);

			std::cout << "  sum has " << sum.size() << " components: ";
			for (auto v : sum) std::cout << v << " ";
			std::cout << "\n";

			// Negate a
			std::vector<double> neg_a = a;
			for (auto& v : neg_a) v = -v;

			std::vector<double> recovered_b = linear_expansion_sum(sum, neg_a);

			std::cout << "  Before compression: ";
			for (auto v : recovered_b) std::cout << v << " ";
			std::cout << "\n";

			// Compress to remove zeros
			recovered_b = compress_expansion(recovered_b, 0.0);

			std::cout << "  After compression: ";
			for (auto v : recovered_b) std::cout << v << " ";
			std::cout << "\n";

			// Check that recovered_b matches b (within tolerance)
			double b_sum = 0.0, recovered_sum = 0.0;
			for (auto v : b) b_sum += v;
			for (auto v : recovered_b) recovered_sum += v;

			std::cout << "  Expected: " << std::setprecision(17) << b_sum << ", Got: " << recovered_sum << "\n";
			std::cout << "  Difference: " << std::abs(b_sum - recovered_sum) << "\n";

			if (std::abs(b_sum - recovered_sum) > 1.0e-14) {
				std::cout << "  Test case 1 FAILED\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Identity with 3-component expansion
		{
			// Build a 3-component expansion using GROW-EXPANSION
			double c_hi, c_lo;
			two_sum(100.0, 1.0e-10, c_hi, c_lo);
			std::vector<double> a = {c_hi, c_lo};
			a = grow_expansion(a, 1.0e-20);  // Add third component

			two_sum(50.0, 5.0e-11, c_hi, c_lo);
			std::vector<double> b = {c_hi, c_lo};
			b = grow_expansion(b, 5.0e-21);

			std::vector<double> sum = linear_expansion_sum(a, b);

			std::vector<double> neg_a = a;
			for (auto& v : neg_a) v = -v;

			std::vector<double> recovered_b = linear_expansion_sum(sum, neg_a);
			recovered_b = compress_expansion(recovered_b, 0.0);

			double b_sum = 0.0, recovered_sum = 0.0;
			for (auto v : b) b_sum += v;
			for (auto v : recovered_b) recovered_sum += v;

			if (std::abs(b_sum - recovered_sum) > 1.0e-14) {
				std::cout << "  Test case 2 FAILED: diff = " << std::abs(b_sum - recovered_sum) << "\n";
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test expansion addition commutivity
	int test_addition_commutative() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion addition commutivity: a + b = b + a\n";

		// Test case 1: Basic commutivity
		{
			std::vector<double> a = { 7.0, 3.5e-16 };
			std::vector<double> b = { 3.0, 1.5e-16 };

			std::vector<double> sum1 = fast_expansion_sum(a, b);
			std::vector<double> sum2 = fast_expansion_sum(b, a);

			double sum1_val = 0.0, sum2_val = 0.0;
			for (auto v : sum1) sum1_val += v;
			for (auto v : sum2) sum2_val += v;

			if (std::abs(sum1_val - sum2_val) > 1.0e-14) {
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test expansion addition associativity
	int test_addition_associative() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion addition associativity: (a + b) + c ≈ a + (b + c)\n";

		// Test case 1: Three-way addition
		{
			std::vector<double> a = { 10.0, 1.0e-15 };
			std::vector<double> b = { 5.0, 5.0e-16 };
			std::vector<double> c = { 2.0, 2.0e-16 };

			// Compute (a + b) + c
			std::vector<double> ab = fast_expansion_sum(a, b);
			std::vector<double> abc1 = fast_expansion_sum(ab, c);

			// Compute a + (b + c)
			std::vector<double> bc = fast_expansion_sum(b, c);
			std::vector<double> abc2 = fast_expansion_sum(a, bc);

			double abc1_val = 0.0, abc2_val = 0.0;
			for (auto v : abc1) abc1_val += v;
			for (auto v : abc2) abc2_val += v;

			// Note: Exact associativity may not hold due to different
			// component ordering, but values should be very close
			if (std::abs(abc1_val - abc2_val) > 1.0e-13) {
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test addition with zeros
	int test_addition_with_zeros() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion addition with zeros\n";

		// Test case 1: Add zero expansion
		{
			std::vector<double> a = { 10.0, 1.0e-15 };
			std::vector<double> zero = { 0.0 };

			std::vector<double> sum = fast_expansion_sum(a, zero);

			double a_val = 0.0, sum_val = 0.0;
			for (auto v : a) a_val += v;
			for (auto v : sum) sum_val += v;

			if (std::abs(a_val - sum_val) > 1.0e-14) {
				++nrOfFailedTests;
			}
		}

		// Test case 2: Add to zero
		{
			std::vector<double> zero = { 0.0 };
			std::vector<double> b = { 7.0, 3.5e-16 };

			std::vector<double> sum = fast_expansion_sum(zero, b);

			double b_val = 0.0, sum_val = 0.0;
			for (auto v : b) b_val += v;
			for (auto v : sum) sum_val += v;

			if (std::abs(b_val - sum_val) > 1.0e-14) {
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test addition with cancellation
	int test_addition_cancellation() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion addition with cancellation\n";

		// Test case 1: Exact cancellation
		{
			std::vector<double> a = { 10.0, 1.0e-15 };
			std::vector<double> neg_a = { -10.0, -1.0e-15 };

			std::vector<double> sum = fast_expansion_sum(a, neg_a);
			sum = compress_expansion(sum, 0.0);

			// Sum should be zero (or very close)
			double sum_val = 0.0;
			for (auto v : sum) sum_val += v;

			if (std::abs(sum_val) > 1.0e-14) {
				++nrOfFailedTests;
			}
		}

		// Test case 2: Partial cancellation
		{
			std::vector<double> a = { 10.0, 1.0e-15 };
			std::vector<double> b = { -9.0, -0.9e-15 };

			std::vector<double> sum = fast_expansion_sum(a, b);
			sum = compress_expansion(sum, 0.0);

			// Sum should be approximately 1.0 + 0.1e-15
			double sum_val = 0.0;
			for (auto v : sum) sum_val += v;

			double expected = 1.0 + 0.1e-15;
			if (std::abs(sum_val - expected) > 1.0e-14) {
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

	std::cout << "Expansion Addition Arithmetic Tests\n";
	std::cout << "====================================\n\n";

	int nrOfFailedTests = 0;

	int failures = 0;
	failures = test_addition_identity();
	std::cout << "  Identity tests: " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	nrOfFailedTests += failures;

	failures = test_addition_commutative();
	std::cout << "  Commutative tests: " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	nrOfFailedTests += failures;

	failures = test_addition_associative();
	std::cout << "  Associative tests: " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	nrOfFailedTests += failures;

	failures = test_addition_with_zeros();
	std::cout << "  Zero tests: " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	nrOfFailedTests += failures;

	failures = test_addition_cancellation();
	std::cout << "  Cancellation tests: " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	nrOfFailedTests += failures;

	std::cout << "\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All addition arithmetic tests passed\n";
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
