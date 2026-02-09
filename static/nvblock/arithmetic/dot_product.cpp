// dot_product.cpp: test suite for nvblock block dot product with dual tensor scales
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

	std::string test_suite = "nvblock dot product tests";
	int nrOfFailedTestCases = 0;

	// Test 1: dot product of unit vectors
	std::cout << "+---------    unit vectors dot product   --------+\n";
	{
		float a_input[16], b_input[16];
		for (int i = 0; i < 16; ++i) {
			a_input[i] = 1.0f;
			b_input[i] = 1.0f;
		}
		float ref_dot = 16.0f;

		nvfp4 a, b;
		a.quantize(a_input, 1.0f);
		b.quantize(b_input, 1.0f);
		float nv_dot = a.dot(b, 1.0f, 1.0f);
		float rel_err = std::fabs(nv_dot - ref_dot) / std::fabs(ref_dot);
		std::cout << "Reference: " << ref_dot << " NV: " << nv_dot << " RelErr: " << rel_err << '\n';
		if (rel_err > 0.1f) {
			std::cout << "FAIL: unit vector dot product\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: unit vector dot product\n";
		}
	}

	// Test 2: orthogonal vectors (dot = 0)
	std::cout << "+---------    orthogonal vectors   --------+\n";
	{
		float a_input[16] = {};
		float b_input[16] = {};
		for (int i = 0; i < 16; ++i) {
			if (i % 2 == 0) {
				a_input[i] = 1.0f;
				b_input[i] = 0.0f;
			} else {
				a_input[i] = 0.0f;
				b_input[i] = 1.0f;
			}
		}
		float ref_dot = 0.0f;

		nvfp4 a, b;
		a.quantize(a_input, 1.0f);
		b.quantize(b_input, 1.0f);
		float nv_dot = a.dot(b, 1.0f, 1.0f);
		std::cout << "Reference: " << ref_dot << " NV: " << nv_dot << '\n';
		if (std::fabs(nv_dot) > 1.0f) {
			std::cout << "FAIL: orthogonal vectors dot product\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: orthogonal vectors dot product\n";
		}
	}

	// Test 3: dot product with tensor scales
	// Use inputs in a range that, after dividing by tensor_scale, land well within e2m1 range
	std::cout << "+---------    dot product with tensor scales   --------+\n";
	{
		float a_input[16], b_input[16];
		float scale_a = 2.0f;
		float scale_b = 3.0f;
		float ref_dot = 0.0f;
		for (int i = 0; i < 16; ++i) {
			a_input[i] = 4.0f;  // 4.0/2.0 = 2.0, well within e2m1 range
			b_input[i] = 6.0f;  // 6.0/3.0 = 2.0, well within e2m1 range
			ref_dot += a_input[i] * b_input[i]; // 4*6=24 per element, total=384
		}

		nvfp4 a, b;
		a.quantize(a_input, scale_a);
		b.quantize(b_input, scale_b);
		float nv_dot = a.dot(b, scale_a, scale_b);
		float rel_err = std::fabs(nv_dot - ref_dot) / std::fabs(ref_dot);
		std::cout << "Reference: " << ref_dot << " NV: " << nv_dot << " RelErr: " << rel_err << '\n';
		if (rel_err > 0.2f) {
			std::cout << "FAIL: tensor-scaled dot product\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: tensor-scaled dot product\n";
		}
	}

	// Test 4: aligned vectors
	std::cout << "+---------    aligned vectors   --------+\n";
	{
		float a_input[16], b_input[16];
		float ref_dot = 0.0f;
		for (int i = 0; i < 16; ++i) {
			float v = static_cast<float>(i + 1);
			a_input[i] = v;
			b_input[i] = v;
			ref_dot += v * v;
		}

		nvfp4 a, b;
		a.quantize(a_input, 1.0f);
		b.quantize(b_input, 1.0f);
		float nv_dot = a.dot(b, 1.0f, 1.0f);
		float rel_err = std::fabs(nv_dot - ref_dot) / std::fabs(ref_dot);
		std::cout << "Reference: " << ref_dot << " NV: " << nv_dot << " RelErr: " << rel_err << '\n';
		if (rel_err > 0.5f) {
			std::cout << "FAIL: aligned vectors dot product\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: aligned vectors dot product\n";
		}
	}

	// Test 5: NaN scale dot product
	std::cout << "+---------    NaN scale dot product   --------+\n";
	{
		float input[16];
		for (int i = 0; i < 16; ++i) input[i] = 1.0f;

		nvfp4 a, b;
		a.quantize(input, 1.0f);
		b.clear();
		b.setscalebits(0x7F); // NaN scale on b

		float result = a.dot(b, 1.0f, 1.0f);
		if (result != result) { // NaN check
			std::cout << "PASS: NaN scale produces NaN dot product\n";
		} else {
			std::cout << "FAIL: NaN scale should produce NaN\n";
			++nrOfFailedTestCases;
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
