// cfloat_fdp.cpp: comprehensive tests for cfloat fused dot product via generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Test structure:
//   quire<cfloat<32,8>> with uint32_t limbs:
//     qbits=592, radix_point=281 (in limb 8, bit 25 of that limb)
//     limb boundaries at accumulator positions 0,32,64,...,576
//     MUL blocktriple: bfbits=48, radix=46
//     accu_base_offset = 281 + scale - 46 = 235 + scale
//     So a product with scale S starts at accumulator bit (235+S)
//     Limb crossings when (235+S+i) mod 32 == 0 for some significand bit i
//
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <vector>
#include <cmath>
#include <random>

namespace sw { namespace universal {

// ============================================================================
// Key quire architectural constants for cfloat<32,8,uint32_t>
// ============================================================================
//   radix_point   = 281
//   limb size     = 32 bits
//   limb of radix = 281/32 = limb 8 (bits 256..287), radix at bit 25 in limb
//   qbits         = 592 (19 limbs: 0..18)
//   MUL bfbits    = 48, MUL radix = 46
//   base_offset   = 281 + scale - 46 = 235 + scale
//
// To target limb boundary N*32, we need: 235 + scale + bit_in_sig = N*32
// For the hidden-bit (bit 47 in 48-bit MUL significand):
//   235 + scale + 47 = N*32  →  scale = N*32 - 282
// Limb  8: 256 → scale = -26   (radix limb lower boundary)
// Limb  9: 288 → scale =   6   (radix limb upper boundary)
// Limb 10: 320 → scale =  38
// Limb  7: 224 → scale = -58
// Limb  0:   0 → scale = -282  (below quire range, would be clipped)

// Helper: compute the exact double-precision dot product of two cfloat vectors
template<typename Scalar>
double exact_dot(const std::vector<Scalar>& x, const std::vector<Scalar>& y) {
	double sum = 0.0;
	for (size_t i = 0; i < x.size() && i < y.size(); ++i) {
		sum += double(float(x[i])) * double(float(y[i]));
	}
	return sum;
}

// Helper: compute naive fp32 dot product
template<typename Scalar>
float naive_dot(const std::vector<Scalar>& x, const std::vector<Scalar>& y) {
	float sum = 0.0f;
	for (size_t i = 0; i < x.size() && i < y.size(); ++i) {
		sum += float(x[i]) * float(y[i]);
	}
	return sum;
}

// ============================================================================
// TestQuireMul: verify unrounded full-precision products
// Focus on: unrounded proof, overflow form, limb-boundary placement
// ============================================================================
int TestQuireMul() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	using QT = quire_traits<Scalar>;

	// --- Unrounded product proof ---
	// 1.1f * 1.3f: exact product is not representable in fp32.
	// If quire_mul rounded, the quire would get the fp32-rounded product.
	// We verify the quire gets the exact double-precision product instead.
	{
		float         fa(1.1f), fb(1.3f);
		double        da(fa), db(fb);
		Scalar        a(fa), b(fb);
		
		auto          fc = fa * fb;  // fp32 product (rounded)
		std::cout << to_binary(fc) << " : " << std::setprecision(std::numeric_limits<float>::max_digits10) << fc << '\n';
		auto          dc = da * db;  // double product (exact)
		std::cout << to_binary(dc) << " : " << std::setprecision(std::numeric_limits<double>::max_digits10) << dc << '\n';

		quire<Scalar> q;
		q += quire_mul(a, b);

		double result = q.convert_to<double>();
		std::cout << to_binary(result) << " : " << std::setprecision(std::numeric_limits<double>::max_digits10) << result << '\n';
		if (result != dc) {
			std::cerr << "FAIL: quire_mul(1.1, 1.3) unrounded proof: got "
			          << result << ", expected " << dc << '\n';
			++nrOfFailedTestCases;
		}
	}

