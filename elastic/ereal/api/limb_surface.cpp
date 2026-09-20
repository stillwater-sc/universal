// limb_surface.cpp: the library surface of ereal<maxlimbs, FpType>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Part 3 of #1355 (#1565): numeric_limits, traits, type_tag, stream insertion and
// extraction, the power-of-two helpers (ldexp, frexp, ilogb, copysign, scalbn), and the
// test-support checks all follow ereal's limb type. Checked for float, double, and long
// double wherever it is a valid limb -- exactly, against the dyadic oracle, where a value
// is involved.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/ereal_test_support.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	template<unsigned N, typename F>
	dyadic value_of(const ereal<N, F>& x) {
		dyadic acc;
		for (F c : x.limbs()) acc = acc + dyadic::from_fp(c);
		return acc;
	}

	int expect(bool ok, const std::string& what, bool reportTestCases) {
		if (ok) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// ---- numeric_limits ------------------------------------------------------------------

	template<unsigned N, typename F>
	int VerifyNumericLimits(const std::string& name, bool reportTestCases) {
		using R = ereal<N, F>;
		using L = std::numeric_limits<R>;
		using LF = std::numeric_limits<F>;
		int fails = 0;
		fails += expect(L::is_specialized, name + ": numeric_limits is specialized", reportTestCases);
		fails += expect(L::digits == static_cast<int>(N) * LF::digits, name + ": digits = maxlimbs * digits of the limb", reportTestCases);
		fails += expect(L::max_exponent == LF::max_exponent && L::min_exponent == LF::min_exponent, name + ": exponent range is the limb type's", reportTestCases);
		fails += expect(L::max() == R(SpecificValue::maxpos) && L::max()[0] == LF::max(), name + ": max() is maxpos, led by the limb type's max", reportTestCases);
		fails += expect(L::lowest() == -L::max(), name + ": lowest() is -max()", reportTestCases);
		// epsilon is 2^(1 - digits) of the whole expansion, so it agrees with digits
		fails += expect(value_of(L::epsilon()) == dyadic::from_fp(std::ldexp(F(1), 1 - L::digits)),
		                name + ": epsilon() is 2^(1 - digits)", reportTestCases);
		// has_denorm is denorm_absent, so C++20 requires denorm_min() == min()
		fails += expect(L::has_denorm == std::denorm_absent && value_of(L::denorm_min()) == value_of((L::min)()),
		                name + ": denorm_min() is min() when subnormals are absent", reportTestCases);
		fails += expect(L::infinity().isinf() && L::quiet_NaN().isnan(), name + ": infinity() and quiet_NaN()", reportTestCases);
		return fails;
	}

	// ---- traits and type_tag --------------------------------------------------------------

	int VerifyTraitsAndTags(bool reportTestCases) {
		int fails = 0;
		static_assert(is_ereal<ereal<8>> && is_ereal<ereal<5, float>>, "is_ereal for every limb type");
		static_assert(!is_ereal<double>);
		fails += expect(type_tag(ereal<8>()) == "ereal<8>", "type_tag of the default is unchanged", reportTestCases);
		fails += expect(type_tag(ereal<5, float>()) == "ereal<5, float>", "type_tag names a float limb", reportTestCases);
		return fails;
	}
	// 8 limbs, not 24: long double is a valid limb on MSVC and Apple ARM64 too, where it
	// IS double, and there max_safe_limbs is 19 -- ereal<24, long double> would fail the
	// class's static_assert at compile time on those platforms (#1574).
	template<typename LD>
	int VerifyLongDoubleTag(bool reportTestCases) {
		static_assert(is_ereal<ereal<8, LD>>);
		return expect(type_tag(ereal<8, LD>()) == "ereal<8, long double>", "type_tag names a long double limb", reportTestCases);
	}

	// ---- streams ---------------------------------------------------------------------------

	template<unsigned N, typename F>
	int VerifyStreams(const std::string& name, int digits, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		std::stringstream ss;
		ss << std::setprecision(digits) << (R(1.0) / R(3.0));
		const std::string expected = "3." + std::string(static_cast<std::size_t>(digits), '3') + "e-01";
		fails += expect(ss.str() == expected, name + ": operator<< prints 1/3 to " + std::to_string(digits) + " digits", reportTestCases);
		// extraction: "0.1" must agree with 1/10 to about the precision of the type
		std::stringstream in("0.1");
		R x;
		in >> x;
		const R tenth = R(1.0) / R(10.0);
		const dyadic diff = value_of(x) - value_of(tenth);
		const int p = std::numeric_limits<F>::digits;
		bool close = diff.iszero();
		if (!close) {
			auto n = diff.numerator; if (n.sign()) n.setsign(false);
			int bits = 0; while (!n.iszero()) { n >>= 1; ++bits; }
			const int log2diff = bits - 1 + diff.scale;       // |x - 1/10| ~ 2^log2diff
			close = log2diff <= -4 - (static_cast<int>(N) - 2) * p;
		}
		fails += expect(!in.fail() && close, name + ": operator>> reads 0.1 to the type's precision", reportTestCases);
		return fails;
	}

	// ---- power-of-two helpers ---------------------------------------------------------------

	template<unsigned N, typename F>
	int VerifyPowerOfTwo(const std::string& name, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		// an exact three-limb value: 5 + 2^-30 + 2^-60. (A quotient would not do for float
		// limbs: ereal's arithmetic does not cap a result at maxlimbs, and 1/3 in
		// ereal<4, float> already carries limbs in float's subnormal range, where no scaling
		// is exact -- see #1568.)
		const R x = R(5.0) + R(std::ldexp(1.0, -30)) + R(std::ldexp(1.0, -60));
		const dyadic X = value_of(x);
		fails += expect(value_of(ldexp(x, 7)) == X * dyadic::from_fp(F(128)), name + ": ldexp(x, 7) is exact", reportTestCases);
		fails += expect(value_of(scalbn(x, -3)) == X * dyadic::from_fp(F(0.125)), name + ": scalbn(x, -3) is exact", reportTestCases);
		int e = 0;
		const R m = frexp(x, &e);
		fails += expect(e == 3 && m >= R(0.5) && m < R(1.0) && value_of(ldexp(m, e)) == X, name + ": frexp gives [0.5, 1) and 2^3", reportTestCases);
		fails += expect(ilogb(x) == 2 && ilogb(R(0.75)) == -1, name + ": ilogb", reportTestCases);
		fails += expect(copysign(x, R(-1.0)) == -x && copysign(-x, R(2.0)) == x, name + ": copysign", reportTestCases);
		return fails;
	}

	// ---- test support at the limb type's precision -----------------------------------------

	template<unsigned N, typename F>
	int VerifyTestSupport(const std::string& name, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		const R q = R(2.0) / R(7.0);
		fails += expect(is_priest_normal(q), name + ": a quotient is in Priest normal form", reportTestCases);
		// two limbs one bit too close: overlapping at the limb type's own precision
		const int p = std::numeric_limits<F>::digits;
		const std::vector<F> overlapping{ F(1), std::ldexp(F(1), -p + 1) };
		fails += expect(check_priest_normal(overlapping).violation == PriestNormalResult::Violation::Overlap,
		                name + ": a component within half an ulp is flagged as overlap", reportTestCases);
		const std::vector<F> adjacent{ F(1), std::ldexp(F(1), -p) };
		fails += expect(check_priest_normal(adjacent).ok(), name + ": a component at exactly half an ulp is not", reportTestCases);
		fails += expect(value_of(widen<N + 1>(q)) == value_of(q), name + ": widen keeps the value", reportTestCases);
		// the limb type itself converts in and out exactly
		const F v = F(1) / F(3);
		fails += expect(static_cast<F>(R(v)) == v, name + ": the limb type converts in and back out exactly", reportTestCases);
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

template<typename LD>
int VerifyLongDouble(bool reportTestCases, const std::string& test_tag) {
	using namespace sw::universal;
	int n = 0;
	n += ReportTestResult(VerifyLongDoubleTag<LD>(reportTestCases), test_tag, "long double type_tag");
	n += ReportTestResult(VerifyNumericLimits<8, LD>("ereal<8, long double>", reportTestCases), test_tag, "long double numeric_limits");
	n += ReportTestResult(VerifyStreams<8, LD>("ereal<8, long double>", 100, reportTestCases), test_tag, "long double streams");
	n += ReportTestResult(VerifyPowerOfTwo<8, LD>("ereal<8, long double>", reportTestCases), test_tag, "long double ldexp/frexp/ilogb");
	n += ReportTestResult(VerifyTestSupport<8, LD>("ereal<8, long double>", reportTestCases), test_tag, "long double test support");
	return n;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal<maxlimbs, FpType>: numeric_limits, traits, streams and helpers";
	std::string test_tag    = "ereal limb surface";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyStreams<5, float>("ereal<5, float>", 25, true), test_tag, "float streams");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTraitsAndTags(reportTestCases), test_tag, "traits and type_tag");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<5, float>("ereal<5, float>", reportTestCases), test_tag, "float numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<8, double>("ereal<8>", reportTestCases), test_tag, "double numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyStreams<5, float>("ereal<5, float>", 25, reportTestCases), test_tag, "float streams");
	nrOfFailedTestCases += ReportTestResult(VerifyStreams<8, double>("ereal<8>", 100, reportTestCases), test_tag, "double streams");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerOfTwo<5, float>("ereal<5, float>", reportTestCases), test_tag, "float ldexp/frexp/ilogb");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerOfTwo<8, double>("ereal<8>", reportTestCases), test_tag, "double ldexp/frexp/ilogb");
	nrOfFailedTestCases += ReportTestResult(VerifyTestSupport<4, float>("ereal<4, float>", reportTestCases), test_tag, "float test support");
	nrOfFailedTestCases += ReportTestResult(VerifyTestSupport<8, double>("ereal<8>", reportTestCases), test_tag, "double test support");
	if constexpr (is_expansion_limb_v<long double>) nrOfFailedTestCases += VerifyLongDouble<long double>(reportTestCases, test_tag);
#endif

#if REGRESSION_LEVEL_2
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
