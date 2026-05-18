// string_parse.cpp: regression tests for string parsing of erational
//                  (Phase E of #835 -- operator>> hygiene; #855 -- extended grammar)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// erational::parse accepts integer, p/q rational, decimal-point, scientific,
// and mixed-form (p/q with decimal sides) inputs.  Results are simplified to
// lowest terms via GCD; q = 0 is rejected.  operator>> sets failbit on parse
// failure (#835 Phase E).

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/erational/erational.hpp>
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

int CheckParse(const char* input, const char* expected_pq, bool reportTestCases) {
	using namespace sw::universal;
	erational v;
	if (!v.parse(input)) {
		if (reportTestCases) std::cout << "FAIL parse rejected: '" << input << "'\n";
		return 1;
	}
	std::ostringstream os;
	os << v;
	if (os.str() != std::string(expected_pq)) {
		if (reportTestCases) {
			std::cout << "FAIL parse('" << input << "') = "
			          << os.str() << " expected " << expected_pq << '\n';
		}
		return 1;
	}
	return 0;
}

int CheckReject(const char* input, bool reportTestCases) {
	using namespace sw::universal;
	erational v;
	if (v.parse(input)) {
		if (reportTestCases) {
			std::ostringstream os; os << v;
			std::cout << "FAIL: '" << input << "' should be rejected, got " << os.str() << '\n';
		}
		return 1;
	}
	return 0;
}

}  // namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "erational string parse (issues #835 Phase E, #855)";
	bool reportTestCases    = true;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- integer literals -> p/1 -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("0",       "0/1",       reportTestCases);
		nrOfFailedTestCases += CheckParse("42",      "42/1",      reportTestCases);
		nrOfFailedTestCases += CheckParse("+7",      "7/1",       reportTestCases);
		nrOfFailedTestCases += CheckParse("-1000",   "-1000/1",   reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "integer parse", "erational parse");
	}

	// ----- p/q rational form with GCD simplification -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("1/2",     "1/2",       reportTestCases);
		nrOfFailedTestCases += CheckParse("4/8",     "1/2",       reportTestCases);
		nrOfFailedTestCases += CheckParse("6/4",     "3/2",       reportTestCases);
		nrOfFailedTestCases += CheckParse("-22/7",   "-22/7",     reportTestCases);
		nrOfFailedTestCases += CheckParse("355/113", "355/113",   reportTestCases);
		// sign on either or both sides
		nrOfFailedTestCases += CheckParse("22/-7",   "-22/7",     reportTestCases);
		nrOfFailedTestCases += CheckParse("-22/-7",  "22/7",      reportTestCases);
		nrOfFailedTestCases += CheckParse("+1/+2",   "1/2",       reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "p/q form (signs + simplification)", "erational parse");
	}

	// ----- decimal literals -> simplified rational -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("3.14",    "157/50",    reportTestCases);
		nrOfFailedTestCases += CheckParse("-3.14",   "-157/50",   reportTestCases);
		nrOfFailedTestCases += CheckParse("0.25",    "1/4",       reportTestCases);
		nrOfFailedTestCases += CheckParse("-0.5",    "-1/2",      reportTestCases);
		nrOfFailedTestCases += CheckParse("0.001",   "1/1000",    reportTestCases);
		nrOfFailedTestCases += CheckParse("0.10",    "1/10",      reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "decimal -> rational", "erational parse");
	}

	// ----- scientific notation -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("1e10",    "10000000000/1", reportTestCases);
		nrOfFailedTestCases += CheckParse("1.5e2",   "150/1",         reportTestCases);
		nrOfFailedTestCases += CheckParse("1.5e0",   "3/2",           reportTestCases);
		nrOfFailedTestCases += CheckParse("1.5e-1",  "3/20",          reportTestCases);
		nrOfFailedTestCases += CheckParse("-1e3",    "-1000/1",       reportTestCases);
		nrOfFailedTestCases += CheckParse("3.14e2",  "314/1",         reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "scientific notation", "erational parse");
	}

	// ----- mixed p/q where each side may be decimal or scientific -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("3.14/2",  "157/100",   reportTestCases);
		nrOfFailedTestCases += CheckParse("1/0.5",   "2/1",       reportTestCases);
		nrOfFailedTestCases += CheckParse("1e2/2e1", "5/1",       reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "mixed p/q (decimal sides)", "erational parse");
	}

	// ----- division by zero must be rejected (no NaR encoding in erational) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckReject("1/0",    reportTestCases);
		nrOfFailedTestCases += CheckReject("5/0.0",  reportTestCases);
		nrOfFailedTestCases += CheckReject("0/0",    reportTestCases);
		nrOfFailedTestCases += CheckReject("1/0e10", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "division by zero rejected", "erational parse");
	}

	// ----- malformed input must be rejected -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckReject("",        reportTestCases);
		nrOfFailedTestCases += CheckReject("abc",     reportTestCases);
		nrOfFailedTestCases += CheckReject("/",       reportTestCases);
		nrOfFailedTestCases += CheckReject("1/",      reportTestCases);
		nrOfFailedTestCases += CheckReject("/2",      reportTestCases);
		nrOfFailedTestCases += CheckReject("1/2/3",   reportTestCases);
		nrOfFailedTestCases += CheckReject("3.14.15", reportTestCases);
		nrOfFailedTestCases += CheckReject("1e",      reportTestCases);
		nrOfFailedTestCases += CheckReject(".",       reportTestCases);
		nrOfFailedTestCases += CheckReject("42x",     reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "malformed reject", "erational parse");
	}

	// ----- negative-zero collapses to +0 across all input forms -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckParse("-0",      "0/1", reportTestCases);
		nrOfFailedTestCases += CheckParse("-0.0",    "0/1", reportTestCases);
		nrOfFailedTestCases += CheckParse("-0/5",    "0/1", reportTestCases);
		nrOfFailedTestCases += CheckParse("0/-5",    "0/1", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "negative-zero collapse", "erational parse");
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		erational p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> failbit on bad", "erational parse");
	}

	// ----- operator>> succeeds on a p/q token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("  -22/7  ");
		erational p;
		is >> p;
		std::ostringstream os; os << p;
		if (is.fail() || os.str() != "-22/7") ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> p/q", "erational parse");
	}

	// ----- operator>> on empty stream sets failbit -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		erational p;
		is >> p;
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> empty stream", "erational parse");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
