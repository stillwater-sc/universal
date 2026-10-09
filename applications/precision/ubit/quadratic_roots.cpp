// quadratic_roots.cpp: provably bound the roots of 3x^2 + 100x + 2 = 0 -- the dependency problem
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
//
// #1646.  The quadratic formula x = (-b +- sqrt(b^2 - 4ac)) / (2a) at a = 3, b = 100, c = 2
// (The End of Error, pp. 181-184).  The roots are
//     r1 = (-100 + sqrt(9976)) / 6 = -0.020012014421636353441812506440...
//     r2 = (-100 - sqrt(9976)) / 6 = -33.313321318911696979891520826893...
// Two things make it hard:
//   - cancellation: -b + sqrt(b^2 - 4ac) subtracts two numbers near 100 to get about -0.12.
//     The rearranged form r1 = 2c / (-b - sqrt(b^2 - 4ac)) is algebraically equal and adds
//     instead; "as written" below means the formula in its usual form.
//   - the dependency problem: a and b occur twice.  Interval arithmetic treats each
//     occurrence as an independent variable, so the bound can be wider than the true range.
//     With exact (point) operands the occurrences are the same number and nothing is lost;
//     with ULP-wide operands the repeated occurrences start to cost.
//
// Every enclosure is checked EXACTLY, in einteger arithmetic.  The endpoints are dyadic
// rationals m 2^e, and so are the coefficients, so the sign of q(x) = a x^2 + b x + c and of
// q'(x) = 2a x + b is exact.  With a > 0 the vertex -b/(2a) separates the roots:
//     x >= r1  <=>  q'(x) > 0 and q(x) >= 0          x <= r2  <=>  q'(x) < 0 and q(x) >= 0
// The same test drives a bisection over the tiles, which gives the tightest enclosure a
// format can state.  Widths are counted in tiles: lattice points and the open intervals
// between them, so n tiles span about n/2 ulps.  For ULP-wide operands each root is monotone in every coefficient
// (dr/dc = -1/q'(r) and its relatives have constant signs), so its range is spanned by the
// roots of the eight corner polynomials.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/einteger/einteger.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/tile_interval.hpp>

