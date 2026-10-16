// tile_interval.cpp: soundness of the tile interval over poxel and areal tiles
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// tile_interval<Tile> claims to ENCLOSE: for every x in a and y in b, x op y lies in
// a op b.  Verified exhaustively over every pair of tiles of small poxel and areal
// configurations, exact and open, with sample points across each tile: the closed ends,
// and interior points a short, a half and nearly all the way across.  The double reference
// is nudged toward the exact result when it rounds (see reference()), so it never lands on
// a lattice point that the exact result does not.  For two exact tiles the interval must also be TIGHT: the single
// tile that contains the exact result.  cos is checked against std::cos on a grid.
//
// Enclosure alone accepts an answer that is too wide: (-inf, inf) encloses everything.  So
// every pair of finite tile intervals is also checked for TIGHTNESS against an exact oracle:
// the hull of the four corners as exact rationals, with the limit at an open zero divisor,
// each end located among the lattice points.  The limits at infinite ends are checked case
// by case (#1702).
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <vector>
#include <universal/number/areal/areal.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/utility/tile_interval.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

enum class Op { add, sub, mul, div };

// x op y in double, nudged one ulp toward the exact result when the double is inexact:
// an inexact sum can round onto a lattice point that a strict (open) bound rightly
// excludes, and the nudged value lies on the same side of it as the exact result
inline double reference(Op op, double x, double y) {
	double v = 0.0, err = 0.0;
	switch (op) {
	case Op::add: v = x + y; { const double bb = v - x; err = (x - (v - bb)) + (y - bb); } break;
	case Op::sub: v = x - y; { const double bb = v - x; err = (x - (v - bb)) + (-y - bb); } break;
	case Op::mul: v = x * y; err = std::fma(x, y, -v); break;
	case Op::div: v = x / y; err = -std::fma(v, y, -x) / y; break;
	}
	if (std::isfinite(v) && err != 0.0) v = std::nextafter(v, err > 0 ? INFINITY : -INFINITY);
	return v;
}

// points of a single tile, as doubles with few significand bits
template<typename I>
std::vector<double> samples(const I& a) {
	std::vector<double> s;
	double l = a.template lower<double>(), u = a.template upper<double>();
	if (!a.lower_open()) s.push_back(l);
	if (!a.upper_open() && u != l) s.push_back(u);
	if (a.lower_open() || a.upper_open()) {
		if (std::isinf(u)) u = (l > 0 ? l * 64.0 : 1.0);            // (maxpos, inf): sample above maxpos
		if (std::isinf(l)) l = (u < 0 ? u * 64.0 : -1.0);           // (-inf, -maxpos)
		for (double f : { 1.0 / 1024.0, 0.5, 1023.0 / 1024.0 }) s.push_back(l + (u - l) * f);
	}
	return s;
}

template<typename Tile>
int VerifyEnclosure(const std::string& tag, bool report) {
	using I = sw::universal::tile_interval<Tile>;
	using T = sw::universal::tile_traits<Tile>;
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << tag << ": " << what << '\n'; };
	for (std::int64_t i = -I::kmax; i <= I::kmax; ++i) {
		const I a = I::from_keys(i, i);
		const auto xs = samples(a);
		for (std::int64_t j = -I::kmax; j <= I::kmax; ++j) {
			const I b = I::from_keys(j, j);
			const auto ys = samples(b);
			for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) {
				I r;
				switch (op) { case Op::add: r = a + b; break; case Op::sub: r = a - b; break; case Op::mul: r = a * b; break; case Op::div: r = a / b; break; }
				const std::string where = std::to_string(i) + (op == Op::add ? " + " : op == Op::sub ? " - " : op == Op::mul ? " * " : " / ") + std::to_string(j) + " = " + r.str();
				if (op == Op::div && j == 0) { if (!r.isnan()) fail(where + " must be nan"); continue; }
				if (r.isnan()) { fail(where + " is nan"); continue; }
				for (double x : xs) for (double y : ys) {
					const double v = reference(op, x, y);
					if (std::isnan(v)) continue;                       // 0 * inf-like samples cannot occur; division by a sampled 0 is inf
					if (std::isinf(v)) { if (!(v > 0 ? r.upper_open() && r.hi_key() == I::kmax : r.lower_open() && r.lo_key() == -I::kmax)) fail(where + " misses an unbounded result"); continue; }
					if (!r.contains(v)) { fail(where + " does not contain " + std::to_string(x) + " op " + std::to_string(y) + " = " + std::to_string(v)); break; }
				}
				// tightness: two exact tiles give exactly the tile of the exact result
				if (a.isexact() && b.isexact()) {
					const Tile ta = T::tile(i), tb = T::tile(j);
					Tile t;
					switch (op) { case Op::add: t = ta + tb; break; case Op::sub: t = ta - tb; break; case Op::mul: t = ta * tb; break; case Op::div: t = ta / tb; break; }
					const std::int64_t k = std::clamp(T::key(t), -I::kmax, I::kmax);
					if (r.lo_key() != k || r.hi_key() != k) fail(where + " is not the single tile of the exact result");
				}
			}
		}
	}
	return fails;
}

