// limits_abs_fma.cpp: numeric_limits, abs and fma for areal, and what the ubit promises
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep in #1962 found (#1970):
//   - numeric_limits<areal> disagreed with the encoding: min() was minpos, denorm_min() was 1,
//     epsilon() was 0 with the ubit set, digits counted the ubit, the exponents were off, and
//     has_denorm and is_bounded denied the subnormals and the finite range;
//   - abs(areal) could not be called: its definition returned a different type than its
//     declaration, through a constructor areal does not have;
//   - fma ignored the inputs' ubits.
// It also asked what a single areal's ubit promises. The answer, documented in areal_impl.hpp and
// docs/number-systems/areal.md, is a flag: an inexact operand makes the result inexact, but the
// result's interval need not enclose the exact interval result. VerifyFlagSemantics pins that.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::areal;

// the limits against the encoding
template<typename A>
int VerifyLimits(bool reportTestCases) {
	using L = std::numeric_limits<A>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << sw::universal::type_tag(A()) << ' ' << what << '\n'; } };
	constexpr int fbits = static_cast<int>(A::fbits);
	// they are constant expressions, and exact values
	constexpr A eps = L::epsilon(), mn = L::min(), dmin = L::denorm_min();
	static_assert(!eps.at(0) && !mn.at(0) && !dmin.at(0), "the limits are exact values");
	check(double(eps) == std::ldexp(1.0, -fbits), "epsilon is 2^-fbits");
	{
		A next(1.0f);
		next.set(1);   // the fraction's least significant bit: 1 + 2^-fbits
		check(double(next) - 1.0 == double(eps), "epsilon is the distance from 1 to the next value");
	}
	check(double(mn) == std::ldexp(1.0, A::MIN_EXP_NORMAL), "min is the smallest normal value");
	check(double(dmin) == std::ldexp(1.0, A::MIN_EXP_SUBNORMAL), "denorm_min is the smallest subnormal value");
	check(dmin == A(sw::universal::SpecificValue::minpos), "denorm_min is minpos");
	{
		// the encoding just below min: exponent field 0, every fraction bit set, the largest subnormal
		A below;
		for (unsigned i = 1; i <= A::fbits; ++i) below.set(i);
		check(double(below) < double(mn) && double(below) > 0.0 && double(mn) - double(below) == double(dmin), "the value just below min is the largest subnormal");
	}
	check(L::digits == fbits + 1, "digits is the significand of an exact value, without the ubit");
	check(std::ldexp(1.0, L::min_exponent - 1) == double(mn), "2^(min_exponent - 1) is min()");
	// areal has one binade more than IEEE, so areal<64,11>'s max() is past double's range
	if constexpr (L::max_exponent < std::numeric_limits<double>::max_exponent) {
		check(std::ldexp(1.0, L::max_exponent - 1) <= double(L::max()) && double(L::max()) < std::ldexp(1.0, L::max_exponent), "max lies in [2^(max_exponent - 1), 2^max_exponent)");
	}
	check(L::has_denorm == std::denorm_present && L::is_bounded, "has_denorm and is_bounded");
	check(L::round_style == std::round_toward_zero && double(L::round_error()) == 1.0, "truncation: round_toward_zero, an error of up to one ulp");
	return fails;
}

// abs over every encoding: the sign bit off, the value bits and the ubit kept, NaN as is
template<typename A>
int VerifyAbs(bool reportTestCases) {
	int fails = 0, reported = 0;
	A x;
	for (std::uint64_t raw = 0; raw < (std::uint64_t(1) << A::nbits); ++raw) {
		x.setbits(raw);
		const A a = abs(x);
		bool ok;
		if (x.isnan()) {
			ok = a.isnan();
		}
		else {
			ok = !a.at(A::nbits - 1) && !std::signbit(double(a));   // the sign bit off
			for (unsigned i = 0; i + 1 < A::nbits; ++i) ok = ok && (a.at(i) == x.at(i));   // the rest, ubit included, kept
			ok = ok && (x.isinf() ? a.isinf() : double(a) == std::fabs(double(x)));
		}
		if (!ok) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL abs(" << sw::universal::to_binary(x) << ") = " << sw::universal::to_binary(a) << '\n';
		}
	}
	return fails;
}

