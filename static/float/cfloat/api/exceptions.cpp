//  exceptions.cpp : test suite for arithmetic exceptions of classic float numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
// configure the number system
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// With exceptions enabled a signalling NaN operand throws, and a quiet NaN operand, of either
	// sign, propagates as a quiet NaN instead of reaching the significand arithmetic (#1896)
	template<typename Cfloat>
	int VerifyNanOperands(bool reportTestCases, const std::string& type) {
		int nrOfFailedTestCases = 0;
		auto fail = [&](const std::string& what) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << type << ' ' << what << '\n';
		};
		const Cfloat x(1.5f);
		Cfloat qnan;
		qnan.setnan(NAN_TYPE_QUIET);
		for (const Cfloat& q : { qnan, Cfloat(-qnan) }) {
			if (!q.isnan(NAN_TYPE_QUIET)) continue;   // sign-typed NaNs: -qNaN is the signalling NaN
			try {
				if (!(q + x).isnan(NAN_TYPE_QUIET)) fail("qNaN + x is not a quiet NaN");
				if (!(x + q).isnan(NAN_TYPE_QUIET)) fail("x + qNaN is not a quiet NaN");
				if (!(q - x).isnan(NAN_TYPE_QUIET)) fail("qNaN - x is not a quiet NaN");
				if (!(q * x).isnan(NAN_TYPE_QUIET)) fail("qNaN * x is not a quiet NaN");
				if (!(x * q).isnan(NAN_TYPE_QUIET)) fail("x * qNaN is not a quiet NaN");
			}
			catch (const cfloat_arithmetic_exception&) {
				fail("a quiet NaN operand threw");
			}
		}
		Cfloat snan;
		snan.setnan(NAN_TYPE_SIGNALLING);
		bool threw = false;
		try { Cfloat r = snan + x; (void)r; }
		catch (const cfloat_operand_is_nan&) { threw = true; }
		if (!threw) fail("sNaN + x did not throw");
		threw = false;
		try { Cfloat r = x * snan; (void)r; }
		catch (const cfloat_operand_is_nan&) { threw = true; }
		if (!threw) fail("x * sNaN did not throw");
		return nrOfFailedTestCases;
	}

} }

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

	std::string test_suite  = "cfloat arithmetic exceptions ";
	std::string test_tag    = "exceptions";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	using Number = sw::universal::cfloat<16, 5, uint16_t>;

	nrOfFailedTestCases += TestDivisionByZero<Number>(reportTestCases);

	// TODO: need to implement conversion from signed int
	// nrOfFailedTestCases += TestNegativeSqrtArgument<Number>(reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNanOperands<half>(reportTestCases, "half"), "half", "NaN operands");
	nrOfFailedTestCases += ReportTestResult(VerifyNanOperands<cfloat<8, 4, std::uint8_t, true, true, false>>(reportTestCases, "cfloat<8,4,sub,max>"), "cfloat<8,4,sub,max>", "NaN operands");
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
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