// wider operands: hulls of a few tiles, sampled at every tile they span
template<typename Tile>
int VerifyHulls(const std::string& tag, bool report) {
	using I = sw::universal::tile_interval<Tile>;
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << tag << ": " << what << '\n'; };
	auto points = [](const I& a) {
		std::vector<double> s;
		for (std::int64_t k = a.lo_key(); k <= a.hi_key(); ++k) for (double p : samples(I::from_keys(k, k))) s.push_back(p);
		return s;
	};
	for (std::int64_t i = -I::kmax; i <= I::kmax; i += 7) {
		for (std::int64_t w : { 1, 2, 5 }) {
			if (i + w > I::kmax) continue;
			const I a = I::from_keys(i, i + w);
			for (std::int64_t j = -I::kmax; j <= I::kmax; j += 11) {
				for (std::int64_t v : { 0, 3 }) {
					if (j + v > I::kmax) continue;
					const I b = I::from_keys(j, j + v);
					for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) {
						I r;
						switch (op) { case Op::add: r = a + b; break; case Op::sub: r = a - b; break; case Op::mul: r = a * b; break; case Op::div: r = a / b; break; }
						if (r.isnan()) { if (!(op == Op::div && b.lo_key() == 0 && b.hi_key() == 0)) fail("unexpected nan"); continue; }
						for (double x : points(a)) for (double y : points(b)) {
							const double z = reference(op, x, y);
							if (std::isnan(z) || std::isinf(z)) continue;
							if (!r.contains(z)) { fail(a.str() + " op " + b.str() + " = " + r.str() + " misses " + std::to_string(z)); break; }
						}
					}
				}
			}
		}
	}
	return fails;
}

// slack: allowance for std::cos's own error, needed once the tile is finer than double
template<typename Tile>
int VerifyCos(const std::string& tag, double max_width, bool report, double slack = 0.0) {
	using I = sw::universal::tile_interval<Tile>;
	int fails = 0;
	for (int n = -64; n <= 64; ++n) {
		const double t = n / 32.0;                         // [-2, 2], exact in every tile here
		const I c = cos(I(t));
		const double v = std::cos(t);
		const double lo = c.template lower<double>(), hi = c.template upper<double>();
		const bool inside = (slack == 0.0) ? c.contains(v) : (lo - slack <= v && v <= hi + slack);
		if (!inside || hi - lo > max_width) {
			++fails;
			if (report && fails < 10) std::cerr << "FAIL " << tag << " cos(" << t << ") = " << c.str() << " vs " << v << '\n';
		}
	}
	// beyond the accurate range the enclosure widens but must still hold; here the Taylor
	// truncation is large enough to expose a lower and upper bound taken from the wrong sums
	for (int n = -64; n <= 64; ++n) {
		const double t = n / 8.0;                          // [-8, 8]
		const I c = cos(I(t));
		const double v = std::cos(t);
		const double lo = c.template lower<double>(), hi = c.template upper<double>();
		if (!(lo - slack <= v && v <= hi + slack)) {
			++fails;
			if (report && fails < 10) std::cerr << "FAIL " << tag << " cos(" << t << ") = " << c.str() << " vs " << v << '\n';
		}
	}
	return fails;
}

