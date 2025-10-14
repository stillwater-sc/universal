// logic.cpp: test suite runner for logic tests on abstract reals represented by blocktriples
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp>

namespace sw::universal {

	template<typename TestType>
	int VerifyBlocktripleLogicEqual() {
		constexpr size_t max = TestType::fbits > 16 ? 16 : TestType::fbits;
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
					//	NaN == NaN  : IEEE = true    Blocktriple = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Blocktriple = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Blocktriple = true
					//	NaN == real : IEEE = false    Blocktriple = false
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
	int VerifyBlocktripleLogicNotEqual() {
		constexpr size_t max = TestType::fbits > 16 ? 16 : TestType::fbits;
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
					//	NaN == NaN  : IEEE = true    Blocktriple = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Blocktriple = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Blocktriple = true
					//	NaN == real : IEEE = false    Blocktriple = false
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
	int VerifyBlocktripleLogicLessThan() {
		constexpr size_t max = TestType::fbits > 16 ? 16 : TestType::fbits;
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
					//	NaN == NaN  : IEEE = true    Blocktriple = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Blocktriple = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Blocktriple = true
					//	NaN == real : IEEE = false    Blocktriple = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small blocktriple<>s, we can depend on the double conversion
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
	int VerifyBlocktripleLogicLessOrEqualThan() {
		constexpr size_t max = TestType::fbits > 16 ? 16 : TestType::fbits;
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
					//	NaN == NaN  : IEEE = true    Blocktriple = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Blocktriple = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Blocktriple = true
					//	NaN == real : IEEE = false    Blocktriple = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small blocktriple<>s, we can depend on the double conversion
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
	int VerifyBlocktripleLogicGreaterThan() {
		constexpr size_t max = TestType::fbits > 16 ? 16 : TestType::fbits;
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
					//	NaN == NaN  : IEEE = true    Blocktriple = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Blocktriple = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Blocktriple = true
					//	NaN == real : IEEE = false    Blocktriple = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small blocktriple<>s, we can depend on the double conversion
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
	int VerifyBlocktripleLogicGreaterOrEqualThan() {
		constexpr size_t max = TestType::fbits > 16 ? 16 : TestType::fbits;
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
					//	NaN == NaN  : IEEE = true    Blocktriple = true  because we have unique encodings for +-NaN
					//	NaN == real : IEEE = true    Blocktriple = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Blocktriple = true
					//	NaN == real : IEEE = false    Blocktriple = false
					// and thus we can't rely on IEEE float as reference

				// since this function is only useful for small blocktriple<>s, we can depend on the double conversion
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

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	// manual exhaustive test
	
	nrOfFailedTestCases = 0;

#else
	std::cout << "BLOCKTRIPLE logic operator validation\n";

	//bool bReportIndividualTestCases = false;
	std::string tag = "Comparison failed: ";

	std::cout << "Logic: operator==()\n";

	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple< 4> >(), "blocktriple< 4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple< 5> >(), "blocktriple< 5>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple< 6> >(), "blocktriple< 6>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple< 7> >(), "blocktriple< 7>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple< 8> >(), "blocktriple< 8>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple< 9> >(), "blocktriple< 9>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple<10> >(), "blocktriple<10>", "==");

	
	std::cout << "Logic: operator!=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple< 4> >(), "blocktriple< 4>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple< 5> >(), "blocktriple< 5>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple< 6> >(), "blocktriple< 6>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple< 7> >(), "blocktriple< 7>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple< 8> >(), "blocktriple< 8>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple< 9> >(), "blocktriple< 9>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple<10> >(), "blocktriple<10>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple<12> >(), "blocktriple<12>", "!=");


#ifdef BLOCKTRIPLE_SUBTRACT_IS_IMPLEMENTED
	std::cout << "Logic: operator<()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 4> >(), "blocktriple< 4>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 5> >(), "blocktriple< 5>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 6> >(), "blocktriple< 6>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 7> >(), "blocktriple< 7>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 8> >(), "blocktriple< 8>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 9> >(), "blocktriple< 9>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple<10> >(), "blocktriple<10>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple<12> >(), "blocktriple<12>", "<");


	std::cout << "Logic: operator<=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 4> >(), "blocktriple< 4>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 5> >(), "blocktriple< 5>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 6> >(), "blocktriple< 6>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 7> >(), "blocktriple< 7>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 8> >(), "blocktriple< 8>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple< 9> >(), "blocktriple< 9>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple<10> >(), "blocktriple<10>", "<");
//	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicLessThan< blocktriple<12> >(), "blocktriple<12>", "<");


#endif //	BLOCKTRIPLE_SUBTRACT_IS_IMPLEMENTED

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple<12> >(), "blocktriple<12>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple<14> >(), "blocktriple<14>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicEqual< blocktriple<16> >(), "blocktriple<16>", "==");

	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple<12> >(), "blocktriple<12>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple<14> >(), "blocktriple<14>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleLogicNotEqual< blocktriple<16> >(), "blocktriple<16>", "!=");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
