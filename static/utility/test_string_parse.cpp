// test_string_parse.cpp: tests for the constexpr string-parsing primitives
//                       (issue #835 Phase A foundation)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <string_view>
#include <universal/utility/string_parse.hpp>

namespace sp = sw::universal::string_parse;

// ============================================================================
// Tiny test bench (no dependency on the regression framework -- this header
// is a leaf utility and its tests should compile standalone).
// ============================================================================

static int g_failures = 0;
static int g_total    = 0;

static void report(bool condition, const char* what) {
	++g_total;
	if (!condition) {
		++g_failures;
		std::cout << "FAIL  " << what << '\n';
	}
}

// ============================================================================
// constexpr-correctness smoke tests: these have to compile, full stop.
// ============================================================================

namespace constexpr_tests {

constexpr auto pfx_0b = sp::scan_prefix("0b1010");
static_assert(pfx_0b.base == sp::number_base::binary, "0b prefix should be binary");

constexpr auto pfx_dec = sp::scan_prefix("123");
static_assert(pfx_dec.base == sp::number_base::decimal, "no prefix should be decimal");

constexpr auto sg_neg = sp::scan_sign("-42");
static_assert(sg_neg.negative,            "minus should be detected");
static_assert(sg_neg.rest == "42",        "scan_sign should advance past '-'");

constexpr auto bin = sp::parse_binary("1010");
static_assert(bin.value == 10u,           "binary 1010 = 10");
static_assert(bin.digits == 4u,           "consumed 4 digits");
static_assert(!bin.overflow,              "no overflow");
static_assert(bin.valid,                  "fully valid");

constexpr auto hex = sp::parse_hex("FF");
static_assert(hex.value == 255u,          "hex FF = 255");
static_assert(hex.digits == 2u,           "consumed 2 digits");
static_assert(hex.valid,                  "fully valid");

constexpr auto df = sp::scan_decimal_float("-3.14e2");
static_assert(df.valid,                       "-3.14e2 parses");
static_assert(df.negative,                    "negative detected");
static_assert(df.int_part == "3",             "int part is \"3\"");
static_assert(df.frac_part == "14",           "frac part is \"14\"");
static_assert(df.exp10 == 2,                  "exponent is 2");

}  // namespace constexpr_tests

// ============================================================================
// scan_prefix
// ============================================================================

static void test_scan_prefix() {
	// Lowercase prefixes
	{
		auto r = sp::scan_prefix("0b1010");
		report(r.base == sp::number_base::binary,  "scan_prefix 0b -> binary");
		report(r.body == "1010",                    "scan_prefix 0b body");
	}
	{
		auto r = sp::scan_prefix("0o777");
		report(r.base == sp::number_base::octal,   "scan_prefix 0o -> octal");
		report(r.body == "777",                     "scan_prefix 0o body");
	}
	{
		auto r = sp::scan_prefix("0xDEADBEEF");
		report(r.base == sp::number_base::hex,     "scan_prefix 0x -> hex");
		report(r.body == "DEADBEEF",                "scan_prefix 0x body");
	}
	// Uppercase prefixes
	{
		auto r = sp::scan_prefix("0B11");
		report(r.base == sp::number_base::binary,  "scan_prefix 0B -> binary");
	}
	{
		auto r = sp::scan_prefix("0O77");
		report(r.base == sp::number_base::octal,   "scan_prefix 0O -> octal");
	}
	{
		auto r = sp::scan_prefix("0X1F");
		report(r.base == sp::number_base::hex,     "scan_prefix 0X -> hex");
	}
	// No prefix -> decimal
	{
		auto r = sp::scan_prefix("123");
		report(r.base == sp::number_base::decimal, "scan_prefix bare -> decimal");
		report(r.body == "123",                     "scan_prefix bare body unchanged");
	}
	// Leading '0' without prefix char -> decimal (just "0" or "012")
	{
		auto r = sp::scan_prefix("0");
		report(r.base == sp::number_base::decimal, "scan_prefix \"0\" -> decimal");
		report(r.body == "0",                       "scan_prefix \"0\" body unchanged");
	}
	{
		auto r = sp::scan_prefix("012");
		report(r.base == sp::number_base::decimal, "scan_prefix \"012\" -> decimal (no octal default)");
	}
	// Empty
	{
		auto r = sp::scan_prefix("");
		report(r.base == sp::number_base::decimal, "scan_prefix empty -> decimal");
		report(r.body.empty(),                       "scan_prefix empty body empty");
	}
}

// ============================================================================
// scan_sign
// ============================================================================

static void test_scan_sign() {
	{
		auto r = sp::scan_sign("-42");
		report(r.negative,           "scan_sign '-' negative");
		report(r.rest == "42",       "scan_sign '-' rest");
	}
	{
		auto r = sp::scan_sign("+42");
		report(!r.negative,          "scan_sign '+' not negative");
		report(r.rest == "42",       "scan_sign '+' rest");
	}
	{
		auto r = sp::scan_sign("42");
		report(!r.negative,          "scan_sign no sign positive");
		report(r.rest == "42",       "scan_sign no sign rest unchanged");
	}
	{
		auto r = sp::scan_sign("");
		report(!r.negative,          "scan_sign empty positive");
		report(r.rest.empty(),       "scan_sign empty rest");
	}
	{
		// Only first character consumed; "--7" -> negative, rest "-7"
		auto r = sp::scan_sign("--7");
		report(r.negative,           "scan_sign '--' first char only");
		report(r.rest == "-7",       "scan_sign '--' rest is \"-7\"");
	}
}

// ============================================================================
// parse_binary / parse_octal / parse_hex
// ============================================================================

static void test_parse_binary() {
	{
		auto r = sp::parse_binary("1010");
		report(r.value == 10u && r.digits == 4 && !r.overflow && r.valid,
		       "parse_binary 1010");
	}
	{
		auto r = sp::parse_binary("0");
		report(r.value == 0u && r.digits == 1 && r.valid, "parse_binary 0");
	}
	{
		// 64 ones = 2^64 - 1, no overflow
		auto r = sp::parse_binary("1111111111111111111111111111111111111111111111111111111111111111");
		report(r.value == 0xFFFFFFFFFFFFFFFFull && r.digits == 64 && !r.overflow && r.valid,
		       "parse_binary 64 ones");
	}
	{
		// 65 bits => overflow
		auto r = sp::parse_binary("10000000000000000000000000000000000000000000000000000000000000000");
		report(r.overflow && r.digits == 65, "parse_binary 65-bit overflow");
	}
	{
		// Stops at invalid char
		auto r = sp::parse_binary("101z01");
		report(r.value == 5u && r.digits == 3 && !r.valid, "parse_binary stops at 'z'");
	}
	{
		auto r = sp::parse_binary("");
		report(!r.valid && r.digits == 0, "parse_binary empty -> invalid");
	}
}

static void test_parse_octal() {
	{
		auto r = sp::parse_octal("777");
		report(r.value == 0777u && r.valid && r.digits == 3 && !r.overflow,
		       "parse_octal 777");
	}
	{
		auto r = sp::parse_octal("01234567");
		report(r.value == 01234567u && r.valid && r.digits == 8 && !r.overflow,
		       "parse_octal 01234567");
	}
	{
		// '8' is not octal
		auto r = sp::parse_octal("128");
		report(r.value == 012u && r.digits == 2 && !r.valid, "parse_octal stops at '8'");
	}
	{
		auto r = sp::parse_octal("");
		report(!r.valid, "parse_octal empty -> invalid");
	}
}

static void test_parse_hex() {
	{
		auto r = sp::parse_hex("FF");
		report(r.value == 0xFFu && r.valid && r.digits == 2, "parse_hex FF");
	}
	{
		auto r = sp::parse_hex("DEADBEEF");
		report(r.value == 0xDEADBEEFu && r.valid && r.digits == 8, "parse_hex DEADBEEF");
	}
	{
		// Lowercase and uppercase mixed
		auto r = sp::parse_hex("DeAdC0De");
		report(r.value == 0xDEADC0DEu && r.valid, "parse_hex mixed case");
	}
	{
		// 16 hex digits = 64 bits, no overflow
		auto r = sp::parse_hex("FFFFFFFFFFFFFFFF");
		report(r.value == 0xFFFFFFFFFFFFFFFFull && r.valid && !r.overflow,
		       "parse_hex 16 F's");
	}
	{
		// 17 hex digits => overflow
		auto r = sp::parse_hex("10000000000000000");
		report(r.overflow && r.digits == 17, "parse_hex 17-digit overflow");
	}
	{
		// Stops at invalid
		auto r = sp::parse_hex("FF.AA");
		report(r.value == 0xFFu && r.digits == 2 && !r.valid, "parse_hex stops at '.'");
	}
}

