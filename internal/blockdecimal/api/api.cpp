// api.cpp: blockdecimal construction, arithmetic, shifts and conversion across the encodings
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This suite was an empty shell. The QA sweep of #2029 (tracking issue #2053) found that the BID
// encoding kept a value modulo 2^nbits rather than modulo 10^ndigits (12345 into 3 digits read
// back 57, where BCD and DPD keep 345), that <<= INT_MIN and >>= INT_MIN called each other without
// end, and that to_double() of more than 308 digits was NaN. Arithmetic beyond division had no
// test at all.
#include <universal/utility/directives.hpp>
#include <climits>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <universal/internal/blockdecimal/blockdecimal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// + - * / % of 4-digit decimals against native arithmetic, modulo 10^4 as a 4-digit register wraps
template<DecimalEncoding E>
int VerifyArithmetic(bool reportTestCases) {
	using D = blockdecimal<4, E>;
	constexpr long long modulus = 10000;
	auto wrap = [](long long v) {   // keep the low 4 decimal digits of the magnitude, and the sign
		const long long m = (v < 0 ? -v : v) % modulus;
		return v < 0 ? -m : m;
	};
	int nrOfFailedTests = 0;
	const long long values[] = { 0, 1, -1, 7, -9, 42, 99, -100, 999, 1234, -4321, 9999, -9999 };
	for (long long x : values) {
		for (long long y : values) {
			const D a(x), b(y);
			long long got[5], expected[5];
			got[0] = (a + b).to_long_long();  expected[0] = wrap(x + y);
			got[1] = (a - b).to_long_long();  expected[1] = wrap(x - y);
			got[2] = (a * b).to_long_long();  expected[2] = wrap(x * y);
			const int n = (y != 0) ? 5 : 3;
			if (y != 0) {
				got[3] = (a / b).to_long_long();  expected[3] = x / y;
				got[4] = (a % b).to_long_long();  expected[4] = x % y;
			}
			const char* ops = "+-*/%";
			for (int k = 0; k < n; ++k) {
				if (got[k] != expected[k]) {
					++nrOfFailedTests;
					if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: blockdecimal<4> " << x << ' ' << ops[k] << ' ' << y << " = " << got[k] << " expected " << expected[k] << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// a value past ndigits keeps its low digits, in every encoding
int VerifyWrapAcrossEncodings(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const long long bcd = blockdecimal<3, DecimalEncoding::BCD>(12345).to_long_long();
	const long long bid = blockdecimal<3, DecimalEncoding::BID>(12345).to_long_long();
	const long long dpd = blockdecimal<3, DecimalEncoding::DPD>(12345).to_long_long();
	if (bcd != 345 || bid != 345 || dpd != 345) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: 12345 into 3 digits: BCD " << bcd << " BID " << bid << " DPD " << dpd << " expected 345\n";
	}
	const long long bid_neg = blockdecimal<3, DecimalEncoding::BID>(-12345).to_long_long();
	if (bid_neg != -345) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: -12345 into 3 BID digits = " << bid_neg << '\n';
	}
	return nrOfFailedTests;
}

template<DecimalEncoding E>
int VerifyShifts(bool reportTestCases) {
	using D = blockdecimal<8, E>;
	int nrOfFailedTests = 0;
	auto check = [&](long long got, long long expected, const char* label) {
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: blockdecimal<8> " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	D a(1234);
	a <<= 2;  check(a.to_long_long(), 123400, "1234 << 2");
	a >>= 3;  check(a.to_long_long(), 123, "123400 >> 3");
	a <<= -1; check(a.to_long_long(), 12, "123 << -1");
	D b(1234); b <<= 8;  check(b.to_long_long(), 0, "1234 << 8");
	D c(1234); c <<= INT_MIN; check(c.to_long_long(), 0, "1234 << INT_MIN");
	D d(-1234); d >>= INT_MIN; check(d.to_long_long(), 0, "-1234 >> INT_MIN");
	D e(1234); e <<= INT_MAX; check(e.to_long_long(), 0, "1234 << INT_MAX");
	return nrOfFailedTests;
}

int VerifyToDouble(bool reportTestCases) {
	int nrOfFailedTests = 0;
	blockdecimal<400> big;
	big.maxval();   // 400 nines, past double's range
	const double d = big.to_double();
	if (!std::isinf(d) || d < 0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: to_double of 400 nines = " << d << " expected +inf\n";
	}
	blockdecimal<400> small(123456789);   // leading zero digits past 10^308
	if (small.to_double() != 123456789.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: to_double of a 400-digit 123456789 = " << small.to_double() << '\n';
	}
	blockdecimal<20> n(-987654321);
	if (n.to_double() != -987654321.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: to_double(-987654321) = " << n.to_double() << '\n';
	}
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

	std::string test_suite  = "blockdecimal API validation";
	std::string test_tag    = "API";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyArithmetic<DecimalEncoding::BCD>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<DecimalEncoding::BCD>(reportTestCases), "blockdecimal<4,BCD>", "+ - * / %");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<DecimalEncoding::BID>(reportTestCases), "blockdecimal<4,BID>", "+ - * / %");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<DecimalEncoding::DPD>(reportTestCases), "blockdecimal<4,DPD>", "+ - * / %");
	nrOfFailedTestCases += ReportTestResult(VerifyWrapAcrossEncodings(reportTestCases), "blockdecimal<3>", "wrap past ndigits");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<DecimalEncoding::BCD>(reportTestCases), "blockdecimal<8,BCD>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<DecimalEncoding::BID>(reportTestCases), "blockdecimal<8,BID>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyToDouble(reportTestCases), "blockdecimal", "to_double");
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
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
