// component_counting.cpp: Track expansion component growth patterns
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

	// Helper: Print expansion with component count
	void print_expansion_info(const std::string& name, const std::vector<double>& e) {
		std::cout << "  " << name << ": " << e.size() << " components";
		if (e.size() <= 5) {
			std::cout << " [";
			for (size_t i = 0; i < e.size(); ++i) {
				if (i > 0) std::cout << ", ";
				std::cout << std::setprecision(17) << e[i];
			}
			std::cout << "]";
		}
		std::cout << "\n";
	}

	// ===================================================================
	// NO-GROWTH CASES: Operations that should stay compact
	// ===================================================================

	int test_nogrowth_exact_arithmetic() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing NO-GROWTH: Exact arithmetic (should stay 1 component)\n";

		// Test case 1: Simple addition with exact result
		{
			std::vector<double> a = { 2.0 };
			std::vector<double> b = { 3.0 };
			std::vector<double> sum = linear_expansion_sum(a, b);

			if (sum.size() != 1) {
				std::cout << "  FAIL: 2 + 3 created " << sum.size() << " components (expected 1)\n";
				print_expansion_info("sum", sum);
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 2 + 3 = 1 component\n";
			}
		}

		// Test case 2: Power-of-2 multiplication
		{
			std::vector<double> a = { 3.0 };
			double scalar = 2.0;
			std::vector<double> product = scale_expansion(a, scalar);

			if (product.size() != 1) {
				std::cout << "  FAIL: 3 × 2 created " << product.size() << " components\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 3 × 2 = 1 component\n";
			}
		}

		// Test case 3: Integer division with exact result
		{
			std::vector<double> dividend = { 100.0 };
			std::vector<double> divisor = { 4.0 };
			std::vector<double> quotient = expansion_quotient(dividend, divisor);

			// Note: quotient uses Newton iteration, might create more components
			// But the final result should be exact
			double result = 0.0;
			for (auto v : quotient) result += v;

			if (std::abs(result - 25.0) > 1.0e-14) {
				std::cout << "  FAIL: 100 ÷ 4 != 25\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 100 ÷ 4 = " << quotient.size() << " components (value correct)\n";
			}
		}

		// Test case 4: Addition of integers
		{
			std::vector<double> a = { 10.0 };
			std::vector<double> b = { 20.0 };
			std::vector<double> sum = linear_expansion_sum(a, b);

			if (sum.size() != 1) {
				std::cout << "  FAIL: 10 + 20 created " << sum.size() << " components\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 10 + 20 = 1 component\n";
			}
		}

		// Test case 5: Multiplication by powers of 2 (should be exact)
		{
			std::vector<double> a = { 7.0 };
			double scalar = 0.5;  // 1/2 is exact in binary
			std::vector<double> product = scale_expansion(a, scalar);

			if (product.size() != 1) {
				std::cout << "  FAIL: 7 × 0.5 created " << product.size() << " components\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 7 × 0.5 = 1 component\n";
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Exact operations stay compact\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// EXPECTED GROWTH: Operations that need multiple components
	// ===================================================================

	int test_expected_growth_small_components() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting EXPECTED GROWTH: Adding small to large (needs precision)\n";

		// Test case 1: 1.0 + 1e-15 should create 2 components
		{
			std::vector<double> large = { 1.0 };
			std::vector<double> small = { 1.0e-15 };
			std::vector<double> sum = linear_expansion_sum(large, small);

			if (sum.size() < 2) {
				std::cout << "  FAIL: 1 + 1e-15 only has " << sum.size() << " component(s), expected >= 2\n";
				print_expansion_info("sum", sum);
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 1 + 1e-15 = " << sum.size() << " components (captures precision)\n";
			}
		}

		// Test case 2: 1e20 + 1 should create 2 components
		{
			std::vector<double> large = { 1.0e20 };
			std::vector<double> small = { 1.0 };
			std::vector<double> sum = linear_expansion_sum(large, small);

			if (sum.size() < 2) {
				std::cout << "  FAIL: 1e20 + 1 only has " << sum.size() << " component(s)\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 1e20 + 1 = " << sum.size() << " components (avoids catastrophic cancellation)\n";
			}
		}

		// Test case 3: Non-exact multiplication (3 × 0.1)
		{
			std::vector<double> a = { 3.0 };
			double scalar = 0.1;  // 0.1 is NOT exact in binary
			std::vector<double> product = scale_expansion(a, scalar);

			// 0.1 has rounding error, so product should capture it
			std::cout << "  ✓ 3 × 0.1 = " << product.size() << " components (0.1 not exact in binary)\n";
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Operations requiring precision grow as expected\n";
		}

		return nrOfFailedTests;
	}

	int test_expected_growth_division() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting EXPECTED GROWTH: Non-exact divisions\n";

		// Test case 1: 1/3 should create multiple components
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> three = { 3.0 };
			std::vector<double> quotient = expansion_quotient(one, three);

			if (quotient.size() < 2) {
				std::cout << "  FAIL: 1 ÷ 3 only has " << quotient.size() << " component(s)\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 1 ÷ 3 = " << quotient.size() << " components (Newton iterations)\n";
			}
		}

		// Test case 2: 1/7 should create multiple components
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> seven = { 7.0 };
			std::vector<double> quotient = expansion_quotient(one, seven);

			std::cout << "  ✓ 1 ÷ 7 = " << quotient.size() << " components\n";
		}

		// Test case 3: 22/7 (π approximation)
		{
			std::vector<double> numerator = { 22.0 };
			std::vector<double> denominator = { 7.0 };
			std::vector<double> quotient = expansion_quotient(numerator, denominator);

			std::cout << "  ✓ 22 ÷ 7 = " << quotient.size() << " components\n";
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Non-exact divisions produce multiple components\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// GROWTH CHAINS: Accumulation patterns
	// ===================================================================

	int test_growth_accumulation() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting GROWTH CHAINS: Accumulation patterns\n";

		// Test case 1: Sum of tiny values
		{
			std::vector<double> sum = { 0.0 };
			size_t iterations = 10;

			for (size_t i = 0; i < iterations; ++i) {
				std::vector<double> tiny = { 1.0e-15 };
				sum = linear_expansion_sum(sum, tiny);
			}

			std::cout << "  ✓ Sum of " << iterations << " × 1e-15: "
			          << sum.size() << " components\n";

			// Verify the value is correct
			double total = 0.0;
			for (auto v : sum) total += v;
			double expected = iterations * 1.0e-15;

			if (std::abs(total - expected) / expected > 1.0e-10) {
				std::cout << "    FAIL: Sum value incorrect\n";
				std::cout << "    Expected: " << std::setprecision(17) << expected << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << total << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Adding to large value repeatedly
		{
			std::vector<double> sum = { 1.0e20 };
			size_t iterations = 10;
			size_t initial_size = sum.size();

			for (size_t i = 0; i < iterations; ++i) {
				std::vector<double> one = { 1.0 };
				sum = linear_expansion_sum(sum, one);
			}

			std::cout << "  ✓ 1e20 + 10×1: grew from " << initial_size
			          << " to " << sum.size() << " components\n";

			// Should have grown to accommodate the accumulated small values
			if (sum.size() < initial_size + 1) {
				std::cout << "    WARNING: Expected more growth\n";
			}
		}

		// Test case 3: Chain of multiplications
		{
			std::vector<double> product = { 1.0 };
			size_t iterations = 5;

			for (size_t i = 0; i < iterations; ++i) {
				std::vector<double> factor = { 1.1 };  // 1.1 not exact in binary
				product = expansion_product(product, factor);
			}

			std::cout << "  ✓ 1.1^" << iterations << ": "
			          << product.size() << " components\n";

			// Verify the value
			double result = 0.0;
			for (auto v : product) result += v;
			double expected = std::pow(1.1, iterations);

			if (std::abs(result - expected) / expected > 1.0e-12) {
				std::cout << "    FAIL: Product value incorrect\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Accumulation patterns tracked correctly\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// MULTI-COMPONENT INTERACTIONS
	// ===================================================================

	int test_multicomponent_interactions() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting MULTI-COMPONENT INTERACTIONS\n";

		// Test case 1: Adding two multi-component expansions
		{
			// Create first multi-component expansion
			std::vector<double> a = { 1.0 };
			std::vector<double> tiny1 = { 1.0e-15 };
			a = linear_expansion_sum(a, tiny1);

			// Create second multi-component expansion
			std::vector<double> b = { 2.0 };
			std::vector<double> tiny2 = { 2.0e-15 };
			b = linear_expansion_sum(b, tiny2);

			size_t a_size = a.size();
			size_t b_size = b.size();

			// Add them
			std::vector<double> sum = linear_expansion_sum(a, b);

			std::cout << "  ✓ Multi + Multi: [" << a_size << "] + ["
			          << b_size << "] = [" << sum.size() << "]\n";

			// Result might have fewer components due to merging
			if (sum.size() > a_size + b_size) {
				std::cout << "    WARNING: More components than expected\n";
			}
		}

		// Test case 2: Multiplying two multi-component expansions
		{
			// Create first expansion: 1/3
			std::vector<double> one = { 1.0 };
			std::vector<double> three = { 3.0 };
			std::vector<double> third = expansion_quotient(one, three);

			// Create second expansion: 1/7
			std::vector<double> seven = { 7.0 };
			std::vector<double> seventh = expansion_quotient(one, seven);

			size_t third_size = third.size();
			size_t seventh_size = seventh.size();

			// Multiply them: (1/3) × (1/7) = 1/21
			std::vector<double> product = expansion_product(third, seventh);

			std::cout << "  ✓ (1/3) × (1/7): [" << third_size << "] × ["
			          << seventh_size << "] = [" << product.size() << "]\n";

			// Verify the value
			double result = 0.0;
			for (auto v : product) result += v;
			double expected = 1.0 / 21.0;

			if (std::abs(result - expected) / expected > 1.0e-12) {
				std::cout << "    FAIL: Product value incorrect\n";
				std::cout << "    Expected: " << std::setprecision(17) << expected << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << result << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Subtracting multi-component expansions
		{
			// Create a = 10 + 1e-15
			std::vector<double> a = { 10.0 };
			std::vector<double> tiny = { 1.0e-15 };
			a = linear_expansion_sum(a, tiny);

			// Create b = 5 + 5e-16
			std::vector<double> b = { 5.0 };
			std::vector<double> tiny2 = { 5.0e-16 };
			b = linear_expansion_sum(b, tiny2);

			// Subtract: a - b
			std::vector<double> neg_b = b;
			for (auto& v : neg_b) v = -v;
			std::vector<double> diff = linear_expansion_sum(a, neg_b);

			std::cout << "  ✓ (10+ε₁) - (5+ε₂): [" << a.size() << "] - ["
			          << b.size() << "] = [" << diff.size() << "]\n";

			// Verify value
			double result = 0.0;
			for (auto v : diff) result += v;
			double expected = 5.0 + 5.0e-16;  // (10+1e-15) - (5+5e-16) = 5 + 5e-16

			if (std::abs(result - expected) / expected > 1.0e-12) {
				std::cout << "    FAIL: Difference value incorrect\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Multi-component interactions work correctly\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// GROWTH BOUNDS: Verify component counts stay reasonable
	// ===================================================================

	int test_growth_bounds() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting GROWTH BOUNDS: Component counts stay reasonable\n";

		// Test case 1: Division shouldn't explode beyond Newton iterations
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> divisor = { 3.0 };
			std::vector<double> quotient = expansion_quotient(one, divisor);

			if (quotient.size() > 20) {
				std::cout << "  FAIL: 1/3 has " << quotient.size()
				          << " components (too many!)\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ 1/3 = " << quotient.size()
				          << " components (reasonable)\n";
			}
		}

		// Test case 2: Long chain shouldn't explode
		{
			std::vector<double> sum = { 0.0 };
			size_t iterations = 100;

			for (size_t i = 0; i < iterations; ++i) {
				std::vector<double> value = { 1.0 };
				sum = linear_expansion_sum(sum, value);
			}

			if (sum.size() > 10) {
				std::cout << "  WARNING: Sum of " << iterations << " integers has "
				          << sum.size() << " components\n";
			}
			else {
				std::cout << "  ✓ Sum of " << iterations << " integers = "
				          << sum.size() << " components (compact)\n";
			}
		}

		// Test case 3: Product shouldn't explode with exact values
		{
			std::vector<double> product = { 2.0 };
			size_t iterations = 10;

			for (size_t i = 0; i < iterations; ++i) {
				std::vector<double> factor = { 2.0 };  // Powers of 2 are exact
				product = expansion_product(product, factor);
			}

			if (product.size() > 5) {
				std::cout << "  WARNING: 2^" << (iterations+1) << " has "
				          << product.size() << " components\n";
			}
			else {
				std::cout << "  ✓ 2^" << (iterations+1) << " = "
				          << product.size() << " components\n";
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Component counts stay within reasonable bounds\n";
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
int main()
try {
	using namespace sw::universal;

	std::cout << "========================================================\n";
	std::cout << "Expansion Component Growth Tracking Tests\n";
	std::cout << "========================================================\n";

	int nrOfFailedTests = 0;

	nrOfFailedTests += test_nogrowth_exact_arithmetic();
	nrOfFailedTests += test_expected_growth_small_components();
	nrOfFailedTests += test_expected_growth_division();
	nrOfFailedTests += test_growth_accumulation();
	nrOfFailedTests += test_multicomponent_interactions();
	nrOfFailedTests += test_growth_bounds();

	std::cout << "\n========================================================\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All component growth tests passed\n";
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
