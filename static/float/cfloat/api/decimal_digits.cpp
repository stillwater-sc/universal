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
	std::cout << "single  digits10=" << std::numeric_limits<single>::digits10
	          << " max_digits10=" << std::numeric_limits<single>::max_digits10 << '\n';
	std::cout << "float   digits10=" << std::numeric_limits<float>::digits10
	          << " max_digits10=" << std::numeric_limits<float>::max_digits10 << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

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
