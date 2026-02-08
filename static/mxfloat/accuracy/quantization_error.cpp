// quantization_error.cpp: accuracy analysis for MX block format quantization
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>
#include <random>

template<typename MXType>
void measure_quantization_error(const std::string& name, float range_min, float range_max) {
	using namespace sw::universal;

	constexpr size_t nBlocks = 100;
	constexpr size_t blockSize = MXType::blockSize;
	constexpr size_t nValues = nBlocks * blockSize;

	// Generate random FP32 values
	std::mt19937 rng(42); // fixed seed for reproducibility
	std::uniform_real_distribution<float> dist(range_min, range_max);

	float mse = 0.0f;
	float max_abs_err = 0.0f;
	float sum_rel_err = 0.0f;
	size_t rel_err_count = 0;

	for (size_t b = 0; b < nBlocks; ++b) {
		float input[blockSize];
		float output[blockSize];
		for (size_t i = 0; i < blockSize; ++i) {
			input[i] = dist(rng);
		}

		MXType blk;
		blk.quantize(input);
		blk.dequantize(output);

		for (size_t i = 0; i < blockSize; ++i) {
			float err = output[i] - input[i];
			float abs_err = std::fabs(err);
			mse += err * err;
			if (abs_err > max_abs_err) max_abs_err = abs_err;
			if (std::fabs(input[i]) > 1e-10f) {
				sum_rel_err += abs_err / std::fabs(input[i]);
				++rel_err_count;
			}
		}
	}

	mse /= static_cast<float>(nValues);
	float rmse = std::sqrt(mse);
	float avg_rel_err = (rel_err_count > 0) ? sum_rel_err / static_cast<float>(rel_err_count) : 0.0f;

	std::cout << std::setw(15) << name
		<< " | range=[" << range_min << "," << range_max << "]"
		<< " | RMSE=" << std::scientific << std::setprecision(3) << rmse
		<< " | MaxErr=" << max_abs_err
		<< " | AvgRelErr=" << std::fixed << std::setprecision(4) << avg_rel_err
		<< '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "mxblock quantization error analysis";
	int nrOfFailedTestCases = 0;

	std::cout << "+---------    Quantization Error Analysis   --------+\n";
	std::cout << "Each format tested with 100 blocks of 32 random FP32 values\n\n";

	// Test with [-1, 1] range (typical neural network activations)
	std::cout << "--- Range [-1.0, 1.0] (typical activations) ---\n";
	measure_quantization_error<mxfp4>("mxfp4 (e2m1)", -1.0f, 1.0f);
	measure_quantization_error<mxfp6>("mxfp6 (e3m2)", -1.0f, 1.0f);
	measure_quantization_error<mxfp6e2m3>("mxfp6e2m3", -1.0f, 1.0f);
	measure_quantization_error<mxfp8>("mxfp8 (e4m3)", -1.0f, 1.0f);
	measure_quantization_error<mxfp8e5m2>("mxfp8e5m2", -1.0f, 1.0f);
	measure_quantization_error<mxint8>("mxint8", -1.0f, 1.0f);

	std::cout << '\n';

	// Test with [-10, 10] range (larger activations)
	std::cout << "--- Range [-10.0, 10.0] (larger activations) ---\n";
	measure_quantization_error<mxfp4>("mxfp4 (e2m1)", -10.0f, 10.0f);
	measure_quantization_error<mxfp6>("mxfp6 (e3m2)", -10.0f, 10.0f);
	measure_quantization_error<mxfp6e2m3>("mxfp6e2m3", -10.0f, 10.0f);
	measure_quantization_error<mxfp8>("mxfp8 (e4m3)", -10.0f, 10.0f);
	measure_quantization_error<mxfp8e5m2>("mxfp8e5m2", -10.0f, 10.0f);
	measure_quantization_error<mxint8>("mxint8", -10.0f, 10.0f);

	std::cout << '\n';

	// Test with [0.001, 100] range (wide dynamic range)
	std::cout << "--- Range [0.001, 100.0] (wide dynamic range) ---\n";
	measure_quantization_error<mxfp4>("mxfp4 (e2m1)", 0.001f, 100.0f);
	measure_quantization_error<mxfp6>("mxfp6 (e3m2)", 0.001f, 100.0f);
	measure_quantization_error<mxfp6e2m3>("mxfp6e2m3", 0.001f, 100.0f);
	measure_quantization_error<mxfp8>("mxfp8 (e4m3)", 0.001f, 100.0f);
	measure_quantization_error<mxfp8e5m2>("mxfp8e5m2", 0.001f, 100.0f);
	measure_quantization_error<mxint8>("mxint8", 0.001f, 100.0f);

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
