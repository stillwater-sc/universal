// expansion_ops.cpp: Unit tests for expansion operations
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

	// Test TWO-SUM error-free transformation
	int test_two_sum() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing TWO-SUM error-free transformation\n";

		// Test case 1: Large + small (classic catastrophic cancellation case)
		{
			double a = 1.0e16;
			double b = 1.0;
			double sum, error;

			two_sum(a, b, sum, error);

			// The floating-point sum loses the 1.0
			if (sum != a) ++nrOfFailedTests;
			// But TWO-SUM captures it in the error term
			if (error != b) ++nrOfFailedTests;
		}

		// Test case 2: Opposite signs (cancellation)
		{
			double a = 1.0;
			double b = -1.0;
			double sum, error;

			two_sum(a, b, sum, error);

			// Should get exact zero
			if (sum != 0.0) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;
		}

		// Test case 3: Values that round
		{
			double a = 1.0;
			double b = 1.0e-20;
			double sum, error;

			two_sum(a, b, sum, error);

			// sum + error should exactly equal a + b (no rounding in double-double)
			// We verify by converting to higher precision
			long double check = static_cast<long double>(sum) + static_cast<long double>(error);
			long double exact = static_cast<long double>(a) + static_cast<long double>(b);

			// They should be equal in long double precision
			if (check != exact) ++nrOfFailedTests;
		}

		// Test case 4: Both positive, close in magnitude
		{
			double a = 3.0;
			double b = 2.0;
			double sum, error;

			two_sum(a, b, sum, error);

			if (sum != 5.0) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;  // Exact in this case
		}

		return nrOfFailedTests;
	}

	// Test FAST-TWO-SUM (when |a| >= |b|)
	int test_fast_two_sum() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing FAST-TWO-SUM error-free transformation\n";

		// Test case 1: |a| > |b| (precondition satisfied)
		{
			double a = 100.0;
			double b = 0.5;
			double sum, error;

			fast_two_sum(a, b, sum, error);

			if (sum != 100.5) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;
		}

		// Test case 2: Large + small
		{
			double a = 1.0e16;
			double b = 1.0;
			double sum, error;

			fast_two_sum(a, b, sum, error);

			// Same behavior as TWO-SUM for this case
			if (sum != a) ++nrOfFailedTests;
			if (error != b) ++nrOfFailedTests;
		}

		// Test case 3: Negative larger magnitude
		{
			double a = -1000.0;
			double b = 1.0;
			double sum, error;

			fast_two_sum(a, b, sum, error);

			if (sum != -999.0) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test TWO-PROD error-free multiplication
	int test_two_prod() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing TWO-PROD error-free multiplication\n";

		// Test case 1: Simple multiplication
		{
			double a = 2.0;
			double b = 3.0;
			double product, error;

			two_prod(a, b, product, error);

			if (product != 6.0) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;
		}

		// Test case 2: Multiplication that rounds
		{
			double a = 1.5;
			double b = 0.3;
			double product, error;

			two_prod(a, b, product, error);

			// product + error should equal a * b exactly
			// The error captures rounding in the multiplication
			double check = product + error;
			double expected = a * b;

			// In double precision, these should be equal
			if (check != expected) ++nrOfFailedTests;
		}

		// Test case 3: Small values
		{
			double a = 1.0e-10;
			double b = 1.0e-10;
			double product, error;

			two_prod(a, b, product, error);

			// Product is 1.0e-20
			// Error captures any rounding
			if (std::abs(product - 1.0e-20) > 1.0e-30) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test GROW-EXPANSION
	int test_grow_expansion() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing GROW-EXPANSION\n";

		// Test case 1: Grow a 2-component expansion
		{
			std::vector<double> e = { 3.0, 5.0e-16 };
			double b = 1.0;

			std::vector<double> h = grow_expansion(e, b);

			// Should have 3 components
			if (h.size() != 3) ++nrOfFailedTests;

			// Verify value is preserved: h should equal e + b
			double e_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : h) h_sum += v;

			// h_sum should equal e_sum + b
			double expected = e_sum + b;
			if (std::abs(h_sum - expected) > 1.0e-14) ++nrOfFailedTests;

			// Verify decreasing magnitude
			if (!is_decreasing_magnitude(h)) ++nrOfFailedTests;
		}

		// Test case 2: Grow empty expansion
		{
			std::vector<double> e;
			double b = 42.0;

			std::vector<double> h = grow_expansion(e, b);

			if (h.size() != 1) ++nrOfFailedTests;
			if (h[0] != b) ++nrOfFailedTests;
		}

		// Test case 3: Grow with zero
		{
			std::vector<double> e = { 1.0 };
			double b = 0.0;

			std::vector<double> h = grow_expansion(e, b);

			// Should still work, may have zeros
			if (h.empty()) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test FAST-EXPANSION-SUM
	int test_fast_expansion_sum() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing FAST-EXPANSION-SUM\n";

		// Test case 1: Add two 2-component expansions
		{
			std::vector<double> e = { 3.0, 5.0e-16 };
			std::vector<double> f = { 2.0, 3.0e-16 };

			std::vector<double> h = fast_expansion_sum(e, f);

			// Verify value
			double e_sum = 0.0, f_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : f) f_sum += v;
			for (auto v : h) h_sum += v;

			// h should equal e + f
			double expected = e_sum + f_sum;
			if (std::abs(h_sum - expected) > 1.0e-14) ++nrOfFailedTests;

			// Verify properties
			if (!is_decreasing_magnitude(h)) ++nrOfFailedTests;
		}

		// Test case 2: Add empty expansion
		{
			std::vector<double> e = { 1.0, 2.0 };
			std::vector<double> f;

			std::vector<double> h = fast_expansion_sum(e, f);

			// Should return e unchanged
			if (h.size() != e.size()) ++nrOfFailedTests;
			for (size_t i = 0; i < e.size(); ++i) {
				if (h[i] != e[i]) ++nrOfFailedTests;
			}
		}

		// Test case 3: Identity test (a + b) - a = b
		{
			std::vector<double> a = { 1.5, 1.5e-17 };
			std::vector<double> b = { 0.5, 5.0e-18 };

			std::vector<double> sum = fast_expansion_sum(a, b);

			// Negate a
			std::vector<double> neg_a = a;
			for (auto& v : neg_a) v = -v;

			std::vector<double> recovered_b = fast_expansion_sum(sum, neg_a);

			// recovered_b should equal b (within precision)
			if (recovered_b.size() == 0) ++nrOfFailedTests;

			// Check first component
			if (std::abs(recovered_b[0] - b[0]) > 1.0e-14) {
				++nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// Test LINEAR-EXPANSION-SUM
	int test_linear_expansion_sum() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing LINEAR-EXPANSION-SUM\n";

		// Test case 1: Same as FAST but using LINEAR
		{
			std::vector<double> e = { 10.0, 1.0e-15 };
			std::vector<double> f = { 5.0, 2.0e-15 };

			std::vector<double> h = linear_expansion_sum(e, f);

			// Verify value
			double e_sum = 0.0, f_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : f) f_sum += v;
			for (auto v : h) h_sum += v;

			double expected = e_sum + f_sum;
			if (std::abs(h_sum - expected) > 1.0e-13) ++nrOfFailedTests;

			// Verify properties
			if (!is_decreasing_magnitude(h)) ++nrOfFailedTests;
		}

		// Test case 2: Compare LINEAR vs FAST results
		{
			std::vector<double> e = { 7.0, 3.5e-16 };
			std::vector<double> f = { 3.0, 1.2e-16 };

			std::vector<double> h_fast = fast_expansion_sum(e, f);
			std::vector<double> h_linear = linear_expansion_sum(e, f);

			// Both should give same value
			double fast_sum = 0.0, linear_sum = 0.0;
			for (auto v : h_fast) fast_sum += v;
			for (auto v : h_linear) linear_sum += v;

			if (std::abs(fast_sum - linear_sum) > 1.0e-14) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test invariant verification functions
	int test_invariants() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing invariant verification functions\n";

		// Test is_decreasing_magnitude
		{
			std::vector<double> e1 = { 10.0, 1.0, 0.1 };
			std::vector<double> e2 = { 10.0, 0.1, 1.0 };
			std::vector<double> e3 = { -10.0, -1.0, -0.1 };  // Negative but decreasing magnitude

			if (!is_decreasing_magnitude(e1)) ++nrOfFailedTests;
			if (is_decreasing_magnitude(e2)) ++nrOfFailedTests;  // Should fail
			if (!is_decreasing_magnitude(e3)) ++nrOfFailedTests;
		}

		// Test estimate
		{
			std::vector<double> e = { 1.0, 5.0e-16, 3.0e-32, 1.0e-48 };
			double est = estimate(e);

			// Should be close to 1.0 (dominated by first component)
			if (std::abs(est - 1.0) > 1.0e-14) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
int main()
try {
	using namespace sw::universal;

	std::cout << "Expansion Operations Unit Tests\n";
	std::cout << "================================\n\n";

	int nrOfFailedTests = 0;

	nrOfFailedTests += test_two_sum();
	nrOfFailedTests += test_fast_two_sum();
	nrOfFailedTests += test_two_prod();
	nrOfFailedTests += test_grow_expansion();
	nrOfFailedTests += test_fast_expansion_sum();
	nrOfFailedTests += test_linear_expansion_sum();
	nrOfFailedTests += test_invariants();

	std::cout << "\nTest Summary:\n";
	std::cout << "  TWO-SUM tests: " << (test_two_sum() == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "  FAST-TWO-SUM tests: " << (test_fast_two_sum() == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "  TWO-PROD tests: " << (test_two_prod() == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "  GROW-EXPANSION tests: " << (test_grow_expansion() == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "  FAST-EXPANSION-SUM tests: " << (test_fast_expansion_sum() == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "  LINEAR-EXPANSION-SUM tests: " << (test_linear_expansion_sum() == 0 ? "PASS" : "FAIL") << "\n";
	std::cout << "  Invariant tests: " << (test_invariants() == 0 ? "PASS" : "FAIL") << "\n";

	std::cout << "\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All expansion operation tests passed\n";
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
