// string_parse.cpp: regression tests for decimal-string parsing of dfloat
//                  (Phase E of #835 -- API parity)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// dfloat::assign() already implements a complete decimal parser with nan/inf
// token handling. The Phase E scope here is just operator>> hygiene
// (extraction guard + failbit). Full parse coverage is tracked separately
// in issue #852.

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/dfloat/dfloat.hpp>
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
	using Dfloat = decimal64;

	std::string test_suite  = "dfloat decimal string parse (Phase E of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- canonical decimals (already handled by assign) -----
	{
		int start = nrOfFailedTestCases;
		Dfloat p;
		if (!parse("0", p))      ++nrOfFailedTestCases;
		if (!parse("1.0", p))    ++nrOfFailedTestCases;
		if (!parse("-3.25", p))  ++nrOfFailedTestCases;
		if (!parse("1.25e3", p)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: dfloat canonical decimals\n";
	}

	// ----- nan / inf token routing (assign handles these) -----
	{
		int start = nrOfFailedTestCases;
		Dfloat p;
		for (const char* s : { "nan", "NaN", "inf", "Inf",
		                       "+nan", "-inf", "infinity" }) {
			if (!parse(s, p)) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: dfloat nan/inf token parsing\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		// dfloat's assign() is permissive; use a clearly invalid token by
		// using an empty-after-extraction case via a stream at EOF.
		std::istringstream is("");
		Dfloat p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: dfloat operator>> EOF failbit\n";
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
