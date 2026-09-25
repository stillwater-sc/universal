// test_decimal_digits_surface.cpp: numeric_limits digits10 / max_digits10 across number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/areal/areal.hpp>
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/unum/unum.hpp>
#include <universal/verification/test_suite.hpp>

// #1597 fixed cfloat; #1601 extends it to the number systems whose `digits` is already a
// correct count. Two different formulas apply, and using the wrong one is how this trait
// went wrong in the first place:
//
//   binary floating-point, digits = significand bits including the implicit one
//     digits10     = floor((digits - 1) * log10(2))
//     max_digits10 = ceil(digits * log10(2)) + 1
//
//   integer, digits = value bits excluding the sign
//     digits10     = floor(digits * log10(2))     -- no -1: no rounding step to survive
//     max_digits10 = 0                            -- no decimal round trip to size
//
// Reference values are hand-computed from those definitions rather than recomputed with
// the helpers under test, since a test that reruns the implementation's own formula cannot
// detect a wrong formula. Where a configuration mirrors a native type, it is additionally
// required to agree with that native type.

#define MANUAL_TESTING 0

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING
// is a config that can be used to run a single test at a time.
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

namespace {

	// A binary floating-point type: digits is the significand width including the implicit bit.
	template<typename Real>
	int VerifyFloatDigits(const std::string& tag, int refDigits, int refDigits10, int refMaxDigits10, bool reportTestCases) {
		using L = std::numeric_limits<Real>;
		int nrFailed = 0;
		if (L::digits != refDigits) {
			++nrFailed;
			if (reportTestCases) std::cerr << tag << " digits " << L::digits << " != " << refDigits << '\n';
		}
		if (L::digits10 != refDigits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << tag << " digits10 " << L::digits10 << " != " << refDigits10 << '\n';
		}
		if (L::max_digits10 != refMaxDigits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << tag << " max_digits10 " << L::max_digits10 << " != " << refMaxDigits10 << '\n';
		}
		return nrFailed;
	}

	// An integer type: digits is the value-bit count, and max_digits10 is 0.
	template<typename Integer>
	int VerifyIntegerDigits(const std::string& tag, int refDigits, int refDigits10, bool reportTestCases) {
		using L = std::numeric_limits<Integer>;
		int nrFailed = 0;
		if (L::digits != refDigits) {
			++nrFailed;
			if (reportTestCases) std::cerr << tag << " digits " << L::digits << " != " << refDigits << '\n';
		}
		if (L::digits10 != refDigits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << tag << " digits10 " << L::digits10 << " != " << refDigits10 << '\n';
		}
		if (L::max_digits10 != 0) {
			++nrFailed;
			if (reportTestCases) std::cerr << tag << " max_digits10 " << L::max_digits10 << " != 0\n";
		}
		return nrFailed;
	}

	// Configurations that mirror a native type must report what that native type reports.
	// This is the substitutability property, and it is what caught #1597.
	int VerifyAgreementWithNative(bool reportTestCases) {
		using namespace sw::universal;
		int nrFailed = 0;

		// areal<32,8> and areal<64,11> carry binary32's and binary64's field widths
		if (std::numeric_limits<areal<32, 8>>::digits10 != std::numeric_limits<float>::digits10
		 || std::numeric_limits<areal<32, 8>>::max_digits10 != std::numeric_limits<float>::max_digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "areal<32,8> disagrees with native float\n";
		}
		if (std::numeric_limits<areal<64, 11>>::digits10 != std::numeric_limits<double>::digits10
		 || std::numeric_limits<areal<64, 11>>::max_digits10 != std::numeric_limits<double>::max_digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "areal<64,11> disagrees with native double\n";
		}
		// integer<N> against the native integer of the same value-bit count
		if (std::numeric_limits<integer<16>>::digits10 != std::numeric_limits<std::int16_t>::digits10
		 || std::numeric_limits<integer<16>>::max_digits10 != std::numeric_limits<std::int16_t>::max_digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "integer<16> disagrees with native int16_t\n";
		}
		if (std::numeric_limits<integer<32>>::digits10 != std::numeric_limits<std::int32_t>::digits10
		 || std::numeric_limits<integer<32>>::max_digits10 != std::numeric_limits<std::int32_t>::max_digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "integer<32> disagrees with native int32_t\n";
		}
		if (std::numeric_limits<integer<64>>::digits10 != std::numeric_limits<std::int64_t>::digits10
		 || std::numeric_limits<integer<64>>::max_digits10 != std::numeric_limits<std::int64_t>::max_digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "integer<64> disagrees with native int64_t\n";
		}
		return nrFailed;
	}

} // anonymous namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "numeric_limits decimal digits across number systems (#1601)";
	std::string test_tag    = "digits10";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// one of each shape, so manual mode exercises both formulas and the native check
	nrOfFailedTestCases += VerifyFloatDigits<areal<32, 8>>("areal<32,8>", 24, 6, 9, reportTestCases);
	nrOfFailedTestCases += VerifyIntegerDigits<integer<32>>("integer<32>", 31, 9, reportTestCases);
	nrOfFailedTestCases += VerifyAgreementWithNative(reportTestCases);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#else

#if REGRESSION_LEVEL_1
	// areal: digits = nbits - es, a faithful float with IEEE field widths
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<areal<16, 5>>("areal<16,5>", 11, 3, 5, reportTestCases),
		test_tag, "areal<16,5>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<areal<32, 8>>("areal<32,8>", 24, 6, 9, reportTestCases),
		test_tag, "areal<32,8>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<areal<64, 11>>("areal<64,11>", 53, 15, 17, reportTestCases),
		test_tag, "areal<64,11>");

	// microfloat: digits = fbits + 1
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<e4m3>("e4m3", 4, 0, 3, reportTestCases),
		test_tag, "microfloat e4m3");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<e5m2>("e5m2", 3, 0, 2, reportTestCases),
		test_tag, "microfloat e5m2");

	// posit: digits is the significand width at scale 1 -- posits are tapered, so this
	// describes the format near 1.0 and not at the extremes of the dynamic range
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<posit<16, 2>>("posit<16,2>", 12, 3, 5, reportTestCases),
		test_tag, "posit<16,2>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<posit<32, 2>>("posit<32,2>", 28, 8, 10, reportTestCases),
		test_tag, "posit<32,2>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<posit<64, 2>>("posit<64,2>", 60, 17, 20, reportTestCases),
		test_tag, "posit<64,2>");

	// integer: the other formula entirely
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerDigits<integer<16>>("integer<16>", 15, 4, reportTestCases),
		test_tag, "integer<16>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerDigits<integer<32>>("integer<32>", 31, 9, reportTestCases),
		test_tag, "integer<32>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerDigits<integer<64>>("integer<64>", 63, 18, reportTestCases),
		test_tag, "integer<64>");

	// the substitutability property
	nrOfFailedTestCases += ReportTestResult(
		VerifyAgreementWithNative(reportTestCases),
		test_tag, "agreement with the native types");
#endif

#if REGRESSION_LEVEL_2
	// unum Type I: digits = 2^fsizesize, the widest fraction plus the implicit bit
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<unum<3, 4>>("unum<3,4>", 16, 4, 6, reportTestCases),
		test_tag, "unum<3,4>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<unum<2, 2>>("unum<2,2>", 4, 0, 3, reportTestCases),
		test_tag, "unum<2,2>");

	// a wide integer, past what a native type offers
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerDigits<integer<128>>("integer<128>", 127, 38, reportTestCases),
		test_tag, "integer<128>");
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
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
