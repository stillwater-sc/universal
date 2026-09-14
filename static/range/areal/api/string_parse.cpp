// string_parse.cpp: text to areal -- parse(), assign() and operator>>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

/*
   std::cin >> someAreal never worked (#1454): the friend declaration took a mutable
   reference, the definition a const one, so extraction was a link error, and the definition
   would not have compiled anyway. assign(std::string) was a stub that printed "assign TBD".

   parse() now reads the three forms an areal is written in: a decimal, the exact form [d],
   and the uncertain form (d, dnext) that operator<< writes. assign() and operator>> use it.

   The reference is exact: every case is built from exact binary values, and their decimal
   text is their exact decimal expansion. For each pair of adjacent exact values v < w:
   - exact(v) is v with the ubit clear; exact(v) with a digit appended far past double
     precision, and the exact midpoint of v and w, are v with the ubit set;
   - [midpoint] is the even one of v and w, [(3v+w)/4] is v, [(v+3w)/4] is w;
   - (exact(v), exact(w)) and (exact(w), exact(v)) are v with the ubit set;
   and every encoding round-trips through the text operator<< writes, through parse(),
   assign() and operator>>. No case lies between maxpos + ulp and the next power of two,
   where the conversion parse() shares with arithmetic does not saturate yet (#1503): past
   maxpos the only case is maxpos nudged away, inside the interval (maxpos, inf). The top
   binade of an 11-bit exponent lies past double, which operator<< and the reference both go
   through, so those values are skipped.
*/

