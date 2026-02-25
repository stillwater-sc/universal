// quantize_nvfp4.cpp: test suite for NVFP4 (e2m1 element, e4m3 scale) quantization
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/nvblock/nvblock.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "nvfp4 quantization tests";
	int nrOfFailedTestCases = 0;

	// Test 1: all-zeros round-trip
	std::cout << "+---------    all-zeros round-trip   --------+\n";
	{
		float input[16] = {};
		float output[16];
		nvfp4 blk;
		blk.quantize(input, 1.0f);
		blk.dequantize(output, 1.0f);
		bool pass = true;
		for (int i = 0; i < 16; ++i) {
			if (output[i] != 0.0f) { pass = false; break; }
		}
		if (!pass) {
			std::cout << "FAIL: all-zeros round-trip\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: all-zeros round-trip\n";
		}
	}

	// Test 2: uniform value round-trip
	std::cout << "+---------    uniform value round-trip   --------+\n";
	{
		float input[16];
		float output[16];
		for (int i = 0; i < 16; ++i) input[i] = 1.0f;
		nvfp4 blk;
		blk.quantize(input, 1.0f);
		blk.dequantize(output, 1.0f);
		bool pass = true;
		for (int i = 0; i < 16; ++i) {
			if (std::fabs(output[i] - 1.0f) > 0.6f) { // e2m1 has limited precision
				pass = false;
				break;
			}
		}
		if (!pass) {
			std::cout << "FAIL: uniform value round-trip\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: uniform value round-trip\n";
		}
	}

	// Test 3: mixed positive and negative values
	std::cout << "+---------    mixed sign round-trip   --------+\n";
	{
		float input[16];
		float output[16];
		for (int i = 0; i < 16; ++i) {
			input[i] = (i % 2 == 0) ? 2.0f : -2.0f;
		}
		nvfp4 blk;
		blk.quantize(input, 1.0f);
		blk.dequantize(output, 1.0f);
		bool pass = true;
		for (int i = 0; i < 16; ++i) {
			float expected = (i % 2 == 0) ? 2.0f : -2.0f;
			if (std::fabs(output[i] - expected) > 1.0f) {
				pass = false;
				break;
			}
		}
		if (!pass) {
			std::cout << "FAIL: mixed sign round-trip\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: mixed sign round-trip\n";
		}
	}

	// Test 4: scale computation correctness
	std::cout << "+---------    scale computation   --------+\n";
	{
		float input[16] = {};
		input[0] = 6.0f; // amax = 6.0, elem_max for e2m1 = 6.0
		// raw_scale = 6.0 / 6.0 = 1.0
		// e4m3 of 1.0 should be exactly 1.0
		nvfp4 blk;
		blk.quantize(input, 1.0f);
		float scale_val = blk.block_scale().to_float();
		if (std::fabs(scale_val - 1.0f) > 1e-6f) {
			std::cout << "FAIL: scale computation, expected 1.0, got " << scale_val << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: scale computation (scale=" << scale_val << ")\n";
		}
	}

	// Test 5: tensor_scale round-trip
	std::cout << "+---------    tensor_scale round-trip   --------+\n";
	{
		float input[16] = {};
		input[0] = 100.0f;
		input[1] = -50.0f;
		float tensor_scale = 50.0f;
		float output[16];
		nvfp4 blk;
		blk.quantize(input, tensor_scale);
		blk.dequantize(output, tensor_scale);
		// After pre-dividing by 50: [2.0, -1.0, 0, ...]
		// These are exactly representable in e2m1
		std::cout << "Input[0]=" << input[0] << " Output[0]=" << output[0] << '\n';
		std::cout << "Input[1]=" << input[1] << " Output[1]=" << output[1] << '\n';
		if (std::fabs(output[0] - 100.0f) > 20.0f) {
			std::cout << "FAIL: tensor_scale round-trip\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: tensor_scale round-trip\n";
		}
	}

	// Test 6: partial block (n < BlockSize)
	std::cout << "+---------    partial block   --------+\n";
	{
		float input[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, -3.0f, -6.0f};
		float output[16];
		nvfp4 blk;
		blk.quantize(input, 1.0f, 8);
		blk.dequantize(output, 1.0f, 16);
		// Remaining elements should be zero
		bool pass = true;
		for (int i = 8; i < 16; ++i) {
			if (output[i] != 0.0f) { pass = false; break; }
		}
		if (!pass) {
			std::cout << "FAIL: partial block zero-fill\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: partial block zero-fill\n";
		}
	}

	// Test 7: NaN propagation from scale
	std::cout << "+---------    NaN scale propagation   --------+\n";
	{
		nvfp4 blk;
		blk.clear();
		blk.setscalebits(0x7F); // e4m3 NaN
		float output[16];
		blk.dequantize(output, 1.0f);
		bool pass = true;
		for (int i = 0; i < 16; ++i) {
			if (output[i] == output[i]) { pass = false; break; } // NaN != NaN
		}
		if (!pass) {
			std::cout << "FAIL: NaN propagation\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: NaN propagation\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
