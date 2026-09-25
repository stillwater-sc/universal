// decimal_digits.cpp: numeric_limits digits10 / max_digits10 for classic floating-point types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable support for native literals in logic and arithmetic operations
#define CFLOAT_ENABLE_LITERALS 1
// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>   // std::isinf, in the xtndd range check

// numeric_limits<T>::digits10 and max_digits10 were computed as digits/3.3, which is off
// by one at widths that matter: single reported 7 where native float reports 6, and duble
// reported 16 where double reports 15 (#1597).
//
// The standard definitions are
//     digits10     = floor((digits - 1) * log10(2))
//     max_digits10 = ceil(digits * log10(2)) + 1
// and the reference values below are those formulas evaluated by hand for the IEEE-754
// interchange formats, NOT recomputed with the same helper the code under test uses --
// a test that recomputes the implementation's own formula cannot detect a wrong formula.

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

	// Check one cfloat configuration against hand-computed reference values.
	template<typename Cfloat>
	int VerifyDecimalDigits(const std::string& tag, int refDigits, int refDigits10, int refMaxDigits10, bool reportTestCases) {
		using namespace sw::universal;
		using L = std::numeric_limits<Cfloat>;
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

	// The two configurations that mirror a native type must agree with that native type.
	// This is the property a caller reaching for `single` as a drop-in `float` depends on.
	int VerifyAgreementWithNative(bool reportTestCases) {
		using namespace sw::universal;
		int nrFailed = 0;

		if (std::numeric_limits<single>::digits10 != std::numeric_limits<float>::digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "single digits10 disagrees with native float\n";
		}
		if (std::numeric_limits<single>::max_digits10 != std::numeric_limits<float>::max_digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "single max_digits10 disagrees with native float\n";
		}
		if (std::numeric_limits<duble>::digits10 != std::numeric_limits<double>::digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "duble digits10 disagrees with native double\n";
		}
		if (std::numeric_limits<duble>::max_digits10 != std::numeric_limits<double>::max_digits10) {
			++nrFailed;
			if (reportTestCases) std::cerr << "duble max_digits10 disagrees with native double\n";
		}
		return nrFailed;
	}

#if REGRESSION_LEVEL_2
	// The helper itself, at the boundaries where an integer approximation would drift.
	int VerifyHelperBoundaries(bool reportTestCases) {
		using namespace sw::universal;
		int nrFailed = 0;
		// digits below 2 are degenerate: a format with one significand bit carries no
		// decimal digit, and the helpers return 0 rather than a negative count.
		if (decimal_digits10(1) != 0 || decimal_digits10(0) != 0 || decimal_digits10(-1) != 0) {
			++nrFailed;
			if (reportTestCases) std::cerr << "decimal_digits10 below 2 is not 0\n";
		}
		if (decimal_max_digits10(0) != 0 || decimal_max_digits10(-1) != 0) {
			++nrFailed;
			if (reportTestCases) std::cerr << "decimal_max_digits10 below 1 is not 0\n";
		}

		struct Case { int digits; int d10; int md10; };
		// hand-computed from floor((d-1)*log10(2)) and ceil(d*log10(2))+1
		constexpr Case cases[] = {
			{   2,   0,   2 },
			{  11,   3,   5 },   // binary16
			{  24,   6,   9 },   // binary32
			{  53,  15,  17 },   // binary64
			{  64,  18,  21 },   // x87 significand
			{ 113,  33,  36 },   // binary128
			{ 237,  71,  73 },   // binary256
			{ 424, 127, 129 },   // ereal<8, double>
			{1007, 302, 305 },   // ereal<19, double>
			{1696, 510, 512 },   // elreal<double> at the default 32 blocks
			// The boundary where a narrower single-multiply ratio stopped being exact.
			// cfloat<325162, 15> instantiates, so these widths are reachable, and the
			// earlier approximation returned max_digits10 = 97880 at 325147 and
			// digits10 = 97878 at 325148 -- both one too small.
			{325146, 97878, 97880 },
			{325147, 97878, 97881 },
			{325148, 97879, 97881 },
			// The far end of the domain, where the product would overflow if it were
			// formed in one multiplication rather than two halves.
			{2147483647, 646456992, 646456994 },
		};
		for (const auto& c : cases) {
			if (decimal_digits10(c.digits) != c.d10) {
				++nrFailed;
				if (reportTestCases) std::cerr << "decimal_digits10(" << c.digits << ") = "
					<< decimal_digits10(c.digits) << " != " << c.d10 << '\n';
			}
			if (decimal_max_digits10(c.digits) != c.md10) {
				++nrFailed;
				if (reportTestCases) std::cerr << "decimal_max_digits10(" << c.digits << ") = "
					<< decimal_max_digits10(c.digits) << " != " << c.md10 << '\n';
			}
		}
		return nrFailed;
	}
#endif

#if REGRESSION_LEVEL_2
	// #1599: xtndd is the x87 80-bit format, so es must be 15. At es = 11 it had double's
	// exponent range in an 80-bit container and overflowed to inf everywhere above 1.8e308
	// that a real x87 long double is finite. The reference values are x87's, and on a host
	// whose long double IS x87 they are additionally required to match it.
	int VerifyXtnddIsX87(bool reportTestCases) {
		using namespace sw::universal;
		using L = std::numeric_limits<xtndd>;
		int nrFailed = 0;

		if (xtndd::es != 15) {
			++nrFailed;
			if (reportTestCases) std::cerr << "xtndd es " << xtndd::es << " != 15 (not the x87 shape)\n";
		}
		if (xtndd::fbits != 64) {
			++nrFailed;
			if (reportTestCases) std::cerr << "xtndd fbits " << xtndd::fbits << " != 64\n";
		}
		if (L::min_exponent != -16381 || L::max_exponent != 16384) {
			++nrFailed;
			if (reportTestCases) std::cerr << "xtndd exponent range [" << L::min_exponent << ","
				<< L::max_exponent << "] != x87's [-16381,16384]\n";
		}
		if (L::min_exponent10 != -4931 || L::max_exponent10 != 4932) {
			++nrFailed;
			if (reportTestCases) std::cerr << "xtndd decimal range [" << L::min_exponent10 << ","
				<< L::max_exponent10 << "] != x87's [-4931,4932]\n";
		}
		// maxpos must sit at x87's top binade. Asserting instead that double(maxpos) is inf
		// does NOT discriminate the defect: cfloat<80,11> carries 68 fraction bits, so its
		// maxpos (2 - 2^-68) * 2^1023 also exceeds double's (2 - 2^-52) * 2^1023 and also
		// converts to inf. The scale is what separates the two shapes -- 16383 against 1023.
		{
			const xtndd mx(SpecificValue::maxpos);
			if (scale(mx) != 16383) {
				++nrFailed;
				if (reportTestCases) std::cerr << "xtndd maxpos scale " << scale(mx) << " != 16383 (x87's top binade)\n";
			}
			// and it must be past what a double can hold, which follows from the scale
			if (!std::isinf(double(L::max()))) {
				++nrFailed;
				if (reportTestCases) std::cerr << "xtndd maxpos is inside double's range\n";
			}
		}
		// on a host where long double is x87, the range must agree exactly
		if (std::numeric_limits<long double>::digits == 64
		 && std::numeric_limits<long double>::max_exponent == 16384) {
			if (L::min_exponent != std::numeric_limits<long double>::min_exponent
			 || L::max_exponent != std::numeric_limits<long double>::max_exponent
			 || L::min_exponent10 != std::numeric_limits<long double>::min_exponent10
			 || L::max_exponent10 != std::numeric_limits<long double>::max_exponent10) {
				++nrFailed;
				if (reportTestCases) std::cerr << "xtndd range disagrees with this host's x87 long double\n";
			}
			// and it carries exactly one bit MORE, because cfloat's leading bit is implicit
			// where x87's is explicit
			if (L::digits != std::numeric_limits<long double>::digits + 1) {
				++nrFailed;
				if (reportTestCases) std::cerr << "xtndd digits " << L::digits << " != x87 digits + 1\n";
			}
		}
		return nrFailed;
	}
#endif

} // anonymous namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat numeric_limits decimal digits (#1597)";
	std::string test_tag    = "digits10";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyDecimalDigits<single>("single", 24, 6, 9, reportTestCases);
	nrOfFailedTestCases += VerifyAgreementWithNative(reportTestCases);
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += VerifyHelperBoundaries(reportTestCases);
#endif
	std::cout << "single  digits10=" << std::numeric_limits<single>::digits10
	          << " max_digits10=" << std::numeric_limits<single>::max_digits10 << '\n';
	std::cout << "float   digits10=" << std::numeric_limits<float>::digits10
	          << " max_digits10=" << std::numeric_limits<float>::max_digits10 << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#else

