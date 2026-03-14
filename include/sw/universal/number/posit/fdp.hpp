#pragma once
// fdp.hpp: fused dot product and quire support for posit using the generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This header provides a blocktriple-based quire_mul and quire_resolve for posit types,
// enabling accumulation in the generalized quire (quire_impl.hpp).
// It does NOT use the old bitblock-based posit/quire.hpp or internal::value.
//
// Relates to #345, #547

#include <vector>
#include <cassert>
#include <universal/traits/posit_traits.hpp>
#include <universal/number/quire/quire.hpp>  // generalized quire -- NOT posit/quire.hpp

namespace sw { namespace universal {

// ============================================================================
// quire_mul: unrounded full-precision product for quire accumulation
//
// Returns a blocktriple<fbits, MUL> containing the exact product of two
// posit values. The MUL blocktriple significand has 2 + 2*fbits bits,
// which captures all 2*(fbits+1) product bits without any rounding.
// ============================================================================
template<unsigned nbits, unsigned es, typename bt>
blocktriple<posit<nbits, es, bt>::fbits, BlockTripleOperator::MUL, bt>
quire_mul(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	constexpr unsigned fbits = posit<nbits, es, bt>::fbits;
	blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;

	if (lhs.iszero() || rhs.iszero()) return product;  // product is zero
	if (lhs.isnar() || rhs.isnar()) {
		product.setnan();
		return product;
	}

	lhs.normalizeMultiplication(a);
	rhs.normalizeMultiplication(b);
	product.mul(a, b);
	return product;
}

// ============================================================================
// quire_resolve: extract a posit from a quire via blocktriple conversion
//
// Extracts the top significant bits from the quire accumulator into a
// blocktriple, then uses the existing posit conversion path for correct
// rounding. This avoids the double-precision intermediate in convert_to<T>().
// ============================================================================
template<unsigned nbits, unsigned es, typename bt,
         unsigned capacity, typename LimbType>
posit<nbits, es, bt>
quire_resolve(const quire<posit<nbits, es, bt>, capacity, LimbType>& q) {
	using Scalar = posit<nbits, es, bt>;
	constexpr unsigned fbits = Scalar::fbits;
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
	BT bt_val;
	bt_val.setnormal();
	bt_val.setsign(q.sign());
	bt_val.setscale(s);

	// Copy bits: blocktriple bit (bt_radix - k) <- accumulator bit (msb_pos - k)
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
// Posit-specific quire convenience functions
// ============================================================================

/// quire_size: return the total quire size in bits for a posit configuration
template<unsigned nbits, unsigned es, unsigned capacity = 30>
inline int quire_size() {
	return static_cast<int>(quire_traits<posit<nbits, es>>::range + capacity);
}

// ============================================================================
// Fused dot product operators
// ============================================================================

/// Fused dot product with quire continuation.
template<typename Qy, typename Vector>
void fdp_qc(Qy& sum_of_products, size_t n,
            const Vector& x, size_t incx,
            const Vector& y, size_t incy,
            std::enable_if_t<is_posit<typename Vector::value_type>, int> = 0) {
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
template<typename Vector>
enable_if_posit<typename Vector::value_type, typename Vector::value_type>
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

	return quire_resolve(q);
}

/// Resolved fused dot product with unit stride.
template<typename Vector>
enable_if_posit<typename Vector::value_type, typename Vector::value_type>
fdp(const Vector& x, const Vector& y) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;

	size_t n = size(x);
	assert(n <= size(y) && "fdp: y vector must be at least as long as x");
	for (size_t i = 0; i < n; ++i) {
		q += quire_mul(x[i], y[i]);
	}

	return quire_resolve(q);
}

}} // namespace sw::universal
