#pragma once
// tile_interval.hpp: a guaranteed enclosure built from a pair of ubit tiles (areal, poxel)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A ubit number system -- areal (float lattice) or poxel (posit lattice) -- splits the
// extended real line into tiles: the exact lattice points and the open intervals between
// them.  A single tile's arithmetic is a flag (the ubit says "inexact"), not an enclosure.
// tile_interval<Tile> is the enclosure: the contiguous run of tiles [lo, hi], in the spirit
// of Gustafson's valid.
//
// It needs one property of the tile type, which the areal and poxel tests verify
// exhaustively: an operation on two EXACT tiles returns the tile that contains the exact
// result.  Every interval operation is therefore evaluated on exact endpoints only, and
// the single containing tile supplies both directed roundings at once -- its lower end
// bounds the candidate from below, its upper end from above.
//
// Tiles are addressed by an integer key in [-kmax, kmax] that is their order on the real
// line: even keys are lattice points, odd keys the open intervals between them, -kmax is
// (-inf, -maxpos) and kmax is (maxpos, inf).  tile_traits<Tile> maps tiles to keys and
// back; specializations for areal and poxel (up to 64 bits) are below.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt> class areal;
template<unsigned nbits, unsigned es, typename bt> class poxel;

template<typename Tile> struct tile_traits;   // key(), tile(), isnan(), nan(), kmax

// poxel: the tile order is the two's-complement order of the pattern
template<unsigned nbits, unsigned es, typename bt>
struct tile_traits<poxel<nbits, es, bt>> {
	using Tile = poxel<nbits, es, bt>;
	static constexpr std::int64_t kmax = static_cast<std::int64_t>((std::uint64_t(1) << (nbits - 1)) - 1u);
	static bool isnan(const Tile& t) noexcept { return t.isnar(); }
	static std::int64_t key(const Tile& t) noexcept { return Tile::signed_value(t.raw()); }
	static Tile nan() noexcept { Tile t; t.setbits(std::uint64_t(1) << (nbits - 1)); return t; }   // NaR
	static Tile tile(std::int64_t k) noexcept { Tile t; t.setbits(static_cast<std::uint64_t>(k)); return t; }
};

// areal: sign-magnitude, the ubit in the lsb; key = +-magnitude, so -(x, next) has the odd
// key between -next and -x.  The magnitude after (maxpos, inf) is the infinity pattern.
template<unsigned nbits, unsigned es, typename bt>
struct tile_traits<areal<nbits, es, bt>> {
	static_assert(nbits <= 64, "tile_traits<areal>: keys are 64-bit integers");
	using Tile = areal<nbits, es, bt>;
	static constexpr std::uint64_t sign_bit = std::uint64_t(1) << (nbits - 1);
	static constexpr std::int64_t kmax = static_cast<std::int64_t>(sign_bit - 3u);
	static bool isnan(const Tile& t) noexcept { return t.isnan(); }
	static Tile nan() noexcept { Tile t; t.setnan(); return t; }
	static std::int64_t key(const Tile& t) noexcept {
		std::uint64_t mag = 0;
		for (unsigned i = nbits - 1; i-- > 0;) mag = (mag << 1) | (t.at(i) ? 1u : 0u);
		const std::int64_t k = static_cast<std::int64_t>(std::min<std::uint64_t>(mag, static_cast<std::uint64_t>(kmax) + 1u));   // inf -> kmax + 1
		return t.sign() ? -k : k;
	}
	static Tile tile(std::int64_t k) noexcept {
		Tile t;
		t.setbits(k < 0 ? (sign_bit | static_cast<std::uint64_t>(-k)) : static_cast<std::uint64_t>(k));
		return t;
	}
};

enum class tile_verdict { negative, zero, positive, undecidable };

inline const char* to_string(tile_verdict v) noexcept {
	switch (v) {
	case tile_verdict::negative: return "negative";
	case tile_verdict::zero:     return "zero";
	case tile_verdict::positive: return "positive";
	default:                     return "undecidable";
	}
}

