// ubound_enclosure.cpp: ubound rounds outward, encloses every result, and is the tightest
// enclosure the unum lattice can express (#1689)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The oracle here is independent of the implementation.  ubound finds the side of each
// rounding with error-free transforms; the oracle instead works in configurations small
// enough that every corner value is a rational p/q whose cross products are exact in
// double (unum<2,2> and unum<2,3>: at most 8 significand bits over 2^-13 .. 2^9), and
// locates it on the lattice with exact comparisons.  The lattice model itself is checked
// against every exact encoding first.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <set>
#include <vector>
#include <universal/number/unum/unum.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/utility/ubox_trace.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// the lattice model agrees with the set of exact encodings, round trips, and next/prev step
// between neighbours
template<unsigned ess, unsigned fss>
int VerifyLatticeModel(bool reportTestCases) {
	using U = unum<ess, fss>;
	using L = unum_lattice<ess, fss>;
	int nrOfFailedTestCases = 0;
	std::set<double> model;
	for (int E = L::emin; E <= L::emax; ++E)
		for (std::uint64_t m = 1ull << L::maxfs; m < (2ull << L::maxfs); ++m) model.insert(std::ldexp(double(m), E - L::maxfs));
	for (std::uint64_t k = 0; k < (1ull << L::maxfs); ++k) model.insert(std::ldexp(double(k), L::emin - L::maxfs));
	std::set<double> encodings;
	for (unsigned long long b = 0; b < (1ull << U::maxbits); ++b) {
		U u;
		u.setbits(b);
		if (u.isnan() || u.ubit() || u.sign() || (b >> u.nbits_used()) != 0) continue;
		encodings.insert(u.to_double());
	}
	if (model != encodings) {
		++nrOfFailedTestCases;
		if (reportTestCases) std::cerr << "FAIL: lattice model of unum<" << ess << ',' << fss << "> differs from its encodings\n";
	}
	if (*model.rbegin() != L::maxpos()) {
		++nrOfFailedTestCases;
		if (reportTestCases) std::cerr << "FAIL: maxpos " << L::maxpos() << " != " << *model.rbegin() << '\n';
	}
	std::vector<double> pts(model.begin(), model.end());
	for (std::size_t i = 0; i < pts.size(); ++i) {
		U u;
		u = pts[i];
		if (u.ubit() || u.to_double() != pts[i]) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: lattice point " << pts[i] << " does not round trip\n";
		}
		if (i + 1 < pts.size() && (L::next(pts[i]) != pts[i + 1] || L::prev(pts[i + 1]) != pts[i])) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: next/prev between " << pts[i] << " and " << pts[i + 1] << '\n';
		}
		if (i + 1 < pts.size()) {
			const double mid = (pts[i] + pts[i + 1]) / 2.0;
			if (L::floor(mid) != pts[i] || L::ceil(mid) != pts[i + 1] || L::floor(pts[i]) != pts[i]) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: floor/ceil around " << mid << '\n';
			}
		}
		if (nrOfFailedTestCases > 10) break;
	}
	return nrOfFailedTestCases;
}

namespace oracle {

// an exact rational n/d, d > 0, with n * d' cross products exact in double
struct Q { double n, d; };
inline int cmp(const Q& a, const Q& b) {
	const double l = a.n * b.d, r = b.n * a.d;
	return (l > r) - (l < r);
}
inline Q make_div(double a, double b) { return b < 0 ? Q{ -a, -b } : Q{ a, b }; }

// an expected end, in the same terms as ubound reports them
struct End { double v; bool open; int inf; };

inline bool same(const End& e, double v, bool open, bool unbounded, int side) {
	if (e.inf != 0) return unbounded && e.inf == side;
	return !unbounded && e.v == v && e.open == open;
}

// the tightest lower (upper) end for an infimum (supremum) T that is or isn't attained
template<typename Lat>
End lower_end(const Q& T, bool attained) {
	const double m = Lat::maxpos();
	if (cmp(T, { -m, 1 }) < 0) return { 0, true, -1 };
	if (cmp(T, { m, 1 }) > 0) return { m, true, 0 };
	double l = Lat::floor(T.n / T.d);
	if (l > m) l = m;
	while (cmp({ l, 1 }, T) > 0) l = Lat::prev(l);
	while (l < m && cmp({ Lat::next(l), 1 }, T) <= 0) l = Lat::next(l);
	return { l, !(attained && cmp({ l, 1 }, T) == 0), 0 };
}
template<typename Lat>
End upper_end(const Q& T, bool attained) {
	const double m = Lat::maxpos();
	if (cmp(T, { m, 1 }) > 0) return { 0, true, +1 };
	if (cmp(T, { -m, 1 }) < 0) return { -m, true, 0 };
	double u = Lat::ceil(T.n / T.d);
	if (u < -m) u = -m;
	while (cmp({ u, 1 }, T) < 0) u = Lat::next(u);
	while (u > -m && cmp({ Lat::prev(u), 1 }, T) >= 0) u = Lat::prev(u);
	return { u, !(attained && cmp({ u, 1 }, T) == 0), 0 };
}

}  // namespace oracle

