// assignment.cpp: test suite runner for assignment on adaptive precision binary integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>

// minimum set of include files to reflect source code dependencies
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename BlockType>
	std::string ToDecimal(const einteger<BlockType>& v) {
		std::ostringstream s;
		s << v;
		return s.str();
	}

	// the values worth checking for a native integer type: the ends of the range, and 2^k - 1, 2^k, 2^k + 1
	// and their negatives for every k that fits
	template<typename NativeInt>
	std::vector<NativeInt> BoundaryValues() {
		using Limits = std::numeric_limits<NativeInt>;
		std::vector<NativeInt> values = { NativeInt(0), NativeInt(1), Limits::max(), NativeInt(Limits::max() - 1) };
		if constexpr (Limits::is_signed) {
			values.push_back(NativeInt(-1));
			values.push_back(Limits::min());
			values.push_back(NativeInt(Limits::min() + 1));
		}
		for (int k = 1; k < Limits::digits; ++k) {
			const NativeInt p = static_cast<NativeInt>(NativeInt(1) << k);
			for (NativeInt v : { NativeInt(p - 1), p, NativeInt(p + 1) }) {
				values.push_back(v);
				if constexpr (Limits::is_signed) values.push_back(NativeInt(-v));
			}
		}
		return values;
	}

	// native integer -> einteger: the decimal string must be the native's, and the signed types
	// convert back to the same value; the conversion back saturates past the target range
	template<typename BlockType, typename NativeInt>
	int VerifyNativeIntegerAssignment(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		for (NativeInt v : BoundaryValues<NativeInt>()) {
			EInteger a(v), b;
			b = v;
			bool fail = (ToDecimal(a) != std::to_string(v)) || (a != b) || (a.isneg() != (v < NativeInt(0))) || (a.iszero() != (v == NativeInt(0)));
			if constexpr (std::numeric_limits<NativeInt>::is_signed) {
				fail = fail || (static_cast<long long>(a) != static_cast<long long>(v));
				if constexpr (sizeof(NativeInt) <= sizeof(int)) fail = fail || (static_cast<int>(a) != static_cast<int>(v));
			}
			if (fail) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << typeid(NativeInt).name() << ' ' << std::to_string(v) << " -> " << a << '\n';
			}
		}
		return nrOfFailedTests;
	}

	// the value an IEEE floating-point v must assign: its integer part, truncated toward zero,
	// built independently from frexp and an exact shift
	template<typename BlockType, typename Real>
	einteger<BlockType> TruncatedReference(Real v) {
		using EInteger = einteger<BlockType>;
		int e = 0;
		const Real m = std::frexp(std::fabs(v), &e);   // |v| = m * 2^e, m in [0.5, 1)
		constexpr int digits = std::numeric_limits<Real>::digits;
		const unsigned long long significand = static_cast<unsigned long long>(std::ldexp(m, digits));   // exact
		EInteger r(significand);
		r <<= (e - digits);   // a negative shift is a right shift, which truncates the magnitude
		return (v < Real(0)) ? -r : r;
	}

	// IEEE floating-point -> einteger: exact integers, fractions truncated toward zero, the full
	// binade range, and NaN and infinity, which are not representable and assign zero
	template<typename BlockType, typename Real>
	int VerifyIeeeAssignment(int randomCases, bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		using Limits = std::numeric_limits<Real>;
		int nrOfFailedTests = 0;
		auto check = [&](Real v, const EInteger& ref) {
			const EInteger a(v);
			// no negative zero: -0.75 truncates to 0, which must not report isneg()
			if (a != ref || (a.iszero() && a.isneg())) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << std::setprecision(Limits::max_digits10) << v << " -> " << a << " reference " << ref << '\n';
			}
		};

		const struct { Real v; long long ref; } directed[] = {
			{ Real(0.0), 0 }, { Real(-0.0), 0 }, { Real(1.0), 1 }, { Real(-1.0), -1 },
			{ Real(0.5), 0 }, { Real(-0.5), 0 }, { Real(0.75), 0 }, { Real(-0.75), 0 }, { Limits::denorm_min(), 0 }, { -Limits::min(), 0 },
			{ Real(1.5), 1 }, { Real(2.5), 2 }, { Real(-2.5), -2 }, { Real(-3.75), -3 }, { Real(255.9375), 255 }, { Real(-65536.5), -65536 },
			{ Real(16777216.0), 16777216 }, { Real(-16777215.0), -16777215 }
		};
		for (const auto& d : directed) check(d.v, EInteger(d.ref));
		for (Real v : { Limits::quiet_NaN(), Limits::infinity(), -Limits::infinity() }) check(v, EInteger(0));

		// every power of two up to the largest binade, and the largest finite value
		for (int k = 0; k < Limits::max_exponent; ++k) {
			EInteger ref(1);
			ref <<= k;
			check(std::ldexp(Real(1), k), ref);
			check(-std::ldexp(Real(1), k), -ref);
		}
		check(Limits::max(), TruncatedReference<BlockType>(Limits::max()));
		check(-Limits::max(), TruncatedReference<BlockType>(-Limits::max()));

		// random values over the whole exponent range, fractions included
		std::mt19937_64 rng(1878);
		std::uniform_real_distribution<Real> mantissa(Real(-1), Real(1));
		std::uniform_int_distribution<int> exponent(-8, Limits::max_exponent - 1);
		for (int i = 0; i < randomCases; ++i) {
			const Real v = std::ldexp(mantissa(rng), exponent(rng));
			check(v, TruncatedReference<BlockType>(v));
		}
		return nrOfFailedTests;
	}

	// einteger -> IEEE floating-point for values the target holds exactly
	template<typename BlockType>
	int VerifyIeeeConversion(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		for (int k = 0; k < 1000; k += 7) {
			EInteger a(3);
			a <<= k;
			const double ref = std::ldexp(3.0, k);
			if (static_cast<double>(a) != ref || static_cast<double>(-a) != -ref) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: double(3 * 2^" << k << ") = " << static_cast<double>(a) << '\n';
			}
			if (k < 120 && static_cast<float>(a) != std::ldexp(3.0f, k)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: float(3 * 2^" << k << ") = " << static_cast<float>(a) << '\n';
			}
		}
		for (long long v : { 0ll, 1ll, -1ll, 9007199254740991ll, -9007199254740991ll }) {
			if (static_cast<double>(EInteger(v)) != static_cast<double>(v)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: double(" << v << ") = " << static_cast<double>(EInteger(v)) << '\n';
			}
		}
		return nrOfFailedTests;
	}

	// einteger -> float/double rounds once, to nearest with ties to even, as a native 64-bit integer
	// converts. Adding 2^i per set bit rounded at every add past 2^24 (float) or 2^53 (double), so
	// (float)(2^30 + 2^29 + 2^6 + 1) was 1610612736 instead of 1610612864 (#1987). Values wider than
	// 64 bits are checked by scaling: rounding commutes with multiplying by 2^k.
	template<typename BlockType>
	int VerifyIeeeRounding(int randomCases, bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		auto check = [&](unsigned long long u, bool negative, int scale) {
			EInteger a(u);
			a <<= scale;
			if (negative) a = -a;
			const float  fref = (negative ? -1.0f : 1.0f) * std::ldexp(static_cast<float>(u), scale);
			const double dref = (negative ? -1.0  : 1.0 ) * std::ldexp(static_cast<double>(u), scale);
			if (static_cast<float>(a) != fref || static_cast<double>(a) != dref) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << (negative ? "-" : "") << u << " * 2^" << scale << " -> float " << static_cast<float>(a) << " (expected " << fref << "), double " << static_cast<double>(a) << " (expected " << dref << ")\n";
			}
		};
		// the issue's case, ties to even in both directions, and the carry into a new binade
		const unsigned long long specific[] = {
			(1ull << 30) + (1ull << 29) + (1ull << 6) + 1,
			(1ull << 24) + 1, (1ull << 24) + 3, (1ull << 25) + 2, (1ull << 25) + 6,
			(1ull << 53) + 1, (1ull << 53) + 3, (1ull << 54) + 2, (1ull << 63) + 1025,
			0xFFFFFFull, 0x1FFFFFFull, 0xFFFFFFFFFFFFFFFFull, 0x20000000000001ull, 0x3FFFFFFFFFFFFFull };
		for (unsigned long long u : specific) {
			for (int scale : { 0, 1, 37, 64, 100 }) {
				check(u, false, scale);
				check(u, true, scale);
			}
		}
		std::mt19937_64 rng(1987);
		for (int i = 0; i < randomCases; ++i) {
			const unsigned long long u = rng() >> (rng() % 40);   // many magnitudes
			check(u, (i & 1) != 0, static_cast<int>(rng() % 120));
		}
		return nrOfFailedTests;
	}

	// 2^k as an einteger
	template<typename BlockType>
	einteger<BlockType> PowerOfTwo(int k) {
		einteger<BlockType> p(1);
		p <<= k;
		return p;
	}

	// einteger -> Real where the rounding needs every bit (#1994, #2010). With D = digits and a
	// leading bit 2^T, the bit 2^(T-D) is the round bit; a sticky bit far below it, in another limb
	// and another 64-bit word, decides the tie. The expected values are exact sums of powers of two.
	// T = 100 is the issue's example (2^100 + 2^47 + 1 to double); T = D + 70 puts the round bit in
	// the second 64-bit word and the sticky bit in the first; T = max_exponent - 1 is the top binade.
	template<typename BlockType, typename Real>
	int VerifyIeeeStickyRounding(const char* realName, bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		constexpr int D = std::numeric_limits<Real>::digits;
		constexpr int E = std::numeric_limits<Real>::max_exponent;   // 2^E is the first power that overflows
		int nrOfFailedTests = 0;
		auto check = [&](const EInteger& a, Real expected, const char* what, int T) {
			for (bool negative : { false, true }) {
				const EInteger v = negative ? -a : a;
				const Real ref = negative ? -expected : expected;
				const Real got = static_cast<Real>(v);
				if (got != ref) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << realName << '(' << (negative ? "-" : "") << what << ") with T = " << T << " is " << std::hexfloat << got << ", expected " << ref << std::defaultfloat << '\n';
				}
			}
		};
		auto p2 = [](int k) { return PowerOfTwo<BlockType>(k); };
		const Real one(1);
		for (int T : { 100, D + 70, E - 1 }) {
			if (T <= D + 1 || T >= E) continue;
			const Real top = std::ldexp(one, T);
			check(p2(T) + p2(T - D) + p2(0), top + std::ldexp(one, T - D + 1), "2^T + 2^(T-D) + 1: sticky rounds up", T);
			check(p2(T) + p2(T - D), top, "2^T + 2^(T-D): tie to even, down", T);
			check(p2(T) + p2(T - D + 1) + p2(T - D), top + std::ldexp(one, T - D + 2), "2^T + 2^(T-D+1) + 2^(T-D): tie to even, up", T);
			check(p2(T) + p2(T - D) - EInteger(1), top, "2^T + 2^(T-D) - 1: below the tie", T);
			check(p2(T) + p2(T - D + 1) + p2(0), top + std::ldexp(one, T - D + 1), "2^T + 2^(T-D+1) + 1: round bit clear", T);
			check(p2(T + 1) - EInteger(1), std::ldexp(one, T + 1), "2^(T+1) - 1: carry into the next binade", T);
		}
		if constexpr (D >= 64) {
			// a long double holds a full 64-bit integer exactly (not on MSVC or macOS ARM64: D == 53 there)
			check(p2(63) + EInteger(1), std::ldexp(one, 63) + one, "2^63 + 1", 63);
		}
		return nrOfFailedTests;
	}

	// einteger -> Real at the top of the range: the largest finite value, the round-to-nearest
	// threshold to infinity at MAX + ulp/2 (a tie with an odd MAX, so it rounds up), 2^E itself and
	// beyond. Also the other direction: Real max converts to the exact einteger.
	template<typename BlockType, typename Real>
	int VerifyIeeeOverflow(const char* realName, bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		constexpr int D = std::numeric_limits<Real>::digits;
		constexpr int E = std::numeric_limits<Real>::max_exponent;
		const Real maxpos = std::numeric_limits<Real>::max();
		const Real inf = std::numeric_limits<Real>::infinity();
		int nrOfFailedTests = 0;
		auto check = [&](const EInteger& a, Real expected, const char* what) {
			for (bool negative : { false, true }) {
				const EInteger v = negative ? -a : a;
				const Real ref = negative ? -expected : expected;
				const Real got = static_cast<Real>(v);
				if (got != ref) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << realName << '(' << (negative ? "-" : "") << what << ") is " << std::hexfloat << got << ", expected " << ref << std::defaultfloat << '\n';
				}
			}
		};
		auto p2 = [](int k) { return PowerOfTwo<BlockType>(k); };
		const EInteger MAX = p2(E) - p2(E - D);   // 2^E - 2^(E-D): D ones
		const EInteger halfUlp = p2(E - D - 1);
		check(MAX, maxpos, "MAX");
		check(MAX - EInteger(1), maxpos, "MAX - 1");
		check(MAX + halfUlp - EInteger(1), maxpos, "MAX + ulp/2 - 1: just below the threshold");
		check(MAX + halfUlp, inf, "MAX + ulp/2: the threshold, a tie that rounds to even");
		check(MAX + halfUlp + EInteger(1), inf, "MAX + ulp/2 + 1");
		check(p2(E) - EInteger(1), inf, "2^E - 1");
		check(p2(E), inf, "2^E");
		check(p2(E) + EInteger(1), inf, "2^E + 1");
		check(p2(E + 100), inf, "2^(E + 100)");
		const EInteger fromMax(maxpos);
		if (fromMax != MAX || EInteger(-maxpos) != -MAX) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: einteger(" << realName << " max) = " << fromMax << ", expected " << MAX << '\n';
		}
		return nrOfFailedTests;
	}

	// the overflow edge of one type is an ordinary value of a wider one
	template<typename BlockType>
	int VerifyIeeeCrossRange(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		const EInteger p128 = PowerOfTwo<BlockType>(128), p1024 = PowerOfTwo<BlockType>(1024);
		const EInteger fltMax(std::numeric_limits<float>::max());
		if (static_cast<double>(p128) != std::ldexp(1.0, 128) || static_cast<float>(p128) != std::numeric_limits<float>::infinity()
		    || static_cast<double>(fltMax) != static_cast<double>(std::numeric_limits<float>::max())
		    || static_cast<float>(p1024) != std::numeric_limits<float>::infinity()
		    || static_cast<double>(p1024) != std::numeric_limits<double>::infinity()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: 2^128, FLT_MAX or 2^1024 across float and double\n";
		}
#if LONG_DOUBLE_SUPPORT
		if constexpr (std::numeric_limits<long double>::max_exponent > 1024) {
			if (static_cast<long double>(p1024) != std::ldexp(1.0l, 1024)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: long double(2^1024)\n";
			}
		}
#endif
		return nrOfFailedTests;
	}

	// copy, move and self-assignment keep the value; a moved-from einteger may be reassigned
	template<typename BlockType>
	int VerifyCopyAndMove(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		EInteger ref;
		ref.assign("-123456789012345678901234567890");
		EInteger a(ref);
		EInteger b;
		b = a;
		EInteger c(std::move(a));
		EInteger d;
		d = std::move(b);
		EInteger& alias = d;
		d = alias;   // self-assignment
		a = 42;      // reassign the moved-from object
		if (c != ref || d != ref || a != EInteger(42) || ToDecimal(d) != "-123456789012345678901234567890") {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: copy/move: " << c << ' ' << d << ' ' << a << '\n';
		}
		return nrOfFailedTests;
	}

