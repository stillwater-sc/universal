// saturation.cpp: conversion and arithmetic at the top of the areal range
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Values just above the largest finite exponent came out with the wrong sign or as nan
   instead of saturating to (maxpos, inf) (#1503): areal<16,5> maxpos + maxpos was
   -6.08e-05, 65536 * 2 was -0, 65536 + 65472 was nan. Two causes, in every conversion:
   MAX_EXP was one more than the largest finite scale, so a value of that scale got a biased
   exponent that ran into the sign bit; and in the top binade, values past maxpos + ulp
   truncated onto the all-ones fraction, which encodes inf or nan.

   The reference does not use any areal conversion. For each small configuration every
   encoding is enumerated and its exact value tabulated; a finite x then maps to the exact
   value equal to it, or to the exact value below it in magnitude with the ubit set, or past
   maxpos to (maxpos, inf), below minpos to (0, minpos), each with the sign of x. Conversion
   from double and float is checked for every exact value, every midpoint between adjacent
   ones, and a sweep of the top binade and beyond; +, - and * of every pair of exact values
   against their exact double result. Wider configurations are checked at the boundary:
   the native integer path of areal<64,6>, and the blocktriple arithmetic of areal<128,15>.
   Every IEEE NaN payload converts to a NaN of its kind. Conversion from long double is checked
   the same way, with values double cannot hold where long double is wider: it used to narrow to
   double first, which rounded them.

   The arithmetic check runs on single-limb configurations: where the exponent field
   straddles two limbs, + - * are wrong across the whole range (#1506). An exact zero result
   may carry either sign: +0 - +0 gives -0 where IEEE gives +0 (#1507).
*/

namespace sw { namespace universal {

namespace saturation {

// every exact, finite, non-negative value of the configuration with its encoding, ascending
template<typename A>
std::vector<std::pair<double, std::uint64_t>> ExactValues() {
	constexpr unsigned nbits = A::nbits;
	std::vector<std::pair<double, std::uint64_t>> table;
	for (std::uint64_t bits = 0; bits < (std::uint64_t(1) << (nbits - 1)); ++bits) {  // sign bit clear
		A a;
		a.setbits(bits);
		if (a.ubit() || a.isnan() || a.isinf()) continue;
		table.emplace_back(double(a), bits);
	}
	std::sort(table.begin(), table.end());
	return table;
}

// the encoding the faithful rule gives the finite x
template<typename A, typename Real>
A Reference(Real x, const std::vector<std::pair<double, std::uint64_t>>& table) {
	const bool negative = std::signbit(x);
	const Real m = std::fabs(x);
	A r;
	const auto it = std::upper_bound(table.begin(), table.end(), m, [](Real v, const std::pair<double, std::uint64_t>& e) { return v < e.first; });
	// the largest exact value <= m; table[0] is zero
	const auto& below = *(it - 1);
	r.setbits(below.second);
	if (below.first != m) r.set(0, true);  // inexact: the interval above it, or (maxpos, inf) past the top
	if (negative) r.setsign(true);
	return r;
}

struct Failures {
	int count = 0;
	bool report;
	explicit Failures(bool reportTestCases) : report(reportTestCases) {}
	template<typename A>
	void check(const std::string& what, const A& got, const A& expected) {
		if (got == expected) return;
		if (report || count < 6)
			std::cerr << "FAIL: " << what << " gave " << to_binary(got) << ' ' << got << ", expected " << to_binary(expected)
			          << ' ' << expected << '\n';
		++count;
	}
};

// conversion from double, and from float when x is a float, against the reference
template<typename A>
void VerifyConversion(double x, const std::vector<std::pair<double, std::uint64_t>>& table, Failures& fail) {
	const A expected = Reference<A>(x, table);
	fail.check("areal(" + std::to_string(x) + ")", A(x), expected);
	const float f = static_cast<float>(x);
	if (static_cast<double>(f) == x && std::isfinite(f)) fail.check("areal(float " + std::to_string(x) + ")", A(f), expected);
}

// every exact value and midpoint, the top binade and beyond, both signs
template<typename A>
int VerifyConversions(bool reportTestCases) {
	Failures fail(reportTestCases);
	const auto table = ExactValues<A>();
	const double top = table.back().first;  // maxpos
	const double ulp = top - table[table.size() - 2].first;
	std::vector<double> xs;
	for (std::size_t i = 0; i < table.size(); ++i) {
		xs.push_back(table[i].first);
		if (i + 1 < table.size()) xs.push_back(table[i].first / 2.0 + table[i + 1].first / 2.0);
	}
	// the top binade past maxpos, the next binade, and far beyond
	const double nextBinade = std::ldexp(1.0, std::ilogb(top) + 1);
	for (double k = 0.125; k < 8.0; k += 0.125) xs.push_back(top + k * ulp);
	for (double t = 0.0; t <= 1.0; t += 0.0625) xs.push_back(nextBinade * (1.0 + t));
	for (double x : { 2.0 * nextBinade, 3.0 * nextBinade, 1.0e30, 1.0e300 }) xs.push_back(x);
	for (double x : xs) {
		VerifyConversion<A>(x, table, fail);
		VerifyConversion<A>(-x, table, fail);
	}
	return fail.count;
}

// conversion from long double, which used to narrow to double first: every exact value, and a
// value 2^-58 of its scale above and below it, which double cannot hold; past the double range
template<typename A>
int VerifyLongDouble(bool reportTestCases) {
	Failures fail(reportTestCases);
	const auto table = ExactValues<A>();
	std::vector<long double> xs;
	for (const auto& e : table) {
		const long double v = e.first;
		xs.push_back(v);
		if (v == 0.0l) continue;
		const long double tiny = std::ldexp(1.0l, std::ilogb(v) - 58);
		xs.push_back(v + tiny);
		xs.push_back(v - tiny);
	}
	if (std::numeric_limits<long double>::max_exponent > 2000) xs.push_back(std::ldexp(1.0l, 2000));
	if (std::numeric_limits<long double>::min_exponent < -2000) xs.push_back(std::ldexp(1.0l, -2000));
	for (long double x : xs) {
		for (long double y : { x, -x }) {
			A a;
			a = y;
			fail.check("areal(" + std::to_string(y) + "l)", a, Reference<A>(y, table));
		}
	}
	return fail.count;
}

// the areal<128,15> encoding of a long double it holds exactly, built with frexp rather than the
// conversion under test: sign, biased exponent, the fraction below the hidden bit, a clear ubit
inline std::string Exact128(long double x) {
	constexpr int es = 15, fbits = 128 - 2 - es, bias = (1 << (es - 1)) - 1;
	int e = 0;
	long double m = std::frexp(std::fabs(x), &e) * 2.0l - 1.0l;  // [0.5, 1) to the fraction of [1, 2)
	std::string s = std::signbit(x) ? "b1" : "b0";
	const int biased = e - 1 + bias;
	for (int i = es - 1; i >= 0; --i) s += ((biased >> i) & 1) ? '1' : '0';
	for (int i = 0; i < fbits; ++i) {
		m *= 2.0l;
		s += (m >= 1.0l) ? '1' : '0';
		if (m >= 1.0l) m -= 1.0l;
	}
	return s + '0';
}

// every IEEE NaN converts to a NaN of the same kind, whatever its payload: only two payloads used
// to be recognized, and the rest came out as (maxpos, inf), inf, or a finite value
inline std::string Hex(std::uint64_t bits) {
	const char* digits = "0123456789ABCDEF";
	std::string s;
	do {
		s.insert(s.begin(), digits[bits & 0xFu]);
		bits >>= 4;
	} while (bits != 0);
	return "0x" + s;
}

template<typename A>
int VerifyNaNPayloads(bool reportTestCases) {
	int fails = 0;
	auto verify = [&](const std::string& what, const A& a, bool quiet) {
		if (a.isnan(quiet ? NAN_TYPE_QUIET : NAN_TYPE_SIGNALLING)) return;
		++fails;
		if (reportTestCases || fails < 6)
			std::cerr << "FAIL: " << what << " gave " << to_binary(a) << ' ' << a << ", expected a " << (quiet ? "quiet" : "signalling") << " nan\n";
	};
	for (std::uint64_t sign : { std::uint64_t(0), std::uint64_t(1) << 63 }) {
		for (std::uint64_t payload : { 0x8'0000'0000'0000ull, 0x8'0000'0000'0001ull, 0x8'0000'0000'0002ull, 0xF'FFFF'FFFF'FFFFull,
		                               0x1ull, 0x2ull, 0x7'FFFF'FFFF'FFFFull, 0x4'0000'0000'0000ull }) {
			const std::uint64_t bits = sign | 0x7FF0'0000'0000'0000ull | payload;
			double d;
			std::memcpy(&d, &bits, sizeof(d));
			A a;
			a = d;
			verify("double " + Hex(bits), a, (payload & 0x8'0000'0000'0000ull) != 0);
		}
	}
	for (std::uint32_t sign : { 0u, 1u << 31 }) {
		for (std::uint32_t payload : { 0x40'0000u, 0x40'0001u, 0x40'0002u, 0x7F'FFFFu, 0x1u, 0x2u, 0x3F'FFFFu, 0x20'0000u }) {
			const std::uint32_t bits = sign | 0x7F80'0000u | payload;
			float f;
			std::memcpy(&f, &bits, sizeof(f));
			A a;
			a = f;
			verify("float " + Hex(bits), a, (payload & 0x40'0000u) != 0);
		}
	}
	return fails;
}

// +, - and * of every pair of exact values against the exact double result
template<typename A>
int VerifyArithmetic(bool reportTestCases) {
	Failures fail(reportTestCases);
	const auto table = ExactValues<A>();
	std::vector<double> values;
	for (const auto& e : table) {
		values.push_back(e.first);
		if (e.first != 0.0) values.push_back(-e.first);
	}
	// an exact zero result may carry either sign (#1507)
	auto sameOrZero = [](const A& got, const A& expected) {
		return got == expected || (got.iszero() && expected.iszero() && !got.ubit() && !expected.ubit());
	};
	for (double x : values) {
		for (double y : values) {
			const A a(x), b(y);
			// sums, differences and products of these values are exact in double
			const A sum = a + b, difference = a - b, product = a * b;
			const A rs = Reference<A>(x + y, table), rd = Reference<A>(x - y, table), rp = Reference<A>(x * y, table);
			const std::string xy = std::to_string(x) + ", " + std::to_string(y);
			if (!sameOrZero(sum, rs)) fail.check("+ of " + xy, sum, rs);
			if (!sameOrZero(difference, rd)) fail.check("- of " + xy, difference, rd);
			if (!sameOrZero(product, rp)) fail.check("* of " + xy, product, rp);
		}
	}
	return fail.count;
}

// the cases #1503 reported, the native integer conversion, and a type wider than a double
inline int VerifyReportedAndWide(bool reportTestCases) {
	Failures fail(reportTestCases);
	{
		using A = areal<16, 5, std::uint16_t>;
		A m;
		m.maxpos();
		A sat(m);
		sat.set(0, true);  // (130816, inf)
		fail.check("maxpos + maxpos", m + m, sat);
		fail.check("65536 * 2", A(65536.0) * A(2.0), sat);
		fail.check("65536 + 65472", A(65536.0) + A(65472.0), sat);
		fail.check("maxpos * 1.001953125", m * A(1.001953125), sat);
		fail.check("areal(131000.0)", A(131000.0), sat);
		fail.check("areal(131072.0)", A(131072.0), sat);
		A satneg(sat);
		satneg.setsign(true);
		fail.check("-maxpos - maxpos", -m - m, satneg);
		A p;
		parse("131000", p);
		fail.check("parse(\"131000\")", p, sat);
	}
	{
		using B = areal<32, 8, std::uint32_t>;
		B m;
		m.maxpos();
		B sat(m);
		sat.set(0, true);
		fail.check("areal<32,8> maxpos * 2", m * B(2.0), sat);
		fail.check("areal<32,8> maxpos + maxpos", m + m, sat);
	}
	{
		// the native integer path (fbits >= 53): scale 32 is the largest, maxpos is just under 2^33
		using W = areal<64, 6, std::uint32_t>;
		W sat;
		sat.maxpos();
		sat.set(0, true);
		const std::uint64_t below = (std::uint64_t(1) << 33) - 1u;  // exact
		W w;
		w = below;
		fail.check("areal<64,6>(2^33 - 1)", w, W(double(below)));
		for (std::uint64_t u : { std::uint64_t(1) << 33, (std::uint64_t(1) << 33) + 1u, std::uint64_t(1) << 34, ~std::uint64_t(0) }) {
			W x;
			x = u;
			fail.check("areal<64,6>(" + std::to_string(u) + "ull)", x, sat);
		}
		W sneg(sat);
		sneg.setsign(true);
		W y;
		y = -(std::int64_t(1) << 40);
		fail.check("areal<64,6>(-2^40)", y, sneg);
	}
	{
		// wider than a double: blocktriple arithmetic saturates the same way
		using X = areal<128, 15, std::uint32_t>;
		X m;
		m.maxpos();
		X sat(m);
		sat.set(0, true);
		fail.check("areal<128,15> maxpos + maxpos", m + m, sat);
		fail.check("areal<128,15> maxpos * 2", m * X(2.0), sat);
		X half(m);
		half *= X(0.5);
		fail.check("areal<128,15> (maxpos / 2) * 2 is maxpos", half * X(2.0), m);
		// wide enough for a 64-bit significand and a long double's exponent range: held exactly.
		// Checked on the encoding: conversion back to long double has its own defects (#1509)
		const long double third = std::ldexp(static_cast<long double>(0x5555'5555'5555'5555ull), -64);  // 63 bits
		for (long double x : { third, 1.0l + std::ldexp(1.0l, -60), -1.0l + std::ldexp(1.0l, -60), std::ldexp(1.0l, 2000),
		                       std::ldexp(1.0l, -2000) }) {
			if (!std::isfinite(x) || x == 0.0l) continue;
			X a(x);
			if (to_binary(a) != Exact128(x)) {
				++fail.count;
				std::cerr << "FAIL: areal<128,15>(" << x << "l) is " << to_binary(a) << ", not " << Exact128(x) << '\n';
			}
		}
	}
	if (std::numeric_limits<long double>::digits > 60) {
		// narrower than a long double significand: 1 +/- 2^-60 lie strictly inside an interval
		using Y = areal<64, 11, std::uint64_t>;
		Y above(1.0), below(1.0 - std::ldexp(1.0, -52));  // 1 and the exact value before it
		above.set(0, true);
		below.set(0, true);
		Y a, b;
		a = 1.0l + std::ldexp(1.0l, -60);
		b = 1.0l - std::ldexp(1.0l, -60);
		fail.check("areal<64,11>(1 + 2^-60)", a, above);
		fail.check("areal<64,11>(1 - 2^-60)", b, below);
		BIT_CAST_CONSTEXPR areal<16, 5, std::uint16_t> c(1.0l - 0x1p-60l);  // the conversion stays constexpr
		areal<16, 5, std::uint16_t> cref(1.0 - 0x1p-10);
		cref.set(0, true);
		fail.check("constexpr areal<16,5>(1 - 2^-60)", c, cref);
	}
	return fail.count;
}

} // namespace saturation

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
	using namespace sw::universal::saturation;

	std::string test_suite  = "areal saturation at the top of the range";
	std::string test_tag    = "saturation";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using A = areal<16, 5, std::uint16_t>;
	A m;
	m.maxpos();
	std::cout << "maxpos + maxpos = " << (m + m) << " (130816, inf)\n";
	nrOfFailedTestCases += ReportTestResult(VerifyConversions<areal<8, 2, std::uint8_t>>(true), "areal<8,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyReportedAndWide(reportTestCases), "reported and wide", test_tag);
	// conversion: every exact value and midpoint, and the top binade and beyond
	nrOfFailedTestCases += ReportTestResult(VerifyConversions<areal< 8, 2, std::uint8_t >>(reportTestCases), "areal< 8,2,uint8_t >", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversions<areal<10, 3, std::uint8_t >>(reportTestCases), "areal<10,3,uint8_t >", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversions<areal<12, 4, std::uint16_t>>(reportTestCases), "areal<12,4,uint16_t>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversions<areal<16, 5, std::uint16_t>>(reportTestCases), "areal<16,5,uint16_t>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversions<areal<16, 5, std::uint8_t >>(reportTestCases), "areal<16,5,uint8_t >", "conversion");
	// every NaN payload stays a NaN of its kind
	nrOfFailedTestCases += ReportTestResult(VerifyNaNPayloads<areal<16, 5, std::uint16_t>>(reportTestCases), "areal<16,5,uint16_t>", "nan payload");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNPayloads<areal<16, 5, std::uint8_t >>(reportTestCases), "areal<16,5,uint8_t >", "nan payload");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNPayloads<areal<32, 8, std::uint32_t>>(reportTestCases), "areal<32,8,uint32_t>", "nan payload");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNPayloads<areal<64, 11, std::uint64_t>>(reportTestCases), "areal<64,11,uint64_t>", "nan payload");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDouble<areal< 8, 2, std::uint8_t >>(reportTestCases), "areal< 8,2,uint8_t >", "long double");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDouble<areal<12, 4, std::uint16_t>>(reportTestCases), "areal<12,4,uint16_t>", "long double");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDouble<areal<16, 5, std::uint16_t>>(reportTestCases), "areal<16,5,uint16_t>", "long double");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDouble<areal<16, 5, std::uint8_t >>(reportTestCases), "areal<16,5,uint8_t >", "long double");
	// arithmetic: every pair of exact values, single-limb configurations (#1506)
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal< 8, 2, std::uint8_t >>(reportTestCases), "areal< 8,2,uint8_t >", "+ - *");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal< 9, 3, std::uint16_t>>(reportTestCases), "areal< 9,3,uint16_t>", "+ - *");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal<10, 3, std::uint16_t>>(reportTestCases), "areal<10,3,uint16_t>", "+ - *");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal<12, 5, std::uint16_t>>(reportTestCases), "areal<12,5,uint16_t>", "+ - *");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal<12, 4, std::uint16_t>>(reportTestCases), "areal<12,4,uint16_t>", "+ - *");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<areal<13, 4, std::uint16_t>>(reportTestCases), "areal<13,4,uint16_t>", "+ - *");
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
