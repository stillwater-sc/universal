// posit_4_0.cpp: test suite runner for specialized 4-bit posits based on look-up tables
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// enable fast specialized posit<4,0>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_4_0 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>

/*
 posits with nbits = 4 have no exponent bits, i.e. es = 0.
*/

int main()
try {
	using namespace sw::universal;

	// no randoms, 4-bit posits can be done exhaustively

	constexpr size_t nbits = 4;
	constexpr size_t es    = 0;

	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;


#if defined(POSIT_FAST_POSIT_4_0)
	std::cout << "Fast specialization posit<4,0> configuration tests\n";
#else
	std::cout << "Reference posit<4,0> configuration tests\n";
#endif

	using TestType = posit<nbits, es>;
	using EnvelopeType = posit<nbits + 1, es>;
	TestType p;
	std::string typeTag = type_tag(p);
	std::cout << dynamic_range(p) << "\n\n";

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

	/*
	Posit Lookup table for a POSIT<4, 0> in TXT format
		#   Binary  Decoded     k    sign   scale     value
		 0 : 0000    0000      -3       0      -2      0 
		 1 : 0001    0001      -2       0      -2      0.25
		 2 : 0010    0010      -1       0      -1      0.5
		 3 : 0011    0011      -1       0      -1      0.75
		 4 : 0100    0100       0       0       0      1
		 5 : 0101    0101       0       0       0      1.5
		 6 : 0110    0110       1       0       1      2
		 7 : 0111    0111       2       0       2      4
		 8 : 1000    1000       3       1      -2     nar
		 9 : 1001    1111       2       1       2     -4
		10 : 1010    1110       1       1       1     -2
		11 : 1011    1101       0       1       0     -1.5
		12 : 1100    1100       0       1       0     -1
		13 : 1101    1011      -1       1      -1     -0.75
		14 : 1110    1010      -1       1      -1     -0.5
		15 : 1111    1001      -2       1      -2     -0.25
	*/

	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<TestType>(reportTestCases), typeTag, "integer conversion ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion       <TestType, EnvelopeType, float>(reportTestCases), typeTag, "float conversion   ");

	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual             <TestType>(reportTestCases), typeTag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual          <TestType>(reportTestCases), typeTag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan          <TestType>(reportTestCases), typeTag, "    <          ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan   <TestType>(reportTestCases), typeTag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan       <TestType>(reportTestCases), typeTag, "    >          ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<TestType>(reportTestCases), typeTag, "    >=         ");
	
	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyAddition         <TestType>(reportTestCases), typeTag, "add            ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction      <TestType>(reportTestCases), typeTag, "subtract       ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication   <TestType>(reportTestCases), typeTag, "multiply       ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision         <TestType>(reportTestCases), typeTag, "divide         ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation         <TestType>(reportTestCases), typeTag, "negate         ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation    <TestType>(reportTestCases), typeTag, "reciprocate    ");

	// elementary function tests
	std::cout << "Elementary function tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt             <TestType>(reportTestCases), typeTag, "sqrt           ");

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
