// scalar_tiles.cpp: a scalar unum op gives the tile that holds the exact result (#1701)
//
// A scalar unum names one tile: a lattice point, or the open tile beside it away from zero.
// Assignment and + - * / sqrt must give the tile that holds the exact result, rounding the
// right way when a double result lands on a lattice point it is not equal to.  With an
// inexact operand the result is the single tile that holds every x op y, or NaN when no
// single tile does.
//
// Exact operands are checked on unum<3,5> against an exact integer<1024> oracle that never
// uses the error-free transforms.  unum<2,x> cannot test the rounding side: its doubles
// never land on a lattice point without being exact.  Inexact operands are checked over
// every pair of tiles of unum<2,2>, and over random tiles of unum<3,4>, against ubound,
// whose interval arithmetic is verified exhaustively by ubound_enclosure.cpp; the scalar
// unum shares the lattice and the error-free transforms with it, not the corner logic.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <vector>
#include <universal/number/unum/unum.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

enum class Op { add, sub, mul, div };
static const char* op_name[] = { " + ", " - ", " * ", " / " };

namespace oracle {
// the exact integer oracle of ubound_enclosure.cpp: sign(L - T) for a lattice point L and
// the exact result T, in integer<1024> arithmetic
using Z = integer<1024, std::uint32_t>;
struct Scaled { long long m; int e; };   // v = m * 2^e, exactly
inline Scaled decompose(double v) {
	if (v == 0.0) return { 0, 0 };
	int e = 0;
	const double f = std::frexp(v, &e);
	return { static_cast<long long>(std::ldexp(f, 53)), e - 53 };
}
inline Z at(long long m, int e, int base) { Z z(m); z <<= (e - base); return z; }
inline int sign(const Z& z) { return z.iszero() ? 0 : (z.sign() ? -1 : 1); }

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
		const int base = std::min(l.e + y.e, x.e);
		Z p = Z(l.m) * Z(y.m);
		p <<= (l.e + y.e - base);
		return sign(p - at(x.m, x.e, base)) * (b > 0 ? 1 : -1);
	}
	}
	return 0;
}
inline int rel_sqrt(double L, double x) {
	const Scaled l = decompose(L), v = decompose(x);
	const int base = std::min(2 * l.e, v.e);
	Z p = Z(l.m) * Z(l.m);
	p <<= (2 * l.e - base);
	return sign(p - at(v.m, v.e, base));
}

// the tile that holds T, from sign(L - T) at lattice points L: as a ubound, the form the
// scalar result is compared in.  Below -maxpos there is no scalar tile (NaN).
template<unsigned ess, unsigned fss, typename RelL>
ubound<ess, fss> tile_of(RelL relL, double guess) {
	using L = unum_lattice<ess, fss>;
	using U = unum<ess, fss>;
	const double m = L::maxpos();
	ubound<ess, fss> nan_result;
	nan_result = ubound<ess, fss>(U(0.0)) / ubound<ess, fss>(U(0.0));   // x / [0] is NaN
	if (relL(m) < 0) return ubound<ess, fss>(U(2.0 * m));            // (maxpos, inf)
	if (relL(-m) > 0) return nan_result;
	double l = L::floor(std::clamp(guess, -m, m));
	while (relL(l) > 0) l = L::prev(l);
	while (l < m && relL(L::next(l)) <= 0) l = L::next(l);
	if (relL(l) == 0) return ubound<ess, fss>(U(l));                 // the exact point
	// the open tile (l, next(l)): built from a point inside it, which a double is exact for
	const double n = L::next(l);
	return ubound<ess, fss>(U(l + (n - l) / 2.0));
}
}  // namespace oracle

