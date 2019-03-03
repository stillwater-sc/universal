// 3bit_posit.cpp: Functionality tests for specialized 3-bit posits based on look-up tables
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// enable fast specialized posit<3,0>
#define POSIT_FAST_SPECIALIZATION
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#include <posit>
#include "../test_helpers.hpp"
#include "../posit_test_helpers.hpp"

/*
 posit of size nbits = 3 without exponent bits, i.e. es = 0.
*/

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// const size_t RND_TEST_CASES = 0;  // no randoms, 3-bit posits can be done exhaustively

	const size_t nbits = 3;
	const size_t es = 0;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<3,0>";

#if defined(POSIT_FAST_POSIT_3_0)
	cout << "Fast specialization posit<3,0> configuration tests" << endl;
#else
	cout << "Reference posit<3,0> configuration tests" << endl;
#endif

	posit<nbits,es> p;
	cout << dynamic_range(p) << endl;

	// logic tests
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual             <nbits, es>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual          <nbits, es>(), tag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan          <nbits, es>(), tag, "    <          ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan       <nbits, es>(), tag, "    >          ");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         ");
	// conversion tests
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<nbits, es>(tag, bReportIndividualTestCases), tag, "integer assign ");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion       <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign   ");
	// arithmetic tests
	nrOfFailedTestCases += ReportTestResult(ValidateAddition         <nbits, es>(tag, bReportIndividualTestCases), tag, "add            ");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction      <nbits, es>(tag, bReportIndividualTestCases), tag, "subtract       ");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication   <nbits, es>(tag, bReportIndividualTestCases), tag, "multiply       ");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision         <nbits, es>(tag, bReportIndividualTestCases), tag, "divide         ");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation         <nbits, es>(tag, bReportIndividualTestCases), tag, "negate         ");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation    <nbits, es>(tag, bReportIndividualTestCases), tag, "reciprocate    ");

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
