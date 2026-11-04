// cfloat_blocktype.cpp: limb-processing cost of cfloat<nbits, es, bt> for BlockType uint8/16/32/64 (#1904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// cfloat<nbits, es, bt, true, false, false> (IEEE-754 style: subnormals, no max-exponent values, not
// saturating) at nbits 64, 96, 128, 196 and 256. The exponent size is fixed per nbits, so between the
// four rows at one nbits only the limb layout changes. 196 is not a multiple of 16, 32 or 64, so every
// BlockType has a partially filled most significant limb there.
//
// The methodology is described in blocktype_harness.hpp. Timings vary by core on a hybrid processor:
// pin the process (taskset -c 2 on Linux) before comparing runs.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include "blocktype_harness.hpp"

namespace sw { namespace universal {

	template<unsigned nbits, unsigned es>
	struct CfloatOf {
		template<typename bt> struct make { using type = cfloat<nbits, es, bt, true, false, false>; };
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

	std::string test_suite = "cfloat BlockType limb-processing benchmark";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	const std::uint64_t seed = 1904;
	Table table;

#if MANUAL_TESTING
	// one width, short samples: for a quick look while working on the limb loops
	Options opt{ 256, 3, 0.002 };
	printHeader("cfloat<nbits, es, bt, true, false, false>", opt);
	measureAll<128, CfloatOf<128, 15>::template make>(table, opt, seed);
	printTable("cfloat", table);
	nrOfFailedTestCases += crossCheck("cfloat", table);
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
	printHeader("cfloat<nbits, es, bt, true, false, false>", opt);

#if REGRESSION_LEVEL_1
	measureAll< 64, CfloatOf< 64, 11>::template make>(table, opt, seed);
	measureAll< 96, CfloatOf< 96, 15>::template make>(table, opt, seed);
	measureAll<128, CfloatOf<128, 15>::template make>(table, opt, seed);
	measureAll<196, CfloatOf<196, 19>::template make>(table, opt, seed);
	measureAll<256, CfloatOf<256, 19>::template make>(table, opt, seed);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	printTable("cfloat", table);
	nrOfFailedTestCases += crossCheck("cfloat", table);
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
