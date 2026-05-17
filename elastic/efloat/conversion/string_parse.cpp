// string_parse.cpp: regression tests for decimal-string parsing of efloat
//                  (Phase E of #835 -- operator>> hygiene only)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// efloat::parse is currently a stub that returns false unconditionally.
// Phase E adds operator>> hygiene only: when parse() returns false, the
// stream's failbit is now set (instead of just a cerr diagnostic).
// Full parse() implementation is tracked in issue #856.

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/efloat/efloat.hpp>
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
	using Float = efloat<4>;

	std::string test_suite  = "efloat decimal string parse (Phase E of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- operator>> sets failbit when parse() returns false. Since
	//       efloat::parse is currently a stub that always returns false,
	//       any input drives failbit. This documents the current contract
	//       and will continue to hold once parse() is implemented (#856) --
	//       only the inputs that pass through parse will change. -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("1.0");
		Float p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: efloat operator>> failbit on stub parse\n";
	}

	// ----- operator>> handles EOF without crashing -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		Float p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: efloat operator>> EOF handling\n";
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