// open ends survive: sums and products of strictly positive sets stay strictly positive
template<typename Tile>
int VerifyStrictness(const std::string& tag, bool report) {
	using I = sw::universal::tile_interval<Tile>;
	using sw::universal::tile_verdict;
	int fails = 0;
	auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (report) std::cerr << "FAIL " << tag << ' ' << what << '\n'; } };
	const I tiny = I::from_keys(1, 1);                       // (0, minpos)
	expect((tiny + tiny).sign() == tile_verdict::positive, "(0, minpos) + (0, minpos) is strictly positive");
	expect((tiny * I(3)).sign() == tile_verdict::positive, "(0, minpos) * 3 is strictly positive");
	expect((tiny / I(3)).sign() == tile_verdict::positive, "(0, minpos) / 3 is strictly positive");
	expect((tiny * I(0)).sign() == tile_verdict::zero, "(0, minpos) * 0 is exactly zero");
	expect((I(1) - I(1)).sign() == tile_verdict::zero, "1 - 1 is exactly zero");
	expect((-tiny - tiny).sign() == tile_verdict::negative, "-(0, minpos) - (0, minpos) is strictly negative");
	expect(intersect(I(1), I(2)).isnan() && !intersect(I(1), hull(I(1), I(2))).isnan(), "disjoint enclosures intersect to nan");
	return fails;
}

// sqrt encloses: for every sample x of a tile interval, sqrt(x) lies in the result.  Checked
// without rounding as L^2 <= x <= U^2 (strict at open ends): for these lattices L^2 and U^2
// are exact in double.  For a single exact tile the result is the single tile containing the
// root, and the single-tile tile_sqrt agrees with it.
template<typename Tile>
int VerifySqrt(const std::string& tag, bool report) {
	using I = sw::universal::tile_interval<Tile>;
	using T = sw::universal::tile_traits<Tile>;
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << tag << ": " << what << '\n'; };
	for (std::int64_t i = -I::kmax; i <= I::kmax; ++i) {
		for (std::int64_t w : { 0, 1, 4 }) {
			if (i + w > I::kmax) continue;
			const I x = I::from_keys(i, i + w);
			const I r = sqrt(x);
			const std::string where = "sqrt(" + x.str() + ") = " + r.str();
			if (i + w < 0) { if (!r.isnan()) fail(where + " must be nan"); continue; }
			if (r.isnan()) { fail(where + " is nan"); continue; }
			const double L = r.template lower<double>(), U = r.template upper<double>();
			for (std::int64_t k = std::max<std::int64_t>(i, 0); k <= i + w; ++k) {
				for (double v : samples(I::from_keys(k, k))) {
					if (v < 0.0) continue;
					const bool above = r.lower_open() ? (L * L < v) : (L * L <= v);
					const bool below = std::isinf(U) || (r.upper_open() ? (v < U * U) : (v <= U * U));
					if (!(above && below)) { fail(where + " misses sqrt(" + std::to_string(v) + ")"); break; }
				}
			}
			if (w == 0 && i >= 0) {
				if (x.isexact() && r.lo_key() != r.hi_key()) fail(where + " is not a single tile for an exact input");
				const std::int64_t s = std::clamp(T::key(sw::universal::tile_sqrt(T::tile(i))), -I::kmax, I::kmax);
				if (x.isexact() && s != r.lo_key()) fail("tile_sqrt disagrees with " + where);
			}
		}
	}
	// open ends stay open, exact squares are exact, negative tiles have no square root
	if (!sqrt(I::from_keys(1, 1)).lower_open()) fail("sqrt((0, minpos)) must stay open at 0");
	if (!sqrt(I(4)).isexact() || sqrt(I(4)).template lower<double>() != 2.0) fail("sqrt(4) must be exactly 2");
	if (!T::isnan(sw::universal::tile_sqrt(Tile(-1)))) fail("tile_sqrt(-1) must be nan");
	return fails;
}

// --- tightness ---------------------------------------------------------------------------

// an exact rational n/d with d > 0; for these lattices the cross products are exact in double
struct Q { double n, d; };
inline int cmp(const Q& a, const Q& b) {
	const double l = a.n * b.d, r = b.n * a.d;
	return (l > r) - (l < r);
}
inline Q quotient(double a, double b) { return b < 0 ? Q{ -a, -b } : Q{ a, b }; }