// the scalar result names the same tile as the expected ubound, or both are NaN.  It must
// also be in canonical form: an open tile at the finest fsize, where the ubit spans one
// lattice spacing in Gustafson's reading as well as ubound's, and an exact point in its
// shortest form (no trailing zero fraction bits).
template<unsigned ess, unsigned fss>
bool same_tile(const unum<ess, fss>& r, const ubound<ess, fss>& expected) {
	if (expected.isnan() || r.isnan()) return expected.isnan() && r.isnan();
	if (r.ubit() && r.fsize() != static_cast<unsigned>(unum_lattice<ess, fss>::maxfs)) return false;
	if (!r.ubit() && r.fsize() > 0 && (r.fraction() & 1u) == 0) return false;
	const ubound<ess, fss> got(r);
	return got.template lower<double>() == expected.template lower<double>() && got.lower_open() == expected.lower_open()
	    && got.template upper<double>() == expected.template upper<double>() && got.upper_open() == expected.upper_open()
	    && got.lower_unbounded() == expected.lower_unbounded() && got.upper_unbounded() == expected.upper_unbounded();
}

template<unsigned ess, unsigned fss>
std::string show(const unum<ess, fss>& u) {
	if (u.isnan()) return "NaN";
	std::stringstream s;
	s << std::setprecision(17) << ubound<ess, fss>(u);
	return s.str();
}

// exact operands: point ops and conversions against the integer oracle
template<unsigned ess, unsigned fss>
int VerifyPointOps(unsigned samples, bool reportTestCases) {
	using U = unum<ess, fss>;
	using Lat = unum_lattice<ess, fss>;
	const double m = Lat::maxpos();
	std::mt19937_64 rng(1701);
	std::uniform_int_distribution<long long> sig(1ll << Lat::maxfs, (2ll << Lat::maxfs) - 1);
	std::uniform_int_distribution<int> binade(Lat::emin, Lat::emax), near(-8, 8), coin(0, 3);
	std::uniform_int_distribution<long long> wide(1ll << 52, (1ll << 53) - 1);
	auto point = [&]() {
		const double v = std::ldexp(static_cast<double>(sig(rng)), binade(rng) - Lat::maxfs);
		return (coin(rng) & 1) ? -v : v;
	};
	int nrOfFailedTestCases = 0;
	unsigned directed = 0;   // a double result on a lattice point that is not the exact value
	auto fail = [&](const std::string& what, const U& r, const ubound<ess, fss>& expected) {
		++nrOfFailedTestCases;
		if (reportTestCases && nrOfFailedTestCases < 10) std::cerr << std::setprecision(17) << "FAIL: " << what << " = " << show(r) << ", expected " << expected << '\n';
	};
	// the last term of TwoProduct, and a sum far below the last bit
	std::vector<std::pair<double, double>> fixed;
	for (int k : { Lat::maxfs, Lat::maxfs - 1, Lat::maxfs / 2 + 1 }) {
		const double e = std::ldexp(1.0, -k);
		fixed.push_back({ 1.0 + e, 1.0 - e });
		fixed.push_back({ -(1.0 + e), 1.0 - e });
		fixed.push_back({ 1.0, std::ldexp(1.0, -2 * k - 60) });
	}
	for (std::size_t s = 0; s < fixed.size() + samples; ++s) {
		double a, b;
		if (s < fixed.size()) { a = fixed[s].first; b = fixed[s].second; }
		else {
			a = point();
			b = point();
			if (coin(rng) == 0) {
				const int k = near(rng);
				b = std::ldexp(a, k) * ((coin(rng) & 1) ? 1.0 : -1.0);
				if (std::abs(b) > m || Lat::floor(b) != b) b = point();
			}
		}
		const U ua(a), ub(b);
		if (ua.ubit() || ub.ubit()) { fail("operand not a lattice point", ua.ubit() ? ua : ub, ubound<ess, fss>(U(a))); continue; }
		for (Op o : { Op::add, Op::sub, Op::mul, Op::div }) {
			U r;
			double guess = 0.0;
			switch (o) {
			case Op::add: r = ua + ub; guess = a + b; break;
			case Op::sub: r = ua - ub; guess = a - b; break;
			case Op::mul: r = ua * ub; guess = a * b; break;
			case Op::div: r = ua / ub; guess = a / b; break;
			}
			const auto expected = oracle::tile_of<ess, fss>([&](double L) { return oracle::rel(L, a, b, o); }, guess);
			if (std::abs(guess) <= m && Lat::floor(guess) == guess && oracle::rel(guess, a, b, o) != 0) ++directed;
			if (!same_tile(r, expected)) {
				std::stringstream w;
				w << std::setprecision(17) << '[' << a << ']' << op_name[static_cast<int>(o)] << '[' << b << ']';
				fail(w.str(), r, expected);
			}
		}
		const double x = std::abs(a);
		const U r = sqrt(U(x));
		const auto expected = oracle::tile_of<ess, fss>([&](double L) { return (L < 0.0) ? -1 : oracle::rel_sqrt(L, x); }, std::sqrt(x));
		if (!same_tile(r, expected)) fail("sqrt[" + std::to_string(x) + "]", r, expected);
		// conversion of a double that is not a lattice point: 53 significant bits anywhere in range
		const double v = std::ldexp(static_cast<double>(wide(rng)), binade(rng) - 52) * ((coin(rng) & 1) ? 1.0 : -1.0);
		const auto ev = oracle::tile_of<ess, fss>([&](double L) { return oracle::rel(L, v, 0.0, Op::add); }, v);
		const U uv(v);
		if (!same_tile(uv, ev)) fail("U(" + std::to_string(v) + ")", uv, ev);
	}
	// the sweep only tests the rounding side if it meets such cases
	if (directed == 0) {
		++nrOfFailedTestCases;
		if (reportTestCases) std::cerr << "FAIL: no directed case in the sweep\n";
	}
	return nrOfFailedTestCases;
}

