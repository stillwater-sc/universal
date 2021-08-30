// logic.cpp: functional tests for logic tests on classic cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/verification/test_status.hpp>

namespace sw::universal {

	template<typename TestType>
	int VerifyCfloatLogicEqual() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
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
					//	NaN == NaN  : IEEE = true    cfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    cfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    cfloat = true
					//	NaN == real : IEEE = false    cfloat = false
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
	int VerifyCfloatLogicNotEqual() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
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
					//	NaN == NaN  : IEEE = true    cfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    cfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    cfloat = true
					//	NaN == real : IEEE = false    cfloat = false
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
	int VerifyCfloatLogicLessThan() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
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
					//	NaN == NaN  : IEEE = true    cfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    cfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    cfloat = true
					//	NaN == real : IEEE = false    cfloat = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small cfloat<>s, we can depend on the double conversion
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
	int VerifyCfloatLogicLessOrEqualThan() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
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
					//	NaN == NaN  : IEEE = true    cfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    cfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    cfloat = true
					//	NaN == real : IEEE = false    cfloat = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small cfloat<>s, we can depend on the double conversion
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
	int VerifyCfloatLogicGreaterThan() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
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
					//	NaN == NaN  : IEEE = true    cfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    cfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    cfloat = true
					//	NaN == real : IEEE = false    cfloat = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small cfloat<>s, we can depend on the double conversion
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
	int VerifyCfloatLogicGreaterOrEqualThan() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
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
					//	NaN == NaN  : IEEE = true    cfloat = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    cfloat = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    cfloat = true
					//	NaN == real : IEEE = false    cfloat = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small cfloat<>s, we can depend on the double conversion
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
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	// manual exhaustive test
	
	nrOfFailedTestCases = 0;

#else
	std::cout << "classic floating-point logic operator validation\n";

	//bool bReportIndividualTestCases = false;
	std::string tag = "Comparison failed: ";

	cfloat<16, 1> a;

	std::cout << "Logic: operator==()\n";

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat< 4, 1> >(), "cfloat< 4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat< 5, 1> >(), "cfloat< 5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat< 6, 1> >(), "cfloat< 6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat< 7, 1> >(), "cfloat< 7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat< 8, 1> >(), "cfloat< 8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat< 9, 1> >(), "cfloat< 9,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat<10, 1> >(), "cfloat<10,1>", "==");

	if (!(a == 0)) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> == 0", "== int literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> == 0", "== int literal");
	}
	if (!(a == 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> == 0.0f", "== float literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> == 0.0f", "== float literal");
	}
	if (!(a == 0.0)) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> == 0.0", "== double literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> == 0.0", "== double literal");
	}
	if (!(a == 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> == 0.0l", "== long double literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> == 0.0l", "== long double literal");
	}
	
	std::cout << "Logic: operator!=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat< 4, 1> >(), "cfloat< 4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat< 5, 1> >(), "cfloat< 5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat< 6, 1> >(), "cfloat< 6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat< 7, 1> >(), "cfloat< 7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat< 8, 1> >(), "cfloat< 8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat< 9, 1> >(), "cfloat< 9,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat<10, 1> >(), "cfloat<10,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat<12, 1> >(), "cfloat<12,1>", "!=");

	if (a != 0) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> != 0", "!= int literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> != 0", "!= int literal");
	}
	if (a != 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> != 0.0f", "!= float literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> != 0.0f", "!= float literal");
	}
	if (a != 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> != 0.0", "!= double literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> != 0.0", "!= double literal");
	}
	if (a != 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> != 0.0l", "!= long double literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> != 0.0l", "!= long double literal");
	}

#ifdef AREAL_SUBTRACT_IS_IMPLEMENTED
	cout << "Logic: operator<()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 4, 1> >(), "cfloat< 4,1>", "<");
	return 0;
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 5, 1> >(), "cfloat< 5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 6, 1> >(), "cfloat< 6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 7, 1> >(), "cfloat< 7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 8, 1> >(), "cfloat< 8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 9, 1> >(), "cfloat< 9,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat<10, 1> >(), "cfloat<10,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat<12, 1> >(), "cfloat<12,1>", "<");

	if (a < 0) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> < 0", "< int literal");
	}
	if (a < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> < 0.0f", "< float literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> < 0.0f", "== float literal");
	}
	if (a < 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> < 0.0", "< double literal");
	}
	if (a < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> < 0.0l", "< long double literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> < 0.0l", "== long double literal");
	}

	cout << "Logic: operator<=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 4, 1> >(), "cfloat< 4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 5, 1> >(), "cfloat< 5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 6, 1> >(), "cfloat< 6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 7, 1> >(), "cfloat< 7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 8, 1> >(), "cfloat< 8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat< 9, 1> >(), "cfloat< 9,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat<10, 1> >(), "cfloat<10,1>", "<");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicLessThan< cfloat<12, 1> >(), "cfloat<12,1>", "<");

	if (a < 0) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> < 0", "< int literal");
	}
	if (a < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> < 0.0f", "< float literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> < 0.0f", "== float literal");
	}
	if (a < 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> < 0.0", "< double literal");
	}
	if (a < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "cfloat<16,1> < 0.0l", "< long double literal");
	}
	else {
		ReportTestResult(0, "cfloat<16,1> < 0.0l", "== long double literal");
	}

#endif //	AREAL_SUBTRACT_IS_IMPLEMENTED

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat<12, 1> >(), "cfloat<12,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat<14, 1> >(), "cfloat<14,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicEqual< cfloat<16, 1> >(), "cfloat<16,1>", "==");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat<12, 1> >(), "cfloat<12,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat<14, 1> >(), "cfloat<14,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatLogicNotEqual< cfloat<16, 1> >(), "cfloat<16,1>", "!=");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_divide_by_zero& err) {
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
