// string_parse.cpp: regression tests for decimal-string parsing of hfloat
//                  (Resolves issue #849)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// hfloat is IBM System/360 hexadecimal floating-point. Notable constraints:
//   - No NaN or Inf encoding: nan / inf / infinity tokens are rejected
//   - Truncation rounding only (no round-up at the cut)
//   - Overflow saturates to maxpos / maxneg
//   - Wobbling precision: 0-3 leading zero bits in the MSB hex digit

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/hfloat/hfloat.hpp>
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

	std::string test_suite  = "hfloat decimal string parse (issue #849)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- hfloat_short: canonical decimals match constructor -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "0",     0.0   },
			{ "1.0",   1.0   },
			{ "-1.0", -1.0   },
			{ "1.5",   1.5   },
			{ "-3.25", -3.25 },
			{ "0.5",   0.5   },
			{ "16.0",  16.0  },  // exact in hex-float (one hex digit)
			{ "256.0", 256.0 },
			{ "1.25e3", 1250.0 },
		};
		for (const auto& c : cases) {
			hfloat_short ours, ref(c.v);
			if (!parse(c.s, ours)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << c.s << '\n';
				continue;
			}
			if (ours != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  mismatch on \"" << c.s << "\":\n"
					          << "    string -> " << double(ours) << '\n'
					          << "    ref    -> " << double(ref)  << '\n';
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat_short canonical decimals\n";
	}

	// ----- hfloat_long: canonical decimals match constructor -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "1.0",        1.0      },
			{ "-2.5",      -2.5      },
			{ "0.25",       0.25     },
			{ "100",        100.0    },
			{ "1024",       1024.0   },
			{ "0.0625",     0.0625   },  // 2^-4 = 16^-1, exact
			{ "1e10",       1e10     },
		};
		for (const auto& c : cases) {
			hfloat_long ours, ref(c.v);
			if (!parse(c.s, ours)) ++nrOfFailedTestCases;
			if (ours != ref)       ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat_long canonical decimals\n";
	}

	// ----- nan / inf tokens are REJECTED (hfloat has no NaN/Inf encoding) -----
	{
		int start = nrOfFailedTestCases;
		hfloat_short p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan",
		                       "inf", "Inf", "infinity", "INFINITY",
		                       "+inf", "-inf", "-infinity" }) {
			if (parse(s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly accepted: \"" << s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat nan/inf rejection\n";
	}

	// ----- overflow saturates to maxpos / maxneg (no Inf) -----
	{
		int start = nrOfFailedTestCases;
		hfloat_short p, maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
		// hfloat_short max is 16^63 ~ 7.24e75. Use a value well above that.
		if (!parse("1e100", p)) ++nrOfFailedTestCases;
		if (p != maxpos)        ++nrOfFailedTestCases;
		if (!parse("-1e100", p)) ++nrOfFailedTestCases;
		if (p != maxneg)         ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat overflow saturation\n";
	}

	// ----- malformed input is rejected -----
	{
		int start = nrOfFailedTestCases;
		hfloat_short p;
		for (const char* s : { "", "+", "-", ".", "e10",
		                       "not-a-number", "abc", "1.5abc",
		                       "1.5.0", "1e", "1ea" }) {
			if (parse(s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly accepted: \"" << s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat malformed input rejection\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		hfloat_short p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat operator>> failbit\n";
	}

	// ----- operator>> handles EOF (empty stream) without crashing -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		hfloat_short p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat operator>> EOF\n";
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
