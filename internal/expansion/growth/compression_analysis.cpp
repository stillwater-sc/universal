// compression_analysis.cpp: Analyze expansion compression effectiveness
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

	// Helper: Sum expansion components
	double sum_expansion(const std::vector<double>& e) {
		double sum = 0.0;
		for (auto v : e) sum += v;
		return sum;
	}

	// Helper: Compute difference between two expansions (as expansion)
	// This preserves the precision difference that would be lost if we summed first
	std::vector<double> subtract_expansions(const std::vector<double>& a, const std::vector<double>& b) {
		using namespace expansion_ops;
		std::vector<double> neg_b = b;
		for (auto& v : neg_b) v = -v;
		return linear_expansion_sum(a, neg_b);
	}

	// Helper: Compute relative error between two expansions
	// Key: Compute difference AS EXPANSION first, then sum
	// This preserves precision that would be lost if we summed each expansion to double first
	double compute_relative_error(const std::vector<double>& full, const std::vector<double>& compressed) {
		// Compute difference as expansion (preserves precision!)
		std::vector<double> diff = subtract_expansions(full, compressed);

		// Sum the difference (error should be small enough for double)
		double error = sum_expansion(diff);

		// Sum the original for the denominator
		double full_val = sum_expansion(full);

		return std::abs(error) / std::abs(full_val);
	}

	// Helper: Print expansion info
	void print_expansion_info(const std::string& name, const std::vector<double>& e) {
		std::cout << "  " << name << ": " << e.size() << " components";
		if (e.size() <= 8) {
			std::cout << " [";
			for (size_t i = 0; i < e.size(); ++i) {
				if (i > 0) std::cout << ", ";
				std::cout << std::setprecision(6) << std::scientific << e[i];
			}
			std::cout << "]";
		}
		std::cout << "\n";
	}

	// ===================================================================
	// THRESHOLD-BASED COMPRESSION
	// ===================================================================

	int test_threshold_compression() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing THRESHOLD COMPRESSION: Remove components below threshold\n";

		// Test case 1: Compress expansion with tiny tail
		{
			// Create expansion: 1/3
			std::vector<double> one = { 1.0 };
			std::vector<double> three = { 3.0 };
			std::vector<double> third = expansion_quotient(one, three);

			double original_val = sum_expansion(third);
			size_t original_size = third.size();

			// Compress with threshold 1e-30
			std::vector<double> compressed = compress_expansion(third, 1.0e-30);

			double compressed_val = sum_expansion(compressed);
			size_t compressed_size = compressed.size();

			std::cout << "  1/3: " << original_size << " → " << compressed_size << " components\n";
			std::cout << "    Value change: " << std::setprecision(17)
			          << std::abs(original_val - compressed_val) << "\n";

			// Verify compression happened
			if (compressed_size >= original_size) {
				std::cout << "    WARNING: No compression occurred\n";
			}

			// Verify value approximately preserved
			if (std::abs(original_val - compressed_val) / original_val > 1.0e-20) {
				std::cout << "    FAIL: Too much precision lost\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: Aggressive compression
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> seven = { 7.0 };
			std::vector<double> seventh = expansion_quotient(one, seven);

			size_t original_size = seventh.size();

			// Very aggressive threshold
			std::vector<double> compressed = compress_expansion(seventh, 1.0e-15);

			size_t compressed_size = compressed.size();

			std::cout << "  1/7 (aggressive): " << original_size << " → "
			          << compressed_size << " components\n";

			// Should have removed many components
			if (compressed_size >= original_size - 1) {
				std::cout << "    WARNING: Aggressive compression didn't remove enough\n";
			}

			// Calculate relative error (properly!)
			double rel_error = compute_relative_error(seventh, compressed);
			std::cout << "    Relative error: " << std::setprecision(6)
			          << std::scientific << rel_error << "\n";
		}

		// Test case 3: Conservative compression (should remove nothing)
		{
			std::vector<double> e = { 10.0, 1.0e-15 };
			size_t original_size = e.size();

			// Very conservative threshold (keep everything)
			std::vector<double> compressed = compress_expansion(e, 1.0e-50);

			size_t compressed_size = compressed.size();

			if (compressed_size != original_size) {
				std::cout << "  FAIL: Conservative compression removed components\n";
				std::cout << "    " << original_size << " → " << compressed_size << "\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ Conservative threshold preserves all components\n";
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Threshold compression works correctly\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// COUNT-BASED COMPRESSION
	// ===================================================================

	int test_count_compression() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting COUNT COMPRESSION: Keep N most significant components\n";

		// Test case 1: Compress to specific count
		{
			// Create multi-component expansion
			std::vector<double> one = { 1.0 };
			std::vector<double> three = { 3.0 };
			std::vector<double> third = expansion_quotient(one, three);

			size_t original_size = third.size();

			// Compress to 4 components
			size_t target = 4;
			std::vector<double> compressed = compress_to_n(third, target);

			size_t compressed_size = compressed.size();

			std::cout << "  1/3: " << original_size << " → " << compressed_size
			          << " components (target: " << target << ")\n";

			if (compressed_size > target) {
				std::cout << "    FAIL: Compression didn't reach target\n";
				++nrOfFailedTests;
			}

			double rel_error = compute_relative_error(third, compressed);
			std::cout << "    Relative error: " << std::setprecision(6)
			          << std::scientific << rel_error << "\n";
		}

		// Test case 2: Compress to 1 (extreme)
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> seven = { 7.0 };
			std::vector<double> seventh = expansion_quotient(one, seven);

			size_t original_size = seventh.size();

			// Compress to just 1 component (most significant)
			std::vector<double> compressed = compress_to_n(seventh, 1);

			std::cout << "  1/7: " << original_size << " → " << compressed.size()
			          << " component (extreme compression)\n";

			// Should be approximately the first component
			double rel_error = compute_relative_error(seventh, compressed);
			std::cout << "    Relative error: " << std::setprecision(6)
			          << std::scientific << rel_error << "\n";

			if (compressed.size() != 1) {
				std::cout << "    FAIL: Didn't compress to 1 component\n";
				++nrOfFailedTests;
			}
		}

		// Test case 3: Target larger than actual (no-op)
		{
			std::vector<double> e = { 10.0, 1.0e-15 };
			size_t original_size = e.size();

			// Ask for more components than we have
			std::vector<double> compressed = compress_to_n(e, 10);

			if (compressed.size() != original_size) {
				std::cout << "  FAIL: Compression changed size when target > size\n";
				++nrOfFailedTests;
			}
			else {
				std::cout << "  ✓ Compress to N>size is no-op\n";
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Count compression works correctly\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// PRECISION LOSS MEASUREMENT
	// ===================================================================

	int test_precision_loss() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting PRECISION LOSS: Measure accuracy after compression\n";

		// Test case 1: Gradual compression of 1/3
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> three = { 3.0 };
			std::vector<double> full = expansion_quotient(one, three);

			size_t full_size = full.size();

			std::cout << "  1/3 precision loss with compression:\n";
			std::cout << "    Full: " << full_size << " components\n";

			// Compress to different levels
			for (size_t n : {6, 4, 2, 1}) {
				if (n <= full_size) {
					std::vector<double> compressed = compress_to_n(full, n);
					double rel_error = compute_relative_error(full, compressed);

					std::cout << "    " << n << " components: error = "
					          << std::setprecision(6) << std::scientific << rel_error << "\n";
				}
			}
		}

		// Test case 2: Verify precision improves with more components
		{
			std::vector<double> one = { 1.0 };
			std::vector<double> seven = { 7.0 };
			std::vector<double> full = expansion_quotient(one, seven);

			double prev_error = 1.0;

			std::cout << "\n  1/7 error decreases with component count:\n";

			for (size_t n = 1; n <= full.size(); ++n) {
				std::vector<double> compressed = compress_to_n(full, n);
				double rel_error = compute_relative_error(full, compressed);

				// Error should decrease (or stay same) as we add components
				if (rel_error > prev_error && n > 1) {
					std::cout << "    WARNING: Error increased from " << n-1
					          << " to " << n << " components\n";
				}

				if (n == 1 || n == full.size() || (n % 2 == 0)) {
					std::cout << "    " << n << " components: error = "
					          << std::setprecision(6) << std::scientific << rel_error << "\n";
				}

				prev_error = rel_error;
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Precision loss measured successfully\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// COMPRESSION BENEFIT ANALYSIS
	// ===================================================================

	int test_when_to_compress() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting COMPRESSION BENEFITS: When is compression worthwhile?\n";

		// Test case 1: Already compact (no benefit)
		{
			std::vector<double> e = { 10.0 };
			std::vector<double> compressed = compress_expansion(e, 1.0e-30);

			if (compressed.size() != e.size()) {
				std::cout << "  WARNING: Compressed already-compact expansion\n";
			}
			else {
				std::cout << "  ✓ Already-compact expansion unchanged\n";
			}
		}

		// Test case 2: Many tiny components (good candidate)
		{
			// Create expansion with accumulation of tiny values
			std::vector<double> sum = { 1.0 };
			for (int i = 0; i < 5; ++i) {
				std::vector<double> tiny = { 1.0e-20 };
				sum = linear_expansion_sum(sum, tiny);
			}

			size_t before = sum.size();
			std::vector<double> compressed = compress_expansion(sum, 1.0e-19);
			size_t after = compressed.size();

			std::cout << "  Accumulation of tiny values: " << before
			          << " → " << after << " components\n";

			if (after < before) {
				std::cout << "    ✓ Compression beneficial ("
				          << (100 * (before - after) / before) << "% reduction)\n";
			}
		}

		// Test case 3: All significant components (compression harmful)
		{
			std::vector<double> e = { 1.0, 0.5, 0.25, 0.125 };
			double original_val = sum_expansion(e);

			// Try aggressive compression
			std::vector<double> compressed = compress_expansion(e, 0.1);
			double compressed_val = sum_expansion(compressed);

			double rel_error = std::abs(original_val - compressed_val) / original_val;

			if (rel_error > 0.01) {
				std::cout << "  ✓ Compressing significant components loses precision ("
				          << std::setprecision(2) << (rel_error * 100) << "%)\n";
			}
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Compression benefit analysis complete\n";
		}

		return nrOfFailedTests;
	}

	// ===================================================================
	// COMPRESSION AFTER OPERATIONS
	// ===================================================================

	int test_compress_after_operations() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "\nTesting COMPRESSION AFTER OPERATIONS: Clean up operation results\n";

		// Test case 1: After long accumulation
		{
			std::vector<double> sum = { 0.0 };

			// Add many small values
			for (int i = 0; i < 20; ++i) {
				std::vector<double> val = { 1.0 };
				sum = linear_expansion_sum(sum, val);
			}

			size_t before = sum.size();

			// Should be just 20.0, but might have accumulated components
			std::vector<double> compressed = compress_expansion(sum, 1.0e-14);

			size_t after = compressed.size();

			std::cout << "  Sum of 20 integers: " << before << " → "
			          << after << " components\n";

			double rel_error = compute_relative_error(sum, compressed);
			if (rel_error > 1.0e-13) {
				std::cout << "    FAIL: Compression changed value significantly\n";
				std::cout << "    Relative error: " << std::setprecision(6)
				          << std::scientific << rel_error << "\n";
				++nrOfFailedTests;
			}
		}

		// Test case 2: After multiplication
		{
			// (1/3) × (1/7)
			std::vector<double> one = { 1.0 };
			std::vector<double> third = expansion_quotient(one, { 3.0 });
			std::vector<double> seventh = expansion_quotient(one, { 7.0 });

			std::vector<double> product = expansion_product(third, seventh);

			size_t before = product.size();

			// Compress moderately
			std::vector<double> compressed = compress_to_n(product, 8);

			size_t after = compressed.size();

			std::cout << "  (1/3) × (1/7): " << before << " → "
			          << after << " components\n";

			double rel_error = compute_relative_error(product, compressed);
			std::cout << "    Relative error: " << std::setprecision(6)
			          << std::scientific << rel_error << "\n";
		}

		if (nrOfFailedTests == 0) {
			std::cout << "  PASS: Post-operation compression works correctly\n";
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
int main()
try {
	using namespace sw::universal;

	std::cout << "========================================================\n";
	std::cout << "Expansion Compression Analysis Tests\n";
	std::cout << "========================================================\n";

	int nrOfFailedTests = 0;

	nrOfFailedTests += test_threshold_compression();
	nrOfFailedTests += test_count_compression();
	nrOfFailedTests += test_precision_loss();
	nrOfFailedTests += test_when_to_compress();
	nrOfFailedTests += test_compress_after_operations();

	std::cout << "\n========================================================\n";
	if (nrOfFailedTests > 0) {
		std::cout << "FAILED: " << nrOfFailedTests << " tests failed\n";
	}
	else {
		std::cout << "SUCCESS: All compression analysis tests passed\n";
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
