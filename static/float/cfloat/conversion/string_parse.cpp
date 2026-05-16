// string_parse.cpp: regression tests for decimal-string parsing of cfloat
//                  (Phase B2c of #835)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <sstream>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat decimal string parse (Phase B2c of #835)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- half: bit-exact match against cfloat constructor -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "0",      0.0 },
			{ "0.0",    0.0 },
			{ "1.0",    1.0 },
			{ "-1.0", -1.0 },
			{ "1.5",    1.5 },
			{ "-3.25", -3.25 },
			{ "2.0",    2.0 },
			{ "0.5",    0.5 },
			{ "1.25e3", 1250.0 },
			{ "65504",  65504.0 },  // maxpos for half
		};
		for (const auto& c : cases) {
			half ours, ref(c.v);
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
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: half (cfloat<16,5>) decimal vs constructor\n";
	}

	// ----- single (IEEE float): match the float constructor for exact-in-float values -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; float v; };
		Case cases[] = {
			{ "0",       0.0f },
			{ "1.0",     1.0f },
			{ "-1.0",   -1.0f },
			{ "0.5",     0.5f },
			{ "0.25",    0.25f },
			{ "100",     100.0f },
			{ "1e10",    1e10f },
			{ "1.5",     1.5f },
		};
		for (const auto& c : cases) {
			single ours, ref(c.v);
			if (!parse(c.s, ours)) ++nrOfFailedTestCases;
			if (ours != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  mismatch on \"" << c.s << "\":\n"
					          << "    string -> " << to_binary(ours) << '\n'
					          << "    ref    -> " << to_binary(ref)  << '\n';
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: single (cfloat<32,8>)\n";
	}

	// ----- duble (IEEE double): match the double constructor for exact inputs -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "1.0",        1.0 },
			{ "-2.5",      -2.5 },
			{ "0.25",       0.25 },
			{ "100",        100.0 },
			{ "1024",       1024.0 },
			{ "0.0625",     0.0625 },  // 2^-4
			{ "4503599627370496",   4503599627370496.0 },  // 2^52
		};
		for (const auto& c : cases) {
			duble ours, ref(c.v);
			if (!parse(c.s, ours)) ++nrOfFailedTestCases;
			if (ours != ref) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: duble (cfloat<64,11>) exact-in-double\n";
	}

	// ----- nan / inf literals route to the corresponding encodings -----
	{
		int start = nrOfFailedTestCases;
		single p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan" }) {
			p.setzero();
			if (!parse(s, p))      ++nrOfFailedTestCases;
			if (!p.isnan())        ++nrOfFailedTestCases;
		}
		for (const char* s : { "inf", "Inf", "infinity", "INFINITY" }) {
			p.setzero();
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isinf())   ++nrOfFailedTestCases;
			if (p.sign())     ++nrOfFailedTestCases;  // positive
		}
		for (const char* s : { "-inf", "-Inf", "-infinity" }) {
			p.setzero();
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.isinf())   ++nrOfFailedTestCases;
			if (!p.sign())    ++nrOfFailedTestCases;  // negative
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: nan/inf token routing\n";
	}

	// ----- native cfloat hex format still works (with and without 0x prefix) -----
	{
		int start = nrOfFailedTestCases;
		half p;
		if (!parse("16.5x3C00c", p))   ++nrOfFailedTestCases;
		if (p != half(1.0))            ++nrOfFailedTestCases;
		if (!parse("16.5x0x3C00c", p)) ++nrOfFailedTestCases;
		if (p != half(1.0))            ++nrOfFailedTestCases;
		// mismatched nbits.es must fail
		if (parse("32.8x12345678c", p)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: native cfloat hex format\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		single p;
		std::streambuf* oldbuf = std::cerr.rdbuf(nullptr);
		is >> p;
		std::cerr.rdbuf(oldbuf);
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: operator>> bad-token failbit\n";
	}

	// ----- trailing junk is rejected -----
	{
		int start = nrOfFailedTestCases;
		single p;
		if (parse("1.5abc", p)) ++nrOfFailedTestCases;
		if (parse("3.14xyz", p)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: trailing-junk rejection\n";
	}

	// ----- signed zero -----
	{
		int start = nrOfFailedTestCases;
		half p;
		if (!parse("0", p)) ++nrOfFailedTestCases;
		if (p.sign())       ++nrOfFailedTestCases;  // +0
		if (!parse("-0", p)) ++nrOfFailedTestCases;
		if (!p.sign())      ++nrOfFailedTestCases;  // -0
		if (!parse("-0.0", p)) ++nrOfFailedTestCases;
		if (!p.sign())      ++nrOfFailedTestCases;  // -0.0
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: signed zero handling\n";
	}

	// ----- precision-win check: a value that's not exact in double should
	//       round differently in cfloat<128,15> (quad, ~112 fbits) vs the
	//       legacy double funnel.
	{
		int start = nrOfFailedTestCases;
		quad via_string, via_double(1e-6);
		if (!parse("1e-6", via_string)) ++nrOfFailedTestCases;
		if (via_string == via_double) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "  expected precision divergence for 1e-6 in cfloat<128,15>\n";
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: quad precision-win\n";
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
