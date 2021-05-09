// logic.cpp: functional tests for logic tests on arbitrary reals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/verification/test_status.hpp>

namespace sw::universal {

	template<typename TestType>
	int VerifyBfloatLogicEqual() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setBits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setBits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Bfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Bfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Bfloat = true
					//	NaN == real : IEEE = false    Bfloat = false
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
	int VerifyBfloatLogicNotEqual() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setBits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setBits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Bfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Bfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Bfloat = true
					//	NaN == real : IEEE = false    Bfloat = false
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
	int VerifyBfloatLogicLessThan() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setBits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setBits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Bfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Bfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Bfloat = true
					//	NaN == real : IEEE = false    Bfloat = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small bfloat<>s, we can depend on the double conversion
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
	int VerifyBfloatLogicLessOrEqualThan() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setBits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setBits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Bfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Bfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Bfloat = true
					//	NaN == real : IEEE = false    Bfloat = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small bfloat<>s, we can depend on the double conversion
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
	int VerifyBfloatLogicGreaterThan() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setBits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setBits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Bfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Bfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Bfloat = true
					//	NaN == real : IEEE = false    Bfloat = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small bfloat<>s, we can depend on the double conversion
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
	int VerifyBfloatLogicGreaterOrEqualThan() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.setBits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.setBits(j);

				// set the golden reference

					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Bfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Bfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Bfloat = true
					//	NaN == real : IEEE = false    Bfloat = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small bfloat<>s, we can depend on the double conversion
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

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	// manual exhaustive test
	
	nrOfFailedTestCases = 0;

#else
	cout << "AREAL logic operator validation" << endl;

	//bool bReportIndividualTestCases = false;
	std::string tag = "Comparison failed: ";

	bfloat<16, 1> a;

	cout << "Logic: operator==()" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat< 4, 1> >(), "bfloat< 4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat< 5, 1> >(), "bfloat< 5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat< 6, 1> >(), "bfloat< 6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat< 7, 1> >(), "bfloat< 7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat< 8, 1> >(), "bfloat< 8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat< 9, 1> >(), "bfloat< 9,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat<10, 1> >(), "bfloat<10,1>", "==");

	if (!(a == 0)) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> == 0", "== int literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> == 0", "== int literal");
	}
	if (!(a == 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> == 0.0f", "== float literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> == 0.0f", "== float literal");
	}
	if (!(a == 0.0)) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> == 0.0", "== double literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> == 0.0", "== double literal");
	}
	if (!(a == 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> == 0.0l", "== long double literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> == 0.0l", "== long double literal");
	}
	
	cout << "Logic: operator!=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat< 4, 1> >(), "bfloat< 4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat< 5, 1> >(), "bfloat< 5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat< 6, 1> >(), "bfloat< 6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat< 7, 1> >(), "bfloat< 7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat< 8, 1> >(), "bfloat< 8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat< 9, 1> >(), "bfloat< 9,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat<10, 1> >(), "bfloat<10,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat<12, 1> >(), "bfloat<12,1>", "!=");

	if (a != 0) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> != 0", "!= int literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> != 0", "!= int literal");
	}
	if (a != 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> != 0.0f", "!= float literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> != 0.0f", "!= float literal");
	}
	if (a != 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> != 0.0", "!= double literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> != 0.0", "!= double literal");
	}
	if (a != 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> != 0.0l", "!= long double literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> != 0.0l", "!= long double literal");
	}

#ifdef AREAL_SUBTRACT_IS_IMPLEMENTED
	cout << "Logic: operator<()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 4, 1> >(), "bfloat< 4,1>", "<");
	return 0;
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 5, 1> >(), "bfloat< 5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 6, 1> >(), "bfloat< 6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 7, 1> >(), "bfloat< 7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 8, 1> >(), "bfloat< 8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 9, 1> >(), "bfloat< 9,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat<10, 1> >(), "bfloat<10,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat<12, 1> >(), "bfloat<12,1>", "<");

	if (a < 0) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> < 0", "< int literal");
	}
	if (a < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> < 0.0f", "< float literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> < 0.0f", "== float literal");
	}
	if (a < 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> < 0.0", "< double literal");
	}
	if (a < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> < 0.0l", "< long double literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> < 0.0l", "== long double literal");
	}

	cout << "Logic: operator<=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 4, 1> >(), "bfloat< 4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 5, 1> >(), "bfloat< 5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 6, 1> >(), "bfloat< 6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 7, 1> >(), "bfloat< 7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 8, 1> >(), "bfloat< 8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat< 9, 1> >(), "bfloat< 9,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat<10, 1> >(), "bfloat<10,1>", "<");
//	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicLessThan< bfloat<12, 1> >(), "bfloat<12,1>", "<");

	if (a < 0) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> < 0", "< int literal");
	}
	if (a < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> < 0.0f", "< float literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> < 0.0f", "== float literal");
	}
	if (a < 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> < 0.0", "< double literal");
	}
	if (a < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "bfloat<16,1> < 0.0l", "< long double literal");
	}
	else {
		ReportTestResult(0, "bfloat<16,1> < 0.0l", "== long double literal");
	}

#endif //	AREAL_SUBTRACT_IS_IMPLEMENTED

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat<12, 1> >(), "bfloat<12,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat<14, 1> >(), "bfloat<14,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicEqual< bfloat<16, 1> >(), "bfloat<16,1>", "==");

	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat<12, 1> >(), "bfloat<12,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat<14, 1> >(), "bfloat<14,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatLogicNotEqual< bfloat<16, 1> >(), "bfloat<16,1>", "!=");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_divide_by_zero& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
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