// every tile of the configuration as a scalar unum: each lattice point, the open tile above
// it, and (maxpos, inf)
template<unsigned ess, unsigned fss>
std::vector<unum<ess, fss>> AllTiles() {
	using L = unum_lattice<ess, fss>;
	using U = unum<ess, fss>;
	std::vector<U> tiles;
	for (double v = -L::maxpos(); v < L::maxpos(); v = L::next(v)) {
		tiles.emplace_back(v);
		tiles.emplace_back(v + (L::next(v) - v) / 2.0);
	}
	tiles.emplace_back(L::maxpos());
	tiles.emplace_back(2.0 * L::maxpos());
	return tiles;
}

// x op y against ubound: its single tile, or NaN when it spans more than one (or reaches
// below -maxpos, which has no scalar tile)
template<unsigned ess, unsigned fss>
ubound<ess, fss> single_tile(const ubound<ess, fss>& b) {
	using L = unum_lattice<ess, fss>;
	using U = unum<ess, fss>;
	ubound<ess, fss> nan_result = ubound<ess, fss>(U(1.0)) / ubound<ess, fss>(U(0.0));
	if (b.isnan() || b.lower_unbounded()) return nan_result;
	const double lo = b.template lower<double>(), hi = b.template upper<double>();
	if (b.upper_unbounded()) return (b.lower_open() && lo == L::maxpos()) ? b : nan_result;
	if (lo == hi && !b.lower_open() && !b.upper_open()) return b;
	if (b.lower_open() && b.upper_open() && L::next(lo) == hi) return b;
	return nan_result;
}

template<unsigned ess, unsigned fss>
int CheckTilePair(const unum<ess, fss>& x, const unum<ess, fss>& y, bool reportTestCases, int& nrOfFailedTestCases) {
	using B = ubound<ess, fss>;
	using U = unum<ess, fss>;
	const B bx(x), by(y);
	for (Op o : { Op::add, Op::sub, Op::mul, Op::div }) {
		if (o == Op::div && !y.ubit() && y.to_double() == 0.0) continue;   // x / 0: VerifyIssueCases
		U r;
		B e;
		switch (o) {
		case Op::add: r = x + y; e = bx + by; break;
		case Op::sub: r = x - y; e = bx - by; break;
		case Op::mul: r = x * y; e = bx * by; break;
		case Op::div: r = x / y; e = bx / by; break;
		}
		const B expected = single_tile(e);
		if (!same_tile(r, expected)) {
			++nrOfFailedTestCases;
			if (reportTestCases && nrOfFailedTestCases < 10)
				std::cerr << std::setprecision(17) << "FAIL: " << show(x) << op_name[static_cast<int>(o)] << show(y) << " = " << show(r) << ", ubound " << e << '\n';
		}
	}
	return nrOfFailedTestCases;
}

