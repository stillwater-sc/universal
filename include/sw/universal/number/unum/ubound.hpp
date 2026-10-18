#pragma once
// ubound.hpp: ubound (unum bound) for interval arithmetic with unum Type I
//
// A ubound is a guaranteed enclosure: a contiguous set on the real line between two ends,
// each an exact unum value that is either included (closed) or excluded (open), or an
// infinity.  An inexact unum (ubit = 1) is the open tile between its exact value and the
// next exact value away from zero, so a ubound built from it is that open tile.
//
// Every operation rounds outward.  An end is computed in double, an error-free transform
// gives the sign of that double's rounding error, and the end is then rounded onto the
// unum lattice in the safe direction: down for a lower end, up for an upper end, open
// whenever the bound is not attained.  The result therefore contains every value the
// operation can produce from the operands, and it is the tightest such set the lattice
// can express (#1689).
//
// The exact unum values of a configuration form an IEEE-like lattice: maxfsize + 1
// significand bits, exponents in [2 - 2^esmax, 2^esmax] with esmax = 2^esizesize - 1,
// and subnormals below.  For esizesize <= 3 and fsizesize <= 5 every lattice point is a
// double, so the rounding is exact; ubound requires that.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <algorithm>
#include <cmath>
#include <limits>

#include <universal/number/unum/lattice.hpp>
#include <universal/number/unum/unum_impl.hpp>

