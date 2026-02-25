// classify.cpp: test suite runner for double-double cascade (dd_cascade) classification functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

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

	std::string test_suite  = "double-double cascade mathlib classification function validation";
	std::string test_tag    = "isfinite/isinf/isnan/isnormal/isdenorm/iszero/signbit";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	std::cout << "fpclassify(qnan)  = " << fpclassify(std::numeric_limits<dd_cascade>::quiet_NaN())     << " == " << FP_NAN << "\n";
	std::cout << "fpclassify(snan)  = " << fpclassify(std::numeric_limits<dd_cascade>::signaling_NaN()) << " == " << FP_NAN << "\n";
	std::cout << "fpclassify(-inf)  = " << fpclassify(-std::numeric_limits<dd_cascade>::infinity())  << " == " << FP_INFINITE << "\n";
	std::cout << "fpclassify(-1.0)  = " << fpclassify(dd_cascade(-1.0))   << " == " << FP_NORMAL << "\n";
	std::cout << "fpclassify(-0.0)  = " << fpclassify(dd_cascade("-0.0")) << " == " << FP_ZERO << "\n";
	std::cout << "fpclassify(0.0)   = " << fpclassify(dd_cascade("0.0"))   << " == " << FP_ZERO << "\n";
	std::cout << "fpclassify(1.0)   = " << fpclassify(dd_cascade(1.0))     << " == " << FP_NORMAL << "\n";
	std::cout << "fpclassify(inf)   = " << fpclassify(std::numeric_limits<dd_cascade>::infinity())    << " == " << FP_INFINITE << "\n";
	std::cout << "\n";

	std::cout << "isfinite(qnan)    = " << isfinite(std::numeric_limits<dd_cascade>::quiet_NaN()) << "\n";
	std::cout << "isfinite(snan)    = " << isfinite(std::numeric_limits<dd_cascade>::signaling_NaN()) << "\n";
	std::cout << "isfinite(-inf)    = " << isfinite(-std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "isfinite(-1.0)    = " << isfinite(dd_cascade(-1.0)) << "\n";
	std::cout << "isfinite(-0.0)    = " << isfinite(dd_cascade("-0.0")) << "\n";
	std::cout << "isfinite(0.0)     = " << isfinite(dd_cascade("0.0")) << "\n";
	std::cout << "isfinite(1.0)     = " << isfinite(dd_cascade(1.0)) << "\n";
	std::cout << "isfinite(inf)     = " << isfinite(std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "\n";

	std::cout << "isinf(qnan)       = " << isinf(std::numeric_limits<dd_cascade>::quiet_NaN()) << "\n";
	std::cout << "isinf(snan)       = " << isinf(std::numeric_limits<dd_cascade>::signaling_NaN()) << "\n";
	std::cout << "isinf(-inf)       = " << isinf(-std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "isinf(-1.0)       = " << isinf(dd_cascade(-1.0)) << "\n";
	std::cout << "isinf(-0.0)       = " << isinf(dd_cascade("-0.0")) << "\n";
	std::cout << "isinf(0.0)        = " << isinf(dd_cascade("0.0")) << "\n";
	std::cout << "isinf(1.0)        = " << isinf(dd_cascade(1.0)) << "\n";
	std::cout << "isinf(inf)        = " << isinf(std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "\n";

	std::cout << "isnan(qnan)       = " << isnan(std::numeric_limits<dd_cascade>::quiet_NaN()) << "\n";
	std::cout << "isnan(snan)       = " << isnan(std::numeric_limits<dd_cascade>::signaling_NaN()) << "\n";
	std::cout << "isnan(-inf)       = " << isnan(-std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "isnan(-1.0)       = " << isnan(dd_cascade(-1.0)) << "\n";
	std::cout << "isnan(-0.0)       = " << isnan(dd_cascade("-0.0")) << "\n";
	std::cout << "isnan(0.0)        = " << isnan(dd_cascade("0.0")) << "\n";
	std::cout << "isnan(1.0)        = " << isnan(dd_cascade(1.0)) << "\n";
	std::cout << "isnan(inf)        = " << isnan(std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "\n";

	std::cout << "isnormal(qnan)    = " << isnormal(std::numeric_limits<dd_cascade>::quiet_NaN()) << "\n";
	std::cout << "isnormal(snan)    = " << isnormal(std::numeric_limits<dd_cascade>::signaling_NaN()) << "\n";
	std::cout << "isnormal(-inf)    = " << isnormal(-std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "isnormal(-1.0)    = " << isnormal(dd_cascade(-1.0)) << "\n";
	std::cout << "isnormal(-0.0)    = " << isnormal(dd_cascade("-0.0")) << "\n";
	std::cout << "isnormal(0.0)     = " << isnormal(dd_cascade("0.0")) << "\n";
	std::cout << "isnormal(1.0)     = " << isnormal(dd_cascade(1.0)) << "\n";
	std::cout << "isnormal(inf)     = " << isnormal(std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "\n";

	constexpr double minpos = std::numeric_limits<double>::min();
	std::cout << to_binary(minpos) << " : " << minpos << '\n';
	double subnormal = minpos / 2.0;
	std::cout << to_binary(subnormal) << " : " << subnormal << '\n';

	std::cout << "isdenorm(qnan)    = " << isdenorm(std::numeric_limits<dd_cascade>::quiet_NaN()) << "\n";
	std::cout << "isdenorm(snan)    = " << isdenorm(std::numeric_limits<dd_cascade>::signaling_NaN()) << "\n";
	std::cout << "isdenorm(-inf)    = " << isdenorm(-std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "isdenorm(-1.0)    = " << isdenorm(dd_cascade(-1.0)) << "\n";
	std::cout << "isdenorm(-0.0)    = " << isdenorm(dd_cascade("-0.0")) << "\n";
	std::cout << "isdenorm(0.0)     = " << isdenorm(dd_cascade("0.0")) << "\n";
	std::cout << "isdenorm(subnorm) = " << isdenorm(subnormal) << "\n";
	std::cout << "isdenorm(1.0)     = " << isdenorm(dd_cascade(1.0)) << "\n";
	std::cout << "isdenorm(inf)     = " << isdenorm(std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "\n";

	std::cout << "iszero(qnan)      = " << iszero(std::numeric_limits<dd_cascade>::quiet_NaN()) << "\n";
	std::cout << "iszero(snan)      = " << iszero(std::numeric_limits<dd_cascade>::signaling_NaN()) << "\n";
	std::cout << "iszero(-inf)      = " << iszero(-std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "iszero(-1.0)      = " << iszero(dd_cascade(-1.0)) << "\n";
	std::cout << "iszero(-0.0)      = " << iszero(dd_cascade("-0.0")) << "\n";
	std::cout << "iszero(0.0)       = " << iszero(dd_cascade("0.0")) << "\n";
	std::cout << "iszero(1.0)       = " << iszero(dd_cascade(1.0)) << "\n";
	std::cout << "iszero(inf)       = " << iszero(std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "\n";

	std::cout << "signbit(-inf)     = " << signbit(-std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "signbit(-1.0)     = " << signbit(dd_cascade(-1.0)) << "\n";
	std::cout << "signbit(-0.0)     = " << signbit(dd_cascade("-0.0")) << "\n";
	std::cout << "signbit(0.0)      = " << signbit(dd_cascade("0.0")) << "\n";
	std::cout << "signbit(1.0)      = " << signbit(dd_cascade(1.0)) << "\n";
	std::cout << "signbit(inf)      = " << signbit(std::numeric_limits<dd_cascade>::infinity()) << "\n";
	std::cout << "\n";

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
