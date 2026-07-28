// sqrt.cpp: test suite runner for posit sqrt
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an SQRT the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_SQRT
// select a native posit sqrt: default is to cheat and marshall through native double precision 
// to pass the regression tests that compare to std::sqrt references
// 
//                    Native posit sqrt algorithm results
// posit<10,2>                                                  sqrt PASS
// posit<12, 2>                                                 sqrt PASS
// posit<14, 2>                                                 sqrt PASS
// posit<16, 2>                                                 sqrt PASS
// posit< 20, 2>                                                sqrt PASS
// posit< 24, 2>                                                sqrt FAIL 5 failed test cases
// posit< 28, 2>                                                sqrt FAIL 20 failed test cases
// posit< 32, 1>                                                sqrt FAIL 188 failed test cases
// posit< 32, 2>                                                sqrt FAIL 180 failed test cases
// posit< 32, 3>                                                sqrt FAIL 157 failed test cases
// posit< 64, 2>                                                sqrt FAIL 998 failed test cases
// posit< 64, 3>                                                sqrt FAIL 999 failed test cases
// posit< 64, 4>                                                sqrt FAIL 999 failed test cases
// The Newton iteration that is used in the native sqrt algorithm
// needs to run on a higher precision intermediate to yield correct approximation
//
// #define POSIT_NATIVE_SQRT 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, psqrt;
	pa = a;
	ref = std::sqrt(a);
	pref = ref;
	psqrt = sw::universal::sqrt(pa);
	auto precision = std::cout.precision();
	std::cout << std::setprecision(17);
	std::cout << std::setw(nbits) <<  a << " -> sqrt("  << a << ") = " << std::setw(nbits) << ref << '\n';
	std::cout << std::setw(nbits) << pa << " -> sqrt(" << pa << ") = " << std::setw(nbits) << psqrt << '\n';
	std::cout << pa.get() << " -> sqrt(" << pa << ") = " << psqrt.get() << '\n';
	std::cout << std::setw(nbits + 35) << " reference = " << pref.get() << " : ";
	std::cout << (pref == psqrt ? "PASS" : "FAIL") << "\n\n";
	std::cout << color_print(psqrt) << '\n';
	std::cout << std::setprecision(precision);
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

	std::string test_suite  = "posit square root verification";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	//GenerateTestCase<6, 3, double>(INFINITY);
	my_test_sqrt(0.25f);
	GenerateTestCase<3, 1, float>(4.0f);
	posit<3, 1> p(2.0000000001f);
	std::cout << p.get() << '\n';

	posit<16, 2> minpos(SpecificValue::minpos);
	double v = sqrt(double(minpos)); // so that we have representable value
	GenerateTestCase < 16, 2, double>(v);
	GenerateTestCase < 32, 2, double>(v);
	GenerateTestCase < 64, 2, double>(v);
	GenerateTestCase <128, 2, double>(v);
	GenerateTestCase <256, 2, double>(v);
	/*
	* 	The posit native fast sqrt algorithm uses double constants which causes approximation error in precise posit configurations
	* 
	* 
	3.7252902984619141e-09 -> sqrt(3.7252902984619141e-09) =  6.103515625e-05
	3.7252902984619141e-09 -> sqrt(3.7252902984619141e-09) =  6.103515625e-05
	0000000010000000 -> sqrt(3.7252902984619141e-09) = 0000011000000000
                                           reference = 0000011000000000 : PASS


    3.7252902984619141e-09 -> sqrt(3.7252902984619141e-09) =                  6.103515625e-05
    3.7252902984619141e-09 -> sqrt(3.7252902984619141e-09) =                  6.103515625e-05
	00000000100000000000000000000000 -> sqrt(3.7252902984619141e-09) = 00000110000000000000000000000000
                                                           reference = 00000110000000000000000000000000 : PASS


     3.7252902984619141e-09 -> sqrt(3.7252902984619141e-09) =                                                  6.103515625e-05
     3.7252902984619141e-09 -> sqrt(3.7252902984619141e-09) =                                           6.1035156273424418e-05
	0000000010000000000000000000000000000000000000000000000000000000 -> sqrt(3.7252902984619141e-09) = 0000011000000000000000000000000000000001101001011111101000001001
                                                                                           reference = 0000011000000000000000000000000000000000000000000000000000000000 : FAIL
	
	*/

	return 0;

#if GENERATE_SQRT_TABLES
	GenerateSqrtTable<3, 0>();
	GenerateSqrtTable<4, 0>();
	GenerateSqrtTable<4, 1>();
	GenerateSqrtTable<5, 0>();
	GenerateSqrtTable<5, 1>();
	GenerateSqrtTable<5, 2>();
	GenerateSqrtTable<6, 0>();
	GenerateSqrtTable<6, 1>();
	GenerateSqrtTable<6, 2>();
	GenerateSqrtTable<6, 3>();
	GenerateSqrtTable<7, 0>();
