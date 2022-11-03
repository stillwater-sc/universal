// sqrt.cpp: test suite runner for posit sqrt
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define POSIT_VERBOSE_OUTPUT executing an SQRT the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_SQRT
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_random.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

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
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> sqrt(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> sqrt( " << pa << ") = " << psqrt.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psqrt ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
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

	std::string test_suite  = "posit square root validation";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	//GenerateTestCase<6, 3, double>(INFINITY);
	my_test_sqrt(0.25f);
	GenerateTestCase<3, 1, float>(4.0f);
	posit<3, 1> p(2.0000000001f);
	std::cout << p.get() << '\n';

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

#if CHECK_REFERENCE_SQRT_ALGORITHM
	// std::sqrt(negative) returns a -NaN(ind)
	cout << setprecision(17);
	float base = 0.5f;
	for (int i = 0; i < 32; i++) {
		float square = base*base;
		float root = sw::universal::my_test_sqrt(square);
		cout << "base " << base << " root " << root << endl;
		base *= 2.0f;
	}
	std::cout << "sqrt(2.0) " << sw::universal::my_test_sqrt(2.0f) << '\n';

#endif

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<2, 0>("Manual Testing", true), "posit<2,0>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<3, 0>("Manual Testing", true), "posit<3,0>", "sqrt");
//	nrOfFailedTestCases += ReportTestResult(VerifySqrt<3, 1>("Manual Testing", true), "posit<3,1>", "sqrt");   // TODO: these configs where nbits < es+sign+regime don't work

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<4, 0>("Manual Testing", true), "posit<4,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<4, 1>("Manual Testing", true), "posit<4,1>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<5, 0>("Manual Testing", true), "posit<5,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<5, 1>("Manual Testing", true), "posit<5,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<5, 2>("Manual Testing", true), "posit<5,2>", "sqrt");

	//nrOfFailedTestCases += ReportTestResult(VerifySqrt<8, 4>("Manual Testing", true), "posit<8,4>", "sqrt");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<2, 0>(reportTestCases), "posit<2,0>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<3, 0>(reportTestCases), "posit<3,0>", "sqrt");
//	nrOfFailedTestCases += ReportTestResult(VerifySqrt<3, 1>(reportTestCases), "posit<3,1>", "sqrt");	// TODO: these configs where nbits < es+sign+regime don't work

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<4, 0>(reportTestCases), "posit<4,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<4, 1>(reportTestCases), "posit<4,1>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<5, 0>(reportTestCases), "posit<5,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<5, 1>(reportTestCases), "posit<5,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<5, 2>(reportTestCases), "posit<5,2>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<6, 0>(reportTestCases), "posit<6,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<6, 1>(reportTestCases), "posit<6,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<6, 2>(reportTestCases), "posit<6,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<6, 3>(reportTestCases), "posit<6,3>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<7, 0>(reportTestCases), "posit<7,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<7, 1>(reportTestCases), "posit<7,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<7, 2>(reportTestCases), "posit<7,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<7, 3>(reportTestCases), "posit<7,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<7, 4>(reportTestCases), "posit<7,4>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<8, 0>(reportTestCases), "posit<8,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<8, 1>(reportTestCases), "posit<8,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<8, 2>(reportTestCases), "posit<8,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<8, 3>(reportTestCases), "posit<8,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<8, 4>(reportTestCases), "posit<8,4>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<8, 5>(reportTestCases), "posit<8,5>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<9, 0>(reportTestCases), "posit<9,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<9, 1>(reportTestCases), "posit<9,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<9, 2>(reportTestCases), "posit<9,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<9, 3>(reportTestCases), "posit<9,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<9, 4>(reportTestCases), "posit<9,4>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<9, 5>(reportTestCases), "posit<9,5>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<9, 6>(reportTestCases), "posit<9,6>", "sqrt");
	
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<10, 0>(reportTestCases), "posit<10,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<10, 1>(reportTestCases), "posit<10,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<10, 2>(reportTestCases), "posit<10,2>", "sqrt");
	// fails due to regime representation not being able to be represented by double
	// nrOfFailedTestCases += ReportTestResult(VerifySqrt<10, 7>(reportTestCases), "posit<10,7>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<12, 0>(reportTestCases), "posit<12,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<12, 1>(reportTestCases), "posit<12,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<12, 2>(reportTestCases), "posit<12,2>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifySqrt<16, 0>(reportTestCases), "posit<16,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<16, 1>(reportTestCases), "posit<16,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<16, 2>(reportTestCases), "posit<16,2>", "sqrt");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<10, 2>(reportTestCases), "posit<10,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<12, 2>(reportTestCases), "posit<12,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<14, 2>(reportTestCases), "posit<14,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<16, 2>(reportTestCases), "posit<16,2>", "sqrt");

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	using Posit64_2 = posit<64, 2>;
	using Posit64_3 = posit<64, 3>;
	using Posit64_4 = posit<64, 4>;

	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit64_2 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit64_2(SpecificValue::minpos))), type_tag(Posit64_2()), "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Posit64_3 >(reportTestCases, OPCODE_SQRT, 1000, double(Posit64_3(SpecificValue::minpos))), type_tag(Posit64_3()), "sqrt");
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
