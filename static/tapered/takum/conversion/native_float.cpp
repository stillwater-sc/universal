// native_float.cpp: conversion from every native floating-point type, both takum variants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The existing conversion suites drive float and double.  Nothing converted from
// a long double, and that gap hid a total failure: takum<>::convert_ieee754()
// assumed extractFields() routes long double through double, which it does only
// when LONG_DOUBLE_DOWNCAST is defined.  Everywhere else -- Linux and macOS
// x86-64 with gcc or clang, so most of the CI matrix -- it returns genuine x87
// 80-bit fields, biased by 16383 instead of 1023.  Read with double's bias, every
// finite value landed past max_characteristic():
//
//     takum<32,3> x = 3.0L;   // gave 5.7896e+76, i.e. maxpos
//
// It was silent, it applied to every finite long double, and it survived the full
// 11-platform matrix because no test exercised the type.
//
// So this suite is deliberately shaped around the TYPE surface rather than around
// values: for each native floating-point type, and both variants, check that a
// value converts to itself.  A test organised that way would have caught it on
// the day it was written.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cfloat>
#include <cstdint>
#include <universal/number/takum/takum_log.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Values every configuration under test can represent, so a failure means the
// conversion is broken rather than the value being out of range.
template<typename Real>
struct probes {
	static constexpr Real value[] = {
		Real(1), Real(2), Real(3), Real(0.5), Real(0.25), Real(-1), Real(-2),
		Real(-0.75), Real(16), Real(-16), Real(1024), Real(0.001953125)
	};
	static constexpr size_t count = sizeof(value) / sizeof(value[0]);
};

// Is a value inside what this configuration can hold?  Narrow regime fields have
// a very small characteristic range -- takum<24,1> tops out around 2^2 -- so a
// probe outside it saturates, which says nothing about the conversion.
template<typename TakumType>
bool in_range(double d) {
	if (d == 0.0) return true;
	const double a = std::fabs(d);
	return a <= double(std::numeric_limits<TakumType>::max())
	    && a >= double(std::numeric_limits<TakumType>::min());
}

