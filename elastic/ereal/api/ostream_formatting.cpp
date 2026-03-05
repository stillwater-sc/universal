// ostream_formatting.cpp: test suite for ereal operator<<() formatting flags
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// Helper: capture operator<< output with given stream flags
template<unsigned maxlimbs>
std::string capture(const ereal<maxlimbs>& v,
	std::streamsize precision = 6,
	std::ios_base::fmtflags flags = std::ios_base::fmtflags(0),
	std::streamsize width = 0,
	char fill = ' ')
{
	std::ostringstream oss;
	oss.precision(precision);
	oss.flags(flags);
	oss.width(width);
	oss.fill(fill);
	oss << v;
	return oss.str();
}

// Test default (scientific) output for normal values
int test_default_output() {
	int nrOfFailedTests = 0;

	{
		ereal<4> x(3.14);
		std::string s = capture(x);
		// Default: scientific, precision 6 -> "3.140000e+00"
		if (s.find("3.14") == std::string::npos) {
			std::cout << "FAIL: default output of 3.14 = '" << s << "', expected to contain '3.14'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x(0.0);
		std::string s = capture(x);
		if (s.find('0') == std::string::npos) {
			std::cout << "FAIL: default output of 0.0 = '" << s << "', expected to contain '0'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x(-42.5);
		std::string s = capture(x);
		if (s[0] != '-') {
			std::cout << "FAIL: default output of -42.5 = '" << s << "', expected leading '-'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test scientific notation output
int test_scientific_output() {
	int nrOfFailedTests = 0;

	{
		ereal<4> x(1.23e+45);
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
			std::cout << "FAIL: scientific output of 1.23e45 = '" << s << "', expected exponent\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x(1.23e-45);
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find("e-") == std::string::npos && s.find("E-") == std::string::npos) {
			std::cout << "FAIL: scientific output of 1.23e-45 = '" << s << "', expected negative exponent\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test fixed-point output
int test_fixed_output() {
	int nrOfFailedTests = 0;

	// Sub-unit value in (0, 1)
	{
		ereal<4> x(0.5);
		std::string s = capture(x, 2, std::ios_base::fixed);
		if (s != "0.50") {
			std::cout << "FAIL: fixed output of 0.5 = '" << s << "', expected '0.50'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x(3.14159);
		std::string s = capture(x, 3, std::ios_base::fixed);
		// fixed precision 3 -> "3.142" (rounded)
		if (s.find('.') == std::string::npos) {
			std::cout << "FAIL: fixed output of 3.14159 = '" << s << "', expected decimal point\n";
			++nrOfFailedTests;
		}
		// Should NOT contain 'e' or 'E'
		if (s.find('e') != std::string::npos || s.find('E') != std::string::npos) {
			std::cout << "FAIL: fixed output of 3.14159 = '" << s << "', should not contain exponent\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<8> pi("3.14159265358979323846");
		std::string s = capture(pi, 15, std::ios_base::fixed);
		// Should have 15 digits after decimal point
		auto dot = s.find('.');
		if (dot == std::string::npos) {
			std::cout << "FAIL: fixed output of pi = '" << s << "', expected decimal point\n";
			++nrOfFailedTests;
		}
		else {
			std::string fraction = s.substr(dot + 1);
			if (static_cast<int>(fraction.length()) != 15) {
				std::cout << "FAIL: fixed pi precision 15 has " << fraction.length() << " fraction digits, expected 15\n";
				++nrOfFailedTests;
			}
		}
	}

	return nrOfFailedTests;
}

// Test showpos flag
int test_showpos() {
	int nrOfFailedTests = 0;

	{
		ereal<4> x(1.0);
		std::string s = capture(x, 6, std::ios_base::scientific | std::ios_base::showpos);
		if (s[0] != '+') {
			std::cout << "FAIL: showpos output of 1.0 = '" << s << "', expected leading '+'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x(-1.0);
		std::string s = capture(x, 6, std::ios_base::scientific | std::ios_base::showpos);
		if (s[0] != '-') {
			std::cout << "FAIL: showpos output of -1.0 = '" << s << "', expected leading '-'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test uppercase flag
int test_uppercase() {
	int nrOfFailedTests = 0;

	{
		ereal<4> x(1.0);
		std::string s = capture(x, 6, std::ios_base::scientific | std::ios_base::uppercase);
		if (s.find('E') == std::string::npos) {
			std::cout << "FAIL: uppercase output of 1.0 = '" << s << "', expected 'E'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x(1.0);
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find('e') == std::string::npos) {
			std::cout << "FAIL: lowercase output of 1.0 = '" << s << "', expected 'e'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test width and alignment
int test_width_alignment() {
	int nrOfFailedTests = 0;

	// Right alignment (default)
	{
		ereal<4> x(1.0);
		std::string s = capture(x, 2, std::ios_base::scientific, 30);
		if (static_cast<int>(s.length()) < 30) {
			std::cout << "FAIL: width 30 output length = " << s.length() << ", expected >= 30\n";
			++nrOfFailedTests;
		}
		// right-justified: leading spaces
		if (s[0] != ' ') {
			std::cout << "FAIL: right-aligned output = '" << s << "', expected leading spaces\n";
			++nrOfFailedTests;
		}
	}

	// Left alignment
	{
		ereal<4> x(1.0);
		std::string s = capture(x, 2, std::ios_base::scientific | std::ios_base::left, 30);
		if (static_cast<int>(s.length()) < 30) {
			std::cout << "FAIL: left-aligned width 30 output length = " << s.length() << ", expected >= 30\n";
			++nrOfFailedTests;
		}
		// left-justified: trailing spaces
		if (s.back() != ' ') {
			std::cout << "FAIL: left-aligned output = '" << s << "', expected trailing spaces\n";
			++nrOfFailedTests;
		}
	}

	// Custom fill character
	{
		ereal<4> x(1.0);
		std::string s = capture(x, 2, std::ios_base::scientific, 20, '*');
		if (s.find('*') == std::string::npos) {
			std::cout << "FAIL: custom fill output = '" << s << "', expected '*' fill\n";
			++nrOfFailedTests;
		}
	}

	// Internal alignment with showpos: sign should stay at column 0, fill between sign and digits
	{
		ereal<4> x(1.0);
		std::string s = capture(x, 2,
			std::ios_base::scientific | std::ios_base::showpos | std::ios_base::internal,
			14, '_');
		if (s.empty() || s[0] != '+' || s[1] != '_') {
			std::cout << "FAIL: internal showpos output = '" << s
			          << "', expected '+' followed by fill\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test special values (NaN, infinity)
int test_special_values() {
	int nrOfFailedTests = 0;

	{
		ereal<4> x;
		x.setnan();
		std::string s = capture(x);
		if (s != "nan") {
			std::cout << "FAIL: NaN output = '" << s << "', expected 'nan'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x;
		x.setnan();
		std::string s = capture(x, 6, std::ios_base::uppercase);
		if (s != "NAN") {
			std::cout << "FAIL: uppercase NaN output = '" << s << "', expected 'NAN'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x;
		x.setinf(false);
		std::string s = capture(x);
		if (s.find("inf") == std::string::npos) {
			std::cout << "FAIL: +inf output = '" << s << "', expected 'inf'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x;
		x.setinf(true);
		std::string s = capture(x);
		if (s.find("-inf") == std::string::npos) {
			std::cout << "FAIL: -inf output = '" << s << "', expected '-inf'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x;
		x.setinf(false);
		std::string s = capture(x, 6, std::ios_base::uppercase);
		if (s.find("INF") == std::string::npos) {
			std::cout << "FAIL: uppercase +inf output = '" << s << "', expected 'INF'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test precision for multi-limb ereals
int test_extended_precision() {
	int nrOfFailedTests = 0;

	{
		// ereal<8> has ~128 decimal digits of precision
		ereal<8> pi("3.14159265358979323846");
		std::string s = capture(pi, 20, std::ios_base::scientific);
		// Should start with "3.14159265358979323846"
		if (s.substr(0, 22) != "3.14159265358979323846") {
			std::cout << "FAIL: ereal<8> pi at precision 20 = '" << s << "'\n";
			std::cout << "      expected to start with '3.14159265358979323846'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Test zero output in various formats
int test_zero_output() {
	int nrOfFailedTests = 0;

	{
		ereal<4> x(0.0);
		std::string s = capture(x, 3, std::ios_base::fixed);
		if (s != "0.000") {
			std::cout << "FAIL: fixed zero precision 3 = '" << s << "', expected '0.000'\n";
			++nrOfFailedTests;
		}
	}

	{
		ereal<4> x(0.0);
		std::string s = capture(x, 3, std::ios_base::scientific);
		if (s.find("0.000") == std::string::npos) {
			std::cout << "FAIL: scientific zero precision 3 = '" << s << "', expected '0.000e+00'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "ereal<maxlimbs> ostream formatting";
	std::string test_tag = "ereal ostream formatting";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		ereal<8> pi("3.14159265358979323846");
		std::cout << "default    : " << pi << '\n';
		std::cout << "fixed(15)  : " << std::fixed << std::setprecision(15) << pi << '\n';
		std::cout << "sci(20)    : " << std::scientific << std::setprecision(20) << pi << '\n';
		std::cout << "showpos    : " << std::showpos << std::scientific << std::setprecision(6) << pi << '\n';
		std::cout << "uppercase  : " << std::noshowpos << std::uppercase << pi << '\n';
		std::cout << "left(30)   : " << std::nouppercase << std::left << std::setw(30) << pi << "|\n";
		std::cout << "right(30)  : " << std::right << std::setw(30) << pi << "|\n";
	}

    {
		ereal<19> pi("1.234567890123456789012345678901234567890123456789012345");
		std::cout << "default    : " << pi << '\n';
		std::cout << "fixed(15)  : " << std::fixed << std::setprecision(15) << pi << '\n';
		std::cout << "sci(20)    : " << std::scientific << std::setprecision(20) << pi << '\n';
		std::cout << "showpos    : " << std::showpos << std::scientific << std::setprecision(6) << pi << '\n';
		std::cout << "uppercase  : " << std::noshowpos << std::uppercase << pi << '\n';
		std::cout << "left(60)   : " << std::nouppercase << std::left << std::setw(60) << pi << "|\n";
		std::cout << "right(60)  : " << std::right << std::setw(60) << pi << "|\n";
		std::cout << "sci(54)    : " << std::scientific << std::setprecision(54) << pi << "|\n";
    }

	std::cout << "Default output\n";
	nrOfFailedTestCases += test_default_output();

	std::cout << "Scientific notation\n";
	nrOfFailedTestCases += test_scientific_output();

	std::cout << "Fixed-point notation\n";
	nrOfFailedTestCases += test_fixed_output();

	std::cout << "Special values\n";
	nrOfFailedTestCases += test_special_values();

	std::cout << "Zero output\n";
	nrOfFailedTestCases += test_zero_output();

	std::cout << "showpos flag\n";
	nrOfFailedTestCases += test_showpos();

	std::cout << "uppercase flag\n";
	nrOfFailedTestCases += test_uppercase();

	std::cout << "Width and alignment\n";
	nrOfFailedTestCases += test_width_alignment();

	std::cout << "Extended precision output\n";
	nrOfFailedTestCases += test_extended_precision();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
