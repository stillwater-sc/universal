// signed_zero.cpp: dd arithmetic gives a zero result the sign IEEE 754 gives it (#2032)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The error-free transformations behind dd's operators combine partial results with + and *,
// which turn every zero into +0: +0 * -1, -0 + -0 and -0 / 2 all came out +0. The sign of a
// zero picks the side of a branch cut, so complex<dd> atanh(2.5 - 0i) landed on the upper
// side of its cut (docs/algorithmic-details/complex-branch-cuts.md). The operators now take a
// zero result from the leading limbs, which double arithmetic signs correctly, and a zero
// converts to double, float and long double from its leading limb (-0 + +0 is +0).
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <string>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	// 1 for -0 or a negative value, 0 for +0, -1 for NaN
	int sign_of(const dd& v) {
		if (v.isnan()) return -1;
		return signbit(v) ? 1 : 0;
	}

	int expect(const dd& got, int sign, bool nan, const char* what, bool reportTestCases) {
		const bool ok = nan ? got.isnan() : (!got.isnan() && got == dd(0) && sign_of(got) == sign);
		if (ok) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << " = " << got << " (signbit " << sign_of(got) << ")\n";
		return 1;
	}

	int VerifySignedZeroArithmetic(bool reportTestCases) {
		const dd pz(0.0), nz(-0.0), one(1.0), m1(-1.0), two(2.0), inf(std::numeric_limits<double>::infinity());
		const dd third = one / dd(3.0);   // a value with every limb populated
		int n = 0;
		// multiplication: the sign is the XOR of the operands' signs
		n += expect(pz * m1, 1, false, "+0 * -1", reportTestCases);
		n += expect(m1 * pz, 1, false, "-1 * +0", reportTestCases);
		n += expect(nz * one, 1, false, "-0 * 1", reportTestCases);
		n += expect(nz * nz, 0, false, "-0 * -0", reportTestCases);
		n += expect(pz * pz, 0, false, "+0 * +0", reportTestCases);
		n += expect(nz * third, 1, false, "-0 * 1/3", reportTestCases);
		n += expect(-third * pz, 1, false, "-1/3 * +0", reportTestCases);
		n += expect(pz * -1.0, 1, false, "+0 * -1.0 (double)", reportTestCases);
		n += expect(inf * pz, 0, true, "inf * 0", reportTestCases);
		// addition and subtraction: -0 only for -0 + -0 and -0 - +0
		n += expect(nz + nz, 1, false, "-0 + -0", reportTestCases);
		n += expect(nz + pz, 0, false, "-0 + +0", reportTestCases);
		n += expect(pz + nz, 0, false, "+0 + -0", reportTestCases);
		n += expect(nz - pz, 1, false, "-0 - +0", reportTestCases);
		n += expect(pz - pz, 0, false, "+0 - +0", reportTestCases);
		n += expect(nz - nz, 0, false, "-0 - -0", reportTestCases);
		n += expect(third - third, 0, false, "1/3 - 1/3", reportTestCases);
		// division
		n += expect(nz / two, 1, false, "-0 / 2", reportTestCases);
		n += expect(pz / m1, 1, false, "+0 / -1", reportTestCases);
		n += expect(nz / m1, 0, false, "-0 / -1", reportTestCases);
		n += expect(nz / third, 1, false, "-0 / 1/3", reportTestCases);
		n += expect(pz / inf, 0, false, "+0 / inf", reportTestCases);
		n += expect(nz / inf, 1, false, "-0 / inf", reportTestCases);
		// compound assignment takes the same path
		dd v = pz;
		v *= m1;
		n += expect(v, 1, false, "+0 *= -1", reportTestCases);
		v = nz;
		v += nz;
		n += expect(v, 1, false, "-0 += -0", reportTestCases);
		v = nz;
		v /= two;
		n += expect(v, 1, false, "-0 /= 2", reportTestCases);
		// conversion to the native types keeps the sign: the trailing limbs of a zero are +0
		for (const dd& z : { nz, pz * m1, -pz }) {
			if (!std::signbit(double(z)) || !std::signbit(float(z))) {
				if (reportTestCases) std::cout << "    FAIL -0 converts to +0 (double " << double(z) << ")\n";
				++n;
			}
#if LONG_DOUBLE_SUPPORT
			if (!std::signbit(static_cast<long double>(z))) {   // the conversion exists only with long double support
				if (reportTestCases) std::cout << "    FAIL -0 converts to +0 as long double\n";
				++n;
			}
#endif
		}
		if (std::signbit(double(pz)) || std::signbit(double(nz * nz))) {
			if (reportTestCases) std::cout << "    FAIL +0 converts to -0\n";
			++n;
		}
		// nonzero results are unchanged
		if (!(third * dd(3.0) - one < dd(1e-30)) || !((two * third) / two == third)) {
			if (reportTestCases) std::cout << "    FAIL nonzero products changed\n";
			++n;
		}
		return n;
	}

	// abs, fma and a product that underflows to zero keep the IEEE sign too (#2060)
	int VerifySignedZeroFollowups(bool reportTestCases) {
		const dd pz(0.0), nz(-0.0), one(1.0), m1(-1.0), tiny(1e-200), third = one / dd(3.0);
		int n = 0;
		n += expect(abs(nz), 0, false, "abs(-0)", reportTestCases);
		n += expect(abs(pz), 0, false, "abs(+0)", reportTestCases);
		if (abs(-third) != third) {
			if (reportTestCases) std::cout << "    FAIL abs(-1/3) != 1/3\n";
			++n;
		}
		// fma: a zero product plus a zero c is -0 only when both are -0; an exact cancellation is +0
		n += expect(fma(nz, one, nz), 1, false, "fma(-0, 1, -0)", reportTestCases);
		n += expect(fma(pz, m1, nz), 1, false, "fma(+0, -1, -0)", reportTestCases);
		n += expect(fma(nz, one, pz), 0, false, "fma(-0, 1, +0)", reportTestCases);
		n += expect(fma(pz, one, pz), 0, false, "fma(+0, 1, +0)", reportTestCases);
		n += expect(fma(third, m1, third), 0, false, "fma(1/3, -1, 1/3)", reportTestCases);
		// an underflowed product plus a zero c has the product's sign
		n += expect(fma(tiny, -tiny, nz), 1, false, "fma(1e-200, -1e-200, -0)", reportTestCases);
		n += expect(fma(tiny, -tiny, pz), 1, false, "fma(1e-200, -1e-200, +0)", reportTestCases);
		// products that underflow: the sign is the XOR of the operands' signs
		n += expect(tiny * -tiny * tiny, 1, false, "1e-200 * -1e-200 * 1e-200", reportTestCases);
		n += expect(-tiny * -tiny * -tiny, 1, false, "-1e-200 * -1e-200 * -1e-200", reportTestCases);
		n += expect(tiny * tiny * tiny, 0, false, "1e-200 * 1e-200 * 1e-200", reportTestCases);
		n += expect((tiny * tiny) * -1e-200, 1, false, "(1e-200 * 1e-200) * -1e-200 (double)", reportTestCases);
		dd v = -tiny;
		v *= tiny;
		v *= tiny;
		n += expect(v, 1, false, "-1e-200 *= 1e-200 *= 1e-200", reportTestCases);
		return n;
	}
}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "double-double signed zero arithmetic (#2032)";
	std::string test_tag    = "signed zero";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySignedZeroArithmetic(reportTestCases), "dd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeroFollowups(reportTestCases), "dd abs, fma, underflow", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeroArithmetic(reportTestCases), "dd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeroFollowups(reportTestCases), "dd abs, fma, underflow", test_tag);
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
