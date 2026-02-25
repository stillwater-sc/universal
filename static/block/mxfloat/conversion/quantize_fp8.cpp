// quantize_fp8.cpp: test suite for MXFP8 (e4m3 and e5m2 element) quantization
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

	std::string test_suite = "mxfp8 quantization tests";
	int nrOfFailedTestCases = 0;

	// Test 1: e4m3 round-trip with linear ramp
	std::cout << "+---------    mxfp8 (e4m3) linear ramp   --------+\n";
	{
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = static_cast<float>(i) * 0.5f - 8.0f;
		}
		mxfp8 blk;
		blk.quantize(input);
		blk.dequantize(output);
		float maxErr = 0.0f;
		for (int i = 0; i < 32; ++i) {
			float err = std::fabs(output[i] - input[i]);
			if (err > maxErr) maxErr = err;
		}
		std::cout << "Max absolute error: " << maxErr << '\n';
		if (maxErr > 1.0f) {
			std::cout << "FAIL: e4m3 linear ramp error too large\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: mxfp8 (e4m3) linear ramp\n";
		}
	}

	// Test 2: e5m2 round-trip with linear ramp
	std::cout << "+---------    mxfp8e5m2 linear ramp   --------+\n";
	{
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = static_cast<float>(i) * 0.5f - 8.0f;
		}
		mxfp8e5m2 blk;
		blk.quantize(input);
		blk.dequantize(output);
		float maxErr = 0.0f;
		for (int i = 0; i < 32; ++i) {
			float err = std::fabs(output[i] - input[i]);
			if (err > maxErr) maxErr = err;
		}
		std::cout << "Max absolute error: " << maxErr << '\n';
		if (maxErr > 2.0f) {
			std::cout << "FAIL: e5m2 linear ramp error too large\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: mxfp8e5m2 linear ramp\n";
		}
	}

	// Test 3: e4m3 scale correctness
	std::cout << "+---------    e4m3 scale correctness   --------+\n";
	{
		float input[32] = {};
		input[0] = 256.0f; // amax = 256 = 2^8, floor(log2(256)) = 8
		// max_elem_exp for e4m3 = 8
		// scale_exp = 8 - 8 = 0, biased = 127
		// scale = 2^0 = 1.0
		mxfp8 blk;
		blk.quantize(input);
		float scale_val = blk.scale().to_float();
		if (std::fabs(scale_val - 1.0f) > 1e-6f) {
			std::cout << "FAIL: scale computation, expected 1.0, got " << scale_val << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: e4m3 scale computation (scale=" << scale_val << ")\n";
		}
	}

	// Test 4: NaN propagation with e4m3
	std::cout << "+---------    NaN propagation (e4m3)   --------+\n";
	{
		mxfp8 blk;
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

	// Test 5: e5m2 NaN propagation
	std::cout << "+---------    NaN propagation (e5m2)   --------+\n";
	{
		mxfp8e5m2 blk;
		blk.clear();
		blk.setbits(0xFF); // NaN scale
		float output[32];
		blk.dequantize(output);
		bool allNaN = true;
		for (int i = 0; i < 32; ++i) {
			if (output[i] == output[i]) { allNaN = false; break; }
		}
		if (!allNaN) {
			std::cout << "FAIL: e5m2 NaN propagation\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: e5m2 NaN propagation\n";
		}
	}

	// Test 6: e4m3 all-ones input (near maxpos)
	std::cout << "+---------    e4m3 large values   --------+\n";
	{
		float input[32];
		float output[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = 100.0f;
		}
		mxfp8 blk;
		blk.quantize(input);
		blk.dequantize(output);
		// All values should be roughly the same
		float ref = output[0];
		bool pass = true;
		for (int i = 1; i < 32; ++i) {
			if (std::fabs(output[i] - ref) > 1e-6f) {
				pass = false;
				break;
			}
		}
		if (!pass) {
			std::cout << "FAIL: uniform large values not consistent\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: e4m3 large values (output=" << ref << ")\n";
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