#if LONG_DOUBLE_SUPPORT
	// long double -> einteger through all of its significand (#1517)
	template<typename BlockType>
	int VerifyLongDoubleAssignment(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		constexpr int digits = std::numeric_limits<long double>::digits;
		// 2^(digits + 10) + 2^11: the low set bit is digits - 1 places below the leading bit, the last one held
		const long double v = std::ldexp(1.0l, digits + 10) + std::ldexp(1.0l, 11);
		EInteger ref(1), low(1);
		ref <<= (digits + 10);
		low <<= 11;
		ref += low;
		for (long double x : { v, -v }) {
			const EInteger a(x);
			const EInteger expected = (x < 0) ? -ref : ref;
			if (a != expected) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: long double " << x << " -> " << a << " reference " << expected << '\n';
			}
		}
		const EInteger fraction(-2.75l);
		if (fraction != EInteger(-2)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: long double -2.75 -> " << fraction << '\n';
		}
		return nrOfFailedTests;
	}
#endif

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "einteger assignment validation";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		einteger<std::uint32_t> a(1.0e20);
		std::cout << "1.0e20 -> " << a << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint8_t, short>(reportTestCases), "einteger<uint8_t>", "short");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint8_t, int>(reportTestCases), "einteger<uint8_t>", "int");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint8_t, long long>(reportTestCases), "einteger<uint8_t>", "long long");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint8_t, unsigned long long>(reportTestCases), "einteger<uint8_t>", "unsigned long long");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint32_t, int>(reportTestCases), "einteger<uint32_t>", "int");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint32_t, long>(reportTestCases), "einteger<uint32_t>", "long");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint32_t, long long>(reportTestCases), "einteger<uint32_t>", "long long");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint32_t, unsigned int>(reportTestCases), "einteger<uint32_t>", "unsigned int");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint32_t, unsigned long>(reportTestCases), "einteger<uint32_t>", "unsigned long");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeIntegerAssignment<std::uint32_t, unsigned long long>(reportTestCases), "einteger<uint32_t>", "unsigned long long");

	nrOfFailedTestCases += ReportTestResult(VerifyIeeeAssignment<std::uint8_t, float>(1000, reportTestCases), "einteger<uint8_t>", "float");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeAssignment<std::uint32_t, float>(1000, reportTestCases), "einteger<uint32_t>", "float");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeAssignment<std::uint32_t, double>(1000, reportTestCases), "einteger<uint32_t>", "double");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeConversion<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "to float/double");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeRounding<std::uint32_t>(2000, reportTestCases), "einteger<uint32_t>", "float/double rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeRounding<std::uint8_t>(500, reportTestCases), "einteger<uint8_t>", "float/double rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeStickyRounding<std::uint32_t, float>("float", reportTestCases), "einteger<uint32_t>", "float sticky rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeStickyRounding<std::uint32_t, double>("double", reportTestCases), "einteger<uint32_t>", "double sticky rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeStickyRounding<std::uint16_t, float>("float", reportTestCases), "einteger<uint16_t>", "float sticky rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeStickyRounding<std::uint16_t, double>("double", reportTestCases), "einteger<uint16_t>", "double sticky rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeStickyRounding<std::uint8_t, double>("double", reportTestCases), "einteger<uint8_t>", "double sticky rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeOverflow<std::uint32_t, float>("float", reportTestCases), "einteger<uint32_t>", "float overflow edge");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeOverflow<std::uint32_t, double>("double", reportTestCases), "einteger<uint32_t>", "double overflow edge");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeOverflow<std::uint16_t, float>("float", reportTestCases), "einteger<uint16_t>", "float overflow edge");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeOverflow<std::uint16_t, double>("double", reportTestCases), "einteger<uint16_t>", "double overflow edge");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeOverflow<std::uint8_t, float>("float", reportTestCases), "einteger<uint8_t>", "float overflow edge");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeCrossRange<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "2^128 and 2^1024");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeCrossRange<std::uint16_t>(reportTestCases), "einteger<uint16_t>", "2^128 and 2^1024");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeRounding<std::uint16_t>(500, reportTestCases), "einteger<uint16_t>", "float/double rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyAndMove<std::uint8_t>(reportTestCases), "einteger<uint8_t>", "copy/move");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyAndMove<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "copy/move");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyLongDoubleAssignment<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "long double");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeStickyRounding<std::uint32_t, long double>("long double", reportTestCases), "einteger<uint32_t>", "long double sticky rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeStickyRounding<std::uint16_t, long double>("long double", reportTestCases), "einteger<uint16_t>", "long double sticky rounding");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeOverflow<std::uint32_t, long double>("long double", reportTestCases), "einteger<uint32_t>", "long double overflow edge");
#endif
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeAssignment<std::uint8_t, double>(1000, reportTestCases), "einteger<uint8_t>", "double");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeAssignment<std::uint16_t, double>(1000, reportTestCases), "einteger<uint16_t>", "double");
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeConversion<std::uint8_t>(reportTestCases), "einteger<uint8_t>", "to float/double");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyIeeeAssignment<std::uint32_t, double>(100000, reportTestCases), "einteger<uint32_t>", "double");
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_arithmetic_exception& err) {
	std::cerr << "Uncaught einteger arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_internal_exception& err) {
	std::cerr << "Uncaught einteger internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
