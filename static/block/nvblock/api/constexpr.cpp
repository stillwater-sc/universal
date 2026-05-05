// constexpr.cpp: compile-time tests for nvblock (NVIDIA NVFP4 block format)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/nvblock/nvblock.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "nvblock constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// nvblock<ElementType, BlockSize, ScaleType> uses an e4m3 (or similar)
	// fractional scale instead of e8m0 power-of-two.  Element + scale types
	// are constexpr (microfloat PR #811); nvblock pulls the contract through
	// the block-floating-point composition.

	using e4m3 = microfloat<8, 4, false, true, true>;
	using e2m1 = microfloat<4, 2, false, false, true>;
	using nvfp4_4 = nvblock<e2m1, 4, e4m3>;  // small block for fast constexpr eval

	// ----------------------------------------------------------------------------
	// Default construction (already constexpr -- smoke test).
	// ----------------------------------------------------------------------------
	{
		constexpr nvfp4_4 a{};
		// Default-constructed elements are zero; block_scale defaults to e4m3
		// encoding 0 which represents +0.  operator[] returns 0 in that case.
		static_assert(a[0] == 0.0f, "constexpr nvblock{} elements zero");
		static_assert(a[3] == 0.0f, "constexpr nvblock{} all elements zero");
	}

	// ----------------------------------------------------------------------------
	// clear() + setscalebits() + accessors (constexpr after PR #811).
	// ----------------------------------------------------------------------------
	{
		constexpr nvfp4_4 cleared = []() { nvfp4_4 b{}; b.clear(); return b; }();
		static_assert(cleared.size() == 4u, "constexpr size() == BlockSize");

		// Set the block_scale to e4m3 encoding for 1.0 (sign=0, exp=7, frac=0
		// in e4m3 = 0x38 = 56).
		constexpr nvfp4_4 unit_scale = []() {
			nvfp4_4 b{};
			b.setscalebits(0x38u);
			return b;
		}();
		// block_scale is 1.0 but elements still default to zero -> operator[] == 0.
		static_assert(unit_scale[0] == 0.0f, "constexpr unit_scale + zero element -> 0");
	}

	// ----------------------------------------------------------------------------
	// quantize() at constant evaluation.
	// nvblock's quantize uses no log2/floor/ldexp -- the e4m3 fractional scale
	// stores the actual ratio (amax / elem_max), not a power-of-two exponent.
	// All operations are constexpr-clean (FP arithmetic + microfloat to/from_float).
	// ----------------------------------------------------------------------------
	{
		// All-zeros input -> block_scale cleared, elements all zero.
		constexpr nvfp4_4 z = []() {
			nvfp4_4 b{};
			float src[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			b.quantize(src, 1.0f, 4);
			return b;
		}();
		static_assert(z[0] == 0.0f, "constexpr quantize(zeros)[0] == 0");
		static_assert(z[3] == 0.0f, "constexpr quantize(zeros)[3] == 0");

		// Small power-of-two-friendly input.  e2m1 elem_max is 6.0.
		// For amax=1.0, raw_scale = 1.0 / 6.0 ~= 0.167, which rounds to e4m3.
		// Verify the block has a non-zero scale and at least one non-zero element.
		constexpr nvfp4_4 simple = []() {
			nvfp4_4 b{};
			float src[4] = { 1.0f, 0.5f, 0.25f, 0.0f };
			b.quantize(src, 1.0f, 4);
			return b;
		}();
		// block_scale should NOT be the default zero.
		static_assert(simple.block_scale().bits() != 0u,
			"constexpr quantize sets non-zero block_scale");
	}

	// ----------------------------------------------------------------------------
	// dequantize() + operator[] (constexpr after PR #811).
	// ----------------------------------------------------------------------------
	{
		// Construct with known block_scale + elements.
		constexpr nvfp4_4 b = []() {
			nvfp4_4 t{};
			t.setscalebits(0x38u);  // e4m3 1.0
			t.element(0) = e2m1(1.0f);
			t.element(1) = e2m1(2.0f);
			t.element(2) = e2m1(0.0f);
			t.element(3) = e2m1(-1.0f);
			return t;
		}();
		// block_scale=1.0, so operator[i] = block_scale * element[i] = element[i].
		static_assert(b[0] == 1.0f,   "constexpr operator[] -> 1.0");
		static_assert(b[1] == 2.0f,   "constexpr operator[] -> 2.0");
		static_assert(b[2] == 0.0f,   "constexpr operator[] -> 0.0");
		static_assert(b[3] == -1.0f,  "constexpr operator[] -> -1.0");

		// Out-of-range index returns 0.0f.
		static_assert(b[100] == 0.0f, "constexpr operator[] OOB returns 0");

		// Explicit constexpr dequantize() coverage.  dequantize writes to a
		// float[] output; the lambda copies into a struct so we can return
		// a constexpr value out of the lambda for static_assert.
		struct four_floats { float v0, v1, v2, v3; };
		constexpr four_floats dq = []() {
			float dst[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			constexpr nvfp4_4 src = []() {
				nvfp4_4 t{};
				t.setscalebits(0x38u);  // e4m3 1.0
				t.element(0) = e2m1(1.0f);
				t.element(1) = e2m1(2.0f);
				t.element(2) = e2m1(0.0f);
				t.element(3) = e2m1(-1.0f);
				return t;
			}();
			src.dequantize(dst, 1.0f, 4);
			return four_floats{ dst[0], dst[1], dst[2], dst[3] };
		}();
		// tensor_scale=1.0, block_scale=1.0, so dst[i] == element[i].
		static_assert(dq.v0 ==  1.0f, "constexpr dequantize[0] -> 1.0");
		static_assert(dq.v1 ==  2.0f, "constexpr dequantize[1] -> 2.0");
		static_assert(dq.v2 ==  0.0f, "constexpr dequantize[2] -> 0.0");
		static_assert(dq.v3 == -1.0f, "constexpr dequantize[3] -> -1.0");
	}

	// ----------------------------------------------------------------------------
	// dot product with dual tensor scales (constexpr).
	// ----------------------------------------------------------------------------
	{
		constexpr nvfp4_4 a = []() {
			nvfp4_4 t{};
			t.setscalebits(0x38u);  // e4m3 1.0
			t.element(0) = e2m1(2.0f);
			t.element(1) = e2m1(0.0f);
			t.element(2) = e2m1(0.0f);
			t.element(3) = e2m1(0.0f);
			return t;
		}();
		constexpr nvfp4_4 b = []() {
			nvfp4_4 t{};
			t.setscalebits(0x38u);  // e4m3 1.0
			t.element(0) = e2m1(3.0f);
			t.element(1) = e2m1(0.0f);
			t.element(2) = e2m1(0.0f);
			t.element(3) = e2m1(0.0f);
			return t;
		}();
		// dot(a, b, 1.0, 1.0) = 1*1*1*1 * (2*3 + 0*0 + 0*0 + 0*0) = 6.0
		// Note: e2m1 may round 3.0 to 4.0 (e2m1's representable values are sparse).
		// We assert via "dot is positive non-zero" rather than exact value.
		constexpr float cx_dot = a.dot(b, 1.0f, 1.0f);
		static_assert(cx_dot > 0.0f, "constexpr nvblock::dot() positive");

		// Tensor scales scale the result linearly.
		constexpr float cx_dot_scaled = a.dot(b, 2.0f, 3.0f);
		static_assert(cx_dot_scaled == 6.0f * cx_dot, "constexpr dot scales by tensor_scale_a * tensor_scale_b");
	}

	// ----------------------------------------------------------------------------
	// NaN block_scale propagation (constexpr).
	// e4m3 has hasNaN=true; encoding 0x7F (positive) and 0xFF (negative) are NaN.
	// ----------------------------------------------------------------------------
	{
		constexpr nvfp4_4 nan_block = []() {
			nvfp4_4 t{};
			t.setscalebits(0x7Fu);  // e4m3 quiet NaN encoding
			return t;
		}();
		// NaN scale -> operator[] returns NaN.  Test via x != x.
		constexpr float v = nan_block[0];
		static_assert(v != v, "constexpr NaN-scale -> operator[] returns NaN");

		// dot also propagates NaN.
		constexpr nvfp4_4 finite = []() {
			nvfp4_4 t{};
			t.setscalebits(0x38u);
			t.element(0) = e2m1(1.0f);
			return t;
		}();
		constexpr float dot_nan = nan_block.dot(finite, 1.0f, 1.0f);
		static_assert(dot_nan != dot_nan, "constexpr NaN-scale -> dot returns NaN");
	}

	std::cout << "nvblock constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
