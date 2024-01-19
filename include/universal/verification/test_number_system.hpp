#pragma once
// test_number_system.hpp: reusable test suite for small number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_mathlib.hpp>
//#include <universal/verification/test_suite_random.hpp>

namespace sw { namespace universal {

	template<typename TestType, typename RefType>
	int ExhaustiveNumberSystemTest(const std::string& test_tag, bool reportTestCases) {
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
		nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual             <TestType>(reportTestCases), test_tag, "    ==         ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual          <TestType>(reportTestCases), test_tag, "    !=         ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan          <TestType>(reportTestCases), test_tag, "    <          ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan   <TestType>(reportTestCases), test_tag, "    <=         ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan       <TestType>(reportTestCases), test_tag, "    >          ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<TestType>(reportTestCases), test_tag, "    >=         ");

		// conversion tests
		cout << "Assignment/conversion tests " << endl;
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion <TestType>(reportTestCases), test_tag, "integer assign (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyConversion        <TestType, RefType, float>(reportTestCases), test_tag, "float assign   (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyConversion        <TestType, RefType, double>(reportTestCases), test_tag, "double assign  (native)  ");

		// arithmetic tests
		cout << "Arithmetic tests " << endl;
		nrOfFailedTestCases += ReportTestResult(VerifyNegation              <TestType>(reportTestCases), test_tag, "negate         (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyAddition              <TestType>(reportTestCases), test_tag, "add            (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifySubtraction           <TestType>(reportTestCases), test_tag, "subtract       (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyMultiplication        <TestType>(reportTestCases), test_tag, "multiply       (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyDivision              <TestType>(reportTestCases), test_tag, "divide         (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyInPlaceAddition       <TestType>(reportTestCases), test_tag, "+=             (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyInPlaceSubtraction    <TestType>(reportTestCases), test_tag, "-=             (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyInPlaceMultiplication <TestType>(reportTestCases), test_tag, "*=             (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyInPlaceDivision       <TestType>(reportTestCases), test_tag, "/=             (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyReciprocation         <TestType>(reportTestCases), test_tag, "reciprocate    (native)  ");

		// elementary function tests
		cout << "Elementary function tests " << endl;
		nrOfFailedTestCases += ReportTestResult(VerifySqrt             <TestType>(reportTestCases), test_tag, "sqrt           (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyExp              <TestType>(reportTestCases), test_tag, "exp                      ");
		nrOfFailedTestCases += ReportTestResult(VerifyExp2             <TestType>(reportTestCases), test_tag, "exp2                     ");
		nrOfFailedTestCases += ReportTestResult(VerifyLog              <TestType>(reportTestCases), test_tag, "log                      ");
		nrOfFailedTestCases += ReportTestResult(VerifyLog2             <TestType>(reportTestCases), test_tag, "log2                     ");
		nrOfFailedTestCases += ReportTestResult(VerifyLog10            <TestType>(reportTestCases), test_tag, "log10                    ");
		nrOfFailedTestCases += ReportTestResult(VerifySine             <TestType>(reportTestCases), test_tag, "sin                      ");
		nrOfFailedTestCases += ReportTestResult(VerifyCosine           <TestType>(reportTestCases), test_tag, "cos                      ");
		nrOfFailedTestCases += ReportTestResult(VerifyTangent          <TestType>(reportTestCases), test_tag, "tan                      ");
		nrOfFailedTestCases += ReportTestResult(VerifyAtan             <TestType>(reportTestCases), test_tag, "atan                     ");
		nrOfFailedTestCases += ReportTestResult(VerifyAsin             <TestType>(reportTestCases), test_tag, "asin                     ");
		nrOfFailedTestCases += ReportTestResult(VerifyAcos             <TestType>(reportTestCases), test_tag, "acos                     ");
		nrOfFailedTestCases += ReportTestResult(VerifySinh             <TestType>(reportTestCases), test_tag, "sinh                     ");
		nrOfFailedTestCases += ReportTestResult(VerifyCosh             <TestType>(reportTestCases), test_tag, "cosh                     ");
		nrOfFailedTestCases += ReportTestResult(VerifyTanh             <TestType>(reportTestCases), test_tag, "tanh                     ");
		nrOfFailedTestCases += ReportTestResult(VerifyAtanh            <TestType>(reportTestCases), test_tag, "atanh                    ");
		nrOfFailedTestCases += ReportTestResult(VerifyAcosh            <TestType>(reportTestCases), test_tag, "acosh                    ");
		nrOfFailedTestCases += ReportTestResult(VerifyAsinh            <TestType>(reportTestCases), test_tag, "asinh                    ");

		nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction    <TestType>(reportTestCases), test_tag, "pow                      ");

		return nrOfFailedTestCases;
	}

} } // namespace sw::universal
