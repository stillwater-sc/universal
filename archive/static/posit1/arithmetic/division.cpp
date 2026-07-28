// division.cpp: test suite runner for posit division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_DIV
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, pdiv;
	pa = a;
	pb = b;
	ref = a / b;
	pref = ref;
	pdiv = pa / pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " / " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " / " << pb.get() << " = " << pdiv.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pdiv ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t es>
void GenerateWorstCaseDivision() {
	std::stringstream posit_descriptor;
	posit_descriptor << "posit<" << nbits << ", " << es << ">";
	sw::universal::posit<nbits, es> p_plus_eps(1), p_minus_eps(1), p_result;
	p_plus_eps++;
	p_minus_eps--;
	p_result = p_plus_eps / p_minus_eps;
	if constexpr (es < 2) {
		std::cout << posit_descriptor.str() << " minpos = " << std::fixed << std::setprecision(nbits) << sw::universal::posit<nbits, es>(sw::universal::SpecificValue::minpos) << std::dec << std::endl;
	}
	else {
		std::cout << posit_descriptor.str() << " minpos = " << std::setprecision(nbits) << sw::universal::posit<nbits, es>(sw::universal::SpecificValue::minpos) << std::endl;

	}
	std::cout << p_plus_eps.get() << " / " << p_minus_eps.get() << " = " << p_result.get() << std::endl;
	std::cout << std::setprecision(nbits - 2) << std::setw(nbits) << p_plus_eps << " / " << std::setw(nbits) << p_minus_eps << " = " << std::setw(nbits) << p_result << std::endl;
	std::cout << std::endl;
}

/*
Posit division validation
posit<8, 0> minpos = 0.01562500
01000001 / 00111111 = 01000010
1.031250 / 0.984375 = 1.062500

posit<12, 0> minpos = 0.000976562500
010000000001 / 001111111111 = 010000000010
1.0019531250 / 0.9990234375 = 1.0039062500

posit<16, 1> minpos = 0.0000000037252903
0100000000000001 / 0011111111111111 = 0100000000000010
1.00024414062500 / 0.99987792968750 = 1.00048828125000

posit<20, 1> minpos = 0.00000000001455191523
01000000000000000001 / 00111111111111111111 = 01000000000000000010
1.000015258789062500 / 0.999992370605468750 = 1.000030517578125000

posit<24, 1> minpos = 0.000000000000056843418861
010000000000000000000001 / 001111111111111111111111 = 010000000000000000000010
1.0000009536743164062500 / 0.9999995231628417968750 = 1.0000019073486328125000

posit<28, 1> minpos = 0.0000000000000002220446049250
0100000000000000000000000001 / 0011111111111111111111111111 = 0100000000000000000000000010
1.00000005960464477539062500 / 0.99999997019767761230468750 = 1.00000011920928955078125000

posit<32, 1> minpos = 0.00000000000000000086736173798840
01000000000000000000000000000001 / 00111111111111111111111111111111 = 01000000000000000000000000000010
1.000000003725290298461914062500 / 0.999999998137354850769042968750 = 1.000000007450580596923828125000

posit<32, 2> minpos = 0.00000000000000000000000000000000
01000000000000000000000000000001 / 00111111111111111111111111111111 = 01000000000000000000000000000010
1.000000007450580596923828125000 / 0.999999996274709701538085937500 = 1.000000014901161193847656250000

posit<40, 2> minpos = 0.0000000000000000000000000000000000000000
0100000000000000000000000000000000000001 / 0011111111111111111111111111111111111111 = 0100000000000000000000000000000000000010
1.00000000002910383045673370361328125000 / 0.99999999998544808477163314819335937500 = 1.00000000005820766091346740722656250000

posit<48, 2> minpos = 0.000000000000000000000000000000000000000000000000
010000000000000000000000000000000000000000000001 / 001111111111111111111111111111111111111111111111 = 010000000000000000000000000000000000000000000010
1.0000000000001136868377216160297393798828125000 / 0.9999999999999431565811391919851303100585937500 = 1.0000000000002273736754432320594787597656250000

posit<56, 2> minpos = 0.00000000000000000000000000000000000000000000000000000000
01000000000000000000000000000000000000000000000000000001 / 00111111111111111111111111111111111111111111111111111111 = 01000000000000000000000000000000000000000000000000000010
1.000000000000000444089209850062616169452667236328125000 / 0.999999999999999777955395074968691915273666381835937500 = 1.000000000000000888178419700125232338905334472656250000

posit<60, 3> minpos = 0.000000000000000000000000000000000000000000000000000000000000
010000000000000000000000000000000000000000000000000000000001 / 001111111111111111111111111111111111111111111111111111111111 = 010000000000000000000000000000000000000000000000000000000010
1.0000000000000000000000000000000000000000000000000000000000 / 1.0000000000000000000000000000000000000000000000000000000000 = 1.0000000000000000000000000000000000000000000000000000000000

last one posit<60,3> shows doubles aren't enough to represent these posit values and the values get rounded to 1.0
*/
void EnumerateToughDivisions() {
	GenerateWorstCaseDivision<8, 0>();
	GenerateWorstCaseDivision<12, 0>();
	GenerateWorstCaseDivision<16, 1>();
	GenerateWorstCaseDivision<20, 1>();
	GenerateWorstCaseDivision<24, 1>();
	GenerateWorstCaseDivision<28, 1>();
	GenerateWorstCaseDivision<32, 1>();
	GenerateWorstCaseDivision<32, 2>();
	GenerateWorstCaseDivision<40, 2>();
	GenerateWorstCaseDivision<48, 2>();
	GenerateWorstCaseDivision<56, 2>();
	GenerateWorstCaseDivision<60, 3>();
}

