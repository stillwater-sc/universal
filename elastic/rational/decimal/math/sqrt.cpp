// sqrt.cpp: erational sqrt: the exact rational root, or the nearest dyadic at the requested precision (#1725)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source
// license.
//
// erational sqrt was erational(std::sqrt(double(x))): 53 bits at most, and it wrote
// "erational_negative_sqrt_arg" to std::cerr on every call.  It now returns the exact root when
// the reduced numerator and denominator are perfect squares, and otherwise the nearest multiple
// of 2^-k to the root, ties to even, at the requested number of significant bits.
//
// The oracle is tools/generators/erational_sqrt_gen.py: the same definition evaluated with
// Python's exact integers (math.isqrt, Fraction), sharing no code with edecimal.
#include <universal/utility/directives.hpp>
#include <cerrno>
#include <iostream>
#include <string>

#include <universal/number/erational/erational.hpp>
#include <universal/verification/erational_sqrt_vectors.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

edecimal from_string(const char* s) {
	edecimal d;
	d.parse(s);
	return d;
}

int VerifyVectors(bool reportTestCases) {
	int fails = 0;
	for (const auto& v : erational_sqrt_verification::rational_sqrt) {
		erational x;
		x.setnumerator(from_string(v.p));
		x.setdenominator(from_string(v.q));
		const erational r = sqrt(x, v.bits);
		if (!(r.top() == from_string(v.num)) || !(r.bottom() == from_string(v.den)) || r.isneg()) {
			if (reportTestCases && fails < 4) {
				std::cerr << "FAIL: sqrt(" << v.p << " / " << v.q << ", " << v.bits << " bits)\n   got  "
				          << r.top() << " / " << r.bottom() << "\n   want " << v.num << " / " << v.den << '\n';
			}
			++fails;
		}
	}
	return fails;
}

int VerifySpecialValues(bool reportTestCases) {
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << what << '\n'; }
	};
	check(sqrt(erational(0)).iszero(), "sqrt(0) is not 0");
	check(sqrt(erational(9, 4)) == erational(3, 2), "sqrt(9/4) is not 3/2");
	check(sqrt(erational(1)) == erational(1), "sqrt(1) is not 1");
	// an unreduced argument: the constructor keeps 8/18, whose root 2/3 is exact
	{
		const erational r = sqrt(erational(8, 18), 2);
		check(r.top() == edecimal(2) && r.bottom() == edecimal(3), "sqrt(8/18) is not 2/3");
		erational y;
		y.setnumerator(edecimal(200));
		y.setdenominator(edecimal(50));                          // 4, unreduced
		const erational ry = sqrt(y);
		check(ry.top() == edecimal(2) && ry.bottom() == edecimal(1), "sqrt(200/50) is not 2");
	}
	// the default precision is 106 bits: the root of 2 has a 106-bit numerator over 2^105
	const erational r2 = sqrt(erational(2));
	check(r2.bottom() == from_string("40564819207303340847894502572032"), "sqrt(2) is not at 106 bits");
#if ERATIONAL_THROW_ARITHMETIC_EXCEPTION
	bool thrown = false;
	try { (void)sqrt(erational(-4, 1)); } catch (const erational_negative_sqrt_arg&) { thrown = true; }
	check(thrown, "sqrt(-4) did not throw erational_negative_sqrt_arg");
#else
	errno = 0;
	check(sqrt(erational(-4, 1)).isnan() && errno == EDOM, "sqrt(-4) is not NaN with EDOM");
#endif
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "erational sqrt (#1725)";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues(reportTestCases), "special values", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyVectors(reportTestCases), "exact-integer vectors", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues(reportTestCases), "special values", test_tag);
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
catch (const sw::universal::erational_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::erational_internal_exception& err) {
	std::cerr << "Uncaught internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
