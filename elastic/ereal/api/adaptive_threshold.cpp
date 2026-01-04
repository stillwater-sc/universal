// adaptive_threshold.cpp: test suite for adaptive threshold utilities
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_mathlib_adaptive.hpp>

namespace sw {
namespace universal {
	template<typename EReal>
	int adaptive_threshold() {
		int    nrOfFailedTestCases = 0;
	    EReal  v(0.0);
	    double threshold_double    = get_adaptive_threshold<double>();
	    std::cout << "  double threshold (digits10=" << std::numeric_limits<double>::digits10
	              << "): " << threshold_double << "\n";

	    double threshold = get_adaptive_threshold<EReal>();
	    std::cout << type_tag(v) << " threshold (digits10=" << std::numeric_limits<EReal>::digits10
	              << "): " << threshold
				  << "\n";

		// ereal should have tighter threshold if it has more precision
	    if (std::numeric_limits<EReal>::digits10 > std::numeric_limits<double>::digits10) {
			if (threshold >= threshold_double) {
			    std::cerr << "FAIL: " << type_tag(v)
			              << " should have tighter threshold than double\n ";
				++nrOfFailedTestCases;
			}
		}
	    return nrOfFailedTestCases;
	}
}}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "ereal adaptive threshold utilities";
	int nrOfFailedTestCases = 0;

	std::cout << "Adaptive Threshold Utilities Test\n";
	std::cout << "==================================\n";

	// Test 1: Verify adaptive thresholds scale with precision
	{
		std::cout << "\nTest 1: Adaptive threshold scaling\n";
	
		nrOfFailedTestCases += adaptive_threshold<ereal<8>>();
		nrOfFailedTestCases += adaptive_threshold<ereal<12>>();
		nrOfFailedTestCases += adaptive_threshold<ereal<16>>();
		nrOfFailedTestCases += adaptive_threshold<ereal<19>>();  
		// 19 limbs is the max number of limbs for the expansion algebra 
		// to still adhere to the two_sum theorem: a + b = fl(a + b) + err
		// and err is representable in the ereal<19> format
		// 19 * 53 = 1007 -> 2^1007 = 1.3e303 -> max double is 1.7e308
		// 20 * 53 = 1060 -> 2^1060 = 1.4e319 -> exceeds max double

		// would this mean that actually can use twice as many limbs?
		// it is the range, not the max precision that is the limiting factor
	}

	// Test 2: Verify exact value checking
	{
		std::cout << "\nTest 2: Exact value checking\n";

		using Real = ereal<16>;
		Real one(1.0);
		Real also_one(1.0);
		Real not_quite_one(1.0000001);

		if (!check_exact_value(one, also_one)) {
			std::cerr << "FAIL: exact value check should pass for identical values\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "  PASS: exact value check for identical values\n";
		}

		if (check_exact_value(one, not_quite_one)) {
			std::cerr << "FAIL: exact value check should fail for different values\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "  PASS: exact value check correctly rejects different values\n";
		}
	}

	// Test 3: Verify relative error checking
	{
		std::cout << "\nTest 3: Relative error checking\n";

		using Real = ereal<16>;
		Real x(1.0);
		Real y(1.0 + 1e-16); // very close to x

		if (!check_relative_error(x, y)) {
			std::cerr << "FAIL: relative error check should pass for very close values\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "  PASS: relative error check for close values\n";
		}

		Real z(2.0); // far from x
		if (check_relative_error(x, z)) {
			std::cerr << "FAIL: relative error check should fail for distant values\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "  PASS: relative error check correctly rejects distant values\n";
		}
	}

	// Test 4: Verify identity checking
	{
		std::cout << "\nTest 4: Identity checking\n";

		using Real = ereal<16>;
		Real x(1.5);
		Real lhs = x * x;  // x²
		Real rhs = x * x;  // should be identical

		int failures = verify_identity("x² == x²", lhs, rhs, 0.0, false);
		if (failures != 0) {
			std::cerr << "FAIL: identity check failed for identical expressions\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "  PASS: identity check for identical expressions\n";
		}
	}

	// Test 5: Error reporting (manual check - should print nicely formatted output)
	{
		std::cout << "\nTest 5: Error reporting format\n";
		std::cout << "  (The following is a test of error reporting format, not a real failure)\n";

		using Real = ereal<16>;
		Real result(2.71828);
		Real expected(2.71829);
		double threshold = get_adaptive_threshold<Real>();

		std::cout << "  Sample error report:\n";
		std::cout << "  --------------------\n";
		report_error_detail("exp", "1.0", result, expected, threshold, true);
		std::cout << "  --------------------\n";
	}

	std::cout << "\n";
	if (nrOfFailedTestCases > 0) {
		std::cout << "FAILED: " << nrOfFailedTestCases << " test case(s)\n";
	} else {
		std::cout << "SUCCESS: All adaptive threshold utility tests passed\n";
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
