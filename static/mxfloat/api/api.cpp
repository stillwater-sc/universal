// api.cpp: application programming interface tests for mxblock (MX block float) number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the mxfloat template environment
#define MXFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "mxfloat API tests";
	int nrOfFailedTestCases = 0;

	// demonstrate all 6 mxblock type aliases
	std::cout << "+---------    mxblock type aliases   --------+\n";
	{
		mxfp4 a;
		std::cout << "mxfp4     : " << type_tag(a) << '\n';

		mxfp6 b;
		std::cout << "mxfp6     : " << type_tag(b) << '\n';

		mxfp6e2m3 c;
		std::cout << "mxfp6e2m3 : " << type_tag(c) << '\n';

		mxfp8 d;
		std::cout << "mxfp8     : " << type_tag(d) << '\n';

		mxfp8e5m2 e;
		std::cout << "mxfp8e5m2 : " << type_tag(e) << '\n';

		mxint8 f;
		std::cout << "mxint8    : " << type_tag(f) << '\n';
	}

	// quantize and dequantize round-trip
	std::cout << "+---------    quantize/dequantize round-trip   --------+\n";
	{
		float input[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = static_cast<float>(i) * 0.1f;
		}

		mxfp8 blk;
		blk.quantize(input);
		std::cout << "mxfp8 scale : " << blk.scale() << '\n';
		std::cout << "First 8 dequantized values:\n";
		for (int i = 0; i < 8; ++i) {
			std::cout << "  [" << i << "] input=" << input[i] << " output=" << blk[i] << '\n';
		}
	}

	// to_binary display
	std::cout << "+---------    to_binary display   --------+\n";
	{
		float input[32] = {};
		input[0] = 1.0f;
		input[1] = 2.0f;
		input[2] = -1.0f;
		input[3] = 0.5f;

		mxfp8 blk;
		blk.quantize(input);
		std::cout << to_binary(blk) << '\n';
	}

	// block dot product
	std::cout << "+---------    block dot product   --------+\n";
	{
		float a_input[32], b_input[32];
		float ref_dot = 0.0f;
		for (int i = 0; i < 32; ++i) {
			a_input[i] = static_cast<float>(i + 1);
			b_input[i] = 1.0f / static_cast<float>(i + 1);
			ref_dot += a_input[i] * b_input[i];
		}

		mxfp8 a, b;
		a.quantize(a_input);
		b.quantize(b_input);
		float mx_dot = a.dot(b);
		std::cout << "FP32 reference dot product : " << ref_dot << '\n';
		std::cout << "mxfp8 block dot product    : " << mx_dot << '\n';
		std::cout << "Relative error             : " << std::fabs(mx_dot - ref_dot) / std::fabs(ref_dot) << '\n';
	}

	// all-zeros input
	std::cout << "+---------    all-zeros input   --------+\n";
	{
		float zeros[32] = {};
		mxfp8 blk;
		blk.quantize(zeros);
		std::cout << "All-zeros scale : " << blk.scale() << '\n';
		bool allZero = true;
		for (size_t i = 0; i < 32; ++i) {
			if (blk[i] != 0.0f) { allZero = false; break; }
		}
		if (allZero) {
			std::cout << "All-zeros test PASSED\n";
		} else {
			std::cout << "All-zeros test FAILED\n";
			++nrOfFailedTestCases;
		}
	}

	// NaN propagation: scale = 0xFF
	std::cout << "+---------    NaN propagation   --------+\n";
	{
		mxfp8 blk;
		blk.clear();
		blk.setbits(0xFF); // NaN scale
		bool allNaN = true;
		for (size_t i = 0; i < 32; ++i) {
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

	// mxfp4 (smallest element type)
	std::cout << "+---------    mxfp4 quantization   --------+\n";
	{
		float input[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = static_cast<float>(i) - 16.0f;
		}
		mxfp4 blk;
		blk.quantize(input);
		std::cout << "mxfp4 scale : " << blk.scale() << '\n';
		std::cout << "First 8 values:\n";
		for (int i = 0; i < 8; ++i) {
			std::cout << "  [" << i << "] input=" << input[i] << " output=" << blk[i] << '\n';
		}
	}

	// mxint8 (integer element type)
	std::cout << "+---------    mxint8 quantization   --------+\n";
	{
		float input[32];
		for (int i = 0; i < 32; ++i) {
			input[i] = static_cast<float>(i) * 0.5f;
		}
		mxint8 blk;
		blk.quantize(input);
		std::cout << "mxint8 scale : " << blk.scale() << '\n';
		std::cout << "First 8 values:\n";
		for (int i = 0; i < 8; ++i) {
			std::cout << "  [" << i << "] input=" << input[i] << " output=" << blk[i] << '\n';
		}
	}

	// dynamic ranges
	std::cout << "+---------    dynamic ranges   --------+\n";
	{
		std::cout << mxblock_range<e2m1, 32>() << '\n';
		std::cout << mxblock_range<e3m2, 32>() << '\n';
		std::cout << mxblock_range<e2m3, 32>() << '\n';
		std::cout << mxblock_range<e4m3, 32>() << '\n';
		std::cout << mxblock_range<e5m2, 32>() << '\n';
		std::cout << mxblock_range<int8_t, 32>() << '\n';
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
