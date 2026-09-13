// signed_zero.cpp: dfloat zero and infinity signs follow IEEE 754
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for #1491. dfloat did not follow IEEE 754's signed-zero rules:
//   -0 * 2 and 0 / -2 gave +0 (a zero product or quotient takes the XOR of the signs),
//   -0 + +0 and -0 - -0 gave -0 (two zeros sum to -0 only when both are -0),
//   -2 + 2 gave -0 (an exact zero sum of opposite signs is +0 under roundTiesToEven),
//   -(+0) gave +0 (negate flips the sign of every value, zeros included).
// The expected signs come from Python's decimal module in the IEEE context that matches
// decimal32. They hold for every precision and encoding, which this test runs through.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

template<typename F>
std::string SignOf(const F& v) {
	if (v.isnan()) return "nan";
	if (v.isinf()) return v.sign() ? "-inf" : "+inf";
	if (v.iszero()) return v.sign() ? "-0" : "+0";
	return v.sign() ? "-x" : "+x";
}

template<typename F>
int VerifySignedZeros(const char* tag, bool reportTestCases) {
	struct Case { char op; const char* a; const char* b; const char* want; };
	// "min" is the smallest positive value of the type, so the underflow rows apply to every precision
	const Case cases[] = {
		{ '*', "-0", "2", "-0" },       { '*', "0", "-2", "-0" },      { '*', "-0", "-2", "+0" },
		{ '*', "-0", "0", "-0" },       { '/', "-0", "2", "-0" },      { '/', "0", "-2", "-0" },
		{ '/', "-0", "-2", "+0" },      { '+', "-0", "0", "+0" },      { '+', "0", "-0", "+0" },
		{ '+', "-0", "-0", "-0" },      { '+', "0", "0", "+0" },       { '-', "-0", "-0", "+0" },
		{ '-', "0", "0", "+0" },        { '-', "-0", "0", "-0" },      { '-', "0", "-0", "+0" },
		{ '+', "-2", "2", "+0" },       { '-', "2", "2", "+0" },       { '-', "-2", "-2", "+0" },
		{ '+', "2", "-0", "+x" },       { '+', "-0", "2", "+x" },      { '/', "2", "inf", "+0" },
		{ '/', "-2", "inf", "-0" },     { '/', "2", "-inf", "-0" },    { '*', "min", "-0.1", "-0" },
		{ '*', "min", "0.1", "+0" },    { '/', "-min", "10", "-0" },   { 'n', "0", "", "-0" },
		{ 'n', "-0", "", "+0" },
	};
	auto value = [](const std::string& s) {
		F v;
		if (s == "min") v = F(SpecificValue::minpos);
		else if (s == "-min") { v = F(SpecificValue::minpos); v = -v; }
		else v.assign(s);
		return v;
	};
	int nrOfFailedTests = 0;
	for (const Case& c : cases) {
		const F a = value(c.a);
		F r;
		switch (c.op) {
		case '+': r = a + value(c.b); break;
		case '-': r = a - value(c.b); break;
		case '*': r = a * value(c.b); break;
		case '/': r = a / value(c.b); break;
		default: r = -a; break;
		}
		const std::string got = SignOf(r);
		if (got != c.want) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: " << tag << ' ' << (c.op == 'n' ? "-(" : "(") << c.a
				          << (c.op == 'n' ? "" : std::string(") ") + c.op + " (" + c.b) << ") = " << got
				          << ", expected " << c.want << '\n';
			}
		}
	}
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

	std::string test_suite  = "dfloat signed zeros (#1491)";
	std::string test_tag    = "signed zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		dfloat<7, 6, DecimalEncoding::BID, std::uint32_t> nz(0), two(2);
		nz = -nz;
		std::cout << "-0 * 2 = " << (nz * two) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	using d32b  = dfloat<7, 6, DecimalEncoding::BID, std::uint32_t>;
	using d32d  = dfloat<7, 6, DecimalEncoding::DPD, std::uint32_t>;
	using d64b  = dfloat<16, 8, DecimalEncoding::BID, std::uint32_t>;
	using d64d  = dfloat<16, 8, DecimalEncoding::DPD, std::uint32_t>;
	using d128b = dfloat<34, 12, DecimalEncoding::BID, std::uint32_t>;
	using n4    = dfloat<4, 6, DecimalEncoding::BID, std::uint32_t>;
	using n5    = dfloat<5, 6, DecimalEncoding::DPD, std::uint32_t>;
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySignedZeros<d32b>("decimal32 BID", reportTestCases), "decimal32 BID", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySignedZeros<d32d>("decimal32 DPD", reportTestCases), "decimal32 DPD", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySignedZeros<d64b>("decimal64 BID", reportTestCases), "decimal64 BID", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySignedZeros<d64d>("decimal64 DPD", reportTestCases), "decimal64 DPD", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySignedZeros<d128b>("decimal128 BID", reportTestCases), "decimal128 BID", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySignedZeros<n4>("dfloat<4,6> BID", reportTestCases), "dfloat<4,6> BID", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySignedZeros<n5>("dfloat<5,6> DPD", reportTestCases), "dfloat<5,6> DPD", test_tag);
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
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
