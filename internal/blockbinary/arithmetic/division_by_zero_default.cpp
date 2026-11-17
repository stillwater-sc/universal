// division_by_zero_default.cpp: blockbinary x / 0 with BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION at its default, 0
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The header's contract (blockbinary.hpp): x / 0 and x % 0 return 0 when the guard is 0 and throw
// blockbinary_divide_by_zero when it is 1 (division_edges.cpp tests the 1). urdiv threw whatever
// the guard said; it follows the guard now (#2107). This test leaves the guard at its default.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <cstdint>
#include <string>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

static_assert(BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION == 0, "this test checks the default guard");

int VerifyQuietDivideByZero(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](bool ok, const char* label) {
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " is not 0\n";
		}
	};
	try {
		{ blockbinary<8, uint8_t> a(5), z(0); a /= z; check(a.iszero(), "blockbinary<8> 5 / 0"); }
		{ blockbinary<8, uint8_t> a(5), z(0); a %= z; check(a.iszero(), "blockbinary<8> 5 % 0"); }
		{ blockbinary<20, uint8_t> a(-7), z(0); a /= z; check(a.iszero(), "blockbinary<20> -7 / 0"); }
		{
			blockbinary<8, uint8_t> a(5), z(0);
			auto q = urdiv<8, 4>(a, z);
			check(q.iszero(), "urdiv<8,4>(5, 0)");
		}
		{
			blockbinary<16, uint16_t> a(-1000), z(0);
			auto q = urdiv<16, 3>(a, z);
			check(q.iszero(), "urdiv<16,3>(-1000, 0)");
		}
	}
	catch (...) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: a division by zero threw with the guard at 0\n";
	}
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

	std::string test_suite  = "blockbinary division by zero, default guard";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyQuietDivideByZero(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyQuietDivideByZero(reportTestCases), "blockbinary", "x / 0 returns 0");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
