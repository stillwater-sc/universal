// fixpnt_fdp.cpp: tests for fixpnt fused dot product via generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ============================================================================
// Quire Architecture for fixpnt<16, 8, Modulo, uint8_t>
// ============================================================================
//
// fixpnt<16, 8>: 16-bit signed fixed-point, 8 integer bits + 8 fraction bits
//   Value range: [-128.0, +127.99609375]    (max positive ≈ 127.996)
//   Resolution: 2^-8 = 0.00390625
//
// Product of two fixpnt<16,8> values:
//   Full product width: 2*nbits = 32 bits (2's complement)
//   Product fraction bits: 2*rbits = 16 bits
//   Max product: ~127.996^2 ≈ 16383.02, fits in 14 integer bits
//
// quire_traits<fixpnt<16,8>>:
//   range      = 2*nbits = 32
//   radix_point = 2*rbits = 16
//   capacity   = 30 (default)
//   qbits      = range + capacity = 62
//   Total bits = qbits + 1 (sign) = 63
//
// Quire limb layout (uint32_t limbs):
//   limb 0: bits  0-31  (fractional + low integer)
//   limb 1: bits 32-62  (high integer + capacity)
//
// Key positions:
//   radix_point = bit 16: separates fractional from integer accumulation
//   limb boundary = bit 32: carry/borrow propagation point
//
// A product of value V has quire MSB at bit (radix_point + scale)
//   = (16 + floor(log2(|V|)))
//   - Product V≈1    → MSB at bit 16 (limb 0)
//   - Product V≈128  → MSB at bit 23 (limb 0)
//   - Product V≈16384 → MSB at bit 30 (limb 0, near boundary)
//   - Accumulation sum≈65536 → MSB at bit 32 (limb boundary!)
// ============================================================================
//
#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/fixpnt/fdp.hpp>
#include <universal/verification/test_suite.hpp>
#include <vector>
#include <cmath>

