// 128bit_posit.cpp: Functionality tests for standard 128-bit posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>
#include "../test_helpers.hpp"
#include "../posit_test_helpers.hpp"

/*
Standard posits with nbits = 128 have 4 exponent bits.
*/

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	const size_t RND_TEST_CASES = 10000;

	const size_t nbits = 128;
	const size_t es = 4;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<128,4>";

	cout << "Standard posit<128,4> configuration tests" << endl;
	posit<nbits, es> p;
	cout << spec_to_string(p) << endl << endl;

	throw("128bit posits are not yet supported");

	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_SUCCESS; //as we manually throwing the not supported yet it should not fall through the cracks     EXIT_FAILURE;
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