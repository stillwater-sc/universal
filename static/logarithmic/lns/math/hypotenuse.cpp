// hypotenuse.cpp: test suite runner for the hypotenuse functions (hypot, hypotf, hypotl)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/lns_test_suite_mathlib.hpp>
#include <universal/verification/test_suite_randoms.hpp>

// generate specific test case that you can trace with the trace conditions in lns.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename bt, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	sw::universal::lns<nbits, rbits, bt> a, b, pref, result;
	a = _a;
	b = _b;
	Ty ref = std::hypot(_a, _b);
	pref = ref;
	result = sw::universal::hypot(a, b);
	std::cout << std::setprecision(nbits - 2);
	std::cout << " hypot(" << _a << ", " << _b << ") = " << ref << '\n';
	std::cout << " hypot(" <<  a << ", " <<  b << ") = " << result << " : " << to_binary(result) << " (reference: " << to_binary(pref) << ")   ";
	std::cout << (pref == result ? "PASS" : "FAIL") << "\n\n";
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif


int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lns hypotenuse validation";
	std::string test_tag    = "hypotenuse";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	lns<8, 3, std::uint8_t> a(SpecificValue::maxpos);
	std::cout << "maxpos " << type_tag(a) << " : " << a << '\n';
	GenerateTestCase< 8, 3, std::uint8_t, float>(3.0f, 4.0f);
	GenerateTestCase<16, 5, std::uint8_t, float>(3.0f, 4.0f);

	nrOfFailedTestCases += ReportTestResult(VerifyHypot< lns<4, 1, std::uint8_t> >(reportTestCases), "lns<4, 1>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot< lns<8, 4, std::uint8_t> >(reportTestCases), "lns<8, 4>", "hypot");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyHypot< lns<4, 1, std::uint8_t> >(reportTestCases), "lns<4, 1>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot< lns<5, 1, std::uint8_t> >(reportTestCases), "lns<5, 1>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot< lns<6, 2, std::uint8_t> >(reportTestCases), "lns<6, 2>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot< lns<7, 2, std::uint8_t> >(reportTestCases), "lns<7, 2>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot< lns<8, 3, std::uint8_t> >(reportTestCases), "lns<8, 3>", "hypot");
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< lns<64, 2, std::uint32_t> >(reportTestCases, RandomsOp::OPCODE_HYPOT, 1000), "lns<64,2>", "hypot");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
