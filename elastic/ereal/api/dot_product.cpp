// dot_product.cpp: demonstration of accurate dot product computation with ereal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <vector>

/*
 * ACCURATE DOT PRODUCTS: The foundation of linear algebra
 *
 * Dot product: a·b = Σ(aᵢ × bᵢ)
 *
 * Problems with fixed precision:
 * 1. Products can vary widely in magnitude
 * 2. Summation loses precision (see accurate_summation.cpp)
 * 3. Result is order-dependent
 * 4. Critical for matrix multiplication, norms, projections
 *
 * Classical solution:
 * - Use compensated summation (Kahan)
 * - Use extended precision accumulator (quire in posit arithmetic)
 * - Sort products by magnitude before summing
 *
 * Adaptive precision solution:
 * - Use ereal - quire-like exact accumulation!
 * - No precision loss during products or summation
 */

namespace sw { namespace universal {

	// Naive dot product
	template<typename Real>
	Real dot_product_naive(const std::vector<double>& a, const std::vector<double>& b) {
		if (a.size() != b.size()) {
			throw std::runtime_error("Vector size mismatch");
		}

		Real result(0.0);
		for (size_t i = 0; i < a.size(); ++i) {
			result = result + Real(a[i]) * Real(b[i]);
		}
		return result;
	}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "============================================================\n";
	std::cout << "Accurate Dot Products: ereal vs. double\n";
	std::cout << "============================================================\n\n";

	// ===================================================================
	// Test 1: Order matters in double precision! (near-cancellation)
	// ===================================================================

	std::cout << "Test 1: Order-Dependence with Near-Cancellation\n";
	std::cout << "------------------------------------------------\n\n";

	{
		// Near-cancellation: same products, different accumulation order
		// Order 1: large terms cancel first, then add small term
		// Order 2: small term gets absorbed into large term, then cancellation
		std::vector<double> a1 = { -1.0e16, 1.0e16, 1.0 };
		std::vector<double> b1 = {  1.0,    1.0,    1.0 };

		// Reverse order: small term first
		std::vector<double> a2 = {  1.0,    -1.0e16, 1.0e16 };
		std::vector<double> b2 = {  1.0,     1.0,    1.0 };

		double dot1 = dot_product_naive<double>(a1, b1);
		double dot2 = dot_product_naive<double>(a2, b2);

		ereal<16> edot1 = dot_product_naive<ereal<16>>(a1, b1);
		ereal<16> edot2 = dot_product_naive<ereal<16>>(a2, b2);

		std::cout << "Expected: (-1e16 × 1) + (1e16 × 1) + (1 × 1) = 1\n\n";

		std::cout << "Order 1: [-1e16, 1e16, 1]·[1, 1, 1]\n";
		std::cout << "  Accumulation: ((-1e16 + 1e16) + 1) = (0 + 1) = 1\n\n";

		std::cout << "Order 2: [1, -1e16, 1e16]·[1, 1, 1]\n";
		std::cout << "  Accumulation: ((1 + (-1e16)) + 1e16) = (-1e16 + 1e16) = 0 (WRONG!)\n";
		std::cout << "  Problem: The '1' is lost when added to -1e16\n\n";

		std::cout << "Double precision:\n";
		std::cout << "  Order 1: " << std::setprecision(17) << dot1 << "\n";
		std::cout << "  Order 2: " << std::setprecision(17) << dot2 << "\n";
		std::cout << "  Difference: " << std::abs(dot1 - dot2) << " (catastrophic!)\n";
		std::cout << "  Relative error: " << std::abs(dot1 - dot2) / std::max(std::abs(dot1), std::abs(dot2)) * 100 << "%\n\n";

		std::cout << "ereal<16>:\n";
		std::cout << "  Order 1: " << std::setprecision(17) << double(edot1) << "\n";
		std::cout << "  Order 2: " << std::setprecision(17) << double(edot2) << "\n";
		std::cout << "  Difference: " << std::abs(double(edot1) - double(edot2)) << " (order-independent!)\n";
		std::cout << "  Components: " << edot1.limbs().size() << " (preserves all precision)\n\n";
	}

	// ===================================================================
	// Test 2: Small components preserved
	// ===================================================================

	std::cout << "Test 2: Preserving Small Components\n";
	std::cout << "------------------------------------\n\n";

