#pragma once
// lattice.hpp: the exact values of unum Type I, and the tile that holds an exact result
//
// The exact values of unum<esizesize, fsizesize> form an IEEE-like lattice: maxfsize + 1
// significand bits, exponents in [2 - 2^esmax, 2^esmax] with esmax = 2^esizesize - 1, and
// subnormals below.  For esizesize <= 3 and fsizesize <= 5 every lattice point is a double,
// and sums, products and quotients of two stay finite and normal (fits_double).
//
// A result is computed in double, and an error-free transform gives the sign of that
// double's rounding error (#1689).  The tile that holds the exact result follows: the
// lattice point itself when the double is that point and the error is zero, otherwise the
// open tile between the two lattice points around the exact value.  An inexact unum
// (ubit = 1) is the open tile between its exact value and the next one away from zero, so
// a scalar unum names a tile, and ubound joins two tiles into an interval.
//
// Everything here is pure arithmetic or bit extraction, so the scalar unum operators that
// use it stay constant-evaluable.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>      // std::sqrt in unum_eft::sqrt
#include <cstdint>

#include <universal/native/extract_fields.hpp>
#include <math/constexpr_math/exp2.hpp>

namespace sw { namespace universal {

// the lattice of exact values of unum<esizesize, fsizesize>, as doubles
template<unsigned esizesize, unsigned fsizesize>
struct unum_lattice {
	static constexpr int maxfs = (1 << fsizesize) - 1;           // fraction bits at the finest fsize
	static constexpr int esmax = (1 << esizesize) - 1;           // largest esize field value
	// largest and smallest normal binade.  2^esmax overflows int for esizesize >= 5; such a
	// lattice is far outside double, so the limits saturate and fits_double is false
	static constexpr int emax  = esmax < 30 ? (1 << esmax) : (1 << 30);
	static constexpr int emin  = 2 - emax;
	// every lattice point is a double, and sums and products of two stay finite and normal
	static constexpr bool fits_double = (maxfs <= 52) && (emax <= 256) && (emin - maxfs >= -512);

	// 2^n, exact for every n a lattice computation needs
	static constexpr double pow2(int n) noexcept { return sw::math::constexpr_math::detail::pow2(n); }

	// the binade of a finite a > 0: 2^e <= a < 2^(e + 1)
	static CONSTEXPRESSION int ilogb(double a) noexcept {
		bool s = false;
		std::uint64_t rawExp = 0, fraction = 0, bits = 0;
		extractFields(a, s, rawExp, fraction, bits);
		if (rawExp != 0) return static_cast<int>(rawExp) - 1023;
		int msb = 51;                                            // a subnormal double
		while (msb > 0 && ((fraction >> msb) & 1u) == 0) --msb;
		return msb - 1074;
	}

	static constexpr double maxpos() noexcept { return (2.0 - pow2(-maxfs)) * pow2(emax); }

