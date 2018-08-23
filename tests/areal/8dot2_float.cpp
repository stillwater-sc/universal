// 8dot2_float.cpp: Functionality tests for 8bit precision floats
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// minimum set of include files to reflect source code dependencies
#include "../../posit/exceptions.hpp"
#include "../../posit/trace_constants.hpp"
#include "../../bitblock/bitblock.hpp"
#include "../../posit/bit_functions.hpp"
#include "../../areal/areal.hpp"
#include "../test_helpers.hpp"
#include "areal_test_helpers.hpp"

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	const size_t nbits = 8;
	const size_t es = 2;

	int nrOfFailedTestCases = 0;
	std::string tag = " areal<8,2>";

	cout << "Standard areal<8,2> configuration tests" << endl;

	areal<nbits, es> r;

#if 0
	bool bReportIndividualTestCases = false;

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
	// arithmetic tests
	nrOfFailedTestCases += ReportTestResult( ValidateAddition         <nbits, es>(tag, bReportIndividualTestCases), tag, "add            ");
	nrOfFailedTestCases += ReportTestResult( ValidateSubtraction      <nbits, es>(tag, bReportIndividualTestCases), tag, "subtract       ");
	nrOfFailedTestCases += ReportTestResult( ValidateMultiplication   <nbits, es>(tag, bReportIndividualTestCases), tag, "multiply       ");
	nrOfFailedTestCases += ReportTestResult( ValidateDivision         <nbits, es>(tag, bReportIndividualTestCases), tag, "divide         ");
	nrOfFailedTestCases += ReportTestResult( ValidateNegation         <nbits, es>(tag, bReportIndividualTestCases), tag, "negate         ");
	nrOfFailedTestCases += ReportTestResult( ValidateReciprocation    <nbits, es>(tag, bReportIndividualTestCases), tag, "reciprocate    ");
#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
