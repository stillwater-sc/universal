// catastrophic_cancellation.cpp: demonstration of catastrophic cancellation avoidance with ereal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

/*
 * CATASTROPHIC CANCELLATION: When subtracting nearly equal numbers
 *
 * Problem: (large + small) - large should equal small, but in fixed
 * precision arithmetic, the small value is often completely lost.
 *
 * Example: (10²⁰ + 1) - 10²⁰
 *   - In double precision: Result is 0 (wrong!)
 *   - With ereal: Result is 1 (correct!)
 *
 * This happens because:
 * 1. 10²⁰ + 1 rounds to 10²⁰ (loses the +1)
 * 2. 10²⁰ - 10²⁰ = 0
 * 3. The small component is catastrophically lost
 */

int main()
try {
	using namespace sw::universal;

	std::cout << "============================================================\n";
	std::cout << "Catastrophic Cancellation: ereal vs. double\n";
	std::cout << "============================================================\n\n";

	// ===================================================================
	// Example 1: (1e20 + 1) - 1e20 = 1
	// ===================================================================

	std::cout << "Example 1: (1e20 + 1) - 1e20 = 1\n";
	std::cout << "-----------------------------------\n\n";

	{
		std::cout << "Double precision:\n";
		double large = 1.0e20;
		double small = 1.0;
		double sum = large + small;
		double result = sum - large;

		std::cout << "  1e20 + 1     = " << std::setprecision(20) << sum << "\n";
		std::cout << "  (1e20+1)-1e20= " << result << "\n";
		std::cout << "  Expected:      1.0\n";
		std::cout << "  Error:         " << std::abs(result - 1.0) << " (100% loss!)\n\n";
	}

	{
		std::cout << "Adaptive precision (ereal<16>):\n";
		ereal<16> large(1.0e20);
		ereal<16> small(1.0);
		ereal<16> sum = large + small;
		ereal<16> result = sum - large;

		std::cout << "  1e20 + 1     = " << sum.limbs().size() << " components\n";
		std::cout << "  (1e20+1)-1e20= " << std::setprecision(20) << double(result) << "\n";
		std::cout << "  Expected:      1.0\n";
		std::cout << "  Error:         " << std::abs(double(result) - 1.0) << " (perfect!)\n\n";
	}

	// ===================================================================
	// Example 2: (1 + 1e-15) - 1 = 1e-15
	// ===================================================================

	std::cout << "Example 2: (1 + 1e-15) - 1 = 1e-15 (extreme precision)\n";
	std::cout << "-------------------------------------------------------\n\n";

	{
		std::cout << "Double precision:\n";
		double one = 1.0;
		double tiny = 1.0e-15;
		double sum = one + tiny;
		double result = sum - one;

		std::cout << "  1 + 1e-15    = " << std::setprecision(20) << sum << "\n";
		std::cout << "  (1+1e-15)-1  = " << std::scientific << result << "\n";
		std::cout << "  Expected:      " << tiny << "\n";
		double rel_error = std::abs(result - tiny) / tiny;
		std::cout << "  Relative error:" << rel_error << "\n\n";
		std::cout << std::defaultfloat;
	}

	{
		std::cout << "Adaptive precision (ereal<16>):\n";
		ereal<16> one(1.0);
		ereal<16> tiny(1.0e-15);
		ereal<16> sum = one + tiny;
		ereal<16> result = sum - one;

		std::cout << "  1 + 1e-15    = " << sum.limbs().size() << " components\n";
		std::cout << "  (1+1e-15)-1  = " << std::scientific << double(result) << "\n";
		std::cout << "  Expected:      " << 1.0e-15 << "\n";
		double rel_error = std::abs(double(result) - 1.0e-15) / 1.0e-15;
		std::cout << "  Relative error:" << rel_error << " (perfect!)\n\n";
		std::cout << std::defaultfloat;
	}

	// ===================================================================
	// Example 3: Multiple scale operations
	// ===================================================================

	std::cout << "Example 3: Mixed-scale arithmetic: 1e100 + 1e-100 - 1e100\n";
	std::cout << "---------------------------------------------------------\n\n";

	{
		std::cout << "Double precision:\n";
		double huge = 1.0e100;
		double minuscule = 1.0e-100;
		double sum = huge + minuscule;
		double result = sum - huge;

		std::cout << "  1e100 + 1e-100     = " << sum << "\n";
		std::cout << "  (sum) - 1e100      = " << std::scientific << result << "\n";
		std::cout << "  Expected:            " << minuscule << "\n";
		std::cout << "  Error: Complete loss (100%)\n\n";
		std::cout << std::defaultfloat;
	}

	{
		std::cout << "Adaptive precision (ereal<16>):\n";
		ereal<16> huge(1.0e100);
		ereal<16> minuscule(1.0e-100);
		ereal<16> sum = huge + minuscule;
		ereal<16> result = sum - huge;

		std::cout << "  1e100 + 1e-100     = " << sum.limbs().size() << " components\n";
		std::cout << "  (sum) - 1e100      = " << std::scientific << double(result) << "\n";
		std::cout << "  Expected:            " << 1.0e-100 << "\n";
		std::cout << "  Components in result:" << result.limbs().size() << "\n";
		std::cout << "  Result: Small value preserved!\n\n";
		std::cout << std::defaultfloat;
	}

	// ===================================================================
	// SUMMARY
	// ===================================================================

	std::cout << "============================================================\n";
	std::cout << "KEY INSIGHT\n";
	std::cout << "============================================================\n\n";

	std::cout << "Fixed precision (double):\n";
	std::cout << "  - Small components lost when combined with large values\n";
	std::cout << "  - (large + small) rounds to large\n";
	std::cout << "  - Subtraction catastrophically loses precision\n\n";

	std::cout << "Adaptive precision (ereal):\n";
	std::cout << "  - Each component stored separately in expansion\n";
	std::cout << "  - No precision loss during addition/subtraction\n";
	std::cout << "  - Component count grows to preserve all information\n\n";

	std::cout << "Use ereal when:\n";
	std::cout << "  - Working with vastly different scales (1e100 + 1e-100)\n";
	std::cout << "  - Subtracting nearly equal numbers\n";
	std::cout << "  - Accumulating many small values\n";
	std::cout << "  - Precision loss would invalidate results\n\n";

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
