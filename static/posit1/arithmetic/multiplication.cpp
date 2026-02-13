// multiplication.cpp: test suite runner for posit multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing a MUL the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_MUL
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_mul
template<unsigned nbits, unsigned es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, pmul;
	pa = a;
	pb = b;
	ref = a * b;
	pref = ref;
	pmul = pa * pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " * " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " * " << pb.get() << " = " << pmul.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pmul ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<unsigned nbits, unsigned es>
void GenerateTestCase( sw::universal::posit<nbits,es> pa, sw::universal::posit<nbits,es> pb, sw::universal::posit<nbits, es> pref) {
	double a = double(pa);
	double b = double(pb);
	double ref = a * b;
	//sw::universal::posit<nbits, es> pref = ref;
	sw::universal::posit<nbits, es> pmul = pa * pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " * " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " * " << pb.get() << " = " << pmul.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pmul ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

/*
Operand1 Operand2  bad     golden
======== ======== ======== ========
00000002 93ff6977 fffffffa fffffff9
00000002 b61e2f1f fffffffe fffffffd
308566ef 7fffffff 7ffffffe 7fffffff
308566ef 80000001 80000002 80000001
503f248b 7ffffffe 7ffffffe 7fffffff
503f248b 80000002 80000002 80000001
7ffffffe 503f248b 7ffffffe 7fffffff
7fffffff 308566ef 7ffffffe 7fffffff
80000001 308566ef 80000002 80000001
80000002 503f248b 80000002 80000001
93ff6977 00000002 fffffffa fffffff9
b61e2f1f 00000002 fffffffe fffffffd
b61e2f1f fffffffe 00000002 00000003
fffffffe b61e2f1f 00000002 00000003
*/
void DifficultRoundingCases() {
	sw::universal::posit<32, 2> a, b, pref;
	std::vector<uint32_t> cases = {
		0x00000002, 0x93ff6977, 0xfffffffa, 0xfffffff9,
		0x00000002, 0xb61e2f1f, 0xfffffffe, 0xfffffffd,
		0x308566ef, 0x7fffffff, 0x7ffffffe, 0x7fffffff,
		0x308566ef, 0x80000001, 0x80000002, 0x80000001,
		0x503f248b, 0x7ffffffe, 0x7ffffffe, 0x7fffffff,
		0x503f248b, 0x80000002, 0x80000002, 0x80000001,
		0x7ffffffe, 0x503f248b, 0x7ffffffe, 0x7fffffff,
		0x7fffffff, 0x308566ef, 0x7ffffffe, 0x7fffffff,
		0x80000001, 0x308566ef, 0x80000002, 0x80000001,
		0x80000002, 0x503f248b, 0x80000002, 0x80000001,
		0x93ff6977, 0x00000002, 0xfffffffa, 0xfffffff9,
		0xb61e2f1f, 0x00000002, 0xfffffffe, 0xfffffffd,
		0xb61e2f1f, 0xfffffffe, 0x00000002, 0x00000003,
		0xfffffffe, 0xb61e2f1f, 0x00000002, 0x00000003,
	};
	// unsigned nrOfTests = cases.size() >> 2;  // divide by 4
	for (unsigned i = 0; i < cases.size(); i+= 4) {
		a.setbits(cases[i]);
		b.setbits(cases[i + 1]);
		pref.setbits(cases[i + 3]);
		//cout << a.get() << " * " << b.get() << " = " << pref.get() << endl;
		GenerateTestCase(a, b, pref);
	}
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

	std::string test_suite  = "posit multiplication verification";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	/*
	Suppose posits x and y are

	x = 0x3BCB2F0D representing the value 0.7371054179966449737548828125
	y = 0x3ADA6F8A representing the value 0.678329028189182281494140625

	If you use IEEE float you get exactly 1/2, which is incorrect. 
	The correct answer is
	z = 0x38000001 representing the value 0.5000000037252902984619140625
	*/

	posit<32, 2> x, y, z;
	x.setbits(0x3BCB2F0D);
	y.setbits(0x3ADA6F8A);
	z = x * y;
	bitblock<32> raw = z.get();
	cout << components_to_string(z) << "\n0x" << hex << raw.to_ulong() <<  endl;


	float fa, fb;
	fa = 0.0f; fb = INFINITY;
	std::cout << fa << " " << fb << std::endl;
	GenerateTestCase<4,0, float>(fa, fb);
	GenerateTestCase<16, 1, float>(float(minpos_value<16, 1>()), float(maxpos_value<16, 1>()));

	DifficultRoundingCases();
	

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<2, 0>>(reportTestCases), "posit<2,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<3, 0>>(reportTestCases), "posit<3,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<3, 1>>(reportTestCases), "posit<3,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<4, 0>>(reportTestCases), "posit<4,0>", "multiplication");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<2, 0>>(reportTestCases), "posit< 2,0>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<3, 0>>(reportTestCases), "posit< 3,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<3, 1>>(reportTestCases), "posit< 3,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<3, 2>>(reportTestCases), "posit< 3,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<3, 3>>(reportTestCases), "posit< 3,3>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<4, 0>>(reportTestCases), "posit< 4,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<4, 1>>(reportTestCases), "posit< 4,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<4, 2>>(reportTestCases), "posit< 4,2>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<5, 0>>(reportTestCases), "posit< 5,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<5, 1>>(reportTestCases), "posit< 5,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<5, 2>>(reportTestCases), "posit< 5,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<5, 3>>(reportTestCases), "posit< 5,3>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<6, 0>>(reportTestCases), "posit< 6,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<6, 1>>(reportTestCases), "posit< 6,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<6, 2>>(reportTestCases), "posit< 6,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<6, 3>>(reportTestCases), "posit< 6,3>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<6, 4>>(reportTestCases), "posit< 6,4>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<7, 0>>(reportTestCases), "posit< 7,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<7, 1>>(reportTestCases), "posit< 7,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<7, 2>>(reportTestCases), "posit< 7,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<7, 3>>(reportTestCases), "posit< 7,3>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<7, 4>>(reportTestCases), "posit< 7,4>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<8, 0>>(reportTestCases), "posit< 8,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<8, 1>>(reportTestCases), "posit< 8,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<8, 2>>(reportTestCases), "posit< 8,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<8, 3>>(reportTestCases), "posit< 8,3>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<8, 4>>(reportTestCases), "posit< 8,4>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<8, 5>>(reportTestCases), "posit< 8,5>", "multiplication");
#endif

#if REGRESSION_LEVEL_2
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<10, 0>>(reportTestCases), "posit<10,0>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<10, 1>>(reportTestCases), "posit<10,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<10, 2>>(reportTestCases), "posit<10,2>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<10, 3>>(reportTestCases), "posit<10,3>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<16, 2>>(reportTestCases, OPCODE_MUL, 1000), "posit<16,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<24, 2>>(reportTestCases, OPCODE_MUL, 1000), "posit<24,2>", "multiplication");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 1>>(reportTestCases, OPCODE_MUL, 1000), "posit<32,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 2>>(reportTestCases, OPCODE_MUL, 1000), "posit<32,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 3>>(reportTestCases, OPCODE_MUL, 1000), "posit<32,3>", "multiplication");
#endif

#if REGRESSION_LEVEL_4
	// nbits=48 is also showing failures
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<48, 2>>(reportTestCases, OPCODE_MUL, 1000), "posit<48,2>", "multiplication");

	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(reportTestCases, OPCODE_MUL, 1000), "posit<64,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 3>>(reportTestCases, OPCODE_MUL, 1000), "posit<64,3>", "multiplication");
	// posit<64,4> is hitting subnormal numbers
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 4>>(reportTestCases, OPCODE_MUL, 1000), "posit<64,4>", "multiplication");

#ifdef HARDWARE_ACCELERATION
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<12, 1>>(reportTestCases), "posit<12,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<14, 1>>(reportTestCases), "posit<14,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posit<16, 1>>(reportTestCases), "posit<16,1>", "multiplication");
#endif // HARDWARE_ACCELERATION

#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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