#if REGRESSION_LEVEL_1
	// the IEEE-754 interchange formats, against hand-computed standard values
	nrOfFailedTestCases += ReportTestResult(
		VerifyDecimalDigits<half>("half", 11, 3, 5, reportTestCases),
		test_tag, "half (binary16)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyDecimalDigits<single>("single", 24, 6, 9, reportTestCases),
		test_tag, "single (binary32)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyDecimalDigits<duble>("duble", 53, 15, 17, reportTestCases),
		test_tag, "duble (binary64)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyDecimalDigits<quad>("quad", 113, 33, 36, reportTestCases),
		test_tag, "quad (binary128)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyDecimalDigits<octo>("octo", 237, 71, 73, reportTestCases),
		test_tag, "octo (binary256)");

	// the drop-in property: same format, same answer as the native type
	nrOfFailedTestCases += ReportTestResult(
		VerifyAgreementWithNative(reportTestCases),
		test_tag, "agreement with native float and double");
#endif

#if REGRESSION_LEVEL_2
	// the helper across the widths this library can actually reach
	nrOfFailedTestCases += ReportTestResult(
		VerifyHelperBoundaries(reportTestCases),
		test_tag, "decimal_digits10 / decimal_max_digits10 boundaries");

	// a non-standard width, to confirm the formula is not a lookup table of IEEE cases
	nrOfFailedTestCases += ReportTestResult(
		VerifyDecimalDigits<cfloat<40, 9, std::uint32_t, true, false, false>>("cfloat<40,9>", 31, 9, 11, reportTestCases),
		test_tag, "cfloat<40,9> (non-standard width)");

	// xtndd is the x87 shape, not an 80-bit container holding double's range (#1599)
	nrOfFailedTestCases += ReportTestResult(
		VerifyXtnddIsX87(reportTestCases),
		test_tag, "xtndd is the x87 80-bit format");
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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