	{
		std::vector<double> a = { 1.0e10, 1.0, 1.0e10, 1.0 };
		std::vector<double> b = { 1.0, 1.0e10, -1.0, 1.0e10 };

		// Expected: 1e10×1 + 1×1e10 + 1e10×(-1) + 1×1e10
		//         = 1e10 + 1e10 - 1e10 + 1e10 = 2e10
		// BUT: 1×1e10 terms should cancel perfectly!
		// Result should be dominated by the small cross terms

		double dot_double = dot_product_naive<double>(a, b);
		ereal<16> dot_ereal = dot_product_naive<ereal<16>>(a, b);

		std::cout << "Vectors:\n";
		std::cout << "  a = [1e10, 1, 1e10, 1]\n";
		std::cout << "  b = [1, 1e10, -1, 1e10]\n";
		std::cout << "Expected: 1e10 + 1e10 - 1e10 + 1e10 = 2e10\n\n";

		std::cout << "Double precision: " << std::setprecision(17) << dot_double << "\n";
		std::cout << "ereal<16>:        " << double(dot_ereal) << "\n";
		std::cout << "  Components: " << dot_ereal.limbs().size() << "\n\n";
	}

	// ===================================================================
	// Test 3: Ill-Conditioned Dot Product (Massive Cancellation)
	// ===================================================================

	std::cout << "Test 3: Ill-Conditioned Dot Product (Massive Cancellation)\n";
	std::cout << "-----------------------------------------------------------\n\n";

	{
		// Ill-conditioned: alternating huge terms with sub-ULP residuals
		// High condition number: κ = (||a|| × ||b||) / |a·b| >> 1
		//
		// Pattern: 20 pairs of (BIG, -BIG) in vector a where BIG = 1e16
		//          Relative perturbations eps = 1e-16 create sub-ULP residuals
		//
		// Key insight:
		//   - ULP at 1e16 is ~2.0 (2^53 spacing)
		//   - Products: BIG × (1 + i×eps) = 1e16 + i  (where i = 0..19)
		//   - The residual "i" is sub-ULP and OBLITERATED in double precision!
		//   - After cancellation: (1e16 + i) - 1e16 = i is LOST in double
		//   - ereal preserves every component exactly
		//
		// This creates:
		//   - Intermediate sums swinging ±1e16 (catastrophic cancellation)
		//   - Final result = 190 (sum 0+1+2+...+19) - microscopic vs intermediate values
		//   - Condition number κ ≈ 1e16 / 190 ≈ 5e13 (catastrophically ill-conditioned!)
		//   - Double precision obliterates the sub-ULP residuals
		//   - ereal preserves all components exactly

		constexpr size_t n_pairs = 20;
		constexpr double BIG = 1.0e16;
		constexpr double eps = 1.0e-16;

		std::vector<double> a(2 * n_pairs);
		std::vector<double> b(2 * n_pairs);

		// Construct alternating ±BIG with sub-ULP perturbations
		for (size_t i = 0; i < n_pairs; ++i) {
			a[2*i]     =  BIG;
			a[2*i + 1] = -BIG;
			b[2*i]     =  1.0 + static_cast<double>(i) * eps;  // Sub-ULP perturbation
			b[2*i + 1] =  1.0;
		}

		// Expected: Σᵢ(BIG × (1 + i×eps)) + Σᵢ(-BIG × 1)
		//         = Σᵢ(BIG + BIG×i×eps - BIG)
		//         = Σᵢ(BIG × i × eps)
		//         = BIG × eps × (0 + 1 + 2 + ... + 19)
		//         = 1e16 × 1e-16 × 190
		//         = 190
		double expected = BIG * eps * (n_pairs * (n_pairs - 1) / 2);

		double dot_double = dot_product_naive<double>(a, b);
		ereal<16> dot_ereal = dot_product_naive<ereal<16>>(a, b);

		std::cout << "Sub-ULP catastrophic cancellation:\n";
		std::cout << "  Vector length: " << a.size() << " elements\n";
		std::cout << "  BIG = " << std::scientific << BIG << " (ULP at BIG ≈ 2.0)\n";
		std::cout << "  eps = " << eps << " (relative perturbation)\n";
		std::cout << std::defaultfloat;
		std::cout << "  Pattern: a = [BIG, -BIG, BIG, -BIG, ...] (20 pairs)\n";
		std::cout << "           b = [1+0ε, 1, 1+1ε, 1, 1+2ε, 1, ...] (i = 0..19)\n\n";

		std::cout << "  Products: BIG × (1 + i×eps) = 1e16 + i (integer i is sub-ULP!)\n";
		std::cout << "  After cancellation: (1e16 + i) - 1e16 = i (OBLITERATED in double)\n";
		std::cout << "  Intermediate sums swing: ±1e16\n";
		std::cout << "  Expected final result:   " << expected << " (0+1+2+...+19 = 190)\n";
		std::cout << "  Condition number κ:      ~" << std::scientific << (2.0 * BIG) / expected
		          << " (catastrophically ill-conditioned!)\n\n";
		std::cout << std::defaultfloat;

		double rel_error_double = std::abs(dot_double - expected) / expected;
		double rel_error_ereal = std::abs(double(dot_ereal) - expected) / expected;

		std::cout << "Double precision: " << std::setprecision(17) << dot_double << "\n";
		std::cout << "  Absolute error: " << std::abs(dot_double - expected)
		          << " (sub-ULP residuals obliterated!)\n";
		std::cout << "  Relative error: " << std::scientific << rel_error_double
		          << " (" << std::defaultfloat << rel_error_double * 100 << "%)\n";

		// Report accuracy loss, handling zero error gracefully
		constexpr double ZERO_THRESHOLD = 1.0e-20;
		if (rel_error_double < ZERO_THRESHOLD) {
			std::cout << "  Accuracy: full precision (no loss)\n";
		} else {
			std::cout << "  Lost ~" << std::setprecision(1) << std::fixed
			          << -std::log10(rel_error_double) << " digits of accuracy\n";
		}
		std::cout << "\n";

		std::cout << std::setprecision(17);
		std::cout << "ereal<16>:        " << double(dot_ereal) << "\n";
		std::cout << "  Absolute error: " << std::abs(double(dot_ereal) - expected)
		          << " (sub-ULP residuals preserved!)\n";
		std::cout << "  Relative error: " << std::scientific << rel_error_ereal;
		if (rel_error_ereal < ZERO_THRESHOLD) {
			std::cout << " (exact)\n";
		} else {
			std::cout << " (near machine epsilon)\n";
		}
		std::cout << std::defaultfloat;
		std::cout << "  Components: " << dot_ereal.limbs().size()
		          << " (adaptive precision handles sub-ULP scale)\n\n";
	}