// every pair of tiles, every operation, and sqrt of every tile
template<unsigned ess, unsigned fss>
int VerifyInexactExhaustive(bool reportTestCases) {
	using B = ubound<ess, fss>;
	const auto tiles = AllTiles<ess, fss>();
	int nrOfFailedTestCases = 0;
	for (const auto& x : tiles) {
		if (!same_tile(sqrt(x), single_tile(sqrt(B(x))))) {
			++nrOfFailedTestCases;
			if (reportTestCases && nrOfFailedTestCases < 10) std::cerr << "FAIL: sqrt" << show(x) << " = " << show(sqrt(x)) << ", ubound " << sqrt(B(x)) << '\n';
		}
		for (const auto& y : tiles) CheckTilePair(x, y, reportTestCases, nrOfFailedTestCases);
	}
	return nrOfFailedTestCases;
}

// random tiles of a lattice too large to enumerate: a random point, the open tile above or
// below it, the tiles beside zero, and (maxpos, inf); the second operand is often near the
// first, so sums cancel and products stay in range
template<unsigned ess, unsigned fss>
int VerifyInexactRandom(unsigned samples, bool reportTestCases) {
	using U = unum<ess, fss>;
	using L = unum_lattice<ess, fss>;
	std::mt19937_64 rng(1701);
	std::uniform_int_distribution<long long> sig(1ll << L::maxfs, (2ll << L::maxfs) - 1);
	std::uniform_int_distribution<int> binade(L::emin, L::emax), near(-4, 4), kind(0, 15);
	auto tile_at = [&](double v) {
		switch (kind(rng)) {
		case 0: return U(2.0 * L::maxpos());                                     // (maxpos, inf)
		case 1: return U(std::ldexp((kind(rng) & 1) ? 1.0 : -1.0, L::emin - L::maxfs - 4));   // beside zero
		case 2: case 3: case 4: case 5: case 6: return U(v);                       // the point
		default: {
			const double n = (kind(rng) & 1) ? L::next(v) : L::prev(v);
			return U(v + (n - v) / 2.0);                                         // an open tile beside it
		}
		}
	};
	auto point = [&]() {
		const double v = std::ldexp(static_cast<double>(sig(rng)), binade(rng) - L::maxfs);
		return (kind(rng) & 1) ? -v : v;
	};
	int nrOfFailedTestCases = 0;
	for (unsigned s = 0; s < samples; ++s) {
		const double a = point();
		double b = point();
		if (kind(rng) < 6) {
			b = std::ldexp(a, near(rng)) * ((kind(rng) & 1) ? 1.0 : -1.0);
			if (std::abs(b) >= L::maxpos() || L::floor(b) != b) b = point();
		}
		if (std::abs(a) >= L::maxpos() || std::abs(b) >= L::maxpos()) continue;   // next() above maxpos is off the lattice
		CheckTilePair(tile_at(a), tile_at(b), reportTestCases, nrOfFailedTestCases);
	}
	return nrOfFailedTestCases;
}

