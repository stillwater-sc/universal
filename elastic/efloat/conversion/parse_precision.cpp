// parse_precision.cpp: regression tests for high-precision efloat parse().
//
// Guards issue #1141: parse() silently returned ~0 once the requested precision
// approached the d2b working budget (the convert reduction's internal shift
// overflowed BigBits). parse() now sizes its target overflow-safely, so a
// high-precision request yields a correctly-rounded value; and parse<BigBits>
// with a larger budget reaches arbitrary precision.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	int VerifyEfloatParsePrecision(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		const std::string pi_short = "3.141592653589793";
		const std::string pi_long  =
			"3.14159265358979323846264338327950288419716939937510582097494459230781"
			"640628620899862803482534211706798214808651328230664709384460955058223";
		const double PI = std::acos(-1.0);

		// ---------------------------------------------------------------------
		// 1. #1141 guard: default parse() must return the correct value (not ~0)
		//    at every requested precision, for short and long literals.
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying default parse() is correct at high precision...\n";
		{
			for (unsigned prec : { 256u, 512u, 2040u, 2048u, 3400u }) {
				efloat<128> a; a.set_precision(prec);
				efloat<128> b; b.set_precision(prec);
				bool oka = parse(pi_short, a);
				bool okb = parse(pi_long, b);
				if (!oka || std::abs(double(a) - PI) > 1e-10) { if (reportTestCases) std::cout << "    FAIL: parse(pi_short)@" << prec << " = " << double(a) << "\n"; ++failures; }
				if (!okb || std::abs(double(b) - PI) > 1e-10) { if (reportTestCases) std::cout << "    FAIL: parse(pi_long)@"  << prec << " = " << double(b) << "\n"; ++failures; }
			}
		}

		// ---------------------------------------------------------------------
		// 2. parse<BigBits> reaches high precision. Independent check via the
		//    exact-in-decimal identity 0.1 * 10 == 1: parse<16384>("0.1") to
		//    ~3300 bits, times 10, must equal 1 to ~3200 bits.
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying parse<16384> high precision (0.1*10==1)...\n";
		{
			efloat<128> tenth; tenth.set_precision(3300);
			bool ok = parse<16384>("0.1", tenth);
			efloat<128> d = tenth * efloat<128>(10.0) - efloat<128>(1.0); d.setsign(false);
			int64_t sc = d.iszero() ? -1000000 : d.scale();
			if (!ok || sc > -3200) { if (reportTestCases) std::cout << "    FAIL: parse<16384>(0.1)*10-1 scale=" << sc << " (want <= -3200)\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 3. More digits -> more accuracy: parse<16384> of the 130-digit pi must
		//    agree with an independent high-precision reference (efloat_pi) to far
		//    more than double precision -- assert actual accuracy, not the
		//    configured limb capacity. The 130-digit literal caps agreement at
		//    ~430 bits, so require >= 400-bit agreement.
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying parse<16384> accuracy of a long literal...\n";
		{
			efloat<128> hp; hp.set_precision(3300);
			bool ok = parse<16384>(pi_long, hp);
			efloat<128> d = hp - efloat_pi<128>(); d.setsign(false);
			int64_t sc = d.iszero() ? -1000000 : d.scale();
			if (!ok || std::abs(double(hp) - PI) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: parse<16384>(pi_long) = " << double(hp) << "\n"; ++failures; }
			if (sc > -400) { if (reportTestCases) std::cout << "    FAIL: parse<16384>(pi_long) accuracy scale=" << sc << " (want <= -400)\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 4. Large POSITIVE exponents (5^E growth). The default budget cannot
		//    hold 5^1000, so parse must FAIL (not return garbage); a large budget
		//    succeeds and yields the correct magnitude (~10^1000 = 2^3321.9).
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying large positive-exponent handling...\n";
		{
			efloat<128> big;
			if (parse("1e1000", big)) { if (reportTestCases) std::cout << "    FAIL: parse(1e1000) default budget did not fail\n"; ++failures; }
			efloat<128> big2; big2.set_precision(3400);
			bool ok = parse<16384>("1e1000", big2);
			if (!ok || big2.iszero() || big2.isinf() || big2.isnan()) { if (reportTestCases) std::cout << "    FAIL: parse<16384>(1e1000) not a finite value\n"; ++failures; }
			// 10^1000 = 2^(1000*log2(10)) ~ 2^3321.9, so scale() (MSB exponent) ~ 3321
			if (ok && (big2.scale() < 3315 || big2.scale() > 3325)) { if (reportTestCases) std::cout << "    FAIL: parse<16384>(1e1000) scale=" << big2.scale() << " (want ~3321)\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 4. Special tokens and error handling still work.
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying tokens / error handling...\n";
		{
			efloat<8> v;
			if (!parse("nan", v) || !v.isnan())          { if (reportTestCases) std::cout << "    FAIL: parse(nan)\n"; ++failures; }
			if (!parse("inf", v) || !v.isinf())          { if (reportTestCases) std::cout << "    FAIL: parse(inf)\n"; ++failures; }
			if (!parse("0.0", v) || !v.iszero())         { if (reportTestCases) std::cout << "    FAIL: parse(0.0)\n"; ++failures; }
			if (parse("not-a-number", v))                { if (reportTestCases) std::cout << "    FAIL: parse(garbage) accepted\n"; ++failures; }
		}

		return failures;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat parse precision library";
	std::string test_tag            = "parse-precision";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatParsePrecision(reportTestCases), "efloat", "parse precision");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
