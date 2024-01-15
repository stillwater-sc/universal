// posit_3_0.cpp: test suite runner for specialized 3-bit posits based on look-up tables
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// enable fast specialized posit<3,0>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_3_0 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>

// posit of size nbits = 3 without exponent bits, i.e. es = 0.

int main()
try {
	using namespace sw::universal;

	// no randoms, 3-bit posits can be done exhaustively

	constexpr size_t nbits = 3;
	constexpr size_t es = 0;

	int nrOfFailedTestCases = 0;
	bool reportTestCases = false;
	std::string tag = " posit<3,0>";

#if defined(POSIT_FAST_POSIT_3_0)
	std::cout << "Fast specialization posit<3,0> configuration tests\n";
#else
	std::cout << "Reference posit<3,0> configuration tests\n";
#endif

	posit<nbits,es> p;
	std::cout << dynamic_range(p) << '\n';

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
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<posit<nbits, es>>(reportTestCases), tag, "integer assign ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion       <posit<nbits, es>, float>(reportTestCases), tag, "float assign   ");

	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual             <posit<nbits,es>>(reportTestCases), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual          <posit<nbits,es>>(reportTestCases), tag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan          <posit<nbits,es>>(reportTestCases), tag, "    <          ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan   <posit<nbits,es>>(reportTestCases), tag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan       <posit<nbits,es>>(reportTestCases), tag, "    >          ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<nbits,es>>(reportTestCases), tag, "    >=         ");

	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyAddition         <posit<nbits,es>>(reportTestCases), tag, "add            ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction      <posit<nbits,es>>(reportTestCases), tag, "subtract       ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication   <posit<nbits,es>>(reportTestCases), tag, "multiply       ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision         <posit<nbits,es>>(reportTestCases), tag, "divide         ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation         <posit<nbits,es>>(reportTestCases), tag, "negate         ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation    <posit<nbits,es>>(reportTestCases), tag, "reciprocate    ");

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
