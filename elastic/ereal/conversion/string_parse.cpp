// string_parse.cpp: regression tests for decimal-string parsing of ereal
//                  (Phase E of #835 -- nan/inf + operator>> hygiene)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_reporters.hpp>

namespace {
struct CerrSilencer {
	std::ostringstream sink;
	std::streambuf*    old;
	CerrSilencer() : old(std::cerr.rdbuf(sink.rdbuf())) {}
	~CerrSilencer() { std::cerr.rdbuf(old); }
	CerrSilencer(const CerrSilencer&)            = delete;
	CerrSilencer& operator=(const CerrSilencer&) = delete;
};
}

int main()
try {
	using namespace sw::universal;
	using Real = ereal<4>;

	std::string test_suite  = "ereal decimal string parse (Phase E of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- canonical decimals (existing digit-accumulate parse path) -----
	{
		int start = nrOfFailedTestCases;
		Real p;
		if (!parse("0", p))      ++nrOfFailedTestCases;
		if (!parse("1.0", p))    ++nrOfFailedTestCases;
		if (!parse("-3.25", p))  ++nrOfFailedTestCases;
		if (!parse("1.25e3", p)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: ereal canonical decimals\n";
	}

	// ----- nan / inf token routing -----
	{
		int start = nrOfFailedTestCases;
		Real p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan" }) {
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isnan())   ++nrOfFailedTestCases;
		}
		for (const char* s : { "inf", "Inf", "infinity", "INFINITY",
		                       "+inf", "+infinity" }) {
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isinf())   ++nrOfFailedTestCases;
		}
		for (const char* s : { "-inf", "-Inf", "-infinity" }) {
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isinf())   ++nrOfFailedTestCases;
			// -inf must have negative sign; std::signbit works for ereal
			// since its leading limb is a regular IEEE double.
			if (std::signbit(static_cast<double>(p)) != true) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: ereal nan/inf token routing\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		Real p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: ereal operator>> failbit\n";
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
