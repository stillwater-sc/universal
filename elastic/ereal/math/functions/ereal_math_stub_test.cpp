// ereal_math_stub_test.cpp: cross-cutting smoke test for the ereal mathlib.
//
// Calls every mathlib function once on a representative in-domain input and
// asserts the result is finite. This is a breadth-first "is everything wired
// and callable" regression that complements the per-function property files
// (classify/numerics/.../error_and_gamma); it deliberately does not re-test
// accuracy. Standardized as part of #950.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>

#include <cmath>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	// Smoke test: each function evaluated on a valid input must produce a finite
	// result (catches link/dispatch regressions and gross NaN/Inf failures).
	template<typename Real>
	int VerifyMathSmoke(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		Real x(2.0), y(3.0);

		auto finite = [&](const char* name, const Real& z) {
			if (!std::isfinite(double(z))) {
				if (reportTestCases) std::cout << "    FAIL " << name << " is not finite\n";
				++nrOfFailedTestCases;
			}
		};

		// numeric support
		int e;
		finite("frexp", frexp(x, &e));
		finite("ldexp", ldexp(x, 3));
		finite("copysign", copysign(x, Real(-1.0)));
		finite("abs", abs(Real(-2.0)));
		// truncation
		finite("floor", floor(Real(2.7)));
		finite("ceil", ceil(Real(2.3)));
		finite("trunc", trunc(Real(2.7)));
		finite("round", round(Real(2.5)));
		// min/max
		finite("min", min(x, y));
		finite("max", max(x, y));
		// fractional
		finite("fmod", fmod(Real(7.0), Real(3.0)));
		finite("remainder", remainder(Real(7.0), Real(3.0)));
		// hypot
		finite("hypot2", hypot(x, y));
		finite("hypot3", hypot(x, y, Real(4.0)));
		// roots
		finite("sqrt", sqrt(x));
		finite("cbrt", cbrt(Real(8.0)));
		// exponentials
		finite("exp", exp(x));
		finite("exp2", exp2(x));
		finite("exp10", exp10(x));
		finite("expm1", expm1(Real(0.1)));
		// logarithms
		finite("log", log(x));
		finite("log2", log2(x));
		finite("log10", log10(x));
		finite("log1p", log1p(Real(0.1)));
		// powers
		finite("pow", pow(x, y));
		finite("pown", pown(x, 3));
		// trigonometric
		finite("sin", sin(Real(1.0)));
		finite("cos", cos(Real(1.0)));
		finite("tan", tan(Real(1.0)));
		finite("asin", asin(Real(0.5)));
		finite("acos", acos(Real(0.5)));
		finite("atan", atan(Real(1.0)));
		finite("atan2", atan2(y, x));
		// hyperbolic
		finite("sinh", sinh(x));
		finite("cosh", cosh(x));
		finite("tanh", tanh(x));
		finite("asinh", asinh(x));
		finite("acosh", acosh(x));
		finite("atanh", atanh(Real(0.5)));
		// error and gamma
		finite("erf", erf(x));
		finite("erfc", erfc(x));
		finite("tgamma", tgamma(x));
		finite("lgamma", lgamma(x));
		// next
		finite("nextafter", nextafter(x, y));

		return nrOfFailedTestCases;
	}

}  // anonymous namespace

// Regression testing guards
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib smoke test";
	std::string test_tag    = "mathlib smoke";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyMathSmoke<ereal<>>(reportTestCases), "mathlib(ereal) smoke", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
