// lns_fdp.cpp: tests for lns fused dot product via generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ============================================================================
// Quire Architecture for lns<16, 8, uint8_t>
// ============================================================================
//
// lns<16, 8>: 16-bit logarithmic number system
//   1 sign bit, 7 integer exponent bits, 8 fractional exponent bits
//   Value = (-1)^sign * 2^(integer.fraction)
//   Dynamic range: 2^(-128) to 2^(+127)
//   Precision: 2^(-8) = 1/256 exponent resolution
//
// quire_traits<lns<16,8>>:
//   integer_bits = 7
//   max_exponent = 128
//   range        = 256
//   radix_point  = 128
//   capacity     = 30
//   qbits        = 286
//   Total bits   = 287 (including sign)
//
// Product of two lns values: exponent sum has 8 integer bits + 16 fractional bits
// The double-precision conversion path is exact for lns types with <=52 exponent
// fractional bits.
//
// Quire limb layout (uint32_t limbs, 10 limbs for 287 bits):
//   limb 0: bits   0-31   (deep negative exponents)
//   limb 1: bits  32-63
//   limb 2: bits  64-95
//   limb 3: bits  96-127
//   limb 4: bits 128-159  (radix point at bit 128)
//   limb 5: bits 160-191
//   ...
// ============================================================================
//
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/fdp.hpp>
#include <universal/verification/test_suite.hpp>
#include <vector>
#include <cmath>

