// teju.cpp: test suite for Teju Jagua decimal string conversion via blocktriple
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstring>
#include <cmath>
#include <random>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/support/teju_formatter.hpp>
#include <universal/verification/test_suite.hpp>

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

namespace sw { namespace universal {

// Test the Teju Jagua core algorithm directly on native IEEE float.
// Uses teju::float_to_decimal to bypass blocktriple entirely.
unsigned test_teju_core_float(bool reportTestCases) {
	unsigned nrOfFailedTests = 0;

	struct { float val; uint64_t expected_mantissa; int expected_exponent; } cases[] = {
		{ 1.0f,    1, 0 },
		{ 10.0f,   1, 1 },
		{ 100.0f,  1, 2 },
		{ 0.1f,    1, -1 },
		{ 0.5f,    5, -1 },
		{ 3.14f,   314, -2 },
	};

	for (const auto& tc : cases) {
		teju::decimal_fp result = teju::float_to_decimal(tc.val);
		if (result.mantissa != tc.expected_mantissa || result.exponent != tc.expected_exponent) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: teju::float_to_decimal(" << tc.val << ")"
					<< " expected {" << tc.expected_mantissa << ", " << tc.expected_exponent << "}"
					<< " got {" << result.mantissa << ", " << result.exponent << "}\n";
			}
		}
	}

	return nrOfFailedTests;
}

// Test that Teju Jagua produces round-trip safe output for native IEEE float values.
// Uses teju::float_to_decimal directly (bypasses blocktriple) to verify
// the core algorithm produces shortest round-trip-safe representations.
unsigned test_teju_float_roundtrip(bool reportTestCases) {
	unsigned nrOfFailedTests = 0;

	float test_values[] = {
		1.0f, 0.1f, 0.5f, 3.14f, 100.0f, 0.001f,
		1.17549435e-38f,  // float min normal
		3.40282347e+38f,  // float max
		1.0e10f, 1.0e-10f,
		1234567.0f, 0.123456789f,
		2.0f, 4.0f, 8.0f, 16.0f,
		0.25f, 0.125f, 0.0625f,
		9.99999f, 10.0f, 10.1f,
	};

	for (float fv : test_values) {
		teju::decimal_fp dec = teju::float_to_decimal(fv);

		// Reconstruct the string: mantissa * 10^exponent
		std::ostringstream oss;
		oss << dec.mantissa << "e" << dec.exponent;
		std::string dec_str = oss.str();

		float parsed = std::strtof(dec_str.c_str(), nullptr);

		uint32_t orig_bits, parsed_bits;
		std::memcpy(&orig_bits, &fv, sizeof(float));
		std::memcpy(&parsed_bits, &parsed, sizeof(float));

		if (orig_bits != parsed_bits) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: float roundtrip for " << fv
					<< " -> dec: {" << dec.mantissa << ", " << dec.exponent << "}"
					<< " -> str: \"" << dec_str << "\""
					<< " -> parsed: " << parsed
					<< " (bits: " << std::hex << orig_bits << " vs " << parsed_bits << std::dec << ")\n";
			}
		}
	}

	return nrOfFailedTests;
}

// Test that Teju Jagua produces round-trip safe output for native IEEE double values.
unsigned test_teju_double_roundtrip(bool reportTestCases) {
	unsigned nrOfFailedTests = 0;

	double test_values[] = {
		1.0, 0.1, 0.5, 3.141592653589793, 100.0, 0.001,
		2.2250738585072014e-308,   // double min normal
		1.7976931348623157e+308,   // double max
		1.0e100, 1.0e-100,
		1234567890.0, 0.123456789,
		2.0, 4.0, 8.0, 16.0,
		0.25, 0.125, 0.0625,
		9.999999999999999, 10.0, 10.1,
	};

	for (double dv : test_values) {
		teju::decimal_fp dec = teju::double_to_decimal(dv);

		std::ostringstream oss;
		oss << dec.mantissa << "e" << dec.exponent;
		std::string dec_str = oss.str();

		double parsed = std::strtod(dec_str.c_str(), nullptr);

		uint64_t orig_bits, parsed_bits;
		std::memcpy(&orig_bits, &dv, sizeof(double));
		std::memcpy(&parsed_bits, &parsed, sizeof(double));

		if (orig_bits != parsed_bits) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: double roundtrip for " << dv
					<< " -> dec: {" << dec.mantissa << ", " << dec.exponent << "}"
					<< " -> str: \"" << dec_str << "\""
					<< " -> parsed: " << parsed
					<< " (bits: " << std::hex << orig_bits << " vs " << parsed_bits << std::dec << ")\n";
			}
		}
	}

	return nrOfFailedTests;
}