// every finite tile of the lattice: the points, and the open tiles between neighbours
template<unsigned ess, unsigned fss>
std::vector<ubound<ess, fss>> FiniteTiles() {
	using L = unum_lattice<ess, fss>;
	std::vector<double> pts;
	for (double v = -L::maxpos(); v < L::maxpos(); v = L::next(v)) pts.push_back(v);
	pts.push_back(L::maxpos());
	std::vector<ubound<ess, fss>> tiles;
	for (std::size_t i = 0; i < pts.size(); ++i) {
		tiles.emplace_back(pts[i]);
		if (i + 1 < pts.size()) tiles.emplace_back((pts[i] + pts[i + 1]) / 2.0);
	}
	return tiles;
}

enum class Op { add, sub, mul, div };

// check x op y against the oracle: the hull of the four corners, each an exact rational
// that is attained or not
template<unsigned ess, unsigned fss>
int CheckBinary(const ubound<ess, fss>& x, const ubound<ess, fss>& y, Op o, bool reportTestCases) {
	using B = ubound<ess, fss>;
	using L = unum_lattice<ess, fss>;
	using namespace oracle;
	B r;
	switch (o) {
	case Op::add: r = x + y; break;
	case Op::sub: r = x - y; break;
	case Op::mul: r = x * y; break;
	case Op::div: r = x / y; break;
	}
	if (o == Op::div && y.contains(0.0)) {
		if (r.isnan()) return 0;
		if (reportTestCases) std::cerr << "FAIL: " << x << " / " << y << " should be NaN, got " << r << '\n';
		return 1;
	}
	// an end and the side the set lies on from it (+1 above a lower end, -1 below an upper end)
	struct C { double v; bool open; int side; };
	const C xs[2] = { { x.template lower<double>(), x.lower_open(), +1 }, { x.template upper<double>(), x.upper_open(), -1 } };
	const C ys[2] = { { y.template lower<double>(), y.lower_open(), +1 }, { y.template upper<double>(), y.upper_open(), -1 } };
	bool first = true;
	Q lo{ 0, 1 }, hi{ 0, 1 };
	bool lo_att = false, hi_att = false;
	bool lo_inf = false, hi_inf = false;   // the set reaches -inf / +inf
	auto take = [&](const Q& t, bool att) {
		if (first) { lo = hi = t; lo_att = hi_att = att; first = false; return; }
		const int cl = cmp(t, lo), ch = cmp(t, hi);
		if (cl < 0) { lo = t; lo_att = att; } else if (cl == 0) lo_att = lo_att || att;
		if (ch > 0) { hi = t; hi_att = att; } else if (ch == 0) hi_att = hi_att || att;
	};
	auto take_inf = [&](int sign) { if (sign < 0) lo_inf = true; else hi_inf = true; };
	if (o == Op::add || o == Op::sub) {
		// add and sub are monotone in each operand: the infimum pairs the low ends
		const C& yl = (o == Op::add) ? ys[0] : ys[1];
		const C& yh = (o == Op::add) ? ys[1] : ys[0];
		const double s = (o == Op::add) ? 1.0 : -1.0;
		take({ xs[0].v + s * yl.v, 1 }, !xs[0].open && !yl.open);
		take({ xs[1].v + s * yh.v, 1 }, !xs[1].open && !yh.open);
	}
	else {
		for (const C& a : xs) for (const C& b : ys) {
			const bool att = (!a.open || (b.v == 0.0 && !b.open)) && (!b.open || (a.v == 0.0 && !a.open));
			if (o == Op::div && b.v == 0.0) {
				// b is an open zero: near it the divisor has the sign of its side.  a / b then
				// grows without bound with sign sgn(a) * side; a closed zero numerator stays 0;
				// an open zero numerator gives every magnitude, from 0 (not attained) to inf.
				const int sa = (a.v > 0) - (a.v < 0);
				if (sa != 0) take_inf(sa * b.side);
				else if (!a.open) take({ 0, 1 }, true);
				else { take({ 0, 1 }, false); take_inf(a.side * b.side); }
				continue;
			}
			take(o == Op::mul ? Q{ a.v * b.v, 1 } : make_div(a.v, b.v), att);
		}
	}
	const End el = lo_inf ? End{ 0, true, -1 } : lower_end<L>(lo, lo_att);
	const End eh = hi_inf ? End{ 0, true, +1 } : upper_end<L>(hi, hi_att);
	const bool ok = !r.isnan()
		&& same(el, r.template lower<double>(), r.lower_open(), r.lower_unbounded(), -1)
		&& same(eh, r.template upper<double>(), r.upper_open(), r.upper_unbounded(), +1);
	if (!ok && reportTestCases) {
		static const char* name[] = { " + ", " - ", " * ", " / " };
		std::cerr << "FAIL: " << x << name[static_cast<int>(o)] << y << " = " << r << ", expected "
		          << (el.inf ? "(-inf" : (el.open ? "(" : "[")) << (el.inf ? "" : std::to_string(el.v)) << ", "
		          << (eh.inf ? "inf)" : std::to_string(eh.v) + (eh.open ? ")" : "]")) << '\n';
	}
	return ok ? 0 : 1;
}

