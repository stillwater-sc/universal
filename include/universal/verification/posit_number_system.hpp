#pragma once
// posit_number_system.hpp: reusable test suite for posit number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>
//#include <universal/verification/posit_test_suite_randoms.hpp>

namespace sw {
	namespace universal {

		template<typename TestType> 
		int VerifySpecialCases(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			TestType v{ 0 }, maxpos(SpecificValue::maxpos);
			if (!v.iszero()) {
				if (reportTestCases) std::cerr << "FAIL: test of zero: " << to_binary(v, true) << " : " << v << '\n';
				++nrOfFailedTestCases;
			}
			v = NAN;
			if (!v.isnar()) {
				if (reportTestCases) std::cerr << "FAIL: test of float assign NaN did not yield NaR: " << to_binary(v, true) << " : " << v << '\n';
				++nrOfFailedTestCases;
			}
			v = INFINITY;
			if (!v.isnar()) {
				if (reportTestCases) std::cerr << "FAIL: test of float assign INF did not yield NaR: " << to_binary(v, true) << " : " << v << '\n';
				++nrOfFailedTestCases;
			}
			v = double(NAN);
			if (!v.isnar()) {
				if (reportTestCases) std::cerr << "FAIL: test of double assign NaN did not yield NaR: " << to_binary(v, true) << " : " << v << '\n';
				++nrOfFailedTestCases;
			}
			v = -double(INFINITY);
			if (!v.isnar()) {
				if (reportTestCases) std::cerr << "FAIL: test of double assign INF did not yield NaR: " << to_binary(v, true) << " : " << v << '\n';
				++nrOfFailedTestCases;
			}
			return nrOfFailedTestCases;
	}

	template<typename TestType, typename EnvelopeType>
	int ExhaustiveNumberSystemTest(const std::string& test_tag, bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// special cases
		nrOfFailedTestCases += ReportTestResult(VerifySpecialCases           <TestType>(reportTestCases), test_tag, "special cases");

		// conversion tests
		std::cerr << "Assignment/conversion tests " << '\n';
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<TestType>(reportTestCases), test_tag, "integer conversion  (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyConversion<TestType, EnvelopeType, float>(reportTestCases), test_tag , "float conversion    (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyConversion<TestType, EnvelopeType, double>(reportTestCases), test_tag, "double conversion   (native)  ");

		// logic tests
		std::cerr << "Logic function tests " << '\n';
		nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual             <TestType>(reportTestCases), test_tag, "==                            ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual          <TestType>(reportTestCases), test_tag, "!=                            ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan          <TestType>(reportTestCases), test_tag, "<                             ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan   <TestType>(reportTestCases), test_tag, "<=                            ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan       <TestType>(reportTestCases), test_tag, ">                             ");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<TestType>(reportTestCases), test_tag, ">=                            ");

		// arithmetic tests
		std::cerr << "Arithmetic tests " << '\n';
		nrOfFailedTestCases += ReportTestResult(VerifyNegation              <TestType>(reportTestCases), test_tag, "negate              (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyAddition              <TestType>(reportTestCases), test_tag, "add                 (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifySubtraction           <TestType>(reportTestCases), test_tag, "subtract            (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyMultiplication        <TestType>(reportTestCases), test_tag, "multiply            (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyDivision              <TestType>(reportTestCases), test_tag, "divide              (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyInPlaceAddition       <TestType>(reportTestCases), test_tag, "+=                  (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyInPlaceSubtraction    <TestType>(reportTestCases), test_tag, "-=                  (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyInPlaceMultiplication <TestType>(reportTestCases), test_tag, "*=                  (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyInPlaceDivision       <TestType>(reportTestCases), test_tag, "/=                  (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyReciprocation         <TestType>(reportTestCases), test_tag, "reciprocate         (native)  ");

		// elementary function tests
		std::cerr << "Elementary function tests " << '\n';
		nrOfFailedTestCases += ReportTestResult(VerifySqrt                  <TestType>(reportTestCases), test_tag, "sqrt                (native)  ");
		nrOfFailedTestCases += ReportTestResult(VerifyExp                   <TestType>(reportTestCases), test_tag, "exp                           ");
		nrOfFailedTestCases += ReportTestResult(VerifyExp2                  <TestType>(reportTestCases), test_tag, "exp2                          ");
		nrOfFailedTestCases += ReportTestResult(VerifyLog                   <TestType>(reportTestCases), test_tag, "log                           ");
		nrOfFailedTestCases += ReportTestResult(VerifyLog2                  <TestType>(reportTestCases), test_tag, "log2                          ");
		nrOfFailedTestCases += ReportTestResult(VerifyLog10                 <TestType>(reportTestCases), test_tag, "log10                         ");
		nrOfFailedTestCases += ReportTestResult(VerifySine                  <TestType>(reportTestCases), test_tag, "sin                           ");
		nrOfFailedTestCases += ReportTestResult(VerifyCosine                <TestType>(reportTestCases), test_tag, "cos                           ");
		nrOfFailedTestCases += ReportTestResult(VerifyTangent               <TestType>(reportTestCases), test_tag, "tan                           ");
		nrOfFailedTestCases += ReportTestResult(VerifyAtan                  <TestType>(reportTestCases), test_tag, "atan                          ");
		nrOfFailedTestCases += ReportTestResult(VerifyAsin                  <TestType>(reportTestCases), test_tag, "asin                          ");
		nrOfFailedTestCases += ReportTestResult(VerifyAcos                  <TestType>(reportTestCases), test_tag, "acos                          ");
		nrOfFailedTestCases += ReportTestResult(VerifySinh                  <TestType>(reportTestCases), test_tag, "sinh                          ");
		nrOfFailedTestCases += ReportTestResult(VerifyCosh                  <TestType>(reportTestCases), test_tag, "cosh                          ");
		nrOfFailedTestCases += ReportTestResult(VerifyTanh                  <TestType>(reportTestCases), test_tag, "tanh                          ");
		nrOfFailedTestCases += ReportTestResult(VerifyAtanh                 <TestType>(reportTestCases), test_tag, "atanh                         ");
		nrOfFailedTestCases += ReportTestResult(VerifyAcosh                 <TestType>(reportTestCases), test_tag, "acosh                         ");
		nrOfFailedTestCases += ReportTestResult(VerifyAsinh                 <TestType>(reportTestCases), test_tag, "asinh                         ");

		nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction         <TestType>(reportTestCases), test_tag, "pow                           ");

		return nrOfFailedTestCases;
	}

} } // namespace sw::universal