// the value of the lattice point with even key k
template<typename I>
double lattice_point(std::int64_t k) { return I::from_keys(k, k).template lower<double>(); }

// the key of the tightest lower (upper) bound of a set whose infimum (supremum) is t, and
// that does or does not contain t
template<typename I>
std::int64_t lower_key(const Q& t, bool attained) {
	const std::int64_t top = I::kmax - 1;                                // the even key of maxpos
	if (cmp(t, { lattice_point<I>(-top), 1 }) < 0) return -I::kmax;
	if (cmp(t, { lattice_point<I>(top), 1 }) > 0) return I::kmax;
	std::int64_t lo = -top / 2, hi = top / 2;                            // key = 2 * index
	while (lo < hi) {                                                    // the largest point <= t
		const std::int64_t m = lo + (hi - lo + 1) / 2;
		if (cmp({ lattice_point<I>(2 * m), 1 }, t) <= 0) lo = m; else hi = m - 1;
	}
	const std::int64_t k = 2 * lo;
	return (attained && cmp({ lattice_point<I>(k), 1 }, t) == 0) ? k : k + 1;
}
template<typename I>
std::int64_t upper_key(const Q& t, bool attained) {
	const std::int64_t top = I::kmax - 1;
	if (cmp(t, { lattice_point<I>(top), 1 }) > 0) return I::kmax;
	if (cmp(t, { lattice_point<I>(-top), 1 }) < 0) return -I::kmax;
	std::int64_t lo = -top / 2, hi = top / 2;
	while (lo < hi) {                                                    // the smallest point >= t
		const std::int64_t m = lo + (hi - lo) / 2;
		if (cmp({ lattice_point<I>(2 * m), 1 }, t) >= 0) hi = m; else lo = m + 1;
	}
	const std::int64_t k = 2 * lo;
	return (attained && cmp({ lattice_point<I>(k), 1 }, t) == 0) ? k : k - 1;
}

// the tightest x op y for two intervals with finite ends: false for nan
template<typename I>
bool tightest(const I& x, const I& y, Op op, std::int64_t& elo, std::int64_t& ehi) {
	if (op == Op::div && y.lo_key() <= 0 && y.hi_key() >= 0) {          // the divisor contains 0
		if (y.lo_key() == 0 && y.hi_key() == 0) return false;
		elo = -I::kmax; ehi = I::kmax;
		return true;
	}
	// an end and the side the set lies on from it (+1 above a lower end, -1 below an upper end)
	struct C { double v; bool open; int side; };
	const C xs[2] = { { x.template lower<double>(), x.lower_open(), +1 }, { x.template upper<double>(), x.upper_open(), -1 } };
	const C ys[2] = { { y.template lower<double>(), y.lower_open(), +1 }, { y.template upper<double>(), y.upper_open(), -1 } };
	bool first = true, lo_att = false, hi_att = false, lo_inf = false, hi_inf = false;
	Q lo{ 0, 1 }, hi{ 0, 1 };
	auto take = [&](const Q& t, bool att) {
		if (first) { lo = hi = t; lo_att = hi_att = att; first = false; return; }
		const int cl = cmp(t, lo), ch = cmp(t, hi);
		if (cl < 0) { lo = t; lo_att = att; } else if (cl == 0) lo_att = lo_att || att;
		if (ch > 0) { hi = t; hi_att = att; } else if (ch == 0) hi_att = hi_att || att;
	};
	if (op == Op::add || op == Op::sub) {
		// monotone in each operand: the infimum pairs the low ends (low with high for sub)
		const C& yl = (op == Op::add) ? ys[0] : ys[1];
		const C& yh = (op == Op::add) ? ys[1] : ys[0];
		const double s = (op == Op::add) ? 1.0 : -1.0;
		take({ xs[0].v + s * yl.v, 1 }, !xs[0].open && !yl.open);
		take({ xs[1].v + s * yh.v, 1 }, !xs[1].open && !yh.open);
	}
	else {
		for (const C& a : xs) for (const C& b : ys) {
			// a corner is attained when each open end meets a closed zero partner
			const bool att = (!a.open || (b.v == 0.0 && !b.open)) && (!b.open || (a.v == 0.0 && !a.open));
			if (op == Op::div && b.v == 0.0) {
				// b is an open zero: near it the divisor has the sign of its side, and a / b
				// grows without bound with sign sgn(a) * side; a closed zero numerator stays 0,
				// an open zero numerator gives every magnitude from 0 (not attained) to inf
				const int sa = (a.v > 0) - (a.v < 0);
				if (sa != 0) { (sa * b.side < 0 ? lo_inf : hi_inf) = true; }
				else if (!a.open) take({ 0, 1 }, true);
				else { take({ 0, 1 }, false); (a.side * b.side < 0 ? lo_inf : hi_inf) = true; }
				continue;
			}
			take(op == Op::mul ? Q{ a.v * b.v, 1 } : quotient(a.v, b.v), att);
		}
	}
	elo = lo_inf ? -I::kmax : lower_key<I>(lo, lo_att);
	ehi = hi_inf ? I::kmax : upper_key<I>(hi, hi_att);
	return true;
}