namespace sw { namespace universal {

// next_exact / prev_exact: the neighbouring exact unum values on the lattice.  They
// saturate at +-maxpos, which has no exact neighbour beyond it.
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> next_exact(const unum<esizesize, fsizesize, bt>& u) {
	using L = unum_lattice<esizesize, fsizesize>;
	if (u.isnan()) return u;
	const double v = u.to_double();
	unum<esizesize, fsizesize, bt> result;
	result = (v >= L::maxpos()) ? L::maxpos() : L::next(v);
	return result;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> prev_exact(const unum<esizesize, fsizesize, bt>& u) {
	using L = unum_lattice<esizesize, fsizesize>;
	if (u.isnan()) return u;
	const double v = u.to_double();
	unum<esizesize, fsizesize, bt> result;
	result = (v <= -L::maxpos()) ? -L::maxpos() : L::prev(v);
	return result;
}

namespace ubound_detail {

// the directed results come from unum_eft (lattice.hpp), shared with the scalar unum
using unum_eft::directed;
using unum_eft::add;
using unum_eft::mul;
using unum_eft::div;
using unum_eft::sqrt;

// an end of an interval: a lattice point v, excluded when open, or an infinity (inf = -1, +1)
struct end { double v; int inf; bool open; };

}  // namespace ubound_detail

///////////////////////////////////////////////////////////////////////////
// ubound: a guaranteed enclosure between two unum ends

template<unsigned esizesize, unsigned fsizesize, typename bt = std::uint8_t>
class ubound {
	using L = unum_lattice<esizesize, fsizesize>;
	using end = ubound_detail::end;
	using directed = ubound_detail::directed;
	static_assert(L::fits_double, "ubound needs a unum lattice that fits in double: esizesize <= 3 and fsizesize <= 5");
public:
	using Unum = unum<esizesize, fsizesize, bt>;

	ubound() { set({ 0.0, 0, false }, { 0.0, 0, false }); }

	ubound(const ubound&) = default;
	ubound(ubound&&) = default;
	ubound& operator=(const ubound&) = default;
	ubound& operator=(ubound&&) = default;

	// the tile of a single unum: [x, x] if exact, the open tile beside it if inexact
	explicit ubound(const Unum& u) {
		if (u.isnan()) { setnan(); return; }
		const end lo = tile_lower(u), hi = tile_upper(u);
		set(lo, hi);
	}

	// the hull of two unum tiles: from the lower end of lo's tile to the upper end of hi's
	ubound(const Unum& lo, const Unum& hi) {
		if (lo.isnan() || hi.isnan()) { setnan(); return; }
		set(tile_lower(lo), tile_upper(hi));
	}

	// the tile that contains v: the lattice point itself, or the open tile around it
	explicit ubound(double v) {
		if (std::isnan(v)) { setnan(); return; }
		if (std::isinf(v)) { setnan(); return; }   // unum Type I has no infinity value
		set(round_lower({ v, 0 }), round_upper({ v, 0 }));
	}

	// the two ends as unums (an unbounded end reports -maxpos or maxpos; see lower<Real>())
	const Unum& lower() const { return _lo; }
	const Unum& upper() const { return _hi; }

	// the infimum and supremum as Real; open ends are excluded from the set
	template<typename Real> Real lower() const noexcept {
		if (isnan()) return std::numeric_limits<Real>::quiet_NaN();
		if (_lo_inf) return -std::numeric_limits<Real>::infinity();
		return static_cast<Real>(_lo.to_double());
	}
	template<typename Real> Real upper() const noexcept {
		if (isnan()) return std::numeric_limits<Real>::quiet_NaN();
		if (_hi_inf) return std::numeric_limits<Real>::infinity();
		return static_cast<Real>(_hi.to_double());
	}
	bool lower_open() const noexcept { return _lo_open; }
	bool upper_open() const noexcept { return _hi_open; }
	bool lower_unbounded() const noexcept { return _lo_inf; }
	bool upper_unbounded() const noexcept { return _hi_inf; }

	// is either bound NaN?
	bool isnan() const { return _lo.isnan() || _hi.isnan(); }

	// a single exact value
	bool isexact() const noexcept {
		return !isnan() && !_lo_open && !_hi_open && !_lo_inf && !_hi_inf && _lo.to_double() == _hi.to_double();
	}
	bool ispoint() const { return isexact(); }

	// width and midpoint of the interval (inf or NaN when an end is unbounded)
	double width() const { return upper<double>() - lower<double>(); }
	double midpoint() const { return (lower<double>() + upper<double>()) / 2.0; }

	// does the set contain v?  Exact: the ends are doubles, so nothing rounds.
	bool contains(double v) const {
		if (isnan() || std::isnan(v)) return false;
		const double l = lower<double>(), u = upper<double>();
		return (_lo_open ? l < v : l <= v) && (_hi_open ? v < u : v <= u);
	}
	// does the set contain the value (exact unum) or the whole tile (inexact unum) of u?
	bool contains(const Unum& u) const { return contains(ubound(u)); }
	// is the other set a subset of this one?
	bool contains(const ubound& o) const {
		if (isnan() || o.isnan()) return false;
		const double l = lower<double>(), u = upper<double>(), ol = o.lower<double>(), ou = o.upper<double>();
		const bool lo_ok = (l < ol) || (l == ol && (!_lo_open || o._lo_open));
		const bool hi_ok = (ou < u) || (ou == u && (!_hi_open || o._hi_open));
		return lo_ok && hi_ok;
	}

	ubound operator-() const {
		if (isnan()) return *this;
		ubound r;
		r.set(negate(hi_end()), negate(lo_end()));
		return r;
	}

	ubound& operator+=(const ubound& rhs) {
		if (isnan() || rhs.isnan()) { setnan(); return *this; }
		const end lo = combine(lo_end(), rhs.lo_end(), op::add, true);
		const end hi = combine(hi_end(), rhs.hi_end(), op::add, false);
		set(lo, hi);
		return *this;
	}
	ubound& operator-=(const ubound& rhs) { return *this += -rhs; }

	ubound& operator*=(const ubound& rhs) {
		if (isnan() || rhs.isnan()) { setnan(); return *this; }
		corners(rhs, op::mul);
		return *this;
	}

	// a divisor that may be zero gives NaN
	ubound& operator/=(const ubound& rhs) {
		if (isnan() || rhs.isnan()) { setnan(); return *this; }
		if (rhs.contains(0.0)) { setnan(); return *this; }
		corners(rhs, op::div);
		return *this;
	}

	// sqrt with guaranteed bounds.  sqrt is monotone, so each end is the rounded root of the
	// corresponding end, open where that end is open.  The domain is the non-negative part
	// of the set; a set with no non-negative part gives NaN.
	friend ubound sqrt(const ubound& x) {
		if (x.isnan() || x.upper<double>() < 0.0 || (x.upper<double>() == 0.0 && x._hi_open)) {
			ubound r; r.setnan(); return r;
		}
		end lo = x.lo_end(), hi = x.hi_end();
		if (lo.inf != 0 || lo.v < 0.0) lo = { 0.0, 0, false };
		end rlo = (lo.v == 0.0) ? end{ 0.0, 0, false } : round_lower(ubound_detail::sqrt(lo.v));
		if (lo.open && !rlo.open) rlo.open = true;
		end rhi = (hi.inf != 0) ? end{ 0.0, +1, true } : round_upper(ubound_detail::sqrt(hi.v));
		if (hi.open && !rhi.open && hi.inf == 0) rhi.open = true;
		ubound r;
		r.set(rlo, rhi);
		return r;
	}

	// the smallest ubound that contains both sets (NaN is ignored, as in tile_interval)
	friend ubound hull(const ubound& a, const ubound& b) {
		if (a.isnan()) return b;
		if (b.isnan()) return a;
		const end al = a.lo_end(), bl = b.lo_end(), ah = a.hi_end(), bh = b.hi_end();
		ubound r;
		r.set(lower_before(bl, al) ? bl : al, upper_after(bh, ah) ? bh : ah);
		return r;
	}

private:
	Unum _lo;          // lower end, an exact lattice point (-maxpos when unbounded)
	Unum _hi;          // upper end, an exact lattice point (maxpos when unbounded)
	bool _lo_open;     // the lower end is excluded
	bool _hi_open;     // the upper end is excluded
	bool _lo_inf;      // the set reaches -inf
	bool _hi_inf;      // the set reaches +inf

	enum class op { add, mul, div };

	void setnan() {
		_lo.setnan(); _hi.setnan();
		_lo_open = _hi_open = _lo_inf = _hi_inf = false;
	}
	void set(const end& lo, const end& hi) {
		_lo_inf = (lo.inf != 0);
		_hi_inf = (hi.inf != 0);
		_lo = _lo_inf ? -L::maxpos() : lo.v;
		_hi = _hi_inf ? L::maxpos() : hi.v;
		_lo_open = lo.open || _lo_inf;
		_hi_open = hi.open || _hi_inf;
	}
	end lo_end() const noexcept { return _lo_inf ? end{ 0.0, -1, true } : end{ _lo.to_double(), 0, _lo_open }; }
	end hi_end() const noexcept { return _hi_inf ? end{ 0.0, +1, true } : end{ _hi.to_double(), 0, _hi_open }; }
	static end negate(const end& e) noexcept { return { -e.v, -e.inf, e.open }; }

	// the ends of a unum's tile: an inexact unum is open between its value and the next
	// exact value away from zero; the tile past maxpos is unbounded
	static end tile_lower(const Unum& u) {
		const double v = u.to_double();
		if (!u.ubit()) return { v, 0, false };
		if (u.sign()) return (v <= -L::maxpos()) ? end{ 0.0, -1, true } : end{ L::prev(v), 0, true };
		return { v, 0, true };
	}
	static end tile_upper(const Unum& u) {
		const double v = u.to_double();
		if (!u.ubit()) return { v, 0, false };
		if (u.sign()) return { v, 0, true };
		return (v >= L::maxpos()) ? end{ 0.0, +1, true } : end{ L::next(v), 0, true };
	}

	// round a directed result outward onto the lattice: a lower end goes down to the largest
	// lattice point below the exact value (closed only when it is the exact value), an upper
	// end up to the smallest above.  Beyond maxpos a lower end stays open at maxpos and an
	// upper end becomes unbounded.
	static end round_lower(const directed& r) {
		const double m = L::maxpos();
		if (r.d > m || (r.d == m && r.dir > 0)) return { m, 0, true };
		if (r.d < -m || (r.d == -m && r.dir < 0)) return { 0.0, -1, true };
		const double f = L::floor(r.d);
		if (f != r.d) return { f, 0, true };
		if (r.dir == 0) return { f, 0, false };
		if (r.dir > 0) return { f, 0, true };
		return { L::prev(f), 0, true };
	}
	static end round_upper(const directed& r) {
		const double m = L::maxpos();
		if (r.d < -m || (r.d == -m && r.dir < 0)) return { -m, 0, true };
		if (r.d > m || (r.d == m && r.dir > 0)) return { 0.0, +1, true };
		const double c = L::ceil(r.d);
		if (c != r.d) return { c, 0, true };
		if (r.dir == 0) return { c, 0, false };
		if (r.dir < 0) return { c, 0, true };
		return { L::next(c), 0, true };
	}

	static int signum(const end& e) noexcept { return e.inf != 0 ? e.inf : (e.v > 0.0) - (e.v < 0.0); }

	// the bound of x (op) y from below (lower = true) or above.  For two finite ends it is
	// the exact result rounded outward; it is open when that corner is not attained (an
	// open end whose partner is not a closed zero).  Corners at an infinity or an open zero
	// take their limit.  The side of an end says where the set lies from it: +1 for a lower
	// end (the set is above it), -1 for an upper end.  So an open zero at a lower end is
	// approached from above, and one at an upper end from below.
	static end combine(const end& x, const end& y, op o, bool lower, int xside = 0, int yside = 0) {
		const end widest = lower ? end{ 0.0, -1, true } : end{ 0.0, +1, true };
		const end closed_zero{ 0.0, 0, false };
		const end open_zero{ 0.0, 0, true };
		// products or quotients of every magnitude, of sign s: from 0 (not attained) to s inf
		auto spread = [&](int s) { return (s > 0) == lower ? open_zero : end{ 0.0, s, true }; };
		auto is_zero = [](const end& e) { return e.inf == 0 && e.v == 0.0; };
		if (x.inf == 0 && y.inf == 0 && !(o == op::div && y.v == 0.0)) {
			directed r{ 0.0, 0 };
			switch (o) {
			case op::add: r = ubound_detail::add(x.v, y.v); break;
			case op::mul: r = ubound_detail::mul(x.v, y.v); break;
			case op::div: r = ubound_detail::div(x.v, y.v); break;
			}
			end b = lower ? round_lower(r) : round_upper(r);
			bool strict = false;
			switch (o) {
			case op::add: strict = x.open || y.open; break;
			case op::mul:
			case op::div:
				// an open end is reached only through a partner that is a closed zero
				strict = (x.open && !(y.v == 0.0 && !y.open)) || (y.open && !(x.v == 0.0 && !x.open));
				break;
			}
			if (strict && b.inf == 0) b.open = true;
			return b;
		}
		switch (o) {
		case op::add:
			if (x.inf != 0 && y.inf != 0 && x.inf != y.inf) return widest;   // inf - inf
			return { 0.0, (x.inf != 0) ? x.inf : y.inf, true };
		case op::mul:
			// an infinite end times a zero end: exactly 0 for a closed zero, every magnitude
			// of one sign for an open one
			if (is_zero(x)) return x.open ? spread(xside * y.inf) : closed_zero;
			if (is_zero(y)) return y.open ? spread(yside * x.inf) : closed_zero;
			return { 0.0, signum(x) * signum(y), true };
		case op::div:
			if (y.inf != 0) {
				if (x.inf != 0) return spread(x.inf * y.inf);                // inf / inf
				return (is_zero(x) && !x.open) ? closed_zero : open_zero;    // finite / inf -> 0, not attained
			}
			if (y.v == 0.0) {
				// an open zero divisor (a closed one was rejected): the quotient grows without
				// bound, on the side of sgn(x) times the side the divisor approaches from
				if (x.inf != 0) return { 0.0, x.inf * yside, true };
				if (!is_zero(x)) return { 0.0, signum(x) * yside, true };
				return x.open ? spread(xside * yside) : closed_zero;         // 0+ / 0+, or exactly 0
			}
			return { 0.0, x.inf * signum(y), true };                         // inf / finite
		}
		return widest;
	}

	// order bounds: the smaller lower bound, the larger upper bound (a closed end reaches
	// further than an open one at the same point)
	static bool lower_before(const end& a, const end& b) noexcept {
		if (a.inf != b.inf) return a.inf < b.inf;
		if (a.inf != 0) return false;
		if (a.v != b.v) return a.v < b.v;
		return !a.open && b.open;
	}
	static bool upper_after(const end& a, const end& b) noexcept {
		if (a.inf != b.inf) return a.inf > b.inf;
		if (a.inf != 0) return false;
		if (a.v != b.v) return a.v > b.v;
		return !a.open && b.open;
	}

	// * and /: the hull of the four corner bounds
	void corners(const ubound& rhs, op o) {
		const end a[2] = { lo_end(), hi_end() }, b[2] = { rhs.lo_end(), rhs.hi_end() };
		const int side[2] = { +1, -1 };   // a lower end has the set above it, an upper end below
		end lo{ 0.0, +1, true }, hi{ 0.0, -1, true };
		for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) {
			const end l = combine(a[i], b[j], o, true, side[i], side[j]);
			const end u = combine(a[i], b[j], o, false, side[i], side[j]);
			if (lower_before(l, lo)) lo = l;
			if (upper_after(u, hi)) hi = u;
		}
		set(lo, hi);
	}
};

// binary operators
template<unsigned esizesize, unsigned fsizesize, typename bt>
ubound<esizesize, fsizesize, bt> operator+(const ubound<esizesize, fsizesize, bt>& lhs, const ubound<esizesize, fsizesize, bt>& rhs) {
	ubound<esizesize, fsizesize, bt> sum(lhs);
	sum += rhs;
	return sum;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
ubound<esizesize, fsizesize, bt> operator-(const ubound<esizesize, fsizesize, bt>& lhs, const ubound<esizesize, fsizesize, bt>& rhs) {
	ubound<esizesize, fsizesize, bt> diff(lhs);
	diff -= rhs;
	return diff;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
ubound<esizesize, fsizesize, bt> operator*(const ubound<esizesize, fsizesize, bt>& lhs, const ubound<esizesize, fsizesize, bt>& rhs) {
	ubound<esizesize, fsizesize, bt> prod(lhs);
	prod *= rhs;
	return prod;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
ubound<esizesize, fsizesize, bt> operator/(const ubound<esizesize, fsizesize, bt>& lhs, const ubound<esizesize, fsizesize, bt>& rhs) {
	ubound<esizesize, fsizesize, bt> quot(lhs);
	quot /= rhs;
	return quot;
}

// operator<< is in iostream.hpp (#1334)

}} // namespace sw::universal
