// 16bit_posit.cpp: Functionality tests for standard 16-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// Configure the posit template environment
// first: enable fast specialized posit<16,1>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_16_1 0
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>
#include "../../test_helpers.hpp"
#include "../../posit_test_helpers.hpp"

/*
Standard posit with nbits = 16 have es = 1 exponent bit.
*/

#define SOFTPOSIT_CMP_
#ifdef SOFTPOSIT_CMP
#include "./softposit_cmp.hpp"
void GenerateP16Test(int opcode, uint16_t _a, uint16_t _b, uint16_t _c) {
	using namespace std;
	using namespace sw::unum;

	posit16_t a, b, c;
	a = _a;
	b = _b;
	switch (opcode) {
	case OPCODE_ADD:
		c = p16_add(a, b);
		break;
	case OPCODE_SUB:
		c = p16_sub(a, b);
		break;
	case OPCODE_MUL:
		c = p16_mul(a, b);
		break;
	case OPCODE_DIV:
		c = p16_div(a, b);
		break;
	case OPCODE_SQRT:
		c = p16_sqrt(a);
		break;
	}
	cout << hex;
	cout << "a = 32.2x" << a << "p" << endl;
	cout << "b = 32.2x" << b << "p" << endl;
	cout << "c = 32.2x" << c << "p" << endl;
	cout << dec;

	posit<16, 1> x, y, z, r;
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

	const size_t RND_TEST_CASES = 500000;

	const size_t nbits = 16;
	const size_t es = 1;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<16,1>";

#if POSIT_FAST_POSIT_16_1
	cout << "Fast specialization posit<16,1> configuration tests" << endl;
#else
	cout << "Standard posit<16,1> configuration tests" << endl;
#endif

#ifdef SOFTPOSIT_CMP
	// FAIL 10011000011101110011010101010000 / 11111011010101010100001001101000 != 01111110010010000101000100110001
	uint16_t a = 0b10011000011101110011010101010000;
	uint16_t b = 0b11111011010101010100001001101000;
	uint16_t c = 0b01111110010010000101000100110001;
	GenerateP16Test(OPCODE_DIV, a, b, c);
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
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual             <nbits, es>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual          <nbits, es>(), tag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan          <nbits, es>(), tag, "    <          ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan       <nbits, es>(), tag, "    >          ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         ");
	// conversion tests
	nrOfFailedTestCases += ReportTestResult( ValidateIntegerConversion<nbits, es>(tag, bReportIndividualTestCases), tag, "integer assign ");
	nrOfFailedTestCases += ReportTestResult( ValidateConversion       <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign   ");

	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition       ");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction    ");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication ");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division       ");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SQRT, RND_TEST_CASES), tag, "sqrt           ");

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
