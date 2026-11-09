// parse_complexity.cpp: guards on the cost of einteger parse(), as ratios against a naive parse
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// parse() has two fast paths, and each replaced a slower one that gave the same answers, so only
// the time tells them apart:
//   - decimal reads nine digits per big-integer multiply, value = value * 10^9 + chunk (#1988). It is
//     still quadratic, by design, but about nine times cheaper than one multiply per digit;
//   - binary, octal and hex write the digits' bits into the limbs directly, in linear time (#2010).
//     Binary and octal used to shift and add per digit: quadratic, with an allocation per digit.
// An absolute time bound would depend on the machine and the build (Debug, coverage at -O0,
// sanitizers). So each guard times parse() against the naive digit-at-a-time parse written here
// with the same einteger operations, on the same input, in the same build, and bounds the ratio.
// The measured ratios are far above the bounds: decimal ~9 (at -O0, -O2 and under ASan) against a
// bound of 3, binary and octal 40 or more against a bound of 8.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// the fastest of a few runs, in seconds: the minimum is the least disturbed by other load
template<typename Work>
double BestTime(Work work, int runs = 3) {
	double best = 1.0e30;
	for (int r = 0; r < runs; ++r) {
		const auto begin = std::chrono::steady_clock::now();
		work();
		const auto end = std::chrono::steady_clock::now();
		best = std::min(best, std::chrono::duration<double>(end - begin).count());
	}
	return best;
}

// a deterministic digit string of the given length over the first radix digits, leading digit nonzero
inline std::string Digits(size_t length, unsigned radix) {
	const char* alphabet = "0123456789";
	std::string s(length, '0');
	for (size_t i = 0; i < length; ++i) s[i] = alphabet[(i * 7 + 3) % radix];
	s[0] = '1';
	return s;
}

// the naive parse of digits in radix: value = value * radix + digit, one big-integer step per digit
template<typename BlockType>
einteger<BlockType> NaiveParse(const std::string& digits, unsigned radix) {
	using EInteger = einteger<BlockType>;
	EInteger value(0);
	int shift = 0;
	if (radix == 2) shift = 1;
	if (radix == 8) shift = 3;
	const EInteger multiplier(static_cast<long long>(radix));
	for (char c : digits) {
		if (shift > 0) value <<= shift; else value *= multiplier;
		value += EInteger(static_cast<long long>(c - '0'));
	}
	return value;
}

// parse() of prefix + digits must equal the naive parse and be at least minRatio times faster
template<typename BlockType>
int VerifyParseSpeedUp(const char* label, const std::string& prefix, size_t length, unsigned radix,
                       double minRatio, bool reportTestCases) {
	using EInteger = einteger<BlockType>;
	const std::string digits = Digits(length, radix);
	const std::string text = prefix + digits;
	EInteger fast, naive;
	const double tFast  = BestTime([&] { if (!parse(text, fast)) fast = EInteger(-1); });
	const double tNaive = BestTime([&] { naive = NaiveParse<BlockType>(digits, radix); });
	const double ratio = (tFast > 0.0) ? tNaive / tFast : 1.0e30;
	int nrOfFailedTests = 0;
	if (fast != naive) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: " << label << ": parse() and the naive parse disagree\n";
	}
	if (ratio < minRatio) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: " << label << ": parse() " << tFast << " s, naive " << tNaive
		                               << " s, ratio " << ratio << " < " << minRatio << '\n';
	}
	if (reportTestCases) std::cout << label << ": parse() " << tFast << " s, naive " << tNaive << " s, ratio " << ratio << '\n';
	return nrOfFailedTests;
}

}} // namespace sw::universal

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

	std::string test_suite  = "einteger parse cost against a naive parse (#1988, #2010)";
	std::string test_tag    = "parse cost";
	bool reportTestCases    = true;   // print the measured ratios
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using U32 = std::uint32_t;

#if MANUAL_TESTING

	VerifyParseSpeedUp<U32>("decimal 40000 digits", "", 40000, 10, 0.0, true);
	VerifyParseSpeedUp<U32>("binary 80000 bits", "0b", 80000, 2, 0.0, true);
	VerifyParseSpeedUp<U32>("octal 80000 bits", "0", 80000 / 3, 8, 0.0, true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyParseSpeedUp<U32>("decimal 4000 digits", "", 4000, 10, 3.0, reportTestCases), "decimal", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseSpeedUp<U32>("binary 20000 bits", "0b", 20000, 2, 8.0, reportTestCases), "binary", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseSpeedUp<U32>("octal 20000 bits", "0", 20000 / 3, 8, 8.0, reportTestCases), "octal", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseSpeedUp<std::uint8_t>("binary 20000 bits", "0b", 20000, 2, 8.0, reportTestCases), "binary uint8_t", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyParseSpeedUp<U32>("decimal 16000 digits", "", 16000, 10, 3.0, reportTestCases), "decimal", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseSpeedUp<U32>("binary 80000 bits", "0b", 80000, 2, 8.0, reportTestCases), "binary", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseSpeedUp<U32>("octal 80000 bits", "0", 80000 / 3, 8, 8.0, reportTestCases), "octal", test_tag);
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