namespace sw { namespace universal {

namespace string_parse_test {

// the exact decimal text of (-1)^negative * m * 2^k
inline std::string ExactDecimal(bool negative, std::uint64_t m, int k) {
	std::vector<int> d;  // decimal digits, least significant first
	if (m == 0) d.push_back(0);
	while (m != 0) {
		d.push_back(static_cast<int>(m % 10));
		m /= 10;
	}
	auto times = [&d](int f) {
		int carry = 0;
		for (auto& x : d) {
			const int y = x * f + carry;
			x = y % 10;
			carry = y / 10;
		}
		while (carry != 0) {
			d.push_back(carry % 10);
			carry /= 10;
		}
	};
	int fractionDigits = 0;
	if (k >= 0) {
		for (int i = 0; i < k; ++i) times(2);
	}
	else {
		for (int i = 0; i < -k; ++i) times(5);  // m * 2^k = m * 5^-k / 10^-k
		fractionDigits = -k;
	}
	std::string s;
	for (auto it = d.rbegin(); it != d.rend(); ++it) s.push_back(static_cast<char>('0' + *it));
	if (fractionDigits > 0) {
		while (static_cast<int>(s.size()) <= fractionDigits) s.insert(s.begin(), '0');
		s.insert(s.end() - fractionDigits, '.');
	}
	return (negative ? "-" : "") + s;
}

// the exact decimal text of a finite double
inline std::string ExactDecimal(double x) {
	if (x == 0.0) return std::signbit(x) ? "-0" : "0";
	int e = 0;
	const double f = std::frexp(std::fabs(x), &e);  // |x| = f * 2^e, f in [0.5, 1)
	return ExactDecimal(std::signbit(x), static_cast<std::uint64_t>(std::ldexp(f, 53)), e - 53);
}

// the same magnitude plus a unit in the 40th digit after the point: past any double, and past
// every exact value of the configurations below
inline std::string NudgeAway(const std::string& s) {
	return s + (s.find('.') == std::string::npos ? "." : "") + std::string(39, '0') + "1";
}

struct Failures {
	int count = 0;
	bool report;
	explicit Failures(bool reportTestCases) : report(reportTestCases) {}
	template<typename A>
	void check(const std::string& what, bool ok, const A& got, const A& expected) {
		if (ok) return;
		if (report || count < 6)
			std::cerr << "FAIL: " << what << " gave " << to_binary(got) << ' ' << got << ", expected "
			          << to_binary(expected) << ' ' << expected << '\n';
		++count;
	}
};

// every case built from the exact value a (ubit clear) and the next exact value after it
template<typename A>
void VerifyExactPair(const A& v, Failures& fail) {
	if (v.isnan() || v.isinf() || v.ubit()) return;
	A vu(v);
	vu.set(0, true);  // the open interval above v
	A w(vu);
	++w;  // the next exact value
	const double dv = double(v);
	if (!std::isfinite(dv)) return;  // the top binade of an 11-bit exponent is past double
	const std::string ev = ExactDecimal(dv);

	A t;
	fail.check("parse(" + ev + ")", parse(ev, t) && t == v, t, v);
	fail.check("parse([" + ev + "])", parse("[" + ev + "]", t) && t == v, t, v);
	if (w.isinf() || w.isnan()) {
		// v is maxpos or maxneg: past it is the saturated interval
		fail.check("parse(" + NudgeAway(ev) + ")", parse(NudgeAway(ev), t) && t == vu, t, vu);
		return;
	}
	const double dw = double(w);
	if (!std::isfinite(dw)) return;
	const std::string ew = ExactDecimal(dw);
	fail.check("parse(" + NudgeAway(ev) + ")", parse(NudgeAway(ev), t) && t == vu, t, vu);
	// halves and quarters first, so values near the top of double's range do not overflow; each
	// is exact, since v and w are adjacent and have far fewer bits than a double
	const std::string mid = ExactDecimal(dv / 2.0 + dw / 2.0);
	fail.check("parse(" + mid + ")", parse(mid, t) && t == vu, t, vu);
	// nearest exact value: the midpoint is a tie, which goes to the even encoding
	const A even = v.at(1) ? w : v;
	fail.check("parse([" + mid + "])", parse("[" + mid + "]", t) && t == even, t, even);
	const std::string nearV = ExactDecimal(0.75 * dv + 0.25 * dw), nearW = ExactDecimal(0.25 * dv + 0.75 * dw);
	fail.check("parse([" + nearV + "])", parse("[" + nearV + "]", t) && t == v, t, v);
	fail.check("parse([" + nearW + "])", parse("[" + nearW + "]", t) && t == w, t, w);
	// the interval between them, in either order
	fail.check("parse((" + ev + ", " + ew + "))", parse("(" + ev + ", " + ew + ")", t) && t == vu, t, vu);
	fail.check("parse((" + ew + ", " + ev + "))", parse("(" + ew + ", " + ev + ")", t) && t == vu, t, vu);
}

// the text operator<< writes for a, read back three ways
template<typename A>
void VerifyRoundTrip(const A& a, Failures& fail, std::streamsize precision) {
	// operator<< prints through double, so a finite value past double's range, or one whose next
	// value is, prints as inf and cannot come back
	if (!a.isnan() && !a.isinf()) {
		A next(a);
		++next;
		if (!std::isfinite(double(a)) || (!next.isinf() && !next.isnan() && !std::isfinite(double(next)))) return;
	}
	std::ostringstream os;
	os << std::setprecision(static_cast<int>(precision)) << a;
	const std::string text = os.str();
	auto same = [&a](const A& b) { return a.isnan() ? b.isnan() : (b == a); };  // nan: the text keeps no kind
	A t;
	fail.check("parse(" + text + ")", parse(text, t) && same(t), t, a);
	A u;
	u.assign(text);
	fail.check("assign(" + text + ")", same(u), u, a);
	std::istringstream is(text + " 1.5");
	A x, y;
	is >> x >> y;
	fail.check("operator>>(" + text + ")", bool(is) && same(x), x, a);
	fail.check("operator>> after " + text, bool(is) && y == A(1.5), y, A(1.5));
}

// every encoding of the configuration
template<unsigned nbits, unsigned es, typename bt>
int VerifyExhaustive(bool reportTestCases) {
	using A = areal<nbits, es, bt>;
	Failures fail(reportTestCases);
	for (std::uint64_t bits = 0; bits < (std::uint64_t(1) << nbits); ++bits) {
		A a;
		a.setbits(bits);
		VerifyExactPair(a, fail);
		VerifyRoundTrip(a, fail, 6);  // operator<<'s default precision is enough at these widths
	}
	return fail.count;
}

// random encodings of a wider configuration; the round trip prints every digit a double has
template<unsigned nbits, unsigned es, typename bt>
int VerifySampled(unsigned samples, bool reportTestCases) {
	using A = areal<nbits, es, bt>;
	Failures fail(reportTestCases);
	std::mt19937_64 rng(1454u + nbits);
	for (unsigned k = 0; k < samples; ++k) {
		A a;
		a.setbits(rng());
		VerifyExactPair(a, fail);
		VerifyRoundTrip(a, fail, std::numeric_limits<double>::max_digits10);
	}
	return fail.count;
}

// the cases #1454 reported, and the forms the parser must reject
inline int VerifyReportedAndRejected(bool reportTestCases) {
	using A = areal<16, 5, std::uint16_t>;
	Failures fail(reportTestCases);
	{
		std::istringstream is("1.5");  // the reproducer: a link error before the fix
		A a;
		is >> a;
		fail.check("istringstream(\"1.5\") >> a", bool(is) && a == A(1.5), a, A(1.5));
	}
	{
		A a(2.0);
		a.assign("0.1");  // was a stub that printed "assign TBD"
		A expected;
		parse("(0.0999756, 0.100098)", expected);
		fail.check("assign(\"0.1\")", a == expected && a.ubit(), a, expected);
		a.assign("not a number");
		fail.check("assign(\"not a number\") is 0", a.iszero() && !a.sign(), a, A(0));
	}
	A t;
	for (const char* bad :
	     {"", "abc", "[1.5", "1.5]", "(1.5, 1.50195", "(1, 2)", "(1.5, 1.5)", "(1, 2, 3)", "(-1.5, 1.50195)", "[]"}) {
		const bool ok = parse(bad, t);
		fail.check(std::string("parse(\"") + bad + "\") is rejected", !ok, t, t);
	}
	{
		std::istringstream is("(1, 2) 1.5");
		A a;
		is >> a;
		fail.check("operator>> of (1, 2) sets failbit", is.fail(), a, a);
	}
	return fail.count;
}

// wider than a double: parse is exact, where conversion from double is not
inline int VerifyWide(bool reportTestCases) {
	using A = areal<128, 15, std::uint32_t>;
	Failures fail(reportTestCases);
	// 1 + 2^-60: exact in this areal, not a double
	A expected(1.0);
	expected.set(A::fbits - 60 + 1);  // fraction bit 2^-60; bit 0 is the ubit
	const std::string text = ExactDecimal(false, (std::uint64_t(1) << 60) + 1u, -60);
	A t;
	fail.check("parse(1 + 2^-60)", parse(text, t) && t == expected && !t.ubit(), t, expected);
	fail.check("parse(0.1) is uncertain", parse("0.1", t) && t.ubit(), t, t);
	A exact15(1.5);
	fail.check("parse([1.5])", parse("[1.5]", t) && t == exact15, t, exact15);
	fail.check("parse(1e5000) saturates", parse("1e5000", t) && t.ubit() && !t.isinf() && !t.sign(), t, t);
	return fail.count;
}

} // namespace string_parse_test

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
	using namespace sw::universal::string_parse_test;

