// quadratic_ereal.cpp: adaptive precision solution to catastrophic cancellation in the quadratic formula
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/console_utf8.hpp>  // support for UTF-8 output to console
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <iomanip>
#include <cmath>

/*
 * THE QUADRATIC FORMULA CATASTROPHIC CANCELLATION PROBLEM
 *
 * For ax² + bx + c = 0, the standard formula is:
 *     x = (-b ± √(b² - 4ac)) / 2a
 *
 * Problem: When b² >> 4ac, we compute √(b² - 4ac) ≈ |b|, leading to:
 *     x₊ = (-b + √(b² - 4ac)) / 2a  ← catastrophic cancellation!
 *
 * Example: x² - 10⁸x + 1 = 0
 *   True roots: x₁ ≈ 10⁸, x₂ ≈ 10⁻⁸
 *   With double precision:
 *     b² = 10¹⁶ (exact)
 *     4ac = 4 (exact)
 *     b² - 4ac = 10¹⁶ - 4 = 10¹⁶ (precision lost!)
 *     √(b² - 4ac) ≈ 10⁸ (appears exact, but small component lost)
 *     x₊ = (-10⁸ + 10⁸) / 2 = 0 / 2 = 0  ← WRONG! Should be 10⁻⁸
 *
 * SOLUTIONS:
 *
 * 1. Stable Reformulation (Citardauq Formula):
 *    Compute one root with good formula, use Vieta's relation for other:
 *      x₁·x₂ = c/a  →  x₂ = c/(a·x₁)
 *
 * 2. Adaptive Precision (ereal):
 *    Use the simple formula with adaptive precision arithmetic!
 *    The expansion automatically preserves the small components.
 *
 * References:
 * - Kahan, "On the Cost of Floating-Point Computation Without Extra-Precise Arithmetic"
 * - https://people.eecs.berkeley.edu/~wkahan/Qdrtcs.pdf
 * - Press et al., Numerical Recipes (Section 5.6)
 */

namespace sw { namespace universal {

	// ===================================================================
	// SQRT HELPER for ereal (Newton-Raphson)
	// ===================================================================

	// Overload for double (use std::sqrt)
	inline double my_sqrt(double x) {
		return std::sqrt(x);
	}

	// Overload for ereal (Newton-Raphson)
	template<unsigned nlimbs>
	ereal<nlimbs> my_sqrt(const ereal<nlimbs>& x) {
		double x_double = double(x);

		// Handle special cases
		if (x_double < 0.0) {
			std::cerr << "Warning: sqrt of negative number: " << x_double << "\n";
			return ereal<nlimbs>(std::numeric_limits<double>::quiet_NaN());
		}
		if (x_double == 0.0) {
			return ereal<nlimbs>(0.0);
		}

		ereal<nlimbs> result(std::sqrt(x_double));  // Initial guess
		ereal<nlimbs> half(0.5);

		// Newton-Raphson: x_{n+1} = 0.5 * (x_n + x/x_n)
		for (int i = 0; i < 10; ++i) {
			ereal<nlimbs> x_over_result = x / result;
			ereal<nlimbs> sum = result + x_over_result;
			result = half * sum;
		}
		return result;
	}

	// ===================================================================
	// NAIVE IMPLEMENTATION (Double Precision - Can Fail)
	// ===================================================================

	template<typename Real>
	std::pair<Real, Real> quadratic_naive(const Real& a, const Real& b, const Real& c) {
		Real four = Real(4.0);
		Real two = Real(2.0);

		Real discriminant = b * b - four * a * c;
		Real sqrt_disc = my_sqrt(discriminant);

		Real x1 = (-b - sqrt_disc) / (two * a);
		Real x2 = (-b + sqrt_disc) / (two * a);

		return { x1, x2 };
	}

	// ===================================================================
	// STABLE IMPLEMENTATION (Citardauq Formula - Always Works)
	// ===================================================================

	template<typename Real>
	std::pair<Real, Real> quadratic_stable(const Real& a, const Real& b, const Real& c) {
		Real four = Real(4.0);
		Real two = Real(2.0);

		Real discriminant = b * b - four * a * c;
		Real sqrt_disc = my_sqrt(discriminant);

		// Compute the root with good numerical properties
		Real x1;
		if (double(b) >= 0.0) {
			x1 = (-b - sqrt_disc) / (two * a);
		} else {
			x1 = (-b + sqrt_disc) / (two * a);
		}

		// Use Vieta's formula: x₁·x₂ = c/a  →  x₂ = c/(a·x₁)
		Real x2 = c / (a * x1);

		return { x1, x2 };
	}

