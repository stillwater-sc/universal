// roundtrip_3d.cpp: 3D float compress/decompress round-trip tests
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

// Verify reversible round-trip for 3D float
// Allow small relative error from lifting rounding across 3 dimensions.
int VerifyReversible3DFloat(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// 4x4x4 = 64 element block with smooth 3D data
	float input[64];
	for (int k = 0; k < 4; ++k)
		for (int j = 0; j < 4; ++j)
			for (int i = 0; i < 4; ++i)
				input[k * 16 + j * 4 + i] = static_cast<float>(i + j + k) * 0.1f;

	zfp3f blk;
	blk.compress_reversible(input);

	float output[64];
	blk.decompress(output);

	float max_val = 0.0f;
	for (int i = 0; i < 64; ++i) {
		if (std::abs(input[i]) > max_val) max_val = std::abs(input[i]);
	}
	float tol = (max_val > 0) ? max_val * 1.0e-5f : 1.0e-20f;

	for (int i = 0; i < 64; ++i) {
		float err = std::abs(output[i] - input[i]);
		if (err > tol) {
			std::cerr << tag << " FAIL: index " << i
			          << " expected=" << input[i]
			          << " got=" << output[i]
			          << " err=" << err << " tol=" << tol << '\n';
			++nrOfFailedTests;
		}
	}

	std::cout << tag << " compressed to " << blk.compressed_bits() << " bits"
	          << " (" << blk.compressed_bytes() << " bytes)"
	          << " ratio=" << blk.compression_ratio() << "x\n";

	return nrOfFailedTests;
}

// Verify fixed-rate compression for 3D float
int VerifyFixedRate3DFloat(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// 3D sinusoidal data
	float input[64];
	for (int k = 0; k < 4; ++k)
		for (int j = 0; j < 4; ++j)
			for (int i = 0; i < 4; ++i) {
				float x = static_cast<float>(i) * 0.5f;
				float y = static_cast<float>(j) * 0.5f;
				float z = static_cast<float>(k) * 0.5f;
				input[k * 16 + j * 4 + i] = std::sin(x) * std::cos(y) * std::sin(z);
			}

	double rates[] = { 2.0, 4.0, 8.0, 16.0 };
	for (double rate : rates) {
		zfp3f blk;
		size_t nbits = blk.compress_fixed_rate(input, rate);

		// verify exact bit count
		size_t expected_bits = static_cast<size_t>(rate * 64);
		if (nbits != expected_bits) {
			std::cerr << tag << " FAIL: rate=" << rate
			          << " expected " << expected_bits << " bits"
			          << " got " << nbits << " bits\n";
			++nrOfFailedTests;
		}

		float output[64];
		blk.decompress(output);

		double max_err = 0.0, rmse = 0.0;
		for (int i = 0; i < 64; ++i) {
			double err = static_cast<double>(output[i]) - static_cast<double>(input[i]);
			if (std::abs(err) > max_err) max_err = std::abs(err);
			rmse += err * err;
		}
		rmse = std::sqrt(rmse / 64.0);

		std::cout << tag << " rate=" << rate
		          << " bits=" << nbits
		          << " max_err=" << max_err
		          << " rmse=" << rmse
		          << " ratio=" << blk.compression_ratio() << "x\n";
	}

	return nrOfFailedTests;
}

// Verify 3D with constant block
int VerifyConstantBlock3DFloat(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	float input[64];
	for (int i = 0; i < 64; ++i) input[i] = 42.0f;

	zfp3f blk;
	blk.compress_reversible(input);

	float output[64];
	blk.decompress(output);

	// Constant blocks should round-trip well (lifting of constant = constant in DC)
	float tol = 42.0f * 1.0e-5f;
	for (int i = 0; i < 64; ++i) {
		float err = std::abs(output[i] - 42.0f);
		if (err > tol) {
			std::cerr << tag << " FAIL: index " << i
			          << " expected=42.0 got=" << output[i]
			          << " err=" << err << '\n';
			++nrOfFailedTests;
		}
	}

	std::cout << tag << " constant block: " << blk.compressed_bits() << " bits"
	          << " ratio=" << blk.compression_ratio() << "x\n";
	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfpblock 3D round-trip tests";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	nrOfFailedTestCases += VerifyReversible3DFloat("3D float reversible");
	nrOfFailedTestCases += VerifyFixedRate3DFloat("3D float fixed-rate");
	nrOfFailedTestCases += VerifyConstantBlock3DFloat("3D float constant");

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
