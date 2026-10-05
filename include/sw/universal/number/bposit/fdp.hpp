#pragma once
// fdp.hpp: fused dot product for the bounded posit, through the generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Exact accumulation is the quire's job for bposit: bounding the taper does not make
// TwoSum or TwoProduct exact (docs/number-systems/bposit.md).  quire_traits sizes the
// accumulator from (nbits, rs, es) so that every product of two bposits lands in it
// exactly; quire_mul forms the unrounded product, and quire_resolve rounds the sum once,
// through bposit's own encoder -- so a tiny or huge result saturates to +/-minpos or
// +/-maxpos as the type does, rather than to zero.
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <universal/number/quire/quire.hpp>   // generalized quire

namespace sw { namespace universal {

// the exact, unrounded product of two bposits
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
blocktriple<bposit<nbits, rs, es, bt>::fbits, BlockTripleOperator::MUL, bt>
quire_mul(const bposit<nbits, rs, es, bt>& lhs, const bposit<nbits, rs, es, bt>& rhs) {
	using B = bposit<nbits, rs, es, bt>;
	blocktriple<B::fbits, BlockTripleOperator::MUL, bt> a, b, product;
	if (lhs.isnar() || rhs.isnar()) { product.setnan(); return product; }
	if (lhs.iszero() || rhs.iszero()) { product.setzero(); return product; }
	lhs.normalize(a);
	rhs.normalize(b);
	product.mul(a, b);
	return product;
}

// round the quire's exact sum into a bposit, once
template<unsigned nbits, unsigned rs, unsigned es, typename bt, unsigned capacity, typename LimbType>
bposit<nbits, rs, es, bt> quire_resolve(const quire<bposit<nbits, rs, es, bt>, capacity, LimbType>& q) {
	using B = bposit<nbits, rs, es, bt>;
	B result;
	if (q.isnan())  { result.setnar(); return result; }
	if (q.iszero()) { result.setzero(); return result; }
	constexpr unsigned rp = quire_traits<B>::radix_point;
	const int s = q.scale();                                    // the sum's binary scale
	const int msb = s + static_cast<int>(rp);                    // its leading bit in the accumulator
	constexpr unsigned qf = B::fbits + 2u;                       // every fraction bit, a guard and one more
	std::uint64_t frac = 0;
	for (unsigned i = 0; i < qf; ++i) {
		const int bit = msb - 1 - static_cast<int>(i);
		frac = (frac << 1) | ((bit >= 0 && q[static_cast<unsigned>(bit)]) ? 1ull : 0ull);
	}
	const int below = msb - 1 - static_cast<int>(qf);
	const bool sticky = (below >= 0) && q.anyAfter(static_cast<unsigned>(below));
	result.setbits(B::encode(q.sign(), s, frac, qf, sticky));
	return result;
}

// the accumulator width, sign excluded
template<unsigned nbits, unsigned rs, unsigned es, unsigned capacity = 30>
constexpr unsigned bposit_quire_size() {
	return quire_traits<bposit<nbits, rs, es, std::uint8_t>>::range + capacity;
}

// fused dot product into a caller-supplied quire, with strides
template<typename Qy, typename Vector>
void fdp_qc(Qy& sum_of_products, std::size_t n, const Vector& x, std::size_t incx, const Vector& y, std::size_t incy,
            std::enable_if_t<is_bposit<typename Vector::value_type>, int> = 0) {
	assert(incx > 0 && incy > 0 && "fdp_qc: strides must be positive");
	for (std::size_t ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		assert(ix < x.size() && iy < y.size() && "fdp_qc: index out of bounds");
		sum_of_products += quire_mul(x[ix], y[iy]);
	}
}

// fused dot product with strides, rounded once
template<typename Vector>
enable_if_bposit<typename Vector::value_type, typename Vector::value_type>
fdp_stride(std::size_t n, const Vector& x, std::size_t incx, const Vector& y, std::size_t incy) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;
	assert(incx > 0 && incy > 0 && "fdp_stride: strides must be positive");
	for (std::size_t ix = 0, iy = 0; ix < n && iy < n; ix += incx, iy += incy) {
		assert(ix < x.size() && iy < y.size() && "fdp_stride: index out of bounds");
		q += quire_mul(x[ix], y[iy]);
	}
	return quire_resolve(q);
}

// fused dot product, rounded once
template<typename Vector>
enable_if_bposit<typename Vector::value_type, typename Vector::value_type>
fdp(const Vector& x, const Vector& y) {
	using Scalar = typename Vector::value_type;
	quire<Scalar> q;
	const std::size_t n = x.size();
	assert(n <= y.size() && "fdp: y must be at least as long as x");
	for (std::size_t i = 0; i < n; ++i) q += quire_mul(x[i], y[i]);
	return quire_resolve(q);
}

}} // namespace sw::universal
