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
/// Accumulates all products in a quire, then resolves with a single rounding.
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
	assert(n <= size(y) && "fdp: y vector must be at least as long as x");
	for (size_t i = 0; i < n; ++i) {
		q += quire_mul(x[i], y[i]);
	}

	// single rounding step
	return q.template convert_to<Scalar>();
}

}} // namespace sw::universal
