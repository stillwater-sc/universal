// hyperbolic.cpp: regression tests for efloat hyperbolic functions.
//
// Categories tested:
//   - sinh, cosh, tanh
//   - asinh, acosh, atanh
//   - identities, round-trips, special values, high-precision residuals (Issue #1114)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<unsigned nlimbs>
bool IsClose(const sw::universal::efloat<nlimbs>& a, double target_val, double tolerance = 1e-12) {
	double diff = std::abs(double(a) - target_val);
	return diff <= tolerance * (1.0 + std::abs(target_val));
}

// |a - b| as a binary scale; a very negative scale means agreement to that
// many bits. Returns a large negative sentinel when the difference is zero.
template<unsigned nlimbs>
int64_t AgreementScale(const sw::universal::efloat<nlimbs>& a, const sw::universal::efloat<nlimbs>& b) {
	using namespace sw::universal;
	efloat<nlimbs> d = a - b;
	d.setsign(false);
	return d.iszero() ? -1000000 : d.scale();
}

int VerifyEfloatHyperbolic(bool reportTestCases) {
	using namespace sw::universal;
	int failures = 0;

	// ---------------------------------------------------------------------
	// 1. sinh
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying sinh...\n";
	{
		clear_efloat_exceptions();
		if (sinh(efloat<8>(0.0)) != 0.0) {
			if (reportTestCases)
				std::cout << "    FAIL: sinh(0) != 0\n";
			++failures;
		}
		if (!IsClose(sinh(efloat<8>(1.0)), std::sinh(1.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: sinh(1)\n";
			++failures;
		}
		if (!IsClose(sinh(efloat<8>(-1.0)), std::sinh(-1.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: sinh(-1)\n";
			++failures;
		}
		if (!IsClose(sinh(efloat<8>(3.5)), std::sinh(3.5))) {
			if (reportTestCases)
				std::cout << "    FAIL: sinh(3.5)\n";
			++failures;
		}
		// small argument (exercises the expm1 accurate path)
		if (!IsClose(sinh(efloat<8>(1e-3)), std::sinh(1e-3))) {
			if (reportTestCases)
				std::cout << "    FAIL: sinh(1e-3)\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 2. cosh
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying cosh...\n";
	{
		if (cosh(efloat<8>(0.0)) != 1.0) {
			if (reportTestCases)
				std::cout << "    FAIL: cosh(0) != 1\n";
			++failures;
		}
		if (!IsClose(cosh(efloat<8>(1.0)), std::cosh(1.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: cosh(1)\n";
			++failures;
		}
		if (!IsClose(cosh(efloat<8>(-2.0)), std::cosh(-2.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: cosh(-2)\n";
			++failures;
		}
		if (!IsClose(cosh(efloat<8>(4.0)), std::cosh(4.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: cosh(4)\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 3. tanh
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying tanh...\n";
	{
		if (tanh(efloat<8>(0.0)) != 0.0) {
			if (reportTestCases)
				std::cout << "    FAIL: tanh(0) != 0\n";
			++failures;
		}
		if (!IsClose(tanh(efloat<8>(1.0)), std::tanh(1.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: tanh(1)\n";
			++failures;
		}
		if (!IsClose(tanh(efloat<8>(-0.5)), std::tanh(-0.5))) {
			if (reportTestCases)
				std::cout << "    FAIL: tanh(-0.5)\n";
			++failures;
		}
		if (!IsClose(tanh(efloat<8>(1e-4)), std::tanh(1e-4))) {
			if (reportTestCases)
				std::cout << "    FAIL: tanh(1e-4)\n";
			++failures;
		}
		// saturation for large argument
		if (!IsClose(tanh(efloat<8>(40.0)), 1.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: tanh(40) != 1\n";
			++failures;
		}
		if (!IsClose(tanh(efloat<8>(-40.0)), -1.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: tanh(-40) != -1\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 4. asinh / acosh / atanh
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying inverse hyperbolic...\n";
	{
		if (asinh(efloat<8>(0.0)) != 0.0) {
			if (reportTestCases)
				std::cout << "    FAIL: asinh(0) != 0\n";
			++failures;
		}
		if (!IsClose(asinh(efloat<8>(1.0)), std::asinh(1.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: asinh(1)\n";
			++failures;
		}
		if (!IsClose(asinh(efloat<8>(-3.0)), std::asinh(-3.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: asinh(-3)\n";
			++failures;
		}
		if (!IsClose(asinh(efloat<8>(1e-4)), std::asinh(1e-4))) {
			if (reportTestCases)
				std::cout << "    FAIL: asinh(1e-4)\n";
			++failures;
		}

		if (!IsClose(acosh(efloat<8>(1.0)), 0.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: acosh(1) != 0\n";
			++failures;
		}
		if (!IsClose(acosh(efloat<8>(2.0)), std::acosh(2.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: acosh(2)\n";
			++failures;
		}
		if (!IsClose(acosh(efloat<8>(1.0001)), std::acosh(1.0001), 1e-9)) {
			if (reportTestCases)
				std::cout << "    FAIL: acosh(1.0001)\n";
			++failures;
		}

		if (atanh(efloat<8>(0.0)) != 0.0) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(0) != 0\n";
			++failures;
		}
		if (!IsClose(atanh(efloat<8>(0.5)), std::atanh(0.5))) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(0.5)\n";
			++failures;
		}
		if (!IsClose(atanh(efloat<8>(-0.9)), std::atanh(-0.9))) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(-0.9)\n";
			++failures;
		}
		if (!IsClose(atanh(efloat<8>(1e-4)), std::atanh(1e-4))) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(1e-4)\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 5. Identity cosh^2 - sinh^2 == 1 at full precision (oracle-grade)
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying cosh^2 - sinh^2 == 1...\n";
	{
		efloat<16> x(0.7);  // 512-bit working precision
		efloat<16> s = sinh(x), c = cosh(x);
		efloat<16> id = c * c - s * s;
		int64_t    sc = AgreementScale(id, efloat<16>(1.0));
		// require agreement to well beyond double (< 2^-200 ~ 60 digits)
		if (sc > -200) {
			if (reportTestCases)
				std::cout << "    FAIL: cosh^2-sinh^2-1 scale=" << sc << " (want <=-200)\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 6. tanh == sinh/cosh (internal consistency)
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying tanh == sinh/cosh...\n";
	{
		efloat<16> x(1.3);
		efloat<16> lhs = tanh(x);
		efloat<16> rhs = sinh(x) / cosh(x);
		int64_t    sc  = AgreementScale(lhs, rhs);
		if (sc > -200) {
			if (reportTestCases)
				std::cout << "    FAIL: tanh vs sinh/cosh scale=" << sc << "\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 7. Inverse round-trips at full precision (oracle-grade)
	//      asinh(sinh(x)) == x, acosh(cosh(x)) == x (x>0), atanh(tanh(x)) == x
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying inverse round-trips...\n";
	{
		efloat<16> x(0.9);
		int64_t    s1 = AgreementScale(asinh(sinh(x)), x);
		int64_t    s2 = AgreementScale(acosh(cosh(x)), x);
		int64_t    s3 = AgreementScale(atanh(tanh(x)), x);
		if (s1 > -200) {
			if (reportTestCases)
				std::cout << "    FAIL: asinh(sinh(x)) scale=" << s1 << "\n";
			++failures;
		}
		if (s2 > -200) {
			if (reportTestCases)
				std::cout << "    FAIL: acosh(cosh(x)) scale=" << s2 << "\n";
			++failures;
		}
		if (s3 > -200) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(tanh(x)) scale=" << s3 << "\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 8. Special values
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying special values...\n";
	{
		efloat<8> pinf;
		pinf.setinf(false);
		efloat<8> ninf;
		ninf.setinf(true);
		efloat<8> nan;
		nan.setnan();

		if (!sinh(pinf).isinf() || sinh(pinf).sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: sinh(+inf)\n";
			++failures;
		}
		if (!sinh(ninf).isinf() || sinh(ninf).sign() != -1) {
			if (reportTestCases)
				std::cout << "    FAIL: sinh(-inf)\n";
			++failures;
		}
		if (!cosh(pinf).isinf() || cosh(pinf).sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: cosh(+inf)\n";
			++failures;
		}
		if (!cosh(ninf).isinf() || cosh(ninf).sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: cosh(-inf) != +inf\n";
			++failures;
		}
		if (!IsClose(tanh(pinf), 1.0) || !IsClose(tanh(ninf), -1.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: tanh(+/-inf)\n";
			++failures;
		}
		if (!sinh(nan).isnan() || !cosh(nan).isnan() || !tanh(nan).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: fwd(nan)\n";
			++failures;
		}

		if (!asinh(pinf).isinf() || asinh(pinf).sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: asinh(+inf)\n";
			++failures;
		}
		if (!asinh(ninf).isinf() || asinh(ninf).sign() != -1) {
			if (reportTestCases)
				std::cout << "    FAIL: asinh(-inf)\n";
			++failures;
		}
		if (!acosh(pinf).isinf() || acosh(pinf).sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: acosh(+inf)\n";
			++failures;
		}

		// domain errors -> NaN
		clear_efloat_exceptions();
		if (!acosh(efloat<8>(0.5)).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: acosh(0.5) not NaN\n";
			++failures;
		}
		if (!acosh(ninf).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: acosh(-inf) not NaN\n";
			++failures;
		}
		if (!atanh(efloat<8>(2.0)).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(2) not NaN\n";
			++failures;
		}
		if (!atanh(pinf).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(+inf) not NaN\n";
			++failures;
		}

		// poles -> +/-inf
		if (!atanh(efloat<8>(1.0)).isinf() || atanh(efloat<8>(1.0)).sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(1) != +inf\n";
			++failures;
		}
		if (!atanh(efloat<8>(-1.0)).isinf() || atanh(efloat<8>(-1.0)).sign() != -1) {
			if (reportTestCases)
				std::cout << "    FAIL: atanh(-1) != -inf\n";
			++failures;
		}
	}

	return failures;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat mathematical hyperbolic library";
	std::string test_tag            = "hyperbolic";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatHyperbolic(reportTestCases), "efloat", "hyperbolic manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyEfloatHyperbolic(reportTestCases), "efloat", "hyperbolic foundational");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
