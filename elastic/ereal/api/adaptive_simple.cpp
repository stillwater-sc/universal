// adaptive_simple.cpp: simple validation of adaptive threshold utilities
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_mathlib_adaptive.hpp>

int main()
try {
	using namespace sw::universal;

	std::cout << "Adaptive Threshold Utilities - Simple Validation\n";
	std::cout << "=================================================\n\n";

	int nrOfFailedTestCases = 0;

	// Test different precision levels
	std::cout << "Precision and Threshold Scaling:\n";
	std::cout << "--------------------------------\n";

	double threshold_f = get_adaptive_threshold<float>();
	std::cout << "float   (digits10=" << std::numeric_limits<float>::digits10
	          << "): threshold = " << threshold_f << "\n";

	double threshold_d = get_adaptive_threshold<double>();
	std::cout << "double  (digits10=" << std::numeric_limits<double>::digits10
	          << "): threshold = " << threshold_d << "\n";

	using Real8 = ereal<8>;
	double threshold_e8 = get_adaptive_threshold<Real8>();
	std::cout << "ereal<8>  (digits10=" << std::numeric_limits<Real8>::digits10
	          << "): threshold = " << threshold_e8 << "\n";

	using Real12 = ereal<12>;
	double threshold_e12 = get_adaptive_threshold<Real12>();
	std::cout << "ereal<12> (digits10=" << std::numeric_limits<Real12>::digits10
	          << "): threshold = " << threshold_e12 << "\n";

	using Real16 = ereal<16>;
	double threshold_e16 = get_adaptive_threshold<Real16>();
	std::cout << "ereal<16> (digits10=" << std::numeric_limits<Real16>::digits10
	          << "): threshold = " << threshold_e16 << "\n";

	using Real19 = ereal<19>;
	double threshold_e19 = get_adaptive_threshold<Real19>();
	std::cout << "ereal<19> (digits10=" << std::numeric_limits<Real19>::digits10
	          << "): threshold = " << threshold_e19 << " (maximum valid)\n";

	// Verify thresholds scale properly
	std::cout << "\nThreshold Validation:\n";
	if (threshold_e19 < threshold_e16) {
		std::cout << "✓ PASS: ereal<19> has tighter threshold than ereal<16>\n";
	} else {
		std::cerr << "✗ FAIL: Threshold scaling incorrect\n";
		++nrOfFailedTestCases;
	}

	if (threshold_e16 < threshold_e12) {
		std::cout << "✓ PASS: ereal<16> has tighter threshold than ereal<12>\n";
	} else {
		std::cerr << "✗ FAIL: Threshold scaling incorrect\n";
		++nrOfFailedTestCases;
	}

	if (threshold_e12 < threshold_e8) {
		std::cout << "✓ PASS: ereal<12> has tighter threshold than ereal<8>\n";
	} else {
		std::cerr << "✗ FAIL: Threshold scaling incorrect\n";
		++nrOfFailedTestCases;
	}

	if (threshold_e16 < threshold_d) {
		std::cout << "✓ PASS: ereal<16> has tighter threshold than double\n";
	} else {
		std::cerr << "✗ FAIL: ereal<16> threshold should be tighter than double\n";
		++nrOfFailedTestCases;
	}

	// Test exact value checking
	std::cout << "\nExact Value Checking:\n";
	Real16 one(1.0);
	Real16 one_copy(1.0);
	if (check_exact_value(one, one_copy)) {
		std::cout << "✓ PASS: Exact values correctly identified as equal\n";
	} else {
		std::cerr << "✗ FAIL: Exact values not recognized as equal\n";
		++nrOfFailedTestCases;
	}

	// Test relative error checking with close values
	std::cout << "\nRelative Error Checking:\n";
	Real16 x(1.0);
	Real16 y(1.0 + 1e-20); // very close
	if (check_relative_error(x, y)) {
		std::cout << "✓ PASS: Very close values pass threshold check\n";
	} else {
		std::cerr << "✗ FAIL: Close values should pass threshold\n";
		++nrOfFailedTestCases;
	}

	Real16 z(100.0); // far from x
	if (!check_relative_error(x, z)) {
		std::cout << "✓ PASS: Distant values correctly rejected\n";
	} else {
		std::cerr << "✗ FAIL: Distant values should be rejected\n";
		++nrOfFailedTestCases;
	}

	// Summary
	std::cout << "\n=================================================\n";
	if (nrOfFailedTestCases == 0) {
		std::cout << "SUCCESS: All adaptive threshold tests passed!\n";
	} else {
		std::cout << "FAILED: " << nrOfFailedTestCases << " test(s) failed\n";
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