// sqrt against the oracle: each end compares a lattice point L >= 0 with sqrt(t) through L^2
template<unsigned ess, unsigned fss>
int CheckSqrt(const ubound<ess, fss>& x, bool reportTestCases) {
	using L = unum_lattice<ess, fss>;
	const auto r = sqrt(x);
	const double xl = x.template lower<double>(), xh = x.template upper<double>();
	if (xh < 0.0 || (xh == 0.0 && x.upper_open())) {
		if (r.isnan()) return 0;
		if (reportTestCases) std::cerr << "FAIL: sqrt" << x << " should be NaN\n";
		return 1;
	}
	// sign of L - sqrt(t) for L >= 0, exactly
	auto rel = [](double l, double t) { const double s = l * l; return (s > t) - (s < t); };
	const bool lo_open = (xl < 0.0) ? false : x.lower_open();
	const double t_lo = std::max(xl, 0.0);
	// lower end: the largest lattice point <= sqrt(t_lo)
	double l = L::floor(std::sqrt(t_lo));
	while (rel(l, t_lo) > 0) l = L::prev(l);
	while (rel(L::next(l), t_lo) <= 0) l = L::next(l);
	const bool l_open = !(rel(l, t_lo) == 0 && !lo_open);
	double u = L::ceil(std::sqrt(xh));
	while (rel(u, xh) < 0) u = L::next(u);
	while (u > 0.0 && rel(L::prev(u), xh) >= 0) u = L::prev(u);
	const bool u_open = !(rel(u, xh) == 0 && !x.upper_open());
	const bool ok = !r.isnan() && r.template lower<double>() == l && r.lower_open() == l_open
		&& r.template upper<double>() == u && r.upper_open() == u_open;
	if (!ok && reportTestCases) std::cerr << "FAIL: sqrt" << x << " = " << r << ", expected " << (l_open ? "(" : "[") << l << ", " << u << (u_open ? ")" : "]") << '\n';
	return ok ? 0 : 1;
}

// all pairs of finite tiles, every operation, and sqrt of every tile
template<unsigned ess, unsigned fss>
int VerifyExhaustive(bool reportTestCases) {
	const auto tiles = FiniteTiles<ess, fss>();
	int nrOfFailedTestCases = 0;
	for (const auto& x : tiles) {
		nrOfFailedTestCases += CheckSqrt(x, reportTestCases);
		for (const auto& y : tiles) {
			for (Op o : { Op::add, Op::sub, Op::mul, Op::div }) {
				nrOfFailedTestCases += CheckBinary(x, y, o, reportTestCases && nrOfFailedTestCases < 10);
			}
		}
	}
	return nrOfFailedTestCases;
}

