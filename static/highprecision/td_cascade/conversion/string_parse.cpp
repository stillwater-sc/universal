// string_parse.cpp: regression tests for decimal-string parsing of td_cascade
//                  (Phase C of #835 -- API parity)
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
#include <universal/number/td_cascade/td_cascade.hpp>
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

	std::string test_suite  = "td_cascade decimal string parse (Phase C of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

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
			{ "1.25e3",  1250.0  },
			{ "1024",    1024.0  },
		};
		for (const auto& c : cases) {
			td_cascade ours, ref(c.v);
			if (!parse(c.s, ours)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << c.s << '\n';
				continue;
			}
			if (ours != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  mismatch on \"" << c.s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: td_cascade canonical decimals\n";
	}

	{
		int start = nrOfFailedTestCases;
		td_cascade p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan", "  nan" }) {
			p = td_cascade(0.0);
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isnan())   ++nrOfFailedTestCases;
		}
		for (const char* s : { "inf", "Inf", "infinity", "INFINITY",
		                       "+inf", "+Inf", "+infinity", "+INFINITY",
		                       "  inf", " infinity" }) {
			p = td_cascade(0.0);
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isinf())   ++nrOfFailedTestCases;
			if (p.signbit())  ++nrOfFailedTestCases;
		}
		for (const char* s : { "-inf", "-Inf", "-infinity", "-INFINITY" }) {
			p = td_cascade(0.0);
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isinf())   ++nrOfFailedTestCases;
			if (!p.signbit()) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: td_cascade nan/inf token routing\n";
	}

	// ----- parse rejects malformed inputs that produce no mantissa digits -----
	{
		int start = nrOfFailedTestCases;
		td_cascade p;
		for (const char* s : { "", "   ", "+", "-", ".", "e10", "+e5", "+.", " " }) {
			if (parse(s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly accepted: \"" << s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: td_cascade no-digit rejection\n";
	}

	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		td_cascade p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: td_cascade operator>> failbit\n";
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
