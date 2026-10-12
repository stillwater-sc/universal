// wide.cpp: poxels wider than 64 bits -- the lattices of posit<64, 2>, posit<128, 2> and posit<256, 2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Beyond 64 bits the poxel codec works in integer<nbits + 2> instead of native words (#1676).
// Sampled, for poxel<65,2>, poxel<129,2> and poxel<257,2>:
//   - lattice: every lattice point's value, as double and as long double, is the nearest one
//     to the exact value; tiles are ordered, exactly; negation mirrors a tile
//   - conversion: a double or long double lands in the one tile that contains it, exactly
//   - posit interop: a posit converts to the exact tile at its value and back
//   - arithmetic: exact operands give the tile the exact dyadic oracle finds for the exact
//     result, and the same operation in posit<nbits - 1, 2> lands on one end of that tile
//   - order: tile order is the order of the tiles on the real line
//   - tile_interval: exact-operand intervals are the oracle's tile, and wider ones enclose
// The exact reference is tile_oracle.hpp: exact dyadics with an independent bisection locate.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/tile_oracle.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
namespace orc = sw::universal::oracle;

static_assert(std::is_trivially_constructible_v<poxel<65, 2, std::uint32_t>>, "poxel<65,2> is trivially constructible");
static_assert(std::is_trivially_constructible_v<poxel<129, 2, std::uint32_t>>, "poxel<129,2> is trivially constructible");
static_assert(std::is_trivially_constructible_v<poxel<257, 2, std::uint32_t>>, "poxel<257,2> is trivially constructible");
static_assert(std::is_trivially_copyable_v<poxel<129, 2, std::uint32_t>>, "poxel<129,2> is trivially copyable");

// a random tile; exact = true clears the ubit
template<typename X>
X random_tile(std::mt19937_64& rng, bool exact) {
	X x;
	x.setbits(rng());
	for (unsigned i = 64; i < X::nbits; ++i) x.setbit(i, (rng() & 1u) != 0u);
	if (exact) x.setubit(false);
	return x;
}

// the exact value of a long double, as a sum of doubles
orc::dyadic exact(long double v) {
	orc::dyadic s{};
	for (int i = 0; i < 4 && v != 0.0l; ++i) {
		const double d = static_cast<double>(v);
		s = orc::add(s, orc::from_double(d));
		v -= static_cast<long double>(d);
	}
	return s;
}

// is r the Real nearest to v (either one on a tie)?
template<typename Real>
bool nearest(Real r, const orc::dyadic& v) {
	const orc::dyadic d = orc::sub(exact(static_cast<long double>(r)), v);
	for (Real n : { std::nextafter(r, -std::numeric_limits<Real>::infinity()), std::nextafter(r, std::numeric_limits<Real>::infinity()) }) {
		if (!std::isfinite(n)) continue;
		const orc::dyadic dn = orc::sub(exact(static_cast<long double>(n)), v);
		if (orc::compare(orc::mul(dn, dn), orc::mul(d, d)) < 0) return false;
	}
	return true;
}

// the exact ends of a tile; the infinite end of an end tile is reported through *inf
template<typename X>
orc::dyadic lower_end(const X& x, bool* inf) {
	*inf = (x.lattice() == X::lattice_nar);
	return *inf ? orc::dyadic{} : orc::exact_value(x);
}
template<typename X>
orc::dyadic upper_end(const X& x, bool* inf) {
	*inf = false;
	if (x.isexact()) return orc::exact_value(x);
	if (x.lattice() == X::lattice_maxpos) { *inf = true; return {}; }
	X next(x); ++next;                                    // the point tile that ends it
	return orc::exact_value(next);
}
// is v inside tile x: equal to an exact tile, strictly between the ends of an open one
template<typename X>
bool contains(const X& x, const orc::dyadic& v) {
	bool li = false, hi = false;
	const orc::dyadic lo = lower_end(x, &li), up = upper_end(x, &hi);
	if (x.isexact()) return orc::compare(lo, v) == 0;
	return (li || orc::compare(lo, v) < 0) && (hi || orc::compare(v, up) < 0);
}

