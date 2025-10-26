// accurate_summation.cpp: demonstration of accurate summation with ereal
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
 * ACCURATE SUMMATION: The challenge of adding many floating-point numbers
 *
 * Problem: When summing many values, rounding errors accumulate.
 * Worse: Summing small values into a large accumulator loses precision.
 *
 * Classical solutions:
 * - Kahan summation (compensated summation)
 * - Pairwise summation
 * - Sort values by magnitude
 *
 * Adaptive precision solution:
 * - Use ereal - automatically maintains all precision!
 */

namespace sw { namespace universal {

	// Naive summation
	template<typename Real>
	Real naive_sum(const std::vector<double>& values) {
		Real sum(0.0);
		for (double v : values) {
			sum = sum + Real(v);
		}
		return sum;
	}

	// Kahan compensated summation
	double kahan_sum(const std::vector<double>& values) {
		double sum = 0.0;
		double compensation = 0.0;

		for (double v : values) {
			double y = v - compensation;
			double t = sum + y;
			compensation = (t - sum) - y;
			sum = t;
		}
		return sum;
	}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "============================================================\n";
	std::cout << "Accurate Summation: ereal vs. double vs. Kahan\n";
	std::cout << "============================================================\n\n";

	// ===================================================================
	// Test 1: Sum many small values (classic rounding error accumulation)
	// ===================================================================

	std::cout << "Test 1: Sum 10,000 copies of 0.1\n";
	std::cout << "---------------------------------\n\n";

	{
		std::vector<double> values(10000, 0.1);
		double expected = 10000 * 0.1;  // Should be 1000.0

		double naive = naive_sum<double>(values);
		double kahan = kahan_sum(values);
		ereal<64> ereal_result = naive_sum<ereal<64>>(values);

		std::cout << "Expected:             " << std::setprecision(17) << expected << "\n";
		std::cout << "Naive (double):       " << naive << " (error: " << std::abs(naive - expected) << ")\n";
		std::cout << "Kahan (double):       " << kahan << " (error: " << std::abs(kahan - expected) << ")\n";
		std::cout << "ereal<64>:            " << double(ereal_result) << " (error: " << std::abs(double(ereal_result) - expected) << ")\n";
		std::cout << "  Components: " << ereal_result.limbs().size() << "\n\n";
	}

	// ===================================================================
	// Test 2: Sum alternating large and small values
	// ===================================================================

	std::cout << "Test 2: Sum 1e10 + 1 + (-1e10) + 1 (repeated 1000 times)\n";
	std::cout << "---------------------------------------------------------\n\n";

	{
		std::vector<double> values;
		for (int i = 0; i < 1000; ++i) {
			values.push_back(1.0e10);
			values.push_back(1.0);
			values.push_back(-1.0e10);
			values.push_back(1.0);
		}
		double expected = 2000.0;  // Should be 2 * 1000

		double naive = naive_sum<double>(values);
		double kahan = kahan_sum(values);
		ereal<64> ereal_result = naive_sum<ereal<64>>(values);

		std::cout << "Expected:             " << std::setprecision(17) << expected << "\n";
		std::cout << "Naive (double):       " << naive << " (error: " << std::abs(naive - expected) << ")\n";
		std::cout << "Kahan (double):       " << kahan << " (error: " << std::abs(kahan - expected) << ")\n";
		std::cout << "ereal<64>:            " << double(ereal_result) << " (error: " << std::abs(double(ereal_result) - expected) << ")\n";
		std::cout << "  Components: " << ereal_result.limbs().size() << "\n\n";
	}

	// ===================================================================
	// Test 3: Sum many tiny values into large accumulator
	// ===================================================================

	std::cout << "Test 3: 1e20 + sum(1000 × 1e10)\n";
	std::cout << "--------------------------------\n\n";

	{
		std::vector<double> values;
		values.push_back(1.0e20);
		for (int i = 0; i < 1000; ++i) {
			values.push_back(1.0e10);
		}
		double expected = 1.0e20 + 1000 * 1.0e10;  // 1.0e20 + 1.0e13

		double naive = naive_sum<double>(values);
		double kahan = kahan_sum(values);
		ereal<64> ereal_result = naive_sum<ereal<64>>(values);

		std::cout << "Expected:             " << std::setprecision(17) << expected << "\n";
		std::cout << "Naive (double):       " << naive << " (error: " << std::abs(naive - expected) << ")\n";
		std::cout << "Kahan (double):       " << kahan << " (error: " << std::abs(kahan - expected) << ")\n";
		std::cout << "ereal<64>:            " << double(ereal_result) << " (error: " << std::abs(double(ereal_result) - expected) << ")\n";
		std::cout << "  Components: " << ereal_result.limbs().size() << "\n\n";
	}

	// ===================================================================
	// Test 4: Worst case for Kahan (requires multiple compensations)
	// ===================================================================

	std::cout << "Test 4: [1e30, 1, -1e30, 1, ...] × 500 (Kahan worst case)\n";
	std::cout << "-----------------------------------------------------------\n\n";

	{
		std::vector<double> values;
		for (int i = 0; i < 500; ++i) {
			values.push_back(1.0e30);
			values.push_back(1.0);
			values.push_back(-1.0e30);
			values.push_back(1.0);
		}
		double expected = 1000.0;

		double naive = naive_sum<double>(values);
		double kahan = kahan_sum(values);
		ereal<64> ereal_result = naive_sum<ereal<64>>(values);

		std::cout << "Expected:             " << std::setprecision(17) << expected << "\n";
		std::cout << "Naive (double):       " << naive << " (error: " << std::abs(naive - expected) << ")\n";
		std::cout << "Kahan (double):       " << kahan << " (error: " << std::abs(kahan - expected) << ")\n";
		std::cout << "ereal<64>:            " << double(ereal_result) << " (error: " << std::abs(double(ereal_result) - expected) << ")\n";
		std::cout << "  Components: " << ereal_result.limbs().size() << "\n\n";
	}

	// ===================================================================
	// SUMMARY
	// ===================================================================

	std::cout << "============================================================\n";
	std::cout << "COMPARISON SUMMARY\n";
	std::cout << "============================================================\n\n";

	std::cout << "Naive Summation (double):\n";
	std::cout << "  + Simple to implement\n";
	std::cout << "  - Accumulates rounding errors\n";
	std::cout << "  - Loses small values when added to large accumulator\n";
	std::cout << "  - Order-dependent results\n\n";

	std::cout << "Kahan Summation (double):\n";
	std::cout << "  + Reduces many rounding errors\n";
	std::cout << "  + More accurate than naive\n";
	std::cout << "  - Still limited by double precision\n";
	std::cout << "  - More complex implementation\n";
	std::cout << "  - Can still fail on pathological cases\n\n";

	std::cout << "Adaptive Precision (ereal):\n";
	std::cout << "  + Simple naive summation works perfectly!\n";
	std::cout << "  + Maintains exact precision (within representation)\n";
	std::cout << "  + Order-independent (mathematically)\n";
	std::cout << "  + No algorithm tricks needed\n";
	std::cout << "  - Grows component count (but stays manageable)\n\n";

	std::cout << "Use ereal when:\n";
	std::cout << "  - Summing many values (especially alternating signs)\n";
	std::cout << "  - Mixing vastly different scales\n";
	std::cout << "  - Result precision is critical\n";
	std::cout << "  - You want simple, obviously correct code\n\n";

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
