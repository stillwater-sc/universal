// signed_zero.cpp: qd's odd functions return -0 at -0 (#2032)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// C99 Annex F: sin, tan, sinh, tanh, asin, atan, asinh, atanh, log1p, expm1 and sqrt return
// their argument at +-0, sign included; cos, cosh and exp return 1. sin, tan, log1p, expm1
// and sqrt used to return +0 at -0.
#include <universal/utility/directives.hpp>
#include <string>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	int VerifySignedZeroFunctions(bool reportTestCases) {
		int n = 0;
		for (double z : { 0.0, -0.0 }) {
			const qd a(z);
			const bool neg = signbit(a);
			struct { const char* name; qd value; } odd[] = {
				{ "sin", sin(a) },     { "tan", tan(a) },     { "sinh", sinh(a) },   { "tanh", tanh(a) },
				{ "asin", asin(a) },   { "atan", atan(a) },   { "asinh", asinh(a) }, { "atanh", atanh(a) },
				{ "log1p", log1p(a) }, { "expm1", expm1(a) }, { "sqrt", sqrt(a) },
			};
			for (const auto& f : odd) {
				if (!(f.value == qd(0)) || signbit(f.value) != neg) {
					++n;
					if (reportTestCases) std::cout << "    FAIL " << f.name << "(" << (neg ? "-0" : "+0") << ") = " << f.value
					                               << " (signbit " << signbit(f.value) << ")\n";
				}
			}
			struct { const char* name; qd value; } even[] = { { "cos", cos(a) }, { "cosh", cosh(a) }, { "exp", exp(a) } };
			for (const auto& f : even) {
				if (!(f.value == qd(1))) {
					++n;
					if (reportTestCases) std::cout << "    FAIL " << f.name << "(" << (neg ? "-0" : "+0") << ") = " << f.value << '\n';
				}
			}
		}
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

	std::string test_suite  = "quad-double odd functions at -0 (#2032)";
	std::string test_tag    = "signed zero";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySignedZeroFunctions(reportTestCases), "qd", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeroFunctions(reportTestCases), "qd", test_tag);
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
