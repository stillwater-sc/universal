// string_parse.cpp: regression tests for string parsing of einteger
//                  (Phase E of #835 -- operator>> hygiene + #853 coverage)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// Issue #853 extended the parse path to all four bases: decimal, hex, binary,
// and octal, plus negative-sign and digit-group-separator handling. The
// operator>> hygiene (failbit, extraction guard, diagnostic) came in Phase E
// of #835; this file pins both behaviours.

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/einteger/einteger.hpp>
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

// helper: parse `input`, render the result via operator<< (decimal), and
// compare against `expected_decimal`. Returns 1 on failure, 0 on success.
template<typename BlockType>
int CheckParse(const char* input, const char* expected_decimal, bool reportTestCases) {
	using namespace sw::universal;
	einteger<BlockType> v;
	if (!parse(input, v)) {
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

template<typename BlockType>
int CheckReject(const char* input, bool reportTestCases) {
	using namespace sw::universal;
	einteger<BlockType> v;
	if (parse(input, v)) {
		if (reportTestCases) {
			std::ostringstream os; os << v;
			std::cout << "FAIL: '" << input << "' should be rejected, got " << os.str() << '\n';
		}
		return 1;
	}
	return 0;
}
}

int main()
try {
	using namespace sw::universal;
	using Int = einteger<std::uint32_t>;

	std::string test_suite  = "einteger string parse (issues #835 Phase E, #853)";
	bool reportTestCases    = true;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- decimal (canonical, signs, large) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0",     "0",     reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("42",    "42",    reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("+42",   "42",    reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-1000", "-1000", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>(
			"123456789012345678901234567890",
			"123456789012345678901234567890",
			reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>(
			"-123456789012345678901234567890",
			"-123456789012345678901234567890",
			reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "decimal parse", "einteger parse");
	}

	// ----- hexadecimal (with and without separators, signs, multi-byte) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0x0",            "0",          reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0xFF",           "255",        reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("+0xFF",          "255",        reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-0xFF",          "-255",       reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0xab",           "171",        reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0xABCD",         "43981",      reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0xDEAD'BEEF",    "3735928559", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>(
			"0xCAFE'BABE'DEAD'BEEF",
			"14627333968688430831",
			reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>(
			"-0xFFFF'FFFF'FFFF'FFFF",
			"-18446744073709551615",
			reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "hex parse", "einteger parse");
	}

	// ----- binary (with and without separators, signs, multi-byte) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0b0",        "0",   reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0b1010",     "10",  reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0b1010'1010","170", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-0b1010",    "-10", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>(
			"0b1111'1111'1111'1111'1111'1111'1111'1111",
			"4294967295",
			reportTestCases);
		// 33 bits: just past a single uint32 limb
		nrOfFailedTestCases += CheckParse<std::uint32_t>(
			"0b1'00000000'00000000'00000000'00000000",
			"4294967296",
			reportTestCases);
		// 64 bits
		nrOfFailedTestCases += CheckParse<std::uint32_t>(
			"0b11111111'11111111'11111111'11111111'11111111'11111111'11111111'11111111",
			"18446744073709551615",
			reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "binary parse", "einteger parse");
	}

	// ----- octal (C-style, no separators) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse<std::uint32_t>("010",          "8",          reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("017",          "15",         reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0777",         "511",        reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("01234567",     "342391",     reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("037777777777", "4294967295", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-017",         "-15",        reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "octal parse", "einteger parse");
	}

	// ----- malformed input must be rejected -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckReject<std::uint32_t>("",         reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("abc",      reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("0xZZ",     reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("0b102",    reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("0x",       reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("0b",       reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("1.5",      reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("12 34",    reportTestCases);
		// trailing garbage after an otherwise-valid prefix (std::regex_match
		// is whole-string, so the regexes reject these without a trailing
		// '$' anchor -- pin that behaviour with explicit tests).
		nrOfFailedTestCases += CheckReject<std::uint32_t>("42x",        reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("123abc",     reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("0xFF_extra", reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("0b1010X",    reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("0777!",      reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "malformed reject", "einteger parse");
	}

	// ----- cross-block-width parity: uint8_t, uint16_t, uint32_t all parse identically -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse<std::uint8_t >("0xCAFEBABE", "3405691582", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint16_t>("0xCAFEBABE", "3405691582", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0xCAFEBABE", "3405691582", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint8_t >("0b1111000011110000", "61680", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint16_t>("0b1111000011110000", "61680", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0b1111000011110000", "61680", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint8_t >("0777", "511", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint16_t>("0777", "511", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0777", "511", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "block-width parity", "einteger parse");
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		Int p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> failbit on bad", "einteger parse");
	}

	// ----- operator>> succeeds on a good token and consumes the value -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("  42  17");
		Int a, b;
		is >> a;
		is >> b;
		if (is.fail() || a != Int(42) || b != Int(17)) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> sequence", "einteger parse");
	}

	// ----- operator>> on empty stream sets failbit -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		Int p;
		is >> p;
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> empty stream", "einteger parse");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
