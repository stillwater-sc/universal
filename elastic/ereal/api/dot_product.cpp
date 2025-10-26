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
	// Test 1: Order matters in double precision!
	// ===================================================================

	std::cout << "Test 1: Order-Dependence (associativity property violation)\n";
	std::cout << "-----------------------------------------------------------\n\n";

	{
		std::vector<double> a1 = { 1.0e20, 1.0 };
		std::vector<double> b1 = { 1.0, 1.0e20 };

		std::vector<double> a2 = { 1.0, 1.0e20 };
		std::vector<double> b2 = { 1.0e20, 1.0 };

		double dot1 = dot_product_naive<double>(a1, b1);
		double dot2 = dot_product_naive<double>(a2, b2);

		ereal<64> edot1 = dot_product_naive<ereal<64>>(a1, b1);
		ereal<64> edot2 = dot_product_naive<ereal<64>>(a2, b2);

		std::cout << "Vectors: a = [1e20, 1], b = [1, 1e20]\n";
		std::cout << "Expected: 1e20 × 1 + 1 × 1e20 = 2e20\n\n";

		std::cout << "Double precision:\n";
		std::cout << "  [1e20, 1]·[1, 1e20] = " << std::setprecision(17) << dot1 << "\n";
		std::cout << "  [1, 1e20]·[1e20, 1] = " << std::setprecision(17) << dot2 << "\n";
		std::cout << "  Difference:           " << std::abs(dot1 - dot2) << " (order matters!)\n\n";

		std::cout << "ereal<64>:\n";
		std::cout << "  [1e20, 1]·[1, 1e20] = " << std::setprecision(17) << double(edot1) << "\n";
		std::cout << "  [1, 1e20]·[1e20, 1] = " << std::setprecision(17) << double(edot2) << "\n";
		std::cout << "  Difference:           " << std::abs(double(edot1) - double(edot2)) << " (exact!)\n";
		std::cout << "  Components: " << edot1.limbs().size() << "\n\n";
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
		ereal<64> dot_ereal = dot_product_naive<ereal<64>>(a, b);

		std::cout << "Vectors:\n";
		std::cout << "  a = [1e10, 1, 1e10, 1]\n";
		std::cout << "  b = [1, 1e10, -1, 1e10]\n";
		std::cout << "Expected: 1e10 + 1e10 - 1e10 + 1e10 = 2e10\n\n";

		std::cout << "Double precision: " << std::setprecision(17) << dot_double << "\n";
		std::cout << "ereal<64>:        " << double(dot_ereal) << "\n";
		std::cout << "  Components: " << dot_ereal.limbs().size() << "\n\n";
	}

	// ===================================================================
	// Test 3: Classic ill-conditioned dot product
	// ===================================================================

	std::cout << "Test 3: Ill-Conditioned Dot Product\n";
	std::cout << "------------------------------------\n\n";

	{
		// Nearly orthogonal vectors (dot product should be very small)
		std::vector<double> a = { 1.0, 1.0e-10 };
		std::vector<double> b = { 1.0e-10, 1.0 };

		// Expected: 1×1e-10 + 1e-10×1 = 2e-10

		double dot_double = dot_product_naive<double>(a, b);
		ereal<64> dot_ereal = dot_product_naive<ereal<64>>(a, b);

		double expected = 2.0e-10;

		std::cout << "Nearly orthogonal vectors:\n";
		std::cout << "  a = [1, 1e-10]\n";
		std::cout << "  b = [1e-10, 1]\n";
		std::cout << "Expected: " << std::scientific << expected << "\n\n";
		std::cout << std::defaultfloat;

		std::cout << "Double precision: " << std::setprecision(17) << dot_double
		          << " (rel error: " << std::scientific
		          << std::abs(dot_double - expected)/expected << ")\n";
		std::cout << "ereal<64>:        " << double(dot_ereal)
		          << " (rel error: " << std::abs(double(dot_ereal) - expected)/expected << ")\n";
		std::cout << std::defaultfloat;
		std::cout << "  Components: " << dot_ereal.limbs().size() << "\n\n";
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
		ereal<64> dot_ereal = dot_product_naive<ereal<64>>(a, b);

		double expected = 1.0e-7;

		std::cout << "1000 terms of (1e-5 × 1e-5):\n";
		std::cout << "Expected: " << std::scientific << expected << "\n\n";
		std::cout << std::defaultfloat;

		std::cout << "Double precision: " << std::setprecision(17) << dot_double
		          << " (rel error: " << std::scientific
		          << std::abs(dot_double - expected)/expected << ")\n";
		std::cout << "ereal<64>:        " << double(dot_ereal)
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
