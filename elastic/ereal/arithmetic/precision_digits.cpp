// precision_digits.cpp: how many digits an ereal actually carries, measured exactly
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Part 4 of #1355 (#1566). A claim about precision can only be checked by an oracle that
// carries more digits than the claim: with long double limbs ereal reaches thousands
// (#1564), and nothing that passes through a double -- or through to_string, which rounds
// -- can see them.
//
// The oracle here is exact: ereal_to_dyadic reads the limbs into an einteger-backed
// dyadic rational, and agreed_decimal_digits cross-multiplies against the decimal
// reference. No rounding, and no code path shared with ereal's arithmetic.
//
// What is checked:
//   - the oracle itself: an exact value scores the cap, a wrong digit is found where it is
//   - identities per limb type: (a/b)*b agrees with a to about the precision of the type
//   - sqrt(2) by Newton iteration -- arithmetic only, no math library -- against the
//     5000-digit reference, which is the point of the wider limbs
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/ereal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>
#include <math/constants/reference_constants.hpp>
#include <math/constants/long_reference_constants.hpp>

namespace {

	using namespace sw::universal;

	int expect_digits(int got, int wanted, const std::string& what, bool reportTestCases) {
		if (got >= wanted) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << got << " digits, wanted at least " << wanted << '\n';
		return 1;
	}
	int expect(bool ok, const std::string& what, bool reportTestCases) {
		if (ok) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// ---- the oracle itself -------------------------------------------------------------

	int VerifyTheOracle(bool reportTestCases) {
		int fails = 0;
		using R = ereal<8>;
		// an exactly representable value scores the cap against its own decimal expansion
		fails += expect(agreed_decimal_digits(R(0.375), "0.375", 320) == 320, "an exact value scores the cap", reportTestCases);
		// a reference that goes wrong at digit 20 is found there, not later
		const std::string tenth_wrong = "0.10000000000000000009";   // 1/10 with digit 20 wrong
		const R tenth = R(1.0) / R(10.0);
		const int d = agreed_decimal_digits(tenth, tenth_wrong, 60);
		fails += expect(d >= 18 && d <= 20, "a wrong digit is found where it is (got " + std::to_string(d) + ")", reportTestCases);
		// a double's worth of pi scores ~16 digits against the 340-digit reference
		const int dpi = agreed_decimal_digits(R(3.14159265358979323846), s_pi, 320);
		fails += expect(dpi >= 15 && dpi <= 17, "a double pi scores ~16 digits (got " + std::to_string(dpi) + ")", reportTestCases);
		// the long references carry what they claim
		fails += expect(s_long_pi.size() > 5000 && s_long_sqrt2.size() > 5000, "the long references carry 5000 digits", reportTestCases);
		fails += expect(agreed_decimal_digits(R(3.14159265358979323846), s_long_pi, kLongReferenceCap) == dpi,
		                "the long and short pi references agree about a double", reportTestCases);
		return fails;
	}

	// ---- identities carry the precision of the limbs -------------------------------------

	template<unsigned N, typename F>
	int VerifyIdentities(const std::string& name, bool reportTestCases) {
		using R = ereal<N, F>;
		constexpr int p = std::numeric_limits<F>::digits;
		// what N limbs can hold, in decimal digits, less two limbs of slack for the
		// quotient's Newton iteration
		const int wanted = static_cast<int>((static_cast<double>(N - 2) * p) * 0.30103);
		int fails = 0;
		for (int b = 3; b <= 11; b += 2) {
			const R x = R(1.0) / R(static_cast<double>(b));
			fails += expect_digits(agreed_decimal_digits(x * R(static_cast<double>(b)), R(1.0), 4 * wanted),
			                       wanted, name + ": (1/" + std::to_string(b) + ")*" + std::to_string(b) + " is 1", reportTestCases);
		}
		// a difference of two nearly equal values keeps its digits
		const R a = R(1.0) / R(3.0), c = a + R(std::ldexp(1.0, -40)) * a;
		fails += expect_digits(agreed_decimal_digits((c - a) * R(std::ldexp(1.0, 40)), a, 4 * wanted),
		                       wanted, name + ": a cancellation keeps its digits", reportTestCases);
		return fails;
	}

	// ---- sqrt(2) by Newton, arithmetic only ----------------------------------------------
	//
	// x_{k+1} = (x_k + 2/x_k) / 2 doubles its correct digits every step, so ~log2(digits)
	// steps reach the precision of the type. Only +, * and / are used: no math library, so
	// this measures the arithmetic (#1567 is the math library's own turn).

	template<unsigned N, typename F>
	int VerifySqrt2(const std::string& name, int wanted, int cap, bool reportTestCases) {
		using R = ereal<N, F>;
		R x(1.5);
		const R two(2.0), half(0.5);
		// Newton doubles its correct digits per step, from the one digit of the 1.5 seed:
		// log2(wanted) steps reach it, plus a few for the seed and rounding
		const int steps = 3 + static_cast<int>(std::log2(static_cast<double>(wanted)));
		for (int k = 0; k < steps; ++k) x = (x + two / x) * half;
		const int digits = agreed_decimal_digits(x, s_long_sqrt2, cap);
		if (reportTestCases) std::cout << "    " << name << ": sqrt(2) to " << digits << " digits (" << steps << " Newton steps)\n";
		return expect_digits(digits, wanted, name + ": sqrt(2) by Newton", reportTestCases);
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

// The long double cases are SLOW, and not because of the oracle: ereal does not cap an
// arithmetic result at maxlimbs. With double limbs a cap emerges by accident, since
// components below 2^-1022 underflow away and a result settles at ~20 limbs. x87 and
// binary128 reach 2^-16382, so nothing prunes them: every quotient grows to ~250 limbs
// whatever maxlimbs says, and a single division costs seconds. So they run at levels 2
// and 3 rather than in CI's level 1. Capping results is #1572.
template<typename LD>
int VerifyLongDoubleDigits(bool reportTestCases, const std::string& test_tag, bool deep) {
	using namespace sw::universal;
	int n = 0;
	if (!deep) {
		// ~37 s: eight Newton steps at 8 limbs, past the 340-digit reference's reach
		n += ReportTestResult(VerifySqrt2<8, LD>("ereal<8, long double>", 300, kLongReferenceCap, reportTestCases), test_tag, "long double sqrt(2)");
	}
	else {
		n += ReportTestResult(VerifyIdentities<8, LD>("ereal<8, long double>", reportTestCases), test_tag, "long double identities");
		// ~125 s, and 1235 digits: what the wider limb is for. Only where the limb type
		// admits 24 of them: long double is a valid limb on MSVC and Apple ARM64 too,
		// where it IS double and max_safe_limbs is 19, and ereal<24, long double> would
		// fail the class's static_assert at compile time there (#1574).
		if constexpr (ereal<8, LD>::max_safe_limbs >= 24) {
			n += ReportTestResult(VerifySqrt2<24, LD>("ereal<24, long double>", 1100, kLongReferenceCap, reportTestCases), test_tag, "long double sqrt(2), 24 limbs");
		}
	}
	return n;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal precision, measured against exact references";
	std::string test_tag    = "ereal digits";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyTheOracle(true), test_tag, "the oracle");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTheOracle(reportTestCases), test_tag, "the oracle");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentities<5, float>("ereal<5, float>", reportTestCases), test_tag, "float identities");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentities<8, double>("ereal<8>", reportTestCases), test_tag, "double identities");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentities<19, double>("ereal<19>", reportTestCases), test_tag, "double identities, 19 limbs");
	// double limbs stop at 19 limbs, about 303 digits
	nrOfFailedTestCases += ReportTestResult(VerifySqrt2<19, double>("ereal<19>", 280, 320, reportTestCases), test_tag, "double sqrt(2)");
#endif

#if REGRESSION_LEVEL_2
	if constexpr (is_expansion_limb_v<long double>) nrOfFailedTestCases += VerifyLongDoubleDigits<long double>(reportTestCases, test_tag, false);
#endif

#if REGRESSION_LEVEL_3
	if constexpr (is_expansion_limb_v<long double>) nrOfFailedTestCases += VerifyLongDoubleDigits<long double>(reportTestCases, test_tag, true);
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
