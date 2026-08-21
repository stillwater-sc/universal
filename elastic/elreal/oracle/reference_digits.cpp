// reference_digits.cpp: unit tests for the decimal-reference oracle itself.
//
// agreed_decimal_digits(value, "3.14159...") is the measuring stick the whole
// elreal high-precision suite is graded against, so a defect in it silently
// mis-grades every test that uses it. These checks pin down its contract on
// values whose agreement is known a priori -- exactly-representable dyadics
// compared against their own exact decimal expansion, where the answer must be
// "equal to the cap" -- plus the parse-level edge cases.
//
// Regression (#1076 follow-up): the digit string is handed to einteger::parse,
// which follows C literal conventions -- a leading '0' followed by octal digits
// is read as OCTAL. The reference "0.75" yields the digit string "075", which
// parsed as 61 instead of 75, so the oracle reported 0 agreeing digits for a
// value that was exactly equal. Every reference of the form 0.<digits 0-7> was
// affected: "0.25" was read as 21, "0.125" as 85, "0.6667" as 3511. It went
// unnoticed because the only fractional reference the suite used was "0.5",
// which survives by coincidence -- octal and decimal agree on every single
// digit, so 05 == 5. Reaching for "0.75" is what exposed it.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::ZBCL;
using sw::universal::agreed_decimal_digits;
using sw::universal::from_native;

constexpr int kCap = 60;

// An exactly-representable value compared against its own exact decimal
// expansion must agree "to the cap" -- there is no error to find.
int check_exact(double v, const char* ref, bool reportTestCases) {
	int got = agreed_decimal_digits(from_native<double>(v), ref, kCap);
	if (got != kCap) {
		std::cout << "  FAIL " << v << " vs \"" << ref << "\": " << got
		          << " digits (expected " << kCap << " -- the value is exactly equal)\n";
		return 1;
	}
	if (reportTestCases) std::cout << "  ok   " << v << " vs \"" << ref << "\" == cap\n";
	return 0;
}

// A reference that is deliberately wrong in a known decimal place must be
// reported as agreeing to exactly that many digits -- this is what stops the
// leading-zero fix from being "return the cap for everything".
int check_agrees(double v, const char* ref, int want, bool reportTestCases) {
	int got = agreed_decimal_digits(from_native<double>(v), ref, kCap);
	if (got != want) {
		std::cout << "  FAIL " << v << " vs \"" << ref << "\": " << got
		          << " digits (expected " << want << ")\n";
		return 1;
	}
	if (reportTestCases) std::cout << "  ok   " << v << " vs \"" << ref << "\" == " << want << " digits\n";
	return 0;
}

int check_throws(const char* ref, const char* why, bool reportTestCases) {
	try {
		(void)agreed_decimal_digits(from_native<double>(1.0), ref, kCap);
	}
	catch (const std::invalid_argument&) {
		if (reportTestCases) std::cout << "  ok   \"" << ref << "\" rejected (" << why << ")\n";
		return 0;
	}
	std::cout << "  FAIL \"" << ref << "\" was accepted (expected rejection: " << why << ")\n";
	return 1;
}

} // anonymous

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

int main()
try {
	using namespace sw::universal;
	std::string test_suite = "elreal decimal-reference oracle (agreed_decimal_digits)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = false;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// TODO: place hand-run diagnostics here (this branch ignores failures)

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	// (1) The octal-prefix regression. Every one of these digit strings is a
	//     valid C octal literal once the '.' is removed, so each was misparsed
	//     before the fix; the parenthesised value is what it used to become.
	nrOfFailedTestCases += check_exact(0.75,       "0.75",       reportTestCases);   // was 61
	nrOfFailedTestCases += check_exact(0.25,       "0.25",       reportTestCases);   // was 21
	nrOfFailedTestCases += check_exact(0.125,      "0.125",      reportTestCases);   // was 85
	nrOfFailedTestCases += check_exact(0.0625,     "0.0625",     reportTestCases);   // was 53
	nrOfFailedTestCases += check_exact(0.5,        "0.50",       reportTestCases);   // was 40
	nrOfFailedTestCases += check_exact(0.00390625, "0.00390625", reportTestCases);
	nrOfFailedTestCases += check_exact(0.375,      "0.375",      reportTestCases);

	// (2) Forms that were already correct must stay correct: the single-digit
	//     coincidence, a leading '.', an all-decimal reference, an integer, and
	//     trailing zeros (which change the digit count but not the value).
	nrOfFailedTestCases += check_exact(0.5,   "0.5",          reportTestCases);
	nrOfFailedTestCases += check_exact(0.75,  ".75",          reportTestCases);
	nrOfFailedTestCases += check_exact(0.75,  "0.7500000000", reportTestCases);
	nrOfFailedTestCases += check_exact(0.9,   "0.9000000000000000222044604925031308084726333618164062500", reportTestCases);
	nrOfFailedTestCases += check_exact(1.5,   "1.5",          reportTestCases);
	nrOfFailedTestCases += check_exact(2.0,   "2",            reportTestCases);
	nrOfFailedTestCases += check_exact(3.0,   "3.0",          reportTestCases);

	// (3) The oracle must still be able to MEASURE disagreement -- a reference
	//     perturbed at a known decimal place reports exactly that many digits.
	//     Without this, "always return the cap" would pass section (1).
	nrOfFailedTestCases += check_agrees(0.75, "0.7499999999999999999999", 21, reportTestCases);
	nrOfFailedTestCases += check_agrees(0.75, "0.76",                      1, reportTestCases);
	nrOfFailedTestCases += check_agrees(0.25, "0.2501",                    3, reportTestCases);

	// (4) Malformed and degenerate references are rejected, not reinterpreted.
	nrOfFailedTestCases += check_throws("0.0",   "zero reference: relative agreement is undefined", reportTestCases);
	nrOfFailedTestCases += check_throws("0",     "zero reference: relative agreement is undefined", reportTestCases);
	nrOfFailedTestCases += check_throws("3e10",  "exponent notation is not supported",              reportTestCases);
	nrOfFailedTestCases += check_throws("-0.5",  "a sign is not supported",                         reportTestCases);
	nrOfFailedTestCases += check_throws("1.2.3", "multiple decimal points",                         reportTestCases);
	nrOfFailedTestCases += check_throws("",      "no digits",                                       reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
