// posit_4_0.cpp: test suite runner for specialized 4-bit posits based on look-up tables
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
// enable fast specialized posit<4,0>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_4_0 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit>
#include <universal/verification/posit_test_suite.hpp>

/*
 posits with nbits = 4 have no exponent bits, i.e. es = 0.
*/

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) { cout << argv[0] << endl; }

	// no randoms, 4-bit posits can be done exhaustively

	constexpr size_t nbits = 4;
	constexpr size_t es = 0;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = true;
	std::string tag = " posit<4,0>";

#if defined(POSIT_FAST_POSIT_4_0)
	cout << "Fast specialization posit<4,0> configuration tests" << endl;
#else
	cout << "Reference posit<4,0> configuration tests" << endl;
#endif

	posit<nbits,es> p;
	cout << dynamic_range(p) << endl;

	// special cases
	p = 0;
	if (!p.iszero()) ++nrOfFailedTestCases;
	p = NAN;
	if (!p.isnar()) ++nrOfFailedTestCases;
	p = INFINITY;
	if (!p.isnar()) ++nrOfFailedTestCases;

	// logic tests
	cout << "Logic operator tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicEqual             <nbits, es>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicNotEqual          <nbits, es>(), tag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicLessThan          <nbits, es>(), tag, "    <          ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicGreaterThan       <nbits, es>(), tag, "    >          ");
	nrOfFailedTestCases += ReportTestResult(VerifyPositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         ");
	
	// conversion tests
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<nbits, es>(bReportIndividualTestCases), tag, "integer assign ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion       <nbits, es>(bReportIndividualTestCases), tag, "float assign   ");
	
	// arithmetic tests
	cout << "Arithmetic tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition         <nbits, es>(bReportIndividualTestCases), tag, "add            ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction      <nbits, es>(bReportIndividualTestCases), tag, "subtract       ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication   <nbits, es>(bReportIndividualTestCases), tag, "multiply       ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision         <nbits, es>(bReportIndividualTestCases), tag, "divide         ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation         <nbits, es>(bReportIndividualTestCases), tag, "negate         ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation    <nbits, es>(bReportIndividualTestCases), tag, "reciprocate    ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult(VerifySqrt             <nbits, es>(bReportIndividualTestCases), tag, "sqrt           ");

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
