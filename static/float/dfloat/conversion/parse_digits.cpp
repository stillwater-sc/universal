// parse_digits.cpp: dfloat parse counts significant digits, not characters
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for #1485. assign() / parse() kept the first ndigits DIGIT CHARACTERS as
// the significand, so leading zeros used up the precision, and digits past the
// precision in the fraction still moved the exponent:
//   decimal32  "0.0001234567"            -> 1.23e-8     (1.234567e-4)
//   decimal32  "0000000123"              -> 0           (123)
//   decimal32  "1.23456789"              -> 0.01234567  (1.234567)
//   decimal32  "12345678.9"              -> 1234567     (1.234567e7)
//   decimal64  "3.14159265358979323846"  -> 3.141592653589793e-5
// and an exponent of ten or more digits overflowed an int (undefined behaviour).
//
// Each value here is a random digit string D and an exponent E. It is written out in
// several textual forms (positional, with leading and trailing zeros, with the point
// shifted against an explicit exponent, scientific) and every form must parse to D * 10^E
// truncated to ndigits significant digits, computed on the digit strings.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// D * 10^E with D a digit string without leading zeros, truncated to n significant
// digits and stripped of trailing zeros, as "digits e exponent"
std::string Truncated(std::string d, int e, unsigned n) {
	if (d.size() > n) { e += static_cast<int>(d.size() - n); d.resize(n); }
	while (d.size() > 1 && d.back() == '0') { d.pop_back(); ++e; }
	return d + "e" + std::to_string(e);
}

// the value a dfloat holds, in the same form
template<typename F>
std::string Held(const F& v) {
	bool s; int e; typename F::significand_t sig;
	v.unpack(s, e, sig);
	std::string d = F::sig_to_string(sig);
	while (d.size() > 1 && d.back() == '0') { d.pop_back(); ++e; }
	return std::string(s ? "-" : "") + d + "e" + std::to_string(e);
}

// D * 10^E written with a decimal point and no exponent
std::string Positional(const std::string& d, int e) {
	const int len = static_cast<int>(d.size());
	if (e >= 0) return d + std::string(static_cast<size_t>(e), '0');
	const int point = len + e;   // digits before the point
	if (point <= 0) return "0." + std::string(static_cast<size_t>(-point), '0') + d;
	return d.substr(0, static_cast<size_t>(point)) + "." + d.substr(static_cast<size_t>(point));
}

