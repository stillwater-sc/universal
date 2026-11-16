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

#include <type_traits>   // std::is_same / std::true_type
#include <cstddef>     // std::size_t in the fdp_qc signatures
#include <vector>
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

	// specials first: a NaN times zero, and inf times zero, are NaN, not a zero product;
	// inf times a nonzero value is an infinity signed by the XOR of the signs (#2019)
	if (lhs.isnan() || rhs.isnan()) {
		product.setnan(false);   // a quiet NaN: the blocktriple carries the type in its sign
		return product;
	}
	if (lhs.isinf() || rhs.isinf()) {
		if (lhs.iszero() || rhs.iszero()) product.setnan(false);
		else product.setinf(lhs.sign() != rhs.sign());
		return product;
	}
	if (lhs.iszero() || rhs.iszero()) return product;  // product is zero

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
	if (q.isnan()) { result.setnan(NAN_TYPE_QUIET); return result; }  // NaN propagated through the accumulator (#1226)
	if (q.isinf()) { result.setinf(q.sign()); return result; }        // a signed infinity (#2019); a saturating cfloat clamps
	if (q.isoverflow()) { result.setinf(q.sign()); return result; }   // a sum past the accumulator is an infinity too (#2094)
	if (q.iszero()) {
		result.setzero();
		return result;
	}

	int s = q.scale();
	constexpr unsigned rp = quire_traits<Scalar>::radix_point;

	// Find the MSB position in the accumulator.  q.scale() is bounded by the accumulator, so msb_pos
	// is in [0, qrange + capacity).  A sum that outgrows the accumulator is lost before this point:
	// the quire discards the carry out of its top bit and keeps no overflow state.
	const int msb_pos = s + static_cast<int>(rp);

	// Extract the top significant bits from the quire into a MUL blocktriple.
	//
	// MUL blocktriple layout: bfbits = 2 + 2*fbits, radix = 2*fbits
	//   bit (radix)   = 1.0 position (hidden bit, no overflow)
	//   bit (radix+1) = 2.0 position (overflow form)
	//
	// The quire MSB goes to the overflow position, bit (radix + 1), with scale s - 1, so that
	// convert() sees significandscale = 1 and exponent = s.  That leaves bits radix .. 1 for the
	// fraction and the guard and round bits, and bit 0 for the sticky bit alone.  With the MSB at
	// bit (radix) the sticky bit was ORed into bit 0, which for fbits = 1 is the guard bit: a
	// cfloat<4,2> fdp of 0.5*0.5 + 3 rounded up to inf (#2089).
	BT bt_val;
	bt_val.setnormal();
	bt_val.setsign(q.sign());
	bt_val.setscale(s - 1);

	// Copy bits: blocktriple bit (bt_radix + 1 - k) <- accumulator bit (msb_pos - k), k = 0 .. bt_radix
	for (int k = 0; k <= bt_radix; ++k) {
		const int bt_bit = bt_radix + 1 - k;
		const int accu_bit = msb_pos - k;
		const bool val = (accu_bit >= 0) ? q[static_cast<unsigned>(accu_bit)] : false;
		bt_val.setbit(static_cast<unsigned>(bt_bit), val);
	}
	static_assert(bfbits == static_cast<unsigned>(bt_radix) + 2u, "the MUL blocktriple holds radix + 2 bits");

	// Sticky bit: if any accumulator bit below the lowest one copied is set, set the blocktriple
	// LSB so that rounding sees the discarded tail.
	const int lowest_copied = msb_pos - bt_radix;
	bt_val.setbit(0, lowest_copied > 0 && q.anyAfter(static_cast<unsigned>(lowest_copied - 1)));

	convert(bt_val, result);
	return result;
}

// ============================================================================
// Fused dot product operators
//
// fdp_qc       fused dot product with quire continuation
// fdp_stride   fused dot product with non-negative stride
// fdp          fused dot product of two vectors (unit stride)
//
// The stride forms take n as the INDEX bound, not the product count: they accumulate
// x[k*incx] * y[k*incy] for every k with k*incx < n and k*incy < n, so about n / max(incx, incy)
// products.  BLAS dot takes n as the count; pass n = count * inc for that.  Every fdp header
// (posit, bposit, cfloat, fixpnt, lns, dbns) uses this one meaning (#2080).
// ============================================================================

/// Fused dot product with quire continuation.
/// Accumulates products into an existing quire without resolving.
/// The quire must be parameterized on the same scalar type as the vectors.
/// A zero stride, or an index past a vector, makes the quire NaN, in every build: the asserts
/// vanished under NDEBUG, so a zero stride re-read x[0] and a long n read past the vectors (#2089).
template<typename Qy, typename Vector>
void fdp_qc(Qy& sum_of_products, std::size_t n,
            const Vector& x, std::size_t incx,
            const Vector& y, std::size_t incy,
            std::enable_if_t<is_cfloat<typename Vector::value_type>, int> = 0) {
	using Scalar = typename Vector::value_type;
	static_assert(std::is_same<Qy, quire<Scalar>>::value,
		"fdp_qc: quire type must match the vector's scalar type");
	if (n == 0) return;
	Scalar nan;
	nan.setnan(NAN_TYPE_QUIET);
	if (incx == 0 || incy == 0) { sum_of_products += quire_mul(nan, nan); return; }
	for (std::size_t ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		if (ix >= x.size() || iy >= y.size()) { sum_of_products += quire_mul(nan, nan); return; }
		sum_of_products += quire_mul(x[ix], y[iy]);
	}
}

/// Resolved fused dot product with stride.
/// Accumulates all products in a quire, then resolves with a single rounding
/// via blocktriple conversion (no double intermediate, supports all cfloat widths).
/// A zero stride, or an index past a vector, gives a quiet NaN in every build (#2089).
template<typename Vector>
enable_if_cfloat<typename Vector::value_type>
fdp_stride(std::size_t n, const Vector& x, std::size_t incx, const Vector& y, std::size_t incy) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;
	Scalar nan;
	nan.setnan(NAN_TYPE_QUIET);
	if (n == 0) return Scalar(0);
	if (incx == 0 || incy == 0) return nan;
	for (std::size_t ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		if (ix >= x.size() || iy >= y.size()) return nan;
		q += quire_mul(x[ix], y[iy]);
	}

	// single rounding step via blocktriple conversion
	return quire_resolve(q);
}

/// Resolved fused dot product with unit stride.
/// This is the primary FDP entry point for cfloat vectors.
/// A y shorter than x gives a quiet NaN in every build (#2089).
template<typename Vector>
enable_if_cfloat<typename Vector::value_type>
fdp(const Vector& x, const Vector& y) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;

	const std::size_t n = size(x);
	if (n > size(y)) { Scalar nan; nan.setnan(NAN_TYPE_QUIET); return nan; }
	for (std::size_t i = 0; i < n; ++i) {
		q += quire_mul(x[i], y[i]);
	}

	// single rounding step via blocktriple conversion
	return quire_resolve(q);
}

}} // namespace sw::universal