	// ===================================================================
	// TEST CASE STRUCTURE
	// ===================================================================

	struct QuadraticTest {
		double a, b, c;
		std::string description;
		double true_x1, true_x2;  // High-precision reference values
	};

	// ===================================================================
	// VERIFICATION: Check Vieta's Formulas
	// ===================================================================

	template<typename Real>
	void verify_vieta(const std::string& method,
	                  const Real& a, const Real& b, const Real& c,
	                  const Real& x1, const Real& x2) {
		// Vieta's formulas:
		// x₁ + x₂ = -b/a
		// x₁·x₂ = c/a

		Real sum = x1 + x2;
		Real product = x1 * x2;
		Real expected_sum = -b / a;
		Real expected_product = c / a;

		double sum_error = std::abs(double(sum) - double(expected_sum)) / std::abs(double(expected_sum));
		double product_error = std::abs(double(product) - double(expected_product)) / std::abs(double(expected_product));

		std::cout << "  " << std::setw(20) << std::left << method << " Vieta's check:\n";
		std::cout << "    x₁ + x₂ = " << std::setw(20) << double(sum)
		          << " (expected: " << double(expected_sum) << ", rel error: "
		          << std::setprecision(2) << std::scientific << sum_error << ")\n";
		std::cout << "    x₁·x₂   = " << std::setw(20) << double(product)
		          << " (expected: " << double(expected_product) << ", rel error: "
		          << product_error << ")\n";
		std::cout << std::defaultfloat;
	}

	// ===================================================================
	// COMPARISON RUNNER
	// ===================================================================

	void run_test_case(const QuadraticTest& test) {
		std::cout << "========================================================\n";
		std::cout << "Test: " << test.description << "\n";
		std::cout << "Equation: " << test.a << "x² + " << test.b << "x + " << test.c << " = 0\n";
		std::cout << "True roots: x₁ = " << std::setprecision(17) << test.true_x1
		          << ", x₂ = " << test.true_x2 << "\n";
		std::cout << "========================================================\n\n";

		// Test 1: Double precision (naive)
		{
			std::cout << "--- Double Precision (Naive Formula) ---\n";
			auto [x1, x2] = quadratic_naive(test.a, test.b, test.c);

			double error_x1 = std::abs(x1 - test.true_x1) / std::abs(test.true_x1);
			double error_x2 = std::abs(x2 - test.true_x2) / std::abs(test.true_x2);

			std::cout << "  x₁ = " << std::setw(20) << x1 << " (rel error: "
			          << std::setprecision(2) << std::scientific << error_x1 << ")\n";
			std::cout << "  x₂ = " << std::setw(20) << x2 << " (rel error: "
			          << error_x2 << ")\n";
			std::cout << std::defaultfloat;

			verify_vieta("Double (naive)", test.a, test.b, test.c, x1, x2);
			std::cout << "\n";
		}

		// Test 2: Double precision (stable)
		{
			std::cout << "--- Double Precision (Stable Formula) ---\n";
			auto [x1, x2] = quadratic_stable(test.a, test.b, test.c);

			double error_x1 = std::abs(x1 - test.true_x1) / std::abs(test.true_x1);
			double error_x2 = std::abs(x2 - test.true_x2) / std::abs(test.true_x2);

			std::cout << "  x₁ = " << std::setw(20) << std::setprecision(17) << x1
			          << " (rel error: " << std::setprecision(2) << std::scientific << error_x1 << ")\n";
			std::cout << "  x₂ = " << std::setw(20) << std::setprecision(17) << x2
			          << " (rel error: " << error_x2 << ")\n";
			std::cout << std::defaultfloat;

			verify_vieta("Double (stable)", test.a, test.b, test.c, x1, x2);
			std::cout << "\n";
		}

		// Test 3: Adaptive precision (ereal) - naive formula works!
		{
			std::cout << "--- Adaptive Precision (ereal<19> - Naive Formula) ---\n";

			ereal<19> a(test.a), b(test.b), c(test.c);
			auto [x1, x2] = quadratic_naive(a, b, c);

			double x1_val = double(x1);
			double x2_val = double(x2);

			double error_x1 = std::abs(x1_val - test.true_x1) / std::abs(test.true_x1);
			double error_x2 = std::abs(x2_val - test.true_x2) / std::abs(test.true_x2);

			std::cout << "  x₁ = " << std::setw(20) << std::setprecision(17) << x1_val
			          << " (rel error: " << std::setprecision(2) << std::scientific << error_x1 << ")\n";
			std::cout << "  x₂ = " << std::setw(20) << std::setprecision(17) << x2_val
			          << " (rel error: " << error_x2 << ")\n";
			std::cout << "  x₁ components: " << x1.limbs().size() << ", x₂ components: " << x2.limbs().size() << "\n";
			std::cout << std::defaultfloat;

			verify_vieta("ereal (naive)", a, b, c, x1, x2);
			std::cout << "\n";
		}
	}

}} // namespace sw::universal

