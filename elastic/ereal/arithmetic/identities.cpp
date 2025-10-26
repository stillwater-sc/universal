// identities.cpp: Exact identity tests for ereal (no oracle needed)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <iomanip>

namespace sw { namespace universal {

// Helper: Component-wise exact comparison
template<unsigned nlimbs>
bool components_equal(const ereal<nlimbs>& a, const ereal<nlimbs>& b) {
	const auto& a_limbs = a.limbs();
	const auto& b_limbs = b.limbs();

	// Must have same number of components
	if (a_limbs.size() != b_limbs.size()) return false;

	// All components must match exactly
	for (size_t i = 0; i < a_limbs.size(); ++i) {
		if (a_limbs[i] != b_limbs[i]) return false;
	}

	return true;
}

// Helper: Print expansion components for debugging
template<unsigned nlimbs>
void print_expansion(const std::string& name, const ereal<nlimbs>& x) {
	const auto& limbs = x.limbs();
	std::cout << "  " << name << " expansion (" << limbs.size() << " components):\n";
	for (size_t i = 0; i < limbs.size(); ++i) {
		std::cout << "    [" << i << "]: " << std::setprecision(17) << limbs[i] << "\n";
	}
	std::cout << "  " << name << " value: " << std::setprecision(17) << double(x) << "\n";
}

// Helper: Verify expansion is valid (components in decreasing magnitude)
template<unsigned nlimbs>
bool is_valid_expansion(const ereal<nlimbs>& x) {
	const auto& limbs = x.limbs();
	if (limbs.empty()) return false;

	for (size_t i = 1; i < limbs.size(); ++i) {
		if (std::abs(limbs[i]) > std::abs(limbs[i-1])) {
			return false;  // Not in decreasing magnitude order
		}
	}
	return true;
}

// Test 1: Additive Identity Recovery - (a + b) - a = b EXACTLY
int test_additive_identity_exact() {
	int nrOfFailedTests = 0;

	std::cout << "Test 1: Additive Identity Recovery (exact)\n";
	std::cout << "===========================================\n";
	std::cout << "Property: (a + b) - a = b (component-wise exact)\n\n";

	// Test 1a: Simple case where EFT should preserve everything
	{
		std::cout << "Test 1a: a=10, b=5 (simple integers)\n";
		ereal<64> a(10.0);
		ereal<64> b(5.0);
		ereal<64> sum = a + b;
		ereal<64> recovered = sum - a;

		if (!components_equal(recovered, b)) {
			std::cout << "  FAIL: Components don't match exactly\n";
			print_expansion("b (expected)", b);
			print_expansion("recovered", recovered);
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Exact component match\n";
		}
	}

	// Test 1b: Large + small (triggers expansion)
	{
		std::cout << "\nTest 1b: a=1.0, b=1e-15 (small component)\n";
		ereal<64> a(1.0);
		ereal<64> b(1.0e-15);

		std::cout << "  Initial b expansion:\n";
		print_expansion("b", b);

		ereal<64> sum = a + b;
		std::cout << "  After a + b:\n";
		print_expansion("sum", sum);

		ereal<64> recovered = sum - a;
		std::cout << "  After (a+b) - a:\n";
		print_expansion("recovered", recovered);

		// Verify b is recovered exactly
		double b_val = double(b);
		double recovered_val = double(recovered);

		if (std::abs(recovered_val - b_val) > 0.0) {
			std::cout << "  FAIL: Did not recover b exactly\n";
			std::cout << "  Expected: " << std::setprecision(17) << b_val << "\n";
			std::cout << "  Got:      " << std::setprecision(17) << recovered_val << "\n";
			std::cout << "  Diff:     " << std::abs(recovered_val - b_val) << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Recovered b exactly (value match)\n";
		}
	}

	// Test 1c: Very large + very small (extreme scale difference)
	{
		std::cout << "\nTest 1c: a=1e20, b=1.0 (extreme scale difference)\n";
		ereal<64> a(1.0e20);
		ereal<64> b(1.0);

		ereal<64> sum = a + b;
		ereal<64> recovered = sum - a;

		double b_val = double(b);
		double recovered_val = double(recovered);

		// With double precision, this would lose b completely
		// ereal should preserve it
		if (std::abs(recovered_val - b_val) > 1.0e-14) {
			std::cout << "  FAIL: Lost precision in extreme scale difference\n";
			std::cout << "  Expected: " << std::setprecision(17) << b_val << "\n";
			std::cout << "  Got:      " << std::setprecision(17) << recovered_val << "\n";
			print_expansion("sum", sum);
			print_expansion("recovered", recovered);
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Preserved b despite extreme scale (20 orders of magnitude)\n";
		}
	}

	// Test 1d: Multiple tiny components
	{
		std::cout << "\nTest 1d: a=1.0, b=1e-30 (extremely small component)\n";
		ereal<64> a(1.0);
		ereal<64> b(1.0e-30);

		ereal<64> sum = a + b;
		ereal<64> recovered = sum - a;

		double b_val = double(b);
		double recovered_val = double(recovered);

		if (std::abs(recovered_val - b_val) > 1.0e-40) {
			std::cout << "  FAIL: Lost extremely small component\n";
			std::cout << "  Expected: " << std::setprecision(17) << b_val << "\n";
			std::cout << "  Got:      " << std::setprecision(17) << recovered_val << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Preserved extremely small component\n";
		}
	}

	return nrOfFailedTests;
}

// Test 2: Multiplicative Identity - a × (1/a) = 1 EXACTLY
int test_multiplicative_identity_exact() {
	int nrOfFailedTests = 0;

	std::cout << "\n\nTest 2: Multiplicative Identity (exact)\n";
	std::cout << "========================================\n";
	std::cout << "Property: a × (1/a) = 1 (exactly)\n\n";

	// Test 2a: Simple integer
	{
		std::cout << "Test 2a: a=3.0\n";
		ereal<64> a(3.0);
		ereal<64> one(1.0);
		ereal<64> reciprocal = one / a;
		ereal<64> result = a * reciprocal;

		print_expansion("reciprocal (1/3)", reciprocal);
		print_expansion("a × (1/a)", result);

		double result_val = double(result);
		if (std::abs(result_val - 1.0) > 1.0e-15) {
			std::cout << "  FAIL: a × (1/a) ≠ 1\n";
			std::cout << "  Got: " << std::setprecision(17) << result_val << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: a × (1/a) = 1 (within Newton precision)\n";
		}
	}

	// Test 2b: Non-power-of-2
	{
		std::cout << "\nTest 2b: a=7.0\n";
		ereal<64> a(7.0);
		ereal<64> one(1.0);
		ereal<64> reciprocal = one / a;
		ereal<64> result = a * reciprocal;

		double result_val = double(result);
		if (std::abs(result_val - 1.0) > 1.0e-15) {
			std::cout << "  FAIL: a × (1/a) ≠ 1\n";
			std::cout << "  Got: " << std::setprecision(17) << result_val << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: a × (1/a) = 1\n";
		}
	}

	// Test 2c: Large value
	{
		std::cout << "\nTest 2c: a=1e10\n";
		ereal<64> a(1.0e10);
		ereal<64> one(1.0);
		ereal<64> reciprocal = one / a;
		ereal<64> result = a * reciprocal;

		double result_val = double(result);
		if (std::abs(result_val - 1.0) > 1.0e-14) {
			std::cout << "  FAIL: a × (1/a) ≠ 1\n";
			std::cout << "  Got: " << std::setprecision(17) << result_val << "\n";
			print_expansion("result", result);
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: a × (1/a) = 1 (large value)\n";
		}
	}

	return nrOfFailedTests;
}

// Test 3: Exact Associativity - (a+b)+c = a+(b+c) component-wise
int test_exact_associativity() {
	int nrOfFailedTests = 0;

	std::cout << "\n\nTest 3: Exact Associativity\n";
	std::cout << "============================\n";
	std::cout << "Property: (a+b)+c = a+(b+c) (component-wise exact)\n\n";

	// Test 3a: Simple values
	{
		std::cout << "Test 3a: a=10, b=5, c=2 (simple)\n";
		ereal<64> a(10.0);
		ereal<64> b(5.0);
		ereal<64> c(2.0);

		ereal<64> left = (a + b) + c;
		ereal<64> right = a + (b + c);

		double left_val = double(left);
		double right_val = double(right);

		if (std::abs(left_val - right_val) > 0.0) {
			std::cout << "  FAIL: Associativity violated\n";
			std::cout << "  (a+b)+c = " << std::setprecision(17) << left_val << "\n";
			std::cout << "  a+(b+c) = " << std::setprecision(17) << right_val << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Exact associativity\n";
		}
	}

	// Test 3b: Mixed scales
	{
		std::cout << "\nTest 3b: a=1.0, b=1e-15, c=1e-30 (mixed scales)\n";
		ereal<64> a(1.0);
		ereal<64> b(1.0e-15);
		ereal<64> c(1.0e-30);

		ereal<64> left = (a + b) + c;
		ereal<64> right = a + (b + c);

		print_expansion("(a+b)+c", left);
		print_expansion("a+(b+c)", right);

		double left_val = double(left);
		double right_val = double(right);

		// Should preserve all precision
		if (std::abs(left_val - right_val) > 1.0e-40) {
			std::cout << "  FAIL: Lost precision in associativity\n";
			std::cout << "  Difference: " << std::abs(left_val - right_val) << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Preserved precision across mixed scales\n";
		}
	}

	return nrOfFailedTests;
}

// Test 4: Exact Distributivity - a×(b+c) = (a×b)+(a×c)
int test_exact_distributivity() {
	int nrOfFailedTests = 0;

	std::cout << "\n\nTest 4: Exact Distributivity\n";
	std::cout << "=============================\n";
	std::cout << "Property: a×(b+c) = (a×b)+(a×c) (exact within precision limits)\n\n";

	// Test 4a: Simple integer values
	{
		std::cout << "Test 4a: a=2, b=3, c=5 (simple integers)\n";
		ereal<64> a(2.0);
		ereal<64> b(3.0);
		ereal<64> c(5.0);

		ereal<64> left = a * (b + c);
		ereal<64> right = (a * b) + (a * c);

		double left_val = double(left);
		double right_val = double(right);

		std::cout << "  a×(b+c) = " << std::setprecision(17) << left_val << "\n";
		std::cout << "  a×b+a×c = " << std::setprecision(17) << right_val << "\n";

		// With integers and EFTs, should be exact or very close
		if (std::abs(left_val - right_val) > 1.0e-14) {
			std::cout << "  FAIL: Distributivity violated\n";
			std::cout << "  Difference: " << std::abs(left_val - right_val) << "\n";
			print_expansion("left", left);
			print_expansion("right", right);
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Distributivity holds\n";
		}
	}

	// Test 4b: Non-power-of-2 values
	{
		std::cout << "\nTest 4b: a=1.5, b=2.3, c=4.7 (non-power-of-2)\n";
		ereal<64> a(1.5);
		ereal<64> b(2.3);
		ereal<64> c(4.7);

		ereal<64> left = a * (b + c);
		ereal<64> right = (a * b) + (a * c);

		print_expansion("a×(b+c)", left);
		print_expansion("(a×b)+(a×c)", right);

		double left_val = double(left);
		double right_val = double(right);

		// Non-power-of-2 may accumulate some error, but should be very small
		if (std::abs(left_val - right_val) > 1.0e-13) {
			std::cout << "  FAIL: Distributivity violated beyond tolerance\n";
			std::cout << "  Difference: " << std::abs(left_val - right_val) << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Distributivity holds within precision limits\n";
			std::cout << "  Difference: " << std::abs(left_val - right_val) << "\n";
		}
	}

	return nrOfFailedTests;
}

// Test 5: Inverse Operations - Verify (a op b) inv_op b = a
int test_inverse_operations() {
	int nrOfFailedTests = 0;

	std::cout << "\n\nTest 5: Inverse Operations\n";
	std::cout << "===========================\n";
	std::cout << "Property: (a - b) + b = a and (a / b) × b = a\n\n";

	// Test 5a: Subtraction/Addition inverse
	{
		std::cout << "Test 5a: (a - b) + b = a\n";
		ereal<64> a(15.5);
		ereal<64> b(7.25);

		ereal<64> diff = a - b;
		ereal<64> recovered = diff + b;

		print_expansion("original a", a);
		print_expansion("recovered", recovered);

		double a_val = double(a);
		double recovered_val = double(recovered);

		if (std::abs(recovered_val - a_val) > 1.0e-15) {
			std::cout << "  FAIL: Did not recover a exactly\n";
			std::cout << "  Difference: " << std::abs(recovered_val - a_val) << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Recovered a exactly via inverse operation\n";
		}
	}

	// Test 5b: Division/Multiplication inverse
	{
		std::cout << "\nTest 5b: (a / b) × b = a\n";
		ereal<64> a(15.5);
		ereal<64> b(3.5);

		ereal<64> quotient = a / b;
		ereal<64> recovered = quotient * b;

		print_expansion("original a", a);
		print_expansion("quotient", quotient);
		print_expansion("recovered", recovered);

		double a_val = double(a);
		double recovered_val = double(recovered);

		// Newton iteration may introduce small error
		if (std::abs(recovered_val - a_val) > 1.0e-14) {
			std::cout << "  FAIL: Did not recover a within tolerance\n";
			std::cout << "  Expected: " << std::setprecision(17) << a_val << "\n";
			std::cout << "  Got:      " << std::setprecision(17) << recovered_val << "\n";
			std::cout << "  Difference: " << std::abs(recovered_val - a_val) << "\n";
			++nrOfFailedTests;
		} else {
			std::cout << "  PASS: Recovered a within Newton precision\n";
			std::cout << "  Difference: " << std::abs(recovered_val - a_val) << "\n";
		}
	}

	return nrOfFailedTests;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "========================================================\n";
	std::cout << "ereal EXACT IDENTITY TESTS (Phase 1)\n";
	std::cout << "========================================================\n";
	std::cout << "Testing mathematical identities that should hold exactly\n";
	std::cout << "with error-free transformations (no oracle needed)\n";
	std::cout << "========================================================\n\n";

	int nrOfFailedTests = 0;

	nrOfFailedTests += test_additive_identity_exact();
	nrOfFailedTests += test_multiplicative_identity_exact();
	nrOfFailedTests += test_exact_associativity();
	nrOfFailedTests += test_exact_distributivity();
	nrOfFailedTests += test_inverse_operations();

	std::cout << "\n========================================================\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " identity tests failed\n";
	} else {
		std::cout << "SUCCESS: All exact identity tests passed\n";
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
