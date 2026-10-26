#pragma once
// hyperbolic.hpp: sinh, cosh, tanh, asinh, acosh and atanh for lindex (#1715)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sinh and cosh of X >= 1 go one level down, at every level:
//     psi(cosh X) = 1 + psi(X - ln 2 + log1p(e^-2X)),   psi(sinh X) = 1 + psi(X - ln 2 + log1p(-e^-2X))
// The correction is below 2^-220 past X = 77, and from there X - ln 2 is a difference of psi
// points, so no value past the working type is formed.  asinh and acosh of a large X are
// ln X + ln 2 + O(X^-2), with ln X = phi(x - 1) the exact point one level down.  tanh
// saturates to 1, which is where the level-0 grid puts 1 - 2 e^-2X from X ~ 22 at fbits 60.
// acosh of X < 1 and atanh of |X| >= 1 are NaR: lindex has no infinity.
#include <universal/number/lindex/math/value_domain.hpp>

namespace sw { namespace universal {

namespace lindex_math {
// psi(sinh X) or psi(cosh X) for the magnitude mag >= 1, one level down
template<typename L, typename W>
typename L::template outcome<W> sinh_cosh_of(std::uint64_t mag, bool plus) {
	using std::exp;
	using std::log1p;
	const level_index::point<W> x = L::template point_of<W>(mag);
	const W X = level_index::phi(x);
	if (X < W(128)) {
		const W e = exp(W(-2) * X);
		const W w = X - ln2<W>() + (plus ? log1p(e) : log1p(-e));
		return L::template up_a_level<W>(level_index::psi(w));
	}
	return L::template up_a_level<W>(L::template sub_points<W>(x, level_index::point<W>{ 0, ln2<W>() }));
}

// psi(ln X + ln 2) for the magnitude mag of a large X: the asinh and acosh of X to O(X^-2)
template<typename L, typename W>
typename L::template outcome<W> ln_twice_of(std::uint64_t mag) {
	const level_index::point<W> x = L::template point_of<W>(mag);
	return L::template add_points<W>(level_index::point<W>{ x.level - 1, x.index }, level_index::point<W>{ 0, ln2<W>() });
}

// whether the magnitude mag is at least 2^120, past which the O(X^-2) terms are below 2^-230
template<typename L>
bool huge_for_inverse(std::uint64_t mag) noexcept {
	static const level_index::point<qd_cascade> edge = level_index::psi(qd_cascade(std::ldexp(1.0, 120)));
	return !L::template point_less<qd_cascade>(L::template point_of<qd_cascade>(mag), edge);
}
}  // namespace lindex_math

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> sinh(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	const bool negative = v.isneg();
	if (mag < L::one_bits) {
		return lindex_math::round_value<L>([&](auto w) {
			using W = decltype(w);
			using std::sinh;
			const W r = sinh(L::template index_in<W>(mag));
			return negative ? W(-r) : r;
		});
	}
	return L::round_psi([&](auto w) { return lindex_math::sinh_cosh_of<L, decltype(w)>(mag, false); }, negative);
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> cosh(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar()) return v;
	if (v.iszero()) return L(1);
	const std::uint64_t mag = v.magnitude_bits();
	if (mag < L::one_bits) {
		return lindex_math::round_value<L>([&](auto w) {
			using W = decltype(w);
			using std::cosh;
			return W(cosh(L::template index_in<W>(mag)));
		});
	}
	return L::round_psi([&](auto w) { return lindex_math::sinh_cosh_of<L, decltype(w)>(mag, true); }, false);
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> tanh(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	const bool negative = v.isneg();
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::exp;
		using std::expm1;
		const W X = level_index::phi(L::template point_of<W>(mag));
		// 1 - 2 e^-2X: 1 to every grid once 2 e^-2X is below 2^-200
		W r(1);
		if (X < W(128)) r = W(-expm1(W(-2) * X)) / (W(1) + exp(W(-2) * X));
		return negative ? W(-r) : r;
	});
}

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> asinh(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	const bool negative = v.isneg();
	if (lindex_math::huge_for_inverse<L>(mag)) {
		return L::round_psi([&](auto w) { return lindex_math::ln_twice_of<L, decltype(w)>(mag); }, negative);
	}
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::asinh;
		const W r = asinh(level_index::phi(L::template point_of<W>(mag)));
		return negative ? W(-r) : r;
	});
}

// acosh of X < 1 is NaR
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> acosh(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (v.isneg() || mag < L::one_bits) return L(SpecificValue::nar);
	if (mag == L::one_bits) return L(0);
	if (lindex_math::huge_for_inverse<L>(mag)) {
		return L::round_psi([&](auto w) { return lindex_math::ln_twice_of<L, decltype(w)>(mag); }, false);
	}
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::expm1;
		using std::log1p;
		using std::sqrt;
		const level_index::point<W> x = L::template point_of<W>(mag);
		// t = X - 1, exact of cancellation: e^f - 1 at level 1
		const W t = (x.level == 1) ? W(expm1(x.index)) : W(level_index::phi(x) - W(1));
		return W(log1p(t + sqrt(t * (t + W(2)))));
	});
}

// atanh of |X| >= 1 is NaR
template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> atanh(const lindex<nbits, lbits, bt>& v) {
	using L = lindex<nbits, lbits, bt>;
	if (v.isnar() || v.iszero()) return v;
	const std::uint64_t mag = v.magnitude_bits();
	if (mag >= L::one_bits) return L(SpecificValue::nar);
	const bool negative = v.isneg();
	return lindex_math::round_value<L>([&](auto w) {
		using W = decltype(w);
		using std::log1p;
		const W x = L::template index_in<W>(mag);   // exact
		const W r = W(0.5) * log1p(W(2) * x / (W(1) - x));
		return negative ? W(-r) : r;
	});
}

}}  // namespace sw::universal
