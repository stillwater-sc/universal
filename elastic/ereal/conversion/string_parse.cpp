// string_parse.cpp: regression tests for decimal-string parsing of ereal
//                  (Phase E of #835 -- nan/inf + operator>> hygiene;
//                   #857 -- coverage + trailing-garbage fix)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// ereal::parse accepts integer, decimal, scientific, and nan/inf literals;
// operator>> sets failbit on parse failure with an extraction guard.  Issue
// #857 also tightened the parse to reject trailing garbage after the
// mantissa or exponent ("1e", "1e3.5", "42x").
//
// Single-operator scope (issue #952): the operation under test is parse() /
// operator>> only. Comparisons project the parsed value to double; there is no
// ereal arithmetic in the test bodies, so no cross-operator contamination.

#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/ereal/ereal.hpp>
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

// Parse `input` and compare the parsed value (as a double) against `expected`
// with a tight tolerance.  ereal's leading limb is an IEEE double, so any
// finite value that fits in double precision must compare equal here.
template<unsigned N>
int CheckValue(const char* input, double expected, bool reportTestCases) {
	using namespace sw::universal;
	ereal<N> v;
	if (!parse(input, v)) {
		if (reportTestCases) std::cout << "FAIL parse rejected: '" << input << "'\n";
		return 1;
	}
	double got = static_cast<double>(v);
	// ereal's digit-accumulate path may drift sub-ulp from the exact value
	// for sub-1-ulp inputs; allow a small relative tolerance.
	double diff = std::fabs(got - expected);
	double tol  = std::fabs(expected) * 1e-12;
	if (tol < 1e-300) tol = 1e-300;
	if (diff > tol) {
		if (reportTestCases) {
			std::cout << "FAIL '" << input << "' got=" << got
			          << " expected=" << expected
			          << " diff=" << diff << '\n';
		}
		return 1;
	}
	return 0;
}

template<unsigned N>
int CheckReject(const char* input, bool reportTestCases) {
	using namespace sw::universal;
	ereal<N> v;
	if (parse(input, v)) {
		if (reportTestCases) {
			std::cout << "FAIL '" << input << "' should reject, got "
			          << static_cast<double>(v) << '\n';
		}
		return 1;
	}
	return 0;
}

template<unsigned N>
int CheckNaN(const char* input, bool reportTestCases) {
	using namespace sw::universal;
	ereal<N> v;
	if (!parse(input, v) || !v.isnan()) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' not nan\n";
		return 1;
	}
	return 0;
}

template<unsigned N>
int CheckInf(const char* input, bool expected_negative, bool reportTestCases) {
	using namespace sw::universal;
	ereal<N> v;
	if (!parse(input, v) || !v.isinf()) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' not inf\n";
		return 1;
	}
	if (std::signbit(static_cast<double>(v)) != expected_negative) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' wrong inf sign\n";
		return 1;
	}
	return 0;
}

}  // namespace

