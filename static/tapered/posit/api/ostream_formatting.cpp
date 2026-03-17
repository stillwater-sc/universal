// ostream_formatting.cpp: test suite for posit operator<<() formatting flags
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
#define POSIT_FAST_SPECIALIZATION
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#define POSIT_ENABLE_LITERALS 1
#define POSIT_ERROR_FREE_IO_FORMAT 0

#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// Helper: capture operator<< output with given stream flags
template<unsigned nbits, unsigned es, typename bt>
std::string capture(const posit<nbits, es, bt>& v,
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

int test_default_output() {
	int nrOfFailedTests = 0;

	{
		posit<32, 2> x(3.14);
		std::string s = capture(x);
		if (s.find("3.14") == std::string::npos) {
			std::cout << "FAIL: default output of 3.14 = '" << s << "', expected to contain '3.14'\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x(0.0);
		std::string s = capture(x);
		if (s.find('0') == std::string::npos) {
			std::cout << "FAIL: default output of 0.0 = '" << s << "', expected to contain '0'\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x(-42.5);
		std::string s = capture(x);
		if (s.empty() || s[0] != '-') {
			std::cout << "FAIL: default output of -42.5 = '" << s << "', expected leading '-'\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x(1.0);
		std::string s = capture(x);
		if (s.find("1.0") == std::string::npos) {
			std::cout << "FAIL: default output of 1.0 = '" << s << "', expected to contain '1.0'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_scientific_output() {
	int nrOfFailedTests = 0;

	{
		posit<32, 2> x(1.23e+10);
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
			std::cout << "FAIL: scientific output of 1.23e10 = '" << s << "', expected exponent\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x(1.23e-10);
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find("e-") == std::string::npos && s.find("E-") == std::string::npos) {
			std::cout << "FAIL: scientific output of 1.23e-10 = '" << s << "', expected negative exponent\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_fixed_output() {
	int nrOfFailedTests = 0;

	{
		posit<32, 2> x(0.5);
		std::string s = capture(x, 2, std::ios_base::fixed);
		if (s != "0.50") {
			std::cout << "FAIL: fixed output of 0.5 = '" << s << "', expected '0.50'\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x(3.14159);
		std::string s = capture(x, 3, std::ios_base::fixed);
		if (s.find('.') == std::string::npos) {
			std::cout << "FAIL: fixed output of 3.14159 = '" << s << "', expected decimal point\n";
			++nrOfFailedTests;
		}
		if (s.find('e') != std::string::npos || s.find('E') != std::string::npos) {
			std::cout << "FAIL: fixed output of 3.14159 = '" << s << "', should not contain exponent\n";
			++nrOfFailedTests;
		}
	}

	// Test precision control in fixed mode
	{
		posit<32, 2> x(1.0);
		std::string s = capture(x, 4, std::ios_base::fixed);
		auto dot = s.find('.');
		if (dot == std::string::npos) {
			std::cout << "FAIL: fixed output of 1.0 precision 4 = '" << s << "', expected decimal point\n";
			++nrOfFailedTests;
		}
		else {
			std::string frac = s.substr(dot + 1);
			if (frac.length() != 4) {
				std::cout << "FAIL: fixed 1.0 precision 4 has " << frac.length()
				          << " fraction digits = '" << s << "', expected 4\n";
				++nrOfFailedTests;
			}
		}
	}

	return nrOfFailedTests;
}

int test_showpos() {
	int nrOfFailedTests = 0;

	{
		posit<32, 2> x(1.0);
		std::string s = capture(x, 6, std::ios_base::scientific | std::ios_base::showpos);
		if (s.empty() || s[0] != '+') {
			std::cout << "FAIL: showpos output of 1.0 = '" << s << "', expected leading '+'\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x(-1.0);
		std::string s = capture(x, 6, std::ios_base::scientific | std::ios_base::showpos);
		if (s.empty() || s[0] != '-') {
			std::cout << "FAIL: showpos output of -1.0 = '" << s << "', expected leading '-'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_uppercase() {
	int nrOfFailedTests = 0;

	{
		posit<32, 2> x(1.0);
		std::string s = capture(x, 6, std::ios_base::scientific | std::ios_base::uppercase);
		if (s.find('E') == std::string::npos) {
			std::cout << "FAIL: uppercase output of 1.0 = '" << s << "', expected 'E'\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x(1.0);
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find('e') == std::string::npos) {
			std::cout << "FAIL: lowercase output of 1.0 = '" << s << "', expected 'e'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_width_alignment() {
	int nrOfFailedTests = 0;

	// Right alignment (default)
	{
		posit<32, 2> x(1.0);
		std::string s = capture(x, 2, std::ios_base::scientific, 30);
		if (static_cast<int>(s.length()) < 30) {
			std::cout << "FAIL: width 30 output length = " << s.length() << ", expected >= 30\n";
			++nrOfFailedTests;
		}
		if (s.empty() || s[0] != ' ') {
			std::cout << "FAIL: right-aligned output = '" << s << "', expected leading spaces\n";
			++nrOfFailedTests;
		}
	}

	// Left alignment
	{
		posit<32, 2> x(1.0);
		std::string s = capture(x, 2, std::ios_base::scientific | std::ios_base::left, 30);
		if (static_cast<int>(s.length()) < 30) {
			std::cout << "FAIL: left-aligned width 30 output length = " << s.length() << ", expected >= 30\n";
			++nrOfFailedTests;
		}
		if (s.empty() || s.back() != ' ') {
			std::cout << "FAIL: left-aligned output = '" << s << "', expected trailing spaces\n";
			++nrOfFailedTests;
		}
	}

	// Custom fill character
	{
		posit<32, 2> x(1.0);
		std::string s = capture(x, 2, std::ios_base::scientific, 20, '*');
		if (s.find('*') == std::string::npos) {
			std::cout << "FAIL: custom fill output = '" << s << "', expected '*' fill\n";
			++nrOfFailedTests;
		}
	}

	// Internal alignment with showpos
	{
		posit<32, 2> x(1.0);
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

int test_nar_output() {
	int nrOfFailedTests = 0;

	{
		posit<32, 2> x;
		x.setnar();
		std::string s = capture(x);
		if (s != "nar") {
			std::cout << "FAIL: NaR output = '" << s << "', expected 'nar'\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x;
		x.setnar();
		std::string s = capture(x, 6, std::ios_base::uppercase);
		if (s != "NAR") {
			std::cout << "FAIL: uppercase NaR output = '" << s << "', expected 'NAR'\n";
			++nrOfFailedTests;
		}
	}

	// NaR with width padding
	{
		posit<32, 2> x;
		x.setnar();
		std::string s = capture(x, 6, std::ios_base::fmtflags(0), 10);
		if (static_cast<int>(s.length()) < 10) {
			std::cout << "FAIL: NaR with width 10 length = " << s.length() << ", expected >= 10\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_special_values() {
	int nrOfFailedTests = 0;

	// maxpos
	{
		posit<32, 2> x;
		x.maxpos();
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
			std::cout << "FAIL: maxpos output = '" << s << "', expected scientific notation\n";
			++nrOfFailedTests;
		}
	}

	// minpos
	{
		posit<32, 2> x;
		x.minpos();
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find("e-") == std::string::npos && s.find("E-") == std::string::npos) {
			std::cout << "FAIL: minpos output = '" << s << "', expected negative exponent\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_multiple_sizes() {
	int nrOfFailedTests = 0;

	// posit<8,0>
	{
		posit<8, 0> x(1.5);
		std::string s = capture(x, 2, std::ios_base::fixed);
		if (s != "1.50") {
			std::cout << "FAIL: posit<8,0> fixed 1.5 = '" << s << "', expected '1.50'\n";
			++nrOfFailedTests;
		}
	}

	// posit<16,1>
	{
		posit<16, 1> x(3.14);
		std::string s = capture(x, 4, std::ios_base::fixed);
		if (s.find("3.14") == std::string::npos) {
			std::cout << "FAIL: posit<16,1> fixed 3.14 = '" << s << "', expected to contain '3.14'\n";
			++nrOfFailedTests;
		}
	}

	// posit<64,3>
	{
		posit<64, 3> x(2.718281828);
		std::string s = capture(x, 9, std::ios_base::fixed);
		if (s.find("2.71828") == std::string::npos) {
			std::cout << "FAIL: posit<64,3> fixed 2.718281828 = '" << s << "', expected to contain '2.71828'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_zero_output() {
	int nrOfFailedTests = 0;

	{
		posit<32, 2> x(0.0);
		std::string s = capture(x, 3, std::ios_base::fixed);
		if (s != "0.000") {
			std::cout << "FAIL: fixed zero precision 3 = '" << s << "', expected '0.000'\n";
			++nrOfFailedTests;
		}
	}

	{
		posit<32, 2> x(0.0);
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

	std::string test_suite = "posit ostream formatting";
	std::string test_tag = "posit ostream formatting";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Visual demo
	{
		posit<32, 2> pi(3.14159265358979);
		std::cout << "default    : " << pi << '\n';
		std::cout << "fixed(10)  : " << std::fixed << std::setprecision(10) << pi << '\n';
		std::cout << "sci(15)    : " << std::scientific << std::setprecision(15) << pi << '\n';
		std::cout << "showpos    : " << std::showpos << std::scientific << std::setprecision(6) << pi << '\n';
		std::cout << "uppercase  : " << std::noshowpos << std::uppercase << pi << '\n';
		std::cout << "left(30)   : " << std::nouppercase << std::left << std::setw(30) << pi << "|\n";
		std::cout << "right(30)  : " << std::right << std::setw(30) << pi << "|\n";
	}

	// Visual demo: posit<64,3>
	{
		std::cout << "\nposit<64,3> formatting:\n";
		posit<64, 3> e(2.718281828459045);
		std::cout << "default    : " << e << '\n';
		std::cout << "fixed(15)  : " << std::fixed << std::setprecision(15) << e << '\n';
		std::cout << "sci(15)    : " << std::scientific << std::setprecision(15) << e << '\n';
	}

	// Visual demo: NaR
	{
		std::cout << "\nNaR formatting:\n";
		posit<32, 2> nar;
		nar.setnar();
		std::cout << "default    : " << nar << '\n';
		std::cout << "uppercase  : " << std::uppercase << nar << '\n';
		std::cout << "width(10)  : " << std::nouppercase << std::setw(10) << nar << "|\n";
	}

	std::cout << "\nDefault output\n";
	nrOfFailedTestCases += test_default_output();

	std::cout << "Scientific notation\n";
	nrOfFailedTestCases += test_scientific_output();

	std::cout << "Fixed-point notation\n";
	nrOfFailedTestCases += test_fixed_output();

	std::cout << "NaR output\n";
	nrOfFailedTestCases += test_nar_output();

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

	std::cout << "Multiple posit sizes\n";
	nrOfFailedTestCases += test_multiple_sizes();

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
