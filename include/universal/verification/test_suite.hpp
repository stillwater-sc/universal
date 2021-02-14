// test_suite.hpp: reusable test suite for small number systems
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)
#pragma warning(disable : 4710)
#endif
#include <iostream>
#include <iomanip>
#include <string>

#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/test_suite_logic.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

template<typename TestType>
int ExhaustiveNumberSystemTest(const std::string& tag, bool bReportIndividualTestCases) {
	using namespace std;
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	TestType v;

	// special cases
	v = 0;
	if (!v.iszero()) ++nrOfFailedTestCases;
	v = NAN;
	if (!v.isnan()) ++nrOfFailedTestCases;
	v = INFINITY;
	if (!v.isinf()) ++nrOfFailedTestCases;

	// logic tests
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual             <TestType>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual          <TestType>(), tag, "    !=         ");
//	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan          <TestType>(), tag, "    <          ");
//	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan   <TestType>(), tag, "    <=         ");
//	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan       <TestType>(), tag, "    >          ");
//	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<TestType>(), tag, "    >=         ");

	// conversion tests
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion <TestType>(tag, bReportIndividualTestCases), tag, "integer assign (native)  ");
//	nrOfFailedTestCases += ReportTestResult(VerifyConversion        <TestType,float>(tag, bReportIndividualTestCases), tag, "float assign   (native)  ");
//	nrOfFailedTestCases += ReportTestResult(VerifyConversion        <TestType,double>(tag, bReportIndividualTestCases), tag, "double assign  (native)  ");

#if 0
	// arithmetic tests
	cout << "Arithmetic tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition              <TestType>(tag, bReportIndividualTestCases), tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyInPlaceAddition       <TestType>(tag, bReportIndividualTestCases), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction           <TestType>(tag, bReportIndividualTestCases), tag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyInPlaceSubtraction    <TestType>(tag, bReportIndividualTestCases), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication        <TestType>(tag, bReportIndividualTestCases), tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyInPlaceMultiplication <TestType>(tag, bReportIndividualTestCases), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision              <TestType>(tag, bReportIndividualTestCases), tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyInPlaceDivision       <TestType>(tag, bReportIndividualTestCases), tag, "/=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation              <TestType>(tag, bReportIndividualTestCases), tag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation         <TestType>(tag, bReportIndividualTestCases), tag, "reciprocate    (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifySqrt             <TestType>(tag, bReportIndividualTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyExp              <TestType>(tag, bReportIndividualTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2             <TestType>(tag, bReportIndividualTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyLog              <TestType>(tag, bReportIndividualTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2             <TestType>(tag, bReportIndividualTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10            <TestType>(tag, bReportIndividualTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult(VerifySine             <TestType>(tag, bReportIndividualTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyCosine           <TestType>(tag, bReportIndividualTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyTangent          <TestType>(tag, bReportIndividualTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyAtan             <TestType>(tag, bReportIndividualTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyAsin             <TestType>(tag, bReportIndividualTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyAcos             <TestType>(tag, bReportIndividualTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult(VerifySinh             <TestType>(tag, bReportIndividualTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh             <TestType>(tag, bReportIndividualTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh             <TestType>(tag, bReportIndividualTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh            <TestType>(tag, bReportIndividualTestCases), tag, "atanh                    ");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh            <TestType>(tag, bReportIndividualTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh            <TestType>(tag, bReportIndividualTestCases), tag, "asinh                    ");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction    <TestType>(tag, bReportIndividualTestCases), tag, "pow                      ");
#endif
	return nrOfFailedTestCases;
}
