#pragma once
// atomic_fused_operators.hpp: atomic fused operators for posits using blocktriple<>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// These operators use blocktriple<> for intermediate computation, chaining
// multiply and add operations with a single final rounding step via convert().
// No dependency on internal::value<>, bitblock<>, module_multiply, or module_add.

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////
// Helper: transfer a blocktriple result into an ADD-type blocktriple.
// Used when chaining MUL->ADD (FMA, FMMA) or when widening an ADD operand.
// The source significand is in magnitude form (blocktriple always normalizes
// to magnitude after add/mul). We extract fraction bits below the MSB and
// place them into the ADD layout: hidden bit at tgt_fbits, shifted by rbits.

template<unsigned src_fbits, BlockTripleOperator src_op, typename bt, unsigned tgt_fbits>
inline void extractToAdd(const blocktriple<src_fbits, src_op, bt>& src,
                         blocktriple<tgt_fbits, BlockTripleOperator::ADD, bt>& tgt) {
	if (src.iszero()) { tgt.setzero(); return; }
	if (src.isnan() || src.isinf()) { tgt.setnan(); return; }

	using Src = blocktriple<src_fbits, src_op, bt>;
	using Tgt = blocktriple<tgt_fbits, BlockTripleOperator::ADD, bt>;

	int sigScale = src.significandscale();
	int realScale = src.scale() + sigScale;
	int msbPos = static_cast<int>(Src::radix) + sigScale;

	tgt.setnormal();
	tgt.setsign(src.sign());
	tgt.setscale(realScale);

	// Build raw significand: hidden bit at tgt_fbits, fraction below, shifted by rbits
	if constexpr (tgt_fbits + Tgt::rbits + 1 < 64) {
		// fast path: everything fits in a uint64_t
		uint64_t raw = 0;
		for (unsigned i = 0; i < tgt_fbits; ++i) {
			int srcPos = msbPos - 1 - static_cast<int>(i);
			if (srcPos >= 0 && srcPos < static_cast<int>(Src::bfbits) &&
			    src.at(static_cast<unsigned>(srcPos)))
				raw |= (1ull << (tgt_fbits - 1 - i));
		}
		raw |= (1ull << tgt_fbits);   // hidden bit
		raw <<= Tgt::rbits;           // rounding bits shift
		tgt.setbits(raw);
	}
	else {
		// block-by-block path for large configurations
		tgt.clear();
		tgt.setnormal();
		tgt.setsign(src.sign());
		tgt.setscale(realScale);
		for (unsigned i = 0; i < tgt_fbits; ++i) {
			int srcPos = msbPos - 1 - static_cast<int>(i);
			if (srcPos >= 0 && srcPos < static_cast<int>(Src::bfbits) &&
			    src.at(static_cast<unsigned>(srcPos)))
				tgt.setbit(Tgt::rbits + tgt_fbits - 1 - i, true);
		}
		tgt.setbit(static_cast<unsigned>(Tgt::radix), true); // hidden bit at radix
		tgt.setradix();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Helper: transfer a blocktriple result into a MUL-type blocktriple.
// Used when chaining ADD->MUL (FAM).
// MUL input layout: hidden bit at tgt_fbits, fraction below, no rounding shift.
// IMPORTANT: only valid for post-operation blocktriples where significandscale()
// correctly identifies the MSB position (at or above radix). Do NOT use on
// freshly-normalized MUL blocktriples (pre-multiply), where MSB is below radix.

template<unsigned src_fbits, BlockTripleOperator src_op, typename bt, unsigned tgt_fbits>
inline void extractToMul(const blocktriple<src_fbits, src_op, bt>& src,
                         blocktriple<tgt_fbits, BlockTripleOperator::MUL, bt>& tgt) {
	if (src.iszero()) { tgt.setzero(); return; }
	if (src.isnan() || src.isinf()) { tgt.setnan(); return; }

	using Src = blocktriple<src_fbits, src_op, bt>;

	int sigScale = src.significandscale();
	int realScale = src.scale() + sigScale;
	int msbPos = static_cast<int>(Src::radix) + sigScale;

	tgt.setnormal();
	tgt.setsign(src.sign());
	tgt.setscale(realScale);

	// Build raw significand: hidden bit at tgt_fbits, fraction below
	if constexpr (tgt_fbits + 1 < 64) {
		uint64_t raw = 0;
		for (unsigned i = 0; i < tgt_fbits; ++i) {
			int srcPos = msbPos - 1 - static_cast<int>(i);
			if (srcPos >= 0 && srcPos < static_cast<int>(Src::bfbits) &&
			    src.at(static_cast<unsigned>(srcPos)))
				raw |= (1ull << (tgt_fbits - 1 - i));
		}
		raw |= (1ull << tgt_fbits);   // hidden bit
		tgt.setbits(raw);
	}
	else {
		tgt.clear();
		tgt.setnormal();
		tgt.setsign(src.sign());
		tgt.setscale(realScale);
		for (unsigned i = 0; i < tgt_fbits; ++i) {
			int srcPos = msbPos - 1 - static_cast<int>(i);
			if (srcPos >= 0 && srcPos < static_cast<int>(Src::bfbits) &&
			    src.at(static_cast<unsigned>(srcPos)))
				tgt.setbit(tgt_fbits - 1 - i, true);
		}
		tgt.setbit(tgt_fbits, true); // hidden bit
	}
}

///////////////////////////////////////////////////////////////////////////////
// Helper: normalize a posit into a wider-than-natural MUL blocktriple.
// Used when the other MUL operand has higher precision (e.g., an ADD result).
// The posit's fbits fraction bits are placed in the top positions and
// the remaining lower bits are zero-extended.

template<unsigned nbits, unsigned es, typename bt, unsigned wfbits>
inline void normalizeMultiplicationWide(const posit<nbits, es, bt>& p,
                                        blocktriple<wfbits, BlockTripleOperator::MUL, bt>& tgt) {
	constexpr unsigned pf = nbits - 3 - es;  // posit's natural fraction bits
	if (p.isnar()) { tgt.setnan(); return; }
	if (p.iszero()) { tgt.setzero(); return; }

	tgt.setnormal();
	tgt.setsign(sign(p));
	tgt.setscale(scale(p));

	if constexpr (wfbits + 1 < 64) {
		blockbinary<pf, bt> frac = extract_fraction<nbits, es, bt, pf>(p);
		uint64_t raw = (pf > 0) ? frac.to_ull() : 0ull;
		raw |= (1ull << pf);              // hidden bit at pf
		raw <<= (wfbits - pf);            // zero-extend: hidden bit at wfbits
		tgt.setbits(raw);
	}
	else {
		tgt.clear();
		tgt.setnormal();
		tgt.setsign(sign(p));
		tgt.setscale(scale(p));
		blockbinary<pf, bt> frac = extract_fraction<nbits, es, bt, pf>(p);
		for (unsigned i = 0; i < pf; ++i) {
			if (frac.test(i))
				tgt.setbit((wfbits - pf) + i, true);
		}
		tgt.setbit(wfbits, true); // hidden bit
	}
}

///////////////////////////////////////////////////////////////////////////////
// FMA: fused multiply-add:  a*b + c  (single rounding at end)
//
// Pattern: MUL -> ADD -> convert
//   1. Multiply a*b via blocktriple (full-precision product)
//   2. Widen product and c to ADD blocktriples at product precision
//   3. Add the two at full precision
//   4. Single rounding step via convert(blocktriple, posit)

template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fma(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b,
                         const posit<nbits, es, bt>& c) {
	constexpr unsigned fbits = nbits - 3 - es;
	constexpr unsigned mbits = 2 * (fbits + 1);  // product precision for ADD step

	posit<nbits, es, bt> result;
	result.setzero();

	if (a.isnar() || b.isnar() || c.isnar()) {
		result.setnar();
		return result;
	}

	// Step 1: Multiply a * b
	if (a.iszero() || b.iszero()) {
		return c;  // product is zero, result = 0 + c = c
	}
	blocktriple<fbits, BlockTripleOperator::MUL, bt> ma, mb, product;
	a.normalizeMultiplication(ma);
	b.normalizeMultiplication(mb);
	product.mul(ma, mb);

	if (product.iszero()) return c;

	if (c.iszero()) {
		convert(product, result);
		return result;
	}

	// Step 2: Add product + c at product precision
	blocktriple<mbits, BlockTripleOperator::ADD, bt> add_product, add_c, sum;
	extractToAdd(product, add_product);
	// normalize c to its natural ADD type, then widen to product precision
	blocktriple<fbits, BlockTripleOperator::ADD, bt> c_natural;
	c.normalizeAddition(c_natural);
	extractToAdd(c_natural, add_c);
	sum.add(add_product, add_c);

	if (sum.iszero()) { result.setzero(); return result; }
	if (sum.isinf()) { result.setnar(); return result; }
	convert(sum, result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// FAM: fused add-multiply:  (a + b) * c  (single rounding at end)
//
// Pattern: ADD -> MUL -> convert
//   1. Add a+b via blocktriple
//   2. Transfer sum to MUL type, multiply by c
//   3. Single rounding step via convert(blocktriple, posit)

template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fam(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b,
                         const posit<nbits, es, bt>& c) {
	constexpr unsigned fbits = nbits - 3 - es;
	constexpr unsigned wfbits = fbits + 3;  // wider MUL to preserve ADD precision

	posit<nbits, es, bt> result;
	result.setzero();

	if (a.isnar() || b.isnar() || c.isnar()) {
		result.setnar();
		return result;
	}

	// special case: c is zero -> (a + b) * 0 = 0
	if (c.iszero()) return result;
	// special case: a and b both zero -> (0 + 0) * c = 0
	if (a.iszero() && b.iszero()) return result;

	// Step 1: Compute a + b, prepare MUL operand at wider precision
	blocktriple<wfbits, BlockTripleOperator::MUL, bt> mul_sum, mul_c, product;

	if (a.iszero()) {
		// sum = b; normalize directly to wide MUL
		normalizeMultiplicationWide<nbits, es, bt, wfbits>(b, mul_sum);
	}
	else if (b.iszero()) {
		// sum = a; normalize directly to wide MUL
		normalizeMultiplicationWide<nbits, es, bt, wfbits>(a, mul_sum);
	}
	else {
		blocktriple<fbits, BlockTripleOperator::ADD, bt> aa, ab, sum_ab;
		a.normalizeAddition(aa);
		b.normalizeAddition(ab);
		sum_ab.add(aa, ab);
		if (sum_ab.iszero()) return result;
		extractToMul(sum_ab, mul_sum);
	}

	// Step 2: Multiply (a+b) * c at wider precision
	normalizeMultiplicationWide<nbits, es, bt, wfbits>(c, mul_c);
	product.mul(mul_sum, mul_c);

	if (product.iszero()) { result.setzero(); return result; }
	if (product.isinf()) { result.setnar(); return result; }
	convert(product, result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// FMMA: fused multiply-multiply-add:  (a * b) +/- (c * d)
//
// Pattern: MUL -> MUL -> ADD -> convert
//   1. Multiply a*b and c*d via blocktriple
//   2. Transfer both products to ADD blocktriples at product precision
//   3. Optionally negate second product (for subtraction)
//   4. Add the two products
//   5. Single rounding step via convert(blocktriple, posit)

template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fmma(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b,
                           const posit<nbits, es, bt>& c, const posit<nbits, es, bt>& d,
                           bool opIsAdd = true) {
	constexpr unsigned fbits = nbits - 3 - es;
	constexpr unsigned mbits = 2 * (fbits + 1);  // product precision for ADD step

	posit<nbits, es, bt> result;
	result.setzero();

	if (a.isnar() || b.isnar() || c.isnar() || d.isnar()) {
		result.setnar();
		return result;
	}

	// Compute product_ab = a * b
	blocktriple<fbits, BlockTripleOperator::MUL, bt> ma, mb, product_ab;
	bool ab_zero = a.iszero() || b.iszero();
	if (!ab_zero) {
		a.normalizeMultiplication(ma);
		b.normalizeMultiplication(mb);
		product_ab.mul(ma, mb);
		ab_zero = product_ab.iszero();
	}

	// Compute product_cd = c * d
	blocktriple<fbits, BlockTripleOperator::MUL, bt> mc, md, product_cd;
	bool cd_zero = c.iszero() || d.iszero();
	if (!cd_zero) {
		c.normalizeMultiplication(mc);
		d.normalizeMultiplication(md);
		product_cd.mul(mc, md);
		cd_zero = product_cd.iszero();
	}

	if (ab_zero && cd_zero) return result;

	// Only one product is non-zero: convert directly
	if (ab_zero) {
		if (!opIsAdd) product_cd.setsign(!product_cd.sign());
		convert(product_cd, result);
		return result;
	}
	if (cd_zero) {
		convert(product_ab, result);
		return result;
	}

	// Both products non-zero: add at product precision
	blocktriple<mbits, BlockTripleOperator::ADD, bt> add_ab, add_cd, sum;
	extractToAdd(product_ab, add_ab);
	extractToAdd(product_cd, add_cd);
	if (!opIsAdd) add_cd.setsign(!add_cd.sign());
	sum.add(add_ab, add_cd);

	if (sum.iszero()) { result.setzero(); return result; }
	if (sum.isinf()) { result.setnar(); return result; }
	convert(sum, result);
	return result;
}

}} // namespace sw::universal
