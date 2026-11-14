#pragma once
// atomic_fused_operators.hpp: fused multiply-add, add-multiply and multiply-multiply-add for posits,
// each computed exactly and rounded once
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fma(a, b, c) = a*b + c, fam(a, b, c) = (a + b)*c and fmma(a, b, c, d) = a*b +/- c*d are evaluated on
// 64-bit limbs (#2071). Each posit enters as its exact significand and scale (posit_limbs::decode),
// products are exact, and sums are exact too, except that an addend too small to move the result
// across any rounding boundary is replaced by one sticky unit below all of them. The result is then
// rounded once (posit_limbs::encode).
//
// These replaced a blocktriple chain that was not correctly rounded for fam: a + b was formed in an
// ADD blocktriple of the operands' width and moved into a MUL blocktriple of fbits + 3 bits with no
// sticky, so posit<6,1> fam(0.09375, 2.5, 0.4375) was 1 instead of 1.25. The chain also sized its
// blocktriples nbits - 3 - es, which wraps for es + 2 >= nbits, so posit<5,3> did not compile.
//
// Why the collapse is safe. Let S = H + L, with H the addend of larger magnitude, p its significand
// bits, and t <= fbits + 1 the posit's precision anywhere. The rounding boundaries near H (the
// representable values and the midpoints between them, also one binade lower when L is negative and
// H a power of two) are at least 2^min(lsb(H), msb(H) - fbits - 3) apart from H unless H is one. With
// cut = min(lsb(H) - extra, msb(H) - fbits - 4) - 2, an L below 2^cut and its replacement, a unit at
// 2^(cut - 1) of L's sign, lie strictly on the same side of every such boundary. For fam the sum is
// multiplied by c before rounding; extra = fbits keeps |L c| below the boundary spacing of H c.

#include <cstddef>
#include <cstdint>
#include <universal/number/posit/posit_limbs.hpp>

namespace sw { namespace universal {

namespace posit_fused {

	using limb_t = limb_kernels::limb_t;

	// a value mag * 2^e, with a sign; mag has N limbs
	template<std::size_t N>
	struct term {
		bool         zero;
		bool         negative;
		std::int64_t e;
		limb_t       mag[N];
	};

	// the number of significant bits of a[0, n), 0 when a is zero
	template<std::size_t N>
	inline std::int64_t bitlength(const limb_t* a) noexcept {
		for (std::size_t i = N; i-- > 0;) {
			if (a[i] != 0u) {
				unsigned b = 64u;
				while (((a[i] >> (b - 1u)) & 1u) == 0u) --b;
				return static_cast<std::int64_t>(64u * i + b);
			}
		}
		return 0;
	}

	// the position of the lowest set bit of a nonzero a[0, n)
	template<std::size_t N>
	inline std::int64_t lowest_bit(const limb_t* a) noexcept {
		for (std::size_t i = 0; i < N; ++i) {
			if (a[i] != 0u) {
				unsigned b = 0;
				while (((a[i] >> b) & 1u) == 0u) ++b;
				return static_cast<std::int64_t>(64u * i + b);
			}
		}
		return 0;
	}

	template<std::size_t N> inline std::int64_t msb(const term<N>& t) noexcept { return t.e + bitlength<N>(t.mag) - 1; }
	template<std::size_t N> inline std::int64_t lsb(const term<N>& t) noexcept { return t.e + lowest_bit<N>(t.mag); }

	template<std::size_t N>
	inline term<N> zero_term() noexcept {
		term<N> t{};
		t.zero = true;
		return t;
	}

	// a finite posit, exactly: its significand with the hidden bit, and the weight of its last bit
	template<std::size_t N, unsigned nbits, unsigned es, typename bt>
	inline term<N> from_posit(const posit<nbits, es, bt>& p) noexcept {
		if (p.iszero()) return zero_term<N>();
		constexpr std::size_t L = posit_limbs::limbs(nbits);
		static_assert(L <= N, "posit_fused: a term holds at least the posit's encoding");
		limb_t w[L];
		posit_limbs::gather(p, w);
		term<N> t{};
		t.negative = p.isneg();
		if (t.negative) posit_limbs::negate<nbits>(w);
		const posit_limbs::Fields f = posit_limbs::decode<nbits, es>(w);   // w = the fraction bits
		posit_limbs::set(w, f.nf);                                          // and the hidden bit
		for (std::size_t i = 0; i < L; ++i) t.mag[i] = w[i];
		t.e = static_cast<std::int64_t>(f.scale) - static_cast<std::int64_t>(f.nf);
		return t;
	}

	// x * y, exactly; the caller sizes N for the product
	template<std::size_t N>
	inline term<N> mul(const term<N>& x, const term<N>& y) noexcept {
		if (x.zero || y.zero) return zero_term<N>();
		const std::size_t m = static_cast<std::size_t>((bitlength<N>(x.mag) + 63) / 64);
		const std::size_t n = static_cast<std::size_t>((bitlength<N>(y.mag) + 63) / 64);
		limb_t r[2 * N]{};
		limb_kernels::mul_basecase(r, x.mag, m, y.mag, n);
		term<N> t{};
		for (std::size_t i = 0; i < N; ++i) t.mag[i] = r[i];   // m + n limbs fit N by the caller's sizing
		t.negative = (x.negative != y.negative);
		t.e = x.e + y.e;
		return t;
	}

