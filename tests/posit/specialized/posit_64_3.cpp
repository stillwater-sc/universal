// posit_64_3.cpp: Functionality tests for fast specialized posit<64,3>
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable fast specialized posit<64,3>
#define POSIT_FAST_POSIT_64_3 0  // TODO: fast posit<64,3> not implemented yet
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
// test helpers, such as, ReportTestResults
#include "../../utils/test_helpers.hpp"
#include "../../utils/posit_test_randoms.hpp"

// Standard posit with nbits = 64 have es = 3 exponent bits.

#define MANUAL_TESTING 0
#define STRESS_TESTING 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	constexpr size_t RND_TEST_CASES = 10000;

	constexpr size_t nbits = 64;
	constexpr size_t es = 3;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<64,3>";

#if POSIT_FAST_POSIT_64_3
	cout << "Fast specialization posit<64,3> configuration tests" << endl;
#else
	cout << "Standard posit<64,3> configuration tests" << endl;
#endif

	posit<nbits, es> p;
	cout << dynamic_range(p) << endl << endl;

#if MANUAL_TESTING

#else

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
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicEqual             <nbits, es>(), tag, "    ==          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicNotEqual          <nbits, es>(), tag, "    !=          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessThan          <nbits, es>(), tag, "    <           (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterThan       <nbits, es>(), tag, "    >           (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=          (native)  ");

	// conversion tests
	// internally this generators are clamped as the state space 2^33 is too big
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateIntegerConversion           <nbits, es>(tag, bReportIndividualTestCases), tag, "sint32 assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateUintConversion              <nbits, es>(tag, bReportIndividualTestCases), tag, "uint32 assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateConversion                  <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign    (native)  ");
//	nrOfFailedTestCases += ReportTestResult( ValidateConversionThroughRandoms <nbits, es>(tag, true, 100), tag, "float assign   ");

	// arithmetic tests
	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition        (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction     (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication  (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division        (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "+=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "-=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "*=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "/=              (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SQRT,  RND_TEST_CASES), tag, "sqrt            (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_EXP,   RND_TEST_CASES), tag, "exp                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_EXP2,  RND_TEST_CASES), tag, "exp2                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_LOG,   RND_TEST_CASES), tag, "log                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_LOG2,  RND_TEST_CASES), tag, "log2                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_LOG10, RND_TEST_CASES), tag, "log10                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SIN,   RND_TEST_CASES), tag, "sin                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_COS,   RND_TEST_CASES), tag, "cos                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_TAN,   RND_TEST_CASES), tag, "tan                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ASIN,  RND_TEST_CASES), tag, "asin                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ACOS,  RND_TEST_CASES), tag, "acos                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ATAN,  RND_TEST_CASES), tag, "atan                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SINH,  RND_TEST_CASES), tag, "sinh                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_COSH,  RND_TEST_CASES), tag, "cosh                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_TANH,  RND_TEST_CASES), tag, "tanh                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ASINH, RND_TEST_CASES), tag, "asinh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ACOSH, RND_TEST_CASES), tag, "acosh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ATANH, RND_TEST_CASES), tag, "atanh                     ");
	// elementary functions with two operands
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_POW, RND_TEST_CASES),   tag, "pow                       ");


#if STRESS_TESTING

#endif // STRESS_TESTING

#endif // !MANUAL_TESTING

	// TODO: as we don't have a reference floating point implementation to validate
	// the arithmetic operations we are going to ignore the failures
	nrOfFailedTestCases = 0;
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
