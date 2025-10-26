// division.cpp: Identity-based tests for expansion division operations
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

	// ===================================================================
	// RECIPROCAL TESTS (expansion_reciprocal)
	// ===================================================================

	// Test reciprocal of [1]: reciprocal([1]) = [1]
	int test_reciprocal_of_one() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_reciprocal: reciprocal([1]) = [1]\n";

		std::vector<double> one = { 1.0 };
		std::vector<double> recip = expansion_reciprocal(one);

		double recip_val = sum_expansion(recip);
		double expected = 1.0;

		if (std::abs(recip_val - expected) > 1.0e-14) {
			std::cout << "  FAIL: reciprocal([1]) != [1]\n";
			print_expansion("reciprocal", recip);
			std::cout << "    Value: " << std::setprecision(17) << recip_val << "\n";
			++nrOfFailedTests;
		}
		else {
			std::cout << "  PASS: reciprocal([1]) = [1]\n";
		}

		return nrOfFailedTests;
	}

	// Test reciprocal of simple values: reciprocal([2]) = [0.5]
	int test_reciprocal_simple_values() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_reciprocal: simple values\n";

		// Test case 1: reciprocal([2]) = [0.5]
		{
			std::vector<double> two = { 2.0 };
			std::vector<double> recip = expansion_reciprocal(two);

			double recip_val = sum_expansion(recip);
			double expected = 0.5;

			if (std::abs(recip_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: reciprocal([2]) != [0.5]\n";
				std::cout << "    Got: " << std::setprecision(17) << recip_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: reciprocal([4]) = [0.25]
		{
			std::vector<double> four = { 4.0 };
			std::vector<double> recip = expansion_reciprocal(four);

			double recip_val = sum_expansion(recip);
			double expected = 0.25;

			if (std::abs(recip_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: reciprocal([4]) != [0.25]\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: reciprocal([10]) = [0.1]
		{
			std::vector<double> ten = { 10.0 };
			std::vector<double> recip = expansion_reciprocal(ten);

			double recip_val = sum_expansion(recip);
			double expected = 0.1;

			if (std::abs(recip_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: reciprocal([10]) != [0.1]\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Simple reciprocals correct\n";
		}

		return nrOfFailedTests;
	}

	// Test multiplicative inverse: e × reciprocal(e) = [1]
	int test_reciprocal_multiplicative_inverse() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_reciprocal: e × reciprocal(e) = [1] (multiplicative inverse)\n";

		// Test case 1: Simple value
		{
			std::vector<double> e = { 3.0 };
			std::vector<double> recip = expansion_reciprocal(e);
			std::vector<double> product = expansion_product(e, recip);

			double product_val = sum_expansion(product);
			double expected = 1.0;

			if (std::abs(product_val - expected) > 1.0e-13) {
				std::cout << "  FAIL: [3] × reciprocal([3]) != [1]\n";
				print_expansion("reciprocal", recip);
				print_expansion("product", product);
				std::cout << "    Product value: " << std::setprecision(17) << product_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Non-power-of-2 value
		{
			std::vector<double> e = { 7.0 };
			std::vector<double> recip = expansion_reciprocal(e);
			std::vector<double> product = expansion_product(e, recip);

			double product_val = sum_expansion(product);
			double expected = 1.0;

			if (std::abs(product_val - expected) > 1.0e-13) {
				std::cout << "  FAIL: [7] × reciprocal([7]) != [1]\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Large value
		{
			std::vector<double> e = { 1.0e10 };
			std::vector<double> recip = expansion_reciprocal(e);
			std::vector<double> product = expansion_product(e, recip);

			double product_val = sum_expansion(product);
			double expected = 1.0;

			if (std::abs(product_val - expected) > 1.0e-12) {
				std::cout << "  FAIL: [1e10] × reciprocal([1e10]) != [1]\n";
				++nrOfFailedTests;
			}
		}

		// Test case 4: Multi-component expansion
		{
			std::vector<double> e = { 5.0, 2.5e-16 };
			std::vector<double> recip = expansion_reciprocal(e);
			std::vector<double> product = expansion_product(e, recip);

			double product_val = sum_expansion(product);
			double expected = 1.0;

			// Newton iteration may have slightly larger error for multi-component
			if (std::abs(product_val - expected) > 1.0e-12) {
				std::cout << "  FAIL: Multi-component × reciprocal != [1]\n";
				print_expansion("e", e);
				print_expansion("reciprocal", recip);
				print_expansion("product", product);
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Multiplicative inverse holds (within Newton precision)\n";
		}

		return nrOfFailedTests;
	}

	// Test double reciprocal: reciprocal(reciprocal(e)) ≈ e
	int test_reciprocal_double_reciprocal() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_reciprocal: reciprocal(reciprocal(e)) ≈ e\n";

		// Test case 1: Simple value
		{
			std::vector<double> e = { 5.0 };
			std::vector<double> recip1 = expansion_reciprocal(e);
			std::vector<double> recip2 = expansion_reciprocal(recip1);

			double e_val = sum_expansion(e);
			double recip2_val = sum_expansion(recip2);

			if (std::abs(e_val - recip2_val) > 1.0e-13) {
				std::cout << "  FAIL: reciprocal(reciprocal([5])) != [5]\n";
				std::cout << "    Expected: " << std::setprecision(17) << e_val << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << recip2_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Non-power-of-2
		{
			std::vector<double> e = { 3.0 };
			std::vector<double> recip1 = expansion_reciprocal(e);
			std::vector<double> recip2 = expansion_reciprocal(recip1);

			double e_val = sum_expansion(e);
			double recip2_val = sum_expansion(recip2);

			if (std::abs(e_val - recip2_val) > 1.0e-13) {
				std::cout << "  FAIL: Double reciprocal of [3]\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Double reciprocal recovers original\n";
		}

		return nrOfFailedTests;
	}

	// Test reciprocal with extreme scales
	int test_reciprocal_extreme_scales() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_reciprocal: extreme scales\n";

		// Test case 1: Very large value
		{
			std::vector<double> large = { 1.0e100 };
			std::vector<double> recip = expansion_reciprocal(large);

			double recip_val = sum_expansion(recip);
			double expected = 1.0e-100;

			// Large relative tolerance for extreme values
			if (std::abs(recip_val - expected) / expected > 1.0e-13) {
				std::cout << "  FAIL: reciprocal([1e100]) != [1e-100]\n";
				std::cout << "    Expected: " << std::setprecision(17) << expected << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << recip_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Very small value
		{
			std::vector<double> small = { 1.0e-20 };
			std::vector<double> recip = expansion_reciprocal(small);

			double recip_val = sum_expansion(recip);
			double expected = 1.0e20;

			if (std::abs(recip_val - expected) / expected > 1.0e-13) {
				std::cout << "  FAIL: reciprocal([1e-20]) != [1e20]\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Verify e × recip = 1 for extreme scale
		{
			std::vector<double> e = { 1.0e50 };
			std::vector<double> recip = expansion_reciprocal(e);
			std::vector<double> product = expansion_product(e, recip);

			double product_val = sum_expansion(product);
			double expected = 1.0;

			if (std::abs(product_val - expected) > 1.0e-12) {
				std::cout << "  FAIL: [1e50] × reciprocal([1e50]) != [1]\n";
				std::cout << "    Product: " << std::setprecision(17) << product_val << "\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Extreme scale reciprocals work correctly\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// QUOTIENT TESTS (expansion_quotient)
	// ===================================================================

	// Test division identity: e ÷ [1] = e
	int test_quotient_division_identity() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_quotient: e ÷ [1] = e (division identity)\n";

		// Test case 1: Simple expansion
		{
			std::vector<double> e = { 15.0 };
			std::vector<double> one = { 1.0 };

			std::vector<double> quotient = expansion_quotient(e, one);

			double e_val = sum_expansion(e);
			double quot_val = sum_expansion(quotient);

			if (std::abs(e_val - quot_val) > 1.0e-14) {
				std::cout << "  FAIL: e ÷ [1] != e\n";
				print_expansion("e", e);
				print_expansion("quotient", quotient);
				++nrOfFailedTests;
			}
		}

		// Test case 2: Multi-component
		{
			std::vector<double> e = { 42.0, 2.1e-15 };
			std::vector<double> one = { 1.0 };

			std::vector<double> quotient = expansion_quotient(e, one);

			double e_val = sum_expansion(e);
			double quot_val = sum_expansion(quotient);

			if (std::abs(e_val - quot_val) > 1.0e-13) {
				std::cout << "  FAIL: Multi-component ÷ [1] != e\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Division identity holds\n";
		}

		return nrOfFailedTests;
	}

	// Test self-division: e ÷ e = [1]
	int test_quotient_self_division() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_quotient: e ÷ e = [1] (self-division)\n";

		// Test case 1: Simple value
		{
			std::vector<double> e = { 42.0 };
			std::vector<double> quotient = expansion_quotient(e, e);

			double quot_val = sum_expansion(quotient);
			double expected = 1.0;

			if (std::abs(quot_val - expected) > 1.0e-13) {
				std::cout << "  FAIL: [42] ÷ [42] != [1]\n";
				std::cout << "    Got: " << std::setprecision(17) << quot_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Non-power-of-2
		{
			std::vector<double> e = { 7.0 };
			std::vector<double> quotient = expansion_quotient(e, e);

			double quot_val = sum_expansion(quotient);
			double expected = 1.0;

			if (std::abs(quot_val - expected) > 1.0e-13) {
				std::cout << "  FAIL: [7] ÷ [7] != [1]\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Multi-component
		{
			std::vector<double> e = { 15.5, 7.75e-16 };
			std::vector<double> quotient = expansion_quotient(e, e);

			double quot_val = sum_expansion(quotient);
			double expected = 1.0;

			if (std::abs(quot_val - expected) > 1.0e-12) {
				std::cout << "  FAIL: Multi-component self-division\n";
				print_expansion("e", e);
				print_expansion("quotient", quotient);
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Self-division produces [1]\n";
		}

		return nrOfFailedTests;
	}

	// Test inverse property: (e ÷ f) × f ≈ e
	int test_quotient_inverse_property() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_quotient: (e ÷ f) × f ≈ e (inverse property)\n";

		// Test case 1: Simple values
		{
			std::vector<double> e = { 15.0 };
			std::vector<double> f = { 3.0 };

			std::vector<double> quotient = expansion_quotient(e, f);
			std::vector<double> recovered = expansion_product(quotient, f);

			double e_val = sum_expansion(e);
			double recovered_val = sum_expansion(recovered);

			if (std::abs(e_val - recovered_val) > 1.0e-13) {
				std::cout << "  FAIL: ([15] ÷ [3]) × [3] != [15]\n";
				std::cout << "    Expected: " << std::setprecision(17) << e_val << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << recovered_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Non-power-of-2 divisor
		{
			std::vector<double> e = { 15.5 };
			std::vector<double> f = { 3.5 };

			std::vector<double> quotient = expansion_quotient(e, f);
			std::vector<double> recovered = expansion_product(quotient, f);

			double e_val = sum_expansion(e);
			double recovered_val = sum_expansion(recovered);

			if (std::abs(e_val - recovered_val) > 1.0e-12) {
				std::cout << "  FAIL: Non-power-of-2 inverse property\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Multi-component dividend
		{
			std::vector<double> e = { 100.0, 5.0e-15 };
			std::vector<double> f = { 4.0 };

			std::vector<double> quotient = expansion_quotient(e, f);
			std::vector<double> recovered = expansion_product(quotient, f);

			double e_val = sum_expansion(e);
			double recovered_val = sum_expansion(recovered);

			if (std::abs(e_val - recovered_val) > 1.0e-12) {
				std::cout << "  FAIL: Multi-component inverse property\n";
				print_expansion("e", e);
				print_expansion("quotient", quotient);
				print_expansion("recovered", recovered);
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Inverse property holds (within Newton precision)\n";
		}

		return nrOfFailedTests;
	}

	// Test quotient vs reciprocal: e ÷ f = e × reciprocal(f)
	int test_quotient_vs_reciprocal() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_quotient: e ÷ f = e × reciprocal(f)\n";

		// Test case 1: Simple values
		{
			std::vector<double> e = { 15.0 };
			std::vector<double> f = { 3.0 };

			std::vector<double> quotient = expansion_quotient(e, f);

			std::vector<double> recip_f = expansion_reciprocal(f);
			std::vector<double> product = expansion_product(e, recip_f);

			double quot_val = sum_expansion(quotient);
			double prod_val = sum_expansion(product);

			if (std::abs(quot_val - prod_val) > 1.0e-13) {
				std::cout << "  FAIL: quotient != product with reciprocal\n";
				std::cout << "    quotient: " << std::setprecision(17) << quot_val << "\n";
				std::cout << "    product:  " << std::setprecision(17) << prod_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Non-power-of-2
		{
			std::vector<double> e = { 21.0 };
			std::vector<double> f = { 7.0 };

			std::vector<double> quotient = expansion_quotient(e, f);
			std::vector<double> recip_f = expansion_reciprocal(f);
			std::vector<double> product = expansion_product(e, recip_f);

			double quot_val = sum_expansion(quotient);
			double prod_val = sum_expansion(product);

			if (std::abs(quot_val - prod_val) > 1.0e-13) {
				std::cout << "  FAIL: Non-power-of-2 quotient vs reciprocal\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Quotient matches product with reciprocal\n";
		}

		return nrOfFailedTests;
	}

	// Test quotient with extreme scales
	int test_quotient_extreme_scales() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_quotient: extreme scales\n";

		// Test case 1: Large ÷ small = very large
		{
			std::vector<double> large = { 1.0e20 };
			std::vector<double> small = { 1.0e-20 };

			std::vector<double> quotient = expansion_quotient(large, small);
			double quot_val = sum_expansion(quotient);
			double expected = 1.0e40;

			// Very loose relative tolerance for extreme values
			if (std::abs(quot_val - expected) / expected > 1.0e-10) {
				std::cout << "  FAIL: [1e20] ÷ [1e-20] != [1e40]\n";
				std::cout << "    Expected: " << std::setprecision(17) << expected << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << quot_val << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Verify inverse property for extreme scale
		{
			std::vector<double> e = { 1.0e50 };
			std::vector<double> f = { 1.0e10 };

			std::vector<double> quotient = expansion_quotient(e, f);
			std::vector<double> recovered = expansion_product(quotient, f);

			double e_val = sum_expansion(e);
			double recovered_val = sum_expansion(recovered);

			if (std::abs(e_val - recovered_val) / e_val > 1.0e-10) {
				std::cout << "  FAIL: Extreme scale inverse property\n";
				std::cout << "    Expected: " << std::setprecision(17) << e_val << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << recovered_val << "\n";
				++nrOfFailedTests;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Extreme scale divisions work correctly\n";
		}

		return nrOfFailedTests;
	}

	// Test simple fractional divisions
	int test_quotient_fractional_results() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing expansion_quotient: fractional results\n";

		// Test case 1: [1] ÷ [3] = [0.333...]
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> three = { 3.0 };

			std::vector<double> quotient = expansion_quotient(one, three);
			double quot_val = sum_expansion(quotient);
			double expected = 1.0 / 3.0;

			if (std::abs(quot_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: [1] ÷ [3] != [1/3]\n";
				std::cout << "    Expected: " << std::setprecision(17) << expected << "\n";
				std::cout << "    Got:      " << std::setprecision(17) << quot_val << "\n";
				print_expansion("quotient", quotient);
				++nrOfFailedTests;
			}
			else {
				// Show that expansion captures more precision than double
				std::cout << "  [1] ÷ [3] produces " << quotient.size() << " components\n";
			}
		}

		// Test case 2: [1] ÷ [7] = [0.142857...]
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> seven = { 7.0 };

			std::vector<double> quotient = expansion_quotient(one, seven);
			double quot_val = sum_expansion(quotient);
			double expected = 1.0 / 7.0;

			if (std::abs(quot_val - expected) > 1.0e-14) {
				std::cout << "  FAIL: [1] ÷ [7] != [1/7]\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  [1] ÷ [7] produces " << quotient.size() << " components\n";
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Fractional divisions produce extended precision\n";
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
int main()
try {
	using namespace sw::universal;

	std::cout << "========================================================\n";
	std::cout << "Expansion Division Tests (Identity-Based)\n";
	std::cout << "========================================================\n\n";

	int nrOfFailedTests = 0;

	std::cout << "RECIPROCAL TESTS (expansion_reciprocal)\n";
	std::cout << "========================================\n";
	nrOfFailedTests += test_reciprocal_of_one();
	nrOfFailedTests += test_reciprocal_simple_values();
	nrOfFailedTests += test_reciprocal_multiplicative_inverse();
	nrOfFailedTests += test_reciprocal_double_reciprocal();
	nrOfFailedTests += test_reciprocal_extreme_scales();

	std::cout << "\nQUOTIENT TESTS (expansion_quotient)\n";
	std::cout << "====================================\n";
	nrOfFailedTests += test_quotient_division_identity();
	nrOfFailedTests += test_quotient_self_division();
	nrOfFailedTests += test_quotient_inverse_property();
	nrOfFailedTests += test_quotient_vs_reciprocal();
	nrOfFailedTests += test_quotient_extreme_scales();
	nrOfFailedTests += test_quotient_fractional_results();

	std::cout << "\n========================================================\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All division tests passed\n";
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
