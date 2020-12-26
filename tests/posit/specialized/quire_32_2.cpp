// quire_32_2.cpp: dot product and fused dot product unctionality tests for fast specialized posit<32,2>
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable fast specialized posit<32,2>
//#define POSIT_FAST_SPECIALIZATION   // turns on all fast specializations
#define POSIT_FAST_POSIT_32_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
// test helpers, such as, ReportTestResults
#include "../../utils/test_helpers.hpp"
#include "../../utils/posit_test_randoms.hpp"

/// Standard posit with nbits = 32 have es = 2 exponent bits.

template<size_t nbits, size_t es>
int Validate() {
	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	constexpr size_t RND_TEST_CASES = 500000;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " quire<32,2>";

#if POSIT_FAST_POSIT_32_2
	cout << "Fast specialization quire<32,2> configuration tests" << endl;
#else
	cout << "Standard quire<32,2> configuration tests" << endl;
#endif

	posit<nbits, es> p;
	quire<nbits, es> q;
	cout << dynamic_range<nbits,es>() << endl << endl;

	// special cases
	cout << "Special case tests " << endl;
	string test = "Initialize to zero: ";
	q = 0;
	nrOfFailedTestCases += ReportCheck(tag, test, q.iszero());

	// logic tests
//	cout << "Logic operator tests " << endl;
//	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicEqual             <nbits, es>(), tag, "    ==          (native)  ");

	// conversion tests
	cout << "Assignment/conversion tests " << endl;
	minpos<nbits, es>(p);
	q = p;

	// arithmetic tests
	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition        (native)  ");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication  (native)  ");

	// elementary function tests
//	cout << "Elementary function tests " << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
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