// Test that teju_to_string matches blocktriple::to_string for the same value.
// blocktriple<23, REP> does not perfectly preserve IEEE float values (the round()
// function shifts the mantissa), but both formatters should agree on the
// blocktriple's actual stored value.
unsigned test_teju_blocktriple_consistency(bool reportTestCases) {
	unsigned nrOfFailedTests = 0;

	float test_values[] = {
		1.0f, 0.1f, 0.5f, 3.14f, 100.0f, 0.001f,
		1.17549435e-38f, 3.40282347e+38f,
		1.0e10f, 1.0e-10f,
		2.0f, 4.0f, 8.0f, 16.0f,
		0.25f, 0.125f, 0.0625f,
		-1.0f, -3.14f, -0.001f,
	};

	for (float fv : test_values) {
		blocktriple<23, BlockTripleOperator::REP, uint32_t> bt(fv);

		// Compare teju_to_string against bt.to_string with the same formatting flags
		// Use scientific notation, precision 6
		std::string teju_str = teju_to_string(bt, 6, 0, false, true, false, false, false, false, ' ');
		std::string bt_str = bt.to_string(6, 0, false, true, false, false, false, false, ' ');

		// Compare numerically: allow small relative difference since the two
		// formatters may round the last displayed digit differently.
		double teju_val = std::strtod(teju_str.c_str(), nullptr);
		double bt_val = std::strtod(bt_str.c_str(), nullptr);
		double rel_err = (bt_val == 0.0) ? std::fabs(teju_val) :
			std::fabs((teju_val - bt_val) / bt_val);

		if (rel_err > 1e-6) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: blocktriple consistency for " << fv
					<< " bt.to_string: \"" << bt_str << "\""
					<< " teju_to_string: \"" << teju_str << "\""
					<< " rel_err: " << rel_err << "\n";
			}
		}
	}

	return nrOfFailedTests;
}

// Test formatting flags using teju_to_string on blocktriple values
unsigned test_teju_formatting(bool reportTestCases) {
	unsigned nrOfFailedTests = 0;

	blocktriple<23, BlockTripleOperator::REP, uint32_t> bt(3.14f);
	// Note: blocktriple<23, REP>(3.14f) stores ~1.57 due to round() shift

	// Scientific notation: should produce valid scientific format
	{
		std::string s = teju_to_string(bt, 2, 0, false, true, false, false, false, false, ' ');
		// Must contain 'e' and a decimal point
		if (s.find('e') == std::string::npos || s.find('.') == std::string::npos) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: scientific format: " << s << "\n";
		}
	}

	// Fixed notation
	{
		std::string s = teju_to_string(bt, 4, 0, true, false, false, false, false, false, ' ');
		// Must contain a decimal point and no 'e'
		if (s.find('.') == std::string::npos || s.find('e') != std::string::npos) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: fixed format: " << s << "\n";
		}
	}

	// showpos
	{
		std::string s = teju_to_string(bt, 2, 0, false, true, false, false, true, false, ' ');
		if (s[0] != '+') {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: showpos: " << s << "\n";
		}
	}

	// uppercase
	{
		std::string s = teju_to_string(bt, 2, 0, false, true, false, false, false, true, ' ');
		if (s.find('E') == std::string::npos) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: uppercase: " << s << "\n";
		}
	}

	// Width and right alignment
	{
		std::string s = teju_to_string(bt, 2, 15, false, true, false, false, false, false, ' ');
		if (s.length() < 15) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: width=15: length=" << s.length() << " \"" << s << "\"\n";
		}
	}

	// Zero
	{
		blocktriple<23, BlockTripleOperator::REP, uint32_t> zero(0.0f);
		std::string s = teju_to_string(zero, 3, 0, false, true, false, false, false, false, ' ');
		if (s != "0.000e+00") {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: zero: \"" << s << "\" expected \"0.000e+00\"\n";
		}
	}

	// Negative value
	{
		blocktriple<23, BlockTripleOperator::REP, uint32_t> neg(-2.5f);
		std::string s = teju_to_string(neg, 2, 0, false, true, false, false, false, false, ' ');
		if (s[0] != '-') {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: negative: \"" << s << "\"\n";
		}
	}

	return nrOfFailedTests;
}

// Test with random float values via teju::float_to_decimal (IEEE roundtrip)
unsigned test_teju_random_float(unsigned count, bool reportTestCases) {
	unsigned nrOfFailedTests = 0;
	std::mt19937 rng(42);

	for (unsigned i = 0; i < count; ++i) {
		uint32_t bits = rng();
		// Mask out NaN, infinity, subnormals, and sign (float_to_decimal works on magnitude)
		bits &= 0x7FFFFFFFu; // clear sign bit
		uint32_t exp_bits = (bits >> 23) & 0xFF;
		if (exp_bits == 0xFF) continue; // skip NaN/Inf
		if (exp_bits == 0x00) continue; // skip subnormals and zero

		float fv;
		std::memcpy(&fv, &bits, sizeof(float));

		teju::decimal_fp dec = teju::float_to_decimal(fv);

		std::ostringstream oss;
		oss << dec.mantissa << "e" << dec.exponent;
		std::string dec_str = oss.str();

		float parsed = std::strtof(dec_str.c_str(), nullptr);

		uint32_t parsed_bits;
		std::memcpy(&parsed_bits, &parsed, sizeof(float));

		if (bits != parsed_bits) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: random float roundtrip bits=0x" << std::hex << bits
					<< " val=" << std::dec << fv
					<< " -> dec: {" << dec.mantissa << ", " << dec.exponent << "}"
					<< " -> parsed bits=0x" << std::hex << parsed_bits << std::dec << "\n";
			}
			if (nrOfFailedTests > 10) break;
		}
	}

	return nrOfFailedTests;
}

