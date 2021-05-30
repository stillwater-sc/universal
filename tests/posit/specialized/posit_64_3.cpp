// posit_64_3.cpp: test suite runner for fast specialized posit<64,3>
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
// first: enable fast specialized posit<64,3>
#define POSIT_FAST_POSIT_64_3 1  // TODO: fast posit<64,3> not implemented yet
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>

// Standard posit with nbits = 64 have es = 3 exponent bits.

#define MANUAL_TESTING 0
#define STRESS_TESTING 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) { cout << argv[0] << endl; }

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

	using Scalar = posit<nbits, es>;
	Scalar p;
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
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicEqual             <nbits, es>(), tag, "    ==          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicNotEqual          <nbits, es>(), tag, "    !=          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessThan          <nbits, es>(), tag, "    <           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterThan       <nbits, es>(), tag, "    >           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=          (native)  ");

	// conversion tests
	// internally this generators are clamped as the state space 2^33 is too big
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion           <nbits, es>(bReportIndividualTestCases), tag, "sint32 assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyUintConversion              <nbits, es>(bReportIndividualTestCases), tag, "uint32 assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyConversion                  <nbits, es>(bReportIndividualTestCases), tag, "float assign    (native)  ");
//	nrOfFailedTestCases += ReportTestResult( VerifyConversionThroughRandoms <nbits, es>(tag, true, 100), tag, "float assign   ");

	// arithmetic tests
	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition        (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction     (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication  (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division        (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "+=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "-=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "*=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "/=              (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	p.minpos();
	double dminpos = double(p);
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_SQRT,  RND_TEST_CASES, dminpos), tag, "sqrt            (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_EXP,   RND_TEST_CASES, dminpos), tag, "exp                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_EXP2,  RND_TEST_CASES, dminpos), tag, "exp2                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_LOG,   RND_TEST_CASES, dminpos), tag, "log                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_LOG2,  RND_TEST_CASES, dminpos), tag, "log2                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_LOG10, RND_TEST_CASES, dminpos), tag, "log10                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_SIN,   RND_TEST_CASES, dminpos), tag, "sin                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_COS,   RND_TEST_CASES, dminpos), tag, "cos                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_TAN,   RND_TEST_CASES, dminpos), tag, "tan                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_ASIN,  RND_TEST_CASES, dminpos), tag, "asin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_ACOS,  RND_TEST_CASES, dminpos), tag, "acos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_ATAN,  RND_TEST_CASES, dminpos), tag, "atan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_SINH,  RND_TEST_CASES, dminpos), tag, "sinh                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_COSH,  RND_TEST_CASES, dminpos), tag, "cosh                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_TANH,  RND_TEST_CASES, dminpos), tag, "tanh                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_ASINH, RND_TEST_CASES, dminpos), tag, "asinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_ACOSH, RND_TEST_CASES, dminpos), tag, "acosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(bReportIndividualTestCases, OPCODE_ATANH, RND_TEST_CASES, dminpos), tag, "atanh                     ");
	// elementary functions with two operands
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_POW, RND_TEST_CASES),   tag, "pow                       ");


#if STRESS_TESTING

#endif // STRESS_TESTING

#endif // !MANUAL_TESTING

	// TODO: as we don't have a reference floating point implementation to Verify
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
