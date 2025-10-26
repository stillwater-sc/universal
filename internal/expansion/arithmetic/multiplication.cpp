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

	// ===================================================================
	// EXPANSION PRODUCT TESTS (expansion × expansion)
	// ===================================================================

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

	// Test multiplicative identity: e × [1] = e
	int test_product_multiplicative_identity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_product: e × [1] = e (multiplicative identity)\n";

		// Test case 1: Simple expansion
		{
			std::vector<double> e = { 3.0, 1.5e-16 };
			std::vector<double> one = { 1.0 };

			std::vector<double> result = expansion_product(e, one);

			double e_val = sum_expansion(e);
			double result_val = sum_expansion(result);

			if (std::abs(e_val - result_val) > 1.0e-14) {
				std::cout << "  FAIL: e × [1] != e\n";
				print_expansion("e", e);
				print_expansion("result", result);
				++nrOfFailedTests;
			}
		}

		// Test case 2: Multi-component expansion
		{
			std::vector<double> e = { 10.0, 5.0e-16, 2.5e-32 };
			std::vector<double> one = { 1.0 };

			std::vector<double> result = expansion_product(e, one);

			double e_val = sum_expansion(e);
			double result_val = sum_expansion(result);

			if (std::abs(e_val - result_val) > 1.0e-14) {
				std::cout << "  FAIL: multi-component × [1]\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Multiplicative identity holds\n";
		}

		return nrOfFailedTests;
	}

	// Test zero property: e × [0] = [0]
	int test_product_zero_property() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_product: e × [0] = [0] (zero property)\n";

		// Test case 1: Any expansion times zero
		{
			std::vector<double> e = { 100.0, 10.0, 1.0 };
			std::vector<double> zero = { 0.0 };

			std::vector<double> result = expansion_product(e, zero);

			double result_val = sum_expansion(result);

			if (result_val != 0.0) {
				std::cout << "  FAIL: e × [0] != [0]\n";
				print_expansion("result", result);
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Zero property holds\n";
		}

		return nrOfFailedTests;
	}

	// Test commutivity: e × f = f × e
	int test_product_commutivity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_product: e × f = f × e (commutivity)\n";

		// Test case 1: Two simple expansions
		{
			std::vector<double> e = { 3.0, 1.5e-16 };
			std::vector<double> f = { 5.0, 2.5e-16 };

			std::vector<double> ef = expansion_product(e, f);
			std::vector<double> fe = expansion_product(f, e);

			double ef_val = sum_expansion(ef);
			double fe_val = sum_expansion(fe);

			if (std::abs(ef_val - fe_val) > 1.0e-13) {
				std::cout << "  FAIL: e × f != f × e\n";
				std::cout << "    e × f = " << std::setprecision(17) << ef_val << "\n";
				std::cout << "    f × e = " << std::setprecision(17) << fe_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Different sizes
		{
			std::vector<double> e = { 7.0 };
			std::vector<double> f = { 3.0, 1.5e-16, 7.5e-33 };

			std::vector<double> ef = expansion_product(e, f);
			std::vector<double> fe = expansion_product(f, e);

			double ef_val = sum_expansion(ef);
			double fe_val = sum_expansion(fe);

			if (std::abs(ef_val - fe_val) > 1.0e-13) {
				std::cout << "  FAIL: Different size commutivity\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Commutivity holds\n";
		}

		return nrOfFailedTests;
	}

	// Test associativity: (e × f) × g = e × (f × g)
	int test_product_associativity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_product: (e × f) × g = e × (f × g) (associativity)\n";

		// Test case 1: Three expansions
		{
			std::vector<double> e = { 2.0 };
			std::vector<double> f = { 3.0 };
			std::vector<double> g = { 5.0 };

			// Compute (e × f) × g
			std::vector<double> ef = expansion_product(e, f);
			std::vector<double> efg_left = expansion_product(ef, g);

			// Compute e × (f × g)
			std::vector<double> fg = expansion_product(f, g);
			std::vector<double> efg_right = expansion_product(e, fg);

			double left_val = sum_expansion(efg_left);
			double right_val = sum_expansion(efg_right);

			if (std::abs(left_val - right_val) > 1.0e-13) {
				std::cout << "  FAIL: Associativity failed\n";
				std::cout << "    (e × f) × g = " << std::setprecision(17) << left_val << "\n";
				std::cout << "    e × (f × g) = " << std::setprecision(17) << right_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: With components
		{
			std::vector<double> e = { 2.0, 1.0e-16 };
			std::vector<double> f = { 3.0, 1.5e-16 };
			std::vector<double> g = { 5.0, 2.5e-16 };

			std::vector<double> ef = expansion_product(e, f);
			std::vector<double> efg_left = expansion_product(ef, g);

			std::vector<double> fg = expansion_product(f, g);
			std::vector<double> efg_right = expansion_product(e, fg);

			double left_val = sum_expansion(efg_left);
			double right_val = sum_expansion(efg_right);

			if (std::abs(left_val - right_val) > 1.0e-12) {
				std::cout << "  FAIL: Multi-component associativity\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Associativity holds\n";
		}

		return nrOfFailedTests;
	}

	// Test distributivity: e × (f + g) = (e × f) + (e × g)
	int test_product_distributivity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_product: e × (f + g) = (e × f) + (e × g) (distributivity)\n";

		// Test case 1: Simple values
		{
			std::vector<double> e = { 2.0 };
			std::vector<double> f = { 3.0 };
			std::vector<double> g = { 5.0 };

			// Compute e × (f + g)
			std::vector<double> fg_sum = linear_expansion_sum(f, g);
			std::vector<double> left = expansion_product(e, fg_sum);

			// Compute (e × f) + (e × g)
			std::vector<double> ef = expansion_product(e, f);
			std::vector<double> eg = expansion_product(e, g);
			std::vector<double> right = linear_expansion_sum(ef, eg);

			double left_val = sum_expansion(left);
			double right_val = sum_expansion(right);

			if (std::abs(left_val - right_val) > 1.0e-14) {
				std::cout << "  FAIL: Distributivity failed\n";
				std::cout << "    e × (f + g) = " << std::setprecision(17) << left_val << "\n";
				std::cout << "    (e×f)+(e×g) = " << std::setprecision(17) << right_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: With precision components
		{
			std::vector<double> e = { 1.5 };
			std::vector<double> f = { 2.3, 1.15e-16 };
			std::vector<double> g = { 4.7, 2.35e-16 };

			std::vector<double> fg_sum = linear_expansion_sum(f, g);
			std::vector<double> left = expansion_product(e, fg_sum);

			std::vector<double> ef = expansion_product(e, f);
			std::vector<double> eg = expansion_product(e, g);
			std::vector<double> right = linear_expansion_sum(ef, eg);

			double left_val = sum_expansion(left);
			double right_val = sum_expansion(right);

			if (std::abs(left_val - right_val) > 1.0e-13) {
				std::cout << "  FAIL: Multi-component distributivity\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Distributivity holds\n";
		}

		return nrOfFailedTests;
	}

	// Test product vs scale_expansion: e × [scalar] should match scale_expansion
	int test_product_vs_scale() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_product vs scale_expansion consistency\n";

		// Test case 1: Product with single-component should match scale
		{
			std::vector<double> e = { 3.0, 1.5e-16, 7.5e-33 };
			double scalar = 5.0;
			std::vector<double> scalar_exp = { scalar };

			std::vector<double> product = expansion_product(e, scalar_exp);
			std::vector<double> scaled = scale_expansion(e, scalar);

			double product_val = sum_expansion(product);
			double scaled_val = sum_expansion(scaled);

			if (std::abs(product_val - scaled_val) > 1.0e-14) {
				std::cout << "  FAIL: product vs scale mismatch\n";
				std::cout << "    product = " << std::setprecision(17) << product_val << "\n";
				std::cout << "    scale   = " << std::setprecision(17) << scaled_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Non-power-of-2 scalar
		{
			std::vector<double> e = { 7.0, 3.5e-16 };
			double scalar = 1.5;
			std::vector<double> scalar_exp = { scalar };

			std::vector<double> product = expansion_product(e, scalar_exp);
			std::vector<double> scaled = scale_expansion(e, scalar);

			double product_val = sum_expansion(product);
			double scaled_val = sum_expansion(scaled);

			if (std::abs(product_val - scaled_val) > 1.0e-14) {
				std::cout << "  FAIL: Non-power-of-2 mismatch\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Product consistent with scale_expansion\n";
		}

		return nrOfFailedTests;
	}

	// Test extreme scale products
	int test_product_extreme_scales() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_product with extreme scales\n";

		// Test case 1: Large × small = 1
		{
			std::vector<double> large = { 1.0e20 };
			std::vector<double> small = { 1.0e-20 };

			std::vector<double> result = expansion_product(large, small);
			double result_val = sum_expansion(result);
			double expected = 1.0;

			if (std::abs(result_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: 1e20 × 1e-20 != 1.0\n";
				std::cout << "    Result: " << std::setprecision(17) << result_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Very large product
		{
			std::vector<double> a = { 1.0e100 };
			std::vector<double> b = { 2.0 };

			std::vector<double> result = expansion_product(a, b);
			double result_val = sum_expansion(result);
			double expected = 2.0e100;

			if (std::abs(result_val - expected) > 1.0e85) {
				std::cout << "  FAIL: 1e100 × 2 failed\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Extreme scale products work correctly\n";
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
int main()
try {
	using namespace sw::universal;

	std::cout << "========================================================\n";
	std::cout << "Expansion Multiplication Tests\n";
	std::cout << "========================================================\n\n";

	int nrOfFailedTests = 0;

	std::cout << "SCALAR MULTIPLICATION (scale_expansion)\n";
	std::cout << "========================================\n";
	nrOfFailedTests += test_scalar_multiplication();
	nrOfFailedTests += test_multiplication_identity();
	nrOfFailedTests += test_multiplication_by_zero();
	nrOfFailedTests += test_multiplication_negation();
	nrOfFailedTests += test_distributive_property();
	nrOfFailedTests += test_multiplication_precision();

	std::cout << "\nEXPANSION PRODUCT (expansion_product)\n";
	std::cout << "======================================\n";
	nrOfFailedTests += test_product_multiplicative_identity();
	nrOfFailedTests += test_product_zero_property();
	nrOfFailedTests += test_product_commutivity();
	nrOfFailedTests += test_product_associativity();
	nrOfFailedTests += test_product_distributivity();
	nrOfFailedTests += test_product_vs_scale();
	nrOfFailedTests += test_product_extreme_scales();

	std::cout << "\n========================================================\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All multiplication tests passed\n";
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
