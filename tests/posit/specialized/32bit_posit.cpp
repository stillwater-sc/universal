// 32bit_posit.cpp: Functionality tests for standard 32-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
// Configure the posit template environment
// first: enable fast specialized posit<32,2>
//#define POSIT_FAST_SPECIALIZATION   // turns on all fast specializations
#define POSIT_FAST_POSIT_32_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>
#include "../../test_helpers.hpp"
#include "../../posit_test_helpers.hpp"

/*
Standard posit with nbits = 32 have es = 2 exponent bits.
*/

#define SOFTPOSIT_CMP_
#ifdef SOFTPOSIT_CMP
#include "./softposit_cmp.hpp"
void GenerateP32Test(int opcode, uint32_t _a, uint32_t _b, uint32_t _c) {
	using namespace std;
	using namespace sw::unum;

	posit32_t a, b, c;
	a = _a;
	b = _b;
	switch (opcode) {
	case OPCODE_ADD:
		c = p32_add(a, b);
		break;
	case OPCODE_SUB:
		c = p32_sub(a, b);
		break;
	case OPCODE_MUL:
		c = p32_mul(a, b);
		break;
	case OPCODE_DIV:
		c = p32_div(a, b);
		break;
	case OPCODE_SQRT:
		c = p32_sqrt(a);
		break;
	}
	cout << hex;
	cout << "a = 32.2x" << a << "p" << endl;
	cout << "b = 32.2x" << b << "p" << endl;
	cout << "c = 32.2x" << c << "p" << endl;
	cout << dec;

	posit<32, 2> x, y, z, r;
	x.set_raw_bits(_a);
	y.set_raw_bits(_b);
	r.set_raw_bits(_c);
	switch (opcode) {
	case OPCODE_ADD:
		z = x + y;
		break;
	case OPCODE_SUB:
		z = a - b;
		break;
	case OPCODE_MUL:
		z = x * y;
		break;
	case OPCODE_DIV:
		z = x / y;
		break;
	case OPCODE_SQRT:
		z = sw::unum::sqrt(x);
		break;
	}
	cout << "x = " << posit_format(x) << endl;
	cout << "y = " << posit_format(y) << endl;
	cout << "z = " << posit_format(z) << endl;
	cout << "r = " << posit_format(r) << endl;
}
#endif // SOFTPOSIT_CMP

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	const size_t RND_TEST_CASES = 2*1000*1000;  // 2M

	const size_t nbits = 32;
	const size_t es = 2;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<32,2>";

#if POSIT_FAST_POSIT_32_2
	cout << "Fast specialization posit<32,2> configuration tests" << endl;
#else
	cout << "Standard posit<32,2> configuration tests" << endl;
#endif

#ifdef SOFTPOSIT_CMP
	// FAIL 10011000011101110011010101010000 / 11111011010101010100001001101000 != 01111110010010000101000100110001
	uint32_t a = 0b10011000011101110011010101010000;
	uint32_t b = 0b11111011010101010100001001101000;
	uint32_t c = 0b01111110010010000101000100110001;
	GenerateP32Test(OPCODE_DIV, a, b, c);
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", true, OPCODE_ADD, 10), tag, " add ");
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", true, OPCODE_SUB, 10), tag, " sub ");
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", true, OPCODE_MUL, 10), tag, " mul ");
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", true, OPCODE_DIV, 10), tag, " div ");
	ReportTestResult(ValidateAgainstSoftPosit<nbits, es>("test", true, OPCODE_SQRT, 10), tag, " sqrt ");
	return 1;
#endif

	posit<nbits, es> p;
	cout << dynamic_range(p) << endl << endl;

	// logic tests
	cout << "Logic operator tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicEqual             <nbits, es>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicNotEqual          <nbits, es>(), tag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessThan          <nbits, es>(), tag, "    <          ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterThan       <nbits, es>(), tag, "    >          ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         ");

	// conversion tests
	// internally this generators are clamped as the state space 2^33 is too big
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateIntegerConversion<nbits, es>(tag, bReportIndividualTestCases), tag, "sint32 assign  ");
	nrOfFailedTestCases += ReportTestResult( ValidateUintConversion<nbits, es>(tag, bReportIndividualTestCases), tag, "uint32 assign  ");
	nrOfFailedTestCases += ReportTestResult( ValidateConversion <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign   ");
//	nrOfFailedTestCases += ReportTestResult( ValidateConversionThroughRandoms <nbits, es>(tag, true, 100), tag, "float assign   ");

	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES),  tag, "addition       ");
	nrOfFailedTestCases += ReportTestResult( ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES),  tag, "subtraction    ");
	nrOfFailedTestCases += ReportTestResult( ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES),  tag, "multiplication ");
	nrOfFailedTestCases += ReportTestResult( ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES),  tag, "division       ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SQRT, RND_TEST_CASES), tag, "sqrt           ");

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