template<unsigned N, unsigned ES, DecimalEncoding E>
int VerifyParseForms(int nrSamples, bool reportTestCases) {
	using F = dfloat<N, ES, E, std::uint32_t>;
	std::mt19937_64 rng(1485 + N);   // deterministic: the engine's output sequence is specified by the standard
	int nrOfFailedTests = 0;
	for (int k = 0; k < nrSamples; ++k) {
		// 1 to 3N digits: many past the precision
		std::string d;
		d += static_cast<char>('1' + rng() % 9);
		const unsigned len = 1 + static_cast<unsigned>(rng() % (3 * N));
		for (unsigned i = 1; i < len; ++i) d += static_cast<char>('0' + rng() % 10);
		// leading digit within +/-(N + 12) decades of 1, well inside every configuration's range
		const int top = static_cast<int>(rng() % (2 * N + 25)) - static_cast<int>(N + 12);
		const int e = top - static_cast<int>(len) + 1;
		const bool neg = (k % 2) != 0;
		const std::string sign = neg ? "-" : "";
		const std::string want = sign + Truncated(d, e, N);

		const unsigned lz = static_cast<unsigned>(rng() % 5), tz = static_cast<unsigned>(rng() % 5);
		const int shift = static_cast<int>(rng() % 9) - 4;
		std::string pos = Positional(d, e);
		std::string forms[] = {
			sign + pos,                                                                   // plain positional
			sign + std::string(lz, '0') + pos,                                            // leading integer zeros
			sign + pos + (pos.find('.') == std::string::npos ? "." : "") + std::string(tz, '0'),   // trailing fraction zeros
			sign + Positional(d, e - shift) + "e" + std::to_string(shift),                // point shifted against an exponent
			sign + d.substr(0, 1) + (len > 1 ? "." + d.substr(1) : "") + "e" + std::to_string(top),   // scientific
			sign + d + "e" + std::to_string(e),                                           // integer significand
		};
		for (const std::string& txt : forms) {
			F a; a.assign(txt);
			F p; const bool ok = parse(txt, p);
			const std::string got = Held(a);
			if (got != want || !ok || !(p == a)) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 12) {
					std::cerr << "FAIL: dfloat<" << N << ',' << ES << "> \"" << txt << "\" -> " << got << ", expected " << want
					          << (ok ? "" : " (parse() rejected it)") << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// 1.234567 * 10^-k in positional form, k = 0..20, the round trip #1485 proposed
template<unsigned N, unsigned ES>
int VerifyNegativePowers(bool reportTestCases) {
	using F = dfloat<N, ES, DecimalEncoding::BID, std::uint32_t>;
	int nrOfFailedTests = 0;
	for (int k = 0; k <= 20; ++k) {
		const std::string txt = Positional("1234567", -6 - k);
		F positional; positional.assign(txt);
		F scientific; scientific.assign("1.234567e-" + std::to_string(k));
		const std::string want = Truncated("1234567", -6 - k, N);
		if (Held(positional) != want || !(positional == scientific)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: dfloat<" << N << ',' << ES << "> \"" << txt << "\" -> " << Held(positional) << ", expected " << want << '\n';
		}
	}
	return nrOfFailedTests;
}

// the cases reported in #1485, and exponents too long for an int
int VerifyReportedCases(bool reportTestCases) {
	using d32 = dfloat<7, 6, DecimalEncoding::BID, std::uint32_t>;
	using d64 = dfloat<16, 8, DecimalEncoding::BID, std::uint32_t>;
	using d4  = dfloat<4, 6, DecimalEncoding::BID, std::uint32_t>;
	int nrOfFailedTests = 0;
	auto expect = [&](const std::string& what, const std::string& got, const std::string& want) {
		if (got != want) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << what << " -> " << got << ", expected " << want << '\n';
		}
	};
	auto p32 = [](const char* s) { d32 v; v.assign(s); return Held(v); };
	auto p64 = [](const char* s) { d64 v; v.assign(s); return Held(v); };
	auto p4  = [](const char* s) { d4 v; v.assign(s); return Held(v); };
	expect("decimal32 \"0.0001234567\"", p32("0.0001234567"), "1234567e-10");
	expect("decimal32 \"0.000001234567\"", p32("0.000001234567"), "1234567e-12");
	expect("decimal32 \"0000000123\"", p32("0000000123"), "123e0");
	expect("decimal32 \"1.23456789\"", p32("1.23456789"), "1234567e-6");
	expect("decimal32 \"12345678.9\"", p32("12345678.9"), "1234567e1");
	expect("decimal64 \"0.00000000001234567890123456\"", p64("0.00000000001234567890123456"), "1234567890123456e-26");
	expect("decimal64 \"3.14159265358979323846\"", p64("3.14159265358979323846"), "3141592653589793e-15");
	expect("dfloat<4> \"-0.09982\"", p4("-0.09982"), "-9982e-5");
	expect("dfloat<4> \"0.00004155\"", p4("0.00004155"), "4155e-8");
	// exponents of ten or more digits saturate instead of overflowing an int
	d32 big; big.assign("1e99999999999");
	d32 tiny; tiny.assign("1e-99999999999");
	if (!big.isinf() || !tiny.iszero()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: 1e+-99999999999 -> " << big << ", " << tiny << '\n';
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dfloat parse of significant digits (#1485)";
	std::string test_tag    = "parse digits";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		dfloat<7, 6, DecimalEncoding::BID, std::uint32_t> v;
		v.assign("0.0001234567");
		std::cout << "decimal32 \"0.0001234567\" = " << v << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyReportedCases(reportTestCases), "reported cases", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNegativePowers<7, 6>(reportTestCases), "decimal32 1.234567e-k positional", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNegativePowers<16, 8>(reportTestCases), "decimal64 1.234567e-k positional", test_tag);
	// a narrow width, where 1.234567 has more digits than the precision
	nrOfFailedTestCases += ReportTestResult(VerifyNegativePowers<4, 6>(reportTestCases), "dfloat<4,6> 1.234567e-k positional", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<4, 6, DecimalEncoding::BID>(500, reportTestCases), "dfloat<4,6,BID> forms", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<7, 6, DecimalEncoding::BID>(500, reportTestCases), "decimal32 BID forms", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<7, 6, DecimalEncoding::DPD>(500, reportTestCases), "decimal32 DPD forms", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<16, 8, DecimalEncoding::BID>(500, reportTestCases), "decimal64 BID forms", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<16, 8, DecimalEncoding::DPD>(500, reportTestCases), "decimal64 DPD forms", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<34, 12, DecimalEncoding::BID>(500, reportTestCases), "decimal128 BID forms", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<34, 12, DecimalEncoding::DPD>(500, reportTestCases), "decimal128 DPD forms", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<10, 8, DecimalEncoding::BID>(2000, reportTestCases), "dfloat<10,8,BID> forms", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<7, 6, DecimalEncoding::BID>(20000, reportTestCases), "decimal32 BID forms 20k", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyParseForms<16, 8, DecimalEncoding::BID>(20000, reportTestCases), "decimal64 BID forms 20k", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
