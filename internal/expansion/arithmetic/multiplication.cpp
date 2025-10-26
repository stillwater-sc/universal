// multiplication.cpp: Tests for expansion scalar multiplication operations
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

	// Test scalar multiplication correctness
	int test_scalar_multiplication() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing scalar multiplication correctness\n";

		// Test case 1: Multiply by integer
		{
			std::vector<double> e = { 3.0, 1.5e-16 };
			double b = 5.0;

			std::vector<double> h = scale_expansion(e, b);

			double e_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : h) h_sum += v;

			double expected = e_sum * b;
			if (std::abs(h_sum - expected) > 1.0e-13) {
				++nrOfFailedTests;
			}
		}

		// Test case 2: Multiply by fraction
		{
			std::vector<double> e = { 10.0, 5.0e-16 };
			double b = 0.125;  // 1/8

			std::vector<double> h = scale_expansion(e, b);

			double e_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : h) h_sum += v;

			double expected = e_sum * b;
			if (std::abs(h_sum - expected) > 1.0e-14) {
				++nrOfFailedTests;
			}
		}

		// Test case 3: Multiply by negative
		{
			std::vector<double> e = { 7.0, 3.5e-16 };
			double b = -2.5;

			std::vector<double> h = scale_expansion(e, b);

			double e_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : h) h_sum += v;

			double expected = e_sum * b;
			if (std::abs(h_sum - expected) > 1.0e-13) {
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test multiplication identity
	int test_multiplication_identity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing multiplication identity: e * 1.0 = e\n";

		// Test case 1: Identity
		{
			std::vector<double> e = { 10.0, 1.0e-15, 1.0e-30 };
			double b = 1.0;

			std::vector<double> h = scale_expansion(e, b);

			// Should return unchanged
			if (h.size() != e.size()) ++nrOfFailedTests;
			for (size_t i = 0; i < e.size(); ++i) {
				if (h[i] != e[i]) ++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test multiplication by zero
	int test_multiplication_by_zero() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing multiplication by zero: e * 0.0 = 0.0\n";

		// Test case 1: Zero
		{
			std::vector<double> e = { 100.0, 10.0, 1.0 };
			double b = 0.0;

			std::vector<double> h = scale_expansion(e, b);

			if (h.size() != 1) ++nrOfFailedTests;
			if (h[0] != 0.0) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test multiplication by -1 (negation)
	int test_multiplication_negation() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing multiplication by -1: e * (-1) = -e\n";

		// Test case 1: Negation
		{
			std::vector<double> e = { 5.0, 2.5e-16, 1.25e-32 };
			double b = -1.0;

			std::vector<double> h = scale_expansion(e, b);

			if (h.size() != e.size()) ++nrOfFailedTests;
			for (size_t i = 0; i < e.size(); ++i) {
				if (h[i] != -e[i]) ++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test distributive property: (a + b) * c = a*c + b*c
	int test_distributive_property() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing distributive property: (a + b) * c ≈ a*c + b*c\n";

		// Test case 1: Distributivity
		{
			std::vector<double> a = { 10.0, 1.0e-15 };
			std::vector<double> b = { 5.0, 5.0e-16 };
			double c = 2.5;

			// Compute (a + b) * c
			std::vector<double> sum = fast_expansion_sum(a, b);
			std::vector<double> left = scale_expansion(sum, c);

			// Compute a*c + b*c
			std::vector<double> ac = scale_expansion(a, c);
			std::vector<double> bc = scale_expansion(b, c);
			std::vector<double> right = fast_expansion_sum(ac, bc);

			double left_val = 0.0, right_val = 0.0;
			for (auto v : left) left_val += v;
			for (auto v : right) right_val += v;

			if (std::abs(left_val - right_val) > 1.0e-12) {
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test multiplication precision preservation
	int test_multiplication_precision() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing multiplication precision preservation\n";

		// Test case 1: High precision multiplication
		{
			// Create expansion with high precision tail
			std::vector<double> e = { 1.0, 1.0e-20, 1.0e-40 };
			double b = 3.0;

			std::vector<double> h = scale_expansion(e, b);

			// Verify all precision is preserved (h should have components for each)
			// After multiplication and TWO-PROD, we expect products and errors
			// The sum of h should equal 3 * sum of e
			double e_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : h) h_sum += v;

			double expected = e_sum * b;
			// Use tighter tolerance for simple integer multiplier
			if (std::abs(h_sum - expected) > 1.0e-14) {
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

	std::cout << "Expansion Scalar Multiplication Tests\n";
	std::cout << "======================================\n\n";

	int nrOfFailedTests = 0;

	nrOfFailedTests += test_scalar_multiplication();
	nrOfFailedTests += test_multiplication_identity();
	nrOfFailedTests += test_multiplication_by_zero();
	nrOfFailedTests += test_multiplication_negation();
	nrOfFailedTests += test_distributive_property();
	nrOfFailedTests += test_multiplication_precision();

	std::cout << "\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All scalar multiplication tests passed\n";
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
