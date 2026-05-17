// string_parse.cpp: regression tests for decimal-string parsing of bfloat16
//                  (Phase D of #835 -- parse() implementation + API parity)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// Before Phase D, bfloat16::parse() was a stub that returned false and set
// value to zero unconditionally. This test exercises the new implementation.

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/bfloat16/bfloat16.hpp>
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

	std::string test_suite  = "bfloat16 decimal string parse (Phase D of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- canonical decimals match bfloat16(double) constructor -----
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
			{ "1.25e3",  1250.0  },
			{ "0.0625",  0.0625  },
		};
		for (const auto& c : cases) {
			bfloat16 ours, ref(c.v);
			if (!parse(c.s, ours)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << c.s << '\n';
				continue;
			}
			if (ours != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  mismatch on \"" << c.s << "\":\n"
					          << "    string -> " << to_binary(ours) << '\n'
					          << "    ref    -> " << to_binary(ref)  << '\n';
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 canonical decimals\n";
	}

	// ----- nan / inf token routing -----
	{
		int start = nrOfFailedTestCases;
		bfloat16 p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan" }) {
			p = bfloat16(0.0);
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isnan())   ++nrOfFailedTestCases;
		}
		for (const char* s : { "inf", "Inf", "infinity", "INFINITY",
		                       "+inf", "+Inf", "+infinity", "+INFINITY" }) {
			p = bfloat16(0.0);
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isinf())   ++nrOfFailedTestCases;
			if (p.sign())     ++nrOfFailedTestCases;  // positive
		}
		for (const char* s : { "-inf", "-Inf", "-infinity", "-INFINITY" }) {
			p = bfloat16(0.0);
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isinf())   ++nrOfFailedTestCases;
			if (!p.sign())    ++nrOfFailedTestCases;  // negative
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 nan/inf token routing\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		bfloat16 p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 operator>> failbit\n";
	}

	// ----- malformed decimal inputs are rejected -----
	{
		int start = nrOfFailedTestCases;
		bfloat16 p;
		for (const char* s : { "1.5abc", "abc", "++1", "1.5e" }) {
			if (parse(s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly accepted: \"" << s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: bfloat16 malformed-input rejection\n";
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
