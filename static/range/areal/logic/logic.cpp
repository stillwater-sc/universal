// logic.cpp: functional tests for logic tests on arbitrary reals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw::universal {

	template<typename TestType>
	int VerifyArealLogicEqual() {
		constexpr unsigned max = TestType::nbits > 16 ? 16 : TestType::nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setbits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Areal = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Areal = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Areal = true
					//	NaN == real : IEEE = false    Areal = false
					// and thus we can't rely on IEEE float as reference

				// instead, use the bit pattern as reference
				bool ref = (i == j);

				bool result = (a == b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " != " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename TestType>
	int VerifyArealLogicNotEqual() {
		constexpr unsigned max = TestType::nbits > 16 ? 16 : TestType::nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setbits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Areal = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Areal = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Areal = true
					//	NaN == real : IEEE = false    Areal = false
					// and thus we can't rely on IEEE float as reference

				// instead, use the bit pattern as reference
				bool ref = (i != j);

				bool result = (a != b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " != " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename TestType>
	int VerifyArealLogicLessThan() {
		constexpr unsigned max = TestType::nbits > 16 ? 16 : TestType::nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setbits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Areal = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Areal = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Areal = true
					//	NaN == real : IEEE = false    Areal = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small areal<>s, we can depend on the double conversion
				bool ref = (double(a) < double(b));

				bool result = (a < b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename TestType>
	int VerifyArealLogicLessOrEqualThan() {
		constexpr unsigned max = TestType::nbits > 16 ? 16 : TestType::nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setbits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Areal = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Areal = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Areal = true
					//	NaN == real : IEEE = false    Areal = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small areal<>s, we can depend on the double conversion
				bool ref = (double(a) <= double(b));

				bool result = (a <= b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename TestType>
	int VerifyArealLogicGreaterThan() {
		constexpr unsigned max = TestType::nbits > 16 ? 16 : TestType::nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setbits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Areal = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Areal = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Areal = true
					//	NaN == real : IEEE = false    Areal = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small areal<>s, we can depend on the double conversion
				bool ref = (double(a) > double(b));

				bool result = (a > b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename TestType>
	int VerifyArealLogicGreaterOrEqualThan() {
		constexpr unsigned max = TestType::nbits > 16 ? 16 : TestType::nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setbits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Areal = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Areal = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Areal = true
					//	NaN == real : IEEE = false    Areal = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small areal<>s, we can depend on the double conversion
				bool ref = (double(a) >= double(b));

				bool result = (a >= b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

} // namespace sw::universal

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

	std::string test_suite  = "areal logic functions verification";
	std::string test_tag    = "logic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	// manual exhaustive test
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

	areal<16, 1> a;

	std::cout << "Logic: operator==()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 4, 1> >(), "areal< 4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 5, 1> >(), "areal< 5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 6, 1> >(), "areal< 6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 7, 1> >(), "areal< 7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 8, 1> >(), "areal< 8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 9, 1> >(), "areal< 9,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal<10, 1> >(), "areal<10,1>", "==");

	if (!(a == 0)) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> == 0", "== int literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> == 0", "== int literal");
	}
	if (!(a == 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> == 0.0f", "== float literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> == 0.0f", "== float literal");
	}
	if (!(a == 0.0)) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> == 0.0", "== double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> == 0.0", "== double literal");
	}
	if (!(a == 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> == 0.0l", "== long double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> == 0.0l", "== long double literal");
	}
	
	std::cout << "Logic: operator!=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal< 4, 1> >(), "areal< 4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal< 5, 1> >(), "areal< 5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal< 6, 1> >(), "areal< 6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal< 7, 1> >(), "areal< 7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal< 8, 1> >(), "areal< 8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal< 9, 1> >(), "areal< 9,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal<10, 1> >(), "areal<10,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal<12, 1> >(), "areal<12,1>", "!=");

	if (a != 0) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> != 0", "!= int literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> != 0", "!= int literal");
	}
	if (a != 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> != 0.0f", "!= float literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> != 0.0f", "!= float literal");
	}
	if (a != 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> != 0.0", "!= double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> != 0.0", "!= double literal");
	}
	if (a != 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> != 0.0l", "!= long double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> != 0.0l", "!= long double literal");
	}

#ifdef AREAL_SUBTRACT_IS_IMPLEMENTED
	std::cout << "Logic: operator<()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 4, 1> >(), "areal< 4,1>", "<");
	return 0;
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 5, 1> >(), "areal< 5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 6, 1> >(), "areal< 6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 7, 1> >(), "areal< 7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 8, 1> >(), "areal< 8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 9, 1> >(), "areal< 9,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal<10, 1> >(), "areal<10,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal<12, 1> >(), "areal<12,1>", "<");

	if (a < 0) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> < 0", "< int literal");
	}
	if (a < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> < 0.0f", "< float literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> < 0.0f", "== float literal");
	}
	if (a < 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> < 0.0", "< double literal");
	}
	if (a < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> < 0.0l", "< long double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> < 0.0l", "== long double literal");
	}

	std::cout << "Logic: operator<=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 4, 1> >(), "areal< 4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 5, 1> >(), "areal< 5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 6, 1> >(), "areal< 6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 7, 1> >(), "areal< 7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 8, 1> >(), "areal< 8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal< 9, 1> >(), "areal< 9,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal<10, 1> >(), "areal<10,1>", "<");
//	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicLessThan< areal<12, 1> >(), "areal<12,1>", "<");

	if (a < 0) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> < 0", "< int literal");
	}
	if (a < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> < 0.0f", "< float literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> < 0.0f", "== float literal");
	}
	if (a < 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> < 0.0", "< double literal");
	}
	if (a < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> < 0.0l", "< long double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> < 0.0l", "== long double literal");
	}

#endif //	AREAL_SUBTRACT_IS_IMPLEMENTED

#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal<12, 1> >(), "areal<12,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal<14, 1> >(), "areal<14,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal<16, 1> >(), "areal<16,1>", "==");

	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal<12, 1> >(), "areal<12,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal<14, 1> >(), "areal<14,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicNotEqual< areal<16, 1> >(), "areal<16,1>", "!=");
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
