#pragma once
// value_domain.hpp: the value-domain evaluation of the lindex math library (#1715)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The functions of #1715 whose results are bounded, or follow X closely, are evaluated on the
// value X = phi(x) rather than on its psi point, in two ways:
//
//   round_value   a signed value in the working type W, rounded once like round_psi: double,
//                 then dd_cascade, then qd_cascade while the result lies within the stage's
//                 band around a midpoint.  For functions whose error in W stays near W's own:
//                 the inverse trigonometric, hyperbolic, error and gamma functions.
//
//   round_exact   a signed efloat value at a precision chosen for X, evaluated twice, 32 bits
//                 apart, and widened until both round to the same encoding with a margin
//                 (Ziv's strategy).  For sin, cos, tan, fmod, remainder and the reflection of
//                 the gamma functions, whose absolute error grows with X: sin of an X near
//                 2^1024 needs X to 1024 + fbits bits, and no cascade holds that many.
//
// Past the double range (psi > psi(DBL_MAX) = 4.632) those last functions are NaR by design:
// phi(5) already has 1.6 million decimal digits in its exponent, so X mod 2 pi, or X mod Y,
// is not determined by anything the format can compute.
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>
#include <universal/number/lindex/math/constants.hpp>
// the working types' functions, dd_cascade and qd_cascade at full precision: their umbrellas,
// since the math library needs abs from attributes.hpp; the lindex math library is not part of
// the I/O-free core
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/number/efloat/core.hpp>
#include <universal/number/efloat/math/exponent.hpp>
#include <universal/number/efloat/math/logarithm.hpp>
#include <universal/number/efloat/math/truncate.hpp>
#include <universal/number/efloat/math/trigonometry.hpp>