	// 1.0000001f * 1.0000002f: near-1.0 products that stress low-bit precision
	{
		Scalar a(1.0000001f), b(1.0000002f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(float(a)) * double(float(b));
		if (result != expected) {
			std::cerr << "FAIL: quire_mul near-unity precision: got "
			          << result << ", expected " << expected << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- Overflow-form products (significand >= 2.0) ---
	// These exercise the radix-based bit placement: MSB is above MUL radix

	// 1.5 * 1.5 = 2.25 (MSB at radix+1)
	{
		Scalar a(1.5f), b(1.5f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 2.25) {
			std::cerr << "FAIL: quire_mul(1.5, 1.5) overflow form: " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// 3 * 7 = 21 (overflow form, larger product)
	{
		Scalar a(3.0f), b(7.0f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 21.0) {
			std::cerr << "FAIL: quire_mul(3, 7) overflow form: " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- Products placed at limb boundaries ---
	// scale=-26 → MSB at accumulator bit 256 (limb 8/9 boundary)
	{
		// 2^(-26) has scale -26 in fp32
		float val_a = std::ldexp(1.0f, -13);  // 2^-13
		float val_b = std::ldexp(1.0f, -13);  // product scale = -26
		Scalar a(val_a), b(val_b);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(val_a) * double(val_b);
		if (result != expected) {
			std::cerr << "FAIL: quire_mul at limb 8/9 boundary: " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// scale=6 → MSB at accumulator bit 288 (limb 9/10 boundary)
	{
		float val_a = std::ldexp(1.0f, 3);  // 2^3
		float val_b = std::ldexp(1.0f, 3);  // product scale = 6
		Scalar a(val_a), b(val_b);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 64.0) {
			std::cerr << "FAIL: quire_mul at limb 9/10 boundary: " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// scale=38 → MSB at accumulator bit 320 (limb 10/11 boundary)
	{
		float val_a = std::ldexp(1.0f, 19);  // 2^19
		float val_b = std::ldexp(1.0f, 19);  // product scale = 38
		Scalar a(val_a), b(val_b);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(val_a) * double(val_b);
		if (result != expected) {
			std::cerr << "FAIL: quire_mul at limb 10/11 boundary: " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- Negative products ---
	{
		Scalar a(-2.0f), b(7.0f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != -14.0) {
			std::cerr << "FAIL: quire_mul(-2, 7): " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	{
		Scalar a(-4.0f), b(-8.0f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 32.0) {
			std::cerr << "FAIL: quire_mul(-4, -8): " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	{
		Scalar a(-1000.0f), b(0.001f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(float(a)) * double(float(b));
		if (result != expected) {
			std::cerr << "FAIL: quire_mul(-1000, 0.001): " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// --- Special values ---
	{
		Scalar zero(0.0f), val(42.0f);
		auto product = quire_mul(zero, val);
		if (!product.iszero()) {
			std::cerr << "FAIL: quire_mul(0, 42) should be zero\n";
			++nrOfFailedTestCases;
		}
	}

	{
		Scalar a, b(1.0f);
		a.setnan();
		auto product = quire_mul(a, b);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(NaN, 1) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	{
		Scalar a, b(1.0f);
		a.setinf();
		auto product = quire_mul(a, b);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(inf, 1) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	// Verify MUL blocktriple dimensioning is correct
	{
		constexpr unsigned fbits = 23;
		constexpr unsigned expected_bfbits = 2 + 2 * fbits;  // 48
		constexpr unsigned actual_bfbits = blocktriple<fbits, BlockTripleOperator::MUL, uint32_t>::bfbits;
		static_assert(actual_bfbits == expected_bfbits, "MUL bfbits mismatch");
		(void)QT{};  // suppress unused warning
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
		auto product = quire_mul(Scalar(0.0f), Scalar(123.0f));
		if (!product.iszero()) { std::cerr << "FAIL: +0*123\n"; ++nrOfFailedTestCases; }
	}
	// -0 * value = 0
	{
		Scalar nzero;
		nzero.setzero();
		nzero.setsign();
		auto product = quire_mul(nzero, Scalar(123.0f));
		if (!product.iszero()) { std::cerr << "FAIL: -0*123\n"; ++nrOfFailedTestCases; }
	}
	// +inf * 1 = NaN (quire cannot represent infinity)
	{
		Scalar pinf; pinf.setinf(false);
		auto product = quire_mul(pinf, Scalar(1.0f));
		if (!product.isnan()) { std::cerr << "FAIL: +inf*1\n"; ++nrOfFailedTestCases; }
	}
	// -inf * 1 = NaN
	{
		Scalar ninf; ninf.setinf(true);
		auto product = quire_mul(ninf, Scalar(1.0f));
		if (!product.isnan()) { std::cerr << "FAIL: -inf*1\n"; ++nrOfFailedTestCases; }
	}
	// inf * inf = NaN
	{
		Scalar inf1, inf2;
		inf1.setinf(false); inf2.setinf(false);
		auto product = quire_mul(inf1, inf2);
		if (!product.isnan()) { std::cerr << "FAIL: inf*inf\n"; ++nrOfFailedTestCases; }
	}
	// val * NaN = NaN
	{
		Scalar nan_val; nan_val.setnan();
		auto product = quire_mul(Scalar(42.0f), nan_val);
		if (!product.isnan()) { std::cerr << "FAIL: 42*NaN\n"; ++nrOfFailedTestCases; }
	}

	// blocktriple MUL accumulation (via quire_mul)
	{
		quire<Scalar> q;
		q += quire_mul(Scalar(6.0f), Scalar(7.0f));
		double result = q.convert_to<double>();
		if (result != 42.0) { std::cerr << "FAIL: MUL bt accumulation\n"; ++nrOfFailedTestCases; }
	}
	// integer assignment
	{
		quire<Scalar> q;
		q = int64_t(42);
		double result = q.convert_to<double>();
		if (result != 42.0) { std::cerr << "FAIL: int64 assignment\n"; ++nrOfFailedTestCases; }
	}
	// integer assignment negative
	{
		quire<Scalar> q;
		q = int64_t(-123);
		double result = q.convert_to<double>();
		if (result != -123.0) { std::cerr << "FAIL: int64 negative\n"; ++nrOfFailedTestCases; }
	}
	// integer assignment (reliable path)
	{
		quire<Scalar> q;
		q = int64_t(256);
		double result = q.convert_to<double>();
		if (result != 256.0) { std::cerr << "FAIL: int64 256\n"; ++nrOfFailedTestCases; }
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestLimbBoundaryCarryBorrow: 10+ cases targeting limb-boundary carry/borrow
//
// For cfloat<32,8> with uint32_t limbs, the limb boundaries in the accumulator
// are at positions 32, 64, 96, ..., 576. We construct products whose
// accumulation causes carry or borrow to propagate across these boundaries.
// ============================================================================
int TestLimbBoundaryCarryBorrow() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	// limb boundaries: accumulator bit positions 0, 32, 64, ..., 576
	// radix_point = 281 (bit 25 of limb 8)
	// base_offset = 235 + scale (for MUL bt with radix=46)
	// A product at scale S occupies accumulator bits [235+S .. 235+S+47]

	// Case 1: carry propagation across limb 8→9 boundary (bit 256)
	// Two products at scale=-26 that sum to carry across bit 256
	{
		// products at scale -26: base = 235 + (-26) = 209, spans bits [209..256]
		// Two of these that sum correctly
		float v = std::ldexp(1.0f, -13);  // 2^-13 * 2^-13 = 2^-26
		quire<Scalar> q;
		// accumulate enough products to trigger carry: 32 * 2^-26 = 2^-21
		for (int i = 0; i < 32; ++i) {
			q += quire_mul(Scalar(v), Scalar(v));
		}
		double expected = 32.0 * double(v) * double(v);
		double result = q.convert_to<double>();
		if (result != expected) {
			std::cerr << "FAIL: limb 8/9 carry, expected " << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: carry propagation across limb 9→10 boundary (bit 288)
	{
		// products at scale 6: base = 241, spans [241..288]
		// MSB at exactly bit 288 (limb boundary)
		quire<Scalar> q;
		Scalar a(8.0f), b(8.0f);  // product = 64, scale = 6
		for (int i = 0; i < 64; ++i) {
			q += quire_mul(a, b);
		}
		// 64 * 64 = 4096
		double result = q.convert_to<double>();
		if (result != 4096.0) {
			std::cerr << "FAIL: limb 9/10 carry, expected 4096, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: carry chain through 3 limbs (10→11→12)
	{
		// products at scale 38: base = 273, spans [273..320]
		// MSB at bit 320 (limb 10/11 boundary)
		quire<Scalar> q;
		// accumulate 2^32 = 4294967296 products to force carry through 3 limbs
		// Instead: accumulate products with carefully chosen magnitudes
		// 2^38 * (2^32) = 2^70, but that's too many iterations
		// Use larger products to trigger carry through multiple limbs
		Scalar big(std::ldexp(1.0f, 19));
		for (int i = 0; i < 16; ++i) {
			q += quire_mul(big, big);  // 16 * 2^38 = 2^42
		}
		double result = q.convert_to<double>();
		double expected = 16.0 * double(big) * double(big);
		if (std::abs(result - expected) > 1.0) {
			std::cerr << "FAIL: 3-limb carry, expected " << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: borrow across limb 9→8 boundary
	{
		quire<Scalar> q;
		// Add large value at scale 6 (spans limb 9)
		q += quire_mul(Scalar(8.0f), Scalar(8.0f));  // 64, scale 6
		// Subtract a slightly smaller value: causes borrow across limb boundary
		q -= quire_mul(Scalar(8.0f), Scalar(7.0f));  // 56, scale 5
		double result = q.convert_to<double>();
		if (result != 8.0) {
			std::cerr << "FAIL: limb 9/8 borrow, expected 8, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: borrow that propagates across the radix point (limb 8)
	{
		quire<Scalar> q;
		q += quire_mul(Scalar(1.0f), Scalar(1.0f));  // 1.0 at radix
		q -= quire_mul(Scalar(0.5f), Scalar(1.0f));  // 0.5 below radix
		double result = q.convert_to<double>();
		if (result != 0.5) {
			std::cerr << "FAIL: borrow across radix, expected 0.5, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 6: carry at limb 7→8 boundary (below radix, bit 224)
	{
		// products at scale -58: base = 177, spans [177..224]
		float v = std::ldexp(1.0f, -29);  // 2^-29 * 2^-29 = 2^-58
		quire<Scalar> q;
		for (int i = 0; i < 64; ++i) {
			q += quire_mul(Scalar(v), Scalar(v));
		}
		double expected = 64.0 * double(v) * double(v);
		double result = q.convert_to<double>();
		if (result != expected) {
			std::cerr << "FAIL: limb 7/8 carry, expected " << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 7: carry at high limb (13→14 boundary, bit 416)
	{
		// scale = 416 - 282 = 134. Need product scale 134: use 2^67 * 2^67
		// But 2^67 overflows fp32. Use 2^63 * 2^63 = 2^126, scale 126
		// bit position = 235 + 126 + 47 = 408, in limb 12
		// Use 2^60 * 2^67 = 2^127 (fp32 max exponent)
		Scalar big(std::ldexp(1.0f, 63));
		quire<Scalar> q;
		q += quire_mul(big, big);  // 2^126
		q += quire_mul(big, big);  // + 2^126 = 2^127
		double result = q.convert_to<double>();
		double expected = 2.0 * std::ldexp(1.0, 126);
		if (result != expected) {
			std::cerr << "FAIL: high-limb carry, expected " << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 8: borrow cascade - subtract produces borrow through 2 limbs
	{
		quire<Scalar> q;
		// Start with a value that spans multiple limbs
		q = int64_t(1LL << 33);  // 2^33, sits at radix+33 = bit 314, in limb 9
		// Subtract 1: requires borrow from bit 314 all the way down to bit 281
		q -= quire_mul(Scalar(1.0f), Scalar(1.0f));
		double result = q.convert_to<double>();
		double expected = double(1LL << 33) - 1.0;
		if (result != expected) {
			std::cerr << "FAIL: 2-limb borrow cascade, expected " << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 9: products at 2 different limbs that carry into a third
	{
		quire<Scalar> q;
		// product at scale 0 (limb ~8): 1.0*1.0 = 1.0
		// product at scale 32 (limb ~10): large value
		// Sum should be exact
		float large = std::ldexp(1.0f, 16);
		q += quire_mul(Scalar(1.0f), Scalar(1.0f));       // scale 0
		q += quire_mul(Scalar(large), Scalar(large));      // scale 32
		double result = q.convert_to<double>();
		double expected = 1.0 + double(large) * double(large);
		if (result != expected) {
			std::cerr << "FAIL: multi-limb product merge, expected " << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 10: accumulate exactly 2^32 (needs carry across full 32-bit limb)
	{
		quire<Scalar> q;
		// 2^16 * 2^16 = 2^32 directly
		Scalar v16(std::ldexp(1.0f, 16));
		q += quire_mul(v16, v16);
		double result = q.convert_to<double>();
		double expected = std::ldexp(1.0, 32);
		if (result != expected) {
			std::cerr << "FAIL: exact 2^32, expected " << expected << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 11: accumulate to fill exactly one limb, then carry
	{
		quire<Scalar> q;
		// 32 products of 2^5 * 2^5 = 2^10 each, sum = 32 * 2^10 = 2^15
		// Then 32 more: total 64 * 2^10 = 2^16
		// Then 32 more: total 96 * 2^10... keep going until carry
		// 2^32/2^10 = 2^22 products needed to fill. That's 4M, too many.
		// Instead: products of 2^21 * 2^21 = 2^42, 2 of them = 2^43
		Scalar v21(std::ldexp(1.0f, 21));
		q += quire_mul(v21, v21);  // 2^42
		q += quire_mul(v21, v21);  // + 2^42 = 2^43
		double result = q.convert_to<double>();
		if (result != std::ldexp(1.0, 43)) {
			std::cerr << "FAIL: fill-limb-then-carry, expected 2^43, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 12: borrow to zero from multi-limb value
	{
		quire<Scalar> q;
		Scalar v(std::ldexp(1.0f, 20));
		q += quire_mul(v, v);  // 2^40
		q -= quire_mul(v, v);  // - 2^40 = 0
		if (!q.iszero()) {
			std::cerr << "FAIL: borrow to zero from multi-limb\n";
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdp1024: FDP with 1024-element vectors targeting catastrophic cancellation
//
// The core value of FDP: where naive fp32 accumulation fails due to
// catastrophic cancellation, the quire preserves all bits and delivers
// the correctly-rounded result.
// ============================================================================
int TestFdp1024() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// ------------------------------------------------------------------
	// Case 1: Classic ill-conditioned vector
	// 1023 pairs of (+big, -big) that cancel, plus one small residual
	// Naive fp32: the small residual is lost in cancellation
	// FDP: quire preserves it exactly
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		// 511 pairs: x[2i]*y[2i] = +1e8, x[2i+1]*y[2i+1] = -1e8
		for (int i = 0; i < 511; ++i) {
			x[2*i]     = Scalar(1e8f);   y[2*i]     = Scalar(1.0f);
			x[2*i + 1] = Scalar(-1e8f);  y[2*i + 1] = Scalar(1.0f);
		}
		// remaining: 1024 - 1022 = 2 elements as small residual
		x[1022] = Scalar(0.5f);   y[1022] = Scalar(1.0f);
		x[1023] = Scalar(0.25f);  y[1023] = Scalar(1.0f);
		// exact dot = 0 + 0.5 + 0.25 = 0.75

		Scalar result = fdp(x, y);
		if (double(result) != 0.75) {
			std::cerr << "FAIL: fdp1024 case 1 (cancellation+residual), expected 0.75, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}

		// Verify naive fp32 gets it wrong or at least less accurate
		float naive = naive_dot(x, y);
		std::cout << "  Case 1 (cancel+residual): fdp=" << double(result) << " naive=" << naive << '\n';
	}

	// ------------------------------------------------------------------
	// Case 2: 1023 small values contributing the LSB to the 1024th large value
	// The 1023 small values each contribute 1/1024, totaling just under 1.0.
	// This 1.0 is the LSB of the large 1024th product (1024.0 * 1.0 = 1024).
	// Naive fp32: 1024 + (small sum) = 1024 (LSB lost due to rounding)
	// FDP: quire accumulates all, result = 1024 + 1023/1024 ≈ 1024.999...
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		// Large product
		x[0] = Scalar(1024.0f);  y[0] = Scalar(1.0f);
		// 1023 small products: each = 1/1024
		float small_val = 1.0f / 1024.0f;
		for (int i = 1; i < 1024; ++i) {
			x[i] = Scalar(small_val);  y[i] = Scalar(1.0f);
		}
		// exact = 1024 + 1023*(cfloat-rounded 1/1024)
		double exact = 1024.0 + 1023.0 * double(float(Scalar(small_val)));

		Scalar result = fdp(x, y);
		double d = double(result);
		// The cfloat result should be the correctly-rounded fp32 of the exact sum
		Scalar expected(static_cast<float>(exact));
		if (d != double(expected)) {
			std::cerr << "FAIL: fdp1024 case 2 (LSB contribution), expected "
			          << double(expected) << ", got " << d << '\n';
			++nrOfFailedTestCases;
		}
		float naive = naive_dot(x, y);
		std::cout << "  Case 2 (LSB contribution): fdp=" << d << " naive=" << naive
		          << " exact=" << exact << '\n';
	}

	// ------------------------------------------------------------------
	// Case 3: Telescoping cancellation across magnitudes
	// Pairs at decreasing magnitudes: 1e30, -1e30, 1e20, -1e20, ..., 1, -1, residual
	// Each pair cancels exactly, leaving only the residual.
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x, y;
		// Create cancelling pairs at different magnitudes
		float magnitudes[] = { 1e30f, 1e25f, 1e20f, 1e15f, 1e10f, 1e5f, 1e3f, 1e1f };
		for (float mag : magnitudes) {
			x.push_back(Scalar(mag));   y.push_back(Scalar(1.0f));
			x.push_back(Scalar(-mag));  y.push_back(Scalar(1.0f));
		}
		// pad to 1024 with ones
		while (x.size() < 1023) {
			x.push_back(Scalar(0.0f));
			y.push_back(Scalar(0.0f));
		}
		// final residual
		x.push_back(Scalar(3.14f));  y.push_back(Scalar(1.0f));

		Scalar result = fdp(x, y);
		double expected = double(float(Scalar(3.14f)));
		if (std::abs(double(result) - expected) > 1e-5) {
			std::cerr << "FAIL: fdp1024 case 3 (telescoping), expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// ------------------------------------------------------------------
	// Case 4: Products at many different scales
	// Each a[i] = 2^(i-512), b[i] = 2^(512-i), so product = 2^0 = 1.0 always
	// Sum of 1024 ones = 1024.0
	// Naive might have issues with varying magnitudes of intermediates
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1024; ++i) {
			// clamp exponents to fp32 range [-126..127]
			int exp_x = std::max(-60, std::min(60, i - 512));
			int exp_y = -exp_x;
			x[i] = Scalar(std::ldexp(1.0f, exp_x));
			y[i] = Scalar(std::ldexp(1.0f, exp_y));
		}
		Scalar result = fdp(x, y);
		if (double(result) != 1024.0) {
			std::cerr << "FAIL: fdp1024 case 4 (multi-scale), expected 1024, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// ------------------------------------------------------------------
	// Case 5: Near-cancellation with tiny residual
	// 512 products of +M and 512 products of -M, except last one is -(M-epsilon)
	// Residual = epsilon
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		float M = 1e6f;
		for (int i = 0; i < 512; ++i) {
			x[i] = Scalar(M);
			y[i] = Scalar(1.0f);
		}
		for (int i = 512; i < 1023; ++i) {
			x[i] = Scalar(-M);
			y[i] = Scalar(1.0f);
		}
		// last product: -(M - 1.0) instead of -M, residual = 1.0
		x[1023] = Scalar(-(M - 1.0f));
		y[1023] = Scalar(1.0f);
		// exact: 512*M - 511*M - (M-1) = M - M + 1 = 1.0

		Scalar result = fdp(x, y);
		if (double(result) != 1.0) {
			std::cerr << "FAIL: fdp1024 case 5 (near-cancel residual), expected 1.0, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// ------------------------------------------------------------------
	// Case 6: Carry propagation through accumulation of many small products
	// 1024 products of (1/32) * (1/32) = 1/1024 each, sum = 1.0
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		float v = 1.0f / 32.0f;  // exactly representable
		for (int i = 0; i < 1024; ++i) {
			x[i] = Scalar(v);
			y[i] = Scalar(v);
		}
		// 1024 * (1/32)^2 = 1024/1024 = 1.0
		Scalar result = fdp(x, y);
		if (double(result) != 1.0) {
			std::cerr << "FAIL: fdp1024 case 6 (small product carry), expected 1.0, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// ------------------------------------------------------------------
	// Case 7: Mixed positive/negative products with carry across limb 8→9
	// Products placed at scale ≈ -26 (limb 8/9 boundary)
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		float va = std::ldexp(1.0f, -13);  // 2^-13, product scale = -26
		for (int i = 0; i < 512; ++i) {
			x[2*i]     = Scalar(va);   y[2*i]     = Scalar(va);
			x[2*i + 1] = Scalar(-va);  y[2*i + 1] = Scalar(va);
		}
		// 512 * va^2 - 512 * va^2 = 0
		Scalar result = fdp(x, y);
		if (double(result) != 0.0) {
			std::cerr << "FAIL: fdp1024 case 7 (limb-boundary cancel), expected 0, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// ------------------------------------------------------------------
	// Case 8: Kahan worst-case: a[i] = 1, b[i] = 1 for i=0..1022, plus
	// a[1023] = -(1023 - epsilon). FDP should recover epsilon.
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1023; ++i) {
			x[i] = Scalar(1.0f);
			y[i] = Scalar(1.0f);
		}
		// exact sum of first 1023 = 1023.0
		// Last product should cancel most of it
		x[1023] = Scalar(-1022.0f);
		y[1023] = Scalar(1.0f);
		// exact result: 1023 - 1022 = 1.0

		Scalar result = fdp(x, y);
		if (double(result) != 1.0) {
			std::cerr << "FAIL: fdp1024 case 8 (Kahan), expected 1.0, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// ------------------------------------------------------------------
	// Case 9: Products spanning the full quire dynamic range
	// Mix very large and very small products, all positive
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1024; ++i) {
			// Alternate between large and small scales
			if (i % 2 == 0) {
				x[i] = Scalar(std::ldexp(1.0f, 50));
				y[i] = Scalar(std::ldexp(1.0f, -50));
				// product = 1.0
			} else {
				x[i] = Scalar(1.0f);
				y[i] = Scalar(std::ldexp(1.0f, -20));
				// product = 2^-20
			}
		}
		// 512 * 1.0 + 512 * 2^-20 = 512 + 512/1048576 ≈ 512.000488
		double expected = 512.0 + 512.0 * std::ldexp(1.0, -20);
		Scalar result = fdp(x, y);
		Scalar exp_cfloat(static_cast<float>(expected));
		if (double(result) != double(exp_cfloat)) {
			std::cerr << "FAIL: fdp1024 case 9 (full range), expected " << double(exp_cfloat)
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// ------------------------------------------------------------------
	// Case 10: Reproducibility test - same vectors, same result
	// (shuffled order shouldn't matter for FDP due to exact accumulation)
	// ------------------------------------------------------------------
	{
		std::vector<Scalar> x(1024), y(1024);
		std::mt19937 rng(42);  // fixed seed
		std::uniform_real_distribution<float> dist(-100.0f, 100.0f);
		for (int i = 0; i < 1024; ++i) {
			x[i] = Scalar(dist(rng));
			y[i] = Scalar(dist(rng));
		}
		Scalar result1 = fdp(x, y);

		// Reverse the vectors (different accumulation order)
		std::vector<Scalar> xr(x.rbegin(), x.rend());
		std::vector<Scalar> yr(y.rbegin(), y.rend());
		Scalar result2 = fdp(xr, yr);

		if (double(result1) != double(result2)) {
			std::cerr << "FAIL: fdp1024 case 10 (reproducibility), "
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

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// stride-2: x[0]*y[0] + x[2]*y[2] = 1*5 + 3*7 = 26
	{
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f), Scalar(4.0f) };
		std::vector<Scalar> y = { Scalar(5.0f), Scalar(6.0f), Scalar(7.0f), Scalar(8.0f) };
		Scalar result = fdp_stride(size_t(4), x, size_t(2), y, size_t(2));
		if (double(result) != 26.0) {
			std::cerr << "FAIL: fdp_stride(4,x,2,y,2) expected 26, got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdpQcStress: quire continuation with 10+ limb-aware stress configurations
// ============================================================================
int TestFdpQcStress() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// Case 1: continuation that causes carry across radix-limb boundary
	{
		quire<Scalar> q;
		// First batch: accumulate just below limb boundary
		std::vector<Scalar> x1(255, Scalar(1.0f));
		std::vector<Scalar> y1(255, Scalar(1.0f));
		fdp_qc(q, size_t(255), x1, size_t(1), y1, size_t(1));
		// q = 255, continuation adds 1 → 256 = 2^8 (potential carry)
		std::vector<Scalar> x2 = { Scalar(1.0f) };
		std::vector<Scalar> y2 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x2, size_t(1), y2, size_t(1));
		double result = q.convert_to<double>();
		if (result != 256.0) {
			std::cerr << "FAIL: qc stress 1 (carry at 256), got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: continuation with borrow that flips sign
	{
		quire<Scalar> q;
		std::vector<Scalar> x1 = { Scalar(5.0f) };
		std::vector<Scalar> y1 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));  // q = 5
		std::vector<Scalar> x2 = { Scalar(-10.0f) };
		std::vector<Scalar> y2 = { Scalar(1.0f) };
		fdp_qc(q, size_t(1), x2, size_t(1), y2, size_t(1));  // q = -5
		double result = q.convert_to<double>();
		if (result != -5.0) {
			std::cerr << "FAIL: qc stress 2 (sign flip), got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: many continuations accumulating at different limb positions
	{
		quire<Scalar> q;
		double expected = 0.0;
		for (int batch = 0; batch < 10; ++batch) {
			float scale_val = std::ldexp(1.0f, batch * 5);  // 2^0, 2^5, 2^10, ...
			std::vector<Scalar> x1 = { Scalar(scale_val) };
			std::vector<Scalar> y1 = { Scalar(scale_val) };
			fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
			expected += double(scale_val) * double(scale_val);
		}
		double result = q.convert_to<double>();
		if (result != expected) {
			std::cerr << "FAIL: qc stress 3 (multi-scale), expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 4: continuation that cancels to exact zero
	{
		quire<Scalar> q;
		std::vector<Scalar> x1(512, Scalar(1.0f));
		std::vector<Scalar> y1(512, Scalar(1.0f));
		fdp_qc(q, size_t(512), x1, size_t(1), y1, size_t(1));  // q = 512
		std::vector<Scalar> x2(512, Scalar(-1.0f));
		std::vector<Scalar> y2(512, Scalar(1.0f));
		fdp_qc(q, size_t(512), x2, size_t(1), y2, size_t(1));  // q = 0
		if (!q.iszero()) {
			std::cerr << "FAIL: qc stress 4 (cancel to zero), got " << q.convert_to<double>() << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 5: carry chain through continuation with large products
	{
		quire<Scalar> q;
		// 10 batches of 2^20 * 2^20 = 2^40
		for (int i = 0; i < 10; ++i) {
			Scalar big(std::ldexp(1.0f, 20));
			std::vector<Scalar> x1 = { big };
			std::vector<Scalar> y1 = { big };
			fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		}
		double result = q.convert_to<double>();
		double expected = 10.0 * std::ldexp(1.0, 40);
		if (result != expected) {
			std::cerr << "FAIL: qc stress 5 (large carry), expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 6: continuation with products below radix (fractional accumulation)
	{
		quire<Scalar> q;
		for (int i = 0; i < 8; ++i) {
			std::vector<Scalar> x1 = { Scalar(0.125f) };  // 2^-3
			std::vector<Scalar> y1 = { Scalar(0.125f) };  // product = 2^-6
			fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		}
		// 8 * 2^-6 = 2^-3 = 0.125
		double result = q.convert_to<double>();
		if (result != 0.125) {
			std::cerr << "FAIL: qc stress 6 (fractional), got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 7: alternating + and - continuations that nearly cancel
	{
		quire<Scalar> q;
		for (int i = 0; i < 100; ++i) {
			std::vector<Scalar> xp = { Scalar(1000.0f) };
			std::vector<Scalar> yp = { Scalar(1.0f) };
			fdp_qc(q, size_t(1), xp, size_t(1), yp, size_t(1));
			std::vector<Scalar> xn = { Scalar(-999.0f) };
			std::vector<Scalar> yn = { Scalar(1.0f) };
			fdp_qc(q, size_t(1), xn, size_t(1), yn, size_t(1));
		}
		// 100 * (1000 - 999) = 100
		double result = q.convert_to<double>();
		if (result != 100.0) {
			std::cerr << "FAIL: qc stress 7 (alternating), got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 8: borrow propagation across 3+ limbs via continuation
	{
		quire<Scalar> q;
		// Accumulate 2^50 first
		Scalar big(std::ldexp(1.0f, 25));
		std::vector<Scalar> x1 = { big };
		std::vector<Scalar> y1 = { big };
		fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));  // 2^50
		// Subtract 2^50 - 1.0 via continuation
		q -= quire_mul(big, big);  // back to 0
		q += quire_mul(Scalar(1.0f), Scalar(1.0f));  // + 1
		double result = q.convert_to<double>();
		if (result != 1.0) {
			std::cerr << "FAIL: qc stress 8 (3-limb borrow), got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 9: continuation with overflow-form products (significand >= 2.0)
	{
		quire<Scalar> q;
		// Products where both operands are in [1,2): product in [1,4)
		// These have overflow form (MSB above MUL radix)
		for (int i = 0; i < 10; ++i) {
			float va = 1.0f + float(i) * 0.1f;
			float vb = 1.0f + float(9 - i) * 0.1f;
			std::vector<Scalar> x1 = { Scalar(va) };
			std::vector<Scalar> y1 = { Scalar(vb) };
			fdp_qc(q, size_t(1), x1, size_t(1), y1, size_t(1));
		}
		// compute exact reference
		double expected = 0.0;
		for (int i = 0; i < 10; ++i) {
			float va = 1.0f + float(i) * 0.1f;
			float vb = 1.0f + float(9 - i) * 0.1f;
			expected += double(float(Scalar(va))) * double(float(Scalar(vb)));
		}
		double result = q.convert_to<double>();
		if (std::abs(result - expected) > 1e-6) {
			std::cerr << "FAIL: qc stress 9 (overflow form), expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 10: continuation accumulating 1024 products in 4 batches of 256
	{
		quire<Scalar> q;
		double expected = 0.0;
		for (int batch = 0; batch < 4; ++batch) {
			std::vector<Scalar> x1(256), y1(256);
			for (int i = 0; i < 256; ++i) {
				float v = float(batch * 256 + i + 1);
				x1[i] = Scalar(v);
				y1[i] = Scalar(1.0f / v);
				expected += double(float(x1[i])) * double(float(y1[i]));
			}
			fdp_qc(q, size_t(256), x1, size_t(1), y1, size_t(1));
		}
		// Each product ≈ 1.0 (with rounding from cfloat), sum ≈ 1024
		double result = q.convert_to<double>();
		// The quire result should match the exact double reference rounded to cfloat
		if (std::abs(result - expected) > 1.0) {
			std::cerr << "FAIL: qc stress 10 (4x256 batches), expected ~" << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestCatastrophicCancellation: 10+ stress cases proving FDP avoids catastrophic
// cancellation that destroys naive fp32 accumulation
// ============================================================================
int TestCatastrophicCancellation() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// Case 1: [1e7, 1, -1e7, 1] . [1,1,1,1] = 2.0
	{
		std::vector<Scalar> x = { Scalar(1e7f), Scalar(1.0f), Scalar(-1e7f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f),  Scalar(1.0f) };
		if (double(fdp(x, y)) != 2.0) {
			std::cerr << "FAIL: catastrophic 1 (1e7)\n"; ++nrOfFailedTestCases;
		}
	}

	// Case 2: scale 1e15
	{
		std::vector<Scalar> x = { Scalar(1e15f), Scalar(1.0f), Scalar(-1e15f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 2.0) {
			std::cerr << "FAIL: catastrophic 2 (1e15)\n"; ++nrOfFailedTestCases;
		}
	}

	// Case 3: scale 1e30 (near maxpos)
	{
		std::vector<Scalar> x = { Scalar(1e30f), Scalar(-1e30f), Scalar(42.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 42.0) {
			std::cerr << "FAIL: catastrophic 3 (1e30)\n"; ++nrOfFailedTestCases;
		}
	}

	// Case 4: many cancellation pairs with single residual
	{
		std::vector<Scalar> x(1025), y(1025);
		for (int i = 0; i < 512; ++i) {
			x[2*i]     = Scalar(1e6f);  y[2*i]     = Scalar(1.0f);
			x[2*i + 1] = Scalar(-1e6f); y[2*i + 1] = Scalar(1.0f);
		}
		x[1024] = Scalar(7.0f);  y[1024] = Scalar(1.0f);
		if (double(fdp(x, y)) != 7.0) {
			std::cerr << "FAIL: catastrophic 4 (512 pairs)\n"; ++nrOfFailedTestCases;
		}
	}

	// Case 5: cascading scales: 1e30-1e30 + 1e20-1e20 + ... + residual
	{
		std::vector<Scalar> x, y;
		float scales[] = { 1e30f, 1e25f, 1e20f, 1e15f, 1e10f, 1e5f };
		for (float s : scales) {
			x.push_back(Scalar(s));   y.push_back(Scalar(1.0f));
			x.push_back(Scalar(-s));  y.push_back(Scalar(1.0f));
		}
		x.push_back(Scalar(0.5f));  y.push_back(Scalar(1.0f));
		if (double(fdp(x, y)) != 0.5) {
			std::cerr << "FAIL: catastrophic 5 (cascading scales)\n"; ++nrOfFailedTestCases;
		}
	}

	// Case 6: 1024 alternating ±1e8 products, expected 0
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1024; ++i) {
			x[i] = Scalar(1e8f * ((i % 2 == 0) ? 1.0f : -1.0f));
			y[i] = Scalar(1.0f);
		}
		if (double(fdp(x, y)) != 0.0) {
			std::cerr << "FAIL: catastrophic 6 (alternating 1e8)\n"; ++nrOfFailedTestCases;
		}
	}

	// Case 7: mixed-sign products with wide scale variation
	{
		std::vector<Scalar> x = {
			Scalar(1e20f), Scalar(1e-10f), Scalar(-1e20f), Scalar(1e-10f)
		};
		std::vector<Scalar> y = {
			Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f)
		};
		double expected = 2.0 * double(float(Scalar(1e-10f)));
		double result = double(fdp(x, y));
		if (std::abs(result - expected) > 1e-15) {
			std::cerr << "FAIL: catastrophic 7 (mixed scale), expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 8: complete cancellation of 200 terms
	{
		std::vector<Scalar> x, y;
		for (int i = 1; i <= 100; ++i) {
			x.push_back(Scalar(float(i)));   y.push_back(Scalar(1.0f));
			x.push_back(Scalar(float(-i)));  y.push_back(Scalar(1.0f));
		}
		if (double(fdp(x, y)) != 0.0) {
			std::cerr << "FAIL: catastrophic 8 (200 terms)\n"; ++nrOfFailedTestCases;
		}
	}

	// Case 9: 200 terms cancellation + tiny residual
	{
		std::vector<Scalar> x, y;
		for (int i = 1; i <= 100; ++i) {
			x.push_back(Scalar(float(i)));   y.push_back(Scalar(1.0f));
			x.push_back(Scalar(float(-i)));  y.push_back(Scalar(1.0f));
		}
		x.push_back(Scalar(0.125f));  y.push_back(Scalar(1.0f));
		if (double(fdp(x, y)) != 0.125) {
			std::cerr << "FAIL: catastrophic 9 (residual 0.125)\n"; ++nrOfFailedTestCases;
		}
	}

	// Case 10: products at maxpos scale cancelling to leave minpos
	{
		// Use products at very different scales:
		// 1e30 * 1.0 - 1e30 * 1.0 + 1e-30 * 1.0 = 1e-30
		std::vector<Scalar> x = { Scalar(1e30f), Scalar(-1e30f), Scalar(1e-30f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		double expected = double(float(Scalar(1e-30f)));
		double result = double(fdp(x, y));
		if (result != expected) {
			std::cerr << "FAIL: catastrophic 10 (maxpos→minpos), expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 11: alternating series with exact cancellation
	// Pairs: (-1 + 2) + (-3 + 4) + ... + (-1023 + 1024) = 512 ones = 512
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1024; ++i) {
			// i=0: -1, i=1: +2, i=2: -3, i=3: +4, ...
			float xi = float(i + 1) * ((i % 2 == 0) ? -1.0f : 1.0f);
			x[i] = Scalar(xi);
			y[i] = Scalar(1.0f);
		}
		Scalar result = fdp(x, y);
		if (double(result) != 512.0) {
			std::cerr << "FAIL: catastrophic 11 (alternating i), expected 512, got "
			          << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdpAccuracy: verify FDP matches double-precision reference
// ============================================================================
int TestFdpAccuracy() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// 1024-element accuracy test
	constexpr size_t N = 1024;
	std::vector<Scalar> x(N), y(N);
	double reference = 0.0;
	for (size_t i = 0; i < N; ++i) {
		float xv = static_cast<float>(i + 1) * 0.1f;
		float yv = static_cast<float>(N - i) * 0.1f;
		x[i] = Scalar(xv);
		y[i] = Scalar(yv);
		reference += static_cast<double>(float(x[i])) * static_cast<double>(float(y[i]));
	}

	Scalar fdp_result = fdp(x, y);
	double fdp_d = double(fdp_result);
	Scalar expected(static_cast<float>(reference));
	double expected_d = double(expected);

	float naive = naive_dot(x, y);

	std::cout << "  reference (double):      " << reference << '\n';
	std::cout << "  expected cfloat:         " << expected_d << '\n';
	std::cout << "  fdp result:              " << fdp_d << '\n';
	std::cout << "  naive fp32 result:       " << naive << '\n';

	if (fdp_d != expected_d) {
		double fdp_error = std::abs(fdp_d - reference);
		double expected_error = std::abs(expected_d - reference);
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

	// --- fp16 (cfloat<16,5>) ---
	{
		using Scalar = cfloat<16, 5, uint16_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(2.0f), Scalar(3.0f) };
		std::vector<Scalar> y = { Scalar(4.0f), Scalar(5.0f) };
		if (double(fdp(x, y)) != 23.0) {
			std::cerr << "FAIL: fp16 fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- fp8 es=5 (E5M2) ---
	{
		using Scalar = cfloat<8, 5, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 2.0) {
			std::cerr << "FAIL: fp8_e5m2 fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- fp8 es=4 (E4M3) ---
	{
		using Scalar = cfloat<8, 4, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 3.0) {
			std::cerr << "FAIL: fp8_e4m3 fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- fp8 es=3 ---
	{
		using Scalar = cfloat<8, 3, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 6.0) {
			std::cerr << "FAIL: fp8_e3m4 fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- fp8 cancellation test (es=4) ---
	{
		using Scalar = cfloat<8, 4, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(4.0f), Scalar(-4.0f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 1.0) {
			std::cerr << "FAIL: fp8 cancellation\n"; ++nrOfFailedTestCases;
		}
	}

	// --- cfloat<20,8> (11 fraction bits) ---
	{
		using Scalar = cfloat<20, 8, uint32_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(100.0f), Scalar(200.0f), Scalar(300.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f) };
		if (double(fdp(x, y)) != 1400.0) {
			std::cerr << "FAIL: cfloat<20,8> fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- cfloat<16,8> (7 fraction bits) ---
	{
		using Scalar = cfloat<16, 8, uint16_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(2.0f), Scalar(4.0f) };
		std::vector<Scalar> y = { Scalar(3.0f), Scalar(5.0f) };
		if (double(fdp(x, y)) != 26.0) {
			std::cerr << "FAIL: cfloat<16,8> fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- cfloat<12,5> (6 fraction bits) ---
	{
		using Scalar = cfloat<12, 5, uint16_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f), Scalar(4.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 10.0) {
			std::cerr << "FAIL: cfloat<12,5> fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- cfloat<6,2> (3 fraction bits) ---
	{
		using Scalar = cfloat<6, 2, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(2.0f) };
		if (double(fdp(x, y)) != 3.0) {
			std::cerr << "FAIL: cfloat<6,2> fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- cfloat<4,1> (es=1 requires subnormals+maxexp) ---
	{
		using Scalar = cfloat<4, 1, uint8_t, true, true, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 2.0) {
			std::cerr << "FAIL: cfloat<4,1> fdp\n"; ++nrOfFailedTestCases;
		}
	}

	// --- cfloat<20,8> cancellation test ---
	{
		using Scalar = cfloat<20, 8, uint32_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1e10f), Scalar(-1e10f), Scalar(1.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f) };
		if (double(fdp(x, y)) != 1.0) {
			std::cerr << "FAIL: cfloat<20,8> cancellation\n"; ++nrOfFailedTestCases;
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

	nrOfFailedTestCases += ReportTestResult(TestQuireMul(), "cfloat quire_mul", "unrounded product + limb placement");
	nrOfFailedTestCases += ReportTestResult(TestSpecialValues(), "cfloat quire_mul", "special values");
	nrOfFailedTestCases += ReportTestResult(TestLimbBoundaryCarryBorrow(), "cfloat quire", "limb-boundary carry/borrow");
	nrOfFailedTestCases += ReportTestResult(TestFdp1024(), "cfloat fdp", "1024-element vectors");
	nrOfFailedTestCases += ReportTestResult(TestFdpStride(), "cfloat fdp_stride", "strided dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdpQcStress(), "cfloat fdp_qc", "quire continuation stress");
	nrOfFailedTestCases += ReportTestResult(TestCatastrophicCancellation(), "cfloat fdp", "catastrophic cancellation");
	nrOfFailedTestCases += ReportTestResult(TestFdpAccuracy(), "cfloat fdp", "accuracy vs double (1024 elements)");
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
