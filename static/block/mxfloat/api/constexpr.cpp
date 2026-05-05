// constexpr.cpp: compile-time tests for mxfloat (OCP MX block format / mxblock)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "mxfloat constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// mxblock<ElementType, BlockSize> pairs an e8m0 scale factor with a block
	// of element-typed values.  e8m0 is constexpr (PR #810); microfloat
	// elements are constexpr (PR #811); mxblock pulls the contract through
	// the block-floating-point composition.

	// Use small block sizes to keep test compile/eval cost reasonable.
	using e4m3 = microfloat<8, 4, false, true, true>;
	using mxfp8_4 = mxblock<e4m3, 4>;

	// ----------------------------------------------------------------------------
	// Default construction (already constexpr -- smoke test).
	// ----------------------------------------------------------------------------
	{
		constexpr mxfp8_4 a{};
		// Default-constructed elements are zero; scale defaults to encoding 0
		// which represents 2^-127.  iszero is not part of mxblock's API; verify
		// via operator[] returning 0 (from element zero * any scale).
		static_assert(a[0] == 0.0f, "constexpr mxblock{} elements zero");
		static_assert(a[1] == 0.0f, "constexpr mxblock{} all elements zero");
	}

	// ----------------------------------------------------------------------------
	// clear() + setbits() + accessors (all constexpr after PR #810/#811).
	// ----------------------------------------------------------------------------
	{
		constexpr mxfp8_4 cleared = []() { mxfp8_4 b{}; b.clear(); return b; }();
		static_assert(cleared.scale().bits() == 0u, "constexpr clear() zeroes scale");
		static_assert(cleared.size() == 4u,         "constexpr size() == BlockSize");
	}

	// ----------------------------------------------------------------------------
	// quantize(): constexpr OCP MX v1.0 quantization of a constexpr float
	// array.  This exercises the new cx_floor_log2 + cx_ldexp helpers and
	// proves the spec computation works at constant evaluation.
	// ----------------------------------------------------------------------------
	{
		// All-zeros input -> scale set to 2^(-elemMaxExp), elements all zero.
		constexpr mxfp8_4 z = []() {
			mxfp8_4 b{};
			float src[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			b.quantize(src, 4);
			return b;
		}();
		static_assert(z[0] == 0.0f, "constexpr quantize(zeros)[0] == 0");
		static_assert(z[3] == 0.0f, "constexpr quantize(zeros)[3] == 0");

		// Power-of-2 input -> exact quantization.
		// e4m3 has elemMaxExp = 8 (representing 448 = 1.110*2^8).
		// For amax = 4.0 = 2^2, shared_exp = 2; scale_exp = 2 - 8 = -6;
		// inv_scale = 2^6 = 64; quantized values = 4*64=256, 2*64=128 (clamped to
		// e4m3 range).  Dequantized = scale * element = 2^-6 * (e4m3 quantized).
		// The exact bit pattern depends on e4m3 RNE rounding.  We just verify
		// non-zero values + roundtrip rough shape.
		constexpr mxfp8_4 simple = []() {
			mxfp8_4 b{};
			float src[4] = { 4.0f, 2.0f, 1.0f, 0.5f };
			b.quantize(src, 4);
			return b;
		}();
		// Scale should be set to a non-zero encoding (zero scale = no input).
		static_assert(simple.scale().bits() != 0u, "constexpr quantize sets non-zero scale");
		// Elements should be non-zero for non-zero inputs.
		static_assert(simple.element(0).bits() != 0u, "constexpr quantize encodes non-zero input element 0");
		static_assert(simple.element(1).bits() != 0u, "constexpr quantize encodes non-zero input element 1");
	}

	// ----------------------------------------------------------------------------
	// dequantize() + operator[] (constexpr after PR #810/#811).
	// ----------------------------------------------------------------------------
	{
		// Construct a block with known scale + elements via setbits + element accessor.
		constexpr mxfp8_4 b = []() {
			mxfp8_4 t{};
			t.setbits(127u);  // scale = 2^0 = 1.0
			// e4m3 encoding 0x38 = sign=0, exp=7, frac=0 -> 1.0
			t.element(0) = e4m3(1.0f);
			t.element(1) = e4m3(2.0f);
			t.element(2) = e4m3(0.0f);
			t.element(3) = e4m3(-1.0f);
			return t;
		}();
		// scale=1.0 so each operator[i] returns the element value as float.
		static_assert(b[0] == 1.0f,   "constexpr operator[] -> 1.0");
		static_assert(b[1] == 2.0f,   "constexpr operator[] -> 2.0");
		static_assert(b[2] == 0.0f,   "constexpr operator[] -> 0.0");
		static_assert(b[3] == -1.0f,  "constexpr operator[] -> -1.0");

		// Out-of-range index returns 0.0f.
		static_assert(b[100] == 0.0f, "constexpr operator[] OOB returns 0");
	}

	// ----------------------------------------------------------------------------
	// dot product (constexpr accumulation after PR #810/#811).
	// ----------------------------------------------------------------------------
	{
		constexpr mxfp8_4 a = []() {
			mxfp8_4 t{};
			t.setbits(127u);  // scale = 1.0
			t.element(0) = e4m3(1.0f);
			t.element(1) = e4m3(2.0f);
			t.element(2) = e4m3(3.0f);
			t.element(3) = e4m3(4.0f);
			return t;
		}();
		constexpr mxfp8_4 b = []() {
			mxfp8_4 t{};
			t.setbits(127u);  // scale = 1.0
			t.element(0) = e4m3(2.0f);
			t.element(1) = e4m3(0.0f);
			t.element(2) = e4m3(0.0f);
			t.element(3) = e4m3(0.0f);
			return t;
		}();
		// a . b = scale_a * scale_b * sum(a_i * b_i) = 1*1 * (1*2 + 2*0 + 3*0 + 4*0) = 2.0
		constexpr float cx_dot = a.dot(b);
		static_assert(cx_dot == 2.0f, "constexpr mxblock::dot()");
	}

	// ----------------------------------------------------------------------------
	// NaN scale propagation (constexpr).  Setting the scale to encoding 0xFF
	// makes operator[] / dot return NaN.
	// ----------------------------------------------------------------------------
	{
		constexpr mxfp8_4 nan_block = []() {
			mxfp8_4 t{};
			t.setbits(0xFFu);  // e8m0 NaN encoding
			return t;
		}();
		// NaN scale -> operator[] returns NaN.  Test via x != x (only NaN
		// satisfies this).
		constexpr float v = nan_block[0];
		static_assert(v != v, "constexpr NaN-scale -> operator[] returns NaN");
	}

	std::cout << "mxfloat constexpr verification: "
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