namespace sw { namespace universal { namespace lindex_math {

// the multi-precision type of the exact path; every value it holds has its precision set
using exact_float = efloat<2>;

// pi in the working type W
template<typename W>
W pi() noexcept {
	if constexpr (std::is_same_v<W, double>) return 3.14159265358979323846;
	else if constexpr (std::is_same_v<W, dd_cascade>) return ddc_pi;
	else return qdc_pi;
}

// whether the magnitude encoding lies past the double range, psi(|X|) > psi(DBL_MAX)
template<typename L>
bool past_double_range(std::uint64_t mag) noexcept {
	static const level_index::point<qd_cascade> edge = level_index::psi(qd_cascade(std::numeric_limits<double>::max()));
	return L::template point_less<qd_cascade>(edge, L::template point_of<qd_cascade>(mag));
}

// the lindex of a magnitude encoding, negated when negative
template<typename L>
L from_magnitude(std::uint64_t mag, bool negative) noexcept {
	L r;
	r.setbits((negative && mag != 0ull) ? (((~mag) + 1ull) & L::nbits_mask) : mag);
	return r;
}

// ------------------------------------------------------------------ round_value

// The lindex nearest the signed value eval(W) returns, rounded once.  eval is evaluated in
// the arithmetic working type, then dd_cascade, then qd_cascade, while the result lies within
// the stage's band around a midpoint.  A value whose magnitude rounds to 0 is zero, whatever
// its sign.
template<typename L, typename Eval>
L round_value(Eval&& eval) noexcept {
	auto stage = [&](auto w, bool& near_tie, bool& negative) {
		using W = decltype(w);
		W v = eval(w);
		negative = v < W(0);
		if (negative) v = -v;
		near_tie = false;
		if (v == W(0)) return std::uint64_t(0);
		return L::round_outcome(L::template from_point<W>(level_index::psi(v)), near_tie);
	};
	bool near_tie = false, negative = false;
	std::uint64_t mag = stage(typename L::ArithmeticWorking{}, near_tie, negative);
	if constexpr (std::is_same_v<typename L::ArithmeticWorking, double>) {
		if (near_tie) mag = stage(dd_cascade{}, near_tie, negative);
	}
	if (near_tie) mag = stage(qd_cascade{}, near_tie, negative);
	return from_magnitude<L>(mag, negative);
}

// ------------------------------------------------------------------ the exact path

// the value of an exact_float holding an integer in [0, 2^63)
inline std::uint64_t integer_of(const exact_float& v) noexcept {
	if (v.iszero()) return 0;
	const std::int64_t s = v.scale();   // the leading bit weighs 2^s
	const std::vector<std::uint32_t> limbs = v.bits();
	std::uint64_t r = 0;
	for (std::size_t i = 0; i < limbs.size(); ++i) {
		const std::int64_t shift = s - 31 - 32 * static_cast<std::int64_t>(i);   // weight of the limb's lsb
		if (shift >= 64 || shift <= -32) continue;
		const std::uint64_t limb = limbs[i];
		r |= (shift >= 0) ? (limb << shift) : (limb >> (-shift));
	}
	return r;
}

// X = phi(x) of the magnitude encoding mag, relative to `bits` bits.  X = phi(l + f) is f
// raised through l exponentials; the value below each needs as many more bits as its own
// magnitude, since e^w carries w's absolute error as relative error.  For X within the double
// range only.
template<typename L>
exact_float phi_value(std::uint64_t mag, unsigned bits) {
	const int l = static_cast<int>(mag >> L::fbits);
	exact_float v(static_cast<unsigned long long>(mag & L::index_mask));
	if (!v.iszero()) v.setexponent(v.scale() - static_cast<std::int64_t>(L::fbits));
	double w = std::ldexp(static_cast<double>(mag & L::index_mask), -static_cast<int>(L::fbits));
	unsigned need[level_index::max_level + 1];
	double magnitude[level_index::max_level + 1];
	for (int k = 0; k <= l; ++k) {
		magnitude[k] = w;
		w = std::exp(w);
	}
	need[l] = bits;
	for (int k = l - 1; k >= 0; --k) {
		const unsigned grow = (magnitude[k] > 1.0) ? static_cast<unsigned>(std::ceil(std::log2(magnitude[k]))) : 0u;
		need[k] = need[k + 1] + grow + 2u;
	}
	v.set_precision(need[0]);
	for (int k = 1; k <= l; ++k) {
		v = exp(v);
		v.set_precision(need[k]);
	}
	return v;
}

// a nonnegative efloat at `bits` bits from a qd_cascade, exactly: its four limbs
inline exact_float from_qd(const qd_cascade& q, unsigned bits) {
	exact_float v(q[0]);
	v.set_precision(bits);
	for (unsigned i = 1; i < 4; ++i) v += exact_float(q[i]);
	return v;
}

// a qd_cascade from an efloat within the double range: four rounded limbs
inline qd_cascade to_qd(exact_float v) {
	double limb[4];
	for (unsigned i = 0; i < 4; ++i) {
		limb[i] = double(v);
		v -= exact_float(limb[i]);
	}
	return qd_cascade(limb[0]) + qd_cascade(limb[1]) + qd_cascade(limb[2]) + qd_cascade(limb[3]);
}

// the psi point of a value v > 0, in efloat
struct exact_point {
	int level;
	exact_float index;
};
inline exact_point psi_of(exact_float v) {
	int level = 0;
	const exact_float one(1);
	while (v >= one && level < level_index::max_level) {
		v = log(v);
		++level;
	}
	return { level, v };
}

// The lindex nearest the signed value eval(bits) returns, an efloat accurate to about 2^-bits
// relative to the scale eval chose for it.  Each attempt evaluates at bits and bits + 32 and
// accepts when both points round to the same encoding with four times their difference to
// spare from the midpoint; otherwise it widens.  An exact zero from both is zero.
template<typename L, typename Eval>
L round_exact(Eval&& eval, unsigned bits) {
	using E = exact_float;
	std::uint64_t mag = 0;
	bool negative = false;
	for (unsigned attempt = 0; attempt < 8; ++attempt, bits += 64u << attempt) {
		const E v1 = eval(bits), v2 = eval(bits + 32u);
		if (v1.iszero() && v2.iszero()) { mag = 0; negative = false; break; }
		if (v1.iszero() != v2.iszero() || v1.sign() != v2.sign()) continue;
		E a1(v1), a2(v2);
		a1.setsign(false);
		a2.setsign(false);
		const exact_point p1 = psi_of(a1), p2 = psi_of(a2);
		negative = v2.sign() == -1;
		if (p1.level != p2.level) continue;
		// the index on the 2^-fbits grid: t = index 2^fbits, q = floor(t), remainder in [0, 1)
		E t = p2.index;
		if (!t.iszero()) t.setexponent(t.scale() + static_cast<std::int64_t>(L::fbits));
		const E q = floor(t);
		const E rem = t - q;
		E off = rem - E(0.5);
		off.setsign(false);
		E spread = p2.index - p1.index;
		spread.setsign(false);
		if (!spread.iszero()) spread.setexponent(spread.scale() + static_cast<std::int64_t>(L::fbits) + 2);   // 4 x, in grid units
		std::uint64_t k = integer_of(q);
		const bool up = rem > E(0.5) || (rem == E(0.5) && (k & 1ull));
		if (up) ++k;
		std::uint64_t m = (static_cast<std::uint64_t>(p2.level) << L::fbits) + k;
		if (p2.level >= static_cast<int>(L::nr_levels) || m > L::maxpos_bits) m = L::maxpos_bits;
		mag = m;
		if (off > spread) break;   // an exact result on the grid has off = 1/2 > 0
	}
	return from_magnitude<L>(mag, negative);
}

}}}  // namespace sw::universal::lindex_math
