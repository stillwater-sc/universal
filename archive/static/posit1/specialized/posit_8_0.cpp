// posit_8_0.cpp: test suite runner for fast specialized posit<8,0>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable fast specialized posit<8,0>
#define POSIT_FAST_POSIT_8_0 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

// Standard posits with nbits = 8 have no exponent bits, i.e. es = 0.
// 
// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	// no randoms, 8-bit posits can be done exhaustively

	constexpr size_t nbits = 8;
	constexpr size_t es = 0;

	int nrOfFailedTestCases = 0;
	bool reportTestCases = false;

#if POSIT_FAST_POSIT_8_0
	std::cout << "Fast specialization posit<8,0> configuration tests\n";
#else
	std::cout << "Standard posit<8,0> configuration tests\n";
#endif

	using TestType = posit<nbits, es>;
	using EnvelopeType = posit<nbits + 1, es>;
	TestType p;
	std::string typeTag = type_tag(p);
	std::cout << dynamic_range(p) << "\n\n";

#if MANUAL_TESTING

	posit<nbits, es> a, b;
	a.setnar(); b.setnar();
	testLogicOperators(a, b);
	a = +1; b = +1; --b;
	testLogicOperators(a, b);
	a = +1; b = +1; ++b;
	testLogicOperators(a, b);
	a = -1; b = -1; --b;
	testLogicOperators(a, b);
	a = -1; b = -1; ++b;
	testLogicOperators(a, b);

	cout << nrOfFailedTestCases << " number of failures\n";

	nrOfFailedTestCases = 0;  // ignore failures in manual testing

#else

#if REGRESSION_LEVEL_1
	// special cases
	std::cout << "Special case tests\n";
	std::string test = "Initialize to zero: ";
	p = 0;
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.iszero());
	test = "Initialize to NAN";
	p = NAN;
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.isnar());
	test = "Initialize to INFINITY";
	p = INFINITY;
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.isnar());
	test = "sign is true";
	p = -1.0f;
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.sign());
	test = "is negative";
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.isneg());
	test = "sign is false";
	p = +1.0f;
	nrOfFailedTestCases += ReportCheck(typeTag, test, !p.sign());
	test = "is positive";
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.ispos());

	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<TestType>(reportTestCases), typeTag, "integer conversion  (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion       <TestType, EnvelopeType, float>(reportTestCases), typeTag, "float conversion    (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion       <TestType, EnvelopeType, double>(reportTestCases), typeTag, "double conversion   (native)  ");


	nrOfFailedTestCases += ReportTestResult(VerifyAddition           <TestType>(reportTestCases), typeTag, "add                 (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication     <TestType>(reportTestCases), typeTag, "multiply            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision           <TestType>(reportTestCases), typeTag, "divide              (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation      <TestType>(reportTestCases), typeTag, "reciprocate         (native)  ");

#endif

#if REGRESSION_LEVEL_2
	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyLogicEqual             <TestType>(reportTestCases), typeTag, "    ==              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicNotEqual          <TestType>(reportTestCases), typeTag, "    !=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicLessThan          <TestType>(reportTestCases), typeTag, "    <               (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicLessOrEqualThan   <TestType>(reportTestCases), typeTag, "    <=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicGreaterThan       <TestType>(reportTestCases), typeTag, "    >               (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicGreaterOrEqualThan<TestType>(reportTestCases), typeTag, "    >=              (native)  ");
#endif

#if REGRESSION_LEVEL_3
	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyAddition           <TestType>(reportTestCases), typeTag,    "add                 (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceAddition    <TestType>(reportTestCases), typeTag,    "+=                  (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifySubtraction        <TestType>(reportTestCases), typeTag,    "subtract            (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceSubtraction <TestType>(reportTestCases), typeTag,    "-=                  (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyMultiplication     <TestType>(reportTestCases), typeTag,    "multiply            (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceMultiplication <TestType>(reportTestCases), typeTag, "*=                  (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyDivision           <TestType>(reportTestCases), typeTag,    "divide              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceDivision    <TestType>(reportTestCases), typeTag,    "/=                  (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyNegation           <TestType>(reportTestCases), typeTag,    "negate              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyReciprocation      <TestType>(reportTestCases), typeTag,    "reciprocate         (native)  ");
#endif

#if REGRESSION_LEVEL_4
	// elementary function tests
	std::cout << "Elementary function tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifySqrt             <TestType>(reportTestCases), typeTag, "sqrt                (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp              <TestType>(reportTestCases), typeTag, "exp                           ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2             <TestType>(reportTestCases), typeTag, "exp2                          ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog              <TestType>(reportTestCases), typeTag, "log                           ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2             <TestType>(reportTestCases), typeTag, "log2                          ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10            <TestType>(reportTestCases), typeTag, "log10                         ");
	nrOfFailedTestCases += ReportTestResult( VerifySine             <TestType>(reportTestCases), typeTag, "sin                           ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine           <TestType>(reportTestCases), typeTag, "cos                           ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent          <TestType>(reportTestCases), typeTag, "tan                           ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan             <TestType>(reportTestCases), typeTag, "atan                          ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin             <TestType>(reportTestCases), typeTag, "asin                          ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos             <TestType>(reportTestCases), typeTag, "acos                          ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh             <TestType>(reportTestCases), typeTag, "sinh                          ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh             <TestType>(reportTestCases), typeTag, "cosh                          ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh             <TestType>(reportTestCases), typeTag, "tanh                          ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh            <TestType>(reportTestCases), typeTag, "atanh                         ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh            <TestType>(reportTestCases), typeTag, "acosh                         ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh            <TestType>(reportTestCases), typeTag, "asinh                         ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction    <TestType>(reportTestCases), typeTag, "pow                           ");
#endif

#endif

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