// fma over every triple of finite areal<8,2> values: exact inputs give a ubit set exactly when the
// fused result is not representable, and an inexact input makes the result inexact
int VerifyFma(bool reportTestCases) {
	using A = areal<8, 2>;
	int fails = 0, reported = 0;
	std::vector<A> values;
	A v;
	for (unsigned raw = 0; raw < 256u; ++raw) {
		v.setbits(raw);
		if (!v.isnan() && !v.isinf()) values.push_back(v);
	}
	for (const A& a : values) for (const A& b : values) for (const A& c : values) {
		const A r = fma(a, b, c);
		if (r.isnan() || r.isinf()) continue;
		const bool inexactInput = a.at(0) || b.at(0) || c.at(0);
		bool ok;
		if (inexactInput) {
			ok = r.at(0);   // the flag carries over
		}
		else {
			// exact inputs: the fused value is exact in double (4-bit significands), so the ubit
			// is set exactly when the areal truncated it
			const double exact = std::fma(double(a), double(b), double(c));
			A t(r);
			t.reset(0);
			ok = (r.at(0) == (double(t) != exact));
		}
		if (!ok) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL fma(" << double(a) << (a.at(0) ? "~" : "") << ", " << double(b) << (b.at(0) ? "~" : "") << ", " << double(c) << (c.at(0) ? "~" : "") << ") ubit " << r.at(0) << '\n';
		}
	}
	// the case the sweep reported: fma(0.1, 1, 0) is as inexact as 0.1 * 1 + 0
	const areal<16, 5> tenth(0.1f), one(1.0f), zero(0.0f);
	if (!fma(tenth, one, zero).at(0) || !(tenth * one + zero).at(0)) ++fails;
	return fails;
}

// A single areal's ubit is a flag (#1970): an inexact operand makes the result inexact, and a
// result from exact operands names the interval holding the exact value. It does not promise that
// the result's interval encloses the exact interval result: (0, minpos) - minpos is flagged, but
// the true range (-minpos, 0) is not the interval it names. tile_interval is the enclosure.
int VerifyFlagSemantics(bool reportTestCases) {
	using A = areal<16, 5>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	A zeroToMinpos;
	zeroToMinpos.setbits(0x0001u);   // the ubit alone: (0, minpos)
	const A minpos(sw::universal::SpecificValue::minpos);
	const A r = zeroToMinpos - minpos;
	check(r.at(0), "an inexact operand flags the result inexact");
	check(!((A(1.0f) - A(0.5f)).at(0)), "exact operands with a representable result stay exact");
	check((A(1.0f) / A(3.0f)).at(0), "exact operands with an unrepresentable result are flagged");
	return fails;
}

}  // namespace

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

	std::string test_suite  = "areal numeric_limits, abs, fma and ubit semantics";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << double(std::numeric_limits<areal<16, 5>>::epsilon()) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<8, 2>>(reportTestCases), "areal<8,2>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<16, 5>>(reportTestCases), "areal<16,5>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<32, 8>>(reportTestCases), "areal<32,8>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<64, 11>>(reportTestCases), "areal<64,11>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<areal<16, 5, std::uint8_t>>(reportTestCases), "areal<16,5,uint8>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyAbs<areal<8, 2>>(reportTestCases), "areal<8,2>", "abs");
	nrOfFailedTestCases += ReportTestResult(VerifyAbs<areal<12, 4, std::uint8_t>>(reportTestCases), "areal<12,4,uint8>", "abs");
	nrOfFailedTestCases += ReportTestResult(VerifyFma(reportTestCases), "areal<8,2>", "fma ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyFlagSemantics(reportTestCases), "areal<16,5>", "ubit is a flag");
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
	std::cerr << msg << std::endl;
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
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