// random intervals: hulls of two tiles, so open and closed ends mix
template<unsigned ess, unsigned fss>
int VerifyRandom(unsigned samples, bool reportTestCases) {
	const auto tiles = FiniteTiles<ess, fss>();
	std::mt19937_64 rng(1689);
	std::uniform_int_distribution<std::size_t> pick(0, tiles.size() - 1);
	auto random_interval = [&]() { return hull(tiles[pick(rng)], tiles[pick(rng)]); };
	int nrOfFailedTestCases = 0;
	for (unsigned s = 0; s < samples; ++s) {
		const auto x = random_interval(), y = random_interval();
		for (Op o : { Op::add, Op::sub, Op::mul, Op::div }) {
			nrOfFailedTestCases += CheckBinary(x, y, o, reportTestCases && nrOfFailedTestCases < 10);
		}
		nrOfFailedTestCases += CheckSqrt(x, reportTestCases && nrOfFailedTestCases < 10);
	}
	return nrOfFailedTestCases;
}

// Point operations on unum<3,5>, against an exact integer oracle.  Here a double result
// can land on a lattice point without being the exact value (32-bit significands make
// products round, and a wide exponent spread makes sums round), so the side of each
// rounding matters.  The oracle never uses the error-free transforms: it compares a
// lattice point L with the exact result T in integer<1024> arithmetic.
namespace oracle {
using Z = integer<1024, std::uint32_t>;
struct Scaled { long long m; int e; };   // v = m * 2^e, exactly
inline Scaled decompose(double v) {
	if (v == 0.0) return { 0, 0 };
	int e = 0;
	const double f = std::frexp(v, &e);
	return { static_cast<long long>(std::ldexp(f, 53)), e - 53 };
}
// m * 2^(e - base) as an integer; base <= e
inline Z at(long long m, int e, int base) { Z z(m); z <<= (e - base); return z; }
inline int sign(const Z& z) { return z.iszero() ? 0 : (z.sign() ? -1 : 1); }

// sign(L - T) for T = a (op) b; for div, b != 0; for sqrt (o == Op::add with b ignored), see rel_sqrt
inline int rel(double L, double a, double b, Op o) {
	const Scaled l = decompose(L), x = decompose(a), y = decompose(b);
	switch (o) {
	case Op::add:
	case Op::sub: {
		const Scaled yy = (o == Op::sub) ? Scaled{ -y.m, y.e } : y;
		const int base = std::min({ l.e, x.e, yy.e });
		return sign(at(l.m, l.e, base) - at(x.m, x.e, base) - at(yy.m, yy.e, base));
	}
	case Op::mul: {
		const int base = std::min(l.e, x.e + y.e);
		Z p = Z(x.m) * Z(y.m);
		p <<= (x.e + y.e - base);
		return sign(at(l.m, l.e, base) - p);
	}
	case Op::div: {
		// sign(L - a/b) = sign(L b - a) * sign(b)
		const int base = std::min(l.e + y.e, x.e);
		Z p = Z(l.m) * Z(y.m);
		p <<= (l.e + y.e - base);
		return sign(p - at(x.m, x.e, base)) * (b > 0 ? 1 : -1);
	}
	}
	return 0;
}
// sign(L - sqrt(x)) for L >= 0, x >= 0
inline int rel_sqrt(double L, double x) {
	const Scaled l = decompose(L), v = decompose(x);
	const int base = std::min(2 * l.e, v.e);
	Z p = Z(l.m) * Z(l.m);
	p <<= (2 * l.e - base);
	return sign(p - at(v.m, v.e, base));
}
}  // namespace oracle