template<typename Tile>
class tile_interval {
	using T = tile_traits<Tile>;
public:
	static constexpr std::int64_t kmax = T::kmax;

	tile_interval() noexcept = default;
	explicit tile_interval(const Tile& t) noexcept { assign(t); }
	template<typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
	explicit tile_interval(Native v) noexcept { assign(Tile(v)); }
	// the hull of two tiles
	tile_interval(const Tile& a, const Tile& b) noexcept {
		if (T::isnan(a) || T::isnan(b)) { setnan(); return; }
		_lo = std::min(clampkey(T::key(a)), clampkey(T::key(b)));
		_hi = std::max(clampkey(T::key(a)), clampkey(T::key(b)));
		_nan = false;
	}
	static tile_interval entire() noexcept { tile_interval r; r._lo = -kmax; r._hi = kmax; return r; }
	static tile_interval nan() noexcept { tile_interval r; r.setnan(); return r; }
	static tile_interval from_keys(std::int64_t lo, std::int64_t hi) noexcept { tile_interval r; r._lo = lo; r._hi = hi; return r; }

	// tiles and keys of the two ends
	Tile lo_tile() const noexcept { return T::tile(_lo); }
	Tile hi_tile() const noexcept { return T::tile(_hi); }
	std::int64_t lo_key() const noexcept { return _lo; }
	std::int64_t hi_key() const noexcept { return _hi; }
	bool isnan() const noexcept { return _nan; }
	// a single exact tile
	bool isexact() const noexcept { return !_nan && _lo == _hi && (_lo & 1) == 0; }

	// the infimum and supremum as Real (rounded to Real if it cannot hold the lattice point);
	// open ends are excluded from the set
	template<typename Real = double> Real lower() const noexcept {
		if (_nan) return std::numeric_limits<Real>::quiet_NaN();
		if (_lo == -kmax) return -std::numeric_limits<Real>::infinity();
		return point<Real>((_lo & 1) ? _lo - 1 : _lo);
	}
	template<typename Real = double> Real upper() const noexcept {
		if (_nan) return std::numeric_limits<Real>::quiet_NaN();
		if (_hi == kmax) return std::numeric_limits<Real>::infinity();
		return point<Real>((_hi & 1) ? _hi + 1 : _hi);
	}
	bool lower_open() const noexcept { return (_lo & 1) != 0; }
	bool upper_open() const noexcept { return (_hi & 1) != 0; }

	// does the set contain v?  (exact when Real holds the lattice points)
	template<typename Real>
	bool contains(Real v) const noexcept {
		if (_nan) return false;
		const Real l = lower<Real>(), u = upper<Real>();
		return (lower_open() ? l < v : l <= v) && (upper_open() ? v < u : v <= u);
	}

	tile_verdict sign() const noexcept {
		if (_nan) return tile_verdict::undecidable;
		if (_lo > 0) return tile_verdict::positive;     // key 1 is (0, minpos): strictly positive
		if (_hi < 0) return tile_verdict::negative;
		if (_lo == 0 && _hi == 0) return tile_verdict::zero;
		return tile_verdict::undecidable;
	}

	tile_interval operator-() const noexcept { if (_nan) return *this; return from_keys(-_hi, -_lo); }

