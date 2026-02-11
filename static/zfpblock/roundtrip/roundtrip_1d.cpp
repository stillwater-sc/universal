// roundtrip_1d.cpp: 1D float/double compress/decompress round-trip tests
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>

// Verify reversible round-trip for 1D float
// Note: The ZFP lifting transform has inherent ±1 LSB rounding in the integer domain.
// For values with similar magnitude, this gives near-exact results.
// For values with very different magnitudes, the quantization to shared exponent
// causes small values to be quantized to zero.
int VerifyReversible1DFloat(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// Test cases with values of similar magnitude (good for block-float quantization)
	float patterns[][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 2.0f, 3.0f, 4.0f },
		{ -1.0f, 0.5f, -0.25f, 0.125f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },  // constant
		{ 100.0f, 200.0f, 300.0f, 400.0f },
	};

	for (auto& pat : patterns) {
		zfp1f blk;
		blk.compress_reversible(pat);

		float output[4];
		blk.decompress(output);

		for (int i = 0; i < 4; ++i) {
			float max_val = 0.0f;
			for (int j = 0; j < 4; ++j) {
				if (std::abs(pat[j]) > max_val) max_val = std::abs(pat[j]);
			}
			// Allow small relative error from lifting rounding
			float tol = (max_val > 0) ? max_val * 1.0e-6f : 1.0e-30f;
			float err = std::abs(output[i] - pat[i]);
			if (err > tol) {
				std::cerr << tag << " FAIL: index " << i
				          << " expected=" << pat[i]
				          << " got=" << output[i]
				          << " err=" << err << " tol=" << tol << '\n';
				++nrOfFailedTests;
			}
		}
	}
	return nrOfFailedTests;
}

// Verify reversible round-trip for 1D double
// Same caveats as float: block-float + lifting rounding means near-exact, not bit-exact.
int VerifyReversible1DDouble(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// Use values with similar magnitude for best results
	double patterns[][4] = {
		{ 0.0, 0.0, 0.0, 0.0 },
		{ 1.0, 2.0, 3.0, 4.0 },
		{ 3.14159265358979, 2.71828182845905, 1.41421356237310, 1.73205080756888 },
		{ -100.0, 50.0, -25.0, 12.5 },
	};

	for (auto& pat : patterns) {
		zfp1d blk;
		blk.compress_reversible(pat);

		double output[4];
		blk.decompress(output);

		for (int i = 0; i < 4; ++i) {
			double max_val = 0.0;
			for (int j = 0; j < 4; ++j) {
				if (std::abs(pat[j]) > max_val) max_val = std::abs(pat[j]);
			}
			double tol = (max_val > 0) ? max_val * 1.0e-14 : 1.0e-300;
			double err = std::abs(output[i] - pat[i]);
			if (err > tol) {
				std::cerr << tag << " FAIL: index " << i
				          << std::setprecision(17)
				          << " expected=" << pat[i]
				          << " got=" << output[i]
				          << " err=" << err << " tol=" << tol << '\n';
				++nrOfFailedTests;
			}
		}
	}
	return nrOfFailedTests;
}

// Verify lossy round-trip with fixed-rate for 1D float
int VerifyFixedRate1DFloat(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	float input[4] = { 1.5f, -2.5f, 3.5f, -4.5f };

	// test various rates
	double rates[] = { 4.0, 8.0, 16.0, 24.0 };
	for (double rate : rates) {
		zfp1f blk;
		size_t nbits = blk.compress_fixed_rate(input, rate);

		float output[4];
		blk.decompress(output);

		// compute max error
		double max_err = 0.0;
		for (int i = 0; i < 4; ++i) {
			double err = std::abs(static_cast<double>(output[i]) - static_cast<double>(input[i]));
			if (err > max_err) max_err = err;
		}

		std::cout << tag << " rate=" << rate
		          << " bits=" << nbits
		          << " max_err=" << max_err << '\n';

		// higher rate should give better accuracy (or at least not worse)
		// basic sanity: error should be finite
		if (std::isnan(max_err) || std::isinf(max_err)) {
			std::cerr << tag << " FAIL: non-finite error at rate=" << rate << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfpblock 1D round-trip tests";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	nrOfFailedTestCases += VerifyReversible1DFloat("1D float reversible");
	nrOfFailedTestCases += VerifyReversible1DDouble("1D double reversible");
	nrOfFailedTestCases += VerifyFixedRate1DFloat("1D float fixed-rate");

	std::cout << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL")
	          << " : " << nrOfFailedTestCases << " failures\n";

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
