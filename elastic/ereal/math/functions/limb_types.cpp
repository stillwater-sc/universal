// limb_types.cpp: ereal's constants and math library on every limb type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Part 5 of #1355 (#1567). Two ceilings held every transcendental at ~313 digits, whatever
// the limb type and however many limbs it was given:
//
//   - the stored constants were double-component expansions. A double component cannot go
//     below 2^-1074, so the table ended after 19 entries. They carry an explicit exponent
//     per component now, so one table serves every limb type and runs to 2^-4298.
//   - every series stopped by converting its term to a double and comparing against a
//     decimal threshold. Any term below ~1e-308 reads as exactly zero there, so the series
//     stopped at ~308 digits on a limb type whose range goes further. The test is on
//     exponents now (series_converged).
//
// Everything here is measured against the exact dyadic oracle (#1566), never through a
// double, and against the 5000-digit references where a value is a known constant.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/ereal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>
#include <math/constants/long_reference_constants.hpp>

namespace {

	using namespace sw::universal;

	int expect(bool ok, const std::string& what, bool reportTestCases) {
		if (ok) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}
	int expect_digits(int got, int wanted, const std::string& what, bool reportTestCases) {
		if (got >= wanted) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << got << " digits, wanted at least " << wanted << '\n';
		return 1;
	}

	// digits a configuration carries: maxlimbs * the limb type's bits, in decimal
	template<unsigned N, typename F>
	constexpr int configuration_digits() {
		return static_cast<int>(static_cast<double>(N) * std::numeric_limits<F>::digits * 0.30103);
	}

	// ---- the stored constants --------------------------------------------------------

	template<unsigned N, typename F>
	int VerifyConstants(const std::string& name, bool reportTestCases) {
		int fails = 0;
		// a constant is worth its configuration, less a limb of slack for the reconstruction
		const int wanted = configuration_digits<N, F>() - static_cast<int>(std::numeric_limits<F>::digits * 0.30103) - 2;
		fails += expect_digits(agreed_decimal_digits(ereal_pi<N, F>(), s_long_pi, kLongReferenceCap), wanted, name + ": pi", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(ereal_ln2<N, F>(), s_long_ln2, kLongReferenceCap), wanted, name + ": ln2", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(ereal_ln10<N, F>(), s_long_ln10, kLongReferenceCap), wanted, name + ": ln10", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(ereal_e<N, F>(), s_long_e, kLongReferenceCap), wanted, name + ": e", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(ereal_sqrt2<N, F>(), s_long_sqrt2, kLongReferenceCap), wanted, name + ": sqrt2", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(ereal_phi<N, F>(), s_long_phi, kLongReferenceCap), wanted, name + ": phi", reportTestCases);
		// the derived ones are exact scalings of the stored ones
		using R = ereal<N, F>;
		fails += expect_digits(agreed_decimal_digits(ereal_pi_2<N, F>() * R(2.0), ereal_pi<N, F>(), 4 * wanted), 4 * wanted, name + ": pi/2 * 2 == pi exactly", reportTestCases);
		return fails;
	}

	// ---- the math library --------------------------------------------------------------

	template<unsigned N, typename F>
	int VerifyMathlib(const std::string& name, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		// a function is worth its configuration, less two limbs for the algorithm
		const int wanted = configuration_digits<N, F>() - 2 * static_cast<int>(std::numeric_limits<F>::digits * 0.30103) - 2;
		const R one(1.0), two(2.0), third = R(1.0) / R(3.0);

		// against the references
		fails += expect_digits(agreed_decimal_digits(atan(one) * R(4.0), s_long_pi, kLongReferenceCap), wanted, name + ": 4*atan(1) is pi", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(log(two), s_long_ln2, kLongReferenceCap), wanted, name + ": log(2)", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(exp(one), s_long_e, kLongReferenceCap), wanted, name + ": exp(1) is e", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(sqrt(two), s_long_sqrt2, kLongReferenceCap), wanted, name + ": sqrt(2)", reportTestCases);

		// and the identities, which need no reference at all
		fails += expect_digits(agreed_decimal_digits(exp(log(two)), two, 4 * wanted), wanted, name + ": exp(log(2)) is 2", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(sin(third) * sin(third) + cos(third) * cos(third), one, 4 * wanted), wanted, name + ": sin^2 + cos^2 is 1", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(cosh(third) * cosh(third) - sinh(third) * sinh(third), one, 4 * wanted), wanted, name + ": cosh^2 - sinh^2 is 1", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(sqrt(third) * sqrt(third), third, 4 * wanted), wanted, name + ": sqrt(x)^2 is x", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(cbrt(two) * cbrt(two) * cbrt(two), two, 4 * wanted), wanted, name + ": cbrt(2)^3 is 2", reportTestCases);
		return fails;
	}

	// ---- the paths a fixed-precision literal or a double buffer used to cap --------------
	//
	// Each of these was capped by something that did not scale with the configuration, and
	// each is checked here against the value the type should carry (#1576).

