// divide_by_zero_optout.cpp: with the exception opted out, an einteger division or remainder by zero is 0
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// opt out through the older switch: EINTEGER_THROW_DIVIDE_BY_ZERO follows it when not set itself (#1894)
#define EINTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_reporters.hpp>

static_assert(EINTEGER_THROW_DIVIDE_BY_ZERO == 0, "EINTEGER_THROW_DIVIDE_BY_ZERO follows EINTEGER_THROW_ARITHMETIC_EXCEPTION");

namespace sw { namespace universal {

template<typename EInteger>
int VerifyDivideByZeroValue(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](const EInteger& result, const char* label) {
		if (!result.iszero()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << result << " expected 0\n";
		}
	};
	EInteger seven(7), zero(0), big;
	big.assign("123456789012345678901234567890");  // several limbs
	check(seven / zero, "7 / 0");
	check(seven % zero, "7 % 0");
	check(zero / zero, "0 / 0");
	check(big / zero, "big / 0");
	check(big % zero, "big % 0");
	EInteger r(seven); r /= zero; check(r, "7 /= 0");
	r = seven; r %= zero; check(r, "7 %= 0");
	// reduce() overwrites whatever its outputs held
	EInteger q(5), rem(5);
	q.reduce(seven, zero, rem);
	check(q, "reduce(7, 0) quotient");
	check(rem, "reduce(7, 0) remainder");
	// a zero held in more than one limb, which setblock() can build
	EInteger widezero; widezero.setblock(1, 0);
	check(seven / widezero, "7 / [0,0]");
	check(seven % widezero, "7 % [0,0]");
	if (seven / EInteger(2) != EInteger(3) || seven % EInteger(2) != EInteger(1)) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: 7 / 2 or 7 % 2\n";
	}
	return nrOfFailedTests;
}

} } // namespace sw::universal

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

	std::string test_suite  = "einteger division by zero, exception opted out";
	std::string test_tag    = "divide by zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< einteger<std::uint32_t> >(true), "einteger", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< einteger<std::uint8_t> >(reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< einteger<std::uint16_t> >(reportTestCases), "einteger<uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< einteger<std::uint32_t> >(reportTestCases), "einteger<uint32_t>", test_tag);
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
catch (const sw::universal::einteger_arithmetic_exception& err) {
	std::cerr << "Caught unexpected einteger arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_internal_exception& err) {
	std::cerr << "Caught unexpected einteger internal exception: " << err.what() << std::endl;
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
