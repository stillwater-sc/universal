#pragma once
// fdp.hpp: fused dot product and quire accumulation support for fixpnt
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fixpnt uses blockbinary exclusively (no blocktriple), so quire_mul bridges
// from blockbinary products to blocktriple for quire accumulation. The full
// unrounded product of two fixpnt<nbits, rbits> values is a 2*nbits-bit
// two's complement number with 2*rbits fractional bits.
//
// Relates to #345, #548

#include <vector>
#include <cassert>
#include <universal/traits/fixpnt_traits.hpp>
#include <universal/number/quire/quire.hpp>

namespace sw { namespace universal {

// ============================================================================
// quire_mul: unrounded full-precision product for quire accumulation
//
// Computes the exact product of two fixpnt values using blockbinary urmul2(),
// then converts the result into a blocktriple for quire accumulation.
//
// For fixpnt<nbits, rbits, arithmetic, bt>:
//   - Full product is blockbinary<2*nbits, bt, Signed> (2's complement)
//   - Product has 2*rbits fractional bits
//   - We extract: sign, magnitude, MSB position → blocktriple
//
// Returns a blocktriple<2*nbits-2, REP, bt> representing the unrounded product.
// The REP tag with fbits=2*nbits-2 gives bfbits=2*nbits, radix=2*nbits-2,
// which can hold the full product magnitude.
// ============================================================================
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
blocktriple<2*nbits - 2, BlockTripleOperator::REP, bt>
quire_mul(const fixpnt<nbits, rbits, arithmetic, bt>& lhs,
          const fixpnt<nbits, rbits, arithmetic, bt>& rhs) {
	constexpr unsigned pbits = 2 * nbits;       // product width
	constexpr unsigned prod_rbits = 2 * rbits;   // product fractional bits
	constexpr unsigned bt_fbits = pbits - 2;     // blocktriple fbits parameter
	using ProductBB = blockbinary<pbits, bt, BinaryNumberType::Signed>;
	using ResultBT = blocktriple<bt_fbits, BlockTripleOperator::REP, bt>;

	ResultBT product;

	if (lhs.iszero() || rhs.iszero()) return product;  // product is zero

	// Compute exact product using blockbinary multiplication
	ProductBB full_product = urmul2(lhs.bits(), rhs.bits());

	// Extract sign and magnitude
	bool product_sign = full_product.sign();
	ProductBB magnitude = full_product;
	if (product_sign) {
		magnitude.twosComplement();
	}

	// Find MSB of magnitude (position of highest set bit)
	int msb = -1;
	for (int i = static_cast<int>(pbits) - 1; i >= 0; --i) {
		if (magnitude.test(static_cast<unsigned>(i))) {
			msb = i;
			break;
		}
	}
	if (msb < 0) return product;  // zero (shouldn't happen, but safety)

	// Scale is the MSB position relative to the product's radix point
	int scale = msb - static_cast<int>(prod_rbits);

	product.setnormal();
	product.setsign(product_sign);
	product.setscale(scale);

	// Copy magnitude bits into blocktriple significand.
	// Blocktriple REP layout: bfbits = bt_fbits + 2 = pbits, radix = bt_fbits = pbits - 2
	// We place the MSB at blocktriple bit (bt_fbits) = radix, giving significandscale = 0.
	// Then copy lower bits below it.
	constexpr unsigned bfbits = ResultBT::bfbits;
	for (unsigned k = 0; k < bfbits && k <= static_cast<unsigned>(msb); ++k) {
		int src_bit = msb - static_cast<int>(k);
		int dst_bit = static_cast<int>(bt_fbits) - static_cast<int>(k);
		if (dst_bit >= 0 && dst_bit < static_cast<int>(bfbits) && src_bit >= 0) {
			product.setbit(static_cast<unsigned>(dst_bit), magnitude.test(static_cast<unsigned>(src_bit)));
		}
	}

	return product;
}

// ============================================================================
// quire_resolve: extract a fixpnt from a quire
//
// For fixpnt types that fit in 64 bits, we use the quire's convert_to<> path.
// For wider types, we extract bits directly from the accumulator.
// ============================================================================
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt,
         unsigned capacity, typename LimbType>
fixpnt<nbits, rbits, arithmetic, bt>
quire_resolve(const quire<fixpnt<nbits, rbits, arithmetic, bt>, capacity, LimbType>& q) {
	using Scalar = fixpnt<nbits, rbits, arithmetic, bt>;

	if (q.iszero()) {
		Scalar result;
		result.setzero();
		return result;
	}

	// For fixpnt up to 64 bits, the double path is exact for integer values
	// and close enough for most fixed-point values.
	// For wider types, we extract bits directly.
	if constexpr (nbits <= 53) {
		return q.template convert_to<Scalar>();
	}
	else {
		// Direct bit extraction from quire accumulator
		constexpr unsigned rp = quire_traits<Scalar>::radix_point;  // = 2*rbits
		Scalar result;
		result.setzero();

		int s = q.scale();
		int msb_pos = s + static_cast<int>(rp);

		// The fixpnt radix point is at bit position rbits.
		// Quire accumulator bit i represents 2^(i - rp) = 2^(i - 2*rbits).
		// fixpnt bit j represents 2^(j - rbits).
		// So quire bit i maps to fixpnt bit j where: i - 2*rbits = j - rbits → j = i - rbits.

		// Extract nbits from the quire starting at the MSB
		// The MSB of the fixpnt result is at bit (nbits-2) for positive values
		// (bit nbits-1 is the sign bit)
		int quire_start = msb_pos;  // highest significant bit in quire
		int fixpnt_start = quire_start - static_cast<int>(rbits);  // corresponding fixpnt bit

		if (fixpnt_start >= static_cast<int>(nbits) - 1) {
			// overflow: saturate or wrap
			if constexpr (arithmetic == Saturate) {
				if (q.sign()) result.maxneg(); else result.maxpos();
			}
			else {
				// Modulo: extract the lower nbits worth of accumulator bits
				for (unsigned j = 0; j < nbits - 1; ++j) {
					unsigned qi = j + rbits;  // quire bit for fixpnt bit j
					if (qi < quire_traits<Scalar>::range + capacity) {
						result.setbit(j, q[qi]);
					}
				}
				// Handle sign
				if (q.sign()) result.twosComplement();
			}
		}
		else {
			// Normal case: extract bits
			for (unsigned j = 0; j < nbits - 1; ++j) {
				unsigned qi = j + rbits;  // quire bit for fixpnt bit j
				if (qi < quire_traits<Scalar>::range + capacity) {
					result.setbit(j, q[qi]);
				}
			}
			if (q.sign()) result.twosComplement();
		}

		return result;
	}
}

// ============================================================================
// Fused dot product operators
//
// fdp_qc       fused dot product with quire continuation
// fdp_stride   fused dot product with non-negative stride
// fdp          fused dot product of two vectors (unit stride)
// ============================================================================

/// Fused dot product with quire continuation.
template<typename Qy, typename Vector>
void fdp_qc(Qy& sum_of_products, size_t n,
            const Vector& x, size_t incx,
            const Vector& y, size_t incy,
            std::enable_if_t<is_fixpnt<typename Vector::value_type>, int> = 0) {
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
template<typename Vector>
enable_if_fixpnt<typename Vector::value_type>
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
enable_if_fixpnt<typename Vector::value_type>
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
