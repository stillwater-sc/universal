// arithmetic_divide.cpp: functional tests for posit division
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_DIV

// minimum set of include files to reflect source code dependencies
#include "../../posit/posit.hpp"
#include "../../posit/numeric_limits.hpp"
#include "../../posit/specializations.hpp"
// posit type manipulators such as pretty printers
#include "../../posit/posit_manipulators.hpp"
#include "../../posit/math_functions.hpp"
// test helpers
#include "../test_helpers.hpp"
#include "../posit_math_helpers.hpp"
#include "../posit_test_randoms.hpp"

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::unum::posit<nbits, es> pa, pb, pref, pdiv;
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
	sw::unum::posit<nbits, es> p_plus_eps(1), p_minus_eps(1), p_result;
	p_plus_eps++;
	p_minus_eps--;
	p_result = p_plus_eps / p_minus_eps;
	if (es < 2) {
		std::cout << posit_descriptor.str() << " minpos = " << std::fixed << std::setprecision(nbits) << sw::unum::minpos_value<nbits, es>() << std::dec << std::endl;
	}
	else {
		std::cout << posit_descriptor.str() << " minpos = " << std::setprecision(nbits) << sw::unum::minpos_value<nbits, es>() << std::endl;

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
	sw::unum::posit<16, 1> a, b, c, d;
	a.set_raw_bits(20479);
	b.set_raw_bits(2);
	c.set_raw_bits(16383);
	d.set_raw_bits(16385);

	GenerateTestCase<16, 1>(b, a);
	GenerateTestCase<16, 1>(a, b);
	GenerateTestCase<16, 1>(c, d);
	GenerateTestCase<16, 1>(d, c);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	cout << "Posit division validation" << endl;

	std::string tag = "Division failed: ";

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

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<2, 0>("Manual Testing", true), "posit<2,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 0>("Manual Testing", true), "posit<3,0>", "division");	
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 1>("Manual Testing", true), "posit<3,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<4, 0>("Manual Testing", true), "posit<4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 0>("Manual Testing", true), "posit<5,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 0>("Manual Testing", true), "posit<8,0>", "division");

#else

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 2>(tag, bReportIndividualTestCases), "posit<3,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 3>(tag, bReportIndividualTestCases), "posit<3,3>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<4, 2>(tag, bReportIndividualTestCases), "posit<4,2>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 3>(tag, bReportIndividualTestCases), "posit<5,3>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 4>(tag, bReportIndividualTestCases), "posit<6,4>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "division");

#if STRESS_TESTING
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<16,1>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<24,1>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<32,1>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<32,2>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<48,2>", "division");

        // nbits=64 requires long double compiler support
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<64,2>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<64,3>", "division");
        // posit<64,4> is hitting subnormal numbers
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<64,4>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "division");

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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

