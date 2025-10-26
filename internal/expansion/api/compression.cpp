// compression.cpp: Tests for expansion compression operations
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

	// Test SCALE-EXPANSION
	int test_scale_expansion() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing SCALE-EXPANSION\n";

		// Test case 1: Scale by 2.0
		{
			std::vector<double> e = { 3.0, 5.0e-16 };
			double b = 2.0;

			std::vector<double> h = scale_expansion(e, b);

			// Verify value: h should equal e * b
			double e_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : h) h_sum += v;

			double expected = e_sum * b;
			if (std::abs(h_sum - expected) > 1.0e-14) ++nrOfFailedTests;
		}

		// Test case 2: Scale by 0.0
		{
			std::vector<double> e = { 10.0, 1.0e-15 };
			double b = 0.0;

			std::vector<double> h = scale_expansion(e, b);

			if (h.size() != 1) ++nrOfFailedTests;
			if (h[0] != 0.0) ++nrOfFailedTests;
		}

		// Test case 3: Scale by 1.0 (should return unchanged)
		{
			std::vector<double> e = { 7.0, 3.5e-16 };
			double b = 1.0;

			std::vector<double> h = scale_expansion(e, b);

			if (h.size() != e.size()) ++nrOfFailedTests;
			for (size_t i = 0; i < e.size(); ++i) {
				if (h[i] != e[i]) ++nrOfFailedTests;
			}
		}

		// Test case 4: Scale by -1.0 (negation)
		{
			std::vector<double> e = { 5.0, 2.5e-16 };
			double b = -1.0;

			std::vector<double> h = scale_expansion(e, b);

			if (h.size() != e.size()) ++nrOfFailedTests;
			for (size_t i = 0; i < e.size(); ++i) {
				if (h[i] != -e[i]) ++nrOfFailedTests;
			}
		}

		// Test case 5: Scale by fractional value
		{
			std::vector<double> e = { 10.0, 1.0e-15 };
			double b = 0.3;

			std::vector<double> h = scale_expansion(e, b);

			double e_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : h) h_sum += v;

			double expected = e_sum * b;
			if (std::abs(h_sum - expected) > 1.0e-13) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test COMPRESS-EXPANSION
	int test_compress_expansion() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing COMPRESS-EXPANSION\n";

		// Test case 1: Remove exact zeros
		{
			std::vector<double> e = { 10.0, 0.0, 1.0e-15, 0.0, 5.0e-30 };

			std::vector<double> h = compress_expansion(e, 0.0);

			// Should remove the two 0.0 components
			if (h.size() >= e.size()) ++nrOfFailedTests;

			// Verify no zeros remain
			for (auto v : h) {
				if (v == 0.0) ++nrOfFailedTests;
			}
		}

		// Test case 2: Aggressive compression (relative threshold)
		{
			std::vector<double> e = { 1.0, 1.0e-10, 1.0e-20, 1.0e-30 };

			// Remove components < 1e-15 * largest
			std::vector<double> h = compress_expansion(e, 1.0e-15);

			// Should keep 1.0 and 1.0e-10, discard 1.0e-20 and 1.0e-30
			if (h.size() > 2) ++nrOfFailedTests;

			// Verify largest components are preserved
			if (h[0] != 1.0) ++nrOfFailedTests;
		}

		// Test case 3: All zeros
		{
			std::vector<double> e = { 0.0, 0.0, 0.0 };

			std::vector<double> h = compress_expansion(e, 0.0);

			// Should return single zero
			if (h.size() != 1) ++nrOfFailedTests;
			if (h[0] != 0.0) ++nrOfFailedTests;
		}

		// Test case 4: No compression needed
		{
			std::vector<double> e = { 5.0, 2.5, 1.25 };

			std::vector<double> h = compress_expansion(e, 0.0);

			// Should return unchanged (no zeros)
			if (h.size() != e.size()) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test COMPRESS-TO-N
	int test_compress_to_n() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing COMPRESS-TO-N\n";

		// Test case 1: Reduce to 2 components
		{
			std::vector<double> e = { 10.0, 1.0, 0.1, 0.01, 0.001 };

			std::vector<double> h = compress_to_n(e, 2);

			if (h.size() != 2) ++nrOfFailedTests;
			if (h[0] != 10.0) ++nrOfFailedTests;
			if (h[1] != 1.0) ++nrOfFailedTests;
		}

		// Test case 2: Request more than available
		{
			std::vector<double> e = { 5.0, 2.5 };

			std::vector<double> h = compress_to_n(e, 10);

			// Should return unchanged
			if (h.size() != e.size()) ++nrOfFailedTests;
		}

		// Test case 3: Compress to 1 (keep only most significant)
		{
			std::vector<double> e = { 100.0, 1.0e-10, 1.0e-20 };

			std::vector<double> h = compress_to_n(e, 1);

			if (h.size() != 1) ++nrOfFailedTests;
			if (h[0] != 100.0) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test SIGN-ADAPTIVE
	int test_sign_adaptive() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing SIGN-ADAPTIVE\n";

		// Test case 1: Positive expansion
		{
			std::vector<double> e = { 10.0, 1.0e-15 };
			int sign = sign_adaptive(e);
			if (sign != 1) ++nrOfFailedTests;
		}

		// Test case 2: Negative expansion
		{
			std::vector<double> e = { -5.0, -2.5e-16 };
			int sign = sign_adaptive(e);
			if (sign != -1) ++nrOfFailedTests;
		}

		// Test case 3: Zero expansion
		{
			std::vector<double> e = { 0.0, 0.0, 0.0 };
			int sign = sign_adaptive(e);
			if (sign != 0) ++nrOfFailedTests;
		}

		// Test case 4: Leading zeros (adaptive!)
		{
			std::vector<double> e = { 0.0, 0.0, 1.0e-100 };
			int sign = sign_adaptive(e);
			// Should find the tiny positive component
			if (sign != 1) ++nrOfFailedTests;
		}

		// Test case 5: Mixed signs (most significant wins)
		{
			std::vector<double> e = { 10.0, -1.0e-15 };
			int sign = sign_adaptive(e);
			// First component is positive, so result is positive
			if (sign != 1) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test COMPARE-ADAPTIVE
	int test_compare_adaptive() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing COMPARE-ADAPTIVE\n";

		// Test case 1: e > f
		{
			std::vector<double> e = { 10.0, 1.0e-15 };
			std::vector<double> f = { 5.0, 2.5e-15 };

			int cmp = compare_adaptive(e, f);
			if (cmp != 1) ++nrOfFailedTests;
		}

		// Test case 2: e < f
		{
			std::vector<double> e = { 3.0, 1.5e-16 };
			std::vector<double> f = { 10.0, 5.0e-16 };

			int cmp = compare_adaptive(e, f);
			if (cmp != -1) ++nrOfFailedTests;
		}

		// Test case 3: e == f
		{
			std::vector<double> e = { 7.0, 3.5e-16 };
			std::vector<double> f = { 7.0, 3.5e-16 };

			int cmp = compare_adaptive(e, f);
			if (cmp != 0) ++nrOfFailedTests;
		}

		// Test case 4: Different sizes, same value
		{
			std::vector<double> e = { 5.0 };
			std::vector<double> f = { 5.0, 0.0, 0.0 };

			int cmp = compare_adaptive(e, f);
			if (cmp != 0) ++nrOfFailedTests;
		}

		// Test case 5: Early termination test
		{
			// First component differs - should terminate immediately
			std::vector<double> e = { 100.0, 50.0, 25.0, 12.5 };
			std::vector<double> f = { 99.0, 50.0, 25.0, 12.5 };

			int cmp = compare_adaptive(e, f);
			// e > f because first component is larger
			if (cmp != 1) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
int main()
try {
	using namespace sw::universal;

	std::cout << "Expansion Compression & Adaptive Operations Tests\n";
	std::cout << "==================================================\n\n";

	int nrOfFailedTests = 0;

	nrOfFailedTests += test_scale_expansion();
	nrOfFailedTests += test_compress_expansion();
	nrOfFailedTests += test_compress_to_n();
	nrOfFailedTests += test_sign_adaptive();
	nrOfFailedTests += test_compare_adaptive();

	std::cout << "\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All compression and adaptive tests passed\n";
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
