// quantize_int8.cpp: test suite for MXINT8 (int8_t element) quantization
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

	std::string test_suite = "mxint8 quantization tests";
	int nrOfFailedTestCases = 0;

	// Test 1: all-zeros round-trip
	std::cout << "+---------    all-zeros round-trip   --------+\n";
	{
		float input[32] = {};
		float output[32];
		mxint8 blk;
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

	// Test 2: verify quantize/dequantize preserves sign and order
	std::cout << "+---------    int8 sign and order preservation   --------+\n";
	{
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = static_cast<float>(i) - 16.0f; // range [-16, 15]
		}
		mxint8 blk;
		blk.quantize(input);
		blk.dequantize(output);
		// Verify sign preservation and monotonicity
		bool pass = true;
		for (int i = 0; i < 32; ++i) {
			// signs should match (or be zero)
			if (input[i] > 0.0f && output[i] < 0.0f) { pass = false; break; }
			if (input[i] < 0.0f && output[i] > 0.0f) { pass = false; break; }
		}
		// Verify monotonicity: output[i+1] >= output[i]
		for (int i = 0; i < 31; ++i) {
			if (output[i+1] < output[i] - 1e-6f) { pass = false; break; }
		}
		if (!pass) {
			std::cout << "FAIL: sign or order not preserved\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: int8 sign and order preservation\n";
		}
	}

	// Test 3: negative values (uniform)
	std::cout << "+---------    negative value round-trip   --------+\n";
	{
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = -5.0f;
		}
		mxint8 blk;
		blk.quantize(input);
		blk.dequantize(output);
		float maxErr = 0.0f;
		for (int i = 0; i < 32; ++i) {
			float err = std::fabs(output[i] - input[i]);
			if (err > maxErr) maxErr = err;
		}
		std::cout << "Max absolute error: " << maxErr << '\n';
		if (maxErr > 2.0f) {
			std::cout << "FAIL: negative value round-trip\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: negative value round-trip\n";
		}
	}

	// Test 4: scale computation for int8
	std::cout << "+---------    int8 scale computation   --------+\n";
	{
		float input[32] = {};
		input[0] = 128.0f; // amax = 128 = 2^7, floor(log2(128)) = 7
		// max_elem_exp for int8_t = 7
		// scale_exp = 7 - 7 = 0, biased = 127
		// scale = 2^0 = 1.0
		mxint8 blk;
		blk.quantize(input);
		float scale_val = blk.scale().to_float();
		if (std::fabs(scale_val - 1.0f) > 1e-6f) {
			std::cout << "FAIL: scale computation, expected 1.0, got " << scale_val << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: int8 scale computation (scale=" << scale_val << ")\n";
		}
	}

	// Test 5: clamping to [-128, 127]
	std::cout << "+---------    range clamping   --------+\n";
	{
		float input[32] = {};
		input[0] = 200.0f;   // exceeds int8 max of 127 in element space
		input[1] = -200.0f;  // exceeds int8 min of -128 in element space
		float output[32];
		mxint8 blk;
		blk.quantize(input);
		blk.dequantize(output);
		std::cout << "Input[0]=" << input[0] << " Output[0]=" << output[0] << '\n';
		std::cout << "Input[1]=" << input[1] << " Output[1]=" << output[1] << '\n';
		// The clamped values should be within the representable range
		bool pass = (output[0] > 0.0f) && (output[1] < 0.0f);
		if (!pass) {
			std::cout << "FAIL: range clamping\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: range clamping\n";
		}
	}

	// Test 6: NaN propagation
	std::cout << "+---------    NaN propagation   --------+\n";
	{
		mxint8 blk;
		blk.clear();
		blk.setbits(0xFF); // NaN scale
		float output[32];
		blk.dequantize(output);
		bool allNaN = true;
		for (int i = 0; i < 32; ++i) {
			if (output[i] == output[i]) { allNaN = false; break; }
		}
		if (!allNaN) {
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
