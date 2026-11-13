// edges.cpp: blockdigit shifts, conversion overflow, strings past radix 10, and division by zero
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2053) found that <<= INT_MIN and >>= INT_MIN called each
// other without end, that to_long_long() overflowed a signed accumulator for magnitudes past
// LLONG_MAX (undefined), that to_string() of a digit past 'Z' wrote non-ASCII bytes for a radix
// above 36, and that division by zero returned 0 with no way to raise. Every case below failed
// before the fix.
#define BLOCKDIGIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/utility/directives.hpp>
#include <climits>
#include <iostream>
#include <limits>
#include <string>
#include <universal/internal/blockdigit/blockdigit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

int VerifyShifts(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](long long got, long long expected, const char* label) {
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: blockdigit " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	blockdigit<8> a(1234);
	a <<= 2;   check(static_cast<long long>(a), 123400, "1234 << 2");
	a >>= 3;   check(static_cast<long long>(a), 123, "123400 >> 3");
	a >>= -1;  check(static_cast<long long>(a), 1230, "123 >> -1");
	blockdigit<8> b(1234); b <<= INT_MIN; check(static_cast<long long>(b), 0, "1234 << INT_MIN");
	blockdigit<8> c(1234); c >>= INT_MIN; check(static_cast<long long>(c), 0, "1234 >> INT_MIN");
	blockdigit<8, 16> h(0xABC); h <<= 1; check(static_cast<long long>(h), 0xABC0, "0xABC << 1 (radix 16)");
	return nrOfFailedTests;
}

int VerifyToLongLong(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](long long got, long long expected, const char* label) {
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	blockdigit<30> big;
	big.maxpos();   // 30 nines, past LLONG_MAX
	check(static_cast<long long>(big), std::numeric_limits<long long>::max(), "to_long_long(10^30 - 1) saturates");
	blockdigit<30> neg(big);
	neg = -neg;
	check(static_cast<long long>(neg), std::numeric_limits<long long>::min(), "to_long_long(-(10^30 - 1)) saturates");
	blockdigit<20> maxll(std::numeric_limits<long long>::max());
	check(static_cast<long long>(maxll), std::numeric_limits<long long>::max(), "LLONG_MAX round trip");
	blockdigit<20> minll(std::numeric_limits<long long>::min());
	check(static_cast<long long>(minll), std::numeric_limits<long long>::min(), "LLONG_MIN round trip");
	return nrOfFailedTests;
}

int VerifyStrings(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](const std::string& got, const std::string& expected) {
		bool ascii = true;
		for (char ch : got) ascii = ascii && (static_cast<unsigned char>(ch) < 128);
		if (got != expected || !ascii) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: to_string = \"" << got << "\" expected \"" << expected << "\"\n";
		}
	};
	check(blockdigit<4, 16>(255).to_string(), "FF");
	check(blockdigit<4, 36>(35).to_string(), "Z");
	check(blockdigit<4, 36>(36 * 36 - 1).to_string(), "ZZ");
	check(blockdigit<4, 256>(70000).to_string(), "1:17:112");   // 70000 = 1*256^2 + 17*256 + 112
	check(blockdigit<4, 100>(-4207).to_string(), "-42:7");
	check(blockdigit<4, 100>(0).to_string(), "0");
	return nrOfFailedTests;
}

// this suite sets BLOCKDIGIT_THROW_ARITHMETIC_EXCEPTION: x / 0 and x % 0 raise blockdigit_divide_by_zero
int VerifyDivideByZero(bool reportTestCases) {
	int nrOfFailedTests = 0;
#if UNIVERSAL_HAS_EXCEPTIONS
	auto expectThrow = [&](auto op, const char* label) {
		bool caught = false;
		try { op(); }
		catch (const blockdigit_divide_by_zero&) { caught = true; }
		if (!caught) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " did not raise blockdigit_divide_by_zero\n";
		}
	};
	expectThrow([] { blockdigit<8> a(5), z(0); a /= z; }, "blockdigit<8> / 0");
	expectThrow([] { blockdigit<8> a(5), z(0); a %= z; }, "blockdigit<8> % 0");
	expectThrow([] { blockdigit<8, 16> a(5), z(0); a /= z; }, "blockdigit<8,16> / 0");
#else
	(void)reportTestCases;
#endif
	return nrOfFailedTests;
}

} // namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockdigit edges";
	std::string test_tag    = "edges";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyStrings(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyShifts(reportTestCases), "blockdigit", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyToLongLong(reportTestCases), "blockdigit", "to_long_long");
	nrOfFailedTestCases += ReportTestResult(VerifyStrings(reportTestCases), "blockdigit", "to_string");
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZero(reportTestCases), "blockdigit", "divide by zero");
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
