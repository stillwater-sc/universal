// posit_interop.cpp: conversion between poxel<nbits, es> and its lattice, posit<nbits - 1, es>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The reference is the posit's own value and the poxel's conversion through double, which
// share no code with the bit-level interop:
//   - every posit converts to the exact tile holding its value (NaR to NaR)
//   - every tile converts to the posit of its lower endpoint: posit(x) == posit(double(x)),
//     a valid reference here because double holds these lattices exactly (fraction <= 52 bits),
//     so (-inf, -maxpos) and NaR go to NaR
//   - a point tile round-trips; an open tile comes back as the point tile below it
// Exhaustive for small configurations, sampled over poxel33 / posit<32,2>.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <random>
#include <string>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<typename X, typename P>
int VerifyTile(const X& x, int& fails, bool report) {
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(X{}) << " tile " << x.raw() << ": " << what << '\n'; };
	const P p = P(x);
	const P via_double{ double(x) };
	if (!(p == via_double) && !(p.isnar() && via_double.isnar())) fail("posit(x) != posit(double(x))");
	if (x.isnar() || x.lattice() == X::lattice_nar) {
		if (!p.isnar()) fail("NaR and (-inf, -maxpos) must convert to NaR");
		return fails;
	}
	const X back(p);
	X point(x); point.setubit(false);
	if (!(back == point)) fail("posit round trip does not give the point tile at the lower endpoint");
	return fails;
}

template<typename X, typename P>
int VerifyPosit(const P& p, int& fails, bool report) {
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(X{}) << " from posit " << p.encoding() << ": " << what << '\n'; };
	const X x(p);
	if (p.isnar()) { if (!x.isnar()) fail("NaR must convert to NaR"); return fails; }
	if (!x.isexact()) fail("a posit must convert to an exact tile");
	if (!(x.template lower<double>() == double(p))) fail("the tile does not hold the posit's value");
	if (!(x == X(double(p)))) fail("poxel(p) != poxel(double(p))");
	return fails;
}

template<unsigned nbits, unsigned es>
int VerifyExhaustive(bool report) {
	using X = sw::universal::poxel<nbits, es, std::uint16_t>;
	using P = sw::universal::posit<nbits - 1, es, std::uint16_t>;
	int fails = 0;
	for (std::uint64_t t = 0; t < (1ull << nbits); ++t) {
		X x; x.setbits(t);
		VerifyTile<X, P>(x, fails, report);
	}
	for (std::uint64_t b = 0; b < (1ull << (nbits - 1)); ++b) {
		P p; p.setbits(b);
		VerifyPosit<X, P>(p, fails, report);
	}
	return fails;
}

template<unsigned nbits, unsigned es, typename bt>
int VerifySampled(unsigned samples, bool report) {
	using X = sw::universal::poxel<nbits, es, bt>;
	using P = sw::universal::posit<nbits - 1, es, bt>;
	std::mt19937_64 rng(0x1631u);
	int fails = 0;
	for (unsigned i = 0; i < samples; ++i) {
		const std::uint64_t r = rng();
		X x; x.setbits(r);
		VerifyTile<X, P>(x, fails, report);
		P p; p.setbits(r & ((1ull << (nbits - 1)) - 1ull));
		VerifyPosit<X, P>(p, fails, report);
	}
	// the special tiles
	auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(X{}) << ' ' << what << '\n'; } };
	using sw::universal::SpecificValue;
	expect(P(X(SpecificValue::maxpos)) == P(SpecificValue::maxpos), "maxpos to posit");
	expect(P(X(SpecificValue::infpos)) == P(SpecificValue::maxpos), "(maxpos, inf) to posit maxpos");
	expect(P(X(SpecificValue::infneg)).isnar(), "(-inf, -maxpos) to posit NaR");
	expect(X(P(SpecificValue::minpos)) == X(SpecificValue::minpos), "posit minpos to poxel minpos");
	expect(P(X(0.0).setubit()).iszero(), "(0, minpos) to posit 0");
	expect(X(P(0)).iszero() && X(P(SpecificValue::nar)).isnar(), "posit 0 and NaR");
	return fails;
}

int VerifyWide(bool report) {
	using X = sw::universal::poxel<80, 2, std::uint32_t>;
	using P = sw::universal::posit<79, 2, std::uint8_t>;
	using sw::universal::SpecificValue;
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) {
			++fails;
			if (report) std::cerr << "FAIL poxel<80,2> " << what << '\n';
		}
	};
	for (const P& p : { P(0), P(1), P(-1), P(3), P(SpecificValue::minpos),
	                    P(SpecificValue::maxpos), P(SpecificValue::nar) }) {
		const X exact(p);
		check(exact.isnar() == p.isnar(), "NaR conversion");
		if (p.isnar()) continue;
		check(exact.isexact(), "posit becomes exact tile");
		check(static_cast<P>(exact) == p, "exact round trip with different block types");
		X open(exact);
		open.setubit();
		check(static_cast<P>(open) == p, "open tile converts to its lower posit");
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
	using namespace sw::universal;
	std::string test_suite  = "poxel <-> posit conversion";
	std::string test_tag    = "posit interop";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 0>(true), "poxel<8,0>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 0>(reportTestCases),  "poxel<8,0>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<9, 2>(reportTestCases),  "poxel<9,2>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<10, 1>(reportTestCases), "poxel<10,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<33, 2, std::uint64_t>(20000, reportTestCases), "poxel33", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<12, 3>(reportTestCases), "poxel<12,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<17, 2>(reportTestCases), "poxel<17,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide(reportTestCases), "poxel<80,2>", test_tag);
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySampled<33, 2, std::uint64_t>(1000000, reportTestCases), "poxel33", test_tag);
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
