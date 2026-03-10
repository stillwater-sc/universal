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
// Relates to #345, #547

#include <vector>
#include <universal/traits/cfloat_traits.hpp>
#include <universal/number/quire/quire.hpp>

namespace sw { namespace universal {

// ============================================================================
// quire_mul: unrounded full-precision product for quire accumulation
//
// Returns a blocktriple<fbits, MUL> containing the exact product of two
// cfloat values. The product's significand has 2*fbits fraction bits,
// preserving all information for exact accumulation in the quire.
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
// Fused dot product operators
//
// fdp_qc       fused dot product with quire continuation
// fdp_stride   fused dot product with non-negative stride
// fdp          fused dot product of two vectors (unit stride)
// ============================================================================

/// Fused dot product with quire continuation.
/// Accumulates products into an existing quire without resolving.
template<typename Qy, typename Vector>
void fdp_qc(Qy& sum_of_products, size_t n,
            const Vector& x, size_t incx,
            const Vector& y, size_t incy,
            std::enable_if_t<is_cfloat<typename Vector::value_type>, int> = 0) {
	size_t ix, iy;
	for (ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		sum_of_products += quire_mul(x[ix], y[iy]);
	}
}

/// Resolved fused dot product with stride.
/// Accumulates all products in a quire, then resolves with a single rounding.
template<typename Vector>
enable_if_cfloat<typename Vector::value_type>
fdp_stride(size_t n, const Vector& x, size_t incx, const Vector& y, size_t incy) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;

	size_t ix, iy;
	for (ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		q += quire_mul(x[ix], y[iy]);
	}

	// single rounding step
	return q.template convert_to<Scalar>();
}

/// Resolved fused dot product with unit stride.
/// This is the primary FDP entry point for cfloat vectors.
template<typename Vector>
enable_if_cfloat<typename Vector::value_type>
fdp(const Vector& x, const Vector& y) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;

	size_t n = size(x);
	for (size_t i = 0; i < n; ++i) {
		q += quire_mul(x[i], y[i]);
	}

	// single rounding step
	return q.template convert_to<Scalar>();
}

}} // namespace sw::universal
