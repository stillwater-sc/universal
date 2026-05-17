// string_parse.cpp: regression tests for decimal-string parsing of unum (Type I)
//                  (Phase D of #835 -- API parity)
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
#include <universal/number/unum/unum.hpp>
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
	using Unum = unum<3, 4>;  // a small unum Type I configuration

	std::string test_suite  = "unum Type I decimal string parse (Phase D of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- canonical decimals match constructor (small inputs only;
	//       the small unum<3,4> configuration has limited range/precision) -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "0",       0.0     },
			{ "1.0",     1.0     },
			{ "-1.0",   -1.0     },
			{ "1.5",     1.5     },
			{ "-3.25",  -3.25    },
			{ "0.5",     0.5     },
			{ "2.0",     2.0     },
		};
		for (const auto& c : cases) {
			Unum ours, ref(c.v);
			if (!parse(c.s, ours)) ++nrOfFailedTestCases;
			if (ours != ref)       ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: unum canonical decimals\n";
	}

	// ----- nan / inf token routing. unum Type I has no Inf encoding;
	//       both nan and inf collapse to NaN, matching the existing
	//       operator=(double) which also maps +/-inf to NaN. -----
	{
		int start = nrOfFailedTestCases;
		Unum p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan",
		                       "inf", "Inf", "infinity", "INFINITY",
		                       "+inf", "-inf", "+infinity", "-infinity" }) {
			p.setnan();
			p = 0.0;  // reset to a known finite value
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isnan())   ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: unum nan/inf -> NaN routing\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		Unum p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: unum operator>> failbit\n";
	}

	// ----- malformed decimal inputs are rejected -----
	{
		int start = nrOfFailedTestCases;
		Unum p;
		for (const char* s : { "1.5abc", "abc", "++1", "1.5e" }) {
			if (parse(s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly accepted: \"" << s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: unum malformed-input rejection\n";
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