// Test with random double values via teju::double_to_decimal (IEEE roundtrip)
unsigned test_teju_random_double(unsigned count, bool reportTestCases) {
	unsigned nrOfFailedTests = 0;
	std::mt19937_64 rng(42);

	for (unsigned i = 0; i < count; ++i) {
		uint64_t bits = rng();
		// Clear sign bit (double_to_decimal works on magnitude)
		bits &= 0x7FFFFFFFFFFFFFFFull;
		uint64_t exp_bits = (bits >> 52) & 0x7FF;
		if (exp_bits == 0x7FF) continue; // skip NaN/Inf
		if (exp_bits == 0x000) continue; // skip subnormals and zero

		double dv;
		std::memcpy(&dv, &bits, sizeof(double));

		teju::decimal_fp dec = teju::double_to_decimal(dv);

		std::ostringstream oss;
		oss << dec.mantissa << "e" << dec.exponent;
		std::string dec_str = oss.str();

		double parsed = std::strtod(dec_str.c_str(), nullptr);

		uint64_t parsed_bits;
		std::memcpy(&parsed_bits, &parsed, sizeof(double));

		if (bits != parsed_bits) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: random double roundtrip bits=0x" << std::hex << bits
					<< " val=" << std::dec << dv
					<< " -> dec: {" << dec.mantissa << ", " << dec.exponent << "}"
					<< " -> parsed bits=0x" << std::hex << parsed_bits << std::dec << "\n";
			}
			if (nrOfFailedTests > 10) break;
		}
	}

	return nrOfFailedTests;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "Teju Jagua decimal conversion";
	std::string test_tag   = "teju";
	bool reportTestCases   = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "\n--- Teju Jagua core algorithm test ---\n";
	nrOfFailedTestCases += static_cast<int>(test_teju_core_float(reportTestCases));

	std::cout << "\n--- Float round-trip test (direct IEEE) ---\n";
	nrOfFailedTestCases += static_cast<int>(test_teju_float_roundtrip(reportTestCases));

	std::cout << "\n--- Double round-trip test (direct IEEE) ---\n";
	nrOfFailedTestCases += static_cast<int>(test_teju_double_roundtrip(reportTestCases));

	std::cout << "\n--- Blocktriple consistency test (teju vs bt.to_string) ---\n";
	nrOfFailedTestCases += static_cast<int>(test_teju_blocktriple_consistency(reportTestCases));

	std::cout << "\n--- Formatting flags test ---\n";
	nrOfFailedTestCases += static_cast<int>(test_teju_formatting(reportTestCases));

	std::cout << "\n--- Random float round-trip (1000 values, direct IEEE) ---\n";
	nrOfFailedTestCases += static_cast<int>(test_teju_random_float(1000, reportTestCases));

	std::cout << "\n--- Random double round-trip (1000 values, direct IEEE) ---\n";
	nrOfFailedTestCases += static_cast<int>(test_teju_random_double(1000, reportTestCases));

	// Demo: show side-by-side output
	std::cout << "\n--- Side-by-side comparison: blocktriple::to_string vs teju_to_string ---\n";
	float demo_values[] = { 1.0f, 3.14f, 0.1f, 100.0f, 1.23456789e10f, 1.17549435e-38f };
	for (float fv : demo_values) {
		blocktriple<23, BlockTripleOperator::REP, uint32_t> bt(fv);
		std::cout << "  float " << std::setw(15) << fv
			<< "  bt.to_string: " << std::setw(20) << bt.to_string(6, 0, false, true)
			<< "  teju_to_string: " << std::setw(20) << teju_to_string(bt, 6) << "\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += static_cast<int>(test_teju_core_float(reportTestCases));
	nrOfFailedTestCases += static_cast<int>(test_teju_float_roundtrip(reportTestCases));
	nrOfFailedTestCases += static_cast<int>(test_teju_double_roundtrip(reportTestCases));
	nrOfFailedTestCases += static_cast<int>(test_teju_blocktriple_consistency(reportTestCases));
	nrOfFailedTestCases += static_cast<int>(test_teju_formatting(reportTestCases));
	nrOfFailedTestCases += static_cast<int>(test_teju_random_float(1000, reportTestCases));
	nrOfFailedTestCases += static_cast<int>(test_teju_random_double(1000, reportTestCases));
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += static_cast<int>(test_teju_random_float(100000, reportTestCases));
	nrOfFailedTestCases += static_cast<int>(test_teju_random_double(100000, reportTestCases));
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);

#endif

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
