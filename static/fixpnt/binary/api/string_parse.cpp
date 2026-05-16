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

#include <iostream>
#include <string>
#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>

namespace sw { namespace universal {

	static int g_failures = 0;
	static int g_total    = 0;

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	static void check_parse(const char* tag, const std::string& input,
	                        const fixpnt<nbits, rbits, arithmetic, bt>& expected) {
		++g_total;
		fixpnt<nbits, rbits, arithmetic, bt> got;
		bool ok = parse(input, got);
		if (!ok || got != expected) {
			++g_failures;
			std::cout << "FAIL  " << tag << "  input=\"" << input << "\"  "
			          << "got=" << double(got) << "  expected=" << double(expected) << '\n';
		}
	}

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	static void check_parse_invalid(const char* tag, const std::string& input) {
		++g_total;
		fixpnt<nbits, rbits, arithmetic, bt> got;
		bool ok = parse(input, got);
		if (ok) {
			++g_failures;
			std::cout << "FAIL  " << tag << "  input=\"" << input << "\"  "
			             "parse should have rejected, got " << double(got) << '\n';
		}
	}

}}  // namespace sw::universal

int main()
try {
	using namespace sw::universal;
	using FP8_4  = fixpnt<8,  4, Modulo, std::uint8_t>;
	using FP16_8 = fixpnt<16, 8, Modulo, std::uint8_t>;

	std::cout << "fixpnt parse() tests (Phase B1 of #835)\n\n";

	// ----- decimal (integer-only in B1) -----
	check_parse<8, 4, Modulo, std::uint8_t>("dec 0", "0", FP8_4(0));
	check_parse<8, 4, Modulo, std::uint8_t>("dec 5", "5", FP8_4(5));
	check_parse<8, 4, Modulo, std::uint8_t>("dec -1", "-1", FP8_4(-1));
	check_parse<16, 8, Modulo, std::uint8_t>("dec 100 (rbits=8)", "100", FP16_8(100));

	// ----- binary (bit-pattern fills storage MSB-first) -----
	// fixpnt<8,4>(0b1010'0000) = 1010.0000 = value 10.0
	{
		FP8_4 expected;
		expected.setbits(0b10100000);
		check_parse<8, 4, Modulo, std::uint8_t>("bin 10100000 -> 10.0", "0b10100000", expected);
	}
	// fixpnt<8,4>(0b0000'1000) = 0000.1000 = value 0.5
	{
		FP8_4 expected;
		expected.setbits(0b00001000);
		check_parse<8, 4, Modulo, std::uint8_t>("bin 00001000 -> 0.5", "0b00001000", expected);
	}

	// ----- hex (bit-pattern fills storage; each nibble = 4 bits) -----
	// fixpnt<8,4>(0xA0) = 1010.0000 = 10.0
	{
		FP8_4 expected;
		expected.setbits(0xA0);
		check_parse<8, 4, Modulo, std::uint8_t>("hex A0 -> 10.0", "0xA0", expected);
	}
	// fixpnt<16,8>(0xFF80) -> -0.5 (signed) or 65408/256 = 255.5 (modulo); we use Modulo
	{
		FP16_8 expected;
		expected.setbits(0xFF80);
		check_parse<16, 8, Modulo, std::uint8_t>("hex FF80", "0xFF80", expected);
	}
	// Apostrophe digit separator
	{
		FP16_8 expected;
		expected.setbits(0xDEAD);
		check_parse<16, 8, Modulo, std::uint8_t>("hex w/separator", "0xDE'AD", expected);
	}

	// ----- octal (bit-pattern; each digit = 3 bits) -----
	// fixpnt<8,4>(0o250) = 010 101 000 truncated to 8 bits MSB = ... hmm tricky
	// Use a width that's a multiple of 3 for clean semantics. fixpnt<9,4> -> 9 bits = 3 octal digits.
	{
		fixpnt<9, 4, Modulo, std::uint8_t> expected;
		expected.setbits(0125);  // octal 125
		check_parse<9, 4, Modulo, std::uint8_t>("oct 125", "0o125", expected);
	}

	// ----- sign on bit-pattern -----
	{
		FP8_4 expected;
		expected.setbits(0xA0);
		FP8_4 expected_neg = -expected;
		check_parse<8, 4, Modulo, std::uint8_t>("hex w/leading -", "-0xA0", expected_neg);
	}

	// ----- invalid -----
	check_parse_invalid<8, 4, Modulo, std::uint8_t>("empty", "");
	check_parse_invalid<8, 4, Modulo, std::uint8_t>("just sign", "-");
	check_parse_invalid<8, 4, Modulo, std::uint8_t>("dec w/letter", "12a");
	check_parse_invalid<8, 4, Modulo, std::uint8_t>("bin w/2", "0b102");
	check_parse_invalid<8, 4, Modulo, std::uint8_t>("hex no body", "0x");
	check_parse_invalid<8, 4, Modulo, std::uint8_t>("hex only-sep '",  "0x'");
	check_parse_invalid<8, 4, Modulo, std::uint8_t>("hex only-sep ''", "0x''");

	std::cout << "\nResults: " << (sw::universal::g_total - sw::universal::g_failures)
	          << " / " << sw::universal::g_total << " tests passed";
	if (sw::universal::g_failures > 0) {
		std::cout << "  (" << sw::universal::g_failures << " FAILED)\n";
		return EXIT_FAILURE;
	}
	std::cout << "\n";
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << err.what() << '\n';
	return EXIT_FAILURE;
}
