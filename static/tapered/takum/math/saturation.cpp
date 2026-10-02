// saturation.cpp: math functions saturate where their double evaluation leaves double's range
//
// The takum and takum_log functions that evaluate in a double -- exp, exp2, exp10,
// expm1, pow, sinh, cosh, erfc -- used to read the double's range limits literally:
// an overflow to +/-inf became NaR and an underflow to 0 became zero, though the
// true result is finite and nonzero.  Takum saturates instead, to maxpos/maxneg and
// minpos/minneg (#1624, following #1620 for conversion and arithmetic).
//
// Verified here:
//   - the overflow and underflow cases, for both variants at several widths
//   - pow keeps the sign of an odd power when it saturates
//   - genuine zeros and poles are NOT saturated: pow(0, -1) is NaR, pow(0, 2),
//     sinh(0) and expm1(0) are zero
//   - wide regime fields, whose range exceeds double's, are NOT saturated, since a
//     double overflow there can be an ordinary representable takum
//   - nothing inside double's range changes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

#include <universal/number/takum/takum.hpp>
#include <universal/number/takum/mathlib.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<typename T>
struct Expect {
	int& fails;
	bool report;
	std::string tag;
	void operator()(const T& got, const T& want, const char* what) const {
		if (got.raw_bits() == want.raw_bits()) return;
		++fails;
		if (report) std::cout << "FAIL " << tag << ' ' << what << ": got " << got << " (bits 0x" << std::hex
		                      << got.raw_bits() << std::dec << ")\n";
	}
};

// The saturation cases.  Works for takum<> and takum_log<> alike.
template<typename T>
int VerifySaturation(const std::string& tag, bool reportTestCases) {
	using sw::universal::SpecificValue;
	int fails = 0;
	Expect<T> expect{ fails, reportTestCases, tag };
	const T maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	const T minpos(SpecificValue::minpos), minneg(SpecificValue::minneg);

	// overflow of the double: the true result is finite and above maxpos
	expect(exp(T(800.0)),            maxpos, "exp(800)");
	expect(exp(maxpos),              maxpos, "exp(maxpos)");
	expect(exp2(T(1100.0)),          maxpos, "exp2(1100)");
	expect(exp10(T(400.0)),          maxpos, "exp10(400)");
	expect(expm1(T(800.0)),          maxpos, "expm1(800)");
	expect(pow(T(10.0), T(400.0)),   maxpos, "pow(10, 400)");
	expect(pow(T(10.0), 400.0),      maxpos, "pow(10, 400.0)");
	expect(sinh(T(800.0)),           maxpos, "sinh(800)");
	expect(sinh(T(-800.0)),          maxneg, "sinh(-800)");
	expect(cosh(T(-800.0)),          maxpos, "cosh(-800)");

	// underflow of the double: the true result is nonzero and below minpos
	expect(exp(T(-800.0)),           minpos, "exp(-800)");
	expect(exp(maxneg),              minpos, "exp(maxneg)");
	expect(exp2(T(-1100.0)),         minpos, "exp2(-1100)");
	expect(exp10(T(-400.0)),         minpos, "exp10(-400)");
	expect(pow(T(2.0), T(-1100.0)),  minpos, "pow(2, -1100)");
	expect(erfc(T(30.0)),            minpos, "erfc(30)");

	// a saturated odd power keeps its sign.  Only where the exponent is an exact
	// integer: takum_log holds 401 inexactly, and a negative base to a non-integer
	// power is NaR, correctly.
	if (double(T(401.0)) == 401.0 && double(T(400.0)) == 400.0) {
		expect(pow(T(-10.0), T(401.0)),  maxneg, "pow(-10, 401)");
		expect(pow(T(-10.0), T(400.0)),  maxpos, "pow(-10, 400)");
		expect(pow(T(-10.0), T(-401.0)), minneg, "pow(-10, -401)");
	}

	// genuine zeros and poles stay what they are
	const T zero(0.0);
	if (!pow(zero, T(-1.0)).isnar()) { ++fails; if (reportTestCases) std::cout << "FAIL " << tag << " pow(0, -1) must be NaR\n"; }
	if (!pow(zero, T(2.0)).iszero()) { ++fails; if (reportTestCases) std::cout << "FAIL " << tag << " pow(0, 2) must be 0\n"; }
	if (!sinh(zero).iszero())        { ++fails; if (reportTestCases) std::cout << "FAIL " << tag << " sinh(0) must be 0\n"; }
	if (!expm1(zero).iszero())       { ++fails; if (reportTestCases) std::cout << "FAIL " << tag << " expm1(0) must be 0\n"; }
	return fails;
}

