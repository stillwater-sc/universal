// posit_16_1.cpp: Functionality tests for specialized posit<16,1>
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable fast specialized posit<16,1>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_16_1 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
// test helpers, such as, ReportTestResults
#include "../../utils/test_helpers.hpp"
#include "../../utils/posit_test_randoms.hpp"
#include "../../utils/posit_math_helpers.hpp"

// Standard posit with nbits = 16 have es = 1 exponent bit.

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	const size_t RND_TEST_CASES = 2*1024*1024;

	constexpr size_t nbits = 16;
	constexpr size_t es = 1;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<16,1>";

#if POSIT_FAST_POSIT_16_1
	cout << "Fast specialization posit<16,1> configuration tests" << endl;
#else
	cout << "Standard posit<16,1> configuration tests" << endl;
#endif

#if MANUAL_TESTING
	float fa, fb;
	posit<nbits, es> a, b, c;
	fa = 2;	fb = 1;
	a = fa; b = fb; c = a; c += b;
	cout << hex_format(a) << " + " << hex_format(b) << " = " << hex_format(a + b) << "(" << (fa + fb) << ") " << hex_format(c) << "(" << c << ")" << endl;
	fa = 2;	fb = -1;
	a = fa; b = fb; c = a; c += b;
	cout << hex_format(a) << " + " << hex_format(b) << " = " << hex_format(a + b) << "(" << (fa + fb) << ") " << hex_format(c) << "(" << c << ")" << endl;
	fa = -2;	fb = 1;
	a = fa; b = fb; c = a; c += b;
	cout << hex_format(a) << " + " << hex_format(b) << " = " << hex_format(a + b) << "(" << (fa + fb) << ") " << hex_format(c) << "(" << c << ")" << endl;
	fa = -2;	fb = -1;
	a = fa; b = fb; c = a; c += b;
	cout << hex_format(a) << " + " << hex_format(b) << " = " << hex_format(a + b) << "(" << (fa + fb) << ") " << hex_format(c) << "(" << c << ")" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_IPA, 100), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_IPS, 100), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_IPM, 100), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_IPD, 100), tag, "/=             (native)  ");


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

	a.set_raw_bits(0xfffd); b.set_raw_bits(0xfffe);
	testLogicOperators(a, b);

	uint16_t v1 = 0x7fff;
	uint16_t v2 = 0x8001;
	cout << v1 << " vs " << int16_t(v1) << endl;
	cout << v2 << " vs " << int16_t(v2) << endl;
	a.set_raw_bits(v1); b.set_raw_bits(v2);
	testLogicOperators(a, b);
	testLogicOperators(b, a);


	cout << nrOfFailedTestCases << " number of failures\n";

	nrOfFailedTestCases = 0;  // ignore failures in manual testing
#else
	posit<nbits, es> p;
	cout << dynamic_range(p) << endl << endl;

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

	// logic tests
	cout << "Logic operator tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicEqual             <nbits, es>(), tag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicNotEqual          <nbits, es>(), tag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessThan          <nbits, es>(), tag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterThan       <nbits, es>(), tag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         (native)  ");

	// conversion tests
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateIntegerConversion           <nbits, es>(tag, bReportIndividualTestCases), tag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateConversion                  <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign   (native)  ");

	// arithmetic tests
	// State space is too large for exhaustive testing, so we use randoms to try to catch any silly regressions
	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition       (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_IPA, RND_TEST_CASES), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction    (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_IPS, RND_TEST_CASES), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_IPM, RND_TEST_CASES), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division       (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_IPD, RND_TEST_CASES), tag, "/=             (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateSqrt                        <nbits, es>(tag, bReportIndividualTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateExp                         <nbits, es>(tag, bReportIndividualTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateExp2                        <nbits, es>(tag, bReportIndividualTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateLog                         <nbits, es>(tag, bReportIndividualTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateLog2                        <nbits, es>(tag, bReportIndividualTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateLog10                       <nbits, es>(tag, bReportIndividualTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( ValidateSine                        <nbits, es>(tag, bReportIndividualTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateCosine                      <nbits, es>(tag, bReportIndividualTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateTangent                     <nbits, es>(tag, bReportIndividualTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateAsin                        <nbits, es>(tag, bReportIndividualTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateAcos                        <nbits, es>(tag, bReportIndividualTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateAtan                        <nbits, es>(tag, bReportIndividualTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateSinh                        <nbits, es>(tag, bReportIndividualTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateCosh                        <nbits, es>(tag, bReportIndividualTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateTanh                        <nbits, es>(tag, bReportIndividualTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateAsinh                       <nbits, es>(tag, bReportIndividualTestCases), tag, "asinh                    ");
	nrOfFailedTestCases += ReportTestResult( ValidateAcosh                       <nbits, es>(tag, bReportIndividualTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( ValidateAtanh                       <nbits, es>(tag, bReportIndividualTestCases), tag, "atanh                    ");

	nrOfFailedTestCases += ReportTestResult( ValidatePowerFunction               <nbits, es>(tag, bReportIndividualTestCases), tag, "pow                      ");
#endif
	cout << flush;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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
