// natural_divide_by_zero_optout.cpp: a NaturalNumber division by zero, with only the divide-by-zero
// exception opted out, is 0 on every limb layout; the domain exception still rejects a zero
// quotient of a nonzero divisor (#2131)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// domain exceptions on, division by zero opted out on its own switch
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#define INTEGER_THROW_DIVIDE_BY_ZERO 0
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_reporters.hpp>

static_assert(INTEGER_THROW_ARITHMETIC_EXCEPTION == 1, "domain exceptions are on");
static_assert(INTEGER_THROW_DIVIDE_BY_ZERO == 0, "division by zero is opted out");

namespace sw { namespace universal {

// 7 / 0 is the opted-out 0, silently and without the NaturalNumber domain exception; 1 / 2 is a zero
// quotient of a nonzero divisor, outside the domain, and throws. The exact-fit single limb and the
// general long-division paths must agree.
template<typename Natural>
int VerifyNaturalDivideByZero(bool reportTestCases) {
	int nrOfFailedTests = 0;
	Natural seven(7), one(1), two(2);
	Natural zero;
	zero.clear();   // the all-zeros encoding, outside the domain, only as the divisor
	try {
		Natural q = seven / zero;
		if (!q.iszero()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: 7 / 0 = " << q << " expected 0\n";
		}
		Natural r(seven);
		r /= zero;
		if (!r.iszero()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: 7 /= 0 = " << r << " expected 0\n";
		}
	}
	catch (const integer_encoding_exception& e) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: 7 / 0 threw the domain exception: " << e.what() << '\n';
	}
	bool thrown = false;
	try {
		Natural q = one / two;
		(void)q;
	}
	catch (const integer_encoding_exception&) {
		thrown = true;
	}
	if (!thrown) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: 1 / 2 did not throw\n";
	}
	if (seven / two != Natural(3) || seven % two != Natural(1)) {
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

	std::string test_suite  = "integer NaturalNumber division by zero, only that exception opted out";
	std::string test_tag    = "natural divide by zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyNaturalDivideByZero< integer<16, std::uint8_t, IntegerNumberType::NaturalNumber> >(true), "integer<16,uint8_t,Natural>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	// single limb of exactly nbits: the native-division path
	nrOfFailedTestCases += ReportTestResult(VerifyNaturalDivideByZero< integer< 16, std::uint16_t, IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<16,uint16_t,Natural>", test_tag);
	// several limbs, or a partial limb: the long-division path
	nrOfFailedTestCases += ReportTestResult(VerifyNaturalDivideByZero< integer< 16, std::uint8_t , IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<16,uint8_t,Natural>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaturalDivideByZero< integer< 16, std::uint32_t, IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<16,uint32_t,Natural>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaturalDivideByZero< integer<128, std::uint32_t, IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<128,uint32_t,Natural>", test_tag);
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
catch (const sw::universal::integer_arithmetic_exception& err) {
	std::cerr << "Caught unexpected integer arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::integer_internal_exception& err) {
	std::cerr << "Caught unexpected integer internal exception: " << err.what() << std::endl;
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
