// divide_by_zero_optout.cpp: with the exception opted out, a fixpnt division by zero returns
// maxpos or maxneg (Saturate) or 0 (Modulo)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// opt out through the older switch: FIXPNT_THROW_DIVIDE_BY_ZERO follows it when not set itself (#1894)
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

static_assert(FIXPNT_THROW_DIVIDE_BY_ZERO == 0, "FIXPNT_THROW_DIVIDE_BY_ZERO follows FIXPNT_THROW_ARITHMETIC_EXCEPTION");

namespace sw { namespace universal {

template<typename Fixpnt>
int VerifyDivideByZeroValue(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](const Fixpnt& result, const Fixpnt& expected, const char* label) {
		if (result != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << to_binary(result) << " expected " << to_binary(expected) << '\n';
		}
	};
	Fixpnt one(1), minusone(-1), zero(0);
	Fixpnt maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	if constexpr (Fixpnt::arithmetic == Saturate) {
		// saturate by the sign of the dividend
		check(one / zero, maxpos, "1 / 0");
		check(minusone / zero, maxneg, "-1 / 0");
		check(zero / zero, maxpos, "0 / 0");
		Fixpnt r(minusone); r /= zero;
		check(r, maxneg, "-1 /= 0");
	}
	else {
		check(one / zero, zero, "1 / 0");
		check(minusone / zero, zero, "-1 / 0");
		check(zero / zero, zero, "0 / 0");
		Fixpnt r(minusone); r /= zero;
		check(r, zero, "-1 /= 0");
	}
	check(one / Fixpnt(2), Fixpnt(0.5), "1 / 2");
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

	std::string test_suite  = "fixpnt division by zero, exception opted out";
	std::string test_tag    = "divide by zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< fixpnt<16, 8, Saturate, std::uint8_t> >(true), "fixpnt<16,8,Saturate>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< fixpnt<  8, 4, Saturate, std::uint8_t > >(reportTestCases), "fixpnt<8,4,Saturate>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< fixpnt<  8, 4, Modulo,   std::uint8_t > >(reportTestCases), "fixpnt<8,4,Modulo>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< fixpnt< 16, 8, Saturate, std::uint8_t > >(reportTestCases), "fixpnt<16,8,Saturate>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< fixpnt< 16, 8, Modulo,   std::uint16_t> >(reportTestCases), "fixpnt<16,8,Modulo>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< fixpnt< 64,32, Saturate, std::uint8_t > >(reportTestCases), "fixpnt<64,32,Saturate,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue< fixpnt< 64,32, Modulo,   std::uint8_t > >(reportTestCases), "fixpnt<64,32,Modulo,uint8_t>", test_tag);
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
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Caught unexpected fixpnt arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Caught unexpected fixpnt internal exception: " << err.what() << std::endl;
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