#endif

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<2, 0>>(reportTestCases), "posit<2,0>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<3, 0>>(reportTestCases), "posit<3,0>", "sqrt");
//	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<3, 1>>(reportTestCases), "posit<3,1>", "sqrt");   // TODO: these configs where nbits < es+sign+regime don't work

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<4, 0>>(reportTestCases), "posit<4,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<4, 1>>(reportTestCases), "posit<4,1>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<5, 0>>(reportTestCases), "posit<5,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<5, 1>>(reportTestCases), "posit<5,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<5, 2>>(reportTestCases), "posit<5,2>", "sqrt");

	//nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<8, 4>("Manual Testing", true), "posit<8,4>", "sqrt");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<2, 0>>(reportTestCases), "posit<2,0>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<3, 0>>(reportTestCases), "posit<3,0>", "sqrt");
//	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<3, 1>>(reportTestCases), "posit<3,1>", "sqrt");	// TODO: these configs where nbits < es+sign+regime don't work

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<4, 0>>(reportTestCases), "posit<4,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<4, 1>>(reportTestCases), "posit<4,1>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<5, 0>>(reportTestCases), "posit<5,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<5, 1>>(reportTestCases), "posit<5,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<5, 2>>(reportTestCases), "posit<5,2>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<6, 0>>(reportTestCases), "posit<6,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<6, 1>>(reportTestCases), "posit<6,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<6, 2>>(reportTestCases), "posit<6,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<6, 3>>(reportTestCases), "posit<6,3>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<7, 0>>(reportTestCases), "posit<7,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<7, 1>>(reportTestCases), "posit<7,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<7, 2>>(reportTestCases), "posit<7,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<7, 3>>(reportTestCases), "posit<7,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<7, 4>>(reportTestCases), "posit<7,4>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<8, 0>>(reportTestCases), "posit<8,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<8, 1>>(reportTestCases), "posit<8,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<8, 2>>(reportTestCases), "posit<8,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<8, 3>>(reportTestCases), "posit<8,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<8, 4>>(reportTestCases), "posit<8,4>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<8, 5>>(reportTestCases), "posit<8,5>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<9, 0>>(reportTestCases), "posit<9,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<9, 1>>(reportTestCases), "posit<9,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<9, 2>>(reportTestCases), "posit<9,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<9, 3>>(reportTestCases), "posit<9,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<9, 4>>(reportTestCases), "posit<9,4>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<9, 5>>(reportTestCases), "posit<9,5>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<9, 6>>(reportTestCases), "posit<9,6>", "sqrt");
	
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<10, 0>>(reportTestCases), "posit<10,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<10, 1>>(reportTestCases), "posit<10,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<10, 2>>(reportTestCases), "posit<10,2>", "sqrt");
	// fails due to regime representation not being able to be represented by double
	// nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<10, 7>>(reportTestCases), "posit<10,7>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<12, 0>>(reportTestCases), "posit<12,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<12, 1>>(reportTestCases), "posit<12,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<12, 2>>(reportTestCases), "posit<12,2>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<16, 0>>(reportTestCases), "posit<16,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<16, 1>>(reportTestCases), "posit<16,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<16, 2>>(reportTestCases), "posit<16,2>", "sqrt");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<10, 2>>(reportTestCases), "posit<10,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<12, 2>>(reportTestCases), "posit<12,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<14, 2>>(reportTestCases), "posit<14,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<posit<16, 2>>(reportTestCases), "posit<16,2>", "sqrt");

	using Posit20_2 = posit<20, 2>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit20_2 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit20_2(SpecificValue::minpos))), type_tag(Posit20_2()), "sqrt");
	using Posit24_2 = posit<24, 2>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit24_2 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit24_2(SpecificValue::minpos))), type_tag(Posit24_2()), "sqrt");
	using Posit28_2 = posit<28, 2>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit28_2 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit28_2(SpecificValue::minpos))), type_tag(Posit28_2()), "sqrt");

#endif

#if REGRESSION_LEVEL_3
	// TBD: currently, these tests will fail as the native posit sqrt algorithm needs one more iteration to match std::sqrt(double)
	using Posit32_1 = posit<32, 1>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit32_1 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit32_1(SpecificValue::minpos))), type_tag(Posit32_1()), "sqrt");
	using Posit32_2 = posit<32, 2>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit32_2 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit32_2(SpecificValue::minpos))), type_tag(Posit32_2()), "sqrt");
	using Posit32_3 = posit<32, 3>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit32_3 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit32_3(SpecificValue::minpos))), type_tag(Posit32_3()), "sqrt");

#endif

#if REGRESSION_LEVEL_4
	// TBD: currently, these tests will fail as the native posit sqrt algorithm needs 2-3 more iterations to match std::sqrt(long double)
	using Posit64_2 = posit<64, 2>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit64_2 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit64_2(SpecificValue::minpos))), type_tag(Posit64_2()), "sqrt");
	using Posit64_3 = posit<64, 3>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit64_3 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit64_3(SpecificValue::minpos))), type_tag(Posit64_3()), "sqrt");
	using Posit64_4 = posit<64, 4>;
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit64_4 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit64_4(SpecificValue::minpos))), type_tag(Posit64_4()), "sqrt");

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
