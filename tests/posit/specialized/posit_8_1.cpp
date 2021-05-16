// posit_8_1.cpp: test suite runner for fast specialized posit<8,1>
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#pragma warning(disable : 4514) // unreferenced inline function has been removed
#pragma warning(disable : 4820) // bytes padding added after data member
#pragma warning(disable : 4710) // function not inlined
#endif
// Configure the posit template environment
// first: enable fast specialized posit<8,1>
#define POSIT_FAST_POSIT_8_1 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

/*
specialized small 8-bit posit with es = 1 to increase dynamic range over standard posit<8,0>.
*/

void GenerateValues() {
	using namespace std;
	using namespace sw::universal;
	constexpr unsigned int NR_POSITS = 256;

	posit<8, 1> a;
	for (unsigned int i = 0; i < NR_POSITS; ++i) {
		a.setbits(i);
		cout << hex << i << " " << dec << a << endl;
	}
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) { cout << argv[0] << endl; }

	// no randoms, 8-bit posits can be done exhaustively

	constexpr size_t nbits = 8;
	constexpr size_t es = 1;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<8,1>";

#if POSIT_FAST_POSIT_8_1
	cout << "Fast specialization posit<8,1> configuration tests" << endl;
#else
	cout << "Standard posit<8,1> configuration tests" << endl;
#endif

	posit<nbits, es> p;
	cout << dynamic_range(p) << endl;

	// special cases
	cout << "Special case tests " << endl;
	string test = "Initialize to zero: ";
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
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion<nbits, es>(bReportIndividualTestCases), tag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyConversion       <nbits, es>(bReportIndividualTestCases), tag, "float assign   (native)  ");

	// arithmetic tests
	cout << "Arithmetic tests " << endl;
	nrOfFailedTestCases += ReportTestResult( VerifyAddition         <nbits, es>(bReportIndividualTestCases), tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifySubtraction      <nbits, es>(bReportIndividualTestCases), tag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyMultiplication   <nbits, es>(bReportIndividualTestCases), tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyDivision         <nbits, es>(bReportIndividualTestCases), tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyNegation         <nbits, es>(bReportIndividualTestCases), tag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyReciprocation    <nbits, es>(bReportIndividualTestCases), tag, "reciprocate    (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
//	nrOfFailedTestCases += ReportTestResult( VerifySqrt             <nbits, es>(bReportIndividualTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp              <nbits, es>(bReportIndividualTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2             <nbits, es>(bReportIndividualTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog              <nbits, es>(bReportIndividualTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2             <nbits, es>(bReportIndividualTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10            <nbits, es>(bReportIndividualTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine             <nbits, es>(bReportIndividualTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine           <nbits, es>(bReportIndividualTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent          <nbits, es>(bReportIndividualTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan             <nbits, es>(bReportIndividualTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin             <nbits, es>(bReportIndividualTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos             <nbits, es>(bReportIndividualTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh             <nbits, es>(bReportIndividualTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh             <nbits, es>(bReportIndividualTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh             <nbits, es>(bReportIndividualTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh            <nbits, es>(bReportIndividualTestCases), tag, "atanh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh            <nbits, es>(bReportIndividualTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh            <nbits, es>(bReportIndividualTestCases), tag, "asinh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction    <nbits, es>(bReportIndividualTestCases), tag, "pow                      ");

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
