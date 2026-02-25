// fixed_rate.cpp: tests for ZFP fixed-rate compression mode
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

// Verify that fixed-rate produces exactly the specified number of bits
int VerifyExactBitCount1D(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	float input[4] = { 1.0f, -2.0f, 3.0f, -4.0f };

	// test various rates: bits per value
	// Minimum useful rate for float: header is 1+8=9 bits, so need rate*4 >= 9 → rate >= 2.25
	double rates[] = { 4.0, 8.0, 12.0, 16.0, 24.0, 32.0 };
	for (double rate : rates) {
		zfp1f blk;
		size_t nbits = blk.compress_fixed_rate(input, rate);
		size_t expected = static_cast<size_t>(rate * 4);  // 4 elements in 1D block

		if (nbits != expected) {
			std::cerr << tag << " FAIL: rate=" << rate
			          << " expected " << expected << " bits"
			          << " got " << nbits << " bits\n";
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

int VerifyExactBitCount2D(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	float input[16];
	for (int i = 0; i < 16; ++i) input[i] = static_cast<float>(i) * 0.3f;

	double rates[] = { 2.0, 4.0, 8.0, 16.0, 32.0 };
	for (double rate : rates) {
		zfp2f blk;
		size_t nbits = blk.compress_fixed_rate(input, rate);
		size_t expected = static_cast<size_t>(rate * 16);  // 16 elements in 2D block

		if (nbits != expected) {
			std::cerr << tag << " FAIL: rate=" << rate
			          << " expected " << expected << " bits"
			          << " got " << nbits << " bits\n";
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

int VerifyExactBitCount3D(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	float input[64];
	for (int i = 0; i < 64; ++i) input[i] = static_cast<float>(i) * 0.1f;

	double rates[] = { 1.0, 2.0, 4.0, 8.0, 16.0 };
	for (double rate : rates) {
		zfp3f blk;
		size_t nbits = blk.compress_fixed_rate(input, rate);
		size_t expected = static_cast<size_t>(rate * 64);  // 64 elements in 3D block

		if (nbits != expected) {
			std::cerr << tag << " FAIL: rate=" << rate
			          << " expected " << expected << " bits"
			          << " got " << nbits << " bits\n";
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// Verify that higher rate gives lower error
int VerifyRateVsError(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// smooth data that compresses well
	float input[16];
	for (int j = 0; j < 4; ++j)
		for (int i = 0; i < 4; ++i)
			input[j * 4 + i] = std::sin(static_cast<float>(i + j) * 0.3f);

	double rates[] = { 4.0, 8.0, 16.0, 32.0 };

	for (double rate : rates) {
		zfp2f blk;
		blk.compress_fixed_rate(input, rate);

		float output[16];
		blk.decompress(output);

		double rmse = 0.0;
		for (int i = 0; i < 16; ++i) {
			double err = static_cast<double>(output[i]) - static_cast<double>(input[i]);
			rmse += err * err;
		}
		rmse = std::sqrt(rmse / 16.0);

		std::cout << tag << " rate=" << rate
		          << " rmse=" << rmse
		          << " ratio=" << blk.compression_ratio() << "x\n";

	}

	return nrOfFailedTests;
}

// Verify compression ratio
int VerifyCompressionRatio(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	float input[4] = { 1.0f, 2.0f, 3.0f, 4.0f };

	// rate=8 means 8 bits/value → 32 bits total for 4 values
	// uncompressed = 4 * 32 = 128 bits → ratio = 128/32 = 4.0
	zfp1f blk;
	blk.compress_fixed_rate(input, 8.0);
	double ratio = blk.compression_ratio();
	double expected_ratio = (4.0 * 32.0) / 32.0;  // 4.0x

	if (std::abs(ratio - expected_ratio) > 0.01) {
		std::cerr << tag << " FAIL: expected ratio=" << expected_ratio
		          << " got=" << ratio << '\n';
		++nrOfFailedTests;
	} else {
		std::cout << tag << " compression ratio verified: " << ratio << "x\n";
	}

	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfpblock fixed-rate mode tests";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	nrOfFailedTestCases += VerifyExactBitCount1D("1D fixed-rate bit count");
	nrOfFailedTestCases += VerifyExactBitCount2D("2D fixed-rate bit count");
	nrOfFailedTestCases += VerifyExactBitCount3D("3D fixed-rate bit count");
	nrOfFailedTestCases += VerifyRateVsError("2D rate vs error");
	nrOfFailedTestCases += VerifyCompressionRatio("1D compression ratio");

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