	tile_interval& operator+=(const tile_interval& rhs) noexcept {
		if (_nan || rhs._nan) { setnan(); return *this; }
		const std::int64_t lo = combine(lo_end(), rhs.lo_end(), op::add, true);
		const std::int64_t hi = combine(hi_end(), rhs.hi_end(), op::add, false);
		_lo = lo; _hi = hi;
		return *this;
	}
	tile_interval& operator-=(const tile_interval& rhs) noexcept { return *this += -rhs; }
	tile_interval& operator*=(const tile_interval& rhs) noexcept {
		if (_nan || rhs._nan) { setnan(); return *this; }
		const end a[2] = { lo_end(), hi_end() }, b[2] = { rhs.lo_end(), rhs.hi_end() };
		std::int64_t lo = kmax, hi = -kmax;
		for (const end& x : a) for (const end& y : b) {
			lo = std::min(lo, combine(x, y, op::mul, true));
			hi = std::max(hi, combine(x, y, op::mul, false));
		}
		_lo = lo; _hi = hi;
		return *this;
	}
	tile_interval& operator/=(const tile_interval& rhs) noexcept {
		if (_nan || rhs._nan) { setnan(); return *this; }
		if (rhs._lo <= 0 && rhs._hi >= 0) {                      // the divisor may be zero
			if (rhs._lo == 0 && rhs._hi == 0) { setnan(); return *this; }
			*this = entire();
			return *this;
		}
		const end a[2] = { lo_end(), hi_end() }, b[2] = { rhs.lo_end(), rhs.hi_end() };
		std::int64_t lo = kmax, hi = -kmax;
		for (const end& x : a) for (const end& y : b) {
			lo = std::min(lo, combine(x, y, op::div, true));
			hi = std::max(hi, combine(x, y, op::div, false));
		}
		_lo = lo; _hi = hi;
		return *this;
	}

	// [lo, hi] as text: "[a, b]", with "(" or ")" for an open end
	std::string str(int precision = 17) const {
		if (_nan) return "nan";
		std::stringstream s;
		s.precision(precision);
		s << (lower_open() ? '(' : '[') << lower<long double>() << ", " << upper<long double>() << (upper_open() ? ')' : ']');
		return s.str();
	}

private:
	std::int64_t _lo{ 0 }, _hi{ 0 };   // tile keys, _lo <= _hi
	bool _nan{ false };

	// an interval end: an exact lattice point (tile key, even) or an infinity; open when the
	// end itself is excluded from the set
	struct end { std::int64_t k; int inf; bool open; };   // inf: -1, 0, +1
	enum class op { add, mul, div };

	void setnan() noexcept { _lo = _hi = 0; _nan = true; }
	void assign(const Tile& t) noexcept {
		if (T::isnan(t)) { setnan(); return; }
		_lo = _hi = clampkey(T::key(t));
		_nan = false;
	}
	static std::int64_t clampkey(std::int64_t k) noexcept { return std::clamp(k, -kmax, kmax); }   // infinity tiles -> end tiles
	template<typename Real> static Real point(std::int64_t k) noexcept { return static_cast<Real>(T::tile(k)); }

	end lo_end() const noexcept {
		if (_lo == -kmax) return { 0, -1, true };
		return { (_lo & 1) ? _lo - 1 : _lo, 0, (_lo & 1) != 0 };
	}
	end hi_end() const noexcept {
		if (_hi == kmax) return { 0, +1, true };
		return { (_hi & 1) ? _hi + 1 : _hi, 0, (_hi & 1) != 0 };
	}
	static int signum(const end& e) noexcept { return e.inf != 0 ? e.inf : (e.k > 0) - (e.k < 0); }

	// the key of the tile that bounds x (op) y from below (lower = true) or above: for two
	// finite ends, the tile that contains the exact result; for an infinite result, the end
	// tile on that side; for an indeterminate form, the widest bound.  A bound is strict when
	// the corner is not attained -- an open end whose partner does not annihilate it -- and a
	// strict bound that lands on a lattice point moves inward to the open tile beside it.
	static std::int64_t combine(const end& x, const end& y, op o, bool lower) noexcept {
		const std::int64_t widest = lower ? -kmax : kmax;
		if (x.inf == 0 && y.inf == 0) {
			const Tile a = T::tile(x.k), b = T::tile(y.k);
			Tile r;
			switch (o) {
			case op::add: r = a + b; break;
			case op::mul: r = a * b; break;
			case op::div: if (y.k == 0) return widest; r = a / b; break;
			}
			if (T::isnan(r)) return widest;
			const std::int64_t k = clampkey(T::key(r));
			bool strict = false;
			switch (o) {
			case op::add: strict = x.open || y.open; break;
			case op::mul:
			case op::div: strict = (x.open && y.k != 0) || (y.open && x.k != 0); break;
			}
			if (strict && (k & 1) == 0) return lower ? k + 1 : k - 1;
			return k;
		}
		int s = 0;
		switch (o) {
		case op::add:
			if (x.inf != 0 && y.inf != 0 && x.inf != y.inf) return widest;   // inf - inf
			s = (x.inf != 0) ? x.inf : y.inf;
			break;
		case op::mul:
			if (signum(x) == 0 || signum(y) == 0) return 0;                  // an exact zero times an unbounded end
			s = signum(x) * signum(y);
			break;
		case op::div:
			if (y.inf != 0) {
				if (x.inf != 0) return widest;                               // inf / inf
				return 0;                                                    // finite / inf
			}
			if (y.k == 0) return widest;
			s = signum(x) * signum(y);
			break;
		}
		return s > 0 ? kmax : -kmax;
	}
};