/*
As we discussed, I think the following cases are tricky for the divide function. I discovered them when trying to approximate x/y with x times (1/y). All are in the <16,1> environment, so you should be able to test them easily.

Let

A = posit represented by integer 20479 (value is 8191/4096 = 1.999755859375)
B = posit represented by integer 2 (value is 1/67108864 = 0.00000001490116119384765625)
C = posit represented by integer 16383 (value is 8191/8192 = 0.9998779296875)
D = posit represented by integer 16385 (value is 4097/4096 = 1.000244140625)

Then the divide routine should return the following:

B / A = posit represented by integer 2 (that is, the division leaves B unchanged)
A / B = posit represented by integer 32766 (value is 67108864)
C / D = posit represented by integer 16381 (value is 0.996337890625)
D / C = posit represented by integer 16386 (value is 1.00048828125)

Notice that multiplying the B/A and A/B results gives 1 exactly, but multiplying the C/D and D/C results gives 1.000121891498565673828125.
*/
void ToughDivisions2() {
	sw::universal::posit<16, 1> a, b, c, d;
	a.setbits(20479);
	b.setbits(2);
	c.setbits(16383);
	d.setbits(16385);

	GenerateTestCase<16, 1>(b, a);
	GenerateTestCase<16, 1>(a, b);
	GenerateTestCase<16, 1>(c, d);
	GenerateTestCase<16, 1>(d, c);
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

	std::string test_suite  = "posit division verification";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	ToughDivisions2();

	return 0;

	const size_t nbits = 16;
	const size_t es = 1;
	double a, b;
	a = 0.9999999999;
	b = 1.0000000001;
	b = 0.5000000001;
	GenerateTestCase<16, 1, double>(a, b);
	GenerateTestCase<20, 1, double>(a, b);
	GenerateTestCase<32, 1, double>(a, b);
	GenerateTestCase<40, 1, double>(a, b);
	GenerateTestCase<48, 1, double>(a, b);



	// Generate the worst fraction pressure for different posit configurations
	EnumerateToughDivisions();

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<2, 0>>(reportTestCases), "posit<2,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<3, 0>>(reportTestCases), "posit<3,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<3, 1>>(reportTestCases), "posit<3,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<4, 0>>(reportTestCases), "posit<4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 0>>(reportTestCases), "posit<5,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 0>>(reportTestCases), "posit<8,0>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<2, 0>>(reportTestCases), "posit< 2,0>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<3, 0>>(reportTestCases), "posit< 3,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<3, 1>>(reportTestCases), "posit< 3,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<3, 2>>(reportTestCases), "posit< 3,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<3, 3>>(reportTestCases), "posit< 3,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<4, 0>>(reportTestCases), "posit< 4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<4, 1>>(reportTestCases), "posit< 4,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<4, 2>>(reportTestCases), "posit< 4,2>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 0>>(reportTestCases), "posit< 5,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 1>>(reportTestCases), "posit< 5,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 2>>(reportTestCases), "posit< 5,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 3>>(reportTestCases), "posit< 5,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 0>>(reportTestCases), "posit< 6,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 1>>(reportTestCases), "posit< 6,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 2>>(reportTestCases), "posit< 6,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 3>>(reportTestCases), "posit< 6,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 4>>(reportTestCases), "posit< 6,4>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 0>>(reportTestCases), "posit< 7,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 1>>(reportTestCases), "posit< 7,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 2>>(reportTestCases), "posit< 7,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 3>>(reportTestCases), "posit< 7,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 4>>(reportTestCases), "posit< 7,4>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 0>>(reportTestCases), "posit< 8,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 1>>(reportTestCases), "posit< 8,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 2>>(reportTestCases), "posit< 8,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 3>>(reportTestCases), "posit< 8,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 4>>(reportTestCases), "posit< 8,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 5>>(reportTestCases), "posit< 8,5>", "division");
#endif

#if REGRESSION_LEVEL_2
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<10, 0>>(reportTestCases), "posit<10,0>", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<10, 1>>(reportTestCases), "posit<10,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<10, 2>>(reportTestCases), "posit<10,2>", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<10, 3>>(reportTestCases), "posit<10,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<16, 2>>(reportTestCases, OPCODE_DIV, 1000), "posit<16,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<24, 2>>(reportTestCases, OPCODE_DIV, 1000), "posit<24,2>", "division");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<20, 1>>(reportTestCases, OPCODE_DIV, 1000), "posit<20,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<28, 1>>(reportTestCases, OPCODE_DIV, 1000), "posit<28,1>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 1>>(reportTestCases, OPCODE_DIV, 1000), "posit<32,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 2>>(reportTestCases, OPCODE_DIV, 1000), "posit<32,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 3>>(reportTestCases, OPCODE_DIV, 1000), "posit<32,3>", "division");
#endif

#if REGRESSION_LEVEL_4
	// nbits = 48 also shows failures
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<48, 2>>(reportTestCases, OPCODE_DIV, 1000), "posit<48,2>", "division");

    // nbits=64 requires long double compiler support
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(reportTestCases, OPCODE_DIV, 1000), "posit<64,2>", "division");
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 3>>(reportTestCases, OPCODE_DIV, 1000), "posit<64,3>", "division");
    // posit<64,4> is hitting subnormal numbers
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 4>>(reportTestCases, OPCODE_DIV, 1000), "posit<64,4>", "division");

#ifdef HARDWARE_ACCELERATION
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<12, 1>>(reportTestCases), "posit<12,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<14, 1>>(reportTestCases), "posit<14,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<16, 1>>(reportTestCases), "posit<16,1>", "division");
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