template<unsigned nbits>
int VerifyLattice(unsigned samples, bool report) {
	using X = poxel<nbits, 2, std::uint32_t>;
	std::mt19937_64 rng(0x1676u + nbits);
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << type_tag(X{}) << ": " << what << '\n'; };
	for (unsigned i = 0; i < samples; ++i) {
		const X x = random_tile<X>(rng, false);
		if (x.isnar()) continue;
		// the lower end, rounded to nearest
		if (x.lattice() != X::lattice_nar) {
			const orc::dyadic v = orc::exact_value(x);
			if (!nearest(x.template lower<double>(), v)) fail("lower<double> of " + to_interval(x) + " is not the nearest double");
			if (!nearest(x.template lower<long double>(), v)) fail("lower<long double> of " + to_interval(x) + " is not the nearest long double");
		}
		// order, exactly: lower <= upper, and the next tile starts where this one ends
		bool li = false, hi = false, ni = false;
		const orc::dyadic lo = lower_end(x, &li), up = upper_end(x, &hi);
		if (!li && !hi && (x.isexact() ? orc::compare(lo, up) != 0 : orc::compare(lo, up) >= 0)) fail("tile " + to_interval(x) + " is not a point or an open interval");
		X next(x); ++next;
		if (!next.isnar() && !hi) {
			const orc::dyadic nlo = lower_end(next, &ni);
			if (orc::compare(nlo, up) != 0) fail("the tile after " + to_interval(x) + " does not start at its end");
		}
		// negation mirrors the tile
		const X n = -x;
		bool nli = false, nhi = false;
		const orc::dyadic nlo = lower_end(n, &nli), nup = upper_end(n, &nhi);
		if (n.ubit() != x.ubit() || nli != hi || nhi != li || (!nli && orc::compare(nlo, orc::neg(up)) != 0) || (!nhi && orc::compare(nup, orc::neg(lo)) != 0)) fail("negation of " + to_interval(x));
	}
	return fails;
}

template<unsigned nbits>
int VerifyConversion(unsigned samples, bool report) {
	using X = poxel<nbits, 2, std::uint32_t>;
	std::mt19937_64 rng(0x1677u + nbits);
	std::uniform_real_distribution<double> u(1.0, 2.0);
	std::uniform_int_distribution<int> e(-1100, 1100);
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << type_tag(X{}) << ": " << what << '\n'; };
	for (unsigned i = 0; i < samples; ++i) {
		const double d = std::ldexp(u(rng), e(rng)) * ((rng() & 1u) ? -1.0 : 1.0);
		if (d == 0.0 || !std::isfinite(d)) continue;
		if (!contains(X(d), orc::from_double(d))) fail("double " + std::to_string(d) + " is not in its tile " + to_interval(X(d)));
		// a long double with more bits than the double
		const long double ld = static_cast<long double>(d) * (1.0l + std::ldexp(static_cast<long double>(rng() >> 11), -std::numeric_limits<long double>::digits + 1) / 3.0l);
		if (std::isfinite(ld) && ld != 0.0l && !contains(X(ld), exact(ld))) fail("long double near " + std::to_string(d) + " is not in its tile");
	}
	// integers, and the end tiles
	for (long long k : { 1LL, -1LL, 3LL, 1000003LL, -(1LL << 62) - 1, (1LL << 62) + 12345LL }) {
		if (!contains(X(k), orc::make_dyadic(k))) fail("integer " + std::to_string(k));
	}
	if (X(std::numeric_limits<double>::infinity()) != X(SpecificValue::infpos)) fail("+inf is (maxpos, inf)");
	if (X(-std::numeric_limits<double>::infinity()) != X(SpecificValue::infneg)) fail("-inf is (-inf, -maxpos)");
	if (!X(std::nan("")).isnar() || !X(0.0).iszero()) fail("NaN and zero");
	return fails;
}

template<unsigned nbits>
int VerifyPositInterop(unsigned samples, bool report) {
	using X = poxel<nbits, 2, std::uint32_t>;
	using P = posit<nbits - 1, 2, std::uint32_t>;
	std::mt19937_64 rng(0x1678u + nbits);
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << type_tag(X{}) << ": " << what << '\n'; };
	for (unsigned i = 0; i < samples; ++i) {
		const X x = random_tile<X>(rng, false);
		const P p = P(x);
		if (x.isnar() || x.lattice() == X::lattice_nar) { if (!p.isnar()) fail("NaR and (-inf, -maxpos) go to NaR"); continue; }
		X point(x); point.setubit(false);
		if (!(X(p) == point)) fail("posit round trip of " + to_interval(x));
		if (!X(p).isexact() || !(P(X(p)) == p)) fail("posit to poxel to posit");
	}
	if (!X(P(SpecificValue::nar)).isnar() || !X(P(0)).iszero() || !(P(X(SpecificValue::maxpos)) == P(SpecificValue::maxpos))) fail("special values");
	return fails;
}

enum class Op { add, sub, mul, div };

