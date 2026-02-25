// quantize_fp4.cpp: test suite for MXFP4 (e2m1 element) quantization
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "mxfp4 quantization tests";
	int nrOfFailedTestCases = 0;

	// Test 1: all-zeros round-trip
	std::cout << "+---------    all-zeros round-trip   --------+\n";
	{
		float input[32] = {};
		float output[32];
		mxfp4 blk;
		blk.quantize(input);
		blk.dequantize(output);
		bool pass = true;
		for (int i = 0; i < 32; ++i) {
			if (output[i] != 0.0f) { pass = false; break; }
		}
		if (!pass) {
			std::cout << "FAIL: all-zeros round-trip\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: all-zeros round-trip\n";
		}
	}

	// Test 2: all same value
	std::cout << "+---------    uniform value round-trip   --------+\n";
	{
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) input[i] = 1.0f;
		mxfp4 blk;
		blk.quantize(input);
		blk.dequantize(output);
		bool pass = true;
		for (int i = 0; i < 32; ++i) {
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
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = (i % 2 == 0) ? 2.0f : -2.0f;
		}
		mxfp4 blk;
		blk.quantize(input);
		blk.dequantize(output);
		bool pass = true;
		for (int i = 0; i < 32; ++i) {
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
		float input[32] = {};
		input[0] = 4.0f; // amax = 4.0, floor(log2(4)) = 2
		// max_elem_exp for e2m1 = 2
		// scale_exp = 2 - 2 = 0, biased = 0 + 127 = 127
		// scale should represent 2^0 = 1.0
		mxfp4 blk;
		blk.quantize(input);
		float scale_val = blk.scale().to_float();
		if (std::fabs(scale_val - 1.0f) > 1e-6f) {
			std::cout << "FAIL: scale computation, expected 1.0, got " << scale_val << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: scale computation (scale=" << scale_val << ")\n";
		}
	}

	// Test 5: large dynamic range
	std::cout << "+---------    large dynamic range   --------+\n";
	{
		float input[32] = {};
		input[0] = 1000.0f;
		input[1] = -500.0f;
		input[2] = 0.001f;
		float output[32];
		mxfp4 blk;
		blk.quantize(input);
		blk.dequantize(output);
		// With only 4-bit elements, small values get quantized to zero
		// but large values should be roughly preserved
		std::cout << "Input[0]=" << input[0] << " Output[0]=" << output[0] << '\n';
		std::cout << "Input[1]=" << input[1] << " Output[1]=" << output[1] << '\n';
		std::cout << "Input[2]=" << input[2] << " Output[2]=" << output[2] << '\n';
		if (std::fabs(output[0]) < 100.0f) {
			std::cout << "FAIL: large value not preserved\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: large dynamic range\n";
		}
	}

	// Test 6: NaN propagation from scale
	std::cout << "+---------    NaN scale propagation   --------+\n";
	{
		mxfp4 blk;
		blk.clear();
		blk.setbits(0xFF); // NaN scale
		float output[32];
		blk.dequantize(output);
		bool pass = true;
		for (int i = 0; i < 32; ++i) {
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
