// string_parse.cpp: tests for fixpnt parse() (Phase B1 of issue #835)
//
// fixpnt::parse() was previously forward-declared but never defined; the
// istream operator>> linked to a non-existent symbol. This test covers the
// new implementation that uses the shared string_parse primitives.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixpnt<> parse() string-parsing test suite";
	std::string test_tag    = "fixpnt<> parse()";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Fixpnt8_4  = fixpnt<8,  4, Modulo, std::uint8_t>;
	using Fixpnt9_4  = fixpnt<9,  4, Modulo, std::uint8_t>;
	using Fixpnt16_8 = fixpnt<16, 8, Modulo, std::uint8_t>;

	// ----- decimal (integer-only in B1) -----
	{
		int start = nrOfFailedTestCases;
		Fixpnt8_4 a;
		if (!parse(std::string("0"),  a) || a != Fixpnt8_4(0))  ++nrOfFailedTestCases;
		if (!parse(std::string("5"),  a) || a != Fixpnt8_4(5))  ++nrOfFailedTestCases;
		if (!parse(std::string("-1"), a) || a != Fixpnt8_4(-1)) ++nrOfFailedTestCases;
		Fixpnt16_8 b;
		if (!parse(std::string("100"), b) || b != Fixpnt16_8(100)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal parsing\n";
	}

	// ----- binary (bit-pattern fills storage MSB-first) -----
	// fixpnt<8,4>(0b1010'0000) = 1010.0000 = value 10.0
	// fixpnt<8,4>(0b0000'1000) = 0000.1000 = value 0.5
	{
		int start = nrOfFailedTestCases;
		Fixpnt8_4 a, expected;
		expected.setbits(0b10100000);
		if (!parse(std::string("0b10100000"), a) || a != expected) ++nrOfFailedTestCases;
		expected.setbits(0b00001000);
		if (!parse(std::string("0b00001000"), a) || a != expected) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: binary parsing\n";
	}

	// ----- hex (bit-pattern fills storage; each nibble = 4 bits) -----
	{
		int start = nrOfFailedTestCases;
		Fixpnt8_4 a, expected;
		expected.setbits(0xA0);
		if (!parse(std::string("0xA0"), a) || a != expected) ++nrOfFailedTestCases;
		// Sign on hex bit-pattern: negate the parsed value
		Fixpnt8_4 expected_neg = -expected;
		if (!parse(std::string("-0xA0"), a) || a != expected_neg) ++nrOfFailedTestCases;
		// Apostrophe digit separator on a wider type
		Fixpnt16_8 b, expected16;
		expected16.setbits(0xDEAD);
		if (!parse(std::string("0xDE'AD"), b) || b != expected16) ++nrOfFailedTestCases;
		// fixpnt<16,8>(0xFF80) with Modulo arithmetic
		expected16.setbits(0xFF80);
		if (!parse(std::string("0xFF80"), b) || b != expected16) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hex parsing\n";
	}

	// ----- octal (bit-pattern; each digit = 3 bits, fixpnt<9,4> = 3 octal digits cleanly) -----
	{
		int start = nrOfFailedTestCases;
		Fixpnt9_4 a, expected;
		expected.setbits(0125);  // octal 125
		if (!parse(std::string("0o125"), a) || a != expected) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: octal parsing\n";
	}

	// ----- invalid inputs must be rejected -----
	{
		int start = nrOfFailedTestCases;
		Fixpnt8_4 a;
		if (parse(std::string(""),     a)) ++nrOfFailedTestCases;
		if (parse(std::string("-"),    a)) ++nrOfFailedTestCases;
		if (parse(std::string("12a"),  a)) ++nrOfFailedTestCases;
		if (parse(std::string("0b102"), a)) ++nrOfFailedTestCases;
		if (parse(std::string("0x"),   a)) ++nrOfFailedTestCases;
		if (parse(std::string("0x'"),  a)) ++nrOfFailedTestCases;
		if (parse(std::string("0x''"), a)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: invalid-input rejection\n";
	}

	// ----- commit-on-success: a failed parse must leave value untouched -----
	{
		int start = nrOfFailedTestCases;
		Fixpnt8_4 v(5);
		if (parse(std::string("0x1G"), v))   ++nrOfFailedTestCases;
		if (v != Fixpnt8_4(5))               ++nrOfFailedTestCases;

		Fixpnt16_8 w(-3);
		if (parse(std::string("12abc"), w))  ++nrOfFailedTestCases;
		if (w != Fixpnt16_8(-3))             ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: commit-on-success\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