// Converting from Real must agree, BIT FOR BIT, with converting from the same
// value as a double.  That equality is the assertion that matters here: it is
// exact, needs no tolerance, and is precisely what the long double bug violated.
//
// The magnitude check that follows is only a smoke test against a conversion that
// agrees with itself while being uniformly wrong -- maxpos for everything, as the
// bug produced.  Its tolerance is deliberately loose and fixed: precision is the
// business of the conversion suites, not of this one, and a tight bound here would
// only encode each configuration's resolution twice.
//
// It is NOT derived from numeric_limits<>::epsilon(), which currently returns zero
// for several configurations -- takum<32,2>, takum<64,3>, takum_log<64,3>,
// takum<24,1> -- because it is computed as (++one) - one through a double, and any
// takum finer than a double's resolution collapses that to zero.  That is a real
// defect, but a separate one, and building this check on it would have made this
// suite fail for the wrong reason.
template<typename TakumType, typename Real>
int VerifyFromNativeFloat(const char* realName, bool reportTestCases) {
	int nrOfFailedTests = 0;
	constexpr double tolerance = 1.0e-2;

	for (size_t i = 0; i < probes<Real>::count; ++i) {
		const Real v = probes<Real>::value[i];
		const double d = static_cast<double>(v);
		if (!in_range<TakumType>(d)) continue;

		TakumType viaReal{ v };
		TakumType viaDouble{ d };

		if (viaReal.raw_bits() != viaDouble.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL from " << realName << " value=" << d
				          << " gave " << double(viaReal)
				          << ", from double gave " << double(viaDouble) << '\n';
			}
			continue;
		}
		const double back = double(viaReal);
		const double rel  = (d == 0.0) ? std::fabs(back) : std::fabs((back - d) / d);
		if (!(rel < tolerance)) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL from " << realName << " value=" << d
				          << " round-tripped to " << back << " (relerr " << rel
				          << ", tolerance " << tolerance << ")\n";
			}
		}
	}

	// Assignment must behave as construction does; the bug was reachable through
	// both, and through the assignment operator in particular.
	for (size_t i = 0; i < probes<Real>::count; ++i) {
		const Real v = probes<Real>::value[i];
		if (!in_range<TakumType>(static_cast<double>(v))) continue;
		TakumType assigned;
		assigned = v;
		TakumType constructed{ v };
		if (assigned.raw_bits() != constructed.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL assignment from " << realName << " disagrees with construction at value="
				          << double(v) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// A value must survive a trip out to a native float and back.  The long double
// round trip was the sharpest form of the bug: the outbound direction was fine,
// so takum -> long double -> takum silently replaced the value with maxpos.
template<typename TakumType, typename Real>
int VerifyRoundTripThroughNative(const char* realName, bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (size_t i = 0; i < probes<double>::count; ++i) {
		if (!in_range<TakumType>(probes<double>::value[i])) continue;
		TakumType original{ probes<double>::value[i] };
		if (original.iszero() || original.isnar()) continue;
		const Real intermediate = static_cast<Real>(original);
		TakumType returned{ intermediate };
		if (returned.raw_bits() != original.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL round trip through " << realName << ": "
				          << double(original) << " -> " << double(returned) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// Special values must survive every native type as well.
template<typename TakumType, typename Real>
int VerifySpecialsFromNative(const char* realName, bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto fail = [&](const char* what) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << what << " from " << realName << '\n';
	};
	TakumType zero{ Real(0) };
	if (!zero.iszero()) fail("zero");
	if (std::numeric_limits<Real>::has_quiet_NaN) {
		TakumType nan{ std::numeric_limits<Real>::quiet_NaN() };
		if (!nan.isnar()) fail("NaN should map to NaR");
	}
	if (std::numeric_limits<Real>::has_infinity) {
		TakumType inf{ std::numeric_limits<Real>::infinity() };
		if (!inf.isnar()) fail("infinity should map to NaR");
		TakumType ninf{ -std::numeric_limits<Real>::infinity() };
		if (!ninf.isnar()) fail("-infinity should map to NaR");
	}
	return nrOfFailedTests;
}

// Run the whole surface for one takum configuration.
template<typename TakumType>
int VerifyNativeFloatSurface(bool reportTestCases) {
	int n = 0;
	n += VerifyFromNativeFloat<TakumType, float>("float", reportTestCases);
	n += VerifyFromNativeFloat<TakumType, double>("double", reportTestCases);
	n += VerifyRoundTripThroughNative<TakumType, float>("float", reportTestCases);
	n += VerifyRoundTripThroughNative<TakumType, double>("double", reportTestCases);
	n += VerifySpecialsFromNative<TakumType, float>("float", reportTestCases);
	n += VerifySpecialsFromNative<TakumType, double>("double", reportTestCases);
#if LONG_DOUBLE_SUPPORT
	n += VerifyFromNativeFloat<TakumType, long double>("long double", reportTestCases);
	n += VerifyRoundTripThroughNative<TakumType, long double>("long double", reportTestCases);
	n += VerifySpecialsFromNative<TakumType, long double>("long double", reportTestCases);
#endif
	return n;
}

} // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "takum conversion from native floating-point types";
	std::string test_tag    = "native float";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if !LONG_DOUBLE_SUPPORT
	std::cout << "note: long double support is off, so those cases are not exercised here\n";
#endif

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum<32, 3>>(true), "takum<32,3>", "native float conversion");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum<32, 3>>(reportTestCases), "takum<32,3>", "native float conversion");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum_log<32, 3>>(reportTestCases), "takum_log<32,3>", "native float conversion");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum<16, 3>>(reportTestCases), "takum<16,3>", "native float conversion");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum_log<16, 3>>(reportTestCases), "takum_log<16,3>", "native float conversion");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum<32, 2>>(reportTestCases), "takum<32,2>", "native float conversion");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum<64, 3>>(reportTestCases), "takum<64,3>", "native float conversion");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum_log<64, 3>>(reportTestCases), "takum_log<64,3>", "native float conversion");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum<24, 1>>(reportTestCases), "takum<24,1>", "native float conversion");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNativeFloatSurface<takum_log<24, 1>>(reportTestCases), "takum_log<24,1>", "native float conversion");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
