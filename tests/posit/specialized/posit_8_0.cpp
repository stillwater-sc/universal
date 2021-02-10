// posit_8_0.cpp: test suite runner for fast specialized posit<8,0>
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable fast specialized posit<8,0>
#define POSIT_FAST_POSIT_8_0 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

// Standard posits with nbits = 8 have no exponent bits, i.e. es = 0.

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	// no randoms, 8-bit posits can be done exhaustively

	constexpr size_t nbits = 8;
	constexpr size_t es = 0;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<8,0>";

#if POSIT_FAST_POSIT_8_0
	cout << "Fast specialization posit<8,0> configuration tests" << endl;
#else
	cout << "Standard posit<8,0> configuration tests" << endl;
#endif
	posit<nbits, es> p;
	cout << dynamic_range(p) << endl;

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
	// special cases
	p = 0;
	if (!p.iszero()) ++nrOfFailedTestCases;
	p = NAN;
	if (!p.isnar()) ++nrOfFailedTestCases;
	p = INFINITY;
	if (!p.isnar()) ++nrOfFailedTestCases;

	// logic tests
	cout << "Logic operator tests " << endl;
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicEqual             <nbits, es>(), tag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicNotEqual          <nbits, es>(), tag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessThan          <nbits, es>(), tag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterThan       <nbits, es>(), tag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         (native)  ");

	// conversion tests
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion<nbits, es>(tag, bReportIndividualTestCases), tag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyConversion       <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign   (native)  ");

	// arithmetic tests
	cout << "Arithmetic tests " << endl;
	nrOfFailedTestCases += ReportTestResult( VerifyAddition           <nbits, es>(tag, bReportIndividualTestCases), tag,    "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceAddition    <nbits, es>(tag, bReportIndividualTestCases), tag,    "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifySubtraction        <nbits, es>(tag, bReportIndividualTestCases), tag,    "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceSubtraction <nbits, es>(tag, bReportIndividualTestCases), tag,    "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyMultiplication     <nbits, es>(tag, bReportIndividualTestCases), tag,    "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceMultiplication <nbits, es>(tag, bReportIndividualTestCases), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyDivision           <nbits, es>(tag, bReportIndividualTestCases), tag,    "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyInPlaceDivision    <nbits, es>(tag, bReportIndividualTestCases), tag,    "/=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyNegation           <nbits, es>(tag, bReportIndividualTestCases), tag,    "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyReciprocation      <nbits, es>(tag, bReportIndividualTestCases), tag,    "reciprocate    (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult( VerifySqrt             <nbits, es>(tag, bReportIndividualTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp              <nbits, es>(tag, bReportIndividualTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2             <nbits, es>(tag, bReportIndividualTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog              <nbits, es>(tag, bReportIndividualTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2             <nbits, es>(tag, bReportIndividualTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10            <nbits, es>(tag, bReportIndividualTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine             <nbits, es>(tag, bReportIndividualTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine           <nbits, es>(tag, bReportIndividualTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent          <nbits, es>(tag, bReportIndividualTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan             <nbits, es>(tag, bReportIndividualTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin             <nbits, es>(tag, bReportIndividualTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos             <nbits, es>(tag, bReportIndividualTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh             <nbits, es>(tag, bReportIndividualTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh             <nbits, es>(tag, bReportIndividualTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh             <nbits, es>(tag, bReportIndividualTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh            <nbits, es>(tag, bReportIndividualTestCases), tag, "atanh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh            <nbits, es>(tag, bReportIndividualTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh            <nbits, es>(tag, bReportIndividualTestCases), tag, "asinh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction    <nbits, es>(tag, bReportIndividualTestCases), tag, "pow                      ");

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
