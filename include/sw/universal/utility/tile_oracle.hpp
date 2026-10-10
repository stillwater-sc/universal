#pragma once
// tile_oracle.hpp: exact dyadic arithmetic and outward-rounded dyadic intervals, to find the
// tightest run of tiles that holds a computed value
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A tile type (areal, poxel) can state a value no more tightly than the tile that contains
// it.  The oracle finds that tile, independently of the tile arithmetic it is used to judge:
//
//   dyadic     an exact m 2^e on einteger.  + - * are exact.
//   interval   a closed interval of dyadics.  + - * are exact on the ends; / and sqrt are
//              rounded outward at a working precision of P bits, and detect an exact result
//              (zero remainder, perfect square).
//   locate     the tile that contains a dyadic, by bisection over the lattice with exact
//              comparisons; box() maps an interval to its run of tiles.
//
// Every lattice point is dyadic.  A value that is not dyadic -- one tenth, one third,
// sqrt(2) -- is therefore never a lattice point, and as P grows its interval separates from
// every lattice point and resolves to one open tile.  The exception is a non-dyadic route to
// a dyadic value, such as (1/3) * 3: its interval straddles the lattice point 1 at every P,
// and the oracle reports the three tiles around it rather than guess.
//
// The oracle also bounds the tightest box of a computation whose inputs are sets:
// see the ucalc ubox command (#1654) for the subdivision that uses it.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>
#include <universal/number/einteger/einteger.hpp>
#include <universal/utility/string_parse.hpp>
#include <universal/utility/tile_interval.hpp>

