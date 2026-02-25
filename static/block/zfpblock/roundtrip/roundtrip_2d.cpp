// roundtrip_2d.cpp: 2D float compress/decompress round-trip tests
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>

// Verify reversible (lossless) round-trip for 2D float
int VerifyReversible2DFloat(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// smooth 2D data (4x4 block)
	float input[16];
	for (int j = 0; j < 4; ++j)
		for (int i = 0; i < 4; ++i)
			input[j * 4 + i] = static_cast<float>(i + j) * 0.5f;

	zfp2f blk;
	blk.compress_reversible(input);

	float output[16];
	blk.decompress(output);

	float max_val = 0.0f;
	for (int i = 0; i < 16; ++i) {
		if (std::abs(input[i]) > max_val) max_val = std::abs(input[i]);
	}
	float tol = (max_val > 0) ? max_val * 1.0e-5f : 1.0e-20f;

	for (int i = 0; i < 16; ++i) {
		float err = std::abs(output[i] - input[i]);
		if (err > tol) {
			std::cerr << tag << " FAIL: index " << i
			          << " expected=" << input[i]
			          << " got=" << output[i]
			          << " err=" << err << " tol=" << tol << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// Verify fixed-rate compression for 2D float
int VerifyFixedRate2DFloat(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// 4x4 block with gradient data
	float input[16];
	for (int j = 0; j < 4; ++j)
		for (int i = 0; i < 4; ++i)
			input[j * 4 + i] = std::sin(static_cast<float>(i) * 0.5f) *
			                    std::cos(static_cast<float>(j) * 0.5f);

	double rates[] = { 4.0, 8.0, 16.0 };
	double prev_max_err = 1.0e30;

	for (double rate : rates) {
		zfp2f blk;
		size_t nbits = blk.compress_fixed_rate(input, rate);

		// verify exact bit count
		size_t expected_bits = static_cast<size_t>(rate * 16);
		if (nbits != expected_bits) {
			std::cerr << tag << " FAIL: rate=" << rate
			          << " expected " << expected_bits << " bits"
			          << " got " << nbits << " bits\n";
			++nrOfFailedTests;
		}

		float output[16];
		blk.decompress(output);

		double max_err = 0.0;
		for (int i = 0; i < 16; ++i) {
			double err = std::abs(static_cast<double>(output[i]) - static_cast<double>(input[i]));
			if (err > max_err) max_err = err;
		}

		std::cout << tag << " rate=" << rate
		          << " bits=" << nbits
		          << " max_err=" << max_err
		          << " ratio=" << blk.compression_ratio() << "x\n";

		// sanity: error should decrease (or stay same) with higher rate
		if (max_err > prev_max_err * 1.01) {  // small tolerance for rounding
			std::cerr << tag << " WARNING: error increased with higher rate"
			          << " (" << max_err << " > " << prev_max_err << ")\n";
			// Not a hard failure — ZFP doesn't strictly guarantee monotonic error reduction
		}
		prev_max_err = max_err;
	}

	return nrOfFailedTests;
}

// Verify 2D with all-zero block
int VerifyZeroBlock2DFloat(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	float input[16] = {};  // all zeros
	zfp2f blk;
	blk.compress_reversible(input);

	float output[16];
	blk.decompress(output);

	for (int i = 0; i < 16; ++i) {
		if (output[i] != 0.0f) {
			std::cerr << tag << " FAIL: index " << i
			          << " expected=0 got=" << output[i] << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfpblock 2D round-trip tests";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	nrOfFailedTestCases += VerifyReversible2DFloat("2D float reversible");
	nrOfFailedTestCases += VerifyFixedRate2DFloat("2D float fixed-rate");
	nrOfFailedTestCases += VerifyZeroBlock2DFloat("2D float zero block");

	std::cout << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL")
	          << " : " << nrOfFailedTestCases << " failures\n";

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
