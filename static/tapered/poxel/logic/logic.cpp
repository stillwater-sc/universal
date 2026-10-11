// logic.cpp: comparison of poxels -- tile order around the ring
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Tiles are ordered by integer order of their patterns.  For every pair this must agree
// with ordering the tiles by position on the real line -- a point before the open tile
// above it, an open tile before the point that ends it -- with NaR below everything.
// Exhaustive for small configurations, sampled over poxel17 and poxel33.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <random>
#include <universal/number/poxel/poxel.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// every comparison of a and b against their positions on the real line
template<typename X>
bool OrderAgrees(const X& a, const X& b) {
	// position key: (lower endpoint, 0 for a point / 1 for the open interval above it)
	auto key = [](const X& x) { return std::make_pair(x.template lower<double>(), x.ubit() ? 1 : 0); };
	const bool an = a.isnar(), bn = b.isnar();
	const bool eq = (a.raw() == b.raw());
	const bool lt = (an && !bn) || (!an && !bn && key(a) < key(b));
	return (a == b) == eq && (a != b) == !eq && (a < b) == lt && (a > b) == (!eq && !lt) && (a <= b) == (eq || lt) && (a >= b) == !lt;
}

template<unsigned nbits, unsigned es>
int VerifyOrder(bool report) {
	using X = sw::universal::poxel<nbits, es, std::uint16_t>;
	int fails = 0;
	const unsigned N = 1u << nbits;
	for (unsigned i = 0; i < N; ++i) {
		X a; a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			X b; b.setbits(j);
			if (!OrderAgrees(a, b)) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(a) << " order of " << i << ", " << j << '\n'; }
		}
	}
	return fails;
}

// random pairs, plus each tile against its neighbours on the ring
template<unsigned nbits, unsigned es, typename bt>
int VerifyOrderSampled(unsigned samples, bool report) {
	using X = sw::universal::poxel<nbits, es, bt>;
	std::mt19937_64 rng(0x1631u + nbits);
	int fails = 0;
	auto check = [&](const X& a, const X& b) {
		if (!OrderAgrees(a, b)) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(a) << " order of " << a.raw() << ", " << b.raw() << '\n'; }
	};
	for (unsigned i = 0; i < samples; ++i) {
		X a, b;
		a.setbits(rng());
		b.setbits(rng());
		check(a, b);
		X next(a); ++next;
		if (!next.isnar()) { check(a, next); check(next, a); }       // no wrap past the top tile
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
	std::string test_suite  = "poxel logic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<8, 0>(true), "poxel<8,0>", "tile order");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<8, 0>(reportTestCases),  "poxel<8,0>",  "tile order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<9, 2>(reportTestCases),  "poxel<9,2>",  "tile order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<10, 1>(reportTestCases), "poxel<10,1>", "tile order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrderSampled<17, 2, std::uint32_t>(100000, reportTestCases), "poxel17", "tile order, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifyOrderSampled<33, 2, std::uint64_t>(100000, reportTestCases), "poxel33", "tile order, sampled");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyOrderSampled<33, 2, std::uint64_t>(2000000, reportTestCases), "poxel33", "tile order, sampled");
#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
