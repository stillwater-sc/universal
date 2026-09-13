// oracle.cpp: dfloat + - * / against exact decimal arithmetic on digit strings
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for #1484. dfloat computed with a significand of 4*ndigits + 8 bits, about
// 1.2*ndigits + 2 digits, but aligning exponents for + and - needs up to 2*ndigits
// digits, and the long division accumulated up to 2*ndigits as well. The overflow
// returned garbage even for decimal32 and decimal64:
//   decimal64  3.812837151747335e16 - 4304.704077704107 = -1298121021.550523
//   decimal32  1234567 / 1 = 8664.995
// The far-gap shortcut in + and - compared exponents instead of magnitudes, so dfloat<5>
// -88925 + 5.1e7 returned 5.1e7; and it returned the larger operand even when the
// smaller one changes a representable result, as across a power of ten: decimal64
// 1e16 - 6 gave 1e16. Division lost digits when the dividend's significand was the
// smaller one: decimal32 1 / 1234567 = 8e-7.
//
// The oracle here is independent of blockbinary: operands are read back from each
// dfloat as a digit string and an exponent, and the exact result is computed on digit
// strings. Every result must be the exact value rounded to ndigits significant digits,
// to nearest with ties to even (IEEE 754 roundTiesToEven, #1487). + and - may return
// the larger operand unchanged when the smaller one's leading digit is more than
// ndigits + 1 below it: that is then the nearest value, and keeps its encoding.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

namespace exact {

// non-negative integers as decimal digit strings, most significant digit first, no
// leading zeros ("0" for zero)
std::string trim(const std::string& a) {
	size_t i = a.find_first_not_of('0');
	return (i == std::string::npos) ? std::string("0") : a.substr(i);
}
int compare(const std::string& a, const std::string& b) {
	if (a.size() != b.size()) return (a.size() < b.size()) ? -1 : 1;
	return (a < b) ? -1 : (a > b ? 1 : 0);
}
std::string add(const std::string& a, const std::string& b) {
	std::string r;
	int carry = 0;
	for (size_t i = 0; i < std::max(a.size(), b.size()) || carry; ++i) {
		int s = carry;
		if (i < a.size()) s += a[a.size() - 1 - i] - '0';
		if (i < b.size()) s += b[b.size() - 1 - i] - '0';
		r += static_cast<char>('0' + s % 10);
		carry = s / 10;
	}
	std::reverse(r.begin(), r.end());
	return trim(r);
}
std::string sub(const std::string& a, const std::string& b) {   // requires a >= b
	std::string r;
	int borrow = 0;
	for (size_t i = 0; i < a.size(); ++i) {
		int d = (a[a.size() - 1 - i] - '0') - borrow - (i < b.size() ? b[b.size() - 1 - i] - '0' : 0);
		borrow = (d < 0) ? 1 : 0;
		r += static_cast<char>('0' + (d + 10) % 10);
	}
	std::reverse(r.begin(), r.end());
	return trim(r);
}
std::string mul(const std::string& a, const std::string& b) {
	std::vector<int> acc(a.size() + b.size(), 0);
	for (size_t i = 0; i < a.size(); ++i)
		for (size_t j = 0; j < b.size(); ++j)
			acc[i + j + 1] += (a[i] - '0') * (b[j] - '0');
	for (size_t k = acc.size() - 1; k > 0; --k) { acc[k - 1] += acc[k] / 10; acc[k] %= 10; }
	std::string r;
	for (int d : acc) r += static_cast<char>('0' + d);
	return trim(r);
}

// a signed decimal: (-1)^neg * digits * 10^exp
struct Value {
	bool neg = false;
	std::string digits = "0";
	int exp = 0;
	bool iszero() const { return digits == "0"; }
	int top() const { return exp + static_cast<int>(digits.size()) - 1; }   // position of the leading digit
};

// round to n significant digits, to nearest with ties to even, then drop trailing
// zeros. sticky says the exact value exceeds v by a nonzero amount below v's last digit;
// callers that set it keep a guard digit, so the deciding digit is always present.
Value round(Value v, unsigned n, bool sticky = false) {
	if (v.iszero()) return Value{};
	if (v.digits.size() > n) {
		const char round_digit = v.digits[n];
		sticky = sticky || v.digits.find_first_not_of('0', n + 1) != std::string::npos;
		v.exp += static_cast<int>(v.digits.size() - n);
		v.digits.resize(n);
		const bool odd = ((v.digits.back() - '0') & 1) != 0;
		if (round_digit > '5' || (round_digit == '5' && (sticky || odd))) {
			v.digits = add(v.digits, "1");
			if (v.digits.size() > n) { v.digits.pop_back(); ++v.exp; }   // 99...9 + 1: the dropped digit is 0
		}
	}
	while (v.digits.size() > 1 && v.digits.back() == '0') { v.digits.pop_back(); ++v.exp; }
	return v;
}

Value add(const Value& x, const Value& y, unsigned n) {
	if (x.iszero()) return round(y, n);
	if (y.iszero()) return round(x, n);
	// dfloat returns the larger operand when the smaller one's leading digit is more
	// than n + 1 below: it must then be the nearest value, which round() of the exact
	// sum would give too; returning it keeps the operand's own encoding
	const int gap = x.exp - y.exp, top_gap = x.top() - y.top();
	if (std::abs(gap) >= static_cast<int>(n) && std::abs(top_gap) >= static_cast<int>(n) + 2)
		return round(gap > 0 ? x : y, n);
	const int e = std::min(x.exp, y.exp);
	const std::string a = x.digits + std::string(static_cast<size_t>(x.exp - e), '0');
	const std::string b = y.digits + std::string(static_cast<size_t>(y.exp - e), '0');
	Value r;
	r.exp = e;
	if (x.neg == y.neg) { r.digits = add(a, b); r.neg = x.neg; }
	else if (compare(a, b) >= 0) { r.digits = sub(a, b); r.neg = x.neg; }
	else { r.digits = sub(b, a); r.neg = y.neg; }
	return round(r, n);
}

Value mul(const Value& x, const Value& y, unsigned n) {
	Value r;
	r.digits = mul(x.digits, y.digits);
	r.exp = x.exp + y.exp;
	r.neg = (x.neg != y.neg);
	return round(r, n);
}

// x / y rounded to n significant digits: schoolbook long division to n + 1 digits,
// with the remainder as the sticky part
Value div(const Value& x, const Value& y, unsigned n) {
	Value q;
	q.neg = (x.neg != y.neg);
	std::string rem = "0", quotient;
	int e = x.exp - y.exp;
	size_t next = 0;   // next digit of x.digits to bring down; zeros after it
	unsigned significant = 0;
	int scaled = 0;    // digits brought down beyond x.digits
	while (significant < n + 1) {
		const char d = (next < x.digits.size()) ? x.digits[next] : '0';
		if (next >= x.digits.size()) ++scaled;
		++next;
		rem = trim(rem + d);
		int qd = 0;
		while (compare(rem, y.digits) >= 0) { rem = sub(rem, y.digits); ++qd; }
		quotient += static_cast<char>('0' + qd);
		if (significant > 0 || qd > 0) ++significant;
	}
	// the quotient's last digit has weight 10^(e - scaled); earlier digits of x that
	// were brought down do not shift it
	q.digits = trim(quotient);
	q.exp = e - scaled;
	if (next < x.digits.size()) q.exp += static_cast<int>(x.digits.size() - next);
	// digits of x not yet brought down count as a nonzero remainder too
	const bool rest = (next < x.digits.size()) && x.digits.find_first_not_of('0', next) != std::string::npos;
	return round(q, n, rem != "0" || rest);
}

} // namespace exact

