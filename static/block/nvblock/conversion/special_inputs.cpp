// special_inputs.cpp: nvblock quantization of NaN and inf inputs, a zero tensor scale, and a block whose
// amax is too small for the e4m3 block scale (#2096)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
//   - a NaN input makes the block NaN, as it does an MX block (#2086): the block scale is e4m3's NaN and
//     every element dequantizes to NaN. Before #2096 the NaN was dropped: NaN is never greater than
//     amax, so the scale came from the other elements, and e2m1, which has no NaN, made the element 0.
//   - an inf input saturates: the block scale is e4m3's maxpos (448), the inf element e2m1's maxpos (6),
//     and the finite elements underflow to 0. This is the behaviour MX blocks keep (#2096).
//   - a tensor scale of 0 gives the zero block.
//   - an amax so small that amax / 6 rounds to 0 in e4m3 gets the smallest e4m3 scale, 2^-9: the
//     scale is never 0, so nothing divides by it.
#include <universal/utility/directives.hpp>
#include <universal/number/nvblock/nvblock.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

	using sw::universal::nvfp4;

	struct quantized {
		nvfp4 block;
		float out[16];
	};

	quantized round_trip(const float* x, std::size_t n, float tensor_scale) {
		quantized q{};
		q.block.quantize(x, tensor_scale, n);
		q.block.dequantize(q.out, tensor_scale, n);
		return q;
	}

	int VerifyNaNInput(bool reportTestCases) {
		const float nan = std::numeric_limits<float>::quiet_NaN();
		const float blocks[][4] = {
			{ nan, 1.0f, -2.0f, 0.5f },
			{ 3.0f, 1.0f, -nan, 0.5f },
			{ 0.0f, 0.0f, 0.0f, nan },
			{ nan, std::numeric_limits<float>::infinity(), 1.0f, 0.0f },
		};
		int fails = 0;
		for (const auto& x : blocks) {
			const quantized q = round_trip(x, 4, 1.0f);
			bool ok = q.block.block_scale().isnan();
			for (std::size_t i = 0; i < 4; ++i) ok = ok && (q.out[i] != q.out[i]) && (q.block[i] != q.block[i]);
			if (!ok) {
				if (reportTestCases) std::cerr << "FAIL: a block holding a NaN has scale bits 0x" << std::hex << unsigned(q.block.block_scale().bits())
				                               << std::dec << " and dequantizes to " << q.out[0] << ", " << q.out[1] << ", expected NaN\n";
				++fails;
			}
		}
		// a NaN tensor scale makes every input NaN
		const float x[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
		const quantized q = round_trip(x, 4, nan);
		if (!q.block.block_scale().isnan()) {
			if (reportTestCases) std::cerr << "FAIL: a NaN tensor scale does not give a NaN block\n";
			++fails;
		}
		return fails;
	}

	int VerifyInfInput(bool reportTestCases) {
		const float inf = std::numeric_limits<float>::infinity();
		const float x[4] = { inf, 1.0f, -2.0f, -inf };
		const quantized q = round_trip(x, 4, 1.0f);
		const float top = 448.0f * 6.0f;   // e4m3 maxpos times e2m1 maxpos
		if (q.block.block_scale().to_float() != 448.0f || q.out[0] != top || q.out[3] != -top || q.out[1] != 0.0f || q.out[2] != 0.0f) {
			if (reportTestCases) std::cerr << "FAIL: {inf, 1, -2, -inf} gives scale " << q.block.block_scale().to_float() << " and {" << q.out[0]
			                               << ", " << q.out[1] << ", " << q.out[2] << ", " << q.out[3] << "}, expected scale 448 and {2688, 0, 0, -2688}\n";
			return 1;
		}
		return 0;
	}

	int VerifyZeroTensorScale(bool reportTestCases) {
		const float x[4] = { 3.0f, 1.0f, -2.0f, std::numeric_limits<float>::quiet_NaN() };
		const quantized q = round_trip(x, 4, 0.0f);
		bool ok = !q.block.block_scale().isnan();
		for (std::size_t i = 0; i < 4; ++i) ok = ok && (q.out[i] == 0.0f);
		if (!ok) {
			if (reportTestCases) std::cerr << "FAIL: a tensor scale of 0 does not give the zero block\n";
			return 1;
		}
		return 0;
	}

	int VerifyTinyAmax(bool reportTestCases) {
		int fails = 0;
		// amax / 6 is below half of e4m3's smallest subnormal, 2^-9: the scale is that minpos, not 0
		for (float amax : { 1e-6f, 1e-20f, 0x1p-126f, std::numeric_limits<float>::denorm_min() }) {
			const float x[4] = { amax, -amax / 2.0f, 0.0f, amax / 4.0f };
			const quantized q = round_trip(x, 4, 1.0f);
			const float s = q.block.block_scale().to_float();
			bool ok = (s == std::ldexp(1.0f, -9));
			for (std::size_t i = 0; i < 4; ++i) ok = ok && (q.out[i] == q.out[i]) && std::fabs(q.out[i]) <= std::ldexp(1.0f, -9);
			if (!ok) {
				if (reportTestCases) std::cerr << "FAIL: amax " << amax << " gives scale " << s << ", expected 2^-9 and finite elements\n";
				++fails;
			}
		}
		// at the smallest scale an element is still kept: 0.625 * 2^-9 rounds to e2m1's 0.5, below the
		// tie at 0.75 between 0.5 and 1
		const float x[2] = { 0.625f * std::ldexp(1.0f, -9), 0.0f };
		const quantized q = round_trip(x, 2, 1.0f);
		if (q.out[0] != 0.5f * std::ldexp(1.0f, -9)) {
			if (reportTestCases) std::cerr << "FAIL: 0.625 * 2^-9 dequantizes to " << q.out[0] << ", expected 2^-10\n";
			++fails;
		}
		return fails;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "nvfp4 special inputs (#2096)";
	std::string test_tag    = "nvfp4";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNaNInput(reportTestCases), test_tag, "NaN input");
	nrOfFailedTestCases += ReportTestResult(VerifyInfInput(reportTestCases), test_tag, "inf input");
	nrOfFailedTestCases += ReportTestResult(VerifyZeroTensorScale(reportTestCases), test_tag, "tensor scale 0");
	nrOfFailedTestCases += ReportTestResult(VerifyTinyAmax(reportTestCases), test_tag, "tiny amax");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