int main()
try {
	using namespace sw::universal;
	using Real = ereal<4>;

	std::string test_suite  = "ereal string parse (issues #835 Phase E, #857)";
	bool reportTestCases    = true;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- canonical decimals: assert parsed value, not just success -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckValue<4>("0",       0.0,    reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("1",       1.0,    reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("-1",     -1.0,    reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("+42",     42.0,   reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("-1000", -1000.0,  reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("3.14",    3.14,   reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("-3.25",  -3.25,   reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("0.001",   0.001,  reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "canonical decimals", "ereal parse");
	}

	// ----- scientific notation across +/-100 decimal exponent -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckValue<4>("1e0",     1.0,        reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("1e10",    1e10,       reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("1.5e2",   150.0,      reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("1.25e3",  1250.0,     reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("-3.14e2", -314.0,     reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("1e-10",   1e-10,      reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("1e100",   1e100,      reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("1e-100",  1e-100,     reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("1E+10",   1e10,       reportTestCases);  // capital E + explicit '+'
		ReportTestResult(nrOfFailedTestCases - start, "scientific notation", "ereal parse");
	}

	// ----- nan / inf / infinity token routing (case-insensitive, signed inf) -----
	{
		int start = nrOfFailedTestCases;
		for (const char* s : { "nan", "NaN", "NAN", "+nan", "-nan" }) {
			nrOfFailedTestCases += CheckNaN<4>(s, reportTestCases);
		}
		for (const char* s : { "inf", "Inf", "INF", "infinity", "Infinity", "INFINITY",
		                       "+inf", "+Infinity" }) {
			nrOfFailedTestCases += CheckInf<4>(s, false, reportTestCases);
		}
		for (const char* s : { "-inf", "-Inf", "-infinity", "-INFINITY" }) {
			nrOfFailedTestCases += CheckInf<4>(s, true, reportTestCases);
		}
		ReportTestResult(nrOfFailedTestCases - start, "nan/inf tokens", "ereal parse");
	}

	// ----- malformed input must be rejected -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckReject<4>("",        reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("  ",      reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("abc",     reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("1.2.3",   reportTestCases);
		nrOfFailedTestCases += CheckReject<4>(".",       reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("42x",     reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("0xFF",    reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("1 2",     reportTestCases);
		// Trailing 'e' with no digits, decimal point inside exponent:
		// both formerly accepted; #857 tightens the parse.
		nrOfFailedTestCases += CheckReject<4>("1e",      reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("1e3.5",   reportTestCases);
		// Almost-token forms that must NOT be misread as nan/inf
		nrOfFailedTestCases += CheckReject<4>("nann",    reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("innf",    reportTestCases);
		nrOfFailedTestCases += CheckReject<4>("infi",    reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "malformed reject", "ereal parse");
	}

	// ----- operator>> failbit on bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		Real p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> failbit on bad", "ereal parse");
	}

	// ----- operator>> success on a scientific token in whitespace -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("  3.14e2  ");
		Real p;
		is >> p;
		if (is.fail() || std::fabs(static_cast<double>(p) - 314.0) > 1e-12) {
			++nrOfFailedTestCases;
		}
		ReportTestResult(nrOfFailedTestCases - start, "operator>> scientific", "ereal parse");
	}

	// ----- operator>> on empty stream sets failbit -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		Real p;
		is >> p;
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> empty stream", "ereal parse");
	}

	// ----- block-width parity: ereal<1>, ereal<2>, ereal<4> match for the
	//       same input -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckValue<1>("3.14e2", 314.0, reportTestCases);
		nrOfFailedTestCases += CheckValue<2>("3.14e2", 314.0, reportTestCases);
		nrOfFailedTestCases += CheckValue<4>("3.14e2", 314.0, reportTestCases);
		nrOfFailedTestCases += CheckNaN<1>("nan", reportTestCases);
		nrOfFailedTestCases += CheckNaN<2>("nan", reportTestCases);
		nrOfFailedTestCases += CheckInf<1>("-inf", true, reportTestCases);
		nrOfFailedTestCases += CheckInf<2>("-inf", true, reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "nlimbs parity", "ereal parse");
	}

	// ----- #1006: high-precision parse (no ~16-digit cliff, no NaN on long input) -----
	{
		int start = nrOfFailedTestCases;
		// 1/7 = 0.142857... to 200 digits must parse to far more than double
		// precision: check 7*v == 1 to >100 digits (was capped near 16 before #1006).
		std::string oneSeventh = "0.";
		for (int i = 0; i < 200; ++i) oneSeventh += "142857"[i % 6];
		ereal<19> v;
		if (!parse(oneSeventh, v) || v.isnan()) {
			if (reportTestCases) std::cout << "FAIL 200-digit fractional parse failed/nan\n";
			++nrOfFailedTestCases;
		}
		else {
			double err = std::fabs(static_cast<double>(v * ereal<19>(7.0) - ereal<19>(1.0)));
			if (!(err < 1e-100)) {
				if (reportTestCases) std::cout << "FAIL 1/7 parse precision: 7*v-1 = " << err << " (want < 1e-100)\n";
				++nrOfFailedTestCases;
			}
		}
		// A ~440-digit string must still parse to a finite value (no NaN/crash).
		std::string longStr = "0.";
		for (int i = 0; i < 440; ++i) longStr += "142857"[i % 6];
		ereal<19> w;
		if (!parse(longStr, w) || w.isnan() || w.isinf()) {
			if (reportTestCases) std::cout << "FAIL 440-digit parse not finite\n";
			++nrOfFailedTestCases;
		}
		ReportTestResult(nrOfFailedTestCases - start, "high-precision fractional parse (#1006)", "ereal parse");
	}

	// ----- #1006: out-of-range exponents saturate to inf/zero, never NaN -----
	{
		int start = nrOfFailedTestCases;
		ereal<19> over;     // 1e400 > DBL_MAX -> +inf, NOT NaN
		if (!parse("1e400", over) || over.isnan() || !over.isinf() || std::signbit(static_cast<double>(over))) {
			if (reportTestCases) std::cout << "FAIL 1e400 should be +inf (not NaN)\n";
			++nrOfFailedTestCases;
		}
		ereal<19> negover;  // -1e400 -> -inf
		if (!parse("-1e400", negover) || negover.isnan() || !negover.isinf() || !std::signbit(static_cast<double>(negover))) {
			if (reportTestCases) std::cout << "FAIL -1e400 should be -inf (not NaN)\n";
			++nrOfFailedTestCases;
		}
		ereal<19> under;    // 1e-400 < DBL_MIN -> 0, NOT NaN
		if (!parse("1e-400", under) || under.isnan() || !under.iszero()) {
			if (reportTestCases) std::cout << "FAIL 1e-400 should be zero (not NaN)\n";
			++nrOfFailedTestCases;
		}
		ReportTestResult(nrOfFailedTestCases - start, "out-of-range exponent saturation (#1006)", "ereal parse");
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