template<unsigned nbits>
int VerifyArithmetic(unsigned samples, bool report) {
	using X = poxel<nbits, 2, std::uint32_t>;
	using P = posit<nbits - 1, 2, std::uint32_t>;
	using I = tile_interval<X>;
	std::mt19937_64 rng(0x1679u + nbits);
	int fails = 0;
	for (unsigned i = 0; i < samples; ++i) {
		X a = random_tile<X>(rng, true), b = random_tile<X>(rng, true);
		if (a.isnar() || b.isnar()) continue;
		// half the time, operands of similar size, so that sums cancel and products stay in range
		if (i % 2) { a = X(std::ldexp(static_cast<double>(rng() >> 11), -50)) ; a.setubit(false); }
		const orc::interval A = orc::point(orc::exact_value(a)), B = orc::point(orc::exact_value(b));
		for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) {
			if (op == Op::div && b.iszero()) continue;
			X r; P pr(0); orc::interval ref;
			switch (op) {
			case Op::add: r = a + b; pr = P(a) + P(b); ref = orc::add(A, B); break;
			case Op::sub: r = a - b; pr = P(a) - P(b); ref = orc::sub(A, B); break;
			case Op::mul: r = a * b; pr = P(a) * P(b); ref = orc::mul(A, B); break;
			case Op::div: r = a / b; pr = P(a) / P(b); break;
			}
			// the oracle's tile, refining the precision of a quotient until it is one tile
			I box;
			if (op == Op::div) {
				for (int prec = 64; prec <= 4096; prec *= 2) { box = orc::box<X>(orc::div(A, B, prec)); if (box.lo_key() == box.hi_key()) break; }
			}
			else box = orc::box<X>(ref);
			const char* name = op == Op::add ? " + " : op == Op::sub ? " - " : op == Op::mul ? " * " : " / ";
			const std::string where = to_interval(a) + name + to_interval(b);
			if (box.isnan() || box.lo_key() != box.hi_key() || !(r.key() == box.lo_key())) {
				++fails;
				if (report && fails < 10) std::cerr << "FAIL " << type_tag(X{}) << ' ' << where << " = " << to_interval(r) << ", oracle " << box.str() << '\n';
				continue;
			}
			// posit rounds to nearest: one end of the tile
			X up(r);
			if (!r.isexact()) ++up;
			if (!(pr == P(r)) && !(pr == P(up))) {
				++fails;
				if (report && fails < 10) std::cerr << "FAIL " << type_tag(X{}) << ' ' << where << ": posit's rounding is not an end of " << to_interval(r) << '\n';
			}
		}
	}
	return fails;
}

template<unsigned nbits>
int VerifyOrder(unsigned samples, bool report) {
	using X = poxel<nbits, 2, std::uint32_t>;
	std::mt19937_64 rng(0x167Au + nbits);
	int fails = 0;
	// position on the line: the lower end (-inf for the bottom tile), a point before the open tile above it
	auto before = [](const X& a, const X& b) {
		bool ai = false, bi = false;
		const orc::dyadic la = lower_end(a, &ai), lb = lower_end(b, &bi);
		if (ai || bi) return ai && !bi;
		const int c = orc::compare(la, lb);
		return c < 0 || (c == 0 && !a.ubit() && b.ubit());
	};
	for (unsigned i = 0; i < samples; ++i) {
		const X a = random_tile<X>(rng, false);
		X b = (i % 4 == 0) ? a : random_tile<X>(rng, false);
		if (i % 4 == 1) { b = a; ++b; }
		if (a.isnar() || b.isnar()) continue;
		const bool eq = a.bits() == b.bits(), lt = !eq && before(a, b);
		const bool ok = (a == b) == eq && (a != b) == !eq && (a < b) == lt && (a > b) == (!eq && !lt) && (a <= b) == (eq || lt) && (a >= b) == !lt;
		if (!ok) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << type_tag(X{}) << " order of " << to_interval(a) << ", " << to_interval(b) << '\n'; }
	}
	X nar(SpecificValue::nar), bottom(SpecificValue::infneg);
	if (!(nar < bottom)) { ++fails; if (report) std::cerr << "FAIL " << type_tag(X{}) << " NaR is below every tile\n"; }
	return fails;
}

