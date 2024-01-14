// posit_8_2.cpp: test suite runner for fast specialized posit<8,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
//
// Configure the posit template environment
// first: enable fast specialized posit<8,2>
#define POSIT_FAST_POSIT_8_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

/*
specialized small standard 8-bit posit with es = 2 
*/

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	constexpr size_t nbits = NBITS_IS_8;
	constexpr size_t es    = ES_IS_2;

#if POSIT_FAST_POSIT_8_2
	std::string test_suite = "Fast specialization posit<8,2>";
#else
	std::string test_suite = "Standard posit<8,2>";
#endif

	std::string test_tag    = "arithmetic type tests";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Scalar = posit<nbits, es>;
	Scalar p;
	std::cout << dynamic_range(p) << "\n\n";
	std::string tag = type_tag(p);

#if MANUAL_TESTING

	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<nbits, es>(true), test_tag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion       <nbits, es>(reportTestCases), test_tag, "float assign   (native)  ");

	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicEqual             <nbits, es>(), test_tag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicNotEqual          <nbits, es>(), test_tag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicLessThan          <nbits, es>(), test_tag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicLessOrEqualThan   <nbits, es>(), test_tag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicGreaterThan       <nbits, es>(), test_tag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicGreaterOrEqualThan<nbits, es>(), test_tag, "    >=         (native)  ");

	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyAddition         <nbits, es>(reportTestCases), test_tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction      <nbits, es>(reportTestCases), test_tag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication   <nbits, es>(reportTestCases), test_tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision         <nbits, es>(reportTestCases), test_tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation         <nbits, es>(reportTestCases), test_tag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation    <nbits, es>(reportTestCases), test_tag, "reciprocate    (native)  ");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// special cases
	std::cout << "Special case tests\n";
	std::string test = "Initialize to zero: ";
	p = 0;
	nrOfFailedTestCases += ReportCheck(test_tag, test, p.iszero());
	test = "Initialize to NAN";
	p = NAN;
	nrOfFailedTestCases += ReportCheck(test_tag, test, p.isnar());
	test = "Initialize to INFINITY";
	p = INFINITY;
	nrOfFailedTestCases += ReportCheck(test_tag, test, p.isnar());
	test = "sign is true";
	p = -1.0f;
	nrOfFailedTestCases += ReportCheck(test_tag, test, p.sign());
	test = "is negative";
	nrOfFailedTestCases += ReportCheck(test_tag, test, p.isneg());
	test = "sign is false";
	p = +1.0f;
	nrOfFailedTestCases += ReportCheck(test_tag, test, !p.sign());
	test = "is positive";
	nrOfFailedTestCases += ReportCheck(test_tag, test, p.ispos());

	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<nbits, es>(reportTestCases), test_tag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion       <nbits, es>(reportTestCases), test_tag, "float assign   (native)  ");

	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicEqual             <nbits, es>(), test_tag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicNotEqual          <nbits, es>(), test_tag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessThan          <nbits, es>(), test_tag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessOrEqualThan   <nbits, es>(), test_tag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterThan       <nbits, es>(), test_tag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterOrEqualThan<nbits, es>(), test_tag, "    >=         (native)  ");

	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyAddition         <nbits, es>(reportTestCases), test_tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifySubtraction      <nbits, es>(reportTestCases), test_tag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyMultiplication   <nbits, es>(reportTestCases), test_tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyDivision         <nbits, es>(reportTestCases), test_tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyNegation         <nbits, es>(reportTestCases), test_tag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyReciprocation    <nbits, es>(reportTestCases), test_tag, "reciprocate    (native)  ");

	// elementary function tests
	std::cout << "Elementary function tests\n";
//	nrOfFailedTestCases += ReportTestResult( VerifySqrt             <nbits, es>(reportTestCases), test_tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp              <nbits, es>(reportTestCases), test_tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2             <nbits, es>(reportTestCases), test_tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog              <nbits, es>(reportTestCases), test_tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2             <nbits, es>(reportTestCases), test_tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10            <nbits, es>(reportTestCases), test_tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine             <nbits, es>(reportTestCases), test_tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine           <nbits, es>(reportTestCases), test_tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent          <nbits, es>(reportTestCases), test_tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan             <nbits, es>(reportTestCases), test_tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin             <nbits, es>(reportTestCases), test_tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos             <nbits, es>(reportTestCases), test_tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh             <nbits, es>(reportTestCases), test_tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh             <nbits, es>(reportTestCases), test_tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh             <nbits, es>(reportTestCases), test_tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh            <nbits, es>(reportTestCases), test_tag, "atanh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh            <nbits, es>(reportTestCases), test_tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh            <nbits, es>(reportTestCases), test_tag, "asinh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction    <nbits, es>(reportTestCases), test_tag, "pow                      ");

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif // MANUAL_TESTING
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