	std::string test_suite  = "areal text to value: parse, assign, operator>>";
	std::string test_tag    = "parse";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	areal<16, 5, std::uint16_t> a;
	std::istringstream is("(0.0999756, 0.100098)");
	is >> a;
	std::cout << "read back: " << a << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 2, std::uint8_t>(true), "areal<8,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyReportedAndRejected(reportTestCases), "areal<16,5>", "reported cases");
	// exhaustive: every encoding, single- and multi-limb
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyExhaustive<8, 2, std::uint8_t>(reportTestCases), "areal< 8,2,uint8_t >", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyExhaustive<10, 3, std::uint8_t>(reportTestCases), "areal<10,3,uint8_t >", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyExhaustive<12, 4, std::uint16_t>(reportTestCases), "areal<12,4,uint16_t>", test_tag);
	// sampled: wider, and a precision beyond the default six digits
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampled<16, 5, std::uint16_t>(2000, reportTestCases), "areal<16,5,uint16_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampled<32, 8, std::uint32_t>(2000, reportTestCases), "areal<32,8,uint32_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampled<32, 8, std::uint8_t>(1000, reportTestCases), "areal<32,8,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide(reportTestCases), "areal<128,15>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyExhaustive<16, 5, std::uint16_t>(reportTestCases), "areal<16,5,uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySampled<48, 11, std::uint16_t>(20000, reportTestCases),
	                                        "areal<48,11,uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifySampled<32, 8, std::uint32_t>(1000000, reportTestCases),
	                                        "areal<32,8,uint32_t>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
