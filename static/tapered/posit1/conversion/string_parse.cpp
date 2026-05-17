// string_parse.cpp: regression tests for decimal-string parsing of posit (posit1)
//                  (Phase D of #835 -- API parity for the older posit revision)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/posit1/posit1.hpp>
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

	std::string test_suite  = "posit1 decimal string parse (Phase D of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- canonical decimals match constructor for small configs -----
	{
		int start = nrOfFailedTestCases;
		using Posit = posit<32, 2>;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "0",       0.0     },
			{ "1.0",     1.0     },
			{ "-1.0",   -1.0     },
			{ "1.5",     1.5     },
			{ "-3.25",  -3.25    },
			{ "0.5",     0.5     },
			{ "1.25e3",  1250.0  },
		};
		for (const auto& c : cases) {
			Posit ours, ref(c.v);
			std::string s(c.s);
			if (!parse(s, ours)) ++nrOfFailedTestCases;
			if (ours != ref)     ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit1<32,2> canonical decimals\n";
	}

	// ----- nan / inf token routing to NaR -----
	{
		int start = nrOfFailedTestCases;
		posit<32, 2> p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan",
		                       "inf", "Inf", "infinity", "INFINITY",
		                       "+inf", "-inf", "+infinity", "-infinity" }) {
			std::string t(s);
			p.setzero();
			if (!parse(t, p)) ++nrOfFailedTestCases;
			if (!p.isnar())   ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit1 nan/inf -> NaR routing\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		posit<32, 2> p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit1 operator>> failbit\n";
	}

	// ----- malformed decimal inputs are rejected -----
	{
		int start = nrOfFailedTestCases;
		posit<32, 2> p;
		for (const char* s : { "1.5abc", "abc", "++1", "1.5e" }) {
			std::string t(s);
			if (parse(t, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly accepted: \"" << s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit1 malformed-input rejection\n";
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