// every pair of single finite tiles, and hulls of several tiles, against the oracle
template<typename Tile>
int VerifyTightness(const std::string& tag, bool report) {
	using I = sw::universal::tile_interval<Tile>;
	int fails = 0;
	auto check = [&](const I& a, const I& b) {
		for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) {
			I r;
			switch (op) { case Op::add: r = a + b; break; case Op::sub: r = a - b; break; case Op::mul: r = a * b; break; case Op::div: r = a / b; break; }
			std::int64_t elo = 0, ehi = 0;
			const bool finite = tightest(a, b, op, elo, ehi);
			const bool ok = finite ? (!r.isnan() && r.lo_key() == elo && r.hi_key() == ehi) : r.isnan();
			if (!ok) {
				++fails;
				if (report && fails < 10) {
					std::cerr << "FAIL " << tag << ": " << a.str() << (op == Op::add ? " + " : op == Op::sub ? " - " : op == Op::mul ? " * " : " / ") << b.str()
					          << " = " << r.str() << ", expected " << (finite ? I::from_keys(elo, ehi).str() : std::string("nan")) << '\n';
				}
			}
		}
	};
	const std::int64_t top = I::kmax - 1;
	for (std::int64_t i = -top; i <= top; ++i) for (std::int64_t j = -top; j <= top; ++j) check(I::from_keys(i, i), I::from_keys(j, j));
	for (std::int64_t i = -top; i <= top; i += 3) for (std::int64_t w : { 1, 2, 7 }) {
		if (i + w > top) continue;
		for (std::int64_t j = -top; j <= top; j += 5) for (std::int64_t v : { 0, 1, 4 }) {
			if (j + v > top) continue;
			check(I::from_keys(i, i + w), I::from_keys(j, j + v));
		}
	}
	return fails;
}

