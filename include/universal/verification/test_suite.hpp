#pragma once
// test_suite.hpp: reusable test suite for small number systems
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>

#include <universal/native/manipulators.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/test_suite_exceptions.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/test_suite_logic.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

template<typename TestType>
void ReportTrivialityOfType() {
	std::string testType = sw::universal::type_tag(TestType());

	bool isTrivial = bool(std::is_trivial<TestType>());
	static_assert(std::is_trivial<TestType>(), " should be trivial but failed the assertion");
	std::cout << (isTrivial ? testType + std::string("  is trivial") : testType + std::string("  failed trivial: FAIL")) << '\n';

	bool isTriviallyConstructible = bool(std::is_trivially_constructible<TestType>());
	static_assert(std::is_trivially_constructible<TestType>(), " should be trivially constructible but failed the assertion");
	std::cout << (isTriviallyConstructible ? testType + std::string("  is trivial constructible") : testType + std::string("  failed trivial constructible: FAIL")) << '\n';

	bool isTriviallyCopyable = bool(std::is_trivially_copyable<TestType>());
	static_assert(std::is_trivially_copyable<TestType>(), " should be trivially copyable but failed the assertion");
	std::cout << (isTriviallyCopyable ? testType + std::string("  is trivially copyable") : testType + std::string("  failed trivially copyable: FAIL")) << '\n';

	bool isTriviallyCopyAssignable = bool(std::is_trivially_copy_assignable<TestType>());
	static_assert(std::is_trivially_copy_assignable<TestType>(), " should be trivially copy-assignable but failed the assertion");
	std::cout << (isTriviallyCopyAssignable ? testType + std::string("  is trivially copy-assignable") : testType + std::string("  failed trivially copy-assignable: FAIL")) << '\n';
}

template<typename Real>
void ArithmeticOperators(Real a, Real b) {
	using namespace sw::universal;
	Real c;

	c = a + b;
	ReportBinaryOperation(a, "+", b, c);
	c = a - b;
	ReportBinaryOperation(a, "-", b, c);
	c = a * b;
	ReportBinaryOperation(a, "*", b, c);
	c = a / b;
	ReportBinaryOperation(a, "/", b, c);

	// negation
	ReportUnaryOperation(" -()", c, -c);

	// ULP manipulations through increment and decrement operators
	// This is Universal specific behavior of Real types.
	// In Universal, increment and decrement will operate on the encoding bits
	// and manipulate the unit in last position.

	// prefix operators
	a = 1;
	b = 1; --b;
	ReportUnaryOperation("--()", a, b);
	b = 1; ++b;
	ReportUnaryOperation("++()", a, b);

	// postfix operators
	a = 1;
	b = 1; b--;
	ReportUnaryOperation("()--", a, b);
	b = 1; ++b;
	ReportUnaryOperation("()++", a, b);
}

#ifdef DEPRECATED
// test_suite_random depends on a number systems math library
// so cannot be included here as this include is used also
// for number systems that do not have a math library.
//#include <universal/verification/test_suite_random.hpp>

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
#endif