namespace sw { namespace universal { namespace oracle {

using Big = einteger<std::uint32_t>;

///////////////////////////////////////////////////////////////////////////
// dyadic: m 2^e, exact

struct dyadic {
	Big m{ 0 };
	int e{ 0 };
};

inline dyadic make_dyadic(long long m, int e = 0) { return { Big(m), e }; }
inline int sign(const dyadic& x);
inline dyadic trim(dyadic x);
// a finite double, exactly
inline dyadic from_double(double v) {
	int e = 0;
	const double f = std::frexp(v, &e);                            // v = f 2^e, |f| in [0.5, 1)
	return trim({ Big(static_cast<long long>(std::ldexp(f, 53))), e - 53 });
}
inline int sign(const dyadic& x) { return x.m.iszero() ? 0 : (x.m.isneg() ? -1 : 1); }
inline dyadic neg(dyadic x) { x.m = -x.m; return x; }

// drop trailing zero bits of the significand, to keep the integers small
inline dyadic trim(dyadic x) {
	if (x.m.iszero()) return { Big(0), 0 };
	int z = 0;
	while (!x.m.test(static_cast<unsigned>(z))) ++z;
	if (z > 0) {
		const bool negative = x.m.isneg();
		Big mag = negative ? Big(-x.m) : x.m;
		mag >>= z;
		x.m = negative ? Big(-mag) : mag;
		x.e += z;
	}
	return x;
}

inline dyadic add(const dyadic& a, const dyadic& b) {
	if (a.m.iszero()) return b;
	if (b.m.iszero()) return a;
	const int emin = std::min(a.e, b.e);
	Big ma = a.m, mb = b.m;
	ma <<= (a.e - emin);
	mb <<= (b.e - emin);
	ma += mb;
	return trim({ ma, emin });
}
inline dyadic sub(const dyadic& a, const dyadic& b) { return add(a, neg(b)); }
inline dyadic mul(const dyadic& a, const dyadic& b) { Big m = a.m; m *= b.m; return trim({ m, a.e + b.e }); }
// the sign of a - b
inline int compare(const dyadic& a, const dyadic& b) { return sign(sub(a, b)); }

// position of the most significant bit of |x.m|, -1 for zero
inline int msb(const Big& m) {
	if (m.iszero()) return -1;
	return m.isneg() ? Big(-m).findMsb() : m.findMsb();
}

// a / b, rounded down and up to at least P significant bits; exact when the division is
struct rounded { dyadic lo, hi; bool exact; };

inline rounded divide(const dyadic& a, const dyadic& b, int P) {
	if (a.m.iszero()) return { a, a, true };
	const int s = std::max(0, P + msb(b.m) - msb(a.m) + 1);
	Big n = a.m;
	n <<= s;
	Big q = n, r = n;
	q /= b.m;
	r %= b.m;
	const int e = a.e - b.e - s;
	if (r.iszero()) { const dyadic x = trim({ q, e }); return { x, x, true }; }
	// truncation toward zero: the quotient lies strictly between q and q + sgn
	const bool positive = (a.m.isneg() == b.m.isneg());
	Big other = q;
	if (positive) ++other; else --other;
	const dyadic x = trim({ q, e }), y = trim({ other, e });
	return positive ? rounded{ x, y, false } : rounded{ y, x, false };
}

// floor(sqrt(n)) for n >= 0, by Newton's method from above
inline Big isqrt(const Big& n) {
	if (n.iszero()) return n;
	Big x(1);
	x <<= (msb(n) / 2 + 1);   // 2^ceil(bits / 2) >= sqrt(n)
	for (;;) {
		Big y = n;
		y /= x;
		y += x;
		y >>= 1;
		if (!(y < x)) return x;
		x = y;
	}
}

// sqrt(a) for a >= 0, rounded down and up to at least P bits; exact for a perfect square
inline rounded square_root(const dyadic& a, int P) {
	if (a.m.iszero()) return { a, a, true };
	int s = std::max(0, 2 * P - msb(a.m));
	if (((a.e - s) & 1) != 0) ++s;                      // an even exponent halves exactly
	Big n = a.m;
	n <<= s;
	const Big r = isqrt(n);
	Big sq = r;
	sq *= r;
	const int e = (a.e - s) / 2;
	const dyadic lo = trim({ r, e });
	if (sq == n) return { lo, lo, true };
	Big up = r;
	++up;
	return { lo, trim({ up, e }), false };
}

///////////////////////////////////////////////////////////////////////////
// interval: a closed set of dyadics, or nan.  The open flags only describe input sets
// (x~ is an open tile); arithmetic treats every end as closed, a superset.

struct interval {
	dyadic lo, hi;
	bool lo_inf{ false }, hi_inf{ false };
	bool lo_open{ false }, hi_open{ false };
	bool nan{ false };
};

inline interval point(const dyadic& x) { return { x, x }; }
inline interval entire() { interval r; r.lo_inf = r.hi_inf = true; return r; }
inline interval nan_interval() { interval r; r.nan = true; return r; }
inline bool is_point(const interval& x) { return !x.nan && !x.lo_inf && !x.hi_inf && compare(x.lo, x.hi) == 0; }
inline bool contains_zero(const interval& x) {
	return !x.nan && (x.lo_inf || sign(x.lo) <= 0) && (x.hi_inf || sign(x.hi) >= 0);
}
inline interval closed(interval x) { x.lo_open = x.hi_open = false; return x; }

inline interval add(const interval& a, const interval& b) {
	if (a.nan || b.nan) return nan_interval();
	interval r;
	r.lo_inf = a.lo_inf || b.lo_inf;
	r.hi_inf = a.hi_inf || b.hi_inf;
	if (!r.lo_inf) r.lo = add(a.lo, b.lo);
	if (!r.hi_inf) r.hi = add(a.hi, b.hi);
	return r;
}
inline interval neg(const interval& a) {
	if (a.nan) return a;
	interval r;
	r.lo_inf = a.hi_inf;
	r.hi_inf = a.lo_inf;
	if (!r.lo_inf) r.lo = neg(a.hi);
	if (!r.hi_inf) r.hi = neg(a.lo);
	r.lo_open = a.hi_open;
	r.hi_open = a.lo_open;
	return r;
}
inline interval sub(const interval& a, const interval& b) { return add(a, neg(b)); }

// the smallest interval holding the finite candidates
inline interval hull_of(const dyadic* lo, const dyadic* hi, int n) {
	interval r{ lo[0], hi[0] };
	for (int i = 1; i < n; ++i) {
		if (compare(lo[i], r.lo) < 0) r.lo = lo[i];
		if (compare(hi[i], r.hi) > 0) r.hi = hi[i];
	}
	return r;
}

inline interval mul(const interval& a, const interval& b) {
	if (a.nan || b.nan) return nan_interval();
	if (a.lo_inf || a.hi_inf || b.lo_inf || b.hi_inf) {
		const bool a_zero = is_point(a) && sign(a.lo) == 0, b_zero = is_point(b) && sign(b.lo) == 0;
		return (a_zero || b_zero) ? point(dyadic{}) : entire();
	}
	const dyadic p[4] = { mul(a.lo, b.lo), mul(a.lo, b.hi), mul(a.hi, b.lo), mul(a.hi, b.hi) };
	return hull_of(p, p, 4);
}

inline interval div(const interval& a, const interval& b, int P) {
	if (a.nan || b.nan) return nan_interval();
	if (contains_zero(b)) return (is_point(b)) ? nan_interval() : entire();
	if (a.lo_inf || a.hi_inf || b.lo_inf || b.hi_inf) return entire();
	dyadic lo[4], hi[4];
	const dyadic* x[2] = { &a.lo, &a.hi };
	const dyadic* y[2] = { &b.lo, &b.hi };
	int n = 0;
	for (const dyadic* u : x) for (const dyadic* v : y) {
		const rounded q = divide(*u, *v, P);
		lo[n] = q.lo;
		hi[n] = q.hi;
		++n;
	}
	return hull_of(lo, hi, 4);
}

inline interval sqrt(const interval& a, int P) {
	if (a.nan) return a;
	if (!a.hi_inf && sign(a.hi) < 0) return nan_interval();
	interval r;
	r.lo = (a.lo_inf || sign(a.lo) <= 0) ? dyadic{} : square_root(a.lo, P).lo;
	if (a.hi_inf) r.hi_inf = true;
	else r.hi = square_root(a.hi, P).hi;
	return r;
}

inline interval abs(const interval& a) {
	if (a.nan) return a;
	if (!a.lo_inf && sign(a.lo) >= 0) return closed(a);
	if (!a.hi_inf && sign(a.hi) <= 0) return closed(neg(a));
	interval r;
	r.lo = dyadic{};
	r.hi_inf = a.lo_inf || a.hi_inf;
	if (!r.hi_inf) r.hi = compare(neg(a.lo), a.hi) > 0 ? neg(a.lo) : a.hi;
	return r;
}

inline interval square(const interval& a) {
	const interval m = abs(a);
	if (m.nan) return m;
	interval r;
	r.lo = mul(m.lo, m.lo);
	r.hi_inf = m.hi_inf;
	if (!r.hi_inf) r.hi = mul(m.hi, m.hi);
	return r;
}

inline interval hull(const interval& a, const interval& b) {
	if (a.nan || b.nan) return nan_interval();
	interval r = closed(a);
	r.lo_inf = a.lo_inf || b.lo_inf;
	r.hi_inf = a.hi_inf || b.hi_inf;
	if (!r.lo_inf) r.lo = compare(a.lo, b.lo) <= 0 ? a.lo : b.lo;
	if (!r.hi_inf) r.hi = compare(a.hi, b.hi) >= 0 ? a.hi : b.hi;
	return r;
}

// the common part of two intervals that both hold the same value (nan when they share nothing)
inline interval intersect(const interval& a, const interval& b) {
	if (a.nan || b.nan) return nan_interval();
	interval r;
	r.lo_inf = a.lo_inf && b.lo_inf;
	r.hi_inf = a.hi_inf && b.hi_inf;
	if (!r.lo_inf) r.lo = a.lo_inf ? b.lo : (b.lo_inf ? a.lo : (compare(a.lo, b.lo) >= 0 ? a.lo : b.lo));
	if (!r.hi_inf) r.hi = a.hi_inf ? b.hi : (b.hi_inf ? a.hi : (compare(a.hi, b.hi) <= 0 ? a.hi : b.hi));
	if (!r.lo_inf && !r.hi_inf && compare(r.lo, r.hi) > 0) return nan_interval();
	return r;
}

// x^n for an integer n, by squaring: tight around zero for even powers
inline interval pow_int(interval x, long long n, int P) {
	const bool invert = n < 0;
	unsigned long long k = static_cast<unsigned long long>(invert ? -n : n);
	interval r = point(make_dyadic(1));
	bool first = true;
	while (k != 0) {
		if (k & 1ull) { r = first ? x : mul(r, x); first = false; }
		k >>= 1;
		if (k != 0) x = square(x);
	}
	return invert ? div(point(make_dyadic(1)), r, P) : r;
}

// the decimal text d, exactly when it is dyadic, otherwise rounded outward to P bits
inline interval from_decimal(std::string_view text, int P, bool& ok) {
	const auto scan = string_parse::scan_decimal_float(text);
	ok = scan.valid;
	if (!ok) return nan_interval();
	Big digits(0);
	for (char c : scan.int_part) { digits *= 10; digits += static_cast<long long>(c - '0'); }
	for (char c : scan.frac_part) { digits *= 10; digits += static_cast<long long>(c - '0'); }
	if (scan.negative) digits = -digits;
	const long long e10 = static_cast<long long>(scan.exp10) - static_cast<long long>(scan.frac_part.size());
	if (digits.iszero()) return point(dyadic{});
	if (e10 > 20000 || e10 < -20000) { ok = false; return nan_interval(); }
	Big ten(1);
	for (long long i = 0; i < (e10 < 0 ? -e10 : e10); ++i) ten *= 10;
	if (e10 >= 0) { digits *= ten; return point(trim({ digits, 0 })); }
	const rounded q = divide({ digits, 0 }, { ten, 0 }, P);
	return { q.lo, q.hi };
}

///////////////////////////////////////////////////////////////////////////
// the lattice of a tile type, exactly

template<unsigned nbits, unsigned es, typename bt>
dyadic exact_value(const poxel<nbits, es, bt>& t) {
	using X = poxel<nbits, es, bt>;
	const std::int64_t L = t.lattice();
	if (L == 0) return {};
	const auto d = X::decode_lattice(L);                   // (-1)^neg (2^nf + frac) 2^(scale - nf)
	Big m(static_cast<unsigned long long>((1ull << d.nf) | d.frac));
	if (d.negative) m = -m;
	return trim({ m, d.scale - static_cast<int>(d.nf) });
}

// areal: sign, es exponent bits, fbits fraction bits, the ubit; an exponent field of 0 is
// subnormal.  Decoded from the bits, since areal's top binade lies beyond double's range.
template<unsigned nbits, unsigned es, typename bt>
dyadic exact_value(const areal<nbits, es, bt>& t) {
	using A = areal<nbits, es, bt>;
	constexpr unsigned fbits = A::fbits;
	std::uint64_t f = 0, E = 0;
	for (unsigned i = 0; i < fbits; ++i) if (t.at(1 + i)) f |= (std::uint64_t(1) << i);
	for (unsigned i = 0; i < es; ++i) if (t.at(1 + fbits + i)) E |= (std::uint64_t(1) << i);
	Big m(static_cast<unsigned long long>(E == 0 ? f : ((std::uint64_t(1) << fbits) | f)));
	if (t.at(nbits - 1)) m = -m;
	const int scale = (E == 0 ? 1 : static_cast<int>(E)) - A::EXP_BIAS - static_cast<int>(fbits);
	return trim({ m, scale });
}

// the exact value of the lattice point with (even) tile key k
template<typename Tile>
dyadic lattice_point(std::int64_t k) { return exact_value(tile_traits<Tile>::tile(k)); }

// the key of the tile that contains x: an even key when x is a lattice point
template<typename Tile>
std::int64_t locate(const dyadic& x) {
	const std::int64_t K = tile_traits<Tile>::kmax;
	const std::int64_t top = (K - 1) / 2;                  // half-key of maxpos
	if (compare(x, lattice_point<Tile>(2 * top)) > 0) return K;
	if (compare(x, lattice_point<Tile>(-2 * top)) < 0) return -K;
	std::int64_t lo = -top, hi = top;                      // largest h with point(2h) <= x
	while (lo < hi) {
		const std::int64_t mid = lo + (hi - lo + 1) / 2;
		if (compare(lattice_point<Tile>(2 * mid), x) <= 0) lo = mid; else hi = mid - 1;
	}
	return compare(lattice_point<Tile>(2 * lo), x) == 0 ? 2 * lo : 2 * lo + 1;
}

// the run of tiles that holds every point of x
template<typename Tile>
tile_interval<Tile> box(const interval& x) {
	using I = tile_interval<Tile>;
	if (x.nan) return I::nan();
	const std::int64_t lo = x.lo_inf ? -I::kmax : locate<Tile>(x.lo);
	const std::int64_t hi = x.hi_inf ? I::kmax : locate<Tile>(x.hi);
	return I::from_keys(lo, std::max(lo, hi));
}

}}} // namespace sw::universal::oracle