template<unsigned ess, unsigned fss>
int VerifyPointOps(unsigned samples, bool reportTestCases) {
	using B = ubound<ess, fss>;
	using Lat = unum_lattice<ess, fss>;
	const double m = Lat::maxpos();
	std::mt19937_64 rng(1689);
	std::uniform_int_distribution<long long> sig(1ll << Lat::maxfs, (2ll << Lat::maxfs) - 1);
	std::uniform_int_distribution<int> binade(Lat::emin, Lat::emax), near(-8, 8), coin(0, 3);
	auto point = [&]() {
		const double v = std::ldexp(static_cast<double>(sig(rng)), binade(rng) - Lat::maxfs);
		return (coin(rng) & 1) ? -v : v;
	};
	// the tightest ends for T, given sign(L - T) for any lattice point L and a starting guess
	auto ends = [&](auto relL, double guess, double& lo, bool& lo_open, int& lo_inf, double& hi, bool& hi_open, int& hi_inf) {
		lo_inf = hi_inf = 0;
		if (relL(m) < 0) { lo = m; lo_open = true; hi_inf = +1; hi_open = true; hi = m; return; }
		if (relL(-m) > 0) { lo_inf = -1; lo_open = true; lo = -m; hi = -m; hi_open = true; return; }
		double l = Lat::floor(std::clamp(guess, -m, m));
		while (relL(l) > 0) l = Lat::prev(l);
		while (l < m && relL(Lat::next(l)) <= 0) l = Lat::next(l);
		lo = l;
		lo_open = (relL(l) != 0);
		double u = Lat::ceil(std::clamp(guess, -m, m));
		while (relL(u) < 0) u = Lat::next(u);
		while (u > -m && relL(Lat::prev(u)) >= 0) u = Lat::prev(u);
		hi = u;
		hi_open = (relL(u) != 0);
	};
	auto matches = [](const B& r, double lo, bool lo_open, int lo_inf, double hi, bool hi_open, int hi_inf) {
		const bool lo_ok = lo_inf ? r.lower_unbounded() : (!r.lower_unbounded() && r.template lower<double>() == lo && r.lower_open() == lo_open);
		const bool hi_ok = hi_inf ? r.upper_unbounded() : (!r.upper_unbounded() && r.template upper<double>() == hi && r.upper_open() == hi_open);
		return !r.isnan() && lo_ok && hi_ok;
	};
	int nrOfFailedTestCases = 0;
	unsigned directed = 0;   // cases where the double result is a lattice point but not the exact value
	// pairs whose rounding error is the last term of the error-free transform: random
	// operands rarely reach these.  (1 + 2^-k)(1 - 2^-k) = 1 - 2^-2k rounds to 1.0 in double,
	// and only the lo * lo term of TwoProduct sees the -2^-2k.
	std::vector<std::pair<double, double>> fixed;
	for (int k : { Lat::maxfs, Lat::maxfs - 1, Lat::maxfs / 2 + 1 }) {
		const double e = std::ldexp(1.0, -k);
		fixed.push_back({ 1.0 + e, 1.0 - e });
		fixed.push_back({ -(1.0 + e), 1.0 - e });
		fixed.push_back({ 1.0, std::ldexp(1.0, -2 * k - 60) });   // a sum far below the last bit
	}
	for (std::size_t s = 0; s < fixed.size() + samples; ++s) {
		double a, b;
		if (s < fixed.size()) {
			a = fixed[s].first;
			b = fixed[s].second;
		}
		else {
			a = point();
			b = point();
			if (coin(rng) == 0) {
				// a power-of-two multiple of a: near cancellation and near overflow.  It has to stay
				// a lattice point in range, or B(b) would be a tile and not the point the oracle uses.
				const int k = near(rng);
				b = std::ldexp(a, k) * ((coin(rng) & 1) ? 1.0 : -1.0);
				if (std::abs(b) > m || Lat::floor(b) != b) b = point();
			}
		}
		for (Op o : { Op::add, Op::sub, Op::mul, Op::div }) {
			B r;
			double guess = 0.0;
			switch (o) {
			case Op::add: r = B(a) + B(b); guess = a + b; break;
			case Op::sub: r = B(a) - B(b); guess = a - b; break;
			case Op::mul: r = B(a) * B(b); guess = a * b; break;
			case Op::div: r = B(a) / B(b); guess = a / b; break;
			}
			double lo, hi; bool lo_open, hi_open; int lo_inf, hi_inf;
			ends([&](double L) { return oracle::rel(L, a, b, o); }, guess, lo, lo_open, lo_inf, hi, hi_open, hi_inf);
			if (std::abs(guess) <= m && Lat::floor(guess) == guess && oracle::rel(guess, a, b, o) != 0) ++directed;
			if (!matches(r, lo, lo_open, lo_inf, hi, hi_open, hi_inf)) {
				++nrOfFailedTestCases;
				static const char* name[] = { " + ", " - ", " * ", " / " };
				if (reportTestCases && nrOfFailedTestCases < 10)
					std::cerr << std::setprecision(17) << "FAIL: [" << a << ']' << name[static_cast<int>(o)] << '[' << b << "] = " << r << '\n';
			}
		}
		const double x = std::abs(a);
		const B r = sqrt(B(x));
		double lo, hi; bool lo_open, hi_open; int lo_inf, hi_inf;
		ends([&](double L) { return (L < 0.0) ? -1 : oracle::rel_sqrt(L, x); }, std::sqrt(x), lo, lo_open, lo_inf, hi, hi_open, hi_inf);
		if (!matches(r, lo, lo_open, lo_inf, hi, hi_open, hi_inf)) {
			++nrOfFailedTestCases;
			if (reportTestCases && nrOfFailedTestCases < 10) std::cerr << "FAIL: sqrt[" << x << "] = " << r << '\n';
		}
	}
	// the sweep only tests the rounding side if it meets such cases
	if (directed == 0) {
		++nrOfFailedTestCases;
		if (reportTestCases) std::cerr << "FAIL: no sample put a double result on a lattice point it is not equal to\n";
	}
	if (reportTestCases) std::cout << "  " << directed << " of " << 4 * (fixed.size() + samples) << " results needed the rounding side\n";
	return nrOfFailedTestCases;
}

