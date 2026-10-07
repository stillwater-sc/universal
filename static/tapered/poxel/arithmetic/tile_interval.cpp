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
	return fails;
}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "tile interval";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<poxel<8, 0, std::uint8_t>>("poxel<8,0>", reportTestCases), "tile_interval<poxel<8,0>>", "exhaustive enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "exhaustive enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), "tile_interval<areal<8,2>>", "exhaustive enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyStrictness<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "open ends");
	nrOfFailedTestCases += ReportTestResult(VerifyStrictness<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), "tile_interval<areal<8,2>>", "open ends");
	nrOfFailedTestCases += ReportTestResult(VerifyHulls<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), "tile_interval<poxel<9,2>>", "multi-tile enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyHulls<areal<9, 3, std::uint16_t>>("areal<9,3>", reportTestCases), "tile_interval<areal<9,3>>", "multi-tile enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyCos<poxel<17, 2, std::uint32_t>>("poxel17", 1.0e-2, reportTestCases), "tile_interval<poxel17>", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyCos<poxel<33, 2, std::uint64_t>>("poxel33", 1.0e-6, reportTestCases), "tile_interval<poxel33>", "cos");
	// 58 fraction bits near 1: fine enough that a too-short Taylor bound (~4e-15 at |t| = 2) shows
	nrOfFailedTestCases += ReportTestResult(VerifyCos<poxel<64, 2, std::uint64_t>>("poxel<64,2>", 1.0e-15, reportTestCases, 4.5e-16), "tile_interval<poxel<64,2>>", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyCos<areal<32, 8, std::uint32_t>>("areal<32,8>", 1.0e-5, reportTestCases), "tile_interval<areal<32,8>>", "cos");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
