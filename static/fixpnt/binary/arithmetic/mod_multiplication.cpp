// mod_multiplication.cpp: test suite runner for arbitrary configuration fixed-point modulo multiplication
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
template<size_t nbits, size_t rbits, bool arithmetic, typename bt, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	sw::universal::fixpnt<nbits, rbits, arithmetic, bt> a, b, cref, result;
	sw::universal::blockbinary<2 * nbits> full;
	a = _a;
	b = _b;
	result = a * b;
	Ty ref = _a * _b;
	full = (long long)ref;
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

	std::string test_suite  = "fixed-point modular multiplication";
	std::string test_tag    = "modular multiplication";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blockbinary<8> a, b;
		a.setbits(0x02);
		b.setbits(0x80);
		blockbinary<16> c;
		c = urmul2(a, b);
		std::cout << a << " * " << b << " = " << c << " : " << (long long)c << '\n';
		c = urmul2(b, a);
		std::cout << b << " * " << a << " = " << c << " : " << (long long)c << '\n';
	}

	{
		// generate overflow condition to observe modulo behavior
		fixpnt<8, 4> a, b, c;
		a.setbits(0x70);
		b.setbits(0x70);
		c = a * b;
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		std::cout << to_binary(c) << " : " << c << '\n';
		// the full precision version of this multiply
		fixpnt<17, 8> aa(a), bb(b), cc;
		aa <<= 4;
		bb <<= 4;
		cc = aa * bb;
		std::cout << to_binary(aa) << " : " << aa << '\n';
		std::cout << to_binary(bb) << " : " << bb << '\n';
		std::cout << to_binary(cc) << " : " << cc << '\n';
		c = cc;  // rounding 
		std::cout << to_binary(c)  << " : " << c << '\n';
	}

	float fa = -8.0f;
	float fb = 0.125f;
	GenerateTestCase<8, 4>(fa, fb);
	GenerateTestCase<8, 4>(fb, fa);

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 0>(0.5f, 1.5f); 
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<4,0,Modulo,uint8_t>", test_tag); 
	
	GenerateTestCase<4, 1>(-0.5f, -3.5f);
	GenerateTestCase<4, 1>(-3.5f, -0.5f);

	//	GenerateTestCase<8, 1>(0.5f, 0.5f);
	GenerateTestCase<8, 1>(0.5f, -32.0f);
	GenerateTestCase<8, 1>(-64.0f, 0.5f);
	GenerateTestCase<8, 1>(0.0f, -64.0f);
	GenerateTestCase<8, 1>(1.5f, -16.0f);
	GenerateTestCase<8, 1>(1.5f, -64.0f);
	GenerateTestCase<8, 1>(-64.0f, -63.5f);
	GenerateTestCase<8, 1>(-63.5f, -64.0f);
	GenerateTestCase<8, 1>(-64.0f, -63.0f);
	GenerateTestCase<8, 1>(-64.0f, -62.5f);

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 1, Modulo, uint8_t>(reportTestCases), "fixpnt<8,1,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<8,4,Modulo,uint8_t>", test_tag);

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<4,0,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 1, Modulo, uint8_t>(reportTestCases), "fixpnt<4,1,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 2, Modulo, uint8_t>(reportTestCases), "fixpnt<4,2,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 3, Modulo, uint8_t>(reportTestCases), "fixpnt<4,3,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<4,4,Modulo,uint8_t>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 0, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 0,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 1, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 1,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 2, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 2,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 3, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 3,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 4, 4, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 4,Modulo,uint8_t >", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 0, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 0,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 1, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 1,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 2, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 2,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 3, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 3,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 4, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 4,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 5, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 5,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 6, 6, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 6,Modulo,uint8_t >", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 4, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 4,Modulo,uint8_t >", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 0,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 0,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 1,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 1,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 2,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 2,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 3,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 3,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 4,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 4,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 5,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 5,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 6,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 6,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 7,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 7,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< 8, 8,Modulo,uint8_t>(reportTestCases), "fixpnt< 8, 8,Modulo,uint8_t >", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 0, Modulo, uint8_t >(reportTestCases), "fixpnt<10, 0,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 4, Modulo, uint8_t >(reportTestCases), "fixpnt<10, 4,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 7, Modulo, uint8_t >(reportTestCases), "fixpnt<10, 7,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 8, Modulo, uint8_t >(reportTestCases), "fixpnt<10, 8,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 9, Modulo, uint8_t >(reportTestCases), "fixpnt<10, 9,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10,10, Modulo, uint8_t >(reportTestCases), "fixpnt<10,10,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10,10, Modulo, uint16_t>(reportTestCases), "fixpnt<10,10,Modulo,uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 0,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 4,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 7, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 7,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 8, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 8,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 9, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 9,Modulo,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12,12, Modulo, uint8_t>(reportTestCases), "fixpnt<12,12,Modulo,uint8_t >", test_tag);
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