// The linear takum's integer pow goes through the double too; takum_log's integer
// pow is exact in the log domain and saturates on its own.
template<typename T>
int VerifyIntegerPowSaturation(const std::string& tag, bool reportTestCases) {
	using sw::universal::SpecificValue;
	int fails = 0;
	Expect<T> expect{ fails, reportTestCases, tag };
	expect(pow(T(10.0), 400),    T(SpecificValue::maxpos), "pow(10, int 400)");
	expect(pow(T(-10.0), 401),   T(SpecificValue::maxneg), "pow(-10, int 401)");
	expect(pow(T(10.0), -400),   T(SpecificValue::minpos), "pow(10, int -400)");
	return fails;
}

// rbits = 5 reaches far past double's range, so a double overflow is not evidence
// that the true result exceeds maxpos: e^800 is an ordinary takum<32,5>.  Those
// must not be saturated -- that would be wrong by hundreds of orders of magnitude.
template<typename T>
int VerifyWideRangeNotSaturated(const std::string& tag, bool reportTestCases) {
	using sw::universal::SpecificValue;
	int fails = 0;
	const T maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
	if (exp(T(800.0)).raw_bits() == maxpos.raw_bits()) {
		++fails;
		if (reportTestCases) std::cout << "FAIL " << tag << " exp(800) saturated although e^800 is representable\n";
	}
	if (exp(T(-800.0)).raw_bits() == minpos.raw_bits()) {
		++fails;
		if (reportTestCases) std::cout << "FAIL " << tag << " exp(-800) saturated although e^-800 is representable\n";
	}
	return fails;
}

// Inside double's range the helper must be invisible: every finite, nonzero double
// result converts exactly as the plain constructor would.
template<typename T>
int VerifyInRangeUnchanged(const std::string& tag, bool reportTestCases) {
	int fails = 0;
	auto check = [&](const T& got, double r, const char* what, double x) {
		if (!std::isfinite(r) || r == 0.0) return;
		if (got.raw_bits() != T(r).raw_bits()) {
			++fails;
			if (reportTestCases) std::cout << "FAIL " << tag << ' ' << what << '(' << x << ") changed inside double's range\n";
		}
	};
	const T base(1.5);                      // takum_log cannot hold 1.5 exactly
	const double b = double(base);
	for (double x = -760.0; x <= 760.0; x += 0.731) {
		const T t(x);
		const double d = double(t);
		check(exp(t),   std::exp(d),          "exp",   x);
		check(exp2(t),  std::exp2(d),         "exp2",  x);
		check(sinh(t),  std::sinh(d),         "sinh",  x);
		check(cosh(t),  std::cosh(d),         "cosh",  x);
		check(erfc(t),  std::erfc(d),         "erfc",  x);
		check(pow(base, t), std::pow(b, d), "pow(1.5, x)", x);
	}
	return fails;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;
	std::string test_suite = "takum math saturation at double's range limits (#1624)";
	std::string test_tag   = "saturation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifySaturation<takum<16, 3, std::uint16_t>>("takum<16,3>", true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	nrOfFailedTestCases += ReportTestResult(VerifySaturation<takum<16, 3, std::uint16_t>>("takum<16,3>", reportTestCases), "takum<16,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<takum<32, 3, std::uint32_t>>("takum<32,3>", reportTestCases), "takum<32,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<takum<64, 3, std::uint64_t>>("takum<64,3>", reportTestCases), "takum<64,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<takum_log<16, 3, std::uint16_t>>("takum_log<16,3>", reportTestCases), "takum_log<16,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<takum_log<32, 3, std::uint32_t>>("takum_log<32,3>", reportTestCases), "takum_log<32,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowSaturation<takum<16, 3, std::uint16_t>>("takum<16,3>", reportTestCases), "takum<16,3> integer pow", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowSaturation<takum<32, 3, std::uint32_t>>("takum<32,3>", reportTestCases), "takum<32,3> integer pow", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWideRangeNotSaturated<takum<32, 5, std::uint32_t>>("takum<32,5>", reportTestCases), "takum<32,5> wide range", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyInRangeUnchanged<takum<32, 3, std::uint32_t>>("takum<32,3>", reportTestCases), "takum<32,3> in range", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyInRangeUnchanged<takum_log<32, 3, std::uint32_t>>("takum_log<32,3>", reportTestCases), "takum_log<32,3> in range", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
