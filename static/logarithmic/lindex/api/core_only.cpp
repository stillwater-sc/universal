// core_only.cpp: lindex/core.hpp alone compiles, links and runs, without any stream (#1694, #1334)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The core layer must not pull <iostream>, <sstream>, <iomanip>, <ostream> or <istream>;
// tools/scripts/measure-include-cost.sh measures that.  This test is the other half: a
// translation unit with only the core in it converts, compares and negates, and reports
// through <cstdio>.  So it does not include test_suite.hpp, which is stream-based.
#include <universal/utility/directives.hpp>
#include <cstdio>
#include <cstdlib>
#include <universal/number/lindex/core.hpp>

namespace {

int check(bool ok, const char* what) {
	if (!ok) std::fprintf(stderr, "FAIL: %s\n", what);
	return ok ? 0 : 1;
}

template<unsigned nbits, unsigned lbits>
int VerifyCore() {
	using L = sw::universal::lindex<nbits, lbits>;
	int fails = 0;
	L one(1.0), two(2), e(2.718281828459045), big(1.0e300);
	fails += check(one.level() == 1 && one.index_bits() == 0, "1.0 is psi 1");
	fails += check(one < two && two < e && e < big, "order");
	fails += check(-big < -e && -e < L(0) && L(0) < one, "order of negatives");
	fails += check(double(one) == 1.0, "double(1.0)");
	fails += check(int(two) == 1 || int(two) == 2, "int(2)");   // 2 is not on the grid
	fails += check(big.level() == 4, "1e300 is at level 4");
	fails += check(L(sw::universal::SpecificValue::nar) < -big, "NaR is below every value");
	fails += check(std::numeric_limits<L>::max() == L(sw::universal::SpecificValue::maxpos), "numeric_limits max");
	return fails;
}

}  // anonymous namespace

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
	int nrOfFailedTestCases = 0;
	std::printf("lindex core.hpp without streams (#1694)\n");

#if MANUAL_TESTING

	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += VerifyCore<16, 3>();
	nrOfFailedTestCases += VerifyCore<32, 3>();
	nrOfFailedTestCases += VerifyCore<64, 3>();
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	std::printf("lindex core.hpp without streams: %s\n", nrOfFailedTestCases > 0 ? "FAIL" : "PASS");
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (...) {
	std::fprintf(stderr, "Caught unknown exception\n");
	return EXIT_FAILURE;
}