// limits at infinite and open-zero ends, as tile keys: 0 is exact zero, +1 the open tile
// above it, -1 the one below, and +-kmax the tiles beyond +-maxpos
template<typename Tile>
int VerifyLimits(const std::string& tag, bool report) {
	using I = sw::universal::tile_interval<Tile>;
	using T = sw::universal::tile_traits<Tile>;
	const std::int64_t K = I::kmax;
	auto key = [](double v) { return T::key(Tile(v)); };
	auto iv = [&](std::int64_t lo, std::int64_t hi) { return I::from_keys(lo, hi); };
	const I one = iv(key(1), key(1)), zero = iv(0, 0);
	const I one_two = iv(key(1), key(2)), m_two_m_one = iv(key(-2), key(-1));
	const I oz_one = iv(1, key(1));                      // (0, 1]
	const I m_one_oz = iv(key(-1), -1);                  // [-1, 0)
	const I z_one = iv(0, key(1));                       // [0, 1]
	const I open_m_one_oz = iv(key(-1) + 1, -1);         // (-1, 0)
	const I one_inf = iv(key(1), K);                     // [1, inf)
	int fails = 0;
	auto expect = [&](const I& r, std::int64_t lo, std::int64_t hi, const std::string& what) {
		if (!r.isnan() && r.lo_key() == lo && r.hi_key() == hi) return;
		++fails;
		if (report) std::cerr << "FAIL " << tag << ' ' << what << " = " << r.str() << ", expected " << iv(lo, hi).str() << '\n';
	};
	expect(one_two / oz_one, key(1), K, "[1,2] / (0,1]");
	expect(m_two_m_one / oz_one, -K, key(-1), "[-2,-1] / (0,1]");
	expect(one_two / m_one_oz, -K, key(-1), "[1,2] / [-1,0)");
	expect(oz_one / oz_one, 1, K, "(0,1] / (0,1]");
	expect(oz_one / m_one_oz, -K, -1, "(0,1] / [-1,0)");
	expect(z_one / oz_one, 0, K, "[0,1] / (0,1]");
	expect(oz_one * oz_one, 1, key(1), "(0,1] * (0,1]");
	expect(oz_one * m_one_oz, key(-1), -1, "(0,1] * [-1,0)");
	expect(oz_one * one_inf, 1, K, "(0,1] * [1,inf)");
	expect(open_m_one_oz * one_inf, -K, -1, "(-1,0) * [1,inf)");
	expect(z_one * one_inf, 0, K, "[0,1] * [1,inf)");
	expect(zero * one_inf, 0, 0, "[0] * [1,inf)");
	expect(one / one_inf, 1, key(1), "[1] / [1,inf)");
	expect(-one / one_inf, key(-1), -1, "[-1] / [1,inf)");
	expect(zero / one_inf, 0, 0, "[0] / [1,inf)");
	expect(one_inf / one_inf, 1, K, "[1,inf) / [1,inf)");
	expect(one_inf / oz_one, key(1), K, "[1,inf) / (0,1]");
	expect(one_inf + one_inf, key(2), K, "[1,inf) + [1,inf)");
	expect(one_inf - one_inf, -K, K, "[1,inf) - [1,inf)");
	return fails;
}

}  // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "tile interval";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyLimits<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "limits");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<poxel<8, 0, std::uint8_t>>("poxel<8,0>", reportTestCases), "tile_interval<poxel<8,0>>", "exhaustive enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "exhaustive enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), "tile_interval<areal<8,2>>", "exhaustive enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyStrictness<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "open ends");
	nrOfFailedTestCases += ReportTestResult(VerifyStrictness<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), "tile_interval<areal<8,2>>", "open ends");
	nrOfFailedTestCases += ReportTestResult(VerifyHulls<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "multi-tile enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyHulls<areal<9, 3, std::uint16_t>>("areal<9,3>", reportTestCases), "tile_interval<areal<9,3>>", "multi-tile enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<poxel<8, 0, std::uint8_t>>("poxel<8,0>", reportTestCases), "tile_interval<poxel<8,0>>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), "tile_interval<areal<8,2>>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<areal<9, 3, std::uint16_t>>("areal<9,3>", reportTestCases), "tile_interval<areal<9,3>>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyCos<poxel<17, 2, std::uint32_t>>("poxel17", 1.0e-2, reportTestCases), "tile_interval<poxel17>", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyCos<poxel<33, 2, std::uint64_t>>("poxel33", 1.0e-6, reportTestCases), "tile_interval<poxel33>", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyCos<poxel<64, 2, std::uint64_t>>("poxel<64,2>", 1.0e-15, reportTestCases, 4.5e-16), "tile_interval<poxel<64,2>>", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyCos<areal<32, 8, std::uint32_t>>("areal<32,8>", 1.0e-5, reportTestCases), "tile_interval<areal<32,8>>", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), "tile_interval<areal<8,2>>", "limits");
	nrOfFailedTestCases += ReportTestResult(VerifyTightness<poxel<8, 0, std::uint8_t>>("poxel<8,0>", reportTestCases), "tile_interval<poxel<8,0>>", "tightness");
	nrOfFailedTestCases += ReportTestResult(VerifyTightness<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), "tile_interval<areal<8,2>>", "tightness");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyTightness<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "tightness");
	nrOfFailedTestCases += ReportTestResult(VerifyTightness<areal<9, 3, std::uint16_t>>("areal<9,3>", reportTestCases), "tile_interval<areal<9,3>>", "tightness");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
