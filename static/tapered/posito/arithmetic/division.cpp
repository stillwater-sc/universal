// division.cpp: test suite runner for posit division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

//
// Configure the posit template environment
// enable/disable general or specialized posit configurations
//#define POSITO_FAST_SPECIALIZATION
// enable/disable posit arithmetic exceptions
#define POSITO_THROW_ARITHMETIC_EXCEPTION 0
// enable/disable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_DIV
#include <universal/number/posito/posito.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename PositType, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	constexpr unsigned nbits = PositType::nbits;
	PositType pa, pb, pc, pref;
	pa = a;
	pb = b;
	pc = pa / pb;
	Ty ref = a / b;
	pref = ref;
	auto prec = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	ReportBinaryOperation(pa, "/", pb, pc);
	std::cout << (pref == pc ? "PASS" : "FAIL") << "\n\n";
	std::cout << std::setprecision(prec);
}

template<typename PositType>
void GenerateWorstCaseDivision() {
	constexpr unsigned nbits = PositType::nbits;
	constexpr unsigned es = PositType::es;
	PositType p_plus_eps(1), p_minus_eps(1), p_result;
	p_plus_eps++;
	p_minus_eps--;
	p_result = p_plus_eps / p_minus_eps;
	if (es < 2) {
		std::cout << type_tag(PositType()) << " minpos = " << std::fixed << std::setprecision(nbits) << sw::universal::posit<nbits, es>(sw::universal::SpecificValue::minpos) << std::dec << std::endl;
	}
	else {
		std::cout << type_tag(PositType()) << " minpos = " << std::setprecision(nbits) << sw::universal::posit<nbits, es>(sw::universal::SpecificValue::minpos) << std::endl;
	}
	std::cout << p_plus_eps.get() << " / " << p_minus_eps.get() << " = " << p_result.get() << std::endl;
	std::cout << std::setprecision(nbits - 2) << std::setw(nbits) << p_plus_eps << " / " << std::setw(nbits) << p_minus_eps << " = " << std::setw(nbits) << p_result << std::endl;
	std::cout << std::endl;
}

namespace sw {
	namespace testing {
		// enumerate all division cases for a posit configuration: is within 10sec till about nbits = 14
		template<typename PositType>
		int VerifyDivision(bool reportTestCases) {
			// constexpr unsigned nbits = PositType::nbits;
			constexpr unsigned NR_POSITS = 16; // (unsigned(1) << nbits);
			int nrOfFailedTests = 0;
			for (unsigned i = 0; i < NR_POSITS; i++) {
				PositType pa;
				pa.setbits(i);
				double da = double(pa);
				for (unsigned j = 0; j < NR_POSITS; j++) {
					PositType pb, pdiv, pref;
					pb.setbits(j);
					double db = double(pb);

#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						pdiv = pa / pb;
						pref = da / db;
					}
					catch (const posit_divide_by_zero&) {
						if (pb.iszero()) {
							// correctly caught the divide by zero condition
							continue;
							//pdiv.setnar();
						}
						else {
							if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
							throw; // rethrow
						}
					}
					catch (const posit_divide_by_nar&) {
						if (pb.isnar()) {
							// correctly caught the divide by nar condition
							continue;
							//pdiv = 0.0f;
						}
						else {
							if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
							throw; // rethrow
						}
					}
					catch (const posit_numerator_is_nar&) {
						if (pa.isnar()) {
							// correctly caught the numerator is nar condition
							continue;
							//pdiv.setnar();
						}
						else {
							if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
							throw; // rethrow
						}
					}
#else
					pdiv = pa / pb;
					pref = da / db;
#endif
					// check against the IEEE reference
					if (pdiv != pref) {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
						nrOfFailedTests++;
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", pa, pb, pdiv, pref);
					}
					if (nrOfFailedTests > 10) return 10;
				}
			}
			return nrOfFailedTests;
		}
	}
}

template<typename PositType>
void ScalesOfGeometricRegime() {
	using namespace sw::universal;
	std::cout << dynamic_range(PositType()) << '\n';
	PositType p(SpecificValue::maxpos);
	for (int i = 0; i < 5; ++i) {
		std::cout << to_binary(p) << " : " << scale(p) << " : " << p << '\n';
		--p;
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

	std::string test_suite  = "posito division verification";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

//	ToughDivisions2<posit<16,1>>();

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 2>>(reportTestCases, OPCODE_DIV, 65536), "posito<16,2>", "division");

//	nrOfFailedTestCases += ReportTestResult(sw::testing::VerifyDivision<posito< 8, 0>>(true), "posito<8,0>", "division");
//	nrOfFailedTestCases += ReportTestResult(sw::testing::VerifyDivision<posito<16, 1>>(true), "posito<16,1>", "division");
//	nrOfFailedTestCases += ReportTestResult(sw::testing::VerifyDivision<posito<16, 2>>(true), "posito<16,2>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<2, 0>>(reportTestCases), "posito< 2,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<3, 0>>(reportTestCases), "posito< 3,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<4, 0>>(reportTestCases), "posito< 4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<8, 0>>(reportTestCases), "posito< 8,0>", "division");
	// TODO: no fast posit<8,1> yet
	//nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<8, 1>>(reportTestCases), "posito< 8,1>", "division");
	// TODO: no working fast posit<8,2> yet
	//nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<posito<8, 2>>(reportTestCases), "posito< 8,2>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 1>>(reportTestCases, OPCODE_DIV, 65536), "posito<16,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 2>>(reportTestCases, OPCODE_DIV, 65536), "posito<16,2>", "division");

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posito<10, 0>>(reportTestCases), "posito<10,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posito<10, 1>>(reportTestCases), "posito<10,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posito<10, 2>>(reportTestCases), "posito<10,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posito<10, 3>>(reportTestCases), "posito<10,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<16, 2>>(reportTestCases, OPCODE_DIV, 1000), "posito<16,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<24, 2>>(reportTestCases, OPCODE_DIV, 1000), "posito<24,2>", "division");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<20, 1>>(reportTestCases, OPCODE_DIV, 1000), "posito<20,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<28, 1>>(reportTestCases, OPCODE_DIV, 1000), "posito<28,1>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<32, 1>>(reportTestCases, OPCODE_DIV, 1000), "posito<32,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<32, 2>>(reportTestCases, OPCODE_DIV, 1000), "posito<32,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<32, 3>>(reportTestCases, OPCODE_DIV, 1000), "posito<32,3>", "division");
#endif

#if REGRESSION_LEVEL_4
	// nbits = 48 also shows failures
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<48, 2>>(reportTestCases, OPCODE_DIV, 1000), "posito<48,2>", "division");

    // nbits=64 requires long double compiler support
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 2>>(reportTestCases, OPCODE_DIV, 1000), "posito<64,2>", "division");
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 3>>(reportTestCases, OPCODE_DIV, 1000), "posito<64,3>", "division");
    // posit<64,4> is hitting subnormal numbers
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posito<64, 4>>(reportTestCases, OPCODE_DIV, 1000), "posito<64,4>", "division");

#ifdef HARDWARE_ACCELERATION
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posito<12, 1>>(reportTestCases), "posito<12,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posito<14, 1>>(reportTestCases), "posito<14,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posito<16, 1>>(reportTestCases), "posito<16,1>", "division");
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

