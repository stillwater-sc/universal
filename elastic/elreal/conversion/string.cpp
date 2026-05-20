// string.cpp: tests for elreal(const std::string&) construction (Phase B)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

static int check_string_value(const char* input, double expected_lead) {
	sw::universal::elreal x(std::string{input});
	double back = x.at(0);
	if (back != expected_lead) {
		std::cerr << "FAIL: elreal(\"" << input << "\").at(0) = " << back
			<< " (expected " << expected_lead << ")\n";
		return 1;
	}
	return 0;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase B string parsing";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- decimal integers ------------------------------------------------
	nrOfFailedTestCases += check_string_value("0",          0.0);
	nrOfFailedTestCases += check_string_value("42",         42.0);
	nrOfFailedTestCases += check_string_value("-7",        -7.0);
	nrOfFailedTestCases += check_string_value("+12",        12.0);

	// --- decimal fractions -----------------------------------------------
	nrOfFailedTestCases += check_string_value("0.5",        0.5);
	nrOfFailedTestCases += check_string_value("3.14",       3.14);
	nrOfFailedTestCases += check_string_value("-2.5",      -2.5);
	nrOfFailedTestCases += check_string_value("0.125",      0.125);

	// --- scientific notation ---------------------------------------------
	nrOfFailedTestCases += check_string_value("1e0",        1.0);
	nrOfFailedTestCases += check_string_value("1.5e2",      150.0);
	nrOfFailedTestCases += check_string_value("3.14e-2",    0.0314);
	nrOfFailedTestCases += check_string_value("6.022e23",   6.022e23);
	nrOfFailedTestCases += check_string_value("-1.0e-10",  -1.0e-10);

	// --- rational form ---------------------------------------------------
	nrOfFailedTestCases += check_string_value("1/2",        0.5);
	nrOfFailedTestCases += check_string_value("3/4",        0.75);
	nrOfFailedTestCases += check_string_value("-22/7",     -22.0 / 7.0);

	// "1/3" delivers a real correction (the acceptance criterion of #875).
	{
		elreal third{ std::string("1/3") };
		if (third.at(0) != 1.0 / 3.0) {
			std::cerr << "FAIL: elreal(\"1/3\").at(0) != IEEE 1.0/3.0\n";
			++nrOfFailedTestCases;
		}
		if (third.at(1) == 0.0) {
			std::cerr << "FAIL: elreal(\"1/3\").at(1) == 0 (no refinement)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- special tokens (case-insensitive) -------------------------------
	{
		elreal nan_lower{ std::string("nan") };
		elreal nan_upper{ std::string("NAN") };
		elreal nan_mixed{ std::string("NaN") };
		if (!nan_lower.isnan() || !nan_upper.isnan() || !nan_mixed.isnan()) {
			std::cerr << "FAIL: nan / NAN / NaN did not all parse as NaN\n";
			++nrOfFailedTestCases;
		}
	}
	{
		elreal pinf{ std::string("inf") };
		elreal pinfinity{ std::string("infinity") };
		elreal ninf{ std::string("-inf") };
		elreal ninfinity{ std::string("-INFINITY") };
		if (!pinf.isinf() || !pinfinity.isinf() || !ninf.isinf() || !ninfinity.isinf()) {
			std::cerr << "FAIL: inf / infinity / -inf / -INFINITY did not all parse as inf\n";
			++nrOfFailedTestCases;
		}
		if (double(ninf) > 0.0 || double(ninfinity) > 0.0) {
			std::cerr << "FAIL: -inf / -INFINITY did not produce negative inf\n";
			++nrOfFailedTestCases;
		}
	}

	// --- parse failures yield canonical zero -----------------------------
	{
		// The std::string constructor swallows parse failures (matching dfloat /
		// ereal). Use the bool-returning parse() to observe the failure directly.
		elreal sink;
		if (parse(std::string{"foo"}, sink)) {
			std::cerr << "FAIL: parse(\"foo\") returned true\n";
			++nrOfFailedTestCases;
		}
		if (parse(std::string{""}, sink)) {
			std::cerr << "FAIL: parse(\"\") returned true\n";
			++nrOfFailedTestCases;
		}
		if (parse(std::string{"1.2.3"}, sink)) {
			std::cerr << "FAIL: parse(\"1.2.3\") returned true\n";
			++nrOfFailedTestCases;
		}
		if (parse(std::string{"1/0"}, sink)) {
			std::cerr << "FAIL: parse(\"1/0\") returned true (denominator zero must be rejected)\n";
			++nrOfFailedTestCases;
		}

		elreal silent{ std::string("garbage") };
		if (!silent.iszero()) {
			std::cerr << "FAIL: silent-failure string ctor did not yield zero\n";
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
