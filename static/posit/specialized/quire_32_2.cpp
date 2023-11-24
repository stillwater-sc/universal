// quire_32_2.cpp: test suite runner for dot product and fused dot product functionality tests for fast specialized posit<32,2>
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable fast specialized posit<32,2>
//#define POSIT_FAST_SPECIALIZATION   // turns on all fast specializations
#define POSIT_FAST_POSIT_32_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>

/// Standard posit with nbits = 32 have es = 2 exponent bits.

template<size_t nbits, size_t es>
int Verify() {
	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	constexpr size_t RND_TEST_CASES = 500000;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " quire<32,2>";

#if POSIT_FAST_POSIT_32_2
	std::cout << "Fast specialization quire<32,2> configuration tests\n";
#else
	std::cout << "Standard quire<32,2> configuration tests\n";
#endif

	quire<nbits, es> q;
	std::cout << dynamic_range<nbits,es>() << "\n\n";

	// special cases
	std::cout << "Special case tests\n";
	std::string test = "Initialize to zero: ";
	q = 0;
	nrOfFailedTestCases += ReportCheck(tag, test, q.iszero());

	// logic tests
//	cout << "Logic operator tests " << endl;
//	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicEqual             <nbits, es>(), tag, "    ==          (native)  ");

	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	posit<nbits, es> p(SpecificValue::minpos);
	q = p;

	// arithmetic tests
	std::cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each\n";
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition        (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication  (native)  ");

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

