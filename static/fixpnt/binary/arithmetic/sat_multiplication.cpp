// sat_multiplication.cpp: test suite runner for arbitrary configuration fixed-point Saturate multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	sw::universal::fixpnt<nbits, rbits, sw::universal::Saturate> a, b, cref, result;
	sw::universal::blockbinary<2 * nbits> full;
	a = _a;
	b = _b;
	result = a * b;
	Ty ref = _a * _b;
	full = ref;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits + 1) << _a << " * " << std::setw(nbits + 1) << _b << " = " << std::setw(nbits + 1) << ref << " (reference: " << to_binary(full) << ")" << std::endl;
	std::cout << std::setw(nbits + 1) << a << " * " << std::setw(nbits + 1) << b << " = " << std::setw(nbits + 1) << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl;
	std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(result) << " (reference: " << to_binary(cref) << ")   ";

	std::cout << std::endl << std::endl << std::dec << std::setprecision(oldPrecision);
}

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

	std::string test_suite  = "fixed-point saturating multiplication ";
	std::string test_tag    = "saturating multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blockbinary<8> a, b;
		a.set_raw_bits(0x02);
		b.set_raw_bits(0x80);
		blockbinary<16> c;
		c = urmul2(a, b);
		cout << a << " * " << b << " = " << c << " : " << (long long)c << endl;
		c = urmul2(b, a);
		cout << b << " * " << a << " = " << c << " : " << (long long)c << endl;
	}

	float fa = -8.0f;
	float fb = 0.125f;
	GenerateTestCase<8, 4>(fa, fb);
	GenerateTestCase<8, 4>(fb, fa);

	// generate individual testcases to hand trace/debug

	// fixpnt<4,1>
	GenerateTestCase<4, 1>( 1.0f,  2.0f);
	GenerateTestCase<4, 1>(-0.5f, -3.5f);
	GenerateTestCase<4, 1>(-3.5f, -0.5f);
	GenerateTestCase<4, 1>( 1.5f,  2.5f);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 1, Saturate, uint8_t>(reportTestCases), "fixpnt<4,1,Saturate,uint8_t>", test_tag);

	cout << endl;

	// fixpnt<6,2>
	GenerateTestCase<6, 2>(0.25f, -8.0f);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 2, Saturate, uint8_t>(reportTestCases), "fixpnt<6,2,Saturate,uint8_t>", test_tag);

	cout << endl;

	// fixpnt<6,5>
	GenerateTestCase<6, 5>(0.03125f, -1.0f);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 5, Saturate, uint8_t>(reportTestCases), "fixpnt<6,5,Saturate,uint8_t>", test_tag);

	cout << endl;

	// fixpnt<8,4>
	GenerateTestCase<8, 4>(1.125f, -7.0625f);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 4, Saturate, uint8_t>(reportTestCases), "fixpnt<8,4,Saturate,uint8_t>", test_tag);

	// fixpnt<8,8>
	GenerateTestCase<8, 8>(0.01171875f, 0.3359375f);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 8, Saturate, uint8_t>(reportTestCases), "fixpnt<8,8,Saturate,uint8_t>", test_tag);

	// fixpnt<10,9>
	GenerateTestCase<10,9>(0.251953125f, 0.994140625f);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10,9, Saturate, uint8_t>(reportTestCases), "fixpnt<10,9,Saturate,uint8_t>", test_tag);

#ifdef REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 0, Saturate, uint8_t>(reportTestCases), "fixpnt<4,0,Saturate,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 1, Saturate, uint8_t>(reportTestCases), "fixpnt<4,1,Saturate,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 2, Saturate, uint8_t>(reportTestCases), "fixpnt<4,2,Saturate,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 3, Saturate, uint8_t>(reportTestCases), "fixpnt<4,3,Saturate,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 4, Saturate, uint8_t>(reportTestCases), "fixpnt<4,4,Saturate,uint8_t>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 0, Saturate, uint8_t >(reportTestCases), "fixpnt< 4, 0,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 1, Saturate, uint8_t >(reportTestCases), "fixpnt< 4, 1,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 2, Saturate, uint8_t >(reportTestCases), "fixpnt< 4, 2,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 3, Saturate, uint8_t >(reportTestCases), "fixpnt< 4, 3,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 4, Saturate, uint8_t >(reportTestCases), "fixpnt< 4, 4,Saturate,uint8_t >", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 0, Saturate, uint8_t >(reportTestCases), "fixpnt< 6, 0,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 1, Saturate, uint8_t >(reportTestCases), "fixpnt< 6, 1,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 2, Saturate, uint8_t >(reportTestCases), "fixpnt< 6, 2,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 3, Saturate, uint8_t >(reportTestCases), "fixpnt< 6, 3,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 4, Saturate, uint8_t >(reportTestCases), "fixpnt< 6, 4,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 5, Saturate, uint8_t >(reportTestCases), "fixpnt< 6, 5,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 6, Saturate, uint8_t >(reportTestCases), "fixpnt< 6, 6,Saturate,uint8_t >", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 0, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 0,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 1, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 1,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 2, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 2,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 3, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 3,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 4, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 4,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 5, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 5,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 6, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 6,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 7, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 7,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 8, Saturate, uint8_t >(reportTestCases), "fixpnt< 8, 8,Saturate,uint8_t >", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 0, Saturate, uint8_t >(reportTestCases), "fixpnt<10, 0,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 3, Saturate, uint8_t >(reportTestCases), "fixpnt<10, 3,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 4, Saturate, uint8_t >(reportTestCases), "fixpnt<10, 4,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 7, Saturate, uint8_t >(reportTestCases), "fixpnt<10, 7,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 8, Saturate, uint8_t >(reportTestCases), "fixpnt<10, 8,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 9, Saturate, uint8_t >(reportTestCases), "fixpnt<10, 9,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10,10, Saturate, uint8_t >(reportTestCases), "fixpnt<10,10,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10,10, Saturate, uint16_t>(reportTestCases), "fixpnt<10,10,Saturate,uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 0, Saturate, uint8_t >(reportTestCases), "fixpnt<12,0,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 4, Saturate, uint8_t >(reportTestCases), "fixpnt<12,4,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 7, Saturate, uint8_t >(reportTestCases), "fixpnt<12,7,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 8, Saturate, uint8_t >(reportTestCases), "fixpnt<12,8,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 9, Saturate, uint8_t >(reportTestCases), "fixpnt<12,9,Saturate,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12,12, Saturate, uint8_t >(reportTestCases), "fixpnt<12,12,Saturate,uint8_t >", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