template<typename Tile> tile_interval<Tile> operator+(tile_interval<Tile> a, const tile_interval<Tile>& b) noexcept { return a += b; }
template<typename Tile> tile_interval<Tile> operator-(tile_interval<Tile> a, const tile_interval<Tile>& b) noexcept { return a -= b; }
template<typename Tile> tile_interval<Tile> operator*(tile_interval<Tile> a, const tile_interval<Tile>& b) noexcept { return a *= b; }
template<typename Tile> tile_interval<Tile> operator/(tile_interval<Tile> a, const tile_interval<Tile>& b) noexcept { return a /= b; }

// x^2, which unlike x * x knows both factors are the same number
template<typename Tile>
tile_interval<Tile> square(const tile_interval<Tile>& x) noexcept {
	tile_interval<Tile> r = x * x;
	if (r.isnan()) return r;
	if (x.lo_key() < 0 && x.hi_key() > 0) return tile_interval<Tile>::from_keys(0, r.hi_key());
	if (r.lo_key() < 0) return tile_interval<Tile>::from_keys(0, r.hi_key());
	return r;
}

// the hull of two intervals
template<typename Tile>
tile_interval<Tile> hull(const tile_interval<Tile>& a, const tile_interval<Tile>& b) noexcept {
	if (a.isnan()) return b;
	if (b.isnan()) return a;
	return tile_interval<Tile>::from_keys(std::min(a.lo_key(), b.lo_key()), std::max(a.hi_key(), b.hi_key()));
}

// the intersection of two enclosures of the same value (used to clamp to a known range);
// disjoint enclosures mean one of them is wrong, which is reported as nan
template<typename Tile>
tile_interval<Tile> intersect(const tile_interval<Tile>& a, const tile_interval<Tile>& b) noexcept {
	if (a.isnan() || b.isnan()) return a;
	const std::int64_t lo = std::max(a.lo_key(), b.lo_key()), hi = std::min(a.hi_key(), b.hi_key());
	if (lo > hi) return tile_interval<Tile>::nan();
	return tile_interval<Tile>::from_keys(lo, hi);
}

// The key of the tile that contains sqrt(v), for the lattice point v with even key kv >= 0.
// The search runs over lattice points t and compares the tile of t * t with v.  For exact
// operands that tile contains t^2 exactly, so its key orders t^2 against v without rounding:
// the largest t with key(t * t) <= kv satisfies t^2 <= v < next(t)^2.  sqrt(v) is then t
// itself when t * t is exactly v, and lies in the open tile above t otherwise.
template<typename Tile>
std::int64_t sqrt_key(std::int64_t kv) noexcept {
	using T = tile_traits<Tile>;
	if (kv <= 0) return 0;
	auto square = [](std::int64_t t) { const Tile x = T::tile(t); return std::clamp(T::key(x * x), -T::kmax, T::kmax); };
	std::int64_t lo = 0, hi = (T::kmax - 1) / 2;   // half-keys: lattice point t = 2h
	while (lo < hi) {                                // largest h with square(2h) <= kv
		const std::int64_t mid = lo + (hi - lo + 1) / 2;
		if (square(2 * mid) <= kv) lo = mid; else hi = mid - 1;
	}
	const std::int64_t t = 2 * lo;
	return (square(t) == kv) ? t : t + 1;
}

