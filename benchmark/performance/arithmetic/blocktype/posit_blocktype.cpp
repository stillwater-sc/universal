// posit_blocktype.cpp: limb-processing cost of posit<nbits, 2, bt> for BlockType uint8/16/32/64 (#1904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// posit<nbits, 2, bt> (the standard es = 2) at nbits 64, 96, 128, 196 and 256. Between the four rows at
// one nbits only the limb layout changes. 196 is not a multiple of 16, 32 or 64, so every BlockType has
// a partially filled most significant limb there.
//
// The methodology is described in blocktype_harness.hpp. Timings vary by core on a hybrid processor:
// pin the process (taskset -c 2 on Linux) before comparing runs.
#include <universal/utility/directives.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>
#include "blocktype_harness.hpp"

namespace sw { namespace universal {

	template<unsigned nbits, unsigned es>
	struct PositOf {
		template<typename bt> struct make { using type = posit<nbits, es, bt>; };
	};

}}  // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::blocktype_benchmark;

	std::string test_suite = "posit BlockType limb-processing benchmark";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	const std::uint64_t seed = 1904;
	Table table;

#if MANUAL_TESTING
	// one width, short samples: for a quick look while working on the limb loops
	Options opt{ 256, 3, 0.002 };
	printHeader("posit<nbits, 2, bt>", opt);
	measureAll<128, PositOf<128, 2>::template make>(table, opt, seed);
	printTable("posit", table);
	nrOfFailedTestCases += crossCheck("posit", table);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_4
	// the long mode: more and longer samples for figures that are stable to a few percent
	Options opt{ 512, 21, 0.025 };
#else
	// the default: a casual local run
	Options opt{ 512, 9, 0.010 };
#endif
	printHeader("posit<nbits, 2, bt>", opt);

#if REGRESSION_LEVEL_1
	measureAll< 64, PositOf< 64, 2>::template make>(table, opt, seed);
	measureAll< 96, PositOf< 96, 2>::template make>(table, opt, seed);
	measureAll<128, PositOf<128, 2>::template make>(table, opt, seed);
	measureAll<196, PositOf<196, 2>::template make>(table, opt, seed);
	measureAll<256, PositOf<256, 2>::template make>(table, opt, seed);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	printTable("posit", table);
	nrOfFailedTestCases += crossCheck("posit", table);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
