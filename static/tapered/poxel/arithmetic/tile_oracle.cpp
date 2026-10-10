// tile_oracle.cpp: the tightest-tile oracle against the tile types' own exact-operand arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The oracle (tile_oracle.hpp) computes in exact dyadics and outward-rounded dyadic intervals,
// and finds the tile of a value by bisection over the lattice.  It shares no code with the tile
// arithmetic, whose contract -- an operation on exact tiles returns the tile that contains the
// exact result -- the areal and poxel suites verify exhaustively.  So the two must agree on
// every pair of exact tiles: + - * / and sqrt.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <universal/number/areal/areal.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/utility/tile_oracle.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
namespace orc = sw::universal::oracle;

// the oracle's tile for an interval, refining the working precision until it is one tile
template<typename Tile, typename F>
tile_interval<Tile> resolve(F f) {
	tile_interval<Tile> b;
	for (int P = 32; P <= 1024; P *= 2) {
		b = orc::box<Tile>(f(P));
		if (b.isnan() || b.lo_key() == b.hi_key()) break;
	}
	return b;
}

// every lattice point decodes exactly (against double, which holds them) and locates to its key;
// every open tile's midpoint locates to the open key
template<typename Tile>
int VerifyLattice(const std::string& tag, bool reportTestCases) {
	using T = tile_traits<Tile>;
	int nrOfFailedTests = 0;
	for (std::int64_t k = -T::kmax + 1; k < T::kmax; ++k) {
		const tile_interval<Tile> x = tile_interval<Tile>::from_keys(k, k);
		const double lo = x.template lower<double>(), hi = x.template upper<double>();
		bool ok = true;
		if ((k & 1) == 0) {
			ok = orc::compare(orc::lattice_point<Tile>(k), orc::from_double(lo)) == 0 && orc::locate<Tile>(orc::from_double(lo)) == k;
		}
		else {
			ok = orc::locate<Tile>(orc::from_double((lo + hi) / 2)) == k;
		}
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 5) std::cerr << tag << " lattice key " << k << " FAIL\n";
		}
	}
	return nrOfFailedTests;
}

// + - * / on every pair of exact tiles, and sqrt on every exact tile
template<typename Tile>
int VerifyArithmetic(const std::string& tag, bool reportTestCases) {
	using T = tile_traits<Tile>;
	using I = tile_interval<Tile>;
	int nrOfFailedTests = 0;
	auto check = [&](const Tile& expected, const I& got, const std::string& what) {
		const std::int64_t k = std::clamp(T::key(expected), -T::kmax, T::kmax);
		if (got.isnan() || got.lo_key() != k || got.hi_key() != k) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 8) std::cerr << tag << " " << what << ": oracle " << got.str() << ", tile " << expected << '\n';
		}
	};
	for (std::int64_t i = -T::kmax + 1; i < T::kmax; i += 2) {
		const Tile a = T::tile(i);
		const orc::interval A = orc::point(orc::lattice_point<Tile>(i));
		if (i >= 0) check(tile_sqrt(a), resolve<Tile>([&](int P) { return orc::sqrt(A, P); }), "sqrt " + std::to_string(i));
		for (std::int64_t j = -T::kmax + 1; j < T::kmax; j += 2) {
			const Tile b = T::tile(j);
			const orc::interval B = orc::point(orc::lattice_point<Tile>(j));
			const std::string ij = std::to_string(i) + " " + std::to_string(j);
			check(a + b, orc::box<Tile>(orc::add(A, B)), "add " + ij);
			check(a - b, orc::box<Tile>(orc::sub(A, B)), "sub " + ij);
			check(a * b, orc::box<Tile>(orc::mul(A, B)), "mul " + ij);
			if (j != 0) check(a / b, resolve<Tile>([&](int P) { return orc::div(A, B, P); }), "div " + ij);
		}
	}
	return nrOfFailedTests;
}

