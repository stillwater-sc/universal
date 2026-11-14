// high_precision_path.cpp: complex<dd> and complex<qd> compute in dd and qd, whatever is included (#2060)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The trait that sends complex<T> down the path computing in T (#2026) was registered for dd and qd
// only by number/{dd,qd}/math/complex/complex.hpp. A program that included dd.hpp and
// <universal/math/complex.hpp>, and not that header, got the std::complex<double> path: 53 bits,
// silently. The trait now comes with the mathlib, as ereal's and efloat's do. This file includes
// exactly the two umbrellas, and checks the trait and a result past double precision.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/math/complex.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	// sqrt(2 + 0i)^2 - 2 and |3 + 4i| - 5, which the double path leaves at about 1e-16 and 0
	template<typename T>
	int VerifyComputesInT(double tolerance, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		if (!is_high_precision_v<T>) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "    FAIL is_high_precision is false without math/complex/complex.hpp\n";
		}
		using C = complex<T>;
		const C r = sqrt(C(T(2.0), T(0.0)));
		const T e = r.real() * r.real() - T(2.0);
		if (!(std::fabs(double(e)) <= tolerance)) {                 // a NaN residual fails too
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "    FAIL sqrt(2 + 0i)^2 - 2 = " << double(e) << ", above " << tolerance << '\n';
		}
		// atanh's real part at 1/2: atanh(1/2) = ln(3) / 2, to the type's precision
		const C h = atanh(C(T(0.5), T(0.0)));
		const T d = h.real() * T(2.0) - log(T(3.0));
		if (!(std::fabs(double(d)) <= tolerance)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "    FAIL 2 atanh(1/2) - ln 3 = " << double(d) << ", above " << tolerance << '\n';
		}
		return nrOfFailedTestCases;
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "complex<dd> and complex<qd> compute in their element type (#2060)";
	std::string test_tag    = "high-precision path";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyComputesInT<dd>(1e-30, reportTestCases), "dd", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyComputesInT<dd>(1e-30, reportTestCases), "dd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComputesInT<qd>(1e-60, reportTestCases), "qd", test_tag);
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
