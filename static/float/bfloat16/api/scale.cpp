// scale.cpp: bfloat16 scale() across the whole encoding space, subnormals included
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// scale() returns the unbiased binary exponent: the e with |value| in
// [2^e, 2^(e+1)). It read the biased exponent field and subtracted the bias, which
// is right for normals and wrong for every subnormal: their exponent field is zero,
// so it answered -127 for all 254 of them when the true scale runs from -127 down
// to -133 with the leading one of the 7-bit fraction.
//
// Nothing in bfloat16's own suites noticed, because none of them asked about a
// subnormal's scale. It surfaced from elreal, where block<FpType>::scale_of_v()
// calls scale() and the block's combined exponent is scale_of_v() + exp -- so a
// block holding a subnormal carried a combined exponent up to 6 binades wrong, and
// the 0-overlap accounting built on it was wrong with it (universal#1051).
//
// The check here is exhaustive rather than sampled: bfloat16 has 65536 encodings,
// so every one can be compared against std::ilogb of the exactly-equal double.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// scale() against std::ilogb of the same value widened to double. bfloat16 ->
	// double is exact, so ilogb there is the exact answer for the bfloat16 value.
	int VerifyScaleAgainstIlogb(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;
		int subnormalsSeen = 0;
		for (unsigned bits = 0; bits < 0x10000u; ++bits) {
			bfloat16 v;
			v.setbits(bits);
			const double d = double(v);
			if (d == 0.0 || !std::isfinite(d)) continue;      // zero/inf/nan have no scale
			if (((bits >> 7) & 0xFFu) == 0u) ++subnormalsSeen;
			const int expected = std::ilogb(d);
			if (v.scale() != expected) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests <= 8)
					std::cerr << "  FAIL bits=0x" << std::hex << bits << std::dec
					          << " scale()=" << v.scale() << " expected " << expected << '\n';
			}
		}
		// a run that saw no subnormal would pass vacuously
		if (subnormalsSeen == 0) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "  FAIL no subnormal encodings were exercised\n";
		}
		return nrOfFailedTests;
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
	using namespace sw::universal;

	std::string test_suite  = "bfloat16 scale() across the encoding space";
	std::string test_tag    = "scale";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyScaleAgainstIlogb(true), test_tag, "scale vs ilogb");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyScaleAgainstIlogb(reportTestCases), test_tag, "scale vs ilogb, all 65536 encodings");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
