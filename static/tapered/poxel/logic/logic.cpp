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
#include <universal/utility/directives.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<unsigned nbits, unsigned es>
int VerifyOrder(bool report) {
	using X = sw::universal::poxel<nbits, es, std::uint16_t>;
	int fails = 0;
	// position key: (lower endpoint, 0 for a point / 1 for the open interval above it)
	auto key = [](const X& x) { return std::make_pair(x.template lower<double>(), x.ubit() ? 1 : 0); };
	const unsigned N = 1u << nbits;
	for (unsigned i = 0; i < N; ++i) {
		X a; a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			X b; b.setbits(j);
			const bool an = a.isnar(), bn = b.isnar();
			const bool eq = (i == j);
			const bool lt = (an && !bn) || (!an && !bn && key(a) < key(b));
			const bool ok = (a == b) == eq && (a != b) == !eq && (a < b) == lt && (a > b) == (!eq && !lt) && (a <= b) == (eq || lt) && (a >= b) == !lt;
			if (!ok) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(a) << " order of " << i << ", " << j << '\n'; }
		}
	}
	return fails;
}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "poxel logic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<8, 0>(reportTestCases),  "poxel<8,0>",  "tile order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<9, 2>(reportTestCases),  "poxel<9,2>",  "tile order");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder<10, 1>(reportTestCases), "poxel<10,1>", "tile order");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
