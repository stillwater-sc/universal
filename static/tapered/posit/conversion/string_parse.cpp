// string_parse.cpp: regression tests for decimal-string parsing of posit
//                  (Phase B2b of #835)
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
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_reporters.hpp>

// RAII guard: redirects std::cerr to a throwaway sink for the scope of a
// test block, and restores the original buffer in the destructor so the
// stream stays consistent even if the body throws. We use an
// ostringstream-backed sink rather than nullptr -- a nullptr buffer can
// leave std::cerr in a badbit state after writes, which would persist
// across subsequent unrelated tests.
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

	std::string test_suite  = "posit decimal string parse (Phase B2b of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- small posit: bit-exact match against double constructor -----
	// For posit<32,2> and below the legacy double-precision parse path
	// already had enough precision, so the new path must produce identical
	// results.
	{
		int start = nrOfFailedTestCases;
		using Posit = posit<32, 2>;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "0",                 0.0          },
			{ "0.0",               0.0          },
			{ "-0.0",              0.0          },
			{ "1.0",               1.0          },
			{ "-1.0",             -1.0          },
			{ "2.0",               2.0          },
			{ "0.5",               0.5          },
			{ "3.14",              3.14         },
			{ "3.141592653589793", 3.141592653589793 },
			{ "1.5",               1.5          },
			{ "-3.25",            -3.25         },
			{ "1.25e3",            1250.0       },
			{ "1e0",               1.0          },
			{ "1e10",              1e10         },
			{ "1e-10",             1e-10        },
			{ "1234567.8901234",   1234567.8901234 },
		};
		for (const auto& c : cases) {
			Posit ours, ref(c.v);
			if (!parse(c.s, ours)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << c.s << '\n';
				continue;
			}
			if (ours != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  mismatch on \"" << c.s << "\":\n"
					          << "    string  -> " << to_binary(ours) << " = " << double(ours) << '\n'
					          << "    double  -> " << to_binary(ref)  << " = " << double(ref)  << '\n';
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit<32,2> decimal vs double constructor\n";
	}

	// ----- narrow posit also round-trips -----
	{
		int start = nrOfFailedTestCases;
		using Posit = posit<16, 2>;
		Posit a, b;
		if (!parse("2.5", a)) ++nrOfFailedTestCases;
		b = 2.5;
		if (a != b) ++nrOfFailedTestCases;
		if (!parse("-0.125", a)) ++nrOfFailedTestCases;
		b = -0.125;
		if (a != b) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit<16,2> basic decimals\n";
	}

	// ----- native posit hex format still works -----
	{
		int start = nrOfFailedTestCases;
		posit<8, 0> p, expected;
		if (!parse("8.0x40p", p)) ++nrOfFailedTestCases;
		expected = 1.0;
		if (p != expected) ++nrOfFailedTestCases;
		// mismatched nbits.es must fail
		if (parse("16.2x40p", p)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: native posit hex format\n";
	}

	// ----- nan / inf literals route to NaR -----
	{
		int start = nrOfFailedTestCases;
		posit<32, 2> p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan",
		                       "inf", "Inf", "+inf", "-inf",
		                       "infinity", "INFINITY" }) {
			p.setzero();
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isnar())   ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: nan/inf -> NaR routing\n";
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
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: operator>> bad-token handling\n";
	}

	// ----- wide posit: decimal-string parse must MATCH bits delivered by
	//       the double-constructor route for inputs that ARE EXACT in
	//       double. Inputs that aren't exact (e.g. 1e-6, 0.1) will legally
	//       diverge -- posit<64,3> has ~56 fraction bits, more than double's
	//       52, so the new path captures precision the old double-funnel
	//       lost. Those cases belong in the precision-win check below.
	{
		int start = nrOfFailedTestCases;
		using Posit = posit<64, 3>;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "1.0",              1.0    },
			{ "-2.5",            -2.5    },
			{ "0.25",             0.25   },
			{ "100",              100.0  },
			{ "1024",             1024.0 },
			{ "0.0625",           0.0625 },  // 2^-4
		};
		for (const auto& c : cases) {
			Posit ours, ref(c.v);
			if (!parse(c.s, ours)) ++nrOfFailedTestCases;
			if (ours != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  mismatch on \"" << c.s << "\":\n"
					          << "    string  -> " << to_binary(ours) << '\n'
					          << "    double  -> " << to_binary(ref)  << '\n';
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit<64,3> sanity (exact-in-double inputs)\n";
	}

	// ----- precision-win demonstration: pin the expected posit<64,3> bit
	//       pattern for "1e-6" via the new high-precision path. The legacy
	//       double-funnel result is also computed for comparison; the two
	//       must differ (because posit<64,3> has ~56 fraction bits, more
	//       than double's 52). Pinning the exact bit pattern means any
	//       future rounding regression in the new path is caught directly,
	//       not just by inequality.
	{
		int start = nrOfFailedTestCases;
		using Posit = posit<64, 3>;
		Posit via_string, via_double(1e-6);
		if (!parse("1e-6", via_string)) ++nrOfFailedTestCases;
		// Expected bit pattern of the high-precision parse for 1e-6 in
		// posit<64,3>: sign=0, regime=0001, exp=100, fraction=56 bits.
		const std::string expected_bits =
		    "0b0.0001.100.00001100011011110111101000001011010111101101100011010011";
		if (to_binary(via_string) != expected_bits) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cout << "  pinned bit mismatch for 1e-6 in posit<64,3>:\n"
				          << "    got      " << to_binary(via_string) << '\n'
				          << "    expected " << expected_bits          << '\n';
			}
		}
		if (via_string == via_double) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cout << "  expected precision divergence for 1e-6 in posit<64,3>; got identity\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit<64,3> precision-win\n";
	}

	// ----- wide posit precision win: a value with more decimal digits
	//       than double can hold should still parse to a defined value.
	//       We don't compare against an oracle here -- the comparison
	//       framework for that lives in the future Phase C tooling --
	//       but we exercise the code path on a wide configuration.
	{
		int start = nrOfFailedTestCases;
		posit<128, 4> p;
		if (!parse("3.14159265358979323846264338327950288419716939937510", p))
			++nrOfFailedTestCases;
		if (p.iszero())          ++nrOfFailedTestCases;
		if (p.isnar())           ++nrOfFailedTestCases;
		if (sign(p))             ++nrOfFailedTestCases;  // positive
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: posit<128,4> wide decimal\n";
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