namespace {

using Big = sw::universal::einteger<std::uint32_t>;   // exact integers for the sign tests
using sw::universal::tile_interval;
using sw::universal::tile_traits;

constexpr double r1_ref = -0.020012014421636353441812506440166613377;   // to double precision
constexpr double r2_ref = -33.313321318911696979891520826893166720;

// an exact dyadic rational m 2^e
struct dyadic { Big m; int e; };

dyadic from_double(double v) {
	int e = 0;
	const double f = std::frexp(v, &e);                                   // v = f 2^e, |f| in [0.5, 1)
	const long long m = static_cast<long long>(std::ldexp(f, 53));        // exact: 53 bits
	return { Big(m), e - 53 };
}
template<unsigned nbits, unsigned es, typename bt>
dyadic exact_value(const sw::universal::areal<nbits, es, bt>& t) { return from_double(double(t)); }   // <= 52 fraction bits: exact
template<unsigned nbits, unsigned es, typename bt>
dyadic exact_value(const sw::universal::poxel<nbits, es, bt>& t) {
	using P = sw::universal::poxel<nbits, es, bt>;
	const std::int64_t L = t.lattice();
	if (L == 0) return { Big(0), 0 };
	const auto d = P::decode_lattice(L);                                  // (-1)^neg (2^nf + frac) 2^(scale - nf)
	const unsigned long long m = (1ull << d.nf) | d.frac;
	Big em(m);
	if (d.negative) em = -em;
	return { em, d.scale - static_cast<int>(d.nf) };
}

// sum of terms m_i 2^{e_i}, exactly; returns its sign
int sign_of(std::initializer_list<dyadic> terms) {
	int emin = 0;
	bool first = true;
	for (const dyadic& t : terms) if (!t.m.iszero()) { emin = first ? t.e : std::min(emin, t.e); first = false; }
	Big s(0);
	for (const dyadic& t : terms) {
		if (t.m.iszero()) continue;
		Big v = t.m;
		v <<= (t.e - emin);
		s += v;
	}
	return s.iszero() ? 0 : (s.isneg() ? -1 : 1);
}
dyadic mul(const dyadic& x, const dyadic& y) { Big m = x.m; m *= y.m; return { m, x.e + y.e }; }

// q(x) = a x^2 + b x + c and q'(x) = 2a x + b, signs only
struct quadratic {
	dyadic a, b, c;
	int q(const dyadic& x) const { return sign_of({ mul(a, mul(x, x)), mul(b, x), c }); }
	int dq(const dyadic& x) const { return sign_of({ mul({ Big(2), 0 }, mul(a, x)), b }); }
	bool at_or_above_r1(const dyadic& x) const { return dq(x) > 0 && q(x) >= 0; }
	bool at_or_below_r2(const dyadic& x) const { return dq(x) < 0 && q(x) >= 0; }
};

// the lattice points that bound an interval, as exact tiles; nullopt-like flags for +-inf
template<typename Tile>
struct ends {
	bool lo_inf, hi_inf;
	Tile lo, hi;
};
template<typename Tile>
ends<Tile> lattice_ends(const tile_interval<Tile>& x) {
	using T = tile_traits<Tile>;
	const std::int64_t K = tile_interval<Tile>::kmax;
	ends<Tile> r{ x.lo_key() == -K, x.hi_key() == K, Tile{}, Tile{} };
	if (!r.lo_inf) r.lo = T::tile((x.lo_key() & 1) ? x.lo_key() - 1 : x.lo_key());
	if (!r.hi_inf) r.hi = T::tile((x.hi_key() & 1) ? x.hi_key() + 1 : x.hi_key());
	return r;
}

// does the enclosure contain the small root (which = 1) or the large root (which = 2) of q?
template<typename Tile>
bool contains_root(const tile_interval<Tile>& x, const quadratic& p, int which) {
	if (x.isnan()) return false;
	const auto e = lattice_ends(x);
	if (which == 1) {
		const bool lo_ok = e.lo_inf || !p.at_or_above_r1(exact_value(e.lo));   // lo <= r1 (r1 is irrational)
		const bool hi_ok = e.hi_inf || p.at_or_above_r1(exact_value(e.hi));
		return lo_ok && hi_ok;
	}
	const bool lo_ok = e.lo_inf || p.at_or_below_r2(exact_value(e.lo));
	const bool hi_ok = e.hi_inf || !p.at_or_below_r2(exact_value(e.hi));
	return lo_ok && hi_ok;
}

// the key of the open tile that contains a root, by bisection over the lattice
template<typename Tile>
std::int64_t root_tile(const quadratic& p, int which) {
	using T = tile_traits<Tile>;
	const std::int64_t K = tile_interval<Tile>::kmax;
	std::int64_t lo = -(K - 1) / 2, hi = (K - 1) / 2;   // half-keys of lattice points
	// largest lattice point t below the root: for r1 "not at or above", for r2 "at or below"
	auto below = [&](std::int64_t h) { const dyadic x = exact_value(T::tile(2 * h)); return which == 1 ? !p.at_or_above_r1(x) : p.at_or_below_r2(x); };
	while (lo < hi) {
		const std::int64_t mid = lo + (hi - lo + 1) / 2;
		if (below(mid)) lo = mid; else hi = mid - 1;
	}
	return 2 * lo + 1;
}

template<typename Real> Real as_written_r1(const Real& a, const Real& b, const Real& c, Real (*sq)(const Real&)) { return (-b + sq(b * b - Real(4) * a * c)) / (Real(2) * a); }
template<typename Real> Real as_written_r2(const Real& a, const Real& b, const Real& c, Real (*sq)(const Real&)) { return (-b - sq(b * b - Real(4) * a * c)) / (Real(2) * a); }
template<typename Real> Real rearranged_r1(const Real& a, const Real& b, const Real& c, Real (*sq)(const Real&)) { return (Real(2) * c) / (-b - sq(b * b - Real(4) * a * c)); }

int fails = 0;
void check(bool ok, const std::string& what) {
	std::cout << (ok ? "  ok    " : "  FAIL  ") << what << '\n';
	if (!ok) ++fails;
}

std::string relerr(double x, double ref) {
	const double e = std::abs((x - ref) / ref);
	std::stringstream s;
	s << std::scientific << std::setprecision(1);
	if (e < 1.0e-15) s << "< 1e-15"; else s << e;
	return s.str();
}

template<typename Real>
void rounded(const char* name, Real (*sq)(const Real&)) {
	const Real a(3), b(100), c(2);
	const double t1 = double(as_written_r1(a, b, c, sq)), s1 = double(rearranged_r1(a, b, c, sq)), t2 = double(as_written_r2(a, b, c, sq));
	std::cout << std::setw(14) << name << "   r1 as written " << std::setw(9) << relerr(t1, r1_ref) << "   r1 rearranged " << std::setw(9) << relerr(s1, r1_ref)
	          << "   r2 " << std::setw(9) << relerr(t2, r2_ref) << '\n';
}

template<typename Tile>
struct report {
	tile_interval<Tile> t1, s1, t2;      // r1 as written, r1 rearranged, r2 as written
	std::int64_t best1, best2;           // the tiles that contain r1 and r2
	tile_interval<Tile> u1, us1, u2;     // the same three with ULP-wide a, b, c
	std::int64_t ubest1_lo, ubest1_hi, ubest2_lo, ubest2_hi;
	bool all_contain;                    // every enclosure contains every root it should
};

std::int64_t tiles(std::int64_t lo, std::int64_t hi) { return hi - lo + 1; }
template<typename Tile> std::int64_t tiles(const tile_interval<Tile>& x) { return x.isnan() ? -1 : tiles(x.lo_key(), x.hi_key()); }

template<typename Tile>
report<Tile> enclose(const char* name, int digits) {
	using I = tile_interval<Tile>;
	using T = tile_traits<Tile>;
	auto sq = [](const I& v) { return sqrt(v); };
	I (*sqf)(const I&) = sq;
	report<Tile> r{};
	const quadratic exact{ exact_value(Tile(3)), exact_value(Tile(100)), exact_value(Tile(2)) };

	// exact operands
	r.t1 = as_written_r1(I(3), I(100), I(2), sqf);
	r.s1 = rearranged_r1(I(3), I(100), I(2), sqf);
	r.t2 = as_written_r2(I(3), I(100), I(2), sqf);
	r.best1 = root_tile<Tile>(exact, 1);
	r.best2 = root_tile<Tile>(exact, 2);

	// ULP-wide operands: the open tile just above each coefficient
	auto above = [](int v) { const std::int64_t k = T::key(Tile(v)); return I::from_keys(k + 1, k + 1); };
	const I A = above(3), B = above(100), C = above(2);
	r.u1 = as_written_r1(A, B, C, sqf);
	r.us1 = rearranged_r1(A, B, C, sqf);
	r.u2 = as_written_r2(A, B, C, sqf);
	// the corner polynomials, from the lattice points that bound each coefficient
	std::array<dyadic, 2> as{ exact_value(T::tile(T::key(Tile(3)))), exact_value(T::tile(T::key(Tile(3)) + 2)) };
	std::array<dyadic, 2> bs{ exact_value(T::tile(T::key(Tile(100)))), exact_value(T::tile(T::key(Tile(100)) + 2)) };
	std::array<dyadic, 2> cs{ exact_value(T::tile(T::key(Tile(2)))), exact_value(T::tile(T::key(Tile(2)) + 2)) };
	r.ubest1_lo = r.ubest2_lo = I::kmax;
	r.ubest1_hi = r.ubest2_hi = -I::kmax;
	bool corners_contained = true;
	for (const dyadic& ca : as) for (const dyadic& cb : bs) for (const dyadic& cc : cs) {
		const quadratic p{ ca, cb, cc };
		const std::int64_t k1 = root_tile<Tile>(p, 1), k2 = root_tile<Tile>(p, 2);
		r.ubest1_lo = std::min(r.ubest1_lo, k1); r.ubest1_hi = std::max(r.ubest1_hi, k1);
		r.ubest2_lo = std::min(r.ubest2_lo, k2); r.ubest2_hi = std::max(r.ubest2_hi, k2);
		corners_contained = corners_contained && contains_root(r.u1, p, 1) && contains_root(r.us1, p, 1) && contains_root(r.u2, p, 2);
	}
	// and, as a cross-check of the bisection, the tightest tiles lie inside every enclosure
	auto holds = [](const I& x, std::int64_t lo, std::int64_t hi) { return !x.isnan() && x.lo_key() <= lo && hi <= x.hi_key(); };
	const bool tight_inside = holds(r.t1, r.best1, r.best1) && holds(r.s1, r.best1, r.best1) && holds(r.t2, r.best2, r.best2)
	                       && holds(r.u1, r.ubest1_lo, r.ubest1_hi) && holds(r.us1, r.ubest1_lo, r.ubest1_hi) && holds(r.u2, r.ubest2_lo, r.ubest2_hi);
	r.all_contain = contains_root(r.t1, exact, 1) && contains_root(r.s1, exact, 1) && contains_root(r.t2, exact, 2) && corners_contained && tight_inside;

	std::cout << name << '\n';
	std::cout << "    exact a, b, c       r1 as written  " << std::setw(46) << std::left << r.t1.str(digits) << std::right << std::setw(8) << tiles(r.t1) << " tiles\n";
	std::cout << "                        r1 rearranged  " << std::setw(46) << std::left << r.s1.str(digits) << std::right << std::setw(8) << tiles(r.s1) << " tiles\n";
	std::cout << "                        r2             " << std::setw(46) << std::left << r.t2.str(digits) << std::right << std::setw(8) << tiles(r.t2) << " tiles   (tightest: 1 tile each)\n";
	std::cout << "    ULP-wide a, b, c    r1 as written  " << std::setw(46) << std::left << r.u1.str(digits) << std::right << std::setw(8) << tiles(r.u1) << " tiles\n";
	std::cout << "                        r1 rearranged  " << std::setw(46) << std::left << r.us1.str(digits) << std::right << std::setw(8) << tiles(r.us1) << " tiles   (tightest: " << tiles(r.ubest1_lo, r.ubest1_hi) << ")\n";
	std::cout << "                        r2             " << std::setw(46) << std::left << r.u2.str(digits) << std::right << std::setw(8) << tiles(r.u2) << " tiles   (tightest: " << tiles(r.ubest2_lo, r.ubest2_hi) << ")\n";
	return r;
}

template<typename Tile>
void single_tile(const char* name) {
	Tile (*sq)(const Tile&) = sw::universal::tile_sqrt<Tile>;
	const Tile a(3), b(100), c(2);
	const Tile t1 = as_written_r1(a, b, c, sq), t2 = as_written_r2(a, b, c, sq);
	auto ubit = [](const Tile& t) { return (tile_traits<Tile>::key(t) & 1) != 0; };
	std::cout << std::setw(14) << name << "   r1 " << std::setw(24) << std::setprecision(10) << double(t1) << (ubit(t1) ? " u=1" : " u=0")
	          << "   r2 " << std::setw(16) << double(t2) << (ubit(t2) ? " u=1" : " u=0") << '\n';
}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	using areal16 = areal<16, 5, std::uint16_t>;
	using areal32 = areal<32, 8, std::uint32_t>;
	using areal64 = areal<64, 11, std::uint64_t>;
	using poxel16 = poxel<16, 2, std::uint16_t>;
	using poxel32 = poxel<32, 2, std::uint32_t>;
	using poxel64 = poxel<64, 2, std::uint64_t>;

