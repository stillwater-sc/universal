// 16bit_posit.cpp: Functionality tests for standard 16-bit posits
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// Configure the posit template environment
// first: enable fast specialized posit<16,1>
//#define POSIT_FAST_SPECIALIZATION
//#define POSIT_FAST_POSIT_16_1 0
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>
#include "../test_helpers.hpp"
#include "../posit_test_helpers.hpp"

#include "softposit_cmp.hpp"
/*
Standard posit with nbits = 16 have es = 1 exponent bit.
*/

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
#if later	
	{
		posit<nbits, es> p, mp;
		mp.set_raw_bits(0x01);
		cout << "posit         value          multiple of minpos\n";
		for (int i = 0; i < 10; ++i) {
			p.set_raw_bits(i);
			cout << setw(10) << posit_format(p) << " = " << setw(10) << p << "   " << setw(5) << p / mp << endl;
		}
	}

	posit16_t a, b, c;
	a = 0x0000'009B;
	b = 0x0000'0043;
	c = p16_add(a, b);
	cout << hex;
	cout << "a = 32.2x" << a << endl;
	cout << "b = 32.2x" << b << endl;
	cout << "c = 32.2x" << c << endl;
	cout << dec;

	posit<nbits, es> x, y, z;
	x.set_raw_bits(0x009B);
	y.set_raw_bits(0x0043);
	z = x + y;
	cout << "x = " << posit_format(x) << endl;
	cout << "y = " << posit_format(y) << endl;
	cout << "z = " << posit_format(z) << endl;
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
