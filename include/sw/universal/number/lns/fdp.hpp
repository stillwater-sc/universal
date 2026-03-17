#pragma once
// fdp.hpp: fused dot product and quire accumulation support for lns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// LNS stores values as (-1)^sign * 2^exponent where the exponent is a
// fixed-point number. Multiplication in LNS is addition of exponents (exact),
// but quire accumulation requires linear-domain values.
//
// quire_mul converts two lns operands to double, multiplies exactly in double
// (since lns values are always exactly representable as powers of 2 to the
// precision of the exponent), then packs the product into a blocktriple
// for quire accumulation.
//
// This double-precision path is exact for lns types up to ~52 exponent
// fractional bits. For wider types, a native log-to-linear conversion
// using extended-precision arithmetic would be needed.
//
// Relates to #345, #549

#include <vector>
#include <cmath>
#include <stdexcept>
#include <universal/number/lns/lns_traits.hpp>
#include <universal/number/quire/quire.hpp>

namespace sw { namespace universal {

// ============================================================================
// quire_mul: unrounded full-precision product for quire accumulation
//
// Converts both lns operands to double, multiplies, then decomposes the
// product into a blocktriple<product_fbits, REP, bt> for quire accumulation.
//
// The REP blocktriple with fbits=product_fbits gives:
//   bfbits = product_fbits + 2
//   radix  = product_fbits
// which is sufficient to hold the double-precision product significand.
// ============================================================================
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
blocktriple<2*rbits, BlockTripleOperator::REP, bt>
quire_mul(const lns<nbits, rbits, bt, xtra...>& lhs,
          const lns<nbits, rbits, bt, xtra...>& rhs) {
	constexpr unsigned product_fbits = 2 * rbits;
	using ResultBT = blocktriple<product_fbits, BlockTripleOperator::REP, bt>;
	constexpr unsigned bt_radix = ResultBT::radix;  // = product_fbits
	constexpr unsigned bfbits = ResultBT::bfbits;    // = product_fbits + 2

	// Check whether the lns exponent range fits within binary64.
	// The lns max exponent is 2^(integer_bits) - 1 where
	// integer_bits = nbits - 1 - rbits. Product exponents can be up to
	// 2x the single-operand maximum.
	constexpr unsigned integer_bits = nbits - 1u - rbits;
	constexpr unsigned max_single_exp = (1u << integer_bits) - 1u;
	constexpr bool fits_in_double = (2u * max_single_exp <= 1023u);

	ResultBT product;

	if (lhs.iszero() || rhs.iszero()) return product;
	if (lhs.isnan() || rhs.isnan()) {
		product.setnan();
		return product;
	}

	bool product_sign = lhs.sign() ^ rhs.sign();
	int scale{0};
	double significand{0.0};

	if constexpr (fits_in_double) {
		// Fast path: convert to double, multiply, decompose
		double lhs_d = double(lhs);
		double rhs_d = double(rhs);
		double product_d = lhs_d * rhs_d;

		if (product_d == 0.0) return product;

		double abs_val = std::abs(product_d);
		int exponent;
		double mantissa = std::frexp(abs_val, &exponent);
		scale = exponent - 1;
		significand = mantissa * 2.0;  // [1.0, 2.0)
	}
	else {
		// Wide path: the product exponent exceeds binary64 range.
		// lns product = 2^(exp_a + exp_b). Compute the exponent sum
		// directly and extract scale + fractional significand.
		//
		// Extract signed fixed-point exponents from each operand.
		// The exponent field is (nbits-1) bits stored in 2's complement
		// with rbits fractional bits.
		double frac_a = std::ldexp(static_cast<double>(static_cast<unsigned long long>(lhs.fraction())), -static_cast<int>(rbits));
		double frac_b = std::ldexp(static_cast<double>(static_cast<unsigned long long>(rhs.fraction())), -static_cast<int>(rbits));
		double exp_a = double(lhs.scale()) + frac_a;
		double exp_b = double(rhs.scale()) + frac_b;
		double exp_sum = exp_a + exp_b;

		// Split into integer and fractional parts
		// scale = floor(exp_sum), frac in [0, 1)
		double int_part;
		double frac = std::modf(exp_sum, &int_part);
		if (frac < 0.0) { frac += 1.0; int_part -= 1.0; }
		scale = static_cast<int>(int_part);

		// significand = 2^frac, always in [1.0, 2.0) — never overflows
		significand = std::pow(2.0, frac);
	}

	product.setnormal();
	product.setsign(product_sign);
	product.setscale(scale);

	// Pack significand bits into blocktriple
	// The hidden bit (1.xxx) goes at position radix
	for (unsigned k = 0; k < bfbits && k <= bt_radix; ++k) {
		if (significand >= 1.0) {
			product.setbit(bt_radix - k, true);
			significand -= 1.0;
		}
		significand *= 2.0;
	}

	return product;
}

// ============================================================================
// quire_resolve: extract an lns value from a quire
//
// For lns the quire is a linear-domain accumulator, so we convert back
// to lns via the double path.
// ============================================================================
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra,
         unsigned capacity, typename LimbType>
lns<nbits, rbits, bt, xtra...>
quire_resolve(const quire<lns<nbits, rbits, bt, xtra...>, capacity, LimbType>& q) {
	using Scalar = lns<nbits, rbits, bt, xtra...>;

	if (q.iszero()) {
		Scalar result;
		result.setzero();
		return result;
	}

	// Convert quire accumulator to double, then assign to lns
	// The lns assignment operator handles log2 conversion
	double value = q.template convert_to<double>();
	return Scalar(value);
}

// ============================================================================
// Fused dot product operators
//
// fdp_qc       fused dot product with quire continuation
// fdp_stride   fused dot product with non-negative stride
// fdp          fused dot product of two vectors (unit stride)
// ============================================================================

/// Fused dot product with quire continuation.
/// Accepts any quire parameterization (capacity, limb type) over the same scalar.
template<typename Scalar, unsigned capacity, typename LimbType, typename Vector>
void fdp_qc(quire<Scalar, capacity, LimbType>& sum_of_products, size_t n,
            const Vector& x, size_t incx,
            const Vector& y, size_t incy,
            std::enable_if_t<is_lns<Scalar> &&
                             std::is_same_v<typename Vector::value_type, Scalar>, int> = 0) {
	if (n == 0) return;
	if (incx == 0 || incy == 0)
		throw std::invalid_argument("fdp_qc: incx and incy must be positive");
	for (size_t i = 0; i < n; ++i) {
		size_t ix = i * incx;
		size_t iy = i * incy;
		if (ix >= x.size() || iy >= y.size())
			throw std::out_of_range("fdp_qc: index out of bounds");
		sum_of_products += quire_mul(x[ix], y[iy]);
	}
}

/// Resolved fused dot product with stride.
template<typename Vector>
enable_if_lns<typename Vector::value_type>
fdp_stride(size_t n, const Vector& x, size_t incx, const Vector& y, size_t incy) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;

	if (n == 0) return Scalar(0);
	if (incx == 0 || incy == 0)
		throw std::invalid_argument("fdp_stride: incx and incy must be positive");
	for (size_t i = 0; i < n; ++i) {
		size_t ix = i * incx;
		size_t iy = i * incy;
		if (ix >= x.size() || iy >= y.size())
			throw std::out_of_range("fdp_stride: index out of bounds");
		q += quire_mul(x[ix], y[iy]);
	}

	return quire_resolve(q);
}

/// Resolved fused dot product with unit stride.
template<typename Vector>
enable_if_lns<typename Vector::value_type>
fdp(const Vector& x, const Vector& y) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;

	size_t n = size(x);
	if (n > size(y))
		throw std::invalid_argument("fdp: y vector must be at least as long as x");
	for (size_t i = 0; i < n; ++i) {
		q += quire_mul(x[i], y[i]);
	}

	return quire_resolve(q);
}

}} // namespace sw::universal
