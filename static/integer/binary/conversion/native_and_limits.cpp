// native_and_limits.cpp: integer <-> native float conversions, casts, shifts, parse and numeric_limits (#2131)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

/*
   The integer findings of QA sweep #2117, tracked in #2131, with the USER decisions:
     - >> past nbits is an arithmetic shift: a negative value becomes -1;
     - a native float converts by truncation toward zero, and NaN, infinity and values past the
       range follow the #2104 cast rule (signed: saturate to [-max, max], NaN to min(); unsigned
       kinds: saturate to [0, max - 1], NaN to max());
     - a cast to a native unsigned type is modular, as C++'s: the value sign-extends.
     - WholeNumber is {0, 1, 2, ...} and NaturalNumber {1, 2, 3, ...}, as documented; the operators
       used to enforce the reverse.
   The references are independent of the code under test: an integer to a native float against
   strtod/strtof of the integer's decimal string (correctly rounded by the C library), and a
   native float to an integer against the decimal printout of trunc(x).
*/

namespace sw { namespace universal { namespace integer_native {

// integer -> double / float, correctly rounded, at wide and narrow limbs
template<typename Integer>
int VerifyToNative(int trials, bool reportTestCases) {
	std::mt19937_64 eng(0x2131u + Integer::nbits);
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Integer v;
		v.clear();
		for (unsigned i = 0; i < Integer::nbits; ++i) if (eng() & 1u) v.setbit(i);
		// also sparse values, which sit on and next to the ties: 2^k + 2^j + 1
		if (t % 3 == 0) {
			v.clear();
			v.setbit(static_cast<unsigned>(eng() % (Integer::nbits - 1)));
			v.setbit(static_cast<unsigned>(eng() % (Integer::nbits - 1)));
			v.setbit(0);
		}
		const std::string decimal = to_string(v);
		const double expectedD = std::strtod(decimal.c_str(), nullptr);
		const float  expectedF = std::strtof(decimal.c_str(), nullptr);
		const double d = double(v);
		const float  f = float(v);
		if (std::memcmp(&d, &expectedD, sizeof d) != 0 || std::memcmp(&f, &expectedF, sizeof f) != 0) {
			++fails;
			if (reportTestCases && fails < 6) {
				std::cerr << "FAIL " << decimal << " -> double " << d << " (expected " << expectedD << "), float " << f << " (expected " << expectedF << ")\n";
			}
		}
	}
	return fails;
}

// double -> integer: truncation toward zero, exactly, at every width
template<typename Integer>
int VerifyFromNative(int trials, bool reportTestCases) {
	std::mt19937_64 eng(0x2131u * 3u + Integer::nbits);
	std::uniform_real_distribution<double> unit(1.0, 2.0);
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		const int e = static_cast<int>(eng() % (Integer::nbits - 1));   // inside the signed range
		double x = std::ldexp(unit(eng), e - 1);
		if (eng() & 1u) x = -x;
		const Integer v(x);
		char buf[512];
		std::snprintf(buf, sizeof buf, "%.0f", std::trunc(x));   // exact: glibc prints every digit
		Integer expected;
		expected.assign(std::string(buf));
		if (!(v == expected)) {
			++fails;
			if (reportTestCases && fails < 6) std::cerr << "FAIL integer(" << buf << " from " << x << ") = " << to_string(v) << '\n';
		}
	}
	return fails;
}

// NaN, infinity and values past the range: the #2104 rule
int VerifyCastRule(bool reportTestCases) {
	int fails = 0;
	auto expect = [&](const char* what, bool ok) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	using I16 = integer<16, std::uint8_t>;
	const double nan = std::numeric_limits<double>::quiet_NaN();
	const double inf = std::numeric_limits<double>::infinity();
	expect("integer<16>(-2.7) is -2", int(I16(-2.7)) == -2);
	expect("integer<16>(2.7) is 2", int(I16(2.7)) == 2);
	expect("integer<16>(0.9) is 0", int(I16(0.9)) == 0);
	expect("integer<16>(NaN) is min()", int(I16(nan)) == -32768);
	expect("integer<16>(inf) is max()", int(I16(inf)) == 32767);
	expect("integer<16>(-inf) is -max()", int(I16(-inf)) == -32767);
	expect("integer<16>(1e9) saturates to max()", int(I16(1e9)) == 32767);
	expect("integer<16>(-1e9) saturates to -max()", int(I16(-1e9)) == -32767);
	expect("integer<16>(-32768.0) saturates to -max(): min() is the NaR", int(I16(-32768.0)) == -32767);
	expect("integer<16>(32767.9) is 32767", int(I16(32767.9)) == 32767);
	using I128 = integer<128, std::uint32_t>;
	expect("integer<128>(1e30) is exact", to_string(I128(1e30)) == "1000000000000000019884624838656");
	expect("integer<128>(-1e30) is exact", to_string(I128(-1e30)) == "-1000000000000000019884624838656");
	using W8 = integer<8, std::uint8_t, WholeNumber>;
	expect("whole<8>(1e9) saturates to max() - 1", static_cast<unsigned>(W8(1e9)) == 254u);
	expect("whole<8>(NaN) is max()", static_cast<unsigned>(W8(nan)) == 255u);
	expect("whole<8>(255.0) saturates to max() - 1", static_cast<unsigned>(W8(255.0)) == 254u);
	expect("whole<8>(254.5) is 254", static_cast<unsigned>(W8(254.5)) == 254u);
	expect("whole<8>(-3.0) clamps to 0", W8(-3.0).iszero());
	using N8 = integer<8, std::uint8_t, NaturalNumber>;
	expect("natural<8>(0.5) clamps to 1, its smallest value", static_cast<unsigned>(N8(0.5)) == 1u);
	expect("natural<8>(-3.0) clamps to 1", static_cast<unsigned>(N8(-3.0)) == 1u);
	return fails;
}

// a cast to a native unsigned type is modular: the value sign-extends
int VerifyUnsignedCasts(bool reportTestCases) {
	int fails = 0;
	auto expect = [&](const char* what, bool ok) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	expect("(unsigned)integer<8>(-1) is 0xFFFFFFFF", static_cast<unsigned>(integer<8>(-1)) == 0xFFFF'FFFFu);
	expect("(unsigned long long)integer<12>(-5)", static_cast<unsigned long long>(integer<12>(-5)) == static_cast<unsigned long long>(-5ll));
	expect("(unsigned short)integer<40>(-3)", static_cast<unsigned short>(integer<40, std::uint16_t>(-3)) == static_cast<unsigned short>(-3));
	expect("(unsigned)integer<8>(100)", static_cast<unsigned>(integer<8>(100)) == 100u);
	return fails;
}

// >> past nbits is arithmetic; logicShiftRight works by limbs and fills with zeros
template<typename Integer>
int VerifyShifts(bool reportTestCases) {
	int fails = 0;
	constexpr int n = static_cast<int>(Integer::nbits);
	for (int s : { n, n + 1, n + 17, 2 * n }) {
		const Integer neg(-5), pos(5);
		if (static_cast<long long>(neg >> s) != -1 || static_cast<long long>(pos >> s) != 0) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL >> " << s << " of -5/5 at " << n << " bits\n";
		}
	}
	std::mt19937_64 eng(0x2131u);
	for (int t = 0; t < 200; ++t) {
		Integer v;
		v.clear();
		for (unsigned i = 0; i < Integer::nbits; ++i) if (eng() & 1u) v.setbit(i);
		const int s = static_cast<int>(eng() % (Integer::nbits + 2));
		Integer logic = v;
		logic.logicShiftRight(s);
		Integer expected;
		expected.clear();
		for (int i = s; i < n; ++i) if (v.at(static_cast<unsigned>(i))) expected.setbit(static_cast<unsigned>(i - s));
		if (!(logic == expected)) {
			++fails;
			if (reportTestCases && fails < 4) std::cerr << "FAIL logicShiftRight " << s << " at " << n << " bits\n";
		}
	}
	return fails;
}

// *= BlockType keeps the bits past nbits clear, as every other operator does
template<typename Integer>
int VerifyScaleByLimb(bool reportTestCases) {
	using bt = typename Integer::BlockType;
	std::mt19937_64 eng(0x2131u + 7u);
	int fails = 0;
	for (int t = 0; t < 500; ++t) {
		Integer v;
		v.clear();
		for (unsigned i = 0; i < Integer::nbits; ++i) if (eng() & 1u) v.setbit(i);
		const bt scale = static_cast<bt>(eng());
		Integer scaled = v;
		scaled *= scale;
		Integer s;
		s.clear();
		for (unsigned i = 0; i < 8u * sizeof(bt) && i < Integer::nbits; ++i) if ((static_cast<std::uint64_t>(scale) >> i) & 1u) s.setbit(i);
		Integer product = v;
		product *= s;   // the full product, modulo 2^nbits
		if (!(scaled == product)) {
			++fails;
			if (reportTestCases && fails < 4) std::cerr << "FAIL *= limb at " << Integer::nbits << " bits\n";
		}
	}
	return fails;
}

// a failed parse leaves 0, whatever the integer held: parse() writes only on success, and assign()
// used to read the stale blocks back (0xAAAA stayed 0xAAAA)
int VerifyFailedParse(bool reportTestCases) {
	using I = integer<16, std::uint8_t>;
	I v;
	v.setbits(0xAAAAu);
	v.assign(std::string("xyz"));
	const bool ok = v.iszero();
	if (!ok && reportTestCases) std::cerr << "FAIL assign(\"xyz\") left " << to_string(v) << ", not 0\n";
	return ok ? 0 : 1;
}

int VerifyNumericLimits(bool reportTestCases) {
	int fails = 0;
	auto expect = [&](const char* what, bool ok) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	using I = std::numeric_limits<integer<8>>;
	expect("integer<8> min() is -128", int(I::min()) == -128 && int(I::lowest()) == -128);
	expect("integer<8> max() is 127", int(I::max()) == 127);
	expect("integer<8> epsilon() is 0, of the integer type", I::epsilon().iszero());
	expect("integer<8> infinity() is 0, as has_infinity is false", I::infinity().iszero() && !I::has_infinity);
	expect("integer<8> is_signed, digits 7", I::is_signed && I::digits == 7);
	using W = std::numeric_limits<integer<8, std::uint8_t, WholeNumber>>;
	expect("whole<8> {0, 1, ...}: min() is 0, max() 255, unsigned", W::min().iszero() && static_cast<unsigned>(W::max()) == 255u && !W::is_signed && W::digits == 8);
	using N = std::numeric_limits<integer<8, std::uint8_t, NaturalNumber>>;
	expect("natural<8> {1, 2, ...}: min() is 1, max() 255, unsigned", static_cast<unsigned>(N::min()) == 1u && static_cast<unsigned>(N::max()) == 255u && !N::is_signed);
	return fails;
}

// the documented domains (USER decision on #2131): WholeNumber {0, 1, 2, ...} holds a zero result,
// NaturalNumber {1, 2, 3, ...} does not. The operators used to enforce the reverse.
int VerifyDomains(bool reportTestCases) {
	int fails = 0;
	auto expect = [&](const char* what, bool ok) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	using W = integer<8, std::uint8_t, WholeNumber>;
	using N = integer<8, std::uint8_t, NaturalNumber>;
	expect("whole 3 - 3 is 0", (W(3) - W(3)).iszero());
	expect("whole 3 / 5 is 0", (W(3) / W(5)).iszero());
	expect("whole 0 constructs", W(0).iszero());
	expect("whole 7 / 2 is 3", static_cast<unsigned>(W(7) / W(2)) == 3u);
	expect("natural 7 / 2 is 3", static_cast<unsigned>(N(7) / N(2)) == 3u);
	expect("natural 5 - 2 is 3", static_cast<unsigned>(N(5) - N(2)) == 3u);
	return fails;
}

// compile-time conversions stay constant expressions
constexpr bool ConstexprConversions() {
	constexpr integer<16> a(1.0);
	constexpr integer<16> b(-2.75);
	constexpr integer<128, std::uint32_t> c(1.0e30);
	return a.isone() && b.isneg() && !c.iszero();
}
static_assert(ConstexprConversions(), "constexpr native conversions");

#if LONG_DOUBLE_SUPPORT
// long double, whatever its significand: integer -> long double against strtold, and back. An IBM
// double-double (106 digits) is the sum of two doubles, not a fixed-precision format; glibc's strtold
// and printf round its value to 106 contiguous bits, so lo carries fewer bits than it can. There the
// check is the canonical pair -- hi the correctly rounded double, the pair at least as close to the
// value as strtold's -- and an exact round trip back to the integer.
template<typename Integer>
int VerifyLongDouble(int trials, bool reportTestCases) {
	std::mt19937_64 eng(0x2131u + 11u);
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Integer v;
		v.clear();
		for (unsigned i = 0; i + 1 < Integer::nbits; ++i) if (eng() & 1u) v.setbit(i);
		const std::string decimal = to_string(v);
		const long double expected = std::strtold(decimal.c_str(), nullptr);
		const long double got = static_cast<long double>(v);
		if constexpr (std::numeric_limits<long double>::digits == 106) {
			using Wide = integer<Integer::nbits + 4, typename Integer::BlockType>;
			const double hi = static_cast<double>(got);
			const double lo = static_cast<double>(got - static_cast<long double>(hi));
			// the value of a pair whose limbs are integers (these values are far past 2^106)
			auto pairValue = [](long double x) {
				const double h = static_cast<double>(x);
				const double l = static_cast<double>(x - static_cast<long double>(h));
				return Wide(Wide(h) + Wide(l));
			};
			Wide exact(v);
			Wide errGot = exact - pairValue(got), errLib = exact - pairValue(expected);
			if (errGot.isneg()) errGot = -errGot;
			if (errLib.isneg()) errLib = -errLib;
			const bool canonicalHi = (hi == static_cast<double>(v));
			const bool closest = !(errLib < errGot);
			const bool roundTrip = (Integer(got) == Integer(Wide(Wide(hi) + Wide(lo))));
			if (!canonicalHi || !closest || !roundTrip) {
				++fails;
				if (reportTestCases && fails < 4) std::cerr << "FAIL " << decimal << " -> double-double (hi " << canonicalHi << ", closest " << closest << ", round trip " << roundTrip << ")\n";
			}
			continue;
		}
		if (std::memcmp(&got, &expected, sizeof(long double)) != 0 && got != expected) {
			++fails;
			if (reportTestCases && fails < 4) std::cerr << "FAIL " << decimal << " -> long double\n";
		}
		const Integer back(std::trunc(got));
		char buf[256];
		std::snprintf(buf, sizeof buf, "%.0Lf", got);
		Integer exactBack;
		exactBack.assign(std::string(buf));
		if (!(back == exactBack)) {
			++fails;
			if (reportTestCases && fails < 4) std::cerr << "FAIL integer(" << buf << "L)\n";
		}
	}
	return fails;
}