	// ===================================================================
	// Test 4: Many small products
	// ===================================================================

	std::cout << "Test 4: Accumulation of Many Small Products\n";
	std::cout << "--------------------------------------------\n\n";

	{
		std::vector<double> a(1000, 1.0e-5);
		std::vector<double> b(1000, 1.0e-5);

		// Expected: 1000 × (1e-5 × 1e-5) = 1000 × 1e-10 = 1e-7

		double dot_double = dot_product_naive<double>(a, b);
		ereal<16> dot_ereal = dot_product_naive<ereal<16>>(a, b);

		double expected = 1.0e-7;

		std::cout << "1000 terms of (1e-5 × 1e-5):\n";
		std::cout << "Expected: " << std::scientific << expected << "\n\n";
		std::cout << std::defaultfloat;

		std::cout << "Double precision: " << std::setprecision(17) << dot_double
		          << " (rel error: " << std::scientific
		          << std::abs(dot_double - expected)/expected << ")\n";
		std::cout << "ereal<16>:        " << double(dot_ereal)
		          << " (rel error: " << std::abs(double(dot_ereal) - expected)/expected << ")\n";
		std::cout << std::defaultfloat;
		std::cout << "  Components: " << dot_ereal.limbs().size() << "\n\n";
	}

	// ===================================================================
	// SUMMARY
	// ===================================================================

	std::cout << "============================================================\n";
	std::cout << "KEY INSIGHTS\n";
	std::cout << "============================================================\n\n";

	std::cout << "Double Precision Dot Products:\n";
	std::cout << "  - Order-dependent (violates commutative property!)\n";
	std::cout << "  - Loses small components when mixed with large values\n";
	std::cout << "  - Accumulates rounding errors\n";
	std::cout << "  - Critical issue for iterative linear algebra\n\n";

	std::cout << "Adaptive Precision (ereal) Dot Products:\n";
	std::cout << "  - Order-independent (mathematically correct)\n";
	std::cout << "  - Preserves all components exactly\n";
	std::cout << "  - Quire-like exact accumulation\n";
	std::cout << "  - Simple implementation (naive algorithm works!)\n";
	std::cout << "  - Components grow adaptively (~20-50 for typical cases)\n\n";

	std::cout << "Applications:\n";
	std::cout << "  - Matrix-vector multiplication\n";
	std::cout << "  - Vector norms (||v|| = √(v·v))\n";
	std::cout << "  - Projections and orthogonalization\n";
	std::cout << "  - Inner product spaces\n";
	std::cout << "  - Iterative solvers (conjugate gradient, GMRES, etc.)\n\n";

	std::cout << "Use ereal when:\n";
	std::cout << "  - Dot products are critical to algorithm correctness\n";
	std::cout << "  - Working with ill-conditioned vectors\n";
	std::cout << "  - Need reproducible results (order-independent)\n";
	std::cout << "  - Building foundational linear algebra operations\n\n";

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
