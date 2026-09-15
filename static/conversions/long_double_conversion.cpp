// long_double_conversion.cpp: conversion between long double and the Universal number types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/rational/rational.hpp>
#include <universal/number/einteger/einteger.hpp>
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Assignment from a long double was wrong on every platform whose long double is not x87 (#1515):
   on aarch64 Linux (IEEE binary128) 0.75 became 0.5 and 3 became 2, on POWER (IBM double-double)
   0.75 became 0, in cfloat, posit, fixpnt, lns, dbns, rational, einteger, edecimal and erational.
   extractFields() read those formats' bits with the x87 decoder. It now computes x87-shaped fields
   from the value there, the significand cut to 64 bits with the rest folded into the last bit
   (round to odd), so that a type rounding it to 62 bits or fewer rounds correctly. On POWER, lns
   and fixpnt also converted TO long double wrongly (every lns to 1): their starting weight was
   ieee754_parameter<long double>::minNormal, x87's 2^-16382, which double-double holds as 0.

   Checked here, on whatever long double the platform has:
   - every type: a value a long double and a double both hold assigns the same from either;
   - cfloat (float's and double's layout), posit and fixpnt: a long double one bit past a rounding
     tie, or at it, rounds as the tie rule says. The bit sits at the long double's last place, so
     on binary128 and double-double it lies below the 64-bit cut and reaches the type only through
     the round-to-odd bit; the expected results are exact values, built by construction;
   - a signalling long double NaN stays signalling in a cfloat. x87's qnanmask used to be double's
     pattern, which classified every x87 NaN with a payload as quiet;
   - fixpnt and lns values that are exact in a long double convert to it exactly.
*/

namespace sw {
namespace universal {

namespace long_double_conversion {

#if LONG_DOUBLE_SUPPORT

struct Failures {
	int  count = 0;
	bool report;
	explicit Failures(bool reportTestCases) : report(reportTestCases) {}
	void fail(const std::string& what) {
		if (report || count < 8)
			std::cerr << "FAIL: " << what << '\n';
		++count;
	}
};

inline std::string Hex(long double v) {
	char buf[64];
	std::snprintf(buf, sizeof(buf), "%La", v);
	return buf;
}

// a value both a long double and a double hold assigns the same from either; the adaptive types
// may reduce a fraction differently, so equal values count as the same
template<typename T>
void VerifyExact(const std::string& name, Failures& f) {
	for (double d : {0.75, 3.0, -2.5, 1024.5, 5.0, 12.0, -0.375, 100.0}) {
		T fromLong{}, fromDouble{};
		fromLong   = static_cast<long double>(d);
		fromDouble = d;
		if (!(fromLong == fromDouble) && double(fromLong) != double(fromDouble))
			f.fail(name + " = " + std::to_string(d) + "l gave " + std::to_string(double(fromLong)) + ", from the double " +
			       std::to_string(double(fromDouble)));
	}
}

inline int VerifyExactValues(bool reportTestCases) {
	Failures f(reportTestCases);
	VerifyExact<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat<32,8>", f);
	VerifyExact<cfloat<64, 11, std::uint64_t, true, false, false>>("cfloat<64,11>", f);
	VerifyExact<cfloat<128, 15, std::uint32_t, true, false, false>>("cfloat<128,15>", f);
	VerifyExact<posit<32, 2>>("posit<32,2>", f);
	VerifyExact<lns<16, 8, std::uint16_t>>("lns<16,8>", f);
	VerifyExact<dbns<8, 3, std::uint8_t>>("dbns<8,3>", f);
	VerifyExact<fixpnt<32, 16>>("fixpnt<32,16>", f);
	VerifyExact<takum<32>>("takum<32>", f);
	VerifyExact<areal<32, 8, std::uint32_t>>("areal<32,8>", f);
	VerifyExact<rational<32>>("rational<32>", f);
	VerifyExact<einteger<std::uint32_t>>("einteger", f);
	VerifyExact<edecimal>("edecimal", f);
	VerifyExact<erational>("erational", f);
	return f.count;
}

// Rounding a long double at a tie of a type whose unit in the last place near 1 is 2^-q. The
// sticky bit 2^t is the long double's last place, t = 2 - digits: past the 64-bit cut on binary128
// and double-double. The cases need t < -(q + 1), so that the sticky bit lies below the tie.
template<typename T>
void VerifyTies(const std::string& name, int q, Failures& f) {
	constexpr int digits = std::numeric_limits<long double>::digits;
	const int     t      = 2 - digits;
	if (t >= -(q + 1))
		return;  // the long double has no bit below this type's tie
	const long double ulp = std::ldexp(1.0l, -q), half = ulp / 2.0l, sticky = std::ldexp(1.0l, t);
	struct Case {
		long double x, expected;
	};
	const Case cases[] = {
	    {1.0l + half, 1.0l},                          // tie, even below
	    {1.0l + half + sticky, 1.0l + ulp},           // just past the tie: only the sticky bit says so
	    {1.0l + half - sticky, 1.0l},                 // just short of it
	    {1.0l + ulp + half, 1.0l + 2.0l * ulp},       // tie, even above
	    {1.0l + ulp + half - sticky, 1.0l + ulp},     // just short of it
	    {1.0l + ulp + sticky, 1.0l + ulp},            // far from a tie
	};
	for (const Case& c : cases) {
		for (int sign : {1, -1}) {
			T got{}, expected{};
			got      = sign * c.x;
			expected = static_cast<double>(sign * c.expected);  // exact in a double: 1 + 2 ulp needs q + 1 bits
			if (!(got == expected))
				f.fail(name + " = " + Hex(sign * c.x) + " gave " + std::to_string(double(got)) + ", expected " +
				       Hex(sign * c.expected));
		}
	}
}

inline int VerifyRounding(bool reportTestCases) {
	Failures f(reportTestCases);
	VerifyTies<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat<32,8>", 23, f);
	VerifyTies<cfloat<64, 11, std::uint64_t, true, false, false>>("cfloat<64,11>", 52, f);
	VerifyTies<posit<32, 2>>("posit<32,2>", 27, f);  // near 1: 32 - sign - regime 2 - es 2 = 27 fraction bits
	VerifyTies<fixpnt<32, 16>>("fixpnt<32,16>", 16, f);
	return f.count;
}

// a signalling NaN stays signalling, a quiet one quiet; inf stays inf
inline int VerifySpecials(bool reportTestCases) {
	using C = cfloat<32, 8, std::uint32_t, true, false, false>;
	Failures f(reportTestCases);
	C        q, s, i, n;
	q = std::numeric_limits<long double>::quiet_NaN();
	s = std::numeric_limits<long double>::signaling_NaN();
	i = std::numeric_limits<long double>::infinity();
	n = -std::numeric_limits<long double>::infinity();
	if (!q.isnan(NAN_TYPE_QUIET)) f.fail("cfloat = quiet NaN long double is " + to_binary(q));
	if (!s.isnan(NAN_TYPE_SIGNALLING)) f.fail("cfloat = signalling NaN long double is " + to_binary(s));
	if (!i.isinf(INF_TYPE_POSITIVE)) f.fail("cfloat = +inf long double is " + to_binary(i));
	if (!n.isinf(INF_TYPE_NEGATIVE)) f.fail("cfloat = -inf long double is " + to_binary(n));
	return f.count;
}

// values a long double holds exactly convert to it exactly
inline int VerifyToLongDouble(bool reportTestCases) {
	Failures f(reportTestCases);
	for (double d : {0.75, -2.5, 1024.5, 0.0625}) {
		fixpnt<32, 16> x(d);
		if (static_cast<long double>(x) != static_cast<long double>(d))
			f.fail("long double(fixpnt<32,16>(" + std::to_string(d) + ")) is " + Hex(static_cast<long double>(x)));
	}
	for (double d : {0.5, 4.0, -8.0, 0.25}) {  // powers of two: exact in the log domain
		lns<16, 8, std::uint16_t> x(d);
		if (static_cast<long double>(x) != static_cast<long double>(d))
			f.fail("long double(lns<16,8>(" + std::to_string(d) + ")) is " + Hex(static_cast<long double>(x)));
	}
	return f.count;
}

#endif  // LONG_DOUBLE_SUPPORT

}  // namespace long_double_conversion

}  // namespace universal
}  // namespace sw

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
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

	std::string test_suite          = "conversion between long double and Universal number types";
	std::string test_tag            = "long double";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if LONG_DOUBLE_SUPPORT
	using namespace sw::universal::long_double_conversion;
	std::cout << "long double: " << std::numeric_limits<long double>::digits << " significand bits\n";

#	if MANUAL_TESTING

	cfloat<32, 8, std::uint32_t, true, false, false> c;
	c = 0.75l;
	std::cout << "cfloat<32,8> = 0.75l : " << c << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyRounding(true), "rounding", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#	else

#		if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExactValues(reportTestCases), "exact values", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRounding(reportTestCases), "rounding at ties", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecials(reportTestCases), "nan and inf", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToLongDouble(reportTestCases), "to long double", test_tag);
#		endif

#		if REGRESSION_LEVEL_2
#		endif

#		if REGRESSION_LEVEL_3
#		endif

#		if REGRESSION_LEVEL_4
#		endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#	endif  // MANUAL_TESTING
#else
	std::cout << test_suite << ": LONG_DOUBLE_SUPPORT is 0, nothing to test\n";
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#endif  // LONG_DOUBLE_SUPPORT
} catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
