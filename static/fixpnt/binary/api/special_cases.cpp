// special_cases.cpp: fixpnt edge cases and helpers from the QA sweep in #2103 (#2111)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <limits>
#include <string>

#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

template<typename F>
int CheckValue(const F& got, const F& expected, const std::string& label) {
	if (got == expected) return 0;
	std::cout << "FAIL: " << label << ": " << to_binary(got) << " (" << got << "), expected " << to_binary(expected) << " (" << expected << ")\n";
	return 1;
}

int CheckTrue(bool ok, const std::string& label) {
	if (ok) return 0;
	std::cout << "FAIL: " << label << '\n';
	return 1;
}

// scale(maxneg) is -1 when there are no integer bits: maxneg is -1/2 (it returned 0)
int VerifyScaleOfMaxneg() {
	int fails = 0;
	fails += CheckTrue(scale(fixpnt<8, 8>(SpecificValue::maxneg)) == -1, "scale(fixpnt<8,8> maxneg) == -1");
	fails += CheckTrue(scale(fixpnt<8, 4>(SpecificValue::maxneg)) == 3, "scale(fixpnt<8,4> maxneg) == 3");
	fails += CheckTrue(scale(fixpnt<8, 8>(-0.25)) == -2, "scale(fixpnt<8,8>(-0.25)) == -2");
	return fails;
}

// |maxneg| does not fit: Saturate gives maxpos (it returned maxneg), Modulo wraps to maxneg
int VerifyAbsOfMaxneg() {
	int fails = 0;
	using S = fixpnt<8, 4, Saturate, std::uint8_t>;
	using M = fixpnt<8, 4, Modulo, std::uint8_t>;
	fails += CheckValue(abs(S(SpecificValue::maxneg)), S(SpecificValue::maxpos), "abs(Saturate maxneg)");
	fails += CheckValue(abs(M(SpecificValue::maxneg)), M(SpecificValue::maxneg), "abs(Modulo maxneg)");
	fails += CheckValue(abs(S(-2.5)), S(2.5), "abs(Saturate -2.5)");
	using W = fixpnt<64, 32, Saturate, std::uint32_t>;
	fails += CheckValue(abs(W(SpecificValue::maxneg)), W(SpecificValue::maxpos), "abs(fixpnt<64,32,Saturate> maxneg)");
	return fails;
}

// the arithmetic rounds to nearest even, which numeric_limits now says (it said round_toward_zero)
int VerifyRoundStyle() {
	int fails = 0;
	fails += CheckTrue(std::numeric_limits<fixpnt<8, 4, Modulo>>::round_style == std::round_to_nearest, "numeric_limits<fixpnt<8,4,Modulo>>::round_style");
	fails += CheckTrue(std::numeric_limits<fixpnt<32, 16, Saturate>>::round_style == std::round_to_nearest, "numeric_limits<fixpnt<32,16,Saturate>>::round_style");
	return fails;
}

// helpers that did not compile when used: nexttoward called isnar(), rsqrt called reciprocate(),
// fraction_value called reset()
int VerifyHelpers() {
	int fails = 0;
	using F = fixpnt<16, 8, Modulo, std::uint8_t>;
	F one(1), two(2), ulp;
	ulp.setbits(1);
	fails += CheckValue(nextafter(one, two), one + ulp, "nextafter(1, 2)");
	fails += CheckValue(nextafter(one, F(0)), one - ulp, "nextafter(1, 0)");
	fails += CheckValue(nextafter(one, one), one, "nextafter(1, 1)");
	fails += CheckValue(nexttoward(one, two), one + ulp, "nexttoward(1, 2)");
	fails += CheckValue(nexttoward(one, F(-3)), one - ulp, "nexttoward(1, -3)");
	fails += CheckValue(rsqrt(F(4)), F(0.5), "rsqrt(4)");
	fails += CheckValue(rsqrt(F(0.25)), F(2), "rsqrt(0.25)");
	fails += CheckTrue(fraction_value(fixpnt<8, 4>(2.75)) == 0.75L, "fraction_value(2.75) == 0.75");
	fails += CheckTrue(fraction_value(fixpnt<8, 4>(3.0)) == 0.0L, "fraction_value(3.0) == 0");
	// integer() is floor and fraction() is x - floor(x), from the encoding
	using W = fixpnt<72, 4, Modulo, std::uint32_t>;
	W x;
	x.setbits(0);
	x.setbit(4 + 60, true);   // 2^60
	x.setbit(4, true);        // + 1
	x.setbit(3, true);        // + 0.5
	W expectedInt(x);
	expectedInt.setbit(3, false);
	W expectedFrac;
	expectedFrac.setbits(0);
	expectedFrac.setbit(3, true);
	fails += CheckValue(x.integer(), expectedInt, "integer(2^60 + 1.5)");
	fails += CheckValue(x.fraction(), expectedFrac, "fraction(2^60 + 1.5)");
	fails += CheckValue(fixpnt<8, 4>(-2.5).integer(), fixpnt<8, 4>(-3), "integer(-2.5)");
	fails += CheckValue(fixpnt<8, 4>(-2.5).fraction(), fixpnt<8, 4>(0.5), "fraction(-2.5)");
	return fails;
}