// an inexact unum is the open tile beside it, away from zero
template<unsigned ess, unsigned fss>
int VerifyUnumTiles(bool reportTestCases) {
	using U = unum<ess, fss>;
	using B = ubound<ess, fss>;
	using L = unum_lattice<ess, fss>;
	int nrOfFailedTestCases = 0;
	for (double v = 0.0; v < L::maxpos(); v = L::next(v)) {
		for (double s : { 1.0, -1.0 }) {
			if (v == 0.0 && s < 0.0) continue;
			U u;
			u = s * v;
			u.setbit(0, true);
			const B t(u);
			const double other = s * L::next(v);
			const double lo = std::min(s * v, other), hi = std::max(s * v, other);
			if (!t.lower_open() || !t.upper_open() || t.template lower<double>() != lo || t.template upper<double>() != hi) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases < 10) std::cerr << "FAIL: tile of inexact " << s * v << " is " << t << '\n';
			}
		}
	}
	U top;
	top = L::maxpos();
	top.setbit(0, true);
	const B t(top);
	if (!t.upper_unbounded() || !t.lower_open() || t.template lower<double>() != L::maxpos()) {
		++nrOfFailedTestCases;
		if (reportTestCases) std::cerr << "FAIL: tile of inexact maxpos is " << t << '\n';
	}
	return nrOfFailedTestCases;
}