	std::cout << "roots of 3x^2 + 100x + 2 = 0 by the quadratic formula\n";
	std::cout << "  r1 = -0.020012014421636353441812506440...   r2 = -33.313321318911696979891520826893...\n\n";

	std::cout << "rounding formats, paired by width: relative error of r1 as written (-b + sqrt(b^2 - 4ac)) / (2a),\n";
	std::cout << "of r1 rearranged as 2c / (-b - sqrt(b^2 - 4ac)), and of r2\n";
	std::cout << "  16 bits\n";
	rounded<half>("half", [](const half& v) { return sqrt(v); });
	rounded<posit<16, 2>>("posit<16,2>", [](const posit<16, 2>& v) { return sqrt(v); });
	std::cout << "  32 bits\n";
	rounded<float>("float", [](const float& v) { return std::sqrt(v); });
	rounded<posit<32, 2>>("posit<32,2>", [](const posit<32, 2>& v) { return sqrt(v); });
	std::cout << "  64 bits\n";
	rounded<double>("double", [](const double& v) { return std::sqrt(v); });
	std::cout << "                 (no posit<64,2> peer: the library's posit sqrt is computed through double by default)\n";
	{
		// why posit<16,2> loses at 16 bits: b^2 = 10000 sits where its regime has used 5 bits
		const posit<16, 2> b(100), bb = b * b;
		const half hb(100), hbb = hb * hb;
		std::cout << "  near 1e4 posit<16,2> keeps 8 fraction bits to half's 10: b^2 = 10000 is stored as "
		          << double(bb) << " (half: " << double(hbb) << ")\n";
	}