// the value a dfloat holds, as its stored significand and exponent
template<typename F>
exact::Value ValueOf(const F& v) {
	bool s; int e; typename F::significand_t sig;
	v.unpack(s, e, sig);
	exact::Value r;
	r.neg = s;
	r.digits = exact::trim(F::sig_to_string(sig));
	r.exp = e;
	if (r.iszero()) { r.neg = false; r.exp = 0; }
	return r;
}

exact::Value Stripped(exact::Value v) {
	while (v.digits.size() > 1 && v.digits.back() == '0') { v.digits.pop_back(); ++v.exp; }
	return v;
}

std::string ToString(const exact::Value& v) {
	return (v.neg ? "-" : "") + v.digits + "e" + std::to_string(v.exp);
}

bool SameValue(const exact::Value& x, const exact::Value& y) {
	if (x.iszero() && y.iszero()) return true;   // +0 == -0
	const exact::Value a = Stripped(x), b = Stripped(y);
	return a.neg == b.neg && a.digits == b.digits && a.exp == b.exp;
}

// random operands with 1 to N significant digits, exponents up to 4N + 5 apart (so
// both the aligned path and the far-gap path of + and - run), and both signs; every
// result must match the exact oracle
template<unsigned N, unsigned ES, DecimalEncoding E>
int VerifyAgainstOracle(int nrSamples, bool reportTestCases) {
	using F = dfloat<N, ES, E, std::uint32_t>;
	// deterministic: the engine's output sequence is specified by the standard
	std::mt19937_64 rng(1484 + N * 7 + static_cast<unsigned>(E));
	int nrOfFailedTests = 0;
	auto check = [&](const char* op, const F& a, const F& b, const F& got, const exact::Value& want) {
		const exact::Value g = ValueOf(got);
		if (!SameValue(g, want)) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 12) {
				std::cerr << "FAIL: dfloat<" << N << ',' << ES << "> " << ToString(ValueOf(a)) << ' ' << op << ' '
				          << ToString(ValueOf(b)) << " = " << ToString(g) << ", exact oracle " << ToString(want)
				          << '\n';
			}
		}
	};
	auto random_operand = [&](int base) {
		std::string sig;
		const unsigned len = 1 + static_cast<unsigned>(rng() % N);
		sig += static_cast<char>('1' + rng() % 9);
		for (unsigned i = 1; i < len; ++i) sig += static_cast<char>('0' + rng() % 10);
		const int e = base + static_cast<int>(rng() % (4 * N + 6)) - static_cast<int>(2 * N + 3);
		F v;
		v.assign(((rng() % 2) ? "-" : "") + sig + "e" + std::to_string(e));
		return v;
	};
	for (int k = 0; k < nrSamples; ++k) {
		// a common base exponent well inside the range, so results neither overflow nor underflow
		const int base = static_cast<int>(rng() % 11) - 5;
		const F a = random_operand(base), b = random_operand(base);
		const exact::Value x = ValueOf(a), y = ValueOf(b);
		exact::Value ny = y; ny.neg = !ny.neg;
		check("+", a, b, a + b, exact::add(x, y, N));
		check("-", a, b, a - b, exact::add(x, ny, N));
		check("*", a, b, a * b, exact::mul(x, y, N));
		if (!b.iszero()) check("/", a, b, a / b, exact::div(x, y, N));
	}
	return nrOfFailedTests;
}