namespace sw { namespace universal {

// ============================================================================
// TestQuireMul: verify lns products via quire accumulation
// ============================================================================
int TestQuireMul() {
	int nrOfFailedTestCases = 0;

	using Scalar = lns<16, 8, uint8_t>;

	// Product of powers of 2: 2^3 * 2^4 = 2^7 = 128
	{
		Scalar a(8.0), b(16.0);   // 2^3 and 2^4
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (std::abs(result - 128.0) > 0.01) {
			std::cerr << "FAIL: quire_mul(8, 16) expected 128, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Non-power-of-2 product: 3.0 * 5.0 = 15.0
	// lns approximates these, so use tolerance
	{
		Scalar a(3.0), b(5.0);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(a) * double(b);
		if (std::abs(result - expected) > 0.5) {
			std::cerr << "FAIL: quire_mul(3, 5) expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Negative product: -2 * 4 = -8
	{
		Scalar a(-2.0), b(4.0);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(a) * double(b);
		if (std::abs(result - expected) > 0.5) {
			std::cerr << "FAIL: quire_mul(-2, 4) expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Both negative: -3 * -4 = 12
	{
		Scalar a(-3.0), b(-4.0);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(a) * double(b);
		if (std::abs(result - expected) > 0.5) {
			std::cerr << "FAIL: quire_mul(-3, -4) expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Zero product
	{
		Scalar a, b(42.0);
		a.setzero();
		auto product = quire_mul(a, b);
		if (!product.iszero()) {
			std::cerr << "FAIL: quire_mul(0, 42) should be zero\n";
			++nrOfFailedTestCases;
		}
	}

	// Accumulation of multiple products
	{
		quire<Scalar> q;
		Scalar a(2.0), b(4.0);
		q += quire_mul(a, b);  // 2*4 = 8
		Scalar c(1.0), d(1.0);
		q += quire_mul(c, d);  // 1*1 = 1
		double result = q.convert_to<double>();
		double expected = double(a) * double(b) + double(c) * double(d);
		if (std::abs(result - expected) > 0.5) {
			std::cerr << "FAIL: accumulation 2*4 + 1*1 expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Accumulation with subtraction
	{
		quire<Scalar> q;
		Scalar a(16.0), b(1.0);
		q += quire_mul(a, b);   // +16
		Scalar c(-4.0);
		q += quire_mul(c, b);   // -4
		double result = q.convert_to<double>();
		double expected = double(a) + double(c);
		if (std::abs(result - expected) > 0.5) {
			std::cerr << "FAIL: accumulation 16 - 4 expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestBasicFdp: lns dot products
// ============================================================================
int TestBasicFdp() {
	int nrOfFailedTestCases = 0;

	using Scalar = lns<16, 8, uint8_t>;

	// Power-of-2 dot product: [1,2,4,8].[1,1,1,1] = 15
	{
		std::vector<Scalar> x = { Scalar(1.0), Scalar(2.0), Scalar(4.0), Scalar(8.0) };
		std::vector<Scalar> y = { Scalar(1.0), Scalar(1.0), Scalar(1.0), Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = 0.0;
		for (size_t i = 0; i < 4; ++i) expected += double(x[i]) * double(y[i]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: power-of-2 fdp expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// General dot product: [3,5].[2,7] ≈ 6 + 35 = 41
	{
		std::vector<Scalar> x = { Scalar(3.0), Scalar(5.0) };
		std::vector<Scalar> y = { Scalar(2.0), Scalar(7.0) };
		Scalar result = fdp(x, y);
		double expected = 0.0;
		for (size_t i = 0; i < 2; ++i) expected += double(x[i]) * double(y[i]);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: general fdp expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Cancellation: [100, -100].[1, 1] ≈ 0
	{
		std::vector<Scalar> x = { Scalar(100.0), Scalar(-100.0) };
		std::vector<Scalar> y = { Scalar(1.0),   Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = double(x[0]) * double(y[0]) + double(x[1]) * double(y[1]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: cancellation fdp expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Single element
	{
		std::vector<Scalar> x = { Scalar(42.0) };
		std::vector<Scalar> y = { Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = double(x[0]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: single-element fdp expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdp1024: 1024-element lns FDP tests
// ============================================================================
int TestFdp1024() {
	int nrOfFailedTestCases = 0;

	using Scalar = lns<16, 8, uint8_t>;

	// Case 1: 1024 products of 1*1 = 1024
	{
		std::vector<Scalar> x(1024, Scalar(1.0));
		std::vector<Scalar> y(1024, Scalar(1.0));
		Scalar result = fdp(x, y);
		double expected = 1024.0;
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: fdp1024 case 1, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: cancellation — 512 pairs of (+1*1) and (-1*1) = 0
	{
		std::vector<Scalar> x(1024), y(1024, Scalar(1.0));
		for (int i = 0; i < 1024; i += 2) {
			x[i]     = Scalar(1.0);
			x[i + 1] = Scalar(-1.0);
		}
		Scalar result = fdp(x, y);
		if (std::abs(double(result)) > 1.0) {
			std::cerr << "FAIL: fdp1024 case 2 (cancel), got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: cancellation + small residual
	{
		std::vector<Scalar> x(1024), y(1024, Scalar(1.0));
		for (int i = 0; i < 1022; i += 2) {
			x[i]     = Scalar(16.0);
			x[i + 1] = Scalar(-16.0);
		}
		x[1022] = Scalar(2.0);
		x[1023] = Scalar(0.5);
		Scalar result = fdp(x, y);
		double expected = double(Scalar(2.0)) + double(Scalar(0.5));
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: fdp1024 case 3 (cancel+residual), expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: accumulate many small products
	// 1024 * (0.5 * 0.5) = 1024 * 0.25 = 256
	{
		Scalar half(0.5);
		std::vector<Scalar> x(1024, half);
		std::vector<Scalar> y(1024, half);
		Scalar result = fdp(x, y);
		double expected = 1024.0 * double(half) * double(half);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: fdp1024 case 4, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: reproducibility (reversed order gives same result)
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1024; ++i) {
			double xv = std::pow(2.0, double(i % 8) * 0.25);  // exact powers of 2
			double yv = std::pow(2.0, double((i + 3) % 6) * 0.25);
			x[i] = Scalar(xv);
			y[i] = Scalar(yv);
		}
		Scalar result1 = fdp(x, y);
		std::vector<Scalar> xr(x.rbegin(), x.rend());
		std::vector<Scalar> yr(y.rbegin(), y.rend());
		Scalar result2 = fdp(xr, yr);
		if (double(result1) != double(result2)) {
			std::cerr << "FAIL: fdp1024 case 5 (reproducibility), "
			          << double(result1) << " != " << double(result2) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 6: large dynamic range — mix of very small and very large products
	{
		std::vector<Scalar> x(1024), y(1024);
		double expected = 0.0;
		for (int i = 0; i < 1024; ++i) {
			// Alternate between large (2^10) and small (2^-10) products
			if (i % 2 == 0) {
				x[i] = Scalar(1024.0);   // 2^10
				y[i] = Scalar(1.0);
			} else {
				x[i] = Scalar(0.0009765625);  // 2^-10
				y[i] = Scalar(1.0);
			}
			expected += double(x[i]) * double(y[i]);
		}
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: fdp1024 case 6 (dynamic range), expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestLimbBoundaryCarryBorrow: accumulations that force carry/borrow across
// quire limb boundaries
// ============================================================================
int TestLimbBoundaryCarryBorrow() {
	int nrOfFailedTestCases = 0;

	using Scalar = lns<16, 8, uint8_t>;
	// quire<lns<16,8>>: radix_point=128, limb boundary every 32 bits
	// Products of value V sit at quire bit (128 + floor(log2(|V|)))

	// Case 1: Carry across limb boundary at bit 128 (radix point)
	// Accumulate 1.0 * 1.0 products: each at quire bit 128.
	// 33 such products → sum = 33, MSB at bit 133 (crosses into next region)
	{
		quire<Scalar> q;
		Scalar one(1.0);
		for (int i = 0; i < 33; ++i) {
			q += quire_mul(one, one);
		}
		double result = q.convert_to<double>();
		if (std::abs(result - 33.0) > 0.5) {
			std::cerr << "FAIL: limb carry case 1, expected 33, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: Borrow across limb boundary
	// Start with large positive, subtract past zero
	{
		quire<Scalar> q;
		Scalar big(64.0), one(1.0);
		for (int i = 0; i < 100; ++i) q += quire_mul(big, one);   // +6400
		Scalar neg(-64.0);
		for (int i = 0; i < 200; ++i) q += quire_mul(neg, one);   // -12800
		double result = q.convert_to<double>();
		double expected = 100.0 * double(big) - 200.0 * double(big);
		if (std::abs(result - expected) > 2.0) {
			std::cerr << "FAIL: limb borrow case 2, expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: Exact cancellation to zero after crossing limb boundary
	{
		quire<Scalar> q;
		Scalar val(256.0), one(1.0);
		for (int i = 0; i < 100; ++i) q += quire_mul(val, one);
		Scalar neg(-256.0);
		for (int i = 0; i < 100; ++i) q += quire_mul(neg, one);
		if (!q.iszero()) {
			double result = q.convert_to<double>();
			std::cerr << "FAIL: limb cancel case 3, expected 0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: Small residual after large cancellation
	{
		quire<Scalar> q;
		Scalar big(1024.0), one(1.0), tiny(0.25);
		for (int i = 0; i < 50; ++i) q += quire_mul(big, one);   // +51200
		Scalar neg(-1024.0);
		for (int i = 0; i < 50; ++i) q += quire_mul(neg, one);   // -51200 → 0
		q += quire_mul(tiny, one);  // +0.25
		double result = q.convert_to<double>();
		if (std::abs(result - double(tiny)) > 0.1) {
			std::cerr << "FAIL: limb residual case 4, expected " << double(tiny)
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: Products spanning deep fractional region
	// 0.0625 * 0.0625 = 0.00390625 = 2^-8, scale = -8, quire bit 120
	// 256 such products → 256 * 2^-8 = 1.0
	{
		quire<Scalar> q;
		Scalar small(0.0625);  // 2^-4
		for (int i = 0; i < 256; ++i) {
			q += quire_mul(small, small);  // each = 2^-8
		}
		double result = q.convert_to<double>();
		double expected = 256.0 * double(small) * double(small);
		if (std::abs(result - expected) > 0.1) {
			std::cerr << "FAIL: fractional case 5, expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestCatastrophicCancellation: tests where naive accumulation would fail
// ============================================================================
int TestCatastrophicCancellation() {
	int nrOfFailedTestCases = 0;

	using Scalar = lns<16, 8, uint8_t>;

	// Case 1: Large values cancel, small residual survives
	{
		std::vector<Scalar> x(1024), y(1024, Scalar(1.0));
		for (int i = 0; i < 511; ++i) {
			x[2*i]     = Scalar(64.0);
			x[2*i + 1] = Scalar(-64.0);
		}
		x[1022] = Scalar(0.5);
		x[1023] = Scalar(0.25);
		Scalar result = fdp(x, y);
		double expected = double(Scalar(0.5)) + double(Scalar(0.25));
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: catastrophic case 1, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: Near-equal subtraction
	// x = [A, A, A, ...], y = [1, -1+eps, 1, -1+eps, ...]
	// Each pair contributes A*eps ≈ small value
	{
		Scalar a(32.0);
		Scalar one(1.0);
		// Find the closest lns value to -1 that isn't exactly -1
		Scalar neg_one(-1.0);
		// Use the next representable value
		Scalar almost_neg_one(-0.9375);  // -15/16 in lns approximation
		std::vector<Scalar> x(1024, a);
		std::vector<Scalar> y(1024);
		double expected = 0.0;
		for (int i = 0; i < 1024; ++i) {
			y[i] = (i % 2 == 0) ? one : almost_neg_one;
			expected += double(x[i]) * double(y[i]);
		}
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - expected) > std::abs(expected) * 0.1 + 2.0) {
			std::cerr << "FAIL: catastrophic case 2, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: Kahan-style: +big, +small, -big patterns
	{
		std::vector<Scalar> x(1024), y(1024, Scalar(1.0));
		double expected = 0.0;
		int i = 0;
		for (; i + 2 < 1024; i += 3) {
			x[i]     = Scalar(256.0);
			x[i + 1] = Scalar(0.125);
			x[i + 2] = Scalar(-256.0);
			expected += double(x[i]) + double(x[i + 1]) + double(x[i + 2]);
		}
		for (; i < 1024; ++i) {
			expected += double(x[i]);
		}
		Scalar result = fdp(x, y);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: catastrophic case 3 (Kahan), expected " << expected
			          << ", got " << double(result) << '\n';
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

	using Scalar = lns<16, 8, uint8_t>;

	// stride-2, n=2: x[0]*y[0] + x[2]*y[2]
	{
		std::vector<Scalar> x = { Scalar(2.0), Scalar(100.0), Scalar(4.0), Scalar(100.0) };
		std::vector<Scalar> y = { Scalar(3.0), Scalar(100.0), Scalar(5.0), Scalar(100.0) };
		Scalar result = fdp_stride(size_t(2), x, size_t(2), y, size_t(2));
		double expected = double(x[0]) * double(y[0]) + double(x[2]) * double(y[2]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: fdp_stride expected " << expected
			          << ", got " << double(result) << '\n';
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

	using Scalar = lns<16, 8, uint8_t>;

	// Basic continuation
	{
		quire<Scalar> q;
		std::vector<Scalar> x1 = { Scalar(2.0), Scalar(4.0) };
		std::vector<Scalar> y1 = { Scalar(1.0), Scalar(1.0) };
		fdp_qc(q, size_t(2), x1, size_t(1), y1, size_t(1));
		double expected1 = double(x1[0]) + double(x1[1]);

		std::vector<Scalar> x2 = { Scalar(8.0) };
		std::vector<Scalar> y2 = { Scalar(1.0) };
		fdp_qc(q, size_t(1), x2, size_t(1), y2, size_t(1));
		double expected2 = expected1 + double(x2[0]);

		double result = q.convert_to<double>();
		if (std::abs(result - expected2) > 1.0) {
			std::cerr << "FAIL: fdp_qc continuation, expected " << expected2
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Cancel to zero
	{
		quire<Scalar> q;
		std::vector<Scalar> x1(100, Scalar(1.0));
		std::vector<Scalar> y1(100, Scalar(1.0));
		fdp_qc(q, size_t(100), x1, size_t(1), y1, size_t(1));
		std::vector<Scalar> x2(100, Scalar(-1.0));
		std::vector<Scalar> y2(100, Scalar(1.0));
		fdp_qc(q, size_t(100), x2, size_t(1), y2, size_t(1));
		if (!q.iszero()) {
			double result = q.convert_to<double>();
			std::cerr << "FAIL: fdp_qc cancel to zero, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestDifferentConfigs: various lns configurations
// ============================================================================
int TestDifferentConfigs() {
	int nrOfFailedTestCases = 0;

	// lns<8, 4> — tiny lns (3 integer exponent bits, 4 fractional)
	{
		using Scalar = lns<8, 4, uint8_t>;
		std::vector<Scalar> x = { Scalar(1.0), Scalar(2.0) };
		std::vector<Scalar> y = { Scalar(1.0), Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = double(x[0]) + double(x[1]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: lns<8,4> fdp, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// lns<12, 6> — medium lns
	{
		using Scalar = lns<12, 6, uint8_t>;
		std::vector<Scalar> x = { Scalar(4.0), Scalar(8.0), Scalar(2.0) };
		std::vector<Scalar> y = { Scalar(1.0), Scalar(1.0), Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = double(x[0]) + double(x[1]) + double(x[2]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: lns<12,6> fdp, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// lns<32, 16> — wider lns
	{
		using Scalar = lns<32, 16, uint32_t>;
		std::vector<Scalar> x = { Scalar(100.0), Scalar(200.0) };
		std::vector<Scalar> y = { Scalar(1.0),   Scalar(2.0) };
		Scalar result = fdp(x, y);
		double expected = double(x[0]) * double(y[0]) + double(x[1]) * double(y[1]);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: lns<32,16> fdp, expected " << expected
			          << ", got " << double(result) << '\n';
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

	using Scalar = lns<16, 8, uint8_t>;

	constexpr size_t N = 100;
	std::vector<Scalar> x(N), y(N);
	double reference = 0.0;
	for (size_t i = 0; i < N; ++i) {
		double xv = std::pow(2.0, double(i % 8) * 0.5);
		double yv = std::pow(2.0, double((N - i) % 6) * 0.5);
		x[i] = Scalar(xv);
		y[i] = Scalar(yv);
		reference += double(x[i]) * double(y[i]);
	}

	Scalar fdp_result = fdp(x, y);
	double fdp_d = double(fdp_result);
	Scalar expected(reference);
	double expected_d = double(expected);

	std::cout << "  reference (double):      " << reference << '\n';
	std::cout << "  expected lns:            " << expected_d << '\n';
	std::cout << "  fdp result:              " << fdp_d << '\n';

	// lns has limited precision, so use generous tolerance
	if (std::abs(fdp_d - expected_d) / (std::abs(expected_d) + 1.0) > 0.1) {
		std::cerr << "FAIL: FDP accuracy test, fdp=" << fdp_d << " expected=" << expected_d << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "lns fused dot product (FDP)";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';
	std::cout << std::string(60, '=') << '\n';

	nrOfFailedTestCases += ReportTestResult(TestQuireMul(), "lns quire_mul", "unrounded product");
	nrOfFailedTestCases += ReportTestResult(TestBasicFdp(), "lns fdp", "basic dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdp1024(), "lns fdp", "1024-element vectors");
	nrOfFailedTestCases += ReportTestResult(TestLimbBoundaryCarryBorrow(), "lns quire", "limb-boundary carry/borrow");
	nrOfFailedTestCases += ReportTestResult(TestCatastrophicCancellation(), "lns fdp", "catastrophic cancellation");
	nrOfFailedTestCases += ReportTestResult(TestFdpStride(), "lns fdp_stride", "strided dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdpQc(), "lns fdp_qc", "quire continuation");
	nrOfFailedTestCases += ReportTestResult(TestDifferentConfigs(), "lns fdp", "different configs");
	nrOfFailedTestCases += ReportTestResult(TestAccuracy(), "lns fdp", "accuracy");

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
