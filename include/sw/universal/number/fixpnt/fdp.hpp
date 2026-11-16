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

#include <cstddef>       // std::size_t
#include <universal/utility/exceptions.hpp>  // throw_or_abort (#1925)
#include <type_traits>   // the type traits used below
#include <vector>
#include <stdexcept>
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
//   - We extract: sign, magnitude, MSB position -> blocktriple
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

	// No infinity: a sum past the accumulator saturates, in Modulo arithmetic as well (#2094, decided).
	// The accumulator no longer holds the sum's low bits once it overflows, so there is nothing to wrap.
	if (q.isoverflow()) return Scalar(q.sign() ? SpecificValue::maxneg : SpecificValue::maxpos);
	if (q.iszero()) {
		Scalar result;
		result.setzero();
		return result;
	}

	// Modulo, every width: the sum rounded to nearest even at the fixpnt's lsb, from the exact
	// accumulator bits, then wrapped mod 2^nbits. The double path cannot wrap: a sum past the range
	// keeps only its leading 53 bits there, so fixpnt<52,0,Modulo> of 2^60 + 1 was 256, not 1 (#2107).
	if constexpr (arithmetic == Modulo) {
		constexpr unsigned qbits = quire<Scalar, capacity, LimbType>::qbits;
		// quire bit i is 2^(i - 2 rbits) and fixpnt bit j is 2^(j - rbits): fixpnt bit j is quire bit j + rbits
		Scalar result;
		result.setzero();
		for (unsigned j = 0; j < nbits; ++j) {
			const unsigned qi = j + rbits;
			if (qi < qbits) result.setbit(j, q.testbit(qi));
		}
		if constexpr (rbits > 0) {
			const bool roundBit = (rbits - 1 < qbits) && q.testbit(rbits - 1);
			const bool sticky   = (rbits >= 2) && q.anyAfter(rbits - 2);
			if (roundBit && (sticky || result.test(0))) {
				// add one ulp to the magnitude, mod 2^nbits
				for (unsigned j = 0; j < nbits; ++j) {
					if (!result.test(j)) { result.setbit(j, true); break; }
					result.setbit(j, false);
				}
			}
		}
		if (q.sign()) result.twosComplement();   // -x mod 2^nbits; RNE of the magnitude is sign-symmetric
		return result;
	}
	// Saturate up to 52 bits (51 significand bits): convert_to<> rounds once, through double rounded
	// to odd, and the fixpnt conversion saturates.
	else if constexpr (nbits <= 52) {
		return q.template convert_to<Scalar>();
	}
	else {
		// Saturate, wider: the magnitude rounded to nearest even at the fixpnt lsb from the exact
		// accumulator bits, then saturated past the range (#2107; it truncated). fixpnt bit j is
		// quire bit j + rbits, as in the Modulo branch.
		constexpr unsigned qbits = quire<Scalar, capacity, LimbType>::qbits;
		Scalar result;
		result.setzero();
		const bool negative = q.sign();
		auto saturate = [&]() { if (negative) result.maxneg(); else result.maxpos(); return result; };
		// a magnitude of 2^nbits or more saturates before rounding
		const int msb_fixpnt = q.scale() + static_cast<int>(quire_traits<Scalar>::radix_point) - static_cast<int>(rbits);
		if (msb_fixpnt >= static_cast<int>(nbits)) return saturate();
		for (unsigned j = 0; j < nbits; ++j) {
			const unsigned qi = j + rbits;
			if (qi < qbits) result.setbit(j, q.testbit(qi));
		}
		if constexpr (rbits > 0) {
			const bool roundBit = (rbits - 1 < qbits) && q.testbit(rbits - 1);
			const bool sticky   = (rbits >= 2) && q.anyAfter(rbits - 2);
			if (roundBit && (sticky || result.test(0))) {
				bool carry = true;
				for (unsigned j = 0; j < nbits && carry; ++j) {
					carry = result.test(j);
					result.setbit(j, !carry);
				}
				if (carry) return saturate();              // the rounding reached 2^nbits
			}
		}
		// the magnitude is below 2^nbits; at or above 2^(nbits - 1) it is past maxpos, and past maxneg
		// unless it is exactly 2^(nbits - 1), which is maxneg
		if (result.test(nbits - 1)) {
			if (!negative) return saturate();
			for (unsigned j = 0; j + 1 < nbits; ++j) {
				if (result.test(j)) return saturate();
			}
			return result;                                 // 100...0 is maxneg as it stands
		}
		if (negative) result.twosComplement();
		return result;
	}
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
/// Accepts any quire parameterization (capacity, limb type) over the same scalar.
template<typename Scalar, unsigned capacity, typename LimbType, typename Vector>
void fdp_qc(quire<Scalar, capacity, LimbType>& sum_of_products, size_t n,
            const Vector& x, size_t incx,
            const Vector& y, size_t incy,
            std::enable_if_t<is_fixpnt<Scalar> &&
                             std::is_same_v<typename Vector::value_type, Scalar>, int> = 0) {
	if (n == 0) return;
	if (incx == 0 || incy == 0)
		throw_or_abort(std::invalid_argument("fdp_qc: incx and incy must be positive"));
	size_t ix, iy;
	for (ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		if (ix >= x.size() || iy >= y.size())
			throw_or_abort(std::out_of_range("fdp_qc: index out of bounds"));
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
	if (incx == 0 || incy == 0)
		throw_or_abort(std::invalid_argument("fdp_stride: incx and incy must be positive"));
	size_t ix, iy;
	for (ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		if (ix >= x.size() || iy >= y.size())
			throw_or_abort(std::out_of_range("fdp_stride: index out of bounds"));
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
	if (n > size(y))
		throw_or_abort(std::invalid_argument("fdp: y vector must be at least as long as x"));
	for (size_t i = 0; i < n; ++i) {
		q += quire_mul(x[i], y[i]);
	}

	return quire_resolve(q);
}

}} // namespace sw::universal
