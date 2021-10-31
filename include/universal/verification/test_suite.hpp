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

#include <universal/verification/test_status.hpp>
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
	if (!v.iszero()) {
		cout << "FAIL: test of zero: " << to_binary(v, true) << " : " << v << '\n';
		++nrOfFailedTestCases;
	}
	v = NAN;
	if (!v.isnan()) {
		cout << "FAIL: test of float assign to NaN: " << to_binary(v, true) << " : " << v << '\n';
		++nrOfFailedTestCases;
	}
	v = INFINITY;
	if (!v.isinf()) {
		cout << "FAIL: test of float assign to INF: " << to_binary(v, true) << " : " << v << '\n';
		++nrOfFailedTestCases;
	}
	v = double(NAN);
	if (!v.isnan()) {
		cout << "FAIL: test of double assign to NaN: " << to_binary(v, true) << " : " << v << '\n';
		++nrOfFailedTestCases;
	}
	v = double(INFINITY);
	if (!v.isinf()) {
		cout << "FAIL: test of double assign to INF: " << to_binary(v, true) << " : " << v << '\n';
		++nrOfFailedTestCases;
	}

	// logic tests
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual             <TestType>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual          <TestType>(), tag, "    !=         ");
//	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan          <TestType>(), tag, "    <          ");
//	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan   <TestType>(), tag, "    <=         ");
//	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan       <TestType>(), tag, "    >          ");
//	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<TestType>(), tag, "    >=         ");

	// conversion tests
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion <TestType>(bReportIndividualTestCases), tag, "integer assign (native)  ");
//	nrOfFailedTestCases += ReportTestResult(VerifyConversion        <TestType,float>(bReportIndividualTestCases), tag, "float assign   (native)  ");
//	nrOfFailedTestCases += ReportTestResult(VerifyConversion        <TestType,double>(bReportIndividualTestCases), tag, "double assign  (native)  ");

#if 0
	// arithmetic tests
	cout << "Arithmetic tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition              <TestType>(bReportIndividualTestCases), tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyInPlaceAddition       <TestType>(bReportIndividualTestCases), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction           <TestType>(bReportIndividualTestCases), tag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyInPlaceSubtraction    <TestType>(bReportIndividualTestCases), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication        <TestType>(bReportIndividualTestCases), tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyInPlaceMultiplication <TestType>(bReportIndividualTestCases), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision              <TestType>(bReportIndividualTestCases), tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyInPlaceDivision       <TestType>(bReportIndividualTestCases), tag, "/=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation              <TestType>(bReportIndividualTestCases), tag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation         <TestType>(bReportIndividualTestCases), tag, "reciprocate    (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifySqrt             <TestType>(bReportIndividualTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyExp              <TestType>(bReportIndividualTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2             <TestType>(bReportIndividualTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyLog              <TestType>(bReportIndividualTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2             <TestType>(bReportIndividualTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10            <TestType>(bReportIndividualTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult(VerifySine             <TestType>(bReportIndividualTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyCosine           <TestType>(bReportIndividualTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyTangent          <TestType>(bReportIndividualTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult(VerifyAtan             <TestType>(bReportIndividualTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyAsin             <TestType>(bReportIndividualTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyAcos             <TestType>(bReportIndividualTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult(VerifySinh             <TestType>(bReportIndividualTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh             <TestType>(bReportIndividualTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh             <TestType>(bReportIndividualTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh            <TestType>(bReportIndividualTestCases), tag, "atanh                    ");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh            <TestType>(bReportIndividualTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh            <TestType>(bReportIndividualTestCases), tag, "asinh                    ");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction    <TestType>(bReportIndividualTestCases), tag, "pow                      ");
#endif
	return nrOfFailedTestCases;
}
