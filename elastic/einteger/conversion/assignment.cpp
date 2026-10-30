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
	nrOfFailedTestCases += ReportTestResult(VerifyCopyAndMove<std::uint8_t>(reportTestCases), "einteger<uint8_t>", "copy/move");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyAndMove<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "copy/move");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyLongDoubleAssignment<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "long double");
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
