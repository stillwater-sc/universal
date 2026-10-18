// sqrt.cpp: edecimal sqrt is the exact floor of the square root (#1725)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// edecimal sqrt was edecimal(std::sqrt(double(n))): exact only while n fits a double, wrong in
// its trailing digits past ~16 digits and inf past ~308.  It is now the integer root
// floor(sqrt(n)) at any size.
//
// Two oracles, neither of them the implementation's Newton iteration:
//   - every n below 10^5: r^2 <= n < (r + 1)^2, by edecimal multiplication;
//   - integers of up to 400 digits, perfect squares and their neighbours, against Python's
//     math.isqrt (tools/generators/edecimal_sqrt_gen.py).
#include <universal/utility/directives.hpp>
#include <cerrno>
#include <string>

#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/edecimal_sqrt_vectors.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

int VerifySmall(bool reportTestCases, long limit) {
	int fails = 0;
	for (long i = 0; i < limit; ++i) {
		const edecimal n(i), r = sqrt(n);
		edecimal r1 = r;
		r1 += edecimal(1);
		if (!(r * r <= n) || !(n < r1 * r1)) {
			if (reportTestCases && fails < 4) std::cerr << "FAIL: sqrt(" << n << ") = " << r << '\n';
			++fails;
		}
	}
	return fails;
}

int VerifyVectors(bool reportTestCases) {
	int fails = 0;
	for (const auto& v : edecimal_sqrt_verification::isqrt_vectors) {
		edecimal n, want;
		n.parse(v.n);
		want.parse(v.root);
		const edecimal r = sqrt(n);
		if (!(r == want)) {
			if (reportTestCases && fails < 4) std::cerr << "FAIL: sqrt of a " << std::string(v.n).size() << "-digit integer\n   got  " << r << "\n   want " << want << '\n';
			++fails;
		}
	}
	return fails;
}

int VerifyNegative(bool reportTestCases) {
#if EDECIMAL_THROW_ARITHMETIC_EXCEPTION
	try { (void)sqrt(edecimal(-4)); }
	catch (const edecimal_negative_sqrt_arg&) { return 0; }
	if (reportTestCases) std::cerr << "FAIL: sqrt(-4) did not throw edecimal_negative_sqrt_arg\n";
	return 1;
#else
	errno = 0;
	const edecimal r = sqrt(edecimal(-4));
	if (r.iszero() && errno == EDOM) return 0;
	if (reportTestCases) std::cerr << "FAIL: sqrt(-4) = " << r << " errno " << errno << '\n';
	return 1;
#endif
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "edecimal sqrt (#1725)";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyVectors(reportTestCases), "isqrt vectors", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySmall(reportTestCases, 20000), "n < 20000", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyVectors(reportTestCases), "isqrt vectors", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNegative(reportTestCases), "negative argument", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySmall(reportTestCases, 100000), "n < 100000", test_tag);
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