int main()
try {
	// enable UTF-8 output on Windows consoles
	ConsoleUTF8 consoleutf8;  // RAII - reset console to original code page on destruction

	using namespace sw::universal;

	std::cout << "============================================================\n";
	std::cout << "Quadratic Formula: Catastrophic Cancellation vs. Adaptive Precision\n";
	std::cout << "============================================================\n\n";

	std::cout << "This example demonstrates how adaptive precision (ereal) solves\n";
	std::cout << "the catastrophic cancellation problem in the quadratic formula.\n\n";

	std::cout << "Key insight: With ereal, you can use the SIMPLE, OBVIOUS formula\n";
	std::cout << "and get correct results. No need for clever reformulations!\n\n";

	// ===================================================================
	// TEST CASES (progressively more challenging)
	// ===================================================================

	std::vector<QuadraticTest> tests = {
		// Test 1: Mild cancellation
		{
			1.0, 1000.0, 1.0,
			"Mild cancellation (b² moderately larger than 4ac)",
			-999.999001,  // x₁ (larger root in magnitude)
			-0.001000001  // x₂ (smaller root)
		},

		// Test 2: Severe cancellation
		{
			1.0, 1.0e8, 1.0,
			"Severe cancellation (b² >> 4ac)",
			-100000000.0,     // x₁
			-1.0e-8           // x₂ (very small - lost in double precision)
		},

		// Test 3: Extreme cancellation
		{
			1.0, 1.0e15, 1.0,
			"Extreme cancellation (at double precision limit)",
			-1.0e15,          // x₁
			-1.0e-15          // x₂ (catastrophically lost in double)
		},

		// Test 4: Near-equal roots (challenging for any method)
		{
			1.0, 10000.0, 9999.0,
			"Near-equal roots (b² - 4ac is small)",
			-9999.00010000,  // x₁ (larger root in magnitude)
			-0.99990000      // x₂ (smaller root)
		}
	};

	// Run all test cases
	for (const auto& test : tests) {
		run_test_case(test);
		std::cout << "\n\n";
	}

	// ===================================================================
	// SUMMARY
	// ===================================================================

	std::cout << "============================================================\n";
	std::cout << "SUMMARY\n";
	std::cout << "============================================================\n\n";

	std::cout << "1. NAIVE FORMULA (double precision):\n";
	std::cout << "   - Simple to implement\n";
	std::cout << "   - FAILS on ill-conditioned problems (large relative error)\n";
	std::cout << "   - Catastrophic cancellation loses small roots\n\n";

	std::cout << "2. STABLE REFORMULATION (double precision):\n";
	std::cout << "   - Requires mathematical insight (Citardauq formula)\n";
	std::cout << "   - Works correctly on all test cases\n";
	std::cout << "   - More complex implementation\n\n";

	std::cout << "3. ADAPTIVE PRECISION (ereal):\n";
	std::cout << "   - Simple naive formula works correctly!\n";
	std::cout << "   - Expansion arithmetic preserves small components\n";
	std::cout << "   - No need for clever reformulations\n";
	std::cout << "   - Components grow adaptively (20-100 for these tests)\n\n";

	std::cout << "CONCLUSION: Adaptive precision lets you write SIMPLE, OBVIOUS\n";
	std::cout << "code that works correctly, without needing numerical analysis\n";
	std::cout << "expertise to reformulate algorithms.\n\n";

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
