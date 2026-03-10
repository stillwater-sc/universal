#pragma once
// fdp.hpp: fused dot product and quire accumulation support for dbns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// DBNS stores values as (-1)^sign * 2^a * 3^b using two exponent fields.
// Multiplication is addition of both exponent pairs, but quire accumulation
// requires linear-domain values.
//
// quire_mul converts both dbns operands to double, multiplies, then packs
// the product into a blocktriple for quire accumulation. This is exact
// for dbns configurations where the product fits within double precision.
//
// Relates to #345, #549

#include <vector>
#include <cmath>
#include <stdexcept>
#include <universal/number/dbns/dbns_traits.hpp>
#include <universal/number/quire/quire.hpp>

namespace sw { namespace universal {

// ============================================================================
// quire_mul: unrounded full-precision product for quire accumulation
//
// Converts both dbns operands to double, multiplies, then decomposes the
// product into a blocktriple<product_fbits, REP, bt> for quire accumulation.
// ============================================================================
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
blocktriple<2*fbbits, BlockTripleOperator::REP, bt>
quire_mul(const dbns<nbits, fbbits, bt, xtra...>& lhs,
          const dbns<nbits, fbbits, bt, xtra...>& rhs) {
	constexpr unsigned product_fbits = 2 * fbbits;
	using ResultBT = blocktriple<product_fbits, BlockTripleOperator::REP, bt>;
	constexpr unsigned bt_radix = ResultBT::radix;  // = product_fbits
	constexpr unsigned bfbits = ResultBT::bfbits;    // = product_fbits + 2

	ResultBT product;

	if (lhs.iszero() || rhs.iszero()) return product;
	if (lhs.isnan() || rhs.isnan()) {
		product.setnan();
		return product;
	}

	double lhs_d = double(lhs);
	double rhs_d = double(rhs);
	double product_d = lhs_d * rhs_d;

	if (product_d == 0.0) return product;

	bool product_sign = (product_d < 0.0);
	double abs_val = std::abs(product_d);

	int exponent;
	double mantissa = std::frexp(abs_val, &exponent);
	int scale = exponent - 1;
	double significand = mantissa * 2.0;  // [1.0, 2.0)

	product.setnormal();
	product.setsign(product_sign);
	product.setscale(scale);

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
// quire_resolve: extract a dbns value from a quire
// ============================================================================
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra,
         unsigned capacity, typename LimbType>
dbns<nbits, fbbits, bt, xtra...>
quire_resolve(const quire<dbns<nbits, fbbits, bt, xtra...>, capacity, LimbType>& q) {
	using Scalar = dbns<nbits, fbbits, bt, xtra...>;

	if (q.iszero()) {
		Scalar result;
		result.setzero();
		return result;
	}

	double value = q.template convert_to<double>();
	return Scalar(value);
}

// ============================================================================
// Fused dot product operators
// ============================================================================

/// Fused dot product with quire continuation.
template<typename Scalar, unsigned capacity, typename LimbType, typename Vector>
void fdp_qc(quire<Scalar, capacity, LimbType>& sum_of_products, size_t n,
            const Vector& x, size_t incx,
            const Vector& y, size_t incy,
            std::enable_if_t<is_dbns<Scalar> &&
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
enable_if_dbns<typename Vector::value_type>
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
enable_if_dbns<typename Vector::value_type>
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