// unbounded ends, overflow, and divisors that touch zero
int VerifyUnbounded(bool reportTestCases) {
	using B = ubound<2, 2>;
	using L = unum_lattice<2, 2>;
	int nrOfFailedTestCases = 0;
	auto expect = [&](bool ok, const std::string& what, const B& r) {
		if (!ok) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: " << what << " = " << r << '\n'; }
	};
	const double m = L::maxpos();
	const B above(2 * m), below(-2 * m), one(1.0), zero(0.0), minus_one(-1.0);
	expect(above.lower_open() && above.template lower<double>() == m && above.upper_unbounded(), "ubound(2 maxpos)", above);
	expect(below.lower_unbounded() && below.upper_open() && below.template upper<double>() == -m, "ubound(-2 maxpos)", below);
	B r = above + one;
	expect(r.template lower<double>() == m && r.lower_open() && r.upper_unbounded(), "(maxpos, inf) + [1]", r);
	r = above * minus_one;
	expect(r.lower_unbounded() && r.template upper<double>() == -m && r.upper_open(), "(maxpos, inf) * [-1]", r);
	r = above * zero;
	expect(r.isexact() && r.template lower<double>() == 0.0, "(maxpos, inf) * [0]", r);
	r = above + below;
	expect(r.lower_unbounded() && r.upper_unbounded(), "(maxpos, inf) + (-inf, -maxpos)", r);
	r = B(m) * B(2.0);
	expect(r.template lower<double>() == m && r.lower_open() && r.upper_unbounded(), "[maxpos] * [2]", r);
	r = B(m) + B(0.0);
	expect(r.isexact() && r.template upper<double>() == m, "[maxpos] + [0]", r);
	r = one / above;
	// 1/y -> 0 as y grows, never reaching it; the supremum 1/maxpos is not attained and not
	// a lattice point: the next one up, open
	expect(r.template lower<double>() == 0.0 && r.lower_open() && r.template upper<double>() == L::ceil(1.0 / m) && r.upper_open(), "[1] / (maxpos, inf)", r);
	r = zero / above;
	expect(r.isexact() && r.template lower<double>() == 0.0, "[0] / (maxpos, inf)", r);
	r = above / above;
	expect(r.template lower<double>() == 0.0 && r.lower_open() && r.upper_unbounded(), "(maxpos, inf) / (maxpos, inf)", r);
	// a divisor with an open end at zero: unbounded on the side it approaches from
	const B pos_zero(L::next(0.0) / 2.0);    // (0, minpos)
	const B neg_zero(-L::next(0.0) / 2.0);   // (-minpos, 0)
	r = one / pos_zero;                      // 1 / minpos = 512 is past maxpos = 480
	expect(r.template lower<double>() == m && r.lower_open() && r.upper_unbounded(), "[1] / (0, minpos)", r);
	r = one / neg_zero;
	expect(r.lower_unbounded() && r.template upper<double>() == -m && r.upper_open(), "[1] / (-minpos, 0)", r);
	r = hull(B(1.0), B(2.0)) / hull(pos_zero, one);   // [1, 2] / (0, 1] = [1, inf)
	expect(r.template lower<double>() == 1.0 && !r.lower_open() && r.upper_unbounded(), "[1, 2] / (0, 1]", r);
	r = above / pos_zero;
	expect(r.template lower<double>() == m && r.lower_open() && r.upper_unbounded(), "(maxpos, inf) / (0, minpos)", r);
	r = pos_zero * above;                    // every positive magnitude
	expect(r.template lower<double>() == 0.0 && r.lower_open() && r.upper_unbounded(), "(0, minpos) * (maxpos, inf)", r);
	r = neg_zero * above;
	expect(r.lower_unbounded() && r.template upper<double>() == 0.0 && r.upper_open(), "(-minpos, 0) * (maxpos, inf)", r);
	r = one / B(zero);
	expect(r.isnan(), "[1] / [0]", r);
	B nan;
	nan = B(std::nan(""));
	expect(nan.isnan() && (nan + one).isnan() && sqrt(nan).isnan(), "NaN propagation", nan);
	expect(sqrt(minus_one).isnan(), "sqrt([-1])", sqrt(minus_one));
	r = sqrt(B(typename B::Unum(-1.0), typename B::Unum(4.0)));
	expect(r.isexact() == false && r.template lower<double>() == 0.0 && !r.lower_open() && r.template upper<double>() == 2.0 && !r.upper_open(), "sqrt([-1, 4])", r);
	r = sqrt(above);
	expect(r.upper_unbounded() && r.lower_open(), "sqrt((maxpos, inf))", r);
	return nrOfFailedTestCases;
}