template<unsigned nbits>
int VerifyTileInterval(unsigned samples, bool report) {
	using X = poxel<nbits, 2, std::uint32_t>;
	using I = tile_interval<X>;
	std::mt19937_64 rng(0x167Bu + nbits);
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL tile_interval<" << type_tag(X{}) << ">: " << what << '\n'; };
	auto encloses = [](const I& r, const orc::interval& v) {   // r holds every point of the exact set v
		const I b = orc::box<X>(v);
		return !r.isnan() && !b.isnan() && r.lo_key() <= b.lo_key() && b.hi_key() <= r.hi_key();
	};
	for (unsigned i = 0; i < samples; ++i) {
		const double x = std::ldexp(static_cast<double>(rng() >> 11), -50) + 0.5, y = std::ldexp(static_cast<double>(rng() >> 11), -51) + 0.25;
		const X a(x), b(y);                                     // exact tiles: the doubles are lattice points
		const I A(a), B(b);
		const orc::interval EA = orc::point(orc::from_double(x)), EB = orc::point(orc::from_double(y));
		// exact operands: one tile, the oracle's
		const I s = A + B, p = A * B;
		const I es = orc::box<X>(orc::add(EA, EB));
		if (!(s.lo_key() == s.hi_key()) || !(es.lo_key() == s.lo_key()) || !(es.hi_key() == s.hi_key())) fail("exact sum is not the oracle's tile");
		if (!encloses(p, orc::mul(EA, EB))) fail("product");
		// an interval spanning a run of tiles encloses every corner
		const I W(X(x), X(x + y)), Q = W / B, D = W - B;
		if (!encloses(Q, orc::div(EA, EB, 512)) || !encloses(Q, orc::div(orc::point(orc::from_double(x + y)), EB, 512))) fail("quotient of " + W.str() + " by " + B.str());
		if (!encloses(D, orc::sub(EA, EB))) fail("difference");
		if (!encloses(sqrt(W), orc::sqrt(EA, 512))) fail("sqrt of " + W.str());
	}
	return fails;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite  = "poxel beyond 64 bits";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<129>(100, true), "poxel<129,2>", "arithmetic");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<65>(1000, reportTestCases),        "poxel<65,2>",  "lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<129>(1000, reportTestCases),       "poxel<129,2>", "lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<257>(250, reportTestCases),        "poxel<257,2>", "lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<65>(1000, reportTestCases),     "poxel<65,2>",  "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<129>(1000, reportTestCases),    "poxel<129,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<257>(250, reportTestCases),     "poxel<257,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyPositInterop<65>(1000, reportTestCases),   "poxel<65,2>",  "posit interop");
	nrOfFailedTestCases += ReportTestResult(VerifyPositInterop<129>(1000, reportTestCases),  "poxel<129,2>", "posit interop");
	nrOfFailedTestCases += ReportTestResult(VerifyPositInterop<257>(250, reportTestCases),   "poxel<257,2>", "posit interop");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<65>(150, reportTestCases),      "poxel<65,2>",  "arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<129>(150, reportTestCases),     "poxel<129,2>", "arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<257>(50, reportTestCases),       "poxel<257,2>", "arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<65>(1000, reportTestCases),          "poxel<65,2>",  "order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<129>(1000, reportTestCases),         "poxel<129,2>", "order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<257>(250, reportTestCases),          "poxel<257,2>", "order");
	nrOfFailedTestCases += ReportTestResult(VerifyTileInterval<129>(50, reportTestCases),    "poxel<129,2>", "tile_interval");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<65>(2000, reportTestCases),        "poxel<65,2>",  "lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<129>(2000, reportTestCases),       "poxel<129,2>", "lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<257>(500, reportTestCases),        "poxel<257,2>", "lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<65>(2000, reportTestCases),     "poxel<65,2>",  "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<129>(2000, reportTestCases),    "poxel<129,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<257>(500, reportTestCases),     "poxel<257,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyPositInterop<65>(2000, reportTestCases),   "poxel<65,2>",  "posit interop");
	nrOfFailedTestCases += ReportTestResult(VerifyPositInterop<129>(2000, reportTestCases),  "poxel<129,2>", "posit interop");
	nrOfFailedTestCases += ReportTestResult(VerifyPositInterop<257>(500, reportTestCases),   "poxel<257,2>", "posit interop");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<65>(300, reportTestCases),      "poxel<65,2>",  "arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<129>(300, reportTestCases),     "poxel<129,2>", "arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<257>(100, reportTestCases),     "poxel<257,2>", "arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<65>(2000, reportTestCases),          "poxel<65,2>",  "order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<129>(2000, reportTestCases),         "poxel<129,2>", "order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<257>(500, reportTestCases),          "poxel<257,2>", "order");
	nrOfFailedTestCases += ReportTestResult(VerifyTileInterval<129>(100, reportTestCases),   "poxel<129,2>", "tile_interval");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<129>(3000, reportTestCases),    "poxel<129,2>", "arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyTileInterval<257>(200, reportTestCases),   "poxel<257,2>", "tile_interval");
#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
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
