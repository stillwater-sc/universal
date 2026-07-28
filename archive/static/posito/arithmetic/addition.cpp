// addition.cpp: test suite runner for posito addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSITO_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSITO_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_ADD
#include <universal/number/posito/posito.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename PositType, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	PositType pa, pb, pc, pref;
	pa = a;
	pb = b;
	pc = pa + pb;
	Ty ref = a + b;
	pref = ref;
	ReportBinaryOperation(pa, "+", pb, pc);
	std::cout << (pref == pc ? "PASS" : "FAIL") << "\n\n";
}

namespace sw {
	namespace testing {

		// enumerate all addition cases for a posit configuration
		template<typename PositType>
		int VerifyAddition(bool reportTestCases) {
			constexpr unsigned nbits = PositType::nbits;
			const unsigned NR_POSITS = (unsigned(1) << nbits);
			int nrOfFailedTests = 0;
			PositType pa, pb, psum, pref;

			double da, db;
			for (unsigned i = 0; i < NR_POSITS; i++) {
				pa.setbits(i);
				da = double(pa);
				for (unsigned j = 0; j < NR_POSITS; j++) {
					pb.setbits(j);
					db = double(pb);
					pref = da + db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						psum = pa + pb;
					}
					catch (const posit_operand_is_nar& err) {
						if (pa.isnar() || pb.isnar()) {
							// correctly caught the exception
							psum.setnar();
						}
						else {
							throw err;
						}
					}

#else
					psum = pa + pb;
#endif
					if (psum != pref) {
						nrOfFailedTests++;
						if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", pa, pb, psum, pref);
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, psum, pref);
					}

					if (nrOfFailedTests > 99) return NR_POSITS;
				}
			}

			return nrOfFailedTests;
		}
	}
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

	std::string test_suite  = "posito addition verification";
	std::string test_tag    = "addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	//GenerateTestCase<posito<6, 3>, double>(INFINITY, INFINITY);
	//GenerateTestCase<posito<8, 4>, float>(0.5f, -0.5f);
	//GenerateTestCase<posito<3, 0>, float>(0.5f, 1.0f);

	using FastPosit = posit<16, 2>;
	using RefPosit = posito<16, 2>;
	double da, db, dc;
	FastPosit fa, fb, fc;
	RefPosit ra, rb, rc;

	fa = 1.0f;
	fb.setbits(0x4A55);   // 0b0.10.01.010'0101'0101
	fc = fa + fb;
	ra = 1.0f;
	rb.setbits(0x4A55);
	rc = ra + rb;
	ReportBinaryOperation(fa, "+", fb, fc);
	ReportBinaryOperation(ra, "+", rb, rc);

//	GenerateTestCase<posito<16, 1>, float>(0.0f, 0.0f);

	// manual exhaustive test
//	nrOfFailedTestCases += ReportTestResult(sw::testing::VerifyAddition< posito<3, 0> >(reportTestCases), "posito<3,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(sw::testing::VerifyAddition< posito<8, 0> >(reportTestCases), "posito<8,0>", "addition");
//	nrOfFailedTestCases += ReportTestResult(sw::testing::VerifyAddition< posito<16, 2> >(reportTestCases), "posito<16,2>", "addition");

	//	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 2>(test_tag, reportTestCases, OPCODE_ADD, 1000), "posito<64,2>", "addition");
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<2, 0>>(reportTestCases), "posito< 2,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<3, 0>>(reportTestCases), "posito< 3,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<4, 0>>(reportTestCases), "posito< 4,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<8, 0>>(reportTestCases), "posito< 8,0>", "addition");
	// TODO: no fast posito<8,1> yet
	//nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<8, 1>>(reportTestCases), "posito< 8,1>", "multiplication");
	// TODO: no working fast posito<8,2> yet
	//nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<8, 2>>(reportTestCases), "posito< 8,2>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 1>>(reportTestCases, OPCODE_ADD, 65536), "posito<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 2>>(reportTestCases, OPCODE_ADD, 65536), "posito<16,2>", "addition");

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<10, 0>>(reportTestCases), "posito<10,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<10, 1>>(reportTestCases), "posito<10,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<10, 2>>(reportTestCases), "posito<10,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<10, 3>>(reportTestCases), "posito<10,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 2>>(reportTestCases, OPCODE_ADD, 1000), "posit<16,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<24, 2>>(reportTestCases, OPCODE_ADD, 1000), "posit<24,1>", "addition");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<20, 1>>(reportTestCases, OPCODE_ADD, 1000), "posito<20,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<28, 1>>(reportTestCases, OPCODE_ADD, 1000), "posito<28,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<32, 2>>(reportTestCases, OPCODE_ADD, 1000), "posito<32,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<32, 3>>(reportTestCases, OPCODE_ADD, 1000), "posito<32,3>", "addition");
#endif

#if REGRESSION_LEVEL_4
	// nbits=48 also shows failures
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<48, 2>>(reportTestCases, OPCODE_ADD, 1000), "posito<48,2>", "addition");

	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 2>>(reportTestCases, OPCODE_ADD, 1000), "posito<64,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 3>>(reportTestCases, OPCODE_ADD, 1000), "posito<64,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 4>>(reportTestCases, OPCODE_ADD, 1000), "posito<64,4>", "addition");

#ifdef HARDWARE_ACCELERATION
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<12, 1>>(reportTestCases), "posito<12,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<14, 1>>(reportTestCases), "posito<14,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posito<16, 1>>(reportTestCases), "posito<16,1>", "addition");
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
