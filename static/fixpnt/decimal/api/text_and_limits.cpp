// text_and_limits.cpp: dfixpnt numeric_limits values, text validation and the sign of a zero ceil (#2112)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <limits>
#include <string>

#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

int CheckTrue(bool ok, const std::string& label) {
	if (ok) return 0;
	std::cout << "FAIL: " << label << '\n';
	return 1;
}

// Fill a stretch of the stack with a pattern, so that a value built in the next frame from
// uninitialized storage shows it: epsilon(), round_error() and denorm_min() set only digit 0 of a
// default-constructed dfixpnt, and read the rest uninitialized (#2112, found under Valgrind).
#if defined(_MSC_VER)
__declspec(noinline)
#else
__attribute__((noinline))
#endif
void PolluteStack() {
	volatile unsigned char junk[4096];
	for (unsigned i = 0; i < sizeof(junk); ++i) junk[i] = 0x99;
	(void)junk[0];
}

template<typename D>
#if defined(_MSC_VER)
__declspec(noinline)
#else
__attribute__((noinline))
#endif
int CheckLimits(const char* label) {
	int fails = 0;
	const D minpos(SpecificValue::minpos);
	fails += CheckTrue(std::numeric_limits<D>::epsilon() == minpos, std::string(label) + " epsilon() == 10^-radix");
	fails += CheckTrue(std::numeric_limits<D>::round_error() == minpos, std::string(label) + " round_error() == 10^-radix");
	fails += CheckTrue(std::numeric_limits<D>::denorm_min() == minpos, std::string(label) + " denorm_min() == 10^-radix");
	fails += CheckTrue(std::numeric_limits<D>::round_style == std::round_to_nearest, std::string(label) + " round_style is round_to_nearest");
	return fails;
}

template<typename D>
int VerifyLimits(const char* label) {
	PolluteStack();
	return CheckLimits<D>(label);
}

// assign() and operator>> took any character as a digit: assign("abc") was 123.00
template<typename D>
int VerifyText() {
	int fails = 0;
	D v;
	v.assign("abc");
	fails += CheckTrue(v.iszero(), "assign(\"abc\") leaves 0");
	v.assign("12x.5");
	fails += CheckTrue(v.iszero(), "assign(\"12x.5\") leaves 0");
	v.assign("1.2.3");
	fails += CheckTrue(v.iszero(), "assign(\"1.2.3\") leaves 0");
	v.assign("-");
	fails += CheckTrue(v.iszero(), "assign(\"-\") leaves 0");
	v.assign("-1.25");
	fails += CheckTrue(v.to_string() == "-1.25", "assign(\"-1.25\")");
	v.assign("+.5");
	fails += CheckTrue(v.to_string() == "0.50", "assign(\"+.5\")");
	v.assign("7.");
	fails += CheckTrue(v.to_string() == "7.00", "assign(\"7.\")");

	D w(3);
	fails += CheckTrue(!parse("abc", w) && w.to_string() == "3.00", "parse(\"abc\") fails and leaves the value");
	fails += CheckTrue(parse("-42.5", w) && w.to_string() == "-42.50", "parse(\"-42.5\")");

	std::istringstream bad("x1.2");
	D x(7);
	bad >> x;
	fails += CheckTrue(bad.fail() && x.to_string() == "7.00", "operator>> of \"x1.2\" sets failbit and leaves the value");
	std::istringstream good("12.75");
	good >> x;
	fails += CheckTrue(!good.fail() && x.to_string() == "12.75", "operator>> of \"12.75\"");
	return fails;
}

// a magnitude that rounds to 0 is +0 (#2076): ceil(-0.5) was -0
template<typename D>
int VerifyZeroSign() {
	int fails = 0;
	const D c = ceil(D(-0.5));
	fails += CheckTrue(c.iszero() && !c.sign(), "ceil(-0.5) is +0");
	const D c2 = ceil(D(-0.01));
	fails += CheckTrue(c2.iszero() && !c2.sign(), "ceil(-0.01) is +0");
	const D f = floor(D(-0.5));
	fails += CheckTrue(f.to_string() == "-1.00", "floor(-0.5) is -1");
	const D c3 = ceil(D(-1.5));
	fails += CheckTrue(c3.to_string() == "-1.00", "ceil(-1.5) is -1");
	return fails;
}

}} // namespace sw::universal

// Regression testing guards
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

	std::string test_suite  = "dfixpnt numeric_limits, text validation and zero sign (#2112)";
	std::string test_tag    = "text and limits";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	std::cout << "epsilon of dfixpnt<8,3> = " << std::numeric_limits<dfixpnt<8, 3>>::epsilon() << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<dfixpnt<8, 3, DecimalEncoding::BCD>>("dfixpnt<8,3,BCD>"), test_tag, "numeric_limits, BCD");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<dfixpnt<8, 3, DecimalEncoding::BID>>("dfixpnt<8,3,BID>"), test_tag, "numeric_limits, BID");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<dfixpnt<9, 3, DecimalEncoding::DPD>>("dfixpnt<9,3,DPD>"), test_tag, "numeric_limits, DPD");
	nrOfFailedTestCases += ReportTestResult(VerifyText<dfixpnt<6, 2, DecimalEncoding::BCD>>(), test_tag, "assign, parse, operator>>, BCD");
	nrOfFailedTestCases += ReportTestResult(VerifyText<dfixpnt<6, 2, DecimalEncoding::DPD>>(), test_tag, "assign, parse, operator>>, DPD");
	nrOfFailedTestCases += ReportTestResult(VerifyZeroSign<dfixpnt<4, 2, DecimalEncoding::BCD>>(), test_tag, "ceil and floor zero sign");
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
