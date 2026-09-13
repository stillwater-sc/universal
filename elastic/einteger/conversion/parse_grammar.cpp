// parse_grammar.cpp: the einteger parse() grammar matchers against the std::regex patterns they replaced
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// parse() used to classify its input with four std::regex patterns. <regex> is 78,114
// preprocessed lines and pulls <sstream>, <istream> and <ostream>, and parse() has to
// stay in the einteger core because assign(const std::string&) is built on it (#1334,
// #1455). So the patterns became four hand-written matchers, and this test is the
// proof that they accept exactly the same languages: the original patterns live on
// here, as the oracle, and nowhere else.
//
// Two sweeps:
//   - every byte value 0..255, at every position that matters to the grammars
//     (after a sign run, after the radix '0', after "0b"/"0x", inside the digits),
//     which pins each character class to its exact boundaries;
//   - every string up to length N over an alphabet chosen to hit every class and its
//     neighbours, which pins the structure (sign runs, radix markers, empty bodies).
#include <universal/utility/directives.hpp>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// the four patterns exactly as einteger_impl.hpp compiled them before #1455
struct GrammarOracle {
	std::regex binary { "^[-+]*0b[01']+" };
	std::regex octal  { "^[-+]*0[0-7]*$" };
	std::regex decimal{ "^[-+]*(0|[1-9][0-9]*)$" };
	std::regex hex    { "^[-+]*0[xX][0-9a-fA-F']+" };
};

// printable rendering of a test string for failure reports, embedded NULs included
inline std::string render(const std::string& s) {
	std::string r = "\"";
	for (unsigned char c : s) {
		if (c >= 0x20 && c < 0x7F) r += static_cast<char>(c);
		else {
			const char* hexdigits = "0123456789ABCDEF";
			r += "\\x";
			r += hexdigits[c >> 4];
			r += hexdigits[c & 0xF];
		}
	}
	return r + "\"";
}

inline int CompareOne(const GrammarOracle& oracle, const std::string& s, bool reportTestCases) {
	int nrOfFailedTests = 0;
	struct Row { const char* name; bool expected; bool actual; };
	const Row rows[] = {
		{ "binary",  std::regex_match(s, oracle.binary),  einteger_detail::matches_binary(s)  },
		{ "octal",   std::regex_match(s, oracle.octal),   einteger_detail::matches_octal(s)   },
		{ "decimal", std::regex_match(s, oracle.decimal), einteger_detail::matches_decimal(s) },
		{ "hex",     std::regex_match(s, oracle.hex),     einteger_detail::matches_hex(s)     },
	};
	for (const Row& row : rows) {
		if (row.expected != row.actual) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: " << row.name << ' ' << render(s) << " regex=" << row.expected
				          << " matcher=" << row.actual << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// every byte value, dropped into every context the grammars distinguish
inline int VerifyEveryByteInContext(bool reportTestCases) {
	GrammarOracle oracle;
	const std::vector<std::string> prefixes = {
		"", "+", "-", "+-", "0", "1", "7", "9", "-0", "+1", "00", "07",
		"0b", "0x", "0X", "0B", "-0b", "+0x", "0b1", "0b'", "0x1", "0xa", "0x'", "12",
	};
	const std::vector<std::string> suffixes = { "", "0", "1", "7", "a", "'" };
	int nrOfFailedTests = 0;
	for (const std::string& prefix : prefixes) {
		for (const std::string& suffix : suffixes) {
			for (int c = 0; c < 256; ++c) {
				std::string s = prefix;
				s += static_cast<char>(c);
				s += suffix;
				nrOfFailedTests += CompareOne(oracle, s, reportTestCases);
			}
		}
	}
	return nrOfFailedTests;
}

// every string of length 0..maxLength over an alphabet that covers each character
// class of the four grammars and the characters just outside each class
inline int VerifyExhaustiveStrings(unsigned maxLength, bool reportTestCases) {
	GrammarOracle oracle;
	const std::string alphabet = "+-017894afgAFbBxX' ";
	int nrOfFailedTests = 0;
	std::vector<unsigned> digit;
	for (unsigned length = 0; length <= maxLength; ++length) {
		digit.assign(length, 0u);
		for (;;) {
			std::string s(length, ' ');
			for (unsigned i = 0; i < length; ++i) s[i] = alphabet[digit[i]];
			nrOfFailedTests += CompareOne(oracle, s, reportTestCases);
			// odometer increment
			unsigned i = 0;
			while (i < length && ++digit[i] == alphabet.size()) { digit[i] = 0; ++i; }
			if (i == length) break;
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "einteger parse grammar matchers vs std::regex (#1455)";
	std::string test_tag    = "parse grammar";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		GrammarOracle oracle;
		for (const char* s : { "0", "-0b1", "0x", "0b", "--0777", "08", "0x'", "+-0XaF'" }) {
			nrOfFailedTestCases += CompareOne(oracle, s, true);
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEveryByteInContext(reportTestCases), "every byte in context", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveStrings(4, reportTestCases), "all strings, length <= 4", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveStrings(5, reportTestCases), "all strings, length <= 5", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveStrings(6, reportTestCases), "all strings, length <= 6", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveStrings(7, reportTestCases), "all strings, length <= 7", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
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