	// x + y, exact up to the collapse described at the top; extra widens the margin for a product
	// that follows (fam)
	template<std::size_t N>
	inline term<N> add(const term<N>& x, const term<N>& y, std::int64_t fbits, std::int64_t extra) noexcept {
		if (x.zero) return y;
		if (y.zero) return x;
		const bool xLarger = msb(x) >= msb(y);
		const term<N>& H = xLarger ? x : y;
		term<N> L = xLarger ? y : x;
		const std::int64_t cutA = lsb(H) - extra;
		const std::int64_t cutB = msb(H) - fbits - 4;
		const std::int64_t cut = ((cutA < cutB) ? cutA : cutB) - 2;
		if (msb(L) < cut) {   // too small to cross a boundary: one sticky unit at 2^(cut - 1)
			for (std::size_t i = 0; i < N; ++i) L.mag[i] = 0u;
			L.mag[0] = 1u;
			L.e = cut - 1;
		}
		const std::int64_t e0 = (H.e < L.e) ? H.e : L.e;
		limb_t a[N], b[N];
		limb_kernels::load_window(a, N, H.mag, N, H.e - e0);
		limb_kernels::load_window(b, N, L.mag, N, L.e - e0);
		term<N> t{};
		t.e = e0;
		if (H.negative == L.negative) {
			limb_kernels::add_n(t.mag, a, b, N);
			t.negative = H.negative;
		}
		else if (limb_kernels::geq_n(a, b, N)) {
			limb_kernels::sub_n(t.mag, a, b, N);
			t.negative = H.negative;
		}
		else {
			limb_kernels::sub_n(t.mag, b, a, N);
			t.negative = L.negative;
		}
		t.zero = (bitlength<N>(t.mag) == 0);   // exact cancellation: the single posit zero
		return t;
	}

	// the term, rounded once into the posit
	template<std::size_t N, unsigned nbits, unsigned es, typename bt>
	inline posit<nbits, es, bt> round(const term<N>& t) noexcept {
		posit<nbits, es, bt> p;
		if (t.zero) { p.setzero(); return p; }
		const std::int64_t bits = bitlength<N>(t.mag);
		limb_t sig[N];   // the leading bit at the top of the window
		limb_kernels::load_window(sig, N, t.mag, N, static_cast<std::int64_t>(64u * N) - bits);
		const std::int64_t scale = t.e + bits - 1;
		posit_limbs::encode(p, static_cast<int>(scale), sig, N, false, t.negative);
		return p;
	}

	// limbs for every intermediate: a sum of two products, or a sum times a posit, stays below
	// 4 fbits + 16 bits after the collapse; the margin covers the posit's own encoding
	template<unsigned nbits, unsigned es, typename bt>
	constexpr std::size_t capacity() noexcept {
		constexpr unsigned F = posit<nbits, es, bt>::fbits;
		constexpr std::size_t widest = posit_limbs::limbs(5u * F + 192u);
		constexpr std::size_t encoding = posit_limbs::limbs(nbits);
		return (widest > encoding) ? widest : encoding;
	}

}  // namespace posit_fused

///////////////////////////////////////////////////////////////////////////////
// FMA: fused multiply-add, a*b + c, rounded once

template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fma(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b,
                         const posit<nbits, es, bt>& c) {
	using namespace posit_fused;
	constexpr std::size_t N = capacity<nbits, es, bt>();
	constexpr std::int64_t F = posit<nbits, es, bt>::fbits;
	if (a.isnar() || b.isnar() || c.isnar()) { posit<nbits, es, bt> r; r.setnar(); return r; }
	const term<N> product = mul(from_posit<N>(a), from_posit<N>(b));
	return round<N, nbits, es, bt>(add(product, from_posit<N>(c), F, 0));
}

///////////////////////////////////////////////////////////////////////////////
// FAM: fused add-multiply, (a + b)*c, rounded once

template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fam(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b,
                         const posit<nbits, es, bt>& c) {
	using namespace posit_fused;
	constexpr std::size_t N = capacity<nbits, es, bt>();
	constexpr std::int64_t F = posit<nbits, es, bt>::fbits;
	if (a.isnar() || b.isnar() || c.isnar()) { posit<nbits, es, bt> r; r.setnar(); return r; }
	const term<N> sum = add(from_posit<N>(a), from_posit<N>(b), F, F);
	return round<N, nbits, es, bt>(mul(sum, from_posit<N>(c)));
}

///////////////////////////////////////////////////////////////////////////////
// FMMA: fused multiply-multiply-add, a*b + c*d, or a*b - c*d when opIsAdd is false, rounded once

template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fmma(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b,
                          const posit<nbits, es, bt>& c, const posit<nbits, es, bt>& d,
                          bool opIsAdd = true) {
	using namespace posit_fused;
	constexpr std::size_t N = capacity<nbits, es, bt>();
	constexpr std::int64_t F = posit<nbits, es, bt>::fbits;
	if (a.isnar() || b.isnar() || c.isnar() || d.isnar()) { posit<nbits, es, bt> r; r.setnar(); return r; }
	const term<N> ab = mul(from_posit<N>(a), from_posit<N>(b));
	term<N> cd = mul(from_posit<N>(c), from_posit<N>(d));
	if (!opIsAdd) cd.negative = !cd.negative;
	return round<N, nbits, es, bt>(add(ab, cd, F, 0));
}

}} // namespace sw::universal
