// increment.cpp: test suite runner for increment operator on classic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> increment operator validation";
	std::string test_tag    = "increment";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		constexpr bool hasSubnormals = true;
		constexpr bool hasMaxExpValues = true;
		constexpr bool notSaturating = false;
		using Cfloat = cfloat<4, 1, uint8_t, hasSubnormals, hasMaxExpValues, notSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatIncrement< Cfloat >(true), "cfloat<4,1,uint8_t,subnormals,max-exponent values,!saturating>", test_tag);
	}

	{	
		constexpr bool hasSubnormals = true;
		constexpr bool hasMaxExpValues = true;
		constexpr bool isSaturating = true;
		using Cfloat = cfloat<17, 3, uint8_t, hasSubnormals, hasMaxExpValues, !isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatIncrement< Cfloat >(true), "cfloat<17,3,uint8_t,subnormals,max-exponent values,!saturating>", test_tag);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1

	cfloat<9, 2, uint8_t, false, false, false> a;
	a.setbits(0x140);
	std::cout << to_binary(a) << " : " << a << '\n';
	a++;
	std::cout << to_binary(a) << " : " << a << '\n';
	a++;
	std::cout << to_binary(a) << " : " << a << '\n';

	// normal encoding only
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<4, 2, uint8_t, false, false, false> >(reportTestCases), type_tag(cfloat<4, 2, uint8_t, false, false, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2, uint8_t, false, false, false> >(reportTestCases), type_tag(cfloat<8, 2, uint8_t, false, false, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2, uint8_t, false, false, false> >(reportTestCases), type_tag(cfloat<9, 2, uint8_t, false, false, false>()), test_tag);


	// subnormal + normal
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2, uint8_t, true, false, false> >(reportTestCases), type_tag(cfloat<8, 2, uint8_t, true, false, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2, uint8_t, true, false, false> >(reportTestCases), type_tag(cfloat<9, 2, uint8_t, true, false, false>()), test_tag);


	// normal + max-exponent value
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2, uint8_t, false, true, false> >(reportTestCases), type_tag(cfloat<8, 2, uint8_t, false, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2, uint8_t, false, true, false> >(reportTestCases), type_tag(cfloat<9, 2, uint8_t, false, true, false>()), test_tag);


	// subnormal + normal + max-exponent value

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement< cfloat<4, 1, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<4, 1, uint8_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<8, 2, uint8_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<9, 2, uint8_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<10, 3, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<10, 3, uint8_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<17, 3, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<17, 4, uint8_t, true, true, false>()), test_tag);

#ifdef LATER
	// these are failing because the test assumes that we jump around the encoding, where as operator++ just cycles through the encodings
	// simplified classic floats without subnormals nor max-exponent values
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<16, 5, uint32_t, false, false, false> >(reportTestCases), type_tag(cfloat<16, 5, uint32_t, false, false, false>()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<32, 8, uint32_t, false, false, false> >(reportTestCases), type_tag(cfloat<32, 8, uint32_t, false, false, false>()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<64, 11, uint32_t, false, false, false> >(reportTestCases), type_tag(cfloat<64, 11, uint32_t, false, false, false>()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<128, 15, uint32_t, false, false, false> >(reportTestCases), type_tag(cfloat<128, 15, uint32_t, false, false, false>()), test_tag + std::string(" special cases"));
#endif

	// traditional, IEEE-754 standard floats with just subnormals
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< half >(reportTestCases), type_tag(half()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< single >(reportTestCases), type_tag(single()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< duble >(reportTestCases), type_tag(duble()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< quad >(reportTestCases), type_tag(quad()), test_tag + std::string(" special cases"));

	// fancy, fully encoded classic floats
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<16, 5, uint32_t, true, true, false> >(reportTestCases), type_tag(cfloat<16, 5, uint32_t, true, true, false>()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<32, 8, uint32_t, true, true, false> >(reportTestCases), type_tag(cfloat<32, 8, uint32_t, true, true, false>()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<64, 11, uint32_t, true, true, false> >(reportTestCases), type_tag(cfloat<64, 11, uint32_t, true, true, false>()), test_tag + std::string(" special cases"));
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<128, 15, uint32_t, true, true, false> >(reportTestCases), type_tag(cfloat<128, 15, uint32_t, true, true, false>()), test_tag + std::string(" special cases"));

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