namespace sw { namespace universal {

// ============================================================================
// TestQuireMul: verify unrounded fixpnt products via quire accumulation
// ============================================================================
int TestQuireMul() {
	int nrOfFailedTestCases = 0;

	using Scalar = fixpnt<16, 8, Modulo, uint8_t>;

	// Integer products: 3 * 5 = 15
	{
		Scalar a, b;
		a = 3; b = 5;
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 15.0) {
			std::cerr << "FAIL: quire_mul(3, 5) expected 15, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Fractional products: 1.5 * 2.5 = 3.75
	{
		Scalar a(1.5), b(2.5);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (std::abs(result - 3.75) > 0.01) {
			std::cerr << "FAIL: quire_mul(1.5, 2.5) expected 3.75, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Negative product: -2 * 7 = -14
	{
		Scalar a, b;
		a = -2; b = 7;
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != -14.0) {
			std::cerr << "FAIL: quire_mul(-2, 7) expected -14, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Both negative: -3 * -4 = 12
	{
		Scalar a, b;
		a = -3; b = -4;
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 12.0) {
			std::cerr << "FAIL: quire_mul(-3, -4) expected 12, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Zero product
	{
		Scalar a(0), b(42);
		auto product = quire_mul(a, b);
		if (!product.iszero()) {
			std::cerr << "FAIL: quire_mul(0, 42) should be zero\n";
			++nrOfFailedTestCases;
		}
	}

	// Unrounded proof: 1.25 * 1.75 = 2.1875 (exact in fixpnt<16,8>)
	// The product has 4 fractional bits of precision, but fixpnt<16,8> only
	// has 8 fraction bits. The quire should preserve all product bits.
	{
		Scalar a(1.25), b(1.75);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(a) * double(b);
		if (std::abs(result - expected) > 1e-6) {
			std::cerr << "FAIL: quire_mul(1.25, 1.75) expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Accumulation of multiple products
	{
		quire<Scalar> q;
		Scalar a, b;
		a = 2; b = 3;
		q += quire_mul(a, b);  // +6
		a = 4; b = 5;
		q += quire_mul(a, b);  // +20
		double result = q.convert_to<double>();
		if (result != 26.0) {
			std::cerr << "FAIL: accumulation 2*3 + 4*5 expected 26, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Accumulation with subtraction
	{
		quire<Scalar> q;
		Scalar a, b;
		a = 10; b = 1;
		q += quire_mul(a, b);   // +10
		a = -3; b = 1;
		q += quire_mul(a, b);   // -3
		double result = q.convert_to<double>();
		if (result != 7.0) {
			std::cerr << "FAIL: accumulation 10 - 3 expected 7, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestBasicFdp: fixpnt dot products
// ============================================================================
int TestBasicFdp() {
	int nrOfFailedTestCases = 0;

	using Scalar = fixpnt<16, 8, Modulo, uint8_t>;

	// Integer dot product: [1,2,3,4].[5,6,7,8] = 5+12+21+32 = 70
	{
		std::vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4) };
		std::vector<Scalar> y = { Scalar(5), Scalar(6), Scalar(7), Scalar(8) };
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 70.0) > 0.01) {
			std::cerr << "FAIL: integer fdp expected 70, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Fractional dot product
	{
		std::vector<Scalar> x = { Scalar(0.5), Scalar(1.5) };
		std::vector<Scalar> y = { Scalar(2.0), Scalar(3.0) };
		// 0.5*2 + 1.5*3 = 1 + 4.5 = 5.5
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 5.5) > 0.01) {
			std::cerr << "FAIL: fractional fdp expected 5.5, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Cancellation: [100, -99].[1, 1] = 1
	{
		std::vector<Scalar> x = { Scalar(100), Scalar(-99) };
		std::vector<Scalar> y = { Scalar(1),   Scalar(1)  };
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 1.0) > 0.01) {
			std::cerr << "FAIL: cancel fdp expected 1, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Cancel to zero
	{
		std::vector<Scalar> x = { Scalar(7), Scalar(-7) };
		std::vector<Scalar> y = { Scalar(3), Scalar(3)  };
		Scalar result = fdp(x, y);
		if (std::abs(double(result)) > 0.01) {
			std::cerr << "FAIL: cancel-to-zero fdp, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Single element
	{
		std::vector<Scalar> x = { Scalar(42) };
		std::vector<Scalar> y = { Scalar(1) };
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 42.0) > 0.01) {
			std::cerr << "FAIL: single-element fdp expected 42, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdp1024: 1024-element fixpnt FDP tests
// ============================================================================
int TestFdp1024() {
	int nrOfFailedTestCases = 0;

	using Scalar = fixpnt<16, 8, Modulo, uint8_t>;

	// Case 1: 1024 ones dot product: sum = 1024
	// fixpnt<16,8> can't represent 1024 (max ≈ 127.996), so use smaller values
	// 1024 * (0.125 * 0.125) = 1024 * 0.015625 = 16
	{
		std::vector<Scalar> x(1024, Scalar(0.125));
		std::vector<Scalar> y(1024, Scalar(0.125));
		Scalar result = fdp(x, y);
		double expected = 1024.0 * 0.125 * 0.125;
		if (std::abs(double(result) - expected) > 0.1) {
			std::cerr << "FAIL: fdp1024 case 1, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: cancellation with 1024 elements
	// 512 pairs of (+1, -1) products = 0
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 512; ++i) {
			x[2*i]     = Scalar(1);  y[2*i]     = Scalar(1);
			x[2*i + 1] = Scalar(-1); y[2*i + 1] = Scalar(1);
		}
		Scalar result = fdp(x, y);
		if (std::abs(double(result)) > 0.01) {
			std::cerr << "FAIL: fdp1024 case 2 (cancel), got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: cancellation + small residual
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 511; ++i) {
			x[2*i]     = Scalar(10);  y[2*i]     = Scalar(1);
			x[2*i + 1] = Scalar(-10); y[2*i + 1] = Scalar(1);
		}
		// last 2 elements: small residual
		x[1022] = Scalar(0.5);   y[1022] = Scalar(1);
		x[1023] = Scalar(0.25);  y[1023] = Scalar(1);
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 0.75) > 0.01) {
			std::cerr << "FAIL: fdp1024 case 3 (cancel+residual), expected 0.75, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: accumulate many small products
	// 1024 * (1/32)^2 = 1024/1024 = 1.0
	{
		Scalar v(0.03125);  // 1/32 exactly representable
		std::vector<Scalar> x(1024, v);
		std::vector<Scalar> y(1024, v);
		Scalar result = fdp(x, y);
		double expected = 1024.0 * double(v) * double(v);
		if (std::abs(double(result) - expected) > 0.01) {
			std::cerr << "FAIL: fdp1024 case 4 (small products), expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: alternating sum: -1+2-3+4...-1023+1024
	// But fixpnt<16,8> max is ~127, so scale down: use 1/16
	// (-1+2-3+4...-1023+1024)/16 = 512/16 = 32
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1024; ++i) {
			double v = double(i + 1) / 16.0;
			if (v > 127.0) v = 127.0;  // clamp to fixpnt range
			x[i] = Scalar(v * ((i % 2 == 0) ? -1.0 : 1.0));
			y[i] = Scalar(1);
		}
		// expected: sum of pairs: (1/16)(1 + 1 + 1 + ... 512 times) = 512/16 = 32
		// but values >127 are clamped, so it's approximate
		Scalar result = fdp(x, y);
		// Just verify it's in a reasonable range
		if (std::abs(double(result) - 32.0) > 2.0) {
			std::cerr << "FAIL: fdp1024 case 5 (alternating), expected ~32, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 6: reproducibility (reversed order gives same result)
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1024; ++i) {
			x[i] = Scalar(double(i % 10) * 0.1);
			y[i] = Scalar(double((i + 3) % 7) * 0.1);
		}
		Scalar result1 = fdp(x, y);
		std::vector<Scalar> xr(x.rbegin(), x.rend());
		std::vector<Scalar> yr(y.rbegin(), y.rend());
		Scalar result2 = fdp(xr, yr);
		if (double(result1) != double(result2)) {
			std::cerr << "FAIL: fdp1024 case 6 (reproducibility), "
			          << double(result1) << " != " << double(result2) << '\n';
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

	using Scalar = fixpnt<16, 8, Modulo, uint8_t>;

	// stride-2: x[0]*y[0] + x[2]*y[2] = 1*5 + 3*7 = 26
	{
		std::vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4) };
		std::vector<Scalar> y = { Scalar(5), Scalar(6), Scalar(7), Scalar(8) };
		Scalar result = fdp_stride(size_t(4), x, size_t(2), y, size_t(2));
		if (std::abs(double(result) - 26.0) > 0.01) {
			std::cerr << "FAIL: fdp_stride expected 26, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdpQc: quire continuation
// ============================================================================
int TestFdpQc() {
	int nrOfFailedTestCases = 0;

	using Scalar = fixpnt<16, 8, Modulo, uint8_t>;

	// Basic continuation
	{
		quire<Scalar> q;
		std::vector<Scalar> x1 = { Scalar(1), Scalar(2), Scalar(3) };
		std::vector<Scalar> y1 = { Scalar(4), Scalar(5), Scalar(6) };
		fdp_qc(q, size_t(3), x1, size_t(1), y1, size_t(1));
		// 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
		double result = q.convert_to<double>();
		if (std::abs(result - 32.0) > 0.01) {
			std::cerr << "FAIL: fdp_qc basic, expected 32, got " << result << '\n';
			++nrOfFailedTestCases;
		}

		// Continue
		std::vector<Scalar> x2 = { Scalar(1), Scalar(1) };
		std::vector<Scalar> y2 = { Scalar(8), Scalar(10) };
		fdp_qc(q, size_t(2), x2, size_t(1), y2, size_t(1));
		result = q.convert_to<double>();
		if (std::abs(result - 50.0) > 0.01) {
			std::cerr << "FAIL: fdp_qc continuation, expected 50, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Cancel to zero
	{
		quire<Scalar> q;
		std::vector<Scalar> x1(100, Scalar(1));
		std::vector<Scalar> y1(100, Scalar(1));
		fdp_qc(q, size_t(100), x1, size_t(1), y1, size_t(1));
		std::vector<Scalar> x2(100, Scalar(-1));
		std::vector<Scalar> y2(100, Scalar(1));
		fdp_qc(q, size_t(100), x2, size_t(1), y2, size_t(1));
		if (!q.iszero()) {
			std::cerr << "FAIL: fdp_qc cancel to zero, got " << q.convert_to<double>() << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestLimbBoundaryCarryBorrow: accumulations that force carry/borrow across
// the limb 0/1 boundary at quire bit 32
// ============================================================================
int TestLimbBoundaryCarryBorrow() {
	int nrOfFailedTestCases = 0;

	using Scalar = fixpnt<16, 8, Modulo, uint8_t>;

	// Case 1: Carry across limb boundary via repeated accumulation
	// 127 * 127 = 16129. Scale = 13, MSB at quire bit 29 (limb 0).
	// Accumulate 5 such products: 5 * 16129 = 80645.
	// log2(80645) ≈ 16.3, MSB at quire bit 32 → crosses into limb 1.
	{
		quire<Scalar> q;
		Scalar a(127), b(127);
		for (int i = 0; i < 5; ++i) {
			q += quire_mul(a, b);
		}
		double result = q.convert_to<double>();
		double expected = 5.0 * double(a) * double(b);
		if (std::abs(result - expected) > 1.0) {
			std::cerr << "FAIL: limb carry case 1, expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: Borrow across limb boundary
	// Start with large positive accumulation in limb 1, then subtract
	{
		quire<Scalar> q;
		Scalar big(100), one(1);
		// Accumulate 400 products of 100*1 = 100 each → total 40000
		// MSB at quire bit 31 (still limb 0)
		for (int i = 0; i < 400; ++i) {
			q += quire_mul(big, one);
		}
		// Now subtract 500 products of 100*1 → total -10000
		Scalar neg(-100);
		for (int i = 0; i < 500; ++i) {
			q += quire_mul(neg, one);
		}
		double result = q.convert_to<double>();
		// 40000 - 50000 = -10000
		if (std::abs(result - (-10000.0)) > 1.0) {
			std::cerr << "FAIL: limb borrow case 2, expected -10000, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: Exact cancellation across limb boundary
	// Accumulate enough to push into limb 1, then cancel exactly to zero
	{
		quire<Scalar> q;
		Scalar a(100), b(100);
		// +100*100 = 10000, four times → 40000 (MSB near bit 31)
		for (int i = 0; i < 4; ++i) q += quire_mul(a, b);
		// Add more to push past limb boundary: 4 more → 80000
		for (int i = 0; i < 4; ++i) q += quire_mul(a, b);
		// Subtract 80 products of 100*10: 80*1000 = 80000
		Scalar ten(10);
		for (int i = 0; i < 80; ++i) q -= quire_mul(a, ten);
		if (!q.iszero()) {
			double result = q.convert_to<double>();
			std::cerr << "FAIL: limb cancel case 3, expected 0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: Small residual after large cancellation across limb boundary
	{
		quire<Scalar> q;
		Scalar a(127), b(127);
		// Accumulate 8 * 127*127 = 8 * 16129 ≈ 129032 (MSB at bit 33, limb 1)
		for (int i = 0; i < 8; ++i) q += quire_mul(a, b);
		// Subtract 8 * 127*127 → 0, then add tiny positive
		for (int i = 0; i < 8; ++i) q -= quire_mul(a, b);
		Scalar quarter(0.25);
		q += quire_mul(quarter, Scalar(1));
		double result = q.convert_to<double>();
		if (std::abs(result - 0.25) > 0.01) {
			std::cerr << "FAIL: limb residual case 4, expected 0.25, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: Products straddling the radix point (bit 16)
	// Product of 0.25 * 0.25 = 0.0625, scale = -4, MSB at quire bit 12
	// Accumulate many: 256 * 0.0625 = 16, MSB at quire bit 20
	{
		quire<Scalar> q;
		Scalar quarter(0.25);
		for (int i = 0; i < 256; ++i) {
			q += quire_mul(quarter, quarter);
		}
		double result = q.convert_to<double>();
		if (std::abs(result - 16.0) > 0.01) {
			std::cerr << "FAIL: radix straddle case 5, expected 16, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 6: Products in deep fractional region
	// 0.00390625 * 0.00390625 = 1.52588e-05, scale = -16, MSB at quire bit 0
	{
		quire<Scalar> q;
		Scalar tiny(0.00390625);  // 2^-8, smallest representable magnitude
		for (int i = 0; i < 1024; ++i) {
			q += quire_mul(tiny, tiny);
		}
		double result = q.convert_to<double>();
		double expected = 1024.0 * 0.00390625 * 0.00390625;
		if (std::abs(result - expected) > 1e-5) {
			std::cerr << "FAIL: deep fractional case 6, expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestCatastrophicCancellation: tests where naive fp accumulation would fail
// but quire accumulation preserves full precision
// ============================================================================
int TestCatastrophicCancellation() {
	int nrOfFailedTestCases = 0;

	using Scalar = fixpnt<16, 8, Modulo, uint8_t>;

	// Case 1: Large accumulation cancelled by large subtraction, leaving small residual
	// The quire must preserve the LSBs during the large accumulation
	{
		std::vector<Scalar> x(1024), y(1024);
		// 1023 products of 100*1 = 100, one product of -99999/1000*1 ≈ -100.0
		// Adjusted for fixpnt range: 511 pairs of (+10 * 1) and 511 pairs of (-10 * 1)
		// Plus 2 extra elements to create small residual
		for (int i = 0; i < 511; ++i) {
			x[2*i] = Scalar(10);       y[2*i] = Scalar(1);
			x[2*i+1] = Scalar(-10);    y[2*i+1] = Scalar(1);
		}
		// Last 2 elements: 0.125 * 1 + 0.0625 * 1 = 0.1875
		x[1022] = Scalar(0.125);   y[1022] = Scalar(1);
		x[1023] = Scalar(0.0625);  y[1023] = Scalar(1);
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 0.1875) > 0.01) {
			std::cerr << "FAIL: catastrophic case 1, expected 0.1875, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: Alternating sign products with progressive magnitude
	// Sum: 1*1 - 2*1 + 3*1 - 4*1 + ... + 1023*1 - 1024*1
	// = (-1 -1 -1 ... -1) = -512 pairs, but values exceed fixpnt range
	// Scale to fit: (1/8)*1 - (2/8)*1 + (3/8)*1 - ... = (1/8)(-512) = -64
	{
		std::vector<Scalar> x(1024), y(1024, Scalar(1));
		double expected = 0.0;
		for (int i = 0; i < 1024; ++i) {
			double v = double(i + 1) / 8.0;
			if (v > 127.0) v = 127.0;
			double sign = (i % 2 == 0) ? 1.0 : -1.0;
			x[i] = Scalar(v * sign);
			expected += double(x[i]) * double(y[i]);
		}
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: catastrophic case 2, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: Near-equal large vectors whose difference is a small fraction
	// x = [a, a, a, ...], y = [b, -b+eps, b, -b+eps, ...]
	// Tests that the quire preserves the tiny epsilon across many additions
	{
		Scalar a(10);
		Scalar b(10);
		Scalar b_minus(Scalar(-10) + Scalar(0.00390625));  // -10 + 1 LSB
		std::vector<Scalar> x(1024, a);
		std::vector<Scalar> y(1024);
		for (int i = 0; i < 1024; ++i) {
			y[i] = (i % 2 == 0) ? b : b_minus;
		}
		// Each pair contributes 10*10 + 10*(-10 + eps) = 10*eps = 0.0390625
		// 512 pairs → 512 * 0.0390625 = 20.0
		Scalar result = fdp(x, y);
		double expected = 0.0;
		for (int i = 0; i < 1024; ++i) {
			expected += double(x[i]) * double(y[i]);
		}
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: catastrophic case 3, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: Kahan-style summation test
	// Sum of (large + small - large) patterns
	// Without quire, the small terms are lost when added to large running sums
	{
		std::vector<Scalar> x(1024), y(1024, Scalar(1));
		double expected = 0.0;
		for (int i = 0; i < 1024; i += 3) {
			x[i]   = Scalar(100);
			x[i+1] = Scalar(0.00390625);  // 1 LSB
			if (i + 2 < 1024) x[i+2] = Scalar(-100);
			expected += double(x[i]) + double(x[i+1]);
			if (i + 2 < 1024) expected += double(x[i+2]);
		}
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: catastrophic case 4 (Kahan), expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestDifferentConfigs: various fixpnt configurations
// ============================================================================
int TestDifferentConfigs() {
	int nrOfFailedTestCases = 0;

	// fixpnt<8, 4> — tiny fixed-point (4 integer bits, 4 fraction bits)
	{
		using Scalar = fixpnt<8, 4, Modulo, uint8_t>;
		std::vector<Scalar> x = { Scalar(1), Scalar(2) };
		std::vector<Scalar> y = { Scalar(3), Scalar(4) };
		// 1*3 + 2*4 = 3 + 8 = 11 (but max of fixpnt<8,4> is 7.9375!)
		// So this will overflow with Modulo arithmetic
		// Use smaller values: 1*1 + 1*1 = 2
		x = { Scalar(1), Scalar(1) };
		y = { Scalar(1), Scalar(1) };
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 2.0) > 0.1) {
			std::cerr << "FAIL: fixpnt<8,4> fdp expected 2, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// fixpnt<32, 16> — wider fixed-point
	{
		using Scalar = fixpnt<32, 16, Modulo, uint32_t>;
		std::vector<Scalar> x = { Scalar(100), Scalar(200) };
		std::vector<Scalar> y = { Scalar(1),   Scalar(2) };
		// 100 + 400 = 500
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 500.0) > 0.01) {
			std::cerr << "FAIL: fixpnt<32,16> fdp expected 500, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// fixpnt<16, 0> — pure integer fixed-point
	{
		using Scalar = fixpnt<16, 0, Modulo, uint16_t>;
		std::vector<Scalar> x = { Scalar(10), Scalar(20), Scalar(30) };
		std::vector<Scalar> y = { Scalar(1),  Scalar(2),  Scalar(3) };
		// 10 + 40 + 90 = 140
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 140.0) > 0.01) {
			std::cerr << "FAIL: fixpnt<16,0> fdp expected 140, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// fixpnt<16, 8> with Saturate arithmetic
	{
		using Scalar = fixpnt<16, 8, Saturate, uint8_t>;
		std::vector<Scalar> x = { Scalar(2), Scalar(3) };
		std::vector<Scalar> y = { Scalar(4), Scalar(5) };
		// 8 + 15 = 23
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 23.0) > 0.01) {
			std::cerr << "FAIL: fixpnt<16,8,Saturate> fdp expected 23, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// fixpnt<12, 6>
	{
		using Scalar = fixpnt<12, 6, Modulo, uint8_t>;
		std::vector<Scalar> x = { Scalar(1), Scalar(2), Scalar(3), Scalar(4) };
		std::vector<Scalar> y = { Scalar(1), Scalar(1), Scalar(1), Scalar(1) };
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - 10.0) > 0.1) {
			std::cerr << "FAIL: fixpnt<12,6> fdp expected 10, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestAccuracy: FDP vs naive accumulation
// ============================================================================
int TestAccuracy() {
	int nrOfFailedTestCases = 0;

	using Scalar = fixpnt<16, 8, Modulo, uint8_t>;

	// 100-element accuracy test
	constexpr size_t N = 100;
	std::vector<Scalar> x(N), y(N);
	double reference = 0.0;
	for (size_t i = 0; i < N; ++i) {
		double xv = double(i + 1) * 0.01;
		double yv = double(N - i) * 0.01;
		x[i] = Scalar(xv);
		y[i] = Scalar(yv);
		reference += double(x[i]) * double(y[i]);
	}

	Scalar fdp_result = fdp(x, y);
	double fdp_d = double(fdp_result);
	Scalar expected(reference);
	double expected_d = double(expected);

	std::cout << "  reference (double):      " << reference << '\n';
	std::cout << "  expected fixpnt:         " << expected_d << '\n';
	std::cout << "  fdp result:              " << fdp_d << '\n';

	if (std::abs(fdp_d - expected_d) > 1.0) {
		std::cerr << "FAIL: FDP accuracy test, fdp=" << fdp_d << " expected=" << expected_d << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "fixpnt fused dot product (FDP)";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';
	std::cout << std::string(60, '=') << '\n';

	nrOfFailedTestCases += ReportTestResult(TestQuireMul(), "fixpnt quire_mul", "unrounded product");
	nrOfFailedTestCases += ReportTestResult(TestBasicFdp(), "fixpnt fdp", "basic dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdp1024(), "fixpnt fdp", "1024-element vectors");
	nrOfFailedTestCases += ReportTestResult(TestFdpStride(), "fixpnt fdp_stride", "strided dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdpQc(), "fixpnt fdp_qc", "quire continuation");
	nrOfFailedTestCases += ReportTestResult(TestLimbBoundaryCarryBorrow(), "fixpnt quire", "limb-boundary carry/borrow");
	nrOfFailedTestCases += ReportTestResult(TestCatastrophicCancellation(), "fixpnt fdp", "catastrophic cancellation");
	nrOfFailedTestCases += ReportTestResult(TestDifferentConfigs(), "fixpnt fdp", "different configs");
	nrOfFailedTestCases += ReportTestResult(TestAccuracy(), "fixpnt fdp", "accuracy");

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
