// ostream_formatting.cpp: test suite for blocktriple operator<<() formatting flags
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/verification/test_reporters.hpp>

#include <universal/internal/blocktriple/blocktriple.hpp>

namespace sw { namespace universal {

// Helper: capture operator<< output with given stream flags
template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string capture(const blocktriple<fbits, op, bt>& v,
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
		blocktriple<23, BlockTripleOperator::REP> x(3.14);
		std::string s = capture(x);
		if (s.find("3.14") == std::string::npos) {
			std::cout << "FAIL: default output of 3.14 = '" << s << "', expected to contain '3.14'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x(0.0);
		std::string s = capture(x);
		if (s.find('0') == std::string::npos) {
			std::cout << "FAIL: default output of 0.0 = '" << s << "', expected to contain '0'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x(-42.5);
		std::string s = capture(x);
		if (s.empty() || s[0] != '-') {
			std::cout << "FAIL: default output of -42.5 = '" << s << "', expected leading '-'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_scientific_output() {
	int nrOfFailedTests = 0;

	{
		blocktriple<52, BlockTripleOperator::REP> x(1.23e+45);
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
			std::cout << "FAIL: scientific output of 1.23e45 = '" << s << "', expected exponent\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<52, BlockTripleOperator::REP> x(1.23e-45);
		std::string s = capture(x, 6, std::ios_base::scientific);
		if (s.find("e-") == std::string::npos && s.find("E-") == std::string::npos) {
			std::cout << "FAIL: scientific output of 1.23e-45 = '" << s << "', expected negative exponent\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_fixed_output() {
	int nrOfFailedTests = 0;

	{
		blocktriple<23, BlockTripleOperator::REP> x(0.5);
		std::string s = capture(x, 2, std::ios_base::fixed);
		if (s != "0.50") {
			std::cout << "FAIL: fixed output of 0.5 = '" << s << "', expected '0.50'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x(3.14159);
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

	return nrOfFailedTests;
}

int test_showpos() {
	int nrOfFailedTests = 0;

	{
		blocktriple<23, BlockTripleOperator::REP> x(1.0);
		std::string s = capture(x, 6, std::ios_base::scientific | std::ios_base::showpos);
		if (s.empty() || s[0] != '+') {
			std::cout << "FAIL: showpos output of 1.0 = '" << s << "', expected leading '+'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x(-1.0);
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
		blocktriple<23, BlockTripleOperator::REP> x(1.0);
		std::string s = capture(x, 6, std::ios_base::scientific | std::ios_base::uppercase);
		if (s.find('E') == std::string::npos) {
			std::cout << "FAIL: uppercase output of 1.0 = '" << s << "', expected 'E'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x(1.0);
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
		blocktriple<23, BlockTripleOperator::REP> x(1.0);
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
		blocktriple<23, BlockTripleOperator::REP> x(1.0);
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
		blocktriple<23, BlockTripleOperator::REP> x(1.0);
		std::string s = capture(x, 2, std::ios_base::scientific, 20, '*');
		if (s.find('*') == std::string::npos) {
			std::cout << "FAIL: custom fill output = '" << s << "', expected '*' fill\n";
			++nrOfFailedTests;
		}
	}

	// Internal alignment with showpos
	{
		blocktriple<23, BlockTripleOperator::REP> x(1.0);
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

int test_special_values() {
	int nrOfFailedTests = 0;

	// setnan() defaults sign=true which means signaling NaN
	{
		blocktriple<23, BlockTripleOperator::REP> x;
		x.setnan(); // sign=true -> snan
		std::string s = capture(x);
		if (s != "snan") {
			std::cout << "FAIL: sNaN output = '" << s << "', expected 'snan'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x;
		x.setnan(false); // sign=false -> qnan
		std::string s = capture(x);
		if (s != "qnan") {
			std::cout << "FAIL: qNaN output = '" << s << "', expected 'qnan'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x;
		x.setnan();
		std::string s = capture(x, 6, std::ios_base::uppercase);
		if (s != "SNAN") {
			std::cout << "FAIL: uppercase sNaN output = '" << s << "', expected 'SNAN'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x;
		x.setinf(false);
		std::string s = capture(x);
		if (s.find("inf") == std::string::npos) {
			std::cout << "FAIL: +inf output = '" << s << "', expected 'inf'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x;
		x.setinf(true);
		std::string s = capture(x);
		if (s.find("-inf") == std::string::npos) {
			std::cout << "FAIL: -inf output = '" << s << "', expected '-inf'\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int test_zero_output() {
	int nrOfFailedTests = 0;

	{
		blocktriple<23, BlockTripleOperator::REP> x(0.0);
		std::string s = capture(x, 3, std::ios_base::fixed);
		if (s != "0.000") {
			std::cout << "FAIL: fixed zero precision 3 = '" << s << "', expected '0.000'\n";
			++nrOfFailedTests;
		}
	}

	{
		blocktriple<23, BlockTripleOperator::REP> x(0.0);
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

	std::string test_suite = "blocktriple ostream formatting";
	std::string test_tag = "blocktriple ostream formatting";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Visual demo
	{
		blocktriple<23, BlockTripleOperator::REP> x(3.14159);
		std::cout << "default    : " << x << '\n';
		std::cout << "fixed(6)   : " << std::fixed << std::setprecision(6) << x << '\n';
		std::cout << "sci(10)    : " << std::scientific << std::setprecision(10) << x << '\n';
		std::cout << "showpos    : " << std::showpos << std::scientific << std::setprecision(6) << x << '\n';
		std::cout << "uppercase  : " << std::noshowpos << std::uppercase << x << '\n';
		std::cout << "left(30)   : " << std::nouppercase << std::left << std::setw(30) << x << "|\n";
		std::cout << "right(30)  : " << std::right << std::setw(30) << x << "|\n";
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

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
