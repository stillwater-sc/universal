// ereal_pown_sweep.cpp: cross-validate ereal<N>::pown against elreal (Phase J, #904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ereal<N> exposes only `pown` from the math suite today; the rest of
// the Phase J sweep does not apply. The oracle for pown is
// elreal::pow(x, elreal(static_cast<double>(n))) since elreal does not
// currently provide a free-function pown overload (see
// math_sweep_common.hpp::sweep_pown).

#include <universal/utility/directives.hpp>

#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include "math_sweep_common.hpp"

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::elreal_oracle_sweep;

	std::string test_suite = "elreal Phase J: ereal pown oracle sweep";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// Modest exponents only: pown of large bases at large exponents
	// overflows even double precision, and the oracle would have to
	// agree on inf which is also not interesting. The point of the
	// sweep is to catch algorithmic disagreement, not floating-point
	// limits.
	std::vector<std::pair<double, int>> pown_inputs = {
		{ 2.0,  0}, { 2.0,  1}, { 2.0,  2}, { 2.0,  3}, { 2.0,  5},
		{ 0.5,  0}, { 0.5,  1}, { 0.5,  2}, { 0.5,  3}, { 0.5,  5},
		{ 1.5,  0}, { 1.5,  1}, { 1.5,  3}, { 1.5,  5},
		{ 3.0, -1}, { 3.0, -2}, { 3.0, -3},
		{-2.0,  0}, {-2.0,  1}, {-2.0,  2}, {-2.0,  3},
	};

	// ereal<2>: dd-equivalent precision
	nrOfFailedTestCases += sweep_pown<ereal<2>>("ereal<2>::pown", pown_inputs,
		[](const ereal<2>& a, int n) { return pown(a, n); });

	// ereal<4>: qd-equivalent
	nrOfFailedTestCases += sweep_pown<ereal<4>>("ereal<4>::pown", pown_inputs,
		[](const ereal<4>& a, int n) { return pown(a, n); });

	// ereal<8>: default ereal max
	nrOfFailedTestCases += sweep_pown<ereal<8>>("ereal<8>::pown", pown_inputs,
		[](const ereal<8>& a, int n) { return pown(a, n); });

	// Anchors specific to pown:
	{
		ereal<2> target; target = 2.0;
		ereal<2> z = pown(target, 0);
		if (double(z) != 1.0) {
			std::cerr << "FAIL: ereal<2>::pown(2,0)=" << double(z) << " expected 1\n";
			++nrOfFailedTestCases;
		}
	}
	{
		ereal<2> one_e2; one_e2 = 1.0;
		ereal<2> z = pown(one_e2, 100);
		if (double(z) != 1.0) {
			std::cerr << "FAIL: ereal<2>::pown(1,100)=" << double(z) << " expected 1\n";
			++nrOfFailedTestCases;
		}
	}

	// Reject path: deliberately wrong elreal must not match.
	{
		ereal<2> target; target = 8.0;  // = pown(2, 3)
		elreal   wrong  = elreal(16.0); // 2x off
		if (check_against_elreal_oracle(target, wrong)) {
			std::cerr << "FAIL: ereal<2> oracle helper accepted a 2x-off reference\n";
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
