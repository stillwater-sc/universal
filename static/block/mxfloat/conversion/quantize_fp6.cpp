// quantize_fp6.cpp: test suite for MXFP6 (e3m2 element) quantization
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

	std::string test_suite = "mxfp6 quantization tests";
	int nrOfFailedTestCases = 0;

	// Test 1: all-zeros round-trip
	std::cout << "+---------    all-zeros round-trip   --------+\n";
	{
		float input[32] = {};
		float output[32];
		mxfp6 blk;
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

	// Test 2: linear ramp
	std::cout << "+---------    linear ramp round-trip   --------+\n";
	{
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = static_cast<float>(i) * 0.25f;
		}
		mxfp6 blk;
		blk.quantize(input);
		blk.dequantize(output);
		float maxErr = 0.0f;
		for (int i = 0; i < 32; ++i) {
			float err = std::fabs(output[i] - input[i]);
			if (err > maxErr) maxErr = err;
		}
		// e3m2 has 2 fraction bits, so relative error should be modest
		std::cout << "Max absolute error: " << maxErr << '\n';
		if (maxErr > 2.0f) { // generous bound for block quantization
			std::cout << "FAIL: linear ramp error too large\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: linear ramp round-trip\n";
		}
	}

	// Test 3: scale computation for powers of 2
	std::cout << "+---------    power-of-2 scale computation   --------+\n";
	{
		float input[32] = {};
		input[0] = 16.0f; // amax = 16, floor(log2(16)) = 4
		// max_elem_exp for e3m2 = 4
		// scale_exp = 4 - 4 = 0, biased = 127
		// scale = 2^0 = 1.0
		mxfp6 blk;
		blk.quantize(input);
		float scale_val = blk.scale().to_float();
		if (std::fabs(scale_val - 1.0f) > 1e-6f) {
			std::cout << "FAIL: scale computation, expected 1.0, got " << scale_val << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: scale computation (scale=" << scale_val << ")\n";
		}
	}

	// Test 4: e2m3 variant
	std::cout << "+---------    mxfp6e2m3 round-trip   --------+\n";
	{
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = static_cast<float>(i) * 0.1f;
		}
		mxfp6e2m3 blk;
		blk.quantize(input);
		blk.dequantize(output);
		float maxErr = 0.0f;
		for (int i = 0; i < 32; ++i) {
			float err = std::fabs(output[i] - input[i]);
			if (err > maxErr) maxErr = err;
		}
		std::cout << "Max absolute error: " << maxErr << '\n';
		if (maxErr > 2.0f) {
			std::cout << "FAIL: e2m3 round-trip error too large\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: mxfp6e2m3 round-trip\n";
		}
	}

	// Test 5: subnormal/normal boundary
	std::cout << "+---------    subnormal boundary   --------+\n";
	{
		// Test values near the subnormal/normal boundary of e3m2
		float input[32] = {};
		// e3m2: bias = 3, subnormal range is v = f * 2^(1-3) = f * 2^(-2)
		// smallest subnormal: 0.25 * 2^(-2) = 0.0625
		// smallest normal: 1.0 * 2^(1-3) = 0.25
		input[0] = 0.0625f;
		input[1] = 0.125f;
		input[2] = 0.25f;
		input[3] = 0.5f;
		float output[32];
		mxfp6 blk;
		blk.quantize(input);
		blk.dequantize(output);
		std::cout << "Subnormal boundary values:\n";
		for (int i = 0; i < 4; ++i) {
			std::cout << "  input=" << input[i] << " output=" << output[i] << '\n';
		}
		// Basic sanity: output should be non-negative and roughly in range
		bool pass = true;
		for (int i = 0; i < 4; ++i) {
			if (output[i] < 0.0f) { pass = false; break; }
		}
		if (!pass) {
			std::cout << "FAIL: subnormal boundary\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: subnormal boundary\n";
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
