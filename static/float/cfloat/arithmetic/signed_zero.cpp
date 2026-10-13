// signed_zero.cpp: cfloat zero signs follow IEEE 754
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for the sign of zero results in cfloat addition and subtraction. When the
// left operand was zero, operator+= returned the right operand, so a sum of two zeros took
// the sign of the right operand: +0 + -0 and 0 - 0 gave -0. Under IEEE 754 roundTiesToEven
// two zeros sum to -0 only when both are -0. The expected signs below are what IEEE 754
// binary arithmetic produces, and they hold for every cfloat configuration this test runs.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
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
	// "min" is the smallest positive value of the type, so the underflow rows apply to every configuration
	const Case cases[] = {
		{ '*', "-0", "2", "-0" },       { '*', "0", "-2", "-0" },      { '*', "-0", "-2", "+0" },
		{ '*', "-0", "0", "-0" },       { '/', "-0", "2", "-0" },      { '/', "0", "-2", "-0" },
		{ '/', "-0", "-2", "+0" },      { '+', "-0", "0", "+0" },      { '+', "0", "-0", "+0" },
		{ '+', "-0", "-0", "-0" },      { '+', "0", "0", "+0" },       { '-', "-0", "-0", "+0" },
		{ '-', "0", "0", "+0" },        { '-', "-0", "0", "-0" },      { '-', "0", "-0", "+0" },
		{ '+', "-2", "2", "+0" },       { '-', "2", "2", "+0" },       { '-', "-2", "-2", "+0" },
		{ '+', "2", "-0", "+x" },       { '+', "-0", "2", "+x" },      { '/', "2", "inf", "+0" },
		{ '/', "-2", "inf", "-0" },     { '/', "2", "-inf", "-0" },    { '*', "min", "-0.25", "-0" },
		{ '*', "min", "0.25", "+0" },   { '/', "-min", "4", "-0" },    { 'n', "0", "", "-0" },
		{ 'n', "-0", "", "+0" },
	};
	auto value = [](const std::string& s) {
		F v;
		if (s == "min") v = F(SpecificValue::minpos);
		else if (s == "-min") { v = F(SpecificValue::minpos); v = -v; }
		else if (s == "inf") v.setinf(false);
		else if (s == "-inf") v.setinf(true);
		else if (s == "-0") { v.setzero(); v.setsign(true); }
		else v = F(std::stod(s));
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

	std::string test_suite  = "cfloat signed zeros";
	std::string test_tag    = "signed zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		single pz(0.0f), nz(-0.0f);
		std::cout << "+0 + -0 = " << to_binary(pz + nz) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	using fp8   = cfloat<8, 2, std::uint8_t, true, false, false>;
	using fp16  = cfloat<16, 5, std::uint16_t, true, false, false>;
	using bf16  = cfloat<16, 8, std::uint16_t, true, false, false>;
	using fp32  = cfloat<32, 8, std::uint32_t, true, false, false>;
	using fp64  = cfloat<64, 11, std::uint64_t, true, false, false>;
	using fp32b = cfloat<32, 8, std::uint8_t, true, false, false>;   // multi-block storage
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeros<fp8>("cfloat<8,2>", reportTestCases), "cfloat<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeros<fp16>("cfloat<16,5>", reportTestCases), "cfloat<16,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeros<bf16>("cfloat<16,8>", reportTestCases), "cfloat<16,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeros<fp32>("cfloat<32,8>", reportTestCases), "cfloat<32,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeros<fp64>("cfloat<64,11>", reportTestCases), "cfloat<64,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeros<fp32b>("cfloat<32,8,uint8_t>", reportTestCases), "cfloat<32,8,uint8_t>", test_tag);
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
