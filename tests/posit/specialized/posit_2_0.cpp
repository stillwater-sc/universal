// posit_2_0.cpp: Functionality tests for specialized 2-bit posits based on look-up tables
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable fast specialized posit<2,0>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_2_0 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
// test helpers, such as, ReportTestResults
#include "../../utils/test_helpers.hpp"
#include "../../utils/posit_test_helpers.hpp"

/*
 posit of size nbits = 2 without exponent bits, i.e. es = 0.
*/

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	// no randoms, 2-bit posits can be done exhaustively

	constexpr size_t nbits = 2;
	constexpr size_t es = 0;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = true;
	std::string tag = " posit<2,0>";

#if defined(POSIT_FAST_POSIT_2_0)
	cout << "Fast specialization posit<2,0> configuration tests" << endl;
#else
	cout << "Reference posit<2,0> configuration tests" << endl;
#endif

	posit<nbits,es> p;
	cout << dynamic_range(p) << endl;

	// special cases
	p = 0;
	if (!p.iszero()) ++nrOfFailedTestCases;
	p = NAN;
	if (!p.isnar()) ++nrOfFailedTestCases;
	p = INFINITY;
	if (!p.isnar()) ++nrOfFailedTestCases;

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
