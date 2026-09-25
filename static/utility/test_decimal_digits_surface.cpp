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
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/rational/rational.hpp>
#include <universal/number/cfloat/cfloat.hpp>
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

	// The decimal exponent range, against the only independent oracle available: the
	// native types whose formats cfloat single and duble reproduce exactly. Checking all
	// four traits at once, because min_exponent10 is the one an approximation gets wrong
	// by relying on a cast to round a negative quotient the right way (#1604).
	int VerifyExponentRangeAgainstNative(bool reportTestCases) {
		using namespace sw::universal;
		int nrFailed = 0;
		if (std::numeric_limits<single>::min_exponent10 != std::numeric_limits<float>::min_exponent10
		 || std::numeric_limits<single>::max_exponent10 != std::numeric_limits<float>::max_exponent10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "single exponent10 range disagrees with native float\n";
		}
		if (std::numeric_limits<duble>::min_exponent10 != std::numeric_limits<double>::min_exponent10
		 || std::numeric_limits<duble>::max_exponent10 != std::numeric_limits<double>::max_exponent10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "duble exponent10 range disagrees with native double\n";
		}
		// hand-computed: ceil((e-1)*log10(2)) and floor(e*log10(2))
		if (decimal_min_exponent10(-125) != -37 || decimal_max_exponent10(128) != 38) {
			++nrFailed;
			if (reportTestCases) std::cerr << "exponent10 helpers wrong at binary32's range\n";
		}
		if (decimal_min_exponent10(-1021) != -307 || decimal_max_exponent10(1024) != 308) {
			++nrFailed;
			if (reportTestCases) std::cerr << "exponent10 helpers wrong at binary64's range\n";
		}
		if (decimal_min_exponent10(-16381) != -4931 || decimal_max_exponent10(16384) != 4932) {
			++nrFailed;
			if (reportTestCases) std::cerr << "exponent10 helpers wrong at binary128's range\n";
		}
		// The branches no numeric_limits specialization reaches. Every number system here
		// has min_exponent <= 1 and max_exponent >= 0, so a suite built only from real
		// types exercises one branch of each helper and the other can be inverted without
		// anything noticing -- which is exactly what had happened.
		struct EdgeCase { int in; int want; bool isMax; };
		const EdgeCase edges[] = {
			{   -1,   -1, true  },   // floor(-1 * log10 2)   = floor(-0.301)
			{   -4,   -2, true  },   // floor(-4 * log10 2)   = floor(-1.204)
			{ -125,  -38, true  },   // floor(-125 * log10 2) = floor(-37.63)
			{    2,    1, false },   // ceil(1 * log10 2)     = ceil(0.301)
			{   11,    4, false },   // ceil(10 * log10 2)    = ceil(3.010)
			{  100,   30, false },   // ceil(99 * log10 2)    = ceil(29.80)
		};
		for (const auto& e : edges) {
			const int got = e.isMax ? decimal_max_exponent10(e.in) : decimal_min_exponent10(e.in);
			if (got != e.want) {
				++nrFailed;
				if (reportTestCases) std::cerr << (e.isMax ? "decimal_max_exponent10(" : "decimal_min_exponent10(")
					<< e.in << ") = " << got << " != " << e.want << '\n';
			}
		}
		return nrFailed;
	}

	// A fixpnt whose integer field is empty tops out BELOW 1 -- fixpnt<8,7> at 127/128 --
	// so the largest representable power of ten is 10^-1. Taking the binary scale above the
	// maximum and converting it directly reports 0, which claims 1.0 is representable when
	// it is not. Reference values are floor(log10(exact maximum)), computed by hand.
	int VerifyFixpntMaxExponent10(bool reportTestCases) {
		using namespace sw::universal;
		int nrFailed = 0;
		struct Case { int got; int want; const char* tag; };
		const Case cases[] = {
			{ std::numeric_limits<fixpnt<8, 7>>::max_exponent10,   -1, "fixpnt<8,7> max 127/128"     },
			{ std::numeric_limits<fixpnt<16, 15>>::max_exponent10, -1, "fixpnt<16,15> max ~0.999969" },
			{ std::numeric_limits<fixpnt<8, 0>>::max_exponent10,    2, "fixpnt<8,0> max 127"         },
			{ std::numeric_limits<fixpnt<32, 16>>::max_exponent10,  4, "fixpnt<32,16> max 32768"     },
		};
		for (const auto& c : cases) {
			if (c.got != c.want) {
				++nrFailed;
				if (reportTestCases) std::cerr << c.tag << " max_exponent10 " << c.got << " != " << c.want << '\n';
			}
		}
		return nrFailed;
	}

	// #1608: min_exponent is the minimum n with radix^(n-1) normalized, so it is one MORE
	// than the scale of the smallest normal. Three types had it equal to that scale, or
	// to the negated bias. Each reference is the standard's definition restated against
	// the type's own min(), not a recomputation of the formula under test.
	int VerifyMinExponentAgainstSmallestNormal(bool reportTestCases) {
		using namespace sw::universal;
		int nrFailed = 0;
		struct Case { int minExp; int minExp10; int wantExp; int wantExp10; const char* tag; };
		const Case cases[] = {
			// bfloat16's smallest normal is 2^-126, the same as float's
			{ std::numeric_limits<bfloat16>::min_exponent, std::numeric_limits<bfloat16>::min_exponent10,
			  -125, -37, "bfloat16" },
			// e4m3 bias 7, smallest normal 2^-6
			{ std::numeric_limits<e4m3>::min_exponent, std::numeric_limits<e4m3>::min_exponent10,
			  -5, -1, "e4m3" },
			// posit<16,2> minpos at scale -56
			{ std::numeric_limits<posit<16, 2>>::min_exponent, std::numeric_limits<posit<16, 2>>::min_exponent10,
			  -55, -16, "posit<16,2>" },
		};
		for (const auto& c : cases) {
			if (c.minExp != c.wantExp) {
				++nrFailed;
				if (reportTestCases) std::cerr << c.tag << " min_exponent " << c.minExp << " != " << c.wantExp << '\n';
			}
			if (c.minExp10 != c.wantExp10) {
				++nrFailed;
				if (reportTestCases) std::cerr << c.tag << " min_exponent10 " << c.minExp10 << " != " << c.wantExp10 << '\n';
			}
		}
		// bfloat16 shares float's exponent field, so it must share both traits
		if (std::numeric_limits<bfloat16>::min_exponent != std::numeric_limits<float>::min_exponent
		 || std::numeric_limits<bfloat16>::min_exponent10 != std::numeric_limits<float>::min_exponent10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "bfloat16 exponent traits disagree with native float\n";
		}
		return nrFailed;
	}

	// #1602: an lns exponent range is 2^(nbits-1-rbits) and outgrows int. Narrowing it
	// silently WRAPPED -- lns<48,10> reported min_exponent 0 and max_exponent -1, which
	// claim a smallest normal of 2^-1. Saturating is the most the int-typed trait can say.
	int VerifyLnsExponentSaturates(bool reportTestCases) {
		using namespace sw::universal;
		int nrFailed = 0;
		constexpr int intMin = -2147483647 - 1;
		constexpr int intMax = 2147483647;

		// configurations that fit report the exact value
		if (std::numeric_limits<lns<8, 3>>::min_exponent != -8
		 || std::numeric_limits<lns<32, 8>>::min_exponent != -4194304) {
			++nrFailed;
			if (reportTestCases) std::cerr << "lns narrow configurations no longer report their exact exponent\n";
		}
		// configurations that do not fit saturate rather than wrap
		if (std::numeric_limits<lns<48, 10>>::min_exponent != intMin
		 || std::numeric_limits<lns<48, 10>>::max_exponent != intMax) {
			++nrFailed;
			if (reportTestCases) std::cerr << "lns<48,10> exponent does not saturate\n";
		}
		if (std::numeric_limits<lns<64, 11>>::min_exponent != intMin
		 || std::numeric_limits<lns<64, 11>>::max_exponent != intMax) {
			++nrFailed;
			if (reportTestCases) std::cerr << "lns<64,11> exponent does not saturate\n";
		}
		// the specific wrapped values that prompted this, as a guard against regressing
		if (std::numeric_limits<lns<48, 10>>::min_exponent == 0
		 || std::numeric_limits<lns<48, 10>>::max_exponent == -1) {
			++nrFailed;
			if (reportTestCases) std::cerr << "lns<48,10> exponent wrapped to the 0/-1 pair again\n";
		}
		// the true value stays available on the type itself
		if (lns<48, 10>::min_exponent != -68719476736LL) {
			++nrFailed;
			if (reportTestCases) std::cerr << "lns<48,10>::min_exponent (int64) is not the true exponent\n";
		}
		// The decimal traits must be derived from the WIDE exponent, not from the
		// saturated binary one. lns<35,1> has a binary exponent of -2^32, which does not
		// fit an int, and a decimal exponent of -1292913986, which does. Deriving the
		// decimal from the clamped binary value reports -646456993 -- off by a factor of
		// two, and needlessly, since the right answer was representable.
		if (std::numeric_limits<lns<35, 1>>::min_exponent10 != -1292913986) {
			++nrFailed;
			if (reportTestCases) std::cerr << "lns<35,1> min_exponent10 "
				<< std::numeric_limits<lns<35, 1>>::min_exponent10 << " != -1292913986 "
				<< "(derived from the saturated binary exponent rather than the wide one)\n";
		}
		// and where the decimal genuinely does not fit either, it saturates rather than
		// reporting the halved value
		if (std::numeric_limits<lns<48, 10>>::min_exponent10 != intMin) {
			++nrFailed;
			if (reportTestCases) std::cerr << "lns<48,10> min_exponent10 does not saturate\n";
		}
		// the helpers themselves, on wide input
		if (decimal_min_exponent10(-4294967296LL) != -1292913986) {
			++nrFailed;
			if (reportTestCases) std::cerr << "decimal_min_exponent10 wrong on a wide exponent\n";
		}
		if (decimal_max_exponent10(4294967296LL) != 1292913986) {
			++nrFailed;
			if (reportTestCases) std::cerr << "decimal_max_exponent10 wrong on a wide exponent\n";
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

	// bfloat16: digits counts the implicit bit, as float's 24 does for 23 fraction bits (#1603)
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<bfloat16>("bfloat16", 8, 2, 4, reportTestCases),
		test_tag, "bfloat16 (digits includes the implicit bit)");

	// lns / dbns: digits is the stored exponent's resolution, rbits + 1, not the
	// exponent RANGE -- lns<32,8> used to report 4194312 significand bits (#1602)
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<lns<8, 3>>("lns<8,3>", 4, 0, 3, reportTestCases),
		test_tag, "lns<8,3>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<lns<16, 5>>("lns<16,5>", 6, 1, 3, reportTestCases),
		test_tag, "lns<16,5>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<lns<32, 8>>("lns<32,8>", 9, 2, 4, reportTestCases),
		test_tag, "lns<32,8>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<dbns<8, 3>>("dbns<8,3>", 4, 0, 3, reportTestCases),
		test_tag, "dbns<8,3>");

	// fixpnt declares is_exact false, so it takes the floating-point pair (#1601)
	nrOfFailedTestCases += ReportTestResult(
		VerifyFloatDigits<fixpnt<32, 16>>("fixpnt<32,16>", 31, 9, 11, reportTestCases),
		test_tag, "fixpnt<32,16>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyFixpntMaxExponent10(reportTestCases),
		test_tag, "fixpnt max_exponent10 below 1");

	// rational declares is_exact and is_integer, so it takes the integer pair (#1601)
	nrOfFailedTestCases += ReportTestResult(
		VerifyIntegerDigits<rational<32>>("rational<32>", 32, 9, reportTestCases),
		test_tag, "rational<32>");

	// the decimal exponent range (#1604)
	nrOfFailedTestCases += ReportTestResult(
		VerifyExponentRangeAgainstNative(reportTestCases),
		test_tag, "decimal exponent range");

	// min_exponent against each type's own smallest normal (#1608)
	nrOfFailedTestCases += ReportTestResult(
		VerifyMinExponentAgainstSmallestNormal(reportTestCases),
		test_tag, "min_exponent vs smallest normal");

	// an exponent range that outgrows int saturates rather than wrapping (#1602)
	nrOfFailedTestCases += ReportTestResult(
		VerifyLnsExponentSaturates(reportTestCases),
		test_tag, "lns exponent saturation");
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