	template<unsigned N, typename F>
	int VerifyNoFixedPrecisionPaths(const std::string& name, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		const int wanted = configuration_digits<N, F>() - 2 * static_cast<int>(std::numeric_limits<F>::digits * 0.30103) - 2;
		const R one(1.0);

		// atan for |x| > 0.5 went through atan(1/2), which was a 64-digit literal
		for (double v : { 0.75, 0.9 }) {
			const R x(v);
			fails += expect_digits(agreed_decimal_digits(tan(atan(x)), x, 4 * wanted), wanted,
			                       name + ": tan(atan(" + std::to_string(v) + ")) is x, past 64 digits", reportTestCases);
		}
		// atan2's quadrant adjustment used double literals for pi and pi/2
		fails += expect_digits(agreed_decimal_digits(atan2(one, -one), ereal_3pi_4<N, F>(), 4 * wanted), wanted,
		                       name + ": atan2(1, -1) is 3pi/4", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(atan2(one, R(0.0)), ereal_pi_2<N, F>(), 4 * wanted), wanted,
		                       name + ": atan2(1, 0) is pi/2", reportTestCases);
		// log1p's series ran a fixed 100 terms, short of what a wide configuration holds
		const R small(0.09);
		fails += expect_digits(agreed_decimal_digits(log1p(small), log(one + small), 4 * wanted), wanted,
		                       name + ": log1p(0.09) agrees with log(1.09)", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(expm1(small), exp(small) - one, 4 * wanted), wanted,
		                       name + ": expm1(0.09) agrees with exp(0.09) - 1", reportTestCases);
		// floor/ceil kept their scratch limbs in a double, which rounds a wider limb
		const R big = R(std::ldexp(1.0, std::numeric_limits<F>::digits - 1)) + one + R(0.5);   // 2^(p-1) + 1.5
		const R expected = R(std::ldexp(1.0, std::numeric_limits<F>::digits - 1)) + one;
		fails += expect(floor(big) == expected, name + ": floor keeps a limb the limb type can hold", reportTestCases);
		fails += expect(ceil(big) == expected + one, name + ": ceil keeps a limb the limb type can hold", reportTestCases);
		return fails;
	}

}  // anonymous namespace

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

// The long double cases are slow -- seconds per transcendental -- because ereal does not
// cap an arithmetic result at maxlimbs and a wide-exponent limb lets every quotient grow
// to ~250 limbs (#1572). They run at levels 2 and 3 until that is fixed. The constants
// are cheap at any width: they are a table sum.
template<typename LD>
int VerifyLongDoubleMath(bool reportTestCases, const std::string& test_tag, bool deep) {
	using namespace sw::universal;
	int n = 0;
	if (!deep) {
		n += ReportTestResult(VerifyConstants<8, LD>("ereal<8, long double>", reportTestCases), test_tag, "long double constants");
		n += ReportTestResult(VerifyMathlib<8, LD>("ereal<8, long double>", reportTestCases), test_tag, "long double mathlib");
		n += ReportTestResult(VerifyNoFixedPrecisionPaths<8, LD>("ereal<8, long double>", reportTestCases), test_tag, "long double fixed-precision paths");
	}
	else if constexpr (ereal<8, LD>::max_safe_limbs >= 24) {
		// 24 limbs of x87 carry ~460 digits: past the ~313 the stored constants and the
		// series thresholds used to cap every transcendental at, whatever the limb type
		n += ReportTestResult(VerifyConstants<24, LD>("ereal<24, long double>", reportTestCases), test_tag, "long double constants, 24 limbs");
		n += ReportTestResult(VerifyMathlib<24, LD>("ereal<24, long double>", reportTestCases), test_tag, "long double mathlib, 24 limbs");
	}
	return n;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal constants and math library on every limb type";
	std::string test_tag    = "ereal mathlib limbs";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyConstants<19, double>("ereal<19>", true), test_tag, "double constants");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<5, float>("ereal<5, float>", reportTestCases), test_tag, "float constants");
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<8, double>("ereal<8>", reportTestCases), test_tag, "double constants");
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<19, double>("ereal<19>", reportTestCases), test_tag, "double constants, 19 limbs");
	nrOfFailedTestCases += ReportTestResult(VerifyMathlib<5, float>("ereal<5, float>", reportTestCases), test_tag, "float mathlib");
	nrOfFailedTestCases += ReportTestResult(VerifyMathlib<8, double>("ereal<8>", reportTestCases), test_tag, "double mathlib");
	nrOfFailedTestCases += ReportTestResult(VerifyMathlib<19, double>("ereal<19>", reportTestCases), test_tag, "double mathlib, 19 limbs");
	nrOfFailedTestCases += ReportTestResult(VerifyNoFixedPrecisionPaths<5, float>("ereal<5, float>", reportTestCases), test_tag, "float fixed-precision paths");
	nrOfFailedTestCases += ReportTestResult(VerifyNoFixedPrecisionPaths<8, double>("ereal<8>", reportTestCases), test_tag, "double fixed-precision paths");
	nrOfFailedTestCases += ReportTestResult(VerifyNoFixedPrecisionPaths<19, double>("ereal<19>", reportTestCases), test_tag, "double fixed-precision paths, 19 limbs");
#endif

#if REGRESSION_LEVEL_2
	if constexpr (is_expansion_limb_v<long double>) nrOfFailedTestCases += VerifyLongDoubleMath<long double>(reportTestCases, test_tag, false);
#endif

#if REGRESSION_LEVEL_3
	if constexpr (is_expansion_limb_v<long double>) nrOfFailedTestCases += VerifyLongDoubleMath<long double>(reportTestCases, test_tag, true);
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
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
