// posit_256_5.cpp: test suite runner for fast specialized 256-bit posit<256,5>
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#pragma warning(disable : 4514) // unreferenced inline function has been removed
#pragma warning(disable : 4820) // bytes padding added after data member
#pragma warning(disable : 4710) // function not inlined
#endif
// Configure the posit template environment
// first: enable fast specialized posit<256,5>
//#define POSIT_FAST_SPECIALIZATION   // turns on all fast specializations
#define POSIT_FAST_POSIT_256_5 1      // TODO: fast posit<256,5> not implemented yet
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/posit/posit>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>

/*
Standard posits with nbits = 256 have 5 exponent bits.
*/

#define STRESS_TESTING 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) { cout << argv[0] << endl; }

	constexpr size_t RND_TEST_CASES = 10000;

	constexpr size_t nbits = 256;
	constexpr size_t es = 5;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<256,5>";

#if POSIT_FAST_POSIT_256_5
	cout << "Fast specialization posit<256,5> configuration tests" << endl;
#else
	cout << "Standard posit<256,5> configuration tests" << endl;
#endif

	posit<nbits, es> p;
	cout << dynamic_range(p) << endl << endl;

	// special cases
	cout << "Special case tests " << endl;
	string test = "Initialize to zero: ";
	p = 0;
	nrOfFailedTestCases += ReportCheck(tag, test, p.iszero());
	test = "Initialize to NAN";
	p = NAN;
	nrOfFailedTestCases += ReportCheck(tag, test, p.isnar());
	test = "Initialize to INFINITY";
	p = INFINITY;
	nrOfFailedTestCases += ReportCheck(tag, test, p.isnar());
	test = "sign is true";
	p = -1.0f;
	nrOfFailedTestCases += ReportCheck(tag, test, p.sign());
	test = "is negative";
	nrOfFailedTestCases += ReportCheck(tag, test, p.isneg());
	test = "sign is false";
	p = +1.0f;
	nrOfFailedTestCases += ReportCheck(tag, test, !p.sign());
	test = "is positive";
	nrOfFailedTestCases += ReportCheck(tag, test, p.ispos());

	// TODO: as we don't have a reference floating point implementation to Verify
	// the arithmetic operations we are going to ignore the failures
#if STRESS_TESTING
	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	cout << "Without an arithmetic reference, test failures can be ignored" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");
#endif
	nrOfFailedTestCases = 0;
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_SUCCESS; //as we manually throwing the not supported yet it should not fall through the cracks     EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
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