// ============================================================================
// scan_decimal_float
// ============================================================================

static void test_scan_decimal_float() {
	// Integer only
	{
		auto r = sp::scan_decimal_float("42");
		report(r.valid && !r.negative && r.int_part == "42" && r.frac_part.empty() && r.exp10 == 0,
		       "scan_decimal_float \"42\"");
	}
	// Negative integer
	{
		auto r = sp::scan_decimal_float("-7");
		report(r.valid && r.negative && r.int_part == "7", "scan_decimal_float \"-7\"");
	}
	// Explicit positive
	{
		auto r = sp::scan_decimal_float("+5");
		report(r.valid && !r.negative && r.int_part == "5", "scan_decimal_float \"+5\"");
	}
	// Integer + fraction
	{
		auto r = sp::scan_decimal_float("3.14");
		report(r.valid && r.int_part == "3" && r.frac_part == "14" && r.exp10 == 0,
		       "scan_decimal_float \"3.14\"");
	}
	// Fraction only
	{
		auto r = sp::scan_decimal_float(".5");
		report(r.valid && r.int_part.empty() && r.frac_part == "5",
		       "scan_decimal_float \".5\"");
	}
	// Integer only with trailing dot
	{
		auto r = sp::scan_decimal_float("5.");
		report(r.valid && r.int_part == "5" && r.frac_part.empty(),
		       "scan_decimal_float \"5.\"");
	}
	// Exponent
	{
		auto r = sp::scan_decimal_float("1e10");
		report(r.valid && r.int_part == "1" && r.exp10 == 10, "scan_decimal_float \"1e10\"");
	}
	{
		auto r = sp::scan_decimal_float("1E-5");
		report(r.valid && r.exp10 == -5, "scan_decimal_float \"1E-5\"");
	}
	// All the things
	{
		auto r = sp::scan_decimal_float("-3.14159e+2");
		report(r.valid && r.negative && r.int_part == "3" && r.frac_part == "14159" && r.exp10 == 2,
		       "scan_decimal_float \"-3.14159e+2\"");
	}
	// Just a dot
	{
		auto r = sp::scan_decimal_float(".");
		report(!r.valid, "scan_decimal_float \".\" should fail");
	}
	// Empty
	{
		auto r = sp::scan_decimal_float("");
		report(!r.valid, "scan_decimal_float empty should fail");
	}
	// Trailing garbage
	{
		auto r = sp::scan_decimal_float("3.14x");
		report(!r.valid, "scan_decimal_float \"3.14x\" should reject trailing garbage");
	}
	// Bad exponent
	{
		auto r = sp::scan_decimal_float("1e");
		report(!r.valid, "scan_decimal_float \"1e\" should fail (no exp digits)");
	}
	{
		auto r = sp::scan_decimal_float("1ex");
		report(!r.valid, "scan_decimal_float \"1ex\" should fail");
	}
	// Long fraction (constexpr-friendly: views point into input)
	{
		auto r = sp::scan_decimal_float("3.14159265358979323846264338327950288419716939937510");
		report(r.valid && r.frac_part.size() == 50,
		       "scan_decimal_float 50-digit fraction");
	}
}

int main()
try {
	std::cout << "string_parse primitives (Phase A of #835): test suite\n\n";

	test_scan_prefix();
	test_scan_sign();
	test_parse_binary();
	test_parse_octal();
	test_parse_hex();
	test_scan_decimal_float();

	std::cout << "\nResults: " << (g_total - g_failures) << " / " << g_total << " tests passed";
	if (g_failures > 0) {
		std::cout << "  (" << g_failures << " FAILED)\n";
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
