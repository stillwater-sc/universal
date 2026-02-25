// api.cpp: application programming interface tests for nvblock (NVIDIA NVFP4 block float) number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the nvblock template environment
#define NVBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/nvblock/nvblock.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "nvblock API tests";
	int nrOfFailedTestCases = 0;

	// demonstrate the nvfp4 type alias
	std::cout << "+---------    nvblock type alias   --------+\n";
	{
		nvfp4 a;
		std::cout << "nvfp4 : " << type_tag(a) << '\n';

		// non-canonical configuration
		nvblock<e3m2, 16, e4m3> b;
		std::cout << "custom: " << type_tag(b) << '\n';
	}

	// quantize and dequantize round-trip with tensor_scale = 1.0
	std::cout << "+---------    quantize/dequantize round-trip (tensor_scale=1.0)   --------+\n";
	{
		float input[16];
		for (int i = 0; i < 16; ++i) {
			input[i] = static_cast<float>(i) * 0.1f;
		}

		nvfp4 blk;
		blk.quantize(input, 1.0f);
		std::cout << "nvfp4 block_scale : " << blk.block_scale().to_float() << '\n';
		std::cout << "First 8 dequantized values:\n";
		for (int i = 0; i < 8; ++i) {
			std::cout << "  [" << i << "] input=" << input[i] << " output=" << blk[i] << '\n';
		}
	}

	// quantize with non-trivial tensor_scale
	std::cout << "+---------    quantize with tensor_scale=100.0   --------+\n";
	{
		float input[16];
		for (int i = 0; i < 16; ++i) {
			input[i] = static_cast<float>(i) * 10.0f; // range [0, 150]
		}
		float tensor_scale = 100.0f;

		nvfp4 blk;
		blk.quantize(input, tensor_scale);
		float output[16];
		blk.dequantize(output, tensor_scale);
		std::cout << "tensor_scale=" << tensor_scale << " block_scale=" << blk.block_scale().to_float() << '\n';
		for (int i = 0; i < 8; ++i) {
			std::cout << "  [" << i << "] input=" << input[i] << " output=" << output[i] << '\n';
		}
	}

	// to_binary display
	std::cout << "+---------    to_binary display   --------+\n";
	{
		float input[16] = {};
		input[0] = 1.0f;
		input[1] = 2.0f;
		input[2] = -1.0f;
		input[3] = 0.5f;

		nvfp4 blk;
		blk.quantize(input, 1.0f);
		std::cout << to_binary(blk) << '\n';
	}

	// block dot product with tensor scales
	std::cout << "+---------    block dot product   --------+\n";
	{
		float a_input[16], b_input[16];
		float ref_dot = 0.0f;
		for (int i = 0; i < 16; ++i) {
			a_input[i] = static_cast<float>(i + 1);
			b_input[i] = 1.0f / static_cast<float>(i + 1);
			ref_dot += a_input[i] * b_input[i];
		}

		nvfp4 a, b;
		a.quantize(a_input, 1.0f);
		b.quantize(b_input, 1.0f);
		float nv_dot = a.dot(b, 1.0f, 1.0f);
		std::cout << "FP32 reference dot product : " << ref_dot << '\n';
		std::cout << "nvfp4 block dot product    : " << nv_dot << '\n';
		std::cout << "Relative error             : " << std::fabs(nv_dot - ref_dot) / std::fabs(ref_dot) << '\n';
	}

	// all-zeros input
	std::cout << "+---------    all-zeros input   --------+\n";
	{
		float zeros[16] = {};
		nvfp4 blk;
		blk.quantize(zeros, 1.0f);
		std::cout << "All-zeros block_scale : " << blk.block_scale().to_float() << '\n';
		bool allZero = true;
		float output[16];
		blk.dequantize(output, 1.0f);
		for (size_t i = 0; i < 16; ++i) {
			if (output[i] != 0.0f) { allZero = false; break; }
		}
		if (allZero) {
			std::cout << "All-zeros test PASSED\n";
		} else {
			std::cout << "All-zeros test FAILED\n";
			++nrOfFailedTestCases;
		}
	}

	// NaN propagation: e4m3 NaN scale (0x7F)
	std::cout << "+---------    NaN propagation   --------+\n";
	{
		nvfp4 blk;
		blk.clear();
		blk.setscalebits(0x7F); // e4m3 NaN encoding
		bool allNaN = true;
		for (size_t i = 0; i < 16; ++i) {
			float v = blk[i];
			if (v == v) { allNaN = false; break; } // NaN != NaN
		}
		if (allNaN) {
			std::cout << "NaN propagation test PASSED\n";
		} else {
			std::cout << "NaN propagation test FAILED\n";
			++nrOfFailedTestCases;
		}
	}

	// tensor_scale = 0 edge case
	std::cout << "+---------    tensor_scale=0 edge case   --------+\n";
	{
		float input[16];
		for (int i = 0; i < 16; ++i) input[i] = static_cast<float>(i + 1);
		nvfp4 blk;
		blk.quantize(input, 0.0f);
		float output[16];
		blk.dequantize(output, 0.0f);
		bool allZero = true;
		for (size_t i = 0; i < 16; ++i) {
			if (output[i] != 0.0f) { allZero = false; break; }
		}
		if (allZero) {
			std::cout << "tensor_scale=0 test PASSED (all outputs zero)\n";
		} else {
			std::cout << "tensor_scale=0 test FAILED\n";
			++nrOfFailedTestCases;
		}
	}

	// dynamic range
	std::cout << "+---------    dynamic range   --------+\n";
	{
		std::cout << nvblock_range<e2m1, 16, e4m3>() << '\n';
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
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
