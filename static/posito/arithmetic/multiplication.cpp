// multiplication.cpp: test suite runner for posit multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized specialized posit configurations
//#define POSITO_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSITO_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing a MUL the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_MUL
#include <universal/number/posito/posito.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

namespace sw {
	namespace testing {
		// enumerate all multiplication cases for a posit configuration: is within 10sec till about nbits = 14
		template<typename PositType>
		int VerifyMultiplication(bool reportTestCases) {
			constexpr unsigned nbits = PositType::nbits;
			const unsigned NR_POSITS = (1 << nbits);
			int nrOfFailedTests = 0;
			for (unsigned i = 0; i < NR_POSITS; i++) {
				PositType pa;
				pa.setbits(i);
				double da = double(pa);
				for (unsigned j = 0; j < NR_POSITS; j++) {
					PositType pb, pc, pref;
					pb.setbits(j);
					double db = double(pb);
					double dc = da * db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						pc = pa * pb;
					}
					catch (const posit_operand_is_nar&) {
						if (pa.isnar() || pb.isnar()) {
							// correctly caught the exception
							pmul.setnar();
						}
						else {
							throw;  // rethrow
						}
					}
#else
					pc = pa * pb;
#endif
					pref = dc;
					//sw::universal::ReportBinaryOperation(pa, "*", pb, pc);
					//sw::universal::ReportBinaryOperation(da, "*", db, dc);
					if (pc != pref) {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", pa, pb, pc, pref);
						nrOfFailedTests++;
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pc, pref);
					}
				}
			}
			return nrOfFailedTests;
		}

		template<typename PositType, typename PositoType>
		int VerifyMultiplicationWithPosito(bool reportTestCases) {
			constexpr unsigned nbits = PositType::nbits;
			const unsigned NR_POSITS = (1 << nbits);
			int nrOfFailedTests = 0;
			for (unsigned i = 0; i < NR_POSITS; i++) {
				PositType pa;
				pa.setbits(i);
				PositoType ra;
				ra.setbits(i);
				for (unsigned j = 0; j < NR_POSITS; j++) {
					PositType pb, pc, pref;
					pb.setbits(j);
					PositoType rb, rc;
					rb.setbits(j);
					rc = ra * rb;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						pc = pa * pb;
					}
					catch (const posit_operand_is_nar&) {
						if (pa.isnar() || pb.isnar()) {
							// correctly caught the exception
							pmul.setnar();
						}
						else {
							throw;  // rethrow
						}
					}
#else
					pc = pa * pb;
#endif
					pref = double(rc);
					if (pc != pref) {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", pa, pb, pc, pref);
						nrOfFailedTests++;
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pc, pref);
					}
				}
			}
			return nrOfFailedTests;
		}
	}
}

template<typename PositType>
void TestDecode(const PositType& a) {
	using namespace sw::universal;
	short m;
	uint16_t bits, exp, fraction;

	bits = a.bits();
	a.decode_posit(bits, m, exp, fraction);
	std::cout << "bits     : " << to_binary(bits, 16, true) << '\n';
	std::cout << "m        : " << int(m) << '\n';
	std::cout << "exponent : " << to_binary(exp, 16, true) << " : " << int(exp) << '\n';
	std::cout << "fraction : " << to_binary(fraction, 16, true) << '\n';
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	std::string test_suite  = "posito multiplication verification";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	/*
	* 	fraction carry processing commensing
	0b0.0000'0000'0001.00.1'' * 0b0.10.01.100'1000'1101
	0b0.0000'0000'0001.00.1'' * 0b0.10.01.100'1000'1110
	0b0.0000'0000'0001.00.1'' * 0b0.10.01.100'1000'1111
	0b0.0000'0000'0001.00.1'' * 0b0.10.01.100'1001'0000
	0b0.0000'0000'0001.00.1'' * 0b0.10.01.100'1001'0001
	*/
	posit<16, 2> a(1), b(16), c;
	a.setbits(0x0009);
	b.setbits(0x4C8D);
	c = a * b;
	ReportBinaryOperation(a, "*", b, c);

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 1>>(reportTestCases, OPCODE_MUL, 65536), "posito<16,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 2>>(reportTestCases, OPCODE_MUL, 65536), "posito<16,2>", "multiplication");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<2, 0>>(reportTestCases), "posito< 2,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<3, 0>>(reportTestCases), "posito< 3,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<4, 0>>(reportTestCases), "posito< 4,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<8, 0>>(reportTestCases), "posito< 8,0>", "multiplication");
	// TODO: no fast posit<8,1> yet
	//nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<8, 1>(reportTestCases), "posito< 8,1>", "multiplication");
	// TODO: no working fast posit<8,2> yet
	//nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<8, 2>(reportTestCases), "posito< 8,2>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 1>>(reportTestCases, OPCODE_MUL, 65536), "posito<16,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 2>>(reportTestCases, OPCODE_MUL, 65536), "posito<16,2>", "multiplication");

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 2>>(reportTestCases, OPCODE_MUL, 1000), "posito<16,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<24, 2>>(reportTestCases, OPCODE_MUL, 1000), "posito<24,2>", "multiplication");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<32, 2>>(reportTestCases, OPCODE_MUL, 1000), "posito<32,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<32, 3>>(reportTestCases, OPCODE_MUL, 1000), "posito<32,3>", "multiplication");
#endif

#if REGRESSION_LEVEL_4
	// nbits=48 is also showing failures
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<48, 2>>(reportTestCases, OPCODE_MUL, 1000), "posit<48,2>", "multiplication");

	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 2>>(reportTestCases, OPCODE_MUL, 1000), "posito<64,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 3>>(reportTestCases, OPCODE_MUL, 1000), "posito<64,3>", "multiplication");
	// posit<64,4> is hitting subnormal numbers
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 4>>(reportTestCases, OPCODE_MUL, 1000), "posito<64,4>", "multiplication");

#ifdef HARDWARE_ACCELERATION
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<12, 1>>(reportTestCases), "posito<12,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<14, 1>>(reportTestCases), "posito<14,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<16, 1>>(reportTestCases), "posito<16,1>", "multiplication");
#endif // HARDWARE_ACCELERATION

#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
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

