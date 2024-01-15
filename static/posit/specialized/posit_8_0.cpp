// posit_8_0.cpp: test suite runner for fast specialized posit<8,0>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable fast specialized posit<8,0>
#define POSIT_FAST_POSIT_8_0 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
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

	using Scalar = posit<nbits, es>;
	Scalar p;
	std::cout << dynamic_range(p) << "\n\n";
	std::string tag = type_tag(p);

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
	nrOfFailedTestCases += ReportCheck(tag, test, p.iszero());
	test = "Initialize to NAN";
	p = NAN;
	nrOfFailedTestCases += ReportCheck(tag, test, p.isnar());
	test = "Initialize to INFINITY";
	p = INFINITY;
	nrOfFailedTestCases += ReportCheck(tag, test, p.isnar());
	test = "sign is true";
	p = -1.0f;
	nrOfFailedTestCases += ReportCheck(tag, test, p.sign());
	test = "is negative";
	nrOfFailedTestCases += ReportCheck(tag, test, p.isneg());
	test = "sign is false";
	p = +1.0f;
	nrOfFailedTestCases += ReportCheck(tag, test, !p.sign());
	test = "is positive";
	nrOfFailedTestCases += ReportCheck(tag, test, p.ispos());

	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<nbits,es>>(reportTestCases), tag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion       <posit<nbits,es>, float>(reportTestCases), tag, "float assign   (native)  ");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition           <posit<nbits,es>>(reportTestCases), tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication     <posit<nbits,es>>(reportTestCases), tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision           <posit<nbits,es>>(reportTestCases), tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation      <posit<nbits,es>>(reportTestCases), tag, "reciprocate    (native)  ");

#endif

#if REGRESSION_LEVEL_2
	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyLogicEqual             <posit<nbits,es>>(reportTestCases), tag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicNotEqual          <posit<nbits,es>>(reportTestCases), tag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicLessThan          <posit<nbits,es>>(reportTestCases), tag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicLessOrEqualThan   <posit<nbits,es>>(reportTestCases), tag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicGreaterThan       <posit<nbits,es>>(reportTestCases), tag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicGreaterOrEqualThan<posit<nbits,es>>(reportTestCases), tag, "    >=         (native)  ");
#endif

#if REGRESSION_LEVEL_3
	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyAddition           <posit<nbits,es>>(reportTestCases), tag,    "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceAddition    <posit<nbits,es>>(reportTestCases), tag,    "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifySubtraction        <posit<nbits,es>>(reportTestCases), tag,    "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceSubtraction <posit<nbits,es>>(reportTestCases), tag,    "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyMultiplication     <posit<nbits,es>>(reportTestCases), tag,    "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceMultiplication <posit<nbits,es>>(reportTestCases), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyDivision           <posit<nbits,es>>(reportTestCases), tag,    "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceDivision    <posit<nbits,es>>(reportTestCases), tag,    "/=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyNegation           <posit<nbits,es>>(reportTestCases), tag,    "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyReciprocation      <posit<nbits,es>>(reportTestCases), tag,    "reciprocate    (native)  ");
#endif

#if REGRESSION_LEVEL_4
	// elementary function tests
	std::cout << "Elementary function tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifySqrt             <posit<nbits,es>>(reportTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp              <posit<nbits,es>>(reportTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2             <posit<nbits,es>>(reportTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog              <posit<nbits,es>>(reportTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2             <posit<nbits,es>>(reportTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10            <posit<nbits,es>>(reportTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine             <posit<nbits,es>>(reportTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine           <posit<nbits,es>>(reportTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent          <posit<nbits,es>>(reportTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan             <posit<nbits,es>>(reportTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin             <posit<nbits,es>>(reportTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos             <posit<nbits,es>>(reportTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh             <posit<nbits,es>>(reportTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh             <posit<nbits,es>>(reportTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh             <posit<nbits,es>>(reportTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh            <posit<nbits,es>>(reportTestCases), tag, "atanh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh            <posit<nbits,es>>(reportTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh            <posit<nbits,es>>(reportTestCases), tag, "asinh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction    <posit<nbits,es>>(reportTestCases), tag, "pow                      ");
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