// the cases reported in #1484
int VerifyReportedCases(bool reportTestCases) {
	using d32 = dfloat<7, 6, DecimalEncoding::BID, std::uint32_t>;
	using d64 = dfloat<16, 8, DecimalEncoding::BID, std::uint32_t>;
	using d5  = dfloat<5, 6, DecimalEncoding::BID, std::uint32_t>;
	int nrOfFailedTests = 0;
	auto expect = [&](const std::string& what, const exact::Value& value, const std::string& want) {
		const std::string got = ToString(Stripped(value));
		if (got != want) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << what << " = " << got << ", expected " << want << '\n';
		}
	};
	auto v32 = [](const char* s) { d32 v; v.assign(s); return v; };
	auto v64 = [](const char* s) { d64 v; v.assign(s); return v; };
	auto v5  = [](const char* s) { d5 v; v.assign(s); return v; };
	expect("decimal64 3812837151747335e1 - 4304704077704107e-12",
	       ValueOf(v64("3812837151747335e1") - v64("4304704077704107e-12")), "3812837151746905e1");
	expect("decimal64 3812837151747335e1 + 4304704077704107e-12",
	       ValueOf(v64("3812837151747335e1") + v64("4304704077704107e-12")), "3812837151747765e1");
	expect("decimal32 1234567 / 1", ValueOf(v32("1234567") / v32("1")), "1234567e0");
	expect("decimal32 9999999 / 0.001", ValueOf(v32("9999999") / v32("1e-3")), "9999999e3");
	expect("decimal32 1 / 1234567", ValueOf(v32("1") / v32("1234567")), "8100006e-13");
	expect("dfloat<5> -88925 + 5.1e7", ValueOf(v5("-88925") + v5("51e6")), "50911e3");
	expect("decimal64 1e16 - 6", ValueOf(v64("1e16") - v64("6")), "9999999999999994e0");
	expect("decimal64 1e16 - 1e-5", ValueOf(v64("1e16") - v64("1e-5")), "1e16");
	expect("decimal64 1e16 + 1e-5", ValueOf(v64("1e16") + v64("1e-5")), "1e16");
	expect("decimal32 1 - 1e-7", ValueOf(v32("1") - v32("1e-7")), "9999999e-7");
	expect("decimal32 1 - 1e-8", ValueOf(v32("1") - v32("1e-8")), "1e0");
	expect("decimal32 -1 + 1e-30", ValueOf(v32("-1") + v32("1e-30")), "-1e0");
	expect("decimal32 5 - 5e-9", ValueOf(v32("5") - v32("5e-9")), "5e0");
	expect("decimal32 5 - 5e-8", ValueOf(v32("5") - v32("5e-8")), "5e0");
	expect("dfloat<5> -88925 / 5.1e7", ValueOf(v5("-88925") / v5("51e6")), "-17436e-7");
	return nrOfFailedTests;
}

