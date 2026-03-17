#pragma once
// fdp.hpp: fused dot product and quire accumulation support for cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// cfloat already uses blocktriple as its internal arithmetic engine, so
// quire_mul simply produces the unrounded full-precision MUL blocktriple
// that the generalized quire can accumulate directly.
//
// The MUL blocktriple has bfbits = 2 + 2*fbits significand bits, which
// exactly captures all 2*(fbits+1) product bits without rounding.
//
// Relates to #345, #547

#include <vector>
#include <cassert>
#include <universal/traits/cfloat_traits.hpp>
#include <universal/number/quire/quire.hpp>

namespace sw { namespace universal {

// ============================================================================
// quire_mul: unrounded full-precision product for quire accumulation
//
// Returns a blocktriple<fbits, MUL> containing the exact product of two
// cfloat values. The MUL blocktriple significand has 2 + 2*fbits bits,
// which captures all 2*(fbits+1) product bits without any rounding.
// ============================================================================
template<unsigned nbits, unsigned es, typename bt,
         bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
blocktriple<nbits - 1u - es, BlockTripleOperator::MUL, bt>
quire_mul(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& lhs,
          const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& rhs) {
	constexpr unsigned fbits = nbits - 1u - es;
	blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;

	if (lhs.iszero() || rhs.iszero()) return product;  // product is zero
	if (lhs.isinf() || lhs.isnan() || rhs.isinf() || rhs.isnan()) {
		product.setnan();
		return product;
	}

	lhs.normalizeMultiplication(a);
	rhs.normalizeMultiplication(b);
	product.mul(a, b);
	return product;
}

// ============================================================================
// quire_resolve: extract a cfloat from a quire via blocktriple conversion
//
// This avoids the double-precision intermediate in quire::convert_to<T>(),
// which loses precision for cfloat types wider than 53 fraction bits.
// Instead, we extract the top significant bits from the quire accumulator
// into a blocktriple<fbits, MUL, bt> (matching the cfloat's block type),
// and use the existing convert(blocktriple, cfloat) rounding logic.
// ============================================================================
template<unsigned nbits, unsigned es, typename bt,
         bool hasSubnormals, bool hasMaxExpValues, bool isSaturating,
         unsigned capacity, typename LimbType>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>
quire_resolve(const quire<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>, capacity, LimbType>& q) {
	using Scalar = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	constexpr unsigned fbits = nbits - 1u - es;
	// Use MUL blocktriple: bfbits = 2 + 2*fbits, radix = 2*fbits
	// This gives enough bits for hidden + fraction + guard/round/sticky
	using BT = blocktriple<fbits, BlockTripleOperator::MUL, bt>;
	constexpr unsigned bfbits = BT::bfbits;
	constexpr int bt_radix = BT::radix;  // = 2*fbits

	Scalar result;
	if (q.iszero()) {
		result.setzero();
		return result;
	}

	int s = q.scale();
	constexpr unsigned qrange = quire_traits<Scalar>::range;
	constexpr unsigned rp = quire_traits<Scalar>::radix_point;

	// Find the MSB position in the accumulator
	int msb_pos = s + static_cast<int>(rp);
	if (msb_pos < 0 || msb_pos >= static_cast<int>(qrange + capacity)) {
		result.setzero();
		return result;
	}

	// Extract the top significant bits from the quire into a MUL blocktriple.
	//
	// MUL blocktriple layout: bfbits = 2 + 2*fbits, radix = 2*fbits
	//   bit (radix)   = 1.0 position (hidden bit, no overflow)
	//   bit (radix+1) = 2.0 position (overflow form)
	//
	// We place the quire MSB at blocktriple bit (radix), giving
	// significandscale = 0, so convert() uses exponent = scale + 0 = s.
	// Then copy (radix) more bits below it for fraction + rounding bits.
	BT bt_val;
	bt_val.setnormal();
	bt_val.setsign(q.sign());
	bt_val.setscale(s);

	// Copy bits: blocktriple bit (bt_radix - k) ← accumulator bit (msb_pos - k)
	// k=0 is the MSB (hidden bit), k increases downward
	for (unsigned k = 0; k < bfbits; ++k) {
		int bt_bit = bt_radix - static_cast<int>(k);
		int accu_bit = msb_pos - static_cast<int>(k);
		if (bt_bit >= 0 && bt_bit < static_cast<int>(bfbits)) {
			bool val = (accu_bit >= 0)
				? q[static_cast<unsigned>(accu_bit)]
				: false;
			bt_val.setbit(static_cast<unsigned>(bt_bit), val);
		}
	}

	// Sticky bit: if any accumulator bits below what we extracted are set,
	// set the blocktriple LSB so rounding correctly rounds up when needed.
	int lowest_copied = msb_pos - static_cast<int>(bt_radix);
	if (lowest_copied > 0 && q.anyAfter(static_cast<unsigned>(lowest_copied - 1))) {
		bt_val.setbit(0, true);
	}

	convert(bt_val, result);
	return result;
}

// ============================================================================
// Fused dot product operators
//
// fdp_qc       fused dot product with quire continuation
// fdp_stride   fused dot product with non-negative stride
// fdp          fused dot product of two vectors (unit stride)
// ============================================================================

/// Fused dot product with quire continuation.
/// Accumulates products into an existing quire without resolving.
/// The quire must be parameterized on the same scalar type as the vectors.
template<typename Qy, typename Vector>
void fdp_qc(Qy& sum_of_products, size_t n,
            const Vector& x, size_t incx,
            const Vector& y, size_t incy,
            std::enable_if_t<is_cfloat<typename Vector::value_type>, int> = 0) {
	using Scalar = typename Vector::value_type;
	static_assert(std::is_same<Qy, quire<Scalar>>::value,
		"fdp_qc: quire type must match the vector's scalar type");
	if (n == 0) return;
	assert(incx > 0 && "fdp_qc: incx must be positive");
	assert(incy > 0 && "fdp_qc: incy must be positive");
	size_t ix, iy;
	for (ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		assert(ix < x.size() && iy < y.size() && "fdp_qc: index out of bounds");
		sum_of_products += quire_mul(x[ix], y[iy]);
	}
}

/// Resolved fused dot product with stride.
/// Accumulates all products in a quire, then resolves with a single rounding
/// via blocktriple conversion (no double intermediate, supports all cfloat widths).
template<typename Vector>
enable_if_cfloat<typename Vector::value_type>
fdp_stride(size_t n, const Vector& x, size_t incx, const Vector& y, size_t incy) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;

	if (n == 0) return Scalar(0);
	assert(incx > 0 && "fdp_stride: incx must be positive");
	assert(incy > 0 && "fdp_stride: incy must be positive");
	size_t ix, iy;
	for (ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		assert(ix < x.size() && iy < y.size() && "fdp_stride: index out of bounds");
		q += quire_mul(x[ix], y[iy]);
	}

	// single rounding step via blocktriple conversion
	return quire_resolve(q);
}

/// Resolved fused dot product with unit stride.
/// This is the primary FDP entry point for cfloat vectors.
template<typename Vector>
enable_if_cfloat<typename Vector::value_type>
fdp(const Vector& x, const Vector& y) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;

	size_t n = size(x);
	assert(n <= size(y) && "fdp: y vector must be at least as long as x");
	for (size_t i = 0; i < n; ++i) {
		q += quire_mul(x[i], y[i]);
	}

	// single rounding step via blocktriple conversion
	return quire_resolve(q);
}

}} // namespace sw::universal
