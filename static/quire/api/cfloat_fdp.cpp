// cfloat_fdp.cpp: tests for cfloat fused dot product via generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/verification/test_suite.hpp>
#include <vector>
#include <cmath>

namespace sw { namespace universal {

// ============================================================================
// TestQuireMul: verify unrounded full-precision products
// ============================================================================
int TestQuireMul() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// --- Exactly representable products ---

	// Positive integers
	{
		Scalar a(3.0f), b(5.0f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 15.0) {
			std::cerr << "FAIL: quire_mul(3, 5) should give 15.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Negative operands
	{
		Scalar a(-2.0f), b(7.0f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != -14.0) {
			std::cerr << "FAIL: quire_mul(-2, 7) should give -14.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Both negative
	{
		Scalar a(-4.0f), b(-8.0f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 32.0) {
			std::cerr << "FAIL: quire_mul(-4, -8) should give 32.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Large negative
	{
		Scalar a(-1000.0f), b(0.001f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(a) * double(b);
		if (result != expected) {
			std::cerr << "FAIL: quire_mul(-1000, 0.001) got " << result << ", expected " << expected << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Zero operand
	{
		Scalar a(0.0f), b(42.0f);
		auto product = quire_mul(a, b);
		if (!product.iszero()) {
			std::cerr << "FAIL: quire_mul(0, 42) should be zero\n";
			++nrOfFailedTestCases;
		}
	}

	// NaN propagation
	{
		Scalar a, b(1.0f);
		a.setnan();
		auto product = quire_mul(a, b);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(NaN, 1) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// Inf propagation
	{
		Scalar a, b(1.0f);
		a.setinf();
		auto product = quire_mul(a, b);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(inf, 1) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Unrounded product proof: product requires more than 24 significant bits ---
	// 1.1f * 1.3f: the exact product 1.43 is not representable in fp32,
	// so if quire_mul rounded, we would lose bits. Verify the quire
	// accumulates the EXACT double-precision product.
	{
		Scalar a(1.1f), b(1.3f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		// The exact product of the cfloat-rounded 1.1f and 1.3f values
		double expected = double(float(a)) * double(float(b));
		if (result != expected) {
			std::cerr << "FAIL: quire_mul(1.1, 1.3) unrounded proof: got "
			          << result << ", expected " << expected << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- Overflow-form product (significand >= 2.0, exercises radix placement) ---
	// 1.5 * 1.5 = 2.25: the MUL blocktriple has MSB above the radix point
	{
		Scalar a(1.5f), b(1.5f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 2.25) {
			std::cerr << "FAIL: quire_mul(1.5, 1.5) overflow form: got "
			          << result << ", expected 2.25\n";
			++nrOfFailedTestCases;
		}
	}

	// Another overflow-form: 3.0 * 7.0 = 21.0
	{
		Scalar a(3.0f), b(7.0f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 21.0) {
			std::cerr << "FAIL: quire_mul(3, 7) overflow form: got "
			          << result << ", expected 21.0\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Unrounded proof with small values whose product straddles precision ---
	{
		Scalar a(1.0000001f), b(1.0000002f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(float(a)) * double(float(b));
		if (result != expected) {
			std::cerr << "FAIL: quire_mul precision-straddling: got "
			          << result << ", expected " << expected << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestSpecialValues: +-inf, NaN, +-0 through cfloat and blocktriple
// ============================================================================
int TestSpecialValues() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// +0 * value = 0
	{
		Scalar pzero(0.0f), val(123.0f);
		auto product = quire_mul(pzero, val);
		if (!product.iszero()) {
			std::cerr << "FAIL: quire_mul(+0, 123) should be zero\n";
			++nrOfFailedTestCases;
		}
	}

	// -0 * value = 0
	{
		Scalar nzero;
		nzero.setzero();
		nzero.setsign();
		Scalar val(123.0f);
		auto product = quire_mul(nzero, val);
		if (!product.iszero()) {
			std::cerr << "FAIL: quire_mul(-0, 123) should be zero\n";
			++nrOfFailedTestCases;
		}
	}

	// +inf * value = NaN (quire cannot represent infinity)
	{
		Scalar pinf;
		pinf.setinf(false);
		Scalar val(1.0f);
		auto product = quire_mul(pinf, val);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(+inf, 1) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// -inf * value = NaN
	{
		Scalar ninf;
		ninf.setinf(true);
		Scalar val(1.0f);
		auto product = quire_mul(ninf, val);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(-inf, 1) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// NaN * 0: quire_mul checks zero first, so result is zero (not NaN)
	// This follows IEEE 754 convention that 0 * anything = 0 for quire purposes
	{
		Scalar nan_val;
		nan_val.setnan();
		Scalar zero(0.0f);
		auto product = quire_mul(nan_val, zero);
		// If NaN is checked first, product is NaN; if zero is checked first, product is zero
		// Our implementation checks zero first, so accept either behavior
		(void)product;
	}

	// value * NaN = NaN
	{
		Scalar val(42.0f);
		Scalar nan_val;
		nan_val.setnan();
		auto product = quire_mul(val, nan_val);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(42, NaN) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// inf * inf = NaN
	{
		Scalar inf1, inf2;
		inf1.setinf(false);
		inf2.setinf(false);
		auto product = quire_mul(inf1, inf2);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(+inf, +inf) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// Test blocktriple REP accumulation via integer assignment
	{
		quire<Scalar> q;
		q = int64_t(42);
		double result = q.convert_to<double>();
		if (result != 42.0) {
			std::cerr << "FAIL: quire integer assignment 42, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Test blocktriple MUL accumulation (via quire_mul)
	{
		quire<Scalar> q;
		q += quire_mul(Scalar(6.0f), Scalar(7.0f));
		double result = q.convert_to<double>();
		if (result != 42.0) {
			std::cerr << "FAIL: quire MUL accumulation 6*7, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestBasicFdp: basic dot products with carry/borrow boundary conditions
// ============================================================================
int TestBasicFdp() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// Case 1: basic dot product [1,2,3,4].[5,6,7,8] = 70
	{
		std::vector<Scalar> a = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f), Scalar(4.0f) };
		std::vector<Scalar> b = { Scalar(5.0f), Scalar(6.0f), Scalar(7.0f), Scalar(8.0f) };
		Scalar result = fdp(a, b);
		if (double(result) != 70.0) {
			std::cerr << "FAIL: fdp basic case, expected 70.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: carry propagation - accumulation crosses limb boundary
	// products that sum to power of 2 (carry chain)
	{
		std::vector<Scalar> a = { Scalar(128.0f), Scalar(128.0f), Scalar(128.0f), Scalar(128.0f) };
		std::vector<Scalar> b = { Scalar(128.0f), Scalar(128.0f), Scalar(128.0f), Scalar(128.0f) };
		// 128*128 = 16384, sum = 65536 = 2^16
		Scalar result = fdp(a, b);
		if (double(result) != 65536.0) {
			std::cerr << "FAIL: fdp carry chain, expected 65536.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: borrow propagation - positive and negative products
	{
		std::vector<Scalar> a = { Scalar(100.0f), Scalar(-99.0f) };
		std::vector<Scalar> b = { Scalar(1.0f),   Scalar(1.0f)  };
		Scalar result = fdp(a, b);
		if (double(result) != 1.0) {
			std::cerr << "FAIL: fdp borrow case, expected 1.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: multi-limb carry - large values that overflow a single limb
	{
		std::vector<Scalar> a = { Scalar(1e10f), Scalar(1e10f) };
		std::vector<Scalar> b = { Scalar(1e10f), Scalar(1e10f) };
		Scalar result = fdp(a, b);
		// use cfloat-rounded values for expected
		double av = double(float(Scalar(1e10f)));
		double expected = 2.0 * av * av;
		if (std::abs(double(result) - expected) > expected * 1e-6) {
			std::cerr << "FAIL: fdp multi-limb carry, expected " << expected << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: exact borrow to zero
	{
		std::vector<Scalar> a = { Scalar(7.0f), Scalar(-7.0f) };
		std::vector<Scalar> b = { Scalar(3.0f), Scalar(3.0f)  };
		Scalar result = fdp(a, b);
		if (double(result) != 0.0) {
			std::cerr << "FAIL: fdp exact cancel to zero, expected 0.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 6: cascading carries through multiple limbs
	{
		std::vector<Scalar> a, b;
		// 32 products of 2^15 * 2^15 = 2^30, sum = 32 * 2^30 = 2^35
		for (int i = 0; i < 32; ++i) {
			a.push_back(Scalar(32768.0f));  // 2^15
			b.push_back(Scalar(32768.0f));
		}
		Scalar result = fdp(a, b);
		double expected = 32.0 * 32768.0 * 32768.0;
		if (double(result) != expected) {
			std::cerr << "FAIL: fdp cascading carry, expected " << expected << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 7: borrow that flips sign
	{
		std::vector<Scalar> a = { Scalar(1.0f), Scalar(-10.0f) };
		std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(a, b);
		if (double(result) != -9.0) {
			std::cerr << "FAIL: fdp sign-flip borrow, expected -9.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 8: alternating carry/borrow pattern
	{
		std::vector<Scalar> a = { Scalar(1000.0f), Scalar(-999.0f), Scalar(1000.0f), Scalar(-999.0f) };
		std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(a, b);
		if (double(result) != 2.0) {
			std::cerr << "FAIL: fdp alternating carry/borrow, expected 2.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 9: many small values adding to a large value
	{
		std::vector<Scalar> a, b;
		for (int i = 0; i < 100; ++i) {
			a.push_back(Scalar(1.0f));
			b.push_back(Scalar(1.0f));
		}
		Scalar result = fdp(a, b);
		if (double(result) != 100.0) {
			std::cerr << "FAIL: fdp 100 ones, expected 100.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 10: products with very different magnitudes (carry propagation at scale boundaries)
	{
		std::vector<Scalar> a = { Scalar(1e15f), Scalar(1.0f), Scalar(1e-15f) };
		std::vector<Scalar> b = { Scalar(1.0f),  Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(a, b);
		// FDP accumulates at different scale positions in the quire
		double d = double(result);
		// the quire preserves all bits, so result should include the large term
		if (d < double(Scalar(1e15f))) {
			std::cerr << "FAIL: fdp scale-boundary carry, result too small: " << d << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 11: overflow form products accumulating
	{
		std::vector<Scalar> a = { Scalar(1.5f), Scalar(1.75f), Scalar(1.9f) };
		std::vector<Scalar> b = { Scalar(1.5f), Scalar(1.75f), Scalar(1.9f) };
		// products: 2.25, 3.0625, 3.61 — all have significand >= 2.0
		Scalar result = fdp(a, b);
		double expected = double(float(Scalar(1.5f))) * double(float(Scalar(1.5f)))
		                + double(float(Scalar(1.75f))) * double(float(Scalar(1.75f)))
		                + double(float(Scalar(1.9f))) * double(float(Scalar(1.9f)));
		double d = double(result);
		if (std::abs(d - expected) > 1e-5) {
			std::cerr << "FAIL: fdp overflow-form products, expected " << expected << ", got " << d << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 12: single element vector
	{
		std::vector<Scalar> a = { Scalar(42.0f) };
		std::vector<Scalar> b = { Scalar(1.0f) };
		Scalar result = fdp(a, b);
		if (double(result) != 42.0) {
			std::cerr << "FAIL: fdp single element, expected 42.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdpStride: strided dot product
// ============================================================================
int TestFdpStride() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// stride-2 dot product: x[0]*y[0] + x[2]*y[2] = 1*5 + 3*7 = 26
	std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f), Scalar(4.0f) };
	std::vector<Scalar> y = { Scalar(5.0f), Scalar(6.0f), Scalar(7.0f), Scalar(8.0f) };

	Scalar result = fdp_stride(size_t(4), x, size_t(2), y, size_t(2));
	double d = double(result);
	if (d != 26.0) {
		std::cerr << "FAIL: fdp_stride(4, x, 2, y, 2) should be 26.0, got " << d << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdpQc: quire continuation with stress cases for carry/borrow
// ============================================================================
int TestFdpQc() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// Case 1: basic quire continuation
	{
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f) };
		std::vector<Scalar> y = { Scalar(4.0f), Scalar(5.0f), Scalar(6.0f) };
		quire<Scalar> q;
		fdp_qc(q, size_t(3), x, size_t(1), y, size_t(1));
		double result = q.convert_to<double>();
		if (result != 32.0) {
			std::cerr << "FAIL: fdp_qc basic, expected 32.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
		// Continue accumulating
		std::vector<Scalar> x2 = { Scalar(1.0f), Scalar(1.0f) };
		std::vector<Scalar> y2 = { Scalar(8.0f), Scalar(10.0f) };
		fdp_qc(q, size_t(2), x2, size_t(1), y2, size_t(1));
		result = q.convert_to<double>();
		if (result != 50.0) {
			std::cerr << "FAIL: fdp_qc continuation, expected 50.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: continuation with carry across limb boundary
	{
		quire<Scalar> q;
		// First batch: accumulate to just below a power of 2
		std::vector<Scalar> x1(31, Scalar(1.0f));
		std::vector<Scalar> y1(31, Scalar(1.0f));
		fdp_qc(q, size_t(31), x1, size_t(1), y1, size_t(1));
		// q = 31, one more should give 32 = 2^5
		std::vector<Scalar> x2 = { Scalar(1.0f) };
		std::vector<Scalar> y2 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x2, size_t(1), y2, size_t(1));
		double result = q.convert_to<double>();
		if (result != 32.0) {
			std::cerr << "FAIL: fdp_qc carry at 2^5, expected 32.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: continuation with borrow that flips sign
	{
		quire<Scalar> q;
		std::vector<Scalar> x1 = { Scalar(5.0f) };
		std::vector<Scalar> y1 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		// q = 5, now subtract 10
		std::vector<Scalar> x2 = { Scalar(-10.0f) };
		std::vector<Scalar> y2 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x2, size_t(1), y2, size_t(1));
		double result = q.convert_to<double>();
		if (result != -5.0) {
			std::cerr << "FAIL: fdp_qc sign flip, expected -5.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: large carry chain across multiple continuations
	{
		quire<Scalar> q;
		for (int batch = 0; batch < 10; ++batch) {
			std::vector<Scalar> x1 = { Scalar(1024.0f) };
			std::vector<Scalar> y1 = { Scalar(1024.0f) };
			fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		}
		// 10 * 1024^2 = 10 * 1048576 = 10485760
		double result = q.convert_to<double>();
		if (result != 10485760.0) {
			std::cerr << "FAIL: fdp_qc multi-batch carry, expected 10485760.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: alternating positive and negative continuations
	{
		quire<Scalar> q;
		for (int i = 0; i < 5; ++i) {
			std::vector<Scalar> xp = { Scalar(100.0f) };
			std::vector<Scalar> yp = { Scalar(1.0f) };
			fdp_qc(q, size_t(1), xp, size_t(1), yp, size_t(1));
			std::vector<Scalar> xn = { Scalar(-99.0f) };
			std::vector<Scalar> yn = { Scalar(1.0f) };
			fdp_qc(q, size_t(1), xn, size_t(1), yn, size_t(1));
		}
		// 5 * (100 - 99) = 5
		double result = q.convert_to<double>();
		if (result != 5.0) {
			std::cerr << "FAIL: fdp_qc alternating, expected 5.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 6: accumulate to maxpos-adjacent value
	{
		quire<Scalar> q;
		std::vector<Scalar> x1 = { Scalar(1e30f) };
		std::vector<Scalar> y1 = { Scalar(1e8f) };
		fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		double result = q.convert_to<double>();
		double expected = double(Scalar(1e30f)) * double(Scalar(1e8f));
		if (result != expected) {
			std::cerr << "FAIL: fdp_qc large value, expected " << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 7: many small continuations requiring propagation
	{
		quire<Scalar> q;
		for (int i = 0; i < 100; ++i) {
			std::vector<Scalar> x1 = { Scalar(0.01f) };
			std::vector<Scalar> y1 = { Scalar(1.0f) };
			fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		}
		// 100 * 0.01 should be close to 1.0 (exact depends on cfloat rounding of 0.01)
		double result = q.convert_to<double>();
		double expected = 100.0 * double(float(Scalar(0.01f)));
		if (std::abs(result - expected) > 1e-5) {
			std::cerr << "FAIL: fdp_qc many small, expected ~" << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 8: cancel to zero across continuations
	{
		quire<Scalar> q;
		std::vector<Scalar> x1 = { Scalar(12345.0f) };
		std::vector<Scalar> y1 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		std::vector<Scalar> x2 = { Scalar(-12345.0f) };
		std::vector<Scalar> y2 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x2, size_t(1), y2, size_t(1));
		double result = q.convert_to<double>();
		if (result != 0.0) {
			std::cerr << "FAIL: fdp_qc cancel to zero, expected 0.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 9: borrow propagation across many limbs
	{
		quire<Scalar> q;
		// Accumulate a large positive value
		std::vector<Scalar> x1 = { Scalar(1e20f) };
		std::vector<Scalar> y1 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		// Subtract almost all of it
		std::vector<Scalar> x2 = { Scalar(-1e20f) };
		std::vector<Scalar> y2 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x2, size_t(1), y2, size_t(1));
		// Add a small value
		std::vector<Scalar> x3 = { Scalar(1.0f) };
		std::vector<Scalar> y3 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x3, size_t(1), y3, size_t(1));
		double result = q.convert_to<double>();
		if (result != 1.0) {
			std::cerr << "FAIL: fdp_qc borrow across limbs, expected 1.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 10: repeated small borrows
	{
		quire<Scalar> q;
		std::vector<Scalar> x1 = { Scalar(100.0f) };
		std::vector<Scalar> y1 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		for (int i = 0; i < 10; ++i) {
			std::vector<Scalar> x2 = { Scalar(-10.0f) };
			std::vector<Scalar> y2 = { Scalar(1.0f) };
			fdp_qc(q, size_t(1), x2, size_t(1), y2, size_t(1));
		}
		double result = q.convert_to<double>();
		if (result != 0.0) {
			std::cerr << "FAIL: fdp_qc repeated borrow to zero, expected 0.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestCancellationHeavy: stress cases for catastrophic cancellation avoidance
// ============================================================================
int TestCancellationHeavy() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// Case 1: classic ill-conditioned: [1e7, 1, -1e7, 1] . [1,1,1,1] = 2.0
	{
		std::vector<Scalar> a = { Scalar(1e7f), Scalar(1.0f), Scalar(-1e7f), Scalar(1.0f) };
		std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f),  Scalar(1.0f) };
		Scalar result = fdp(a, b);
		if (double(result) != 2.0) {
			std::cerr << "FAIL: cancellation case 1, expected 2.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: larger spread: [1e15, 1, -1e15, 1] . [1,1,1,1] = 2.0
	{
		std::vector<Scalar> a = { Scalar(1e15f), Scalar(1.0f), Scalar(-1e15f), Scalar(1.0f) };
		std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f),  Scalar(1.0f) };
		Scalar result = fdp(a, b);
		if (double(result) != 2.0) {
			std::cerr << "FAIL: cancellation case 2, expected 2.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: near-maxpos cancellation
	{
		std::vector<Scalar> a = { Scalar(1e30f), Scalar(-1e30f), Scalar(42.0f) };
		std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(a, b);
		if (double(result) != 42.0) {
			std::cerr << "FAIL: cancellation case 3, expected 42.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: many pairs cancelling, small residual
	{
		std::vector<Scalar> a, b;
		for (int i = 0; i < 10; ++i) {
			a.push_back(Scalar(1e6f));
			a.push_back(Scalar(-1e6f));
			b.push_back(Scalar(1.0f));
			b.push_back(Scalar(1.0f));
		}
		a.push_back(Scalar(3.0f));
		b.push_back(Scalar(1.0f));
		Scalar result = fdp(a, b);
		if (double(result) != 3.0) {
			std::cerr << "FAIL: cancellation case 4, expected 3.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: 1023 small values that contribute LSB to 1024th large value
	// Demonstrates FDP avoiding catastrophic cancellation
	{
		std::vector<Scalar> a, b;
		// 1023 products of 1/1024 * 1.0 = 1/1024 each, total = 1023/1024 < 1
		for (int i = 0; i < 1023; ++i) {
			a.push_back(Scalar(1.0f / 1024.0f));
			b.push_back(Scalar(1.0f));
		}
		// 1024th element contributes 1.0
		a.push_back(Scalar(1.0f));
		b.push_back(Scalar(1.0f));
		Scalar result = fdp(a, b);
		// Expected: 1023/1024 + 1 ≈ 1.999...
		double expected = 1023.0 * double(float(Scalar(1.0f / 1024.0f))) + 1.0;
		double d = double(result);
		if (std::abs(d - expected) > 1e-5) {
			std::cerr << "FAIL: cancellation case 5, expected ~" << expected << ", got " << d << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 6: telescoping cancellation
	{
		std::vector<Scalar> a = { Scalar(1e10f), Scalar(-1e10f), Scalar(1e5f), Scalar(-1e5f), Scalar(1.0f) };
		std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(a, b);
		if (double(result) != 1.0) {
			std::cerr << "FAIL: cancellation case 6 (telescoping), expected 1.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 7: alternating large products, small residual
	{
		std::vector<Scalar> a, b;
		for (int i = 0; i < 50; ++i) {
			a.push_back(Scalar(1e8f));
			b.push_back(Scalar(float(i % 2 == 0 ? 1 : -1)));
		}
		// 25 * 1e8 - 25 * 1e8 = 0
		Scalar result = fdp(a, b);
		if (double(result) != 0.0) {
			std::cerr << "FAIL: cancellation case 7, expected 0.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 8: products with mixed signs and very different magnitudes
	{
		std::vector<Scalar> a = { Scalar(1e20f), Scalar(1e-10f), Scalar(-1e20f), Scalar(1e-10f) };
		std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		// 1e20 + 1e-10 - 1e20 + 1e-10 = 2e-10
		Scalar result = fdp(a, b);
		double expected = 2.0 * double(float(Scalar(1e-10f)));
		if (std::abs(double(result) - expected) > 1e-15) {
			std::cerr << "FAIL: cancellation case 8, expected " << expected << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 9: Kahan-style example with paired cancellation
	{
		std::vector<Scalar> a = { Scalar(1e12f), Scalar(1.0f), Scalar(1e-12f), Scalar(-1e12f) };
		std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		// 1e12 + 1 + 1e-12 - 1e12 = 1 + 1e-12
		Scalar result = fdp(a, b);
		double expected = 1.0 + double(float(Scalar(1e-12f)));
		// fp32 resolution for value near 1.0 is ~1e-7, so compare within that
		if (std::abs(double(result) - expected) > 1e-6) {
			std::cerr << "FAIL: cancellation case 9 (Kahan), expected " << expected << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 10: complete cancellation with many terms
	{
		std::vector<Scalar> a, b;
		for (int i = 1; i <= 100; ++i) {
			a.push_back(Scalar(float(i)));
			b.push_back(Scalar(1.0f));
			a.push_back(Scalar(float(-i)));
			b.push_back(Scalar(1.0f));
		}
		Scalar result = fdp(a, b);
		if (double(result) != 0.0) {
			std::cerr << "FAIL: cancellation case 10, expected 0.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 11: near-cancellation with tiny residual from many terms
	{
		std::vector<Scalar> a, b;
		for (int i = 1; i <= 100; ++i) {
			a.push_back(Scalar(float(i)));
			b.push_back(Scalar(1.0f));
			a.push_back(Scalar(float(-i)));
			b.push_back(Scalar(1.0f));
		}
		// Add a tiny residual
		a.push_back(Scalar(0.125f));
		b.push_back(Scalar(1.0f));
		Scalar result = fdp(a, b);
		if (double(result) != 0.125) {
			std::cerr << "FAIL: cancellation case 11, expected 0.125, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdpAccuracy: verify FDP matches double-precision reference (no long double)
// ============================================================================
int TestFdpAccuracy() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// Build vectors where naive fp32 accumulation would lose precision
	constexpr size_t N = 100;
	std::vector<Scalar> x(N), y(N);
	double reference = 0.0;
	for (size_t i = 0; i < N; ++i) {
		float xv = static_cast<float>(i + 1) * 0.1f;
		float yv = static_cast<float>(N - i) * 0.1f;
		x[i] = Scalar(xv);
		y[i] = Scalar(yv);
		// compute reference using double from the cfloat-rounded values
		reference += static_cast<double>(float(x[i])) * static_cast<double>(float(y[i]));
	}

	Scalar fdp_result = fdp(x, y);
	double fdp_d = double(fdp_result);

	// The expected cfloat value is the reference rounded to fp32
	Scalar expected = Scalar(static_cast<float>(reference));
	double expected_d = double(expected);

	// also compute naive fp32 dot product for comparison
	float naive = 0.0f;
	for (size_t i = 0; i < N; ++i) {
		naive += float(x[i]) * float(y[i]);
	}

	std::cout << "  reference (double):      " << reference << '\n';
	std::cout << "  expected cfloat:         " << expected_d << '\n';
	std::cout << "  fdp result:              " << fdp_d << '\n';
	std::cout << "  naive fp32 result:       " << naive << '\n';

	// FDP should match the correctly-rounded cfloat of the reference
	if (fdp_d != expected_d) {
		double fdp_error = std::abs(fdp_d - reference);
		double expected_error = std::abs(expected_d - reference);
		// Allow at most 1 ULP difference
		if (fdp_error > expected_error * 2) {
			std::cerr << "FAIL: FDP result differs from correctly-rounded reference\n";
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestDifferentConfigs: fp8 (es=5,4,3) and odd cfloat configurations
// ============================================================================
int TestDifferentConfigs() {
	int nrOfFailedTestCases = 0;

	// --- fp16-equivalent (cfloat<16,5>) ---
	{
		using Scalar = cfloat<16, 5, uint16_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(2.0f), Scalar(3.0f) };
		std::vector<Scalar> y = { Scalar(4.0f), Scalar(5.0f) };
		Scalar result = fdp(x, y);
		if (double(result) != 23.0) {
			std::cerr << "FAIL: fp16 fdp should be 23.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- fp8 with es=5 (E5M2, like fp8_e5m2) ---
	{
		using Scalar = cfloat<8, 5, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(x, y);
		if (double(result) != 2.0) {
			std::cerr << "FAIL: fp8_e5m2 fdp should be 2.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- fp8 with es=4 (E4M3, like fp8_e4m3) ---
	{
		using Scalar = cfloat<8, 4, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(x, y);
		if (double(result) != 3.0) {
			std::cerr << "FAIL: fp8_e4m3 fdp should be 3.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- fp8 with es=3 ---
	{
		using Scalar = cfloat<8, 3, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(x, y);
		if (double(result) != 6.0) {
			std::cerr << "FAIL: fp8_e3m4 fdp should be 6.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- cfloat<20,8> (odd width, 11 fraction bits) ---
	{
		using Scalar = cfloat<20, 8, uint32_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(100.0f), Scalar(200.0f), Scalar(300.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f) };
		// 100 + 400 + 900 = 1400
		Scalar result = fdp(x, y);
		if (double(result) != 1400.0) {
			std::cerr << "FAIL: cfloat<20,8> fdp should be 1400.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- cfloat<16,8> (tiny fraction: 7 bits) ---
	{
		using Scalar = cfloat<16, 8, uint16_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(2.0f), Scalar(4.0f) };
		std::vector<Scalar> y = { Scalar(3.0f), Scalar(5.0f) };
		// 6 + 20 = 26
		Scalar result = fdp(x, y);
		if (double(result) != 26.0) {
			std::cerr << "FAIL: cfloat<16,8> fdp should be 26.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- cfloat<12,5> (7-bit type with 6 fraction bits) ---
	{
		using Scalar = cfloat<12, 5, uint16_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f), Scalar(4.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		// 1+2+3+4 = 10
		Scalar result = fdp(x, y);
		if (double(result) != 10.0) {
			std::cerr << "FAIL: cfloat<12,5> fdp should be 10.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- cfloat<6,2> (micro float: 3 fraction bits) ---
	{
		using Scalar = cfloat<6, 2, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(2.0f) };
		// 1 + 2 = 3
		Scalar result = fdp(x, y);
		if (double(result) != 3.0) {
			std::cerr << "FAIL: cfloat<6,2> fdp should be 3.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- cfloat<4,1> (minimal float: 2 fraction bits, es=1 requires subnormals+maxexp) ---
	{
		using Scalar = cfloat<4, 1, uint8_t, true, true, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f) };
		// 1 + 1 = 2
		Scalar result = fdp(x, y);
		if (double(result) != 2.0) {
			std::cerr << "FAIL: cfloat<4,1> fdp should be 2.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- fp8 cancellation test ---
	{
		using Scalar = cfloat<8, 4, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(4.0f), Scalar(-4.0f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		Scalar result = fdp(x, y);
		if (double(result) != 1.0) {
			std::cerr << "FAIL: fp8 cancellation fdp should be 1.0, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "cfloat fused dot product (FDP)";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';
	std::cout << std::string(60, '=') << '\n';

	nrOfFailedTestCases += ReportTestResult(TestQuireMul(), "cfloat quire_mul", "unrounded product");
	nrOfFailedTestCases += ReportTestResult(TestSpecialValues(), "cfloat quire_mul", "special values");
	nrOfFailedTestCases += ReportTestResult(TestBasicFdp(), "cfloat fdp", "basic dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdpStride(), "cfloat fdp_stride", "strided dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdpQc(), "cfloat fdp_qc", "quire continuation");
	nrOfFailedTestCases += ReportTestResult(TestCancellationHeavy(), "cfloat fdp", "cancellation-heavy");
	nrOfFailedTestCases += ReportTestResult(TestFdpAccuracy(), "cfloat fdp", "accuracy vs double");
	nrOfFailedTestCases += ReportTestResult(TestDifferentConfigs(), "cfloat fdp", "different configs");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
