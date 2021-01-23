// logic.cpp: functional tests for logic tests on arbitrary reals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#include <universal/areal/areal.hpp>
#include <universal/verification/test_status.hpp>

namespace sw::universal {

	template<typename TestType>
	int VerifyArealLogicEqual() {
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.set_raw_bits(j);

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
		constexpr size_t max = TestType::nbits > 16 ? 16 : TestType::nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.set_raw_bits(j);

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

} // namespace sw::universal

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	// manual exhaustive test
	
	nrOfFailedTestCases = 0;

#else
	cout << "AREAL logic operator validation" << endl;

	bool bReportIndividualTestCases = false;
	std::string tag = "Comparison failed: ";

	areal<16, 1> a;

	cout << "Logic: operator==()" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 4, 1> >(), "areal< 4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 5, 1> >(), "areal< 5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 6, 1> >(), "areal< 6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 7, 1> >(), "areal< 7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 8, 1> >(), "areal< 8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal< 9, 1> >(), "areal< 9,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal<10, 1> >(), "areal<10,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal<12, 1> >(), "areal<12,1>", "==");

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
	
	cout << "Logic: operator!=()" << endl;

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
		ReportTestResult(0, "areal<16,1> != 0.0f", "== float literal");
	}
	if (a != 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> != 0.0", "!= double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> != 0.0", "== double literal");
	}
	if (a != 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "areal<16,1> != 0.0l", "!= long double literal");
	}
	else {
		ReportTestResult(0, "areal<16,1> != 0.0l", "== long double literal");
	}
#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal<14, 1> >(), "areal<14,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyArealLogicEqual< areal<16, 1> >(), "areal<16,1>", "==");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_divide_by_zero& err) {
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