// The range limits (CodeRabbit on #1490), with expectations from Python's decimal module
// in the matching IEEE context (prec 7, Emax 96, Emin -95, clamp 1, ties to even). A value
// past emax folds down while its significand has room (1e91 is 10e90) and saturates to
// inf only when it cannot; a value below 10^emin keeps what it can at exponent emin,
// rounded there (gradual underflow), instead of flushing to 0.
int VerifyRangeLimits(bool reportTestCases) {
	using d32 = dfloat<7, 6, DecimalEncoding::BID, std::uint32_t>;
	int nrOfFailedTests = 0;
	auto show = [](const d32& v) -> std::string {
		if (v.isinf()) return v.sign() ? "-inf" : "inf";
		if (v.iszero()) return "0";
		return ToString(Stripped(ValueOf(v)));
	};
	auto expect = [&](const std::string& what, const d32& got, const std::string& want) {
		if (show(got) != want) {
			++nrOfFailedTests;
			if (reportTestCases)
				std::cerr << "FAIL: decimal32 " << what << " = " << show(got) << ", expected " << want << '\n';
		}
	};
	auto v = [](const char* s) { d32 x; x.assign(s); return x; };
	const struct { const char* txt; const char* want; } parses[] = {
		{ "1e91", "1e91" },           { "1e96", "1e96" },           { "9999999e90", "9999999e90" },
		{ "1e97", "inf" },            { "99999995e90", "inf" },     { "-1e96", "-1e96" },
		{ "15e-102", "2e-101" },      { "25e-102", "2e-101" },      { "35e-102", "4e-101" },
		{ "5e-102", "0" },            { "51e-103", "1e-101" },      { "4e-102", "0" },
		{ "1e-101", "1e-101" },       { "123456789e-104", "123457e-101" }, { "-15e-102", "-2e-101" },
	};
	for (const auto& c : parses) expect(std::string("\"") + c.txt + "\"", v(c.txt), c.want);
	const d32 maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
	expect("1e90 * 10", v("1e90") * v("10"), "1e91");
	expect("maxpos + 5e89", maxpos + v("5e89"), "inf");
	expect("maxpos + 4e89", maxpos + v("4e89"), "9999999e90");
	expect("maxpos * 1.000001", maxpos * v("1.000001"), "inf");
	expect("minpos / 2", minpos / v("2"), "0");
	expect("minpos * 0.6", minpos * v("0.6"), "1e-101");
	expect("minpos * 0.5", minpos * v("0.5"), "0");
	expect("3e-101 / 2", v("3e-101") / v("2"), "2e-101");
	expect("1e-100 - 9e-101", v("1e-100") - v("9e-101"), "1e-101");
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

	std::string test_suite  = "dfloat arithmetic against an exact decimal oracle (#1484)";
	std::string test_tag    = "oracle";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		dfloat<7, 6, DecimalEncoding::BID, std::uint32_t> a(1234567), b(1);
		std::cout << "decimal32 1234567 / 1 = " << (a / b) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyReportedCases(reportTestCases), "reported cases", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRangeLimits(reportTestCases), "decimal32 range limits", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<4, 6, DecimalEncoding::BID>(1000, reportTestCases),
	                                        "dfloat<4,6,BID>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<5, 6, DecimalEncoding::BID>(1000, reportTestCases),
	                                        "dfloat<5,6,BID>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<7, 6, DecimalEncoding::BID>(1000, reportTestCases),
	                                        "decimal32 BID", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<7, 6, DecimalEncoding::DPD>(1000, reportTestCases),
	                                        "decimal32 DPD", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<16, 8, DecimalEncoding::BID>(500, reportTestCases),
	                                        "decimal64 BID", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<16, 8, DecimalEncoding::DPD>(500, reportTestCases),
	                                        "decimal64 DPD", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<34, 12, DecimalEncoding::BID>(200, reportTestCases),
	                                        "decimal128 BID", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<34, 12, DecimalEncoding::DPD>(200, reportTestCases),
	                                        "decimal128 DPD", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<10, 8, DecimalEncoding::BID>(2000, reportTestCases),
	                                        "dfloat<10,8,BID>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<7, 6, DecimalEncoding::BID>(20000, reportTestCases),
	                                        "decimal32 BID 20k", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<16, 8, DecimalEncoding::BID>(5000, reportTestCases),
	                                        "decimal64 BID 5k", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstOracle<34, 12, DecimalEncoding::BID>(2000, reportTestCases),
	                                        "decimal128 BID 2k", test_tag);
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