// assign(string) parses decimals: the branch was a stub that set 6.90234375, and "-1.5" threw
int VerifyAssign() {
	int fails = 0;
	using F = fixpnt<8, 4, Modulo, std::uint8_t>;
	using S = fixpnt<8, 4, Saturate, std::uint8_t>;
	F f;
	fails += CheckValue(f.assign("-1.5"), F(-1.5), "assign(\"-1.5\")");
	fails += CheckValue(f.assign("2.25"), F(2.25), "assign(\"2.25\")");
	fails += CheckValue(f.assign("5"), F(5), "assign(\"5\")");
	fails += CheckValue(f.assign("+0.0625"), F(0.0625), "assign(\"+0.0625\")");
	fails += CheckValue(f.assign("0.03125"), F(0), "assign(\"0.03125\"), a tie to even");
	fails += CheckValue(f.assign("0.09375"), F(0.125), "assign(\"0.09375\"), a tie to even");
	fails += CheckValue(f.assign("0b0011.1000"), F(3.5), "assign(\"0b0011.1000\")");
	fails += CheckValue(f.assign("abc"), F(0), "assign(\"abc\") leaves 0");
	S s;
	fails += CheckValue(s.assign("100"), S(SpecificValue::maxpos), "Saturate assign(\"100\")");
	fails += CheckValue(s.assign("-100"), S(SpecificValue::maxneg), "Saturate assign(\"-100\")");
	return fails;
}

// fixpnt - integer comparisons are exact: the integer was converted to the fixpnt first, which
// wrapped in Modulo, so fixpnt<8,4,Modulo>(-8) == 8 was true
int VerifyIntegerComparisons() {
	int fails = 0;
	using M = fixpnt<8, 4, Modulo, std::uint8_t>;
	using S = fixpnt<8, 4, Saturate, std::uint8_t>;
	const M m(-8);
	fails += CheckTrue(!(m == 8), "Modulo -8 == 8 is false");
	fails += CheckTrue(m != 8, "Modulo -8 != 8");
	fails += CheckTrue(m < 8, "Modulo -8 < 8");
	fails += CheckTrue(!(8 == m), "8 == Modulo -8 is false");
	fails += CheckTrue(8 > m, "8 > Modulo -8");
	fails += CheckTrue(m == -8, "Modulo -8 == -8");
	fails += CheckTrue(M(7) < 100, "Modulo 7 < 100");
	fails += CheckTrue(M(7) > -100, "Modulo 7 > -100");
	fails += CheckTrue(M(7) <= 7 && M(7) >= 7, "Modulo 7 <= 7 and >= 7");
	fails += CheckTrue(!(S(SpecificValue::maxpos) == 8), "Saturate maxpos == 8 is false");
	fails += CheckTrue(S(SpecificValue::maxpos) < 8, "Saturate maxpos < 8");
	fails += CheckTrue(M(1.5) > 1 && M(1.5) < 2, "1 < 1.5 < 2");
	fails += CheckTrue(M(-1.5) < -1 && M(-1.5) > -2, "-2 < -1.5 < -1");
	fails += CheckTrue(M(3) == 3u && M(3) < 4ull && !(M(-1) == 4294967295u), "unsigned literals");
	using W = fixpnt<64, 0, Modulo, std::uint64_t>;
	fails += CheckTrue(W(-1) < 18446744073709551615ull, "fixpnt<64,0>(-1) < ULLONG_MAX");
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

	std::string test_suite  = "fixpnt special cases (#2111)";
	std::string test_tag    = "special cases";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	std::cout << "scale(fixpnt<8,8> maxneg) = " << scale(fixpnt<8, 8>(SpecificValue::maxneg)) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyScaleOfMaxneg(), test_tag, "scale(maxneg)");
	nrOfFailedTestCases += ReportTestResult(VerifyAbsOfMaxneg(), test_tag, "abs(maxneg)");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundStyle(), test_tag, "numeric_limits round_style");
	nrOfFailedTestCases += ReportTestResult(VerifyHelpers(), test_tag, "nextafter, nexttoward, rsqrt, fraction_value, integer, fraction");
	nrOfFailedTestCases += ReportTestResult(VerifyAssign(), test_tag, "assign(string)");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerComparisons(), test_tag, "integer comparisons");
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