// sqrt with guaranteed bounds.  sqrt is monotone, so each end of the result is the tile that
// contains the root of the corresponding end.  An open end stays open: a strict bound that
// lands on a lattice point moves inward to the open tile beside it.  The domain is the
// non-negative part of x (an interval reaching below zero is restricted to [0, hi]); an
// interval entirely below zero has no square root and gives nan.
template<typename Tile>
tile_interval<Tile> sqrt(const tile_interval<Tile>& x) noexcept {
	using I = tile_interval<Tile>;
	if (x.isnan() || x.hi_key() < 0) return I::nan();
	std::int64_t lo = 0;
	if (x.lo_key() > 0) {
		const bool open = (x.lo_key() & 1) != 0;
		const std::int64_t k = sqrt_key<Tile>(open ? x.lo_key() - 1 : x.lo_key());
		lo = (open && (k & 1) == 0) ? k + 1 : k;
	}
	std::int64_t hi = I::kmax;
	if (x.hi_key() < I::kmax) {
		const bool open = (x.hi_key() & 1) != 0;
		const std::int64_t k = sqrt_key<Tile>(open ? x.hi_key() + 1 : x.hi_key());
		hi = (open && (k & 1) == 0) ? k - 1 : k;
	}
	return I::from_keys(lo, std::max(lo, hi));
}

// sqrt of a single tile, with the sticky-flag semantics of tile arithmetic: computed on the
// stored (lower) value, the result is the tile that contains its root; an open input keeps
// the result open.  A negative tile or nan gives nan.
template<typename Tile>
Tile tile_sqrt(const Tile& t) noexcept {
	using T = tile_traits<Tile>;
	if (T::isnan(t)) return t;
	const std::int64_t k = std::clamp(T::key(t), -T::kmax, T::kmax);
	if (k < 0) return T::nan();
	const bool open = (k & 1) != 0;
	const std::int64_t r = sqrt_key<Tile>(open ? k - 1 : k);
	return T::tile((open && (r & 1) == 0) ? r + 1 : r);
}

// cos with guaranteed bounds, from the Taylor partial sums S_m(t) = sum_{j<=m/2} (-1)^j t^2j/(2j)!:
// for every real t, S_{4n+2}(t) <= cos t <= S_{4n}(t).  Both bounds are polynomials in
// s = t^2, evaluated in tile_interval arithmetic, so rounding is enclosed as well; the
// result is intersected with [-1, 1].  Accurate for |t| <~ 2; larger arguments still give a
// correct but wider enclosure (no range reduction).
template<typename Tile>
tile_interval<Tile> cos(const tile_interval<Tile>& x) {
	using I = tile_interval<Tile>;
	if (x.isnan()) return x;
	constexpr int upper_terms = 14;   // S_28 is the upper bound, S_30 the lower: t^30/30! ~ 4e-24 at |t| = 2
	const I s = square(x);
	// coefficients c_j = 1/(2j)! as enclosures, each from the last by an exact small divisor
	I c[upper_terms + 2];
	c[0] = I(1);
	for (int j = 1; j <= upper_terms + 1; ++j) c[j] = c[j - 1] / I((2 * j - 1) * (2 * j));
	// Horner: sum_{j=0..m} (-1)^j c_j s^j
	auto partial = [&](int m) {
		I p = (m % 2) ? -c[m] : c[m];
		for (int j = m - 1; j >= 0; --j) p = p * s + ((j % 2) ? -c[j] : c[j]);
		return p;
	};
	const I lo = partial(upper_terms + 1);   // S_30: a lower bound
	const I hi = partial(upper_terms);       // S_28: an upper bound
	I r = I::from_keys(lo.lo_key(), hi.hi_key());
	const I unit(Tile(-1), Tile(1));
	return intersect(r, unit);
}

}}  // namespace sw::universal
