// string_parse.cpp: tests for integer parse() migrated to string_parse primitives
//                  (Phase B1 of issue #835)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/utility/directives.hpp>
#include <universal/number/integer/integer.hpp>

namespace sw { namespace universal {

	// Tiny test bench: every check increments g_total and either passes silently
	// or prints a FAIL line and increments g_failures.
	static int g_failures = 0;
	static int g_total    = 0;

	template<unsigned nbits, typename Bt>
	static void check_parse(const char* tag, const std::string& input,
	                        const integer<nbits, Bt, IntegerNumberType::IntegerNumber>& expected) {
		++g_total;
		integer<nbits, Bt, IntegerNumberType::IntegerNumber> got;
		bool ok = parse(input, got);
		if (!ok || got != expected) {
			++g_failures;
			std::cout << "FAIL  " << tag << "  input=\"" << input << "\"  "
			          << "got=" << static_cast<long long>(got) << "  expected=" << static_cast<long long>(expected) << '\n';
		}
	}

	template<unsigned nbits, typename Bt>
	static void check_parse_invalid(const char* tag, const std::string& input) {
		++g_total;
		integer<nbits, Bt, IntegerNumberType::IntegerNumber> got;
		bool ok = parse(input, got);
		if (ok) {
			++g_failures;
			std::cout << "FAIL  " << tag << "  input=\"" << input << "\"  "
			             "parse should have rejected, got " << static_cast<long long>(got) << '\n';
		}
	}

}}  // namespace sw::universal

int main()
try {
	using namespace sw::universal;
	using I8  = integer< 8, std::uint8_t,  IntegerNumberType::IntegerNumber>;
	using I16 = integer<16, std::uint8_t,  IntegerNumberType::IntegerNumber>;
	using I32 = integer<32, std::uint32_t, IntegerNumberType::IntegerNumber>;
	using I64 = integer<64, std::uint32_t, IntegerNumberType::IntegerNumber>;

	std::cout << "integer parse() tests (Phase B1 of #835)\n\n";

	// ----- decimal -----
	check_parse<8, std::uint8_t>("dec 0",   "0",   I8(0));
	check_parse<8, std::uint8_t>("dec 5",   "5",   I8(5));
	check_parse<8, std::uint8_t>("dec 127", "127", I8(127));
	check_parse<8, std::uint8_t>("dec -1",  "-1",  I8(-1));
	check_parse<8, std::uint8_t>("dec -128","-128",I8(-128));
	check_parse<8, std::uint8_t>("dec +42", "+42", I8(42));
	check_parse<32, std::uint32_t>("dec 1_000_000", "1000000", I32(1000000));
	check_parse<64, std::uint32_t>("dec 64-bit",   "1234567890123", I64(1234567890123LL));

	// ----- binary -----
	check_parse<8, std::uint8_t>("bin 1010",  "0b1010",     I8(10));
	check_parse<8, std::uint8_t>("bin 0",     "0b0",        I8(0));
	check_parse<8, std::uint8_t>("bin FF",    "0b11111111", I8(-1));  // 2s-complement for nbits=8
	check_parse<32, std::uint32_t>("bin 32",  "0b10101010101010101010101010101010", I32(0xAAAAAAAA));
	check_parse<8, std::uint8_t>("bin negative", "-0b1010", I8(-10));
	check_parse<8, std::uint8_t>("bin 0B (uppercase)", "0B1010", I8(10));

	// ----- octal -----
	check_parse<16, std::uint8_t>("oct 17",   "0o17",   I16(15));
	check_parse<16, std::uint8_t>("oct 100",  "0o100",  I16(64));
	check_parse<8,  std::uint8_t>("oct 0",    "0o0",    I8(0));
	check_parse<8,  std::uint8_t>("oct 0O upper", "0O17", I8(15));

	// ----- hex -----
	check_parse<8,  std::uint8_t>("hex FF",  "0xFF", I8(-1));  // saturates as 2s-complement
	check_parse<16, std::uint8_t>("hex FF16","0xFF", integer<16, std::uint8_t, IntegerNumberType::IntegerNumber>(255));
	check_parse<32, std::uint32_t>("hex DEADBEEF", "0xDEADBEEF", I32(0xDEADBEEF));
	check_parse<32, std::uint32_t>("hex w/separator", "0xDEAD'BEEF", I32(0xDEADBEEF));
	check_parse<8,  std::uint8_t>("hex 0X (uppercase prefix)", "0X1F", I8(31));
	check_parse<8,  std::uint8_t>("hex mixed case digits", "0xaB", I8(0xAB));
	check_parse<8,  std::uint8_t>("hex negative", "-0x05", I8(-5));

	// ----- invalid -----
	check_parse_invalid<8, std::uint8_t>("empty",       "");
	check_parse_invalid<8, std::uint8_t>("just sign",   "-");
	check_parse_invalid<8, std::uint8_t>("dec w/letter","12a");
	check_parse_invalid<8, std::uint8_t>("bin w/2",     "0b102");
	check_parse_invalid<8, std::uint8_t>("oct w/8",     "0o18");
	check_parse_invalid<8, std::uint8_t>("hex w/z",     "0xZZ");
	check_parse_invalid<8, std::uint8_t>("0b no body",  "0b");
	check_parse_invalid<8, std::uint8_t>("0x no body",  "0x");
	// Hex with only separator chars (no real digits) must be rejected.
	check_parse_invalid<8,  std::uint8_t>("hex only-sep '",     "0x'");
	check_parse_invalid<8,  std::uint8_t>("hex only-sep ''",    "0x''");
	check_parse_invalid<32, std::uint32_t>("hex only-sep '''",  "0x'''");

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
