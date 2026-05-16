// test_string_parse.cpp: tests for the constexpr string-parsing primitives
//                       (issue #835 Phase A foundation)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <string_view>
#include <universal/utility/string_parse.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sp = sw::universal::string_parse;

// ============================================================================
// constexpr-correctness smoke tests: these have to compile, full stop.
// Compile-time failure aborts the build before main() is reached.
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "string_parse primitives (Phase A of #835) test suite";
	std::string test_tag    = "string_parse";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- scan_prefix -----
	{
		int start = nrOfFailedTestCases;
		// Lowercase prefixes
		{
			auto r = sp::scan_prefix("0b1010");
			if (r.base != sp::number_base::binary || r.body != "1010") ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_prefix("0o777");
			if (r.base != sp::number_base::octal || r.body != "777")   ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_prefix("0xDEADBEEF");
			if (r.base != sp::number_base::hex || r.body != "DEADBEEF") ++nrOfFailedTestCases;
		}
		// Uppercase prefixes
		{
			auto r = sp::scan_prefix("0B11");
			if (r.base != sp::number_base::binary) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_prefix("0O77");
			if (r.base != sp::number_base::octal)  ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_prefix("0X1F");
			if (r.base != sp::number_base::hex)    ++nrOfFailedTestCases;
		}
		// No prefix or bare leading zero -> decimal
		{
			auto r = sp::scan_prefix("123");
			if (r.base != sp::number_base::decimal || r.body != "123") ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_prefix("0");
			if (r.base != sp::number_base::decimal || r.body != "0")   ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_prefix("012");  // no C-style octal default
			if (r.base != sp::number_base::decimal) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_prefix("");
			if (r.base != sp::number_base::decimal || !r.body.empty()) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: scan_prefix\n";
	}

	// ----- scan_sign -----
	{
		int start = nrOfFailedTestCases;
		{
			auto r = sp::scan_sign("-42");
			if (!r.negative || r.rest != "42") ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_sign("+42");
			if (r.negative || r.rest != "42")  ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_sign("42");
			if (r.negative || r.rest != "42")  ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_sign("");
			if (r.negative || !r.rest.empty()) ++nrOfFailedTestCases;
		}
		{
			// Only first char consumed; "--7" -> negative, rest "-7"
			auto r = sp::scan_sign("--7");
			if (!r.negative || r.rest != "-7") ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: scan_sign\n";
	}

	// ----- parse_binary -----
	{
		int start = nrOfFailedTestCases;
		{
			auto r = sp::parse_binary("1010");
			if (r.value != 10u || r.digits != 4 || r.overflow || !r.valid) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::parse_binary("0");
			if (r.value != 0u || r.digits != 1 || !r.valid) ++nrOfFailedTestCases;
		}
		{
			// 64 ones = 2^64 - 1, no overflow
			auto r = sp::parse_binary("1111111111111111111111111111111111111111111111111111111111111111");
			if (r.value != 0xFFFFFFFFFFFFFFFFull || r.digits != 64 || r.overflow || !r.valid)
				++nrOfFailedTestCases;
		}
		{
			// 65 bits -> overflow
			auto r = sp::parse_binary("10000000000000000000000000000000000000000000000000000000000000000");
			if (!r.overflow || r.digits != 65) ++nrOfFailedTestCases;
		}
		{
			// Stops at invalid char
			auto r = sp::parse_binary("101z01");
			if (r.value != 5u || r.digits != 3 || r.valid) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::parse_binary("");
			if (r.valid || r.digits != 0) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: parse_binary\n";
	}

	// ----- parse_octal -----
	{
		int start = nrOfFailedTestCases;
		{
			auto r = sp::parse_octal("777");
			if (r.value != 0777u || !r.valid || r.digits != 3 || r.overflow) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::parse_octal("01234567");
			if (r.value != 01234567u || !r.valid || r.digits != 8 || r.overflow) ++nrOfFailedTestCases;
		}
		{
			// '8' is not octal -- stops at it
			auto r = sp::parse_octal("128");
			if (r.value != 012u || r.digits != 2 || r.valid) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::parse_octal("");
			if (r.valid) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: parse_octal\n";
	}

	// ----- parse_hex -----
	{
		int start = nrOfFailedTestCases;
		{
			auto r = sp::parse_hex("FF");
			if (r.value != 0xFFu || !r.valid || r.digits != 2) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::parse_hex("DEADBEEF");
			if (r.value != 0xDEADBEEFu || !r.valid || r.digits != 8) ++nrOfFailedTestCases;
		}
		{
			// Lowercase + uppercase mixed
			auto r = sp::parse_hex("DeAdC0De");
			if (r.value != 0xDEADC0DEu || !r.valid) ++nrOfFailedTestCases;
		}
		{
			// 16 hex digits = 64 bits, no overflow
			auto r = sp::parse_hex("FFFFFFFFFFFFFFFF");
			if (r.value != 0xFFFFFFFFFFFFFFFFull || !r.valid || r.overflow) ++nrOfFailedTestCases;
		}
		{
			// 17 hex digits -> overflow
			auto r = sp::parse_hex("10000000000000000");
			if (!r.overflow || r.digits != 17) ++nrOfFailedTestCases;
		}
		{
			// Stops at invalid
			auto r = sp::parse_hex("FF.AA");
			if (r.value != 0xFFu || r.digits != 2 || r.valid) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: parse_hex\n";
	}

	// ----- scan_decimal_float -----
	{
		int start = nrOfFailedTestCases;
		// Integer only
		{
			auto r = sp::scan_decimal_float("42");
			if (!r.valid || r.negative || r.int_part != "42" || !r.frac_part.empty() || r.exp10 != 0)
				++nrOfFailedTestCases;
		}
		// Negative integer
		{
			auto r = sp::scan_decimal_float("-7");
			if (!r.valid || !r.negative || r.int_part != "7") ++nrOfFailedTestCases;
		}
		// Explicit positive
		{
			auto r = sp::scan_decimal_float("+5");
			if (!r.valid || r.negative || r.int_part != "5") ++nrOfFailedTestCases;
		}
		// Integer + fraction
		{
			auto r = sp::scan_decimal_float("3.14");
			if (!r.valid || r.int_part != "3" || r.frac_part != "14" || r.exp10 != 0)
				++nrOfFailedTestCases;
		}
		// Fraction only
		{
			auto r = sp::scan_decimal_float(".5");
			if (!r.valid || !r.int_part.empty() || r.frac_part != "5") ++nrOfFailedTestCases;
		}
		// Integer only with trailing dot
		{
			auto r = sp::scan_decimal_float("5.");
			if (!r.valid || r.int_part != "5" || !r.frac_part.empty()) ++nrOfFailedTestCases;
		}
		// Exponent (positive)
		{
			auto r = sp::scan_decimal_float("1e10");
			if (!r.valid || r.int_part != "1" || r.exp10 != 10) ++nrOfFailedTestCases;
		}
		// Exponent (negative)
		{
			auto r = sp::scan_decimal_float("1E-5");
			if (!r.valid || r.exp10 != -5) ++nrOfFailedTestCases;
		}
		// All the things
		{
			auto r = sp::scan_decimal_float("-3.14159e+2");
			if (!r.valid || !r.negative || r.int_part != "3" || r.frac_part != "14159" || r.exp10 != 2)
				++nrOfFailedTestCases;
		}
		// Malformed
		{
			auto r = sp::scan_decimal_float(".");
			if (r.valid) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_decimal_float("");
			if (r.valid) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_decimal_float("3.14x");
			if (r.valid) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_decimal_float("1e");
			if (r.valid) ++nrOfFailedTestCases;
		}
		{
			auto r = sp::scan_decimal_float("1ex");
			if (r.valid) ++nrOfFailedTestCases;
		}
		// Long fraction (constexpr-friendly: views point into input)
		{
			auto r = sp::scan_decimal_float("3.14159265358979323846264338327950288419716939937510");
			if (!r.valid || r.frac_part.size() != 50) ++nrOfFailedTestCases;
		}
		// int32 exponent boundary (positive max) -- exp parser must accept INT32_MAX
		{
			auto r = sp::scan_decimal_float("1e2147483647");
			if (!r.valid || r.exp10 != 2147483647) ++nrOfFailedTestCases;
		}
		// int32 exponent boundary (negative min) -- exp parser must accept INT32_MIN.
		// Regression: an earlier version rejected this because the magnitude check
		// (v > INT32_MAX) fired before the sign was applied.
		{
			auto r = sp::scan_decimal_float("1e-2147483648");
			if (!r.valid || r.exp10 != (-2147483647 - 1)) ++nrOfFailedTestCases;
		}
		// One past int32 max on positive side -- must reject
		{
			auto r = sp::scan_decimal_float("1e2147483648");
			if (r.valid) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: scan_decimal_float\n";
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
