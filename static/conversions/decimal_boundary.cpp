// decimal_boundary.cpp: boundary regression for support::decimal integer conversion (#1273)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// LLONG_MIN used to trigger signed-overflow UB: convert_to_decimal did `v *= -1` and
// setvalue did `-v`, whose magnitude 2^63 is not representable in long long. In
// convert_to_decimal that left v negative and the bit loop spun forever (a hang); the
// fix negates in the unsigned domain (#1273).
#include <universal/utility/directives.hpp>
#include <climits>
#include <sstream>
#include <string>
#include <iostream>

#include <universal/number/support/decimal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::support::decimal;

	std::string toStr(const decimal& d) { std::stringstream ss; ss << d; return ss.str(); }

	int VerifyDecimalConversion(bool reportTestCases) {
		int nrOfFailures = 0;
		struct Case { long long v; const char* expected; };
		const Case cases[] = {
			{ LLONG_MIN,     "-9223372036854775808" },   // the UB / infinite-loop case
			{ LLONG_MIN + 1, "-9223372036854775807" },
			{ LLONG_MAX,      "9223372036854775807" },
			{ 0,   "0" }, { -1, "-1" }, { 1, "1" }, { -1000000, "-1000000" }, { 123456789, "123456789" },
		};
		for (const auto& c : cases) {
			decimal a; sw::universal::support::convert_to_decimal(c.v, a);
			if (toStr(a) != c.expected) {
				++nrOfFailures;
				if (reportTestCases) std::cout << "    FAIL convert_to_decimal(" << c.v << ") = " << toStr(a)
					<< " expected " << c.expected << '\n';
			}
			decimal b; b.setvalue(c.v);
			if (toStr(b) != c.expected) {
				++nrOfFailures;
				if (reportTestCases) std::cout << "    FAIL setvalue(" << c.v << ") = " << toStr(b)
					<< " expected " << c.expected << '\n';
			}
		}
		return nrOfFailures;
	}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "support::decimal integer conversion boundaries (#1273)";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyDecimalConversion(reportTestCases),
		"convert_to_decimal / setvalue (incl. LLONG_MIN)", "decimal");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