// the cast rule through long double, at a width where the value is past the range
int VerifyLongDoubleRange(bool reportTestCases) {
	int fails = 0;
	auto expect = [&](const char* what, bool ok) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	using I16 = integer<16, std::uint16_t>;
	expect("integer<16>(1e9L) saturates to 32767", int(I16(1.0e9L)) == 32767);
	expect("integer<16>(-1e9L) saturates to -32767", int(I16(-1.0e9L)) == -32767);
	expect("integer<16>(-2.75L) truncates to -2", int(I16(-2.75L)) == -2);
	expect("integer<16>(1e30L) saturates to 32767", int(I16(1.0e30L)) == 32767);
	return fails;
}
#endif

}}}  // namespace sw::universal::integer_native

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::integer_native;

	std::string test_suite  = "integer native conversions, casts, shifts, parse, numeric_limits";
	std::string test_tag    = "native";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<integer<128, std::uint32_t>>(1000, reportTestCases), "integer<128>", "to native");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<integer< 64, std::uint8_t >>(2000, reportTestCases), "integer<64,uint8>", "to native");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<integer<128, std::uint32_t>>(2000, reportTestCases), "integer<128,uint32>", "to native");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<integer<200, std::uint64_t>>(1000, reportTestCases), "integer<200,uint64>", "to native");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<integer< 40, std::uint16_t, WholeNumber>>(2000, reportTestCases), "whole<40,uint16>", "to native");

	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<integer< 32, std::uint8_t >>(2000, reportTestCases), "integer<32,uint8>", "from native");
	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<integer<128, std::uint32_t>>(2000, reportTestCases), "integer<128,uint32>", "from native");
	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<integer<300, std::uint64_t>>(1000, reportTestCases), "integer<300,uint64>", "from native");

	nrOfFailedTestCases += ReportTestResult(VerifyCastRule(reportTestCases), "integer", "#2104 cast rule");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedCasts(reportTestCases), "integer", "unsigned casts");

	nrOfFailedTestCases += ReportTestResult(VerifyShifts<integer< 8, std::uint8_t >>(reportTestCases), "integer<8>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<integer<17, std::uint8_t >>(reportTestCases), "integer<17>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<integer<64, std::uint32_t>>(reportTestCases), "integer<64,uint32>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<integer<100, std::uint64_t>>(reportTestCases), "integer<100,uint64>", "shifts");

	nrOfFailedTestCases += ReportTestResult(VerifyScaleByLimb<integer< 12, std::uint8_t >>(reportTestCases), "integer<12,uint8>", "*= limb");
	nrOfFailedTestCases += ReportTestResult(VerifyScaleByLimb<integer< 50, std::uint16_t>>(reportTestCases), "integer<50,uint16>", "*= limb");
	nrOfFailedTestCases += ReportTestResult(VerifyScaleByLimb<integer<100, std::uint64_t>>(reportTestCases), "integer<100,uint64>", "*= limb");

	nrOfFailedTestCases += ReportTestResult(VerifyFailedParse(reportTestCases), "integer<16>", "failed parse");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits(reportTestCases), "integer", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyDomains(reportTestCases), "integer", "Whole/Natural domains");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyLongDouble<integer<128, std::uint32_t>>(1000, reportTestCases), "integer<128>", "long double");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDoubleRange(reportTestCases), "integer<16>", "long double range");
#endif
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
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
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
