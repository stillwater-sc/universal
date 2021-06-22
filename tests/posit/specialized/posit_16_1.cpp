// posit_16_1.cpp: test suite runner for specialized posit<16,1>
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
// first: enable fast specialized posit<16,1>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_16_1 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
//#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

// Standard posit with nbits = 16 have es = 1 exponent bit.

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) { cout << argv[0] << endl; }

	constexpr size_t RND_TEST_CASES = 2*1024*1024;

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

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPA, 100), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPS, 100), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPM, 100), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPD, 100), tag, "/=             (native)  ");


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
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion           <nbits, es>(bReportIndividualTestCases), tag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyConversion                  <nbits, es>(bReportIndividualTestCases), tag, "float assign   (native)  ");

	// arithmetic tests
	// State space is too large for exhaustive testing, so we use randoms to try to catch any silly regressions
	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPA, RND_TEST_CASES), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction    (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPS, RND_TEST_CASES), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPM, RND_TEST_CASES), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPD, RND_TEST_CASES), tag, "/=             (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult( VerifySqrt                        <nbits, es>(bReportIndividualTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp                         <nbits, es>(bReportIndividualTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2                        <nbits, es>(bReportIndividualTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog                         <nbits, es>(bReportIndividualTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2                        <nbits, es>(bReportIndividualTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10                       <nbits, es>(bReportIndividualTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine                        <nbits, es>(bReportIndividualTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine                      <nbits, es>(bReportIndividualTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent                     <nbits, es>(bReportIndividualTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin                        <nbits, es>(bReportIndividualTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos                        <nbits, es>(bReportIndividualTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan                        <nbits, es>(bReportIndividualTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh                        <nbits, es>(bReportIndividualTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh                        <nbits, es>(bReportIndividualTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh                        <nbits, es>(bReportIndividualTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh                       <nbits, es>(bReportIndividualTestCases), tag, "asinh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh                       <nbits, es>(bReportIndividualTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh                       <nbits, es>(bReportIndividualTestCases), tag, "atanh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction               <nbits, es>(bReportIndividualTestCases), tag, "pow                      ");
#endif
	cout << flush;

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
