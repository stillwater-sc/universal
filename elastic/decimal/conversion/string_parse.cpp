// string_parse.cpp: regression tests for string parsing of edecimal
//                  (Phase E of #835 -- operator>> hygiene; #854 -- extended grammar)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// edecimal::parse accepts integer, decimal-point, and scientific-notation
// inputs.  Forms whose effective decimal exponent is negative (i.e. that
// carry fractional digits with no matching positive exponent) are rejected
// because edecimal is an integer-only number system and the issue spec
// explicitly requires that we preserve the input digits exactly rather than
// silently truncate them.  operator>> sets failbit on parse failure (#835
// Phase E).

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/test_reporters.hpp>

namespace {

struct CerrSilencer {
	std::ostringstream sink;
	std::streambuf*    old;
	CerrSilencer() : old(std::cerr.rdbuf(sink.rdbuf())) {}
	~CerrSilencer() { std::cerr.rdbuf(old); }
	CerrSilencer(const CerrSilencer&)            = delete;
	CerrSilencer& operator=(const CerrSilencer&) = delete;
};

int CheckParse(const char* input, const char* expected_decimal, bool reportTestCases) {
	using namespace sw::universal;
	edecimal v;
	if (!v.parse(input)) {
		if (reportTestCases) std::cout << "FAIL parse rejected: '" << input << "'\n";
		return 1;
	}
	std::ostringstream os;
	os << v;
	if (os.str() != std::string(expected_decimal)) {
		if (reportTestCases) {
			std::cout << "FAIL parse('" << input << "') = "
			          << os.str() << " expected " << expected_decimal << '\n';
		}
		return 1;
	}
	return 0;
}

int CheckReject(const char* input, bool reportTestCases) {
	using namespace sw::universal;
	edecimal v;
	if (v.parse(input)) {
		if (reportTestCases) {
			std::ostringstream os; os << v;
			std::cout << "FAIL: '" << input << "' should be rejected, got " << os.str() << '\n';
		}
		return 1;
	}
	return 0;
}

}  // namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "edecimal string parse (issues #835 Phase E, #854)";
	bool reportTestCases    = true;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- integer-format (canonical / pre-existing path) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("0",        "0",        reportTestCases);
		nrOfFailedTestCases += CheckParse("42",       "42",       reportTestCases);
		nrOfFailedTestCases += CheckParse("+42",      "42",       reportTestCases);
		nrOfFailedTestCases += CheckParse("-1000",    "-1000",    reportTestCases);
		nrOfFailedTestCases += CheckParse(
			"1234567890123456789012345",
			"1234567890123456789012345", reportTestCases);
		nrOfFailedTestCases += CheckParse(
			"-1234567890123456789012345",
			"-1234567890123456789012345", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "integer parse", "edecimal parse");
	}

	// ----- scientific notation that yields exact integers -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("1e0",       "1",            reportTestCases);
		nrOfFailedTestCases += CheckParse("1e10",      "10000000000",  reportTestCases);
		nrOfFailedTestCases += CheckParse("-1e10",     "-10000000000", reportTestCases);
		nrOfFailedTestCases += CheckParse("3e+2",      "300",          reportTestCases);
		// 3.14e+200 -> 314 followed by 198 zeros: 200 digits total.
		nrOfFailedTestCases += CheckParse(
			"3.14e+200",
			"314"
			"000000000000000000000000000000000000000000000000000000000000000000000000000"
			"000000000000000000000000000000000000000000000000000000000000000000000000000"
			"000000000000000000000000000000000000000000000000",
			reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "scientific exact", "edecimal parse");
	}

	// ----- decimal-point form: accepted iff effective exponent is non-negative -----
	{
		int start = nrOfFailedTestCases;
		// Accept: the decimal point shifts the exponent, but the combined value
		// is still an integer (no precision loss).
		nrOfFailedTestCases += CheckParse("3.14e2",    "314",       reportTestCases);
		nrOfFailedTestCases += CheckParse("-2.5e1",    "-25",       reportTestCases);
		nrOfFailedTestCases += CheckParse("1.5e10",    "15000000000", reportTestCases);
		nrOfFailedTestCases += CheckParse("-1.0e3",    "-1000",     reportTestCases);
		// Edge syntax: trailing dot, leading dot
		nrOfFailedTestCases += CheckParse("5.",        "5",         reportTestCases);
		nrOfFailedTestCases += CheckParse(".5e1",      "5",         reportTestCases);
		nrOfFailedTestCases += CheckParse("-.25e2",    "-25",       reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "decimal point (integer result)", "edecimal parse");
	}

	// ----- fractional / negative-exponent forms must be rejected -----
	// edecimal cannot represent these without precision loss, and the spec
	// requires preserving the input digits exactly.
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckReject("3.14",     reportTestCases);
		nrOfFailedTestCases += CheckReject("-0.5",     reportTestCases);
		nrOfFailedTestCases += CheckReject("0.001",    reportTestCases);
		nrOfFailedTestCases += CheckReject("1.5e-100", reportTestCases);
		// "3.0" has frac="0" and exp10=0 -> effective_exp = -1.  Reject as
		// non-integer per the strict policy (use "3" or "3e0" instead).
		nrOfFailedTestCases += CheckReject("3.0",      reportTestCases);
		nrOfFailedTestCases += CheckReject("10.50e0",  reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "fractional input rejected", "edecimal parse");
	}

	// ----- malformed input must be rejected -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckReject("",      reportTestCases);
		nrOfFailedTestCases += CheckReject("abc",   reportTestCases);
		nrOfFailedTestCases += CheckReject("1e",    reportTestCases);
		nrOfFailedTestCases += CheckReject(".",     reportTestCases);
		nrOfFailedTestCases += CheckReject("1.2.3", reportTestCases);
		nrOfFailedTestCases += CheckReject("1e3.5", reportTestCases);
		nrOfFailedTestCases += CheckReject("42x",   reportTestCases);
		nrOfFailedTestCases += CheckReject("0x1F",  reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "malformed reject", "edecimal parse");
	}

	// ----- negative-zero collapses to +0 across all input forms -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("-0",      "0", reportTestCases);
		nrOfFailedTestCases += CheckParse("-0e10",   "0", reportTestCases);
		nrOfFailedTestCases += CheckParse("-0.0e5",  "0", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "negative-zero collapse", "edecimal parse");
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		edecimal p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> failbit on bad", "edecimal parse");
	}

	// ----- operator>> succeeds on a good scientific token and consumes the value -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("  3.14e2  ");
		edecimal p;
		is >> p;
		if (is.fail() || p != edecimal(314)) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> scientific", "edecimal parse");
	}

	// ----- operator>> on empty stream sets failbit -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		edecimal p;
		is >> p;
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> empty stream", "edecimal parse");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
