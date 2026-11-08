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

	// ----- hex leading zero digits leave no zero limbs (#1878) -----
	// setbyte wrote "0x0001" as limbs [1, 0] on uint8_t limbs: not == 1, not isone(),
	// and "-0x0000" a negative zero. The decimal print above does not see the extra limb.
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* text; long long value; };
		for (const Case& c : { Case{ "0x0001", 1 }, Case{ "-0x0001", -1 }, Case{ "0x0000", 0 }, Case{ "-0x0000", 0 },
		                       Case{ "0x0000'0000'0100", 256 }, Case{ "-0x00FF", -255 } }) {
			einteger<std::uint8_t> v;
			const einteger<std::uint8_t> ref(c.value);
			if (!parse(c.text, v) || v != ref || v.limbs() != ref.limbs() || v.isone() != (c.value == 1) || v.isneg() != (c.value < 0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL parse('" << c.text << "') has " << v.limbs() << " limbs, expected " << ref.limbs() << '\n';
			}
		}
		ReportTestResult(nrOfFailedTestCases - start, "hex leading zeros", "einteger parse");
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
		// A LEADING ZERO SELECTS OCTAL, however many of them there are. The pattern
		// used to require the second character to be [1-7], so the radix depended on
		// the number of leading zeros: "0777" was octal 511 but "00777" fell through
		// to decimal 777, and "075" was 61 while "0075" was 75. Zero-padding a value
		// silently changed what it meant (#1370).
		nrOfFailedTestCases += CheckParse<std::uint32_t>("00777",        "511",        reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0075",         "61",         reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("000075",       "61",         reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("00010",        "8",          reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-00017",       "-15",        reportTestCases);
		// "0" is zero in any radix
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0",            "0",          reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("00",           "0",          reportTestCases);
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
		// A leading zero commits the string to octal, so a digit outside [0-7] makes it
		// malformed -- exactly as C rejects 08 and 0749. These used to be re-read as
		// DECIMAL, which meant "0747" was 487 while "0749" was 749: the radix depended
		// on the digits (#1370).
		nrOfFailedTestCases += CheckReject<std::uint32_t>("08",         reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("09",         reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("0749",       reportTestCases);
		nrOfFailedTestCases += CheckReject<std::uint32_t>("-08",        reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "malformed reject", "einteger parse");
	}

	// ----- a radix prefix needs a digit: separators alone are not a number (#2010) -----
	// "0x'" and "0b''" parsed as 0, since [0-9a-fA-F']+ is satisfied by a separator. The digits
	// after "0b" and "0x" must now include one digit; a separator may still sit anywhere among them.
	// Octal and decimal never took a separator, so "0'" and "''" were already rejected.
	{
		int start = nrOfFailedTestCases;
		for (const char* bad : { "0x'", "0X'", "0b'", "0x''", "0b'''", "-0x'", "+0b'", "-0X''",
		                         "0x", "0b", "-0x", "+0b", "0'", "-0'", "''", "'", "0''", "00'", "'0", "'0x1" }) {
			nrOfFailedTestCases += CheckReject<std::uint32_t>(bad, reportTestCases);
			nrOfFailedTestCases += CheckReject<std::uint8_t >(bad, reportTestCases);
			Int v(-12345);
			if (parse(bad, v) || v != Int(-12345)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: parse('" << bad << "') changed the destination to " << v << '\n';
			}
		}
		// one digit is enough, wherever the separators are
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0x'0",     "0",    reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0x0'",     "0",    reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0x'f'",    "15",   reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-0X''1",   "-1",   reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0b'1",     "1",    reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("0b1''0'",  "2",    reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint8_t >("-0b'0'0",  "0",    reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint8_t >("0x'1'00",  "256",  reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "prefix without digits", "einteger parse");
	}

	// ----- binary, octal and hex agree on long inputs, across every limb width (#2010) -----
	// binary and octal now share the byte-filling path of hex: a value spanning many limbs, with
	// octal digits straddling byte boundaries, must come out the same in all three radixes
	{
		int start = nrOfFailedTestCases;
		std::string bin = "0b", oct = "0", hex = "0x";
		const char* hexdigits = "0123456789abcdef";
		// 3 * 4 * 50 bits, so all three spell the same value without padding
		std::string bits;
		for (int i = 0; i < 600; ++i) bits += ((i * 7 + 3) % 5 < 2) ? '1' : '0';
		bits[0] = '1';
		bin += bits;
		for (size_t i = 0; i < bits.size(); i += 3) oct += static_cast<char>('0' + (bits[i] - '0') * 4 + (bits[i + 1] - '0') * 2 + (bits[i + 2] - '0'));
		for (size_t i = 0; i < bits.size(); i += 4) hex += hexdigits[(bits[i] - '0') * 8 + (bits[i + 1] - '0') * 4 + (bits[i + 2] - '0') * 2 + (bits[i + 3] - '0')];
		auto agree = [&](auto zero) {
			using E = decltype(zero);
			E b, o, h;
			if (!parse(bin, b) || !parse(oct, o) || !parse(hex, h) || b != o || b != h || b.limbs() != h.limbs()) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: binary, octal and hex parse of a 600-bit value differ\n";
			}
			E ref(1), top(1);   // the leading bit, as a check that the 600 bits all landed
			ref <<= 599;
			top <<= 600;
			if (b < ref || b >= top) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: the 600-bit value is out of [2^599, 2^600)\n";
			}
		};
		agree(einteger<std::uint8_t>{});
		agree(einteger<std::uint16_t>{});
		agree(einteger<std::uint32_t>{});
		nrOfFailedTestCases += CheckParse<std::uint8_t >("01234567", "342391",  reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint16_t>("0777777777777777777777", "9223372036854775807", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint8_t >("-0b1'0000'0000'0000'0001", "-65537", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "binary octal hex agree", "einteger parse");
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

	// ----- one optional sign, in every radix (#1988) -----
	// A run of signs used to be accepted and read differently per radix: decimal "-+5" was 5 and
	// "--5" -5, octal "-+05" was -5, hex looked only at the sign next to "0x".
	{
		int start = nrOfFailedTestCases;
		for (const char* bad : { "--5", "-+5", "+-5", "++5", "--05", "-+05", "--0x5", "-+0x5", "+-0x5",
		                         "--0b101", "-+0b101", "-", "+", "--", "-+0", "+-0" }) {
			nrOfFailedTestCases += CheckReject<std::uint32_t>(bad, reportTestCases);
			nrOfFailedTestCases += CheckReject<std::uint8_t >(bad, reportTestCases);
		}
		nrOfFailedTestCases += CheckParse<std::uint32_t>("+5",      "5",   reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-5",      "-5",  reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-017",    "-15", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("+017",    "15",  reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-0x1F",   "-31", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("+0x1F",   "31",  reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("-0b101",  "-5",  reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("+0b101",  "5",   reportTestCases);
		// a negative zero in any radix is +0
		for (const char* z : { "-0", "-00", "-0x0", "-0x000", "-0b0", "-0b'0'0" }) {
			Int v;
			if (!parse(z, v) || !v.iszero() || v.isneg()) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: '" << z << "' is not +0\n";
			}
		}
		ReportTestResult(nrOfFailedTestCases - start, "single sign", "einteger parse");
	}

	// ----- a failed parse leaves the destination unchanged (#1988) -----
	{
		int start = nrOfFailedTestCases;
		for (const char* bad : { "", "abc", "--5", "0x", "08", "12 34" }) {
			Int v(-12345);
			if (parse(bad, v) || v != Int(-12345)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: parse('" << bad << "') changed the destination to " << v << '\n';
			}
		}
		ReportTestResult(nrOfFailedTestCases - start, "failure leaves value", "einteger parse");
	}

	// ----- long decimal strings round-trip through print, at chunk boundaries (#1988) -----
	{
		int start = nrOfFailedTestCases;
		for (size_t n : { 1u, 8u, 9u, 10u, 17u, 18u, 19u, 27u, 100u, 1000u, 5001u }) {
			for (const char* sign : { "", "-" }) {
				std::string digits(n, '0');
				for (size_t i = 0; i < n; ++i) digits[i] = static_cast<char>('0' + (i * 7 + 3) % 10);
				digits[0] = '9';
				const std::string s = std::string(sign) + digits;
				nrOfFailedTestCases += CheckParse<std::uint32_t>(s.c_str(), s.c_str(), reportTestCases);
				nrOfFailedTestCases += CheckParse<std::uint8_t >(s.c_str(), s.c_str(), reportTestCases);
			}
		}
		// zeros inside and at the end of a nine-digit chunk
		nrOfFailedTestCases += CheckParse<std::uint32_t>("1000000000000000000", "1000000000000000000", reportTestCases);
		nrOfFailedTestCases += CheckParse<std::uint32_t>("1000000001000000001", "1000000001000000001", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "long decimal round trip", "einteger parse");
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