	// the spacing of the lattice at a >= 0 (the spacing just above a at a binade boundary)
	static CONSTEXPRESSION double spacing(double a) noexcept {
		const int e = (a == 0.0) ? emin : (ilogb(a) > emin ? ilogb(a) : emin);
		return pow2(e - maxfs);
	}
	// the largest lattice point <= a, and the smallest >= a; a is finite and |a| <= maxpos.
	// a / g is exact and below 2^(maxfs + 2), so the integer cast is the floor.
	static CONSTEXPRESSION double floor(double a) noexcept {
		if (a < 0.0) return -ceil(-a);
		const double g = spacing(a);
		return static_cast<double>(static_cast<std::uint64_t>(a / g)) * g;
	}
	static CONSTEXPRESSION double ceil(double a) noexcept {
		if (a < 0.0) return -floor(-a);
		const double g = spacing(a);
		const double q = a / g;
		const double t = static_cast<double>(static_cast<std::uint64_t>(q));
		return (t < q ? t + 1.0 : t) * g;
	}
	// the neighbours of a lattice point v
	static CONSTEXPRESSION double next(double v) noexcept {
		if (v < 0.0) return -prev(-v);
		return v + spacing(v);
	}
	static CONSTEXPRESSION double prev(double v) noexcept {
		if (v <= 0.0) return -next(-v);
		// at the bottom of a binade above emin the spacing below is half the spacing above
		const int e = ilogb(v);
		const bool binade_bottom = (e > emin) && (pow2(e) == v);
		return v - (binade_bottom ? pow2(e - 1 - maxfs) : spacing(v));
	}
};

namespace unum_eft {

// a double result d and the sign of the exact value's offset from it: T = d + delta, with
// sign(delta) = dir.  Each comes from an error-free transform, so dir is exact.
struct directed { double d; int dir; };

constexpr int sgn(double x) noexcept { return (x > 0.0) - (x < 0.0); }

// Knuth TwoSum: a + b = s + e exactly
constexpr directed add(double a, double b) noexcept {
	const double s = a + b;
	const double bb = s - a;
	const double e = (a - (s - bb)) + (b - bb);
	return { s, sgn(e) };
}

// Veltkamp split and Dekker TwoProduct: a * b = p + e exactly (no fma needed)
constexpr void split(double a, double& hi, double& lo) noexcept {
	const double c = 134217729.0 * a;   // 2^27 + 1
	hi = c - (c - a);
	lo = a - hi;
}
constexpr void two_prod(double a, double b, double& p, double& e) noexcept {
	p = a * b;
	double ah = 0.0, al = 0.0, bh = 0.0, bl = 0.0;
	split(a, ah, al);
	split(b, bh, bl);
	e = ((ah * bh - p) + ah * bl + al * bh) + al * bl;
}
constexpr directed mul(double a, double b) noexcept {
	double p = 0.0, e = 0.0;
	two_prod(a, b, p, e);
	return { p, sgn(e) };
}

// a / b: the residual a - q b has the sign of (T - q) * b, and a - p is exact (Sterbenz)
constexpr directed div(double a, double b) noexcept {
	const double q = a / b;
	double p = 0.0, e = 0.0;
	two_prod(q, b, p, e);
	const double r = (a - p) - e;
	return { q, sgn(r) * sgn(b) };
}

// sqrt(x): the residual x - s^2 has the sign of T - s
inline directed sqrt(double x) noexcept {
	const double s = std::sqrt(x);
	double p = 0.0, e = 0.0;
	two_prod(s, s, p, e);
	const double r = (x - p) - e;
	return { s, sgn(r) };
}

}  // namespace unum_eft

// The tiles of the lattice, as a scalar unum names them.  A tile is a lattice point t
// (ubit clear), or the open tile between t and its neighbour away from zero (ubit set);
// for t = 0 the side is given by neg.  (maxpos, inf) is the open tile at maxpos.
template<unsigned esizesize, unsigned fsizesize>
struct unum_tiles {
	using L = unum_lattice<esizesize, fsizesize>;
	using directed = unum_eft::directed;

	struct tile {
		double t; bool neg; bool ubit;
		constexpr bool operator==(const tile& o) const noexcept { return t == o.t && neg == o.neg && ubit == o.ubit; }
	};

	// an end of an interval: a value that is a lattice point or a directed result, excluded
	// when open, or an infinity (inf = -1, +1)
	struct bound { double d; int dir; bool open; int inf; };

	// the tile that holds the exact value d + delta, sign(delta) = dir
	static CONSTEXPRESSION tile tile_of(double d, int dir) noexcept {
		if (d == 0.0) return dir == 0 ? tile{ 0.0, false, false } : tile{ 0.0, dir < 0, true };
		const bool s = d < 0.0;
		const double m = s ? -d : d;
		const int mdir = s ? -dir : dir;                         // the offset away from zero
		const double M = L::maxpos();
		auto signed_tile = [s](double v, bool ubit) { return tile{ s ? -v : v, s, ubit }; };
		if (m > M || (m == M && mdir > 0)) return signed_tile(M, true);
		const double t = L::floor(m);
		if (t != m) return signed_tile(t, true);
		if (mdir == 0) return signed_tile(m, false);
		if (mdir > 0) return signed_tile(m, true);
		return signed_tile(L::prev(m), true);
	}

	// the tile just inside a lower (upper) bound: the one that holds the values right above
	// (below) it.  A set that reaches -inf has no lower tile a unum can name.
	static CONSTEXPRESSION bool lower_tile(const bound& b, tile& r) noexcept {
		if (b.inf < 0) return false;
		if (b.inf > 0) { r = tile{ L::maxpos(), false, true }; return true; }
		r = tile_of(b.d, b.dir != 0 ? b.dir : (b.open ? +1 : 0));
		return true;
	}
	static CONSTEXPRESSION bool upper_tile(const bound& b, tile& r) noexcept {
		if (b.inf > 0) { r = tile{ L::maxpos(), false, true }; return true; }
		if (b.inf < 0) return false;
		r = tile_of(b.d, b.dir != 0 ? b.dir : (b.open ? -1 : 0));
		return true;
	}
};

}}  // namespace sw::universal