// the parts that have no tile counterpart
int VerifyOracleAlgebra(bool reportTestCases) {
	using X = poxel<64, 2, std::uint64_t>;
	int nrOfFailedTests = 0;
	auto fail = [&](const char* what) { ++nrOfFailedTests; if (reportTestCases) std::cerr << "FAIL: " << what << '\n'; };
	bool ok = false;
	// exact detection: 0.5 is dyadic, sqrt(9/4) is 3/2
	if (!orc::is_point(orc::from_decimal("0.5", 64, ok)) || !ok) fail("0.5 should be exact");
	const orc::interval q = orc::div(orc::point(orc::make_dyadic(9)), orc::point(orc::make_dyadic(4)), 64);
	const orc::interval s = orc::sqrt(q, 64);
	if (!orc::is_point(s) || orc::compare(s.lo, orc::make_dyadic(3, -1)) != 0) fail("sqrt(9/4) should be exactly 3/2");
	// one tenth is not dyadic: an interval at every precision, one open tile of poxel<64,2> at 256 bits
	const orc::interval tenth = orc::from_decimal("0.1", 256, ok);
	const tile_interval<X> b = orc::box<X>(tenth);
	if (orc::is_point(tenth) || b.lo_key() != b.hi_key() || (b.lo_key() & 1) == 0) fail("0.1 should be one open tile");
	// sqrt(2) brackets: lo^2 < 2 < hi^2, within 2^-60 relative
	const orc::rounded r = orc::square_root(orc::make_dyadic(2), 64);
	if (r.exact || orc::compare(orc::mul(r.lo, r.lo), orc::make_dyadic(2)) >= 0 || orc::compare(orc::mul(r.hi, r.hi), orc::make_dyadic(2)) <= 0) fail("sqrt(2) bracket");
	// a non-dyadic route to a lattice point does not resolve: (1/3) * 3 straddles 1
	const orc::interval third = orc::div(orc::point(orc::make_dyadic(1)), orc::point(orc::make_dyadic(3)), 512);
	const tile_interval<X> one = orc::box<X>(orc::mul(third, orc::point(orc::make_dyadic(3))));
	const std::int64_t k1 = tile_traits<X>::key(X(1));
	if (one.lo_key() != k1 - 1 || one.hi_key() != k1 + 1) fail("(1/3)*3 should be the three tiles around 1");
	// division by a set holding zero: the entire line; by zero itself: nan
	if (!orc::div(q, orc::interval{ orc::make_dyadic(-1), orc::make_dyadic(1) }, 64).lo_inf) fail("division by a set holding zero");
	if (!orc::div(q, orc::point(orc::dyadic{}), 64).nan) fail("division by zero");
	return nrOfFailedTests;
}

}  // namespace

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

	std::string test_suite  = "tile oracle";
	std::string test_tag    = "tile oracle";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<poxel<6, 0, std::uint8_t>>("poxel<6,0>", reportTestCases), test_tag, "exact-operand arithmetic");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyOracleAlgebra(reportTestCases), test_tag, "exactness and resolution");
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<poxel<8, 2, std::uint8_t>>("poxel<8,2>", reportTestCases), test_tag, "poxel<8,2> lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<poxel<16, 2, std::uint16_t>>("poxel<16,2>", reportTestCases), test_tag, "poxel<16,2> lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), test_tag, "areal<8,2> lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyLattice<areal<16, 5, std::uint16_t>>("areal<16,5>", reportTestCases), test_tag, "areal<16,5> lattice");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<poxel<7, 0, std::uint8_t>>("poxel<7,0>", reportTestCases), test_tag, "poxel<7,0> arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal<7, 2, std::uint8_t>>("areal<7,2>", reportTestCases), test_tag, "areal<7,2> arithmetic");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<poxel<8, 2, std::uint8_t>>("poxel<8,2>", reportTestCases), test_tag, "poxel<8,2> arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal<8, 2, std::uint8_t>>("areal<8,2>", reportTestCases), test_tag, "areal<8,2> arithmetic");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<poxel<9, 2, std::uint16_t>>("poxel<9,2>", reportTestCases), test_tag, "poxel<9,2> arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal<9, 3, std::uint16_t>>("areal<9,3>", reportTestCases), test_tag, "areal<9,3> arithmetic");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<poxel<10, 2, std::uint16_t>>("poxel<10,2>", reportTestCases), test_tag, "poxel<10,2> arithmetic");
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
