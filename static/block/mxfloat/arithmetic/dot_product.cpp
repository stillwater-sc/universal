// dot_product.cpp: test suite for mxblock block dot product
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

	std::string test_suite = "mxblock dot product tests";
	int nrOfFailedTestCases = 0;

	// Test 1: dot product of unit vectors
	std::cout << "+---------    unit vectors dot product   --------+\n";
	{
		float a_input[32], b_input[32];
		for (int i = 0; i < 32; ++i) {
			a_input[i] = 1.0f;
			b_input[i] = 1.0f;
		}
		float ref_dot = 32.0f;

		mxfp8 a, b;
		a.quantize(a_input);
		b.quantize(b_input);
		float mx_dot = a.dot(b);
		float rel_err = std::fabs(mx_dot - ref_dot) / std::fabs(ref_dot);
		std::cout << "Reference: " << ref_dot << " MX: " << mx_dot << " RelErr: " << rel_err << '\n';
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
		float a_input[32] = {};
		float b_input[32] = {};
		// a = [1,0,1,0,...], b = [0,1,0,1,...]
		for (int i = 0; i < 32; ++i) {
			if (i % 2 == 0) {
				a_input[i] = 1.0f;
				b_input[i] = 0.0f;
			} else {
				a_input[i] = 0.0f;
				b_input[i] = 1.0f;
			}
		}
		float ref_dot = 0.0f;

		mxfp8 a, b;
		a.quantize(a_input);
		b.quantize(b_input);
		float mx_dot = a.dot(b);
		std::cout << "Reference: " << ref_dot << " MX: " << mx_dot << '\n';
		if (std::fabs(mx_dot) > 1.0f) {
			std::cout << "FAIL: orthogonal vectors dot product\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: orthogonal vectors dot product\n";
		}
	}

	// Test 3: aligned vectors (same direction)
	std::cout << "+---------    aligned vectors   --------+\n";
	{
		float a_input[32], b_input[32];
		float ref_dot = 0.0f;
		for (int i = 0; i < 32; ++i) {
			float v = static_cast<float>(i + 1);
			a_input[i] = v;
			b_input[i] = v;
			ref_dot += v * v;
		}

		mxfp8 a, b;
		a.quantize(a_input);
		b.quantize(b_input);
		float mx_dot = a.dot(b);
		float rel_err = std::fabs(mx_dot - ref_dot) / std::fabs(ref_dot);
		std::cout << "Reference: " << ref_dot << " MX: " << mx_dot << " RelErr: " << rel_err << '\n';
		if (rel_err > 0.5f) {
			std::cout << "FAIL: aligned vectors dot product\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: aligned vectors dot product\n";
		}
	}

	// Test 4: mxfp4 dot product (most lossy)
	std::cout << "+---------    mxfp4 dot product   --------+\n";
	{
		float a_input[32], b_input[32];
		float ref_dot = 0.0f;
		for (int i = 0; i < 32; ++i) {
			a_input[i] = 1.0f;
			b_input[i] = 2.0f;
			ref_dot += 2.0f;
		}

		mxfp4 a, b;
		a.quantize(a_input);
		b.quantize(b_input);
		float mx_dot = a.dot(b);
		float rel_err = std::fabs(mx_dot - ref_dot) / std::fabs(ref_dot);
		std::cout << "Reference: " << ref_dot << " MX: " << mx_dot << " RelErr: " << rel_err << '\n';
		if (rel_err > 0.5f) {
			std::cout << "FAIL: mxfp4 dot product\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: mxfp4 dot product\n";
		}
	}

	// Test 5: mxint8 dot product
	std::cout << "+---------    mxint8 dot product   --------+\n";
	{
		float a_input[32], b_input[32];
		float ref_dot = 0.0f;
		for (int i = 0; i < 32; ++i) {
			a_input[i] = static_cast<float>(i + 1);
			b_input[i] = 1.0f;
			ref_dot += static_cast<float>(i + 1);
		}

		mxint8 a, b;
		a.quantize(a_input);
		b.quantize(b_input);
		float mx_dot = a.dot(b);
		float rel_err = std::fabs(mx_dot - ref_dot) / std::fabs(ref_dot);
		std::cout << "Reference: " << ref_dot << " MX: " << mx_dot << " RelErr: " << rel_err << '\n';
		if (rel_err > 0.2f) {
			std::cout << "FAIL: mxint8 dot product\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: mxint8 dot product\n";
		}
	}

	// Test 6: NaN scale dot product
	std::cout << "+---------    NaN scale dot product   --------+\n";
	{
		float input[32];
		for (int i = 0; i < 32; ++i) input[i] = 1.0f;

		mxfp8 a, b;
		a.quantize(input);
		b.clear();
		b.setbits(0xFF); // NaN scale on b

		float result = a.dot(b);
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