// the cases of the issue, and the conversions from wider native types
int VerifyIssueCases(bool reportTestCases) {
	using U = unum<3, 4>;
	using L = unum_lattice<3, 4>;
	int nrOfFailedTestCases = 0;
	auto expect = [&](bool ok, const std::string& what) {
		if (!ok) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: " << what << '\n'; }
	};
	auto is_tile = [](const U& u, double v, bool ubit) { return !u.isnan() && u.to_double() == v && u.ubit() == ubit; };
	const U one(1.0), tiny(std::ldexp(1.0, -100));
	expect(is_tile(one + tiny, 1.0, true), "1 + 2^-100 is the open tile above 1, got " + show(one + tiny));
	expect(is_tile(one - tiny, L::prev(1.0), true), "1 - 2^-100 is the open tile below 1, got " + show(one - tiny));
	expect(is_tile(U(1e40), L::maxpos(), true), "U(1e40) is (maxpos, inf), got " + show(U(1e40)));
	expect(U(-1e40).isnan(), "U(-1e40) is NaN: (-inf, -maxpos) shares the NaN encoding");
	expect(is_tile(U(L::maxpos()) + U(L::maxpos()), L::maxpos(), true), "maxpos + maxpos is (maxpos, inf)");
	expect(is_tile(U(2.0) + U(2.0), 4.0, false), "2 + 2 is exactly 4");
	expect(is_tile(U(1.0) / U(4.0), 0.25, false), "1 / 4 is exactly 0.25");
	expect(is_tile(sqrt(U(4.0)), 2.0, false), "sqrt(4) is exactly 2");
	expect((U(1.0) / U(0.0)).isnan(), "1 / 0 is NaN");
	// inexact operands: one tile when the result fits, NaN when it spans several
	const U third = U(1.0) / U(3.0);
	expect(third.ubit() && !(third + third).isnan(), "(1/3) + (1/3) still fits one tile");
	expect((third * U(3.0)).isnan(), "(1/3) * 3 spans three tiles, so it is NaN");
	expect((third - third).isnan(), "x - x for an inexact x spans zero, so it is NaN");
	expect(is_tile(U(0.0) * U(1e40), 0.0, false), "0 * (maxpos, inf) is exactly 0");
	expect((U(1.0) / U(1e40)).isnan(), "1 / (maxpos, inf) = (0, 1/maxpos) spans many tiles, so it is NaN");
	// a value far below minpos is the tile beside zero, on its own side
	const U up(std::ldexp(1.0, -200)), dn(-std::ldexp(1.0, -200));
	expect(up.ubit() && !up.sign() && up.to_double() == 0.0, "U(2^-200) is (0, minpos)");
	expect(dn.ubit() && dn.sign() && dn.to_double() == 0.0, "U(-2^-200) is (-minpos, 0)");
	// 64-bit integers that are not doubles: the rounding of the int-to-double cast is undone
	const long long big = (1ll << 60) + 1;
	expect(is_tile(U(big), std::ldexp(1.0, 60), true), "U(2^60 + 1) is the open tile above 2^60");
	expect(is_tile(U(-big), -std::ldexp(1.0, 60), true), "U(-(2^60 + 1)) is the open tile below -2^60");
	expect(is_tile(U((1ll << 60) - 1), L::prev(std::ldexp(1.0, 60)), true), "U(2^60 - 1) is the open tile below 2^60");
	expect(is_tile(U(~0ull), std::ldexp(1.0, 64) - std::ldexp(1.0, 64 - 16), true), "U(2^64 - 1) is the open tile below 2^64");
	expect(is_tile(U(std::numeric_limits<long long>::min()), -std::ldexp(1.0, 63), false), "U(-2^63) is exact");
#if LONG_DOUBLE_SUPPORT
	if constexpr (std::numeric_limits<long double>::digits > 53) {
		const long double ld = 1.0L + std::ldexp(1.0L, -60);
		expect(is_tile(U(ld), 1.0, true), "U(1 + 2^-60 as long double) is the open tile above 1");
		const long double ld2 = 1.0L - std::ldexp(1.0L, -60);
		expect(is_tile(U(ld2), L::prev(1.0), true), "U(1 - 2^-60 as long double) is the open tile below 1");
	}
#endif
	return nrOfFailedTestCases;
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

	std::string test_suite  = "unum Type I scalar tiles (#1701)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIssueCases(reportTestCases), "unum<3,4>", "issue cases");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIssueCases(reportTestCases), "unum<3,4>", "issue cases");
	nrOfFailedTestCases += ReportTestResult(VerifyPointOps<3, 5>(2000, reportTestCases), "unum<3,5>", "point ops, exact oracle");
	nrOfFailedTestCases += ReportTestResult(VerifyInexactExhaustive<2, 2>(reportTestCases), "unum<2,2>", "every tile pair vs ubound");
	nrOfFailedTestCases += ReportTestResult(VerifyInexactRandom<3, 4>(20000, reportTestCases), "unum<3,4>", "random tile pairs vs ubound");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyPointOps<3, 5>(20000, reportTestCases), "unum<3,5>", "point ops, exact oracle");
	nrOfFailedTestCases += ReportTestResult(VerifyInexactRandom<3, 5>(200000, reportTestCases), "unum<3,5>", "random tile pairs vs ubound");
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