	std::cout << "\nsingle tiles (sticky ubit), formula as written:\n";
	single_tile<areal32>("areal<32,8>");
	single_tile<poxel32>("poxel<32,2>");

	std::cout << "\ntile intervals (enclosures), equal storage:\n";
	const auto a16 = enclose<areal16>("  areal<16,5>", 6);
	const auto p16 = enclose<poxel16>("  poxel<16,2>", 6);
	const auto a32 = enclose<areal32>("  areal<32,8>", 9);
	const auto p32 = enclose<poxel32>("  poxel<32,2>", 9);
	const auto a64 = enclose<areal64>("  areal<64,11>", 17);
	const auto p64 = enclose<poxel64>("  poxel<64,2>", 17);

	{
		const areal32 a(3), b(100), c(2);
		const areal32 t1 = as_written_r1(a, b, c, &tile_sqrt<areal32>);
		const poxel32 x(3), y(100), z(2);
		const poxel32 u1 = as_written_r1(x, y, z, &tile_sqrt<poxel32>);
		std::cout << "\nassertions:\n";
		check(t1.at(0) && u1.ubit(), "both single tiles flag r1 as inexact: sqrt(9976) is irrational");
	}
	check(a16.all_contain && p16.all_contain && a32.all_contain && p32.all_contain && a64.all_contain && p64.all_contain,
	      "every enclosure contains its root -- for exact a, b, c and for every corner of the ULP-wide a, b, c (checked exactly)");
	auto rearranged_no_wider = [](const auto& r) { return tiles(r.s1) <= tiles(r.t1) && tiles(r.us1) <= tiles(r.u1); };
	check(rearranged_no_wider(a16) && rearranged_no_wider(p16) && rearranged_no_wider(a32) && rearranged_no_wider(p32) && rearranged_no_wider(a64) && rearranged_no_wider(p64),
	      "the rearranged r1 is never wider than r1 as written");
	auto rearranged_tight = [](const auto& r) { return tiles(r.s1) <= 3; };
	check(rearranged_tight(a32) && rearranged_tight(p32) && rearranged_tight(a64) && rearranged_tight(p64),
	      "with exact a, b, c the rearranged r1 spans at most 3 tiles at 32 and 64 bits; the tightest is 1");
	auto cancellation_costs = [](const auto& r) { return tiles(r.t1) > 100 * tiles(r.s1); };
	check(cancellation_costs(a32) && cancellation_costs(p32) && cancellation_costs(a64) && cancellation_costs(p64),
	      "cancellation: r1 as written is more than 100 times wider than the rearranged r1, even for exact a, b, c");
	auto widens = [](const auto& r) { return tiles(r.u1) > tiles(r.t1); };
	check(widens(a16) && widens(p16) && widens(a32) && widens(p32) && widens(a64) && widens(p64),
	      "dependency: ULP-wide a, b, c widen r1 as written, where a and b occur twice, beyond the exact-operand case");
	auto dependency_costs = [](const auto& r) { return tiles(r.us1) > tiles(r.ubest1_lo, r.ubest1_hi); };
	check(dependency_costs(a32) && dependency_costs(p32) && dependency_costs(a64) && dependency_costs(p64),
	      "dependency: at 32 and 64 bits even the rearranged r1 is wider than the tightest enclosure of the root's range");
	check(p16.t1.sign() == tile_verdict::undecidable && a16.t1.sign() == tile_verdict::negative,
	      "at 16 bits r1 as written of poxel<16,2> cannot decide its sign; areal<16,5>'s is negative but 10000+ tiles wide");
	check(p16.s1.sign() == tile_verdict::negative && a16.s1.sign() == tile_verdict::negative, "the rearranged r1 at 16 bits is provably negative");

	std::cout << (fails == 0 ? "PASS\n" : "FAIL\n");
	return (fails == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
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