// the cases from #1689 in the realistic unum<3,4>, where a double sum is not exact
int VerifyIssueCases(bool reportTestCases) {
	using B = ubound<3, 4>;
	using U = unum<3, 4>;
	using L = unum_lattice<3, 4>;
	int nrOfFailedTestCases = 0;
	auto expect = [&](bool ok, const std::string& what, const B& r) {
		if (!ok) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: " << what << " = " << r << '\n'; }
	};
	// 1 + 2^-100 rounds to 1 in double: the old ubound returned the point [1]
	B r = B(1.0) + B(std::ldexp(1.0, -100));
	expect(r.lower_open() && r.template lower<double>() == 1.0 && r.upper_open() && r.template upper<double>() == L::next(1.0), "[1] + [2^-100]", r);
	r = B(1.0) - B(std::ldexp(1.0, -100));
	expect(r.template lower<double>() == L::prev(1.0) && r.upper_open() && r.template upper<double>() == 1.0, "[1] - [2^-100]", r);
	// sqrt(2): the open tile between two neighbours, lo^2 < 2 < hi^2
	r = sqrt(B(2.0));
	const double lo = r.template lower<double>(), hi = r.template upper<double>();
	expect(r.lower_open() && r.upper_open() && L::next(lo) == hi && lo * lo < 2.0 && hi * hi > 2.0, "sqrt([2])", r);
	expect(sqrt(B(2.25)).isexact() && sqrt(B(2.25)).template lower<double>() == 1.5, "sqrt([2.25])", sqrt(B(2.25)));
	// 1/3, both signs
	r = B(1.0) / B(3.0);
	expect(r.contains(1.0 / 3.0) && L::next(r.template lower<double>()) == r.template upper<double>(), "[1] / [3]", r);
	r = B(-1.0) / B(3.0);
	expect(r.contains(-1.0 / 3.0) && L::next(r.template lower<double>()) == r.template upper<double>(), "[-1] / [3]", r);
	// next_exact steps one lattice point, not one binade
	U one(1.0);
	expect(next_exact(one).to_double() == 1.0 + std::ldexp(1.0, -15) && prev_exact(one).to_double() == 1.0 - std::ldexp(1.0, -16), "next/prev_exact(1)", B(next_exact(one)));
	// the quadratic-formula pieces of #196: b^2 - 4ac and its root, enclosing the exact values
	const B a(3.0), b(100.0), c(2.0);
	const B disc = b * b - B(4.0) * a * c;
	expect(disc.isexact() && disc.template lower<double>() == 9976.0, "b^2 - 4ac", disc);
	const B root = sqrt(disc);
	expect(root.contains(std::sqrt(9976.0)) && !root.isexact(), "sqrt(9976)", root);
	return nrOfFailedTestCases;
}

// ubound satisfies the ubox_trace contract
int VerifyUboxTrace(bool reportTestCases) {
	using B = ubound<3, 4>;
	const ubox_box box = make_ubox(B(1.0) / B(3.0));
	const bool ok = !box.nan && box.lo_open && box.hi_open && !box.exact && box.lo < box.hi && box.contains(1.0L / 3.0L);
	if (!ok && reportTestCases) std::cerr << "FAIL: make_ubox(ubound) " << box.text << '\n';
	return ok ? 0 : 1;
}

}}  // namespace sw::universal

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

	std::string test_suite  = "unum Type I ubound enclosure (#1689)";
	std::string test_tag    = "ubound";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIssueCases(reportTestCases), "ubound<3,4>", "issue cases");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLatticeModel<2, 2>(reportTestCases), "unum<2,2>", "lattice model");
	nrOfFailedTestCases += ReportTestResult(VerifyLatticeModel<2, 3>(reportTestCases), "unum<2,3>", "lattice model");
	nrOfFailedTestCases += ReportTestResult(VerifyLatticeModel<3, 2>(reportTestCases), "unum<3,2>", "lattice model");
	nrOfFailedTestCases += ReportTestResult(VerifyUnumTiles<2, 2>(reportTestCases), "ubound<2,2>", "inexact unum tiles");
	nrOfFailedTestCases += ReportTestResult(VerifyUnumTiles<3, 4>(reportTestCases), "ubound<3,4>", "inexact unum tiles");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<2, 2>(reportTestCases), "ubound<2,2>", "exhaustive + - * / sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyUnbounded(reportTestCases), "ubound<2,2>", "unbounded and overflow");
	nrOfFailedTestCases += ReportTestResult(VerifyIssueCases(reportTestCases), "ubound<3,4>", "issue cases");
	nrOfFailedTestCases += ReportTestResult(VerifyPointOps<3, 5>(2000, reportTestCases), "ubound<3,5>", "point ops, exact oracle");
	nrOfFailedTestCases += ReportTestResult(VerifyUboxTrace(reportTestCases), "ubound<3,4>", "ubox_trace contract");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<2, 3>(20000, reportTestCases), "ubound<2,3>", "random + - * / sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyPointOps<3, 5>(20000, reportTestCases), "ubound<3,5>", "point ops, exact oracle");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<2, 3>(200000, reportTestCases), "ubound<2,3>", "random + - * / sqrt");
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
catch (const sw::universal::unum_arithmetic_exception& err) {
	std::cerr << "Caught unum arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
