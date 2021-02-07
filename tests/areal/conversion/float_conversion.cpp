// float_conversion.cpp: test suite runner for IEEE float conversions to areals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
// Configure the areal template environment
// first: enable general or specialized configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

// minimum set of include files to reflect source code dependencies
#include <universal/number/areal/areal.hpp>
#include <universal/number/areal/manipulators.hpp>
#include <universal/number/areal/math_functions.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/number/areal/table.hpp> // only used for value table generation

namespace sw::universal {

	template<typename SrcType, typename TestType>
	void ReportIntervalConversionError(const std::string& test_case, const std::string& op, SrcType input, const TestType& reference, const TestType& result) {
		constexpr size_t nbits = result.nbits;  // number system concept requires a static member indicating its size in bits
		auto old_precision = std::cerr.precision();
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(NUMBER_COLUMN_WIDTH) << input
			<< " did not convert to "
			<< std::setw(NUMBER_COLUMN_WIDTH) << reference << " instead it yielded  "
			<< std::setw(NUMBER_COLUMN_WIDTH) << result
			<< "  raw " << to_binary(result)
			<< std::setprecision(old_precision)
			<< std::endl;
	}

	template<typename SrcType, typename TestType>
	void ReportIntervalConversionSuccess(const std::string& test_case, const std::string& op, SrcType input, const TestType& reference, const TestType& result) {
		constexpr size_t nbits = result.nbits;  // number system concept requires a static member indicating its size in bits
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(NUMBER_COLUMN_WIDTH) << input
			<< " success            "
			<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
			<< std::setw(NUMBER_COLUMN_WIDTH) << reference
			<< "  raw " << std::setw(nbits) << to_binary(result)
			<< std::endl;
	}

	template<typename SrcType, typename TestType>
	int Compare(SrcType input, const TestType& testValue, const TestType& reference, bool bReportIndividualTestCases) {
		int fail = 0;
		if (testValue != reference) {
			fail++;
			if (bReportIndividualTestCases)	ReportIntervalConversionError("FAIL", "=", input, reference, testValue);
		}
		else {
			//if (bReportIndividualTestCases) ReportIntervalConversionSuccess("PASS", "=", input, reference, testValue);
		}
		return fail;
	}

	/// <summary>
	/// enumerate all conversion cases for a number system with ubits
	/// </summary>
	/// <typeparam name="TestType">the test configuration</typeparam>
	/// <typeparam name="SrcType">the source type to convert from</typeparam>
	/// <param name="tag">string to indicate what is being tested</param>
	/// <param name="bReportIndividualTestCases">if true print results of each test case. Default is false.</param>
	/// <returns>number of failed test cases</returns>
	template<typename TestType, typename SrcType>
	int VerifyArealIntervalConversion(const std::string& tag, bool bReportIndividualTestCases) {
		// areal<> is organized as a set of exact samples followed by an interval to the next exact value
		//
		// vprev    exact value          ######-0     ubit = false     some value [vprev,vprev]
		//          interval value       ######-1     ubit = true      (vprev, v)
		// v        exact value          ######-0     ubit = false     some value [v,v]
		//          interval value       ######-1     ubit = true      (v, vnext)
		// vnext    exact value          ######-0     ubit = false     some value [vnext,vnext]
		//          interval value       ######-1     ubit = true      (vnext, vnextnext)
		//
		// the assignment test can thus be constructed by enumerating the exact values of a configuration
		// and taking a -diff to obtain the interval value of vprev, 
		// and taking a +diff to obtain the interval value of v
		constexpr size_t nbits = TestType::nbits;
		constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);

		const unsigned max = nbits > 20 ? 20 : nbits + 1;
		size_t max_tests = (size_t(1) << max);
		if (max_tests < NR_TEST_CASES) {
			std::cout << "VerifyArealIntervalConversion " << typeid(TestType).name() << ": NR_TEST_CASES = " << NR_TEST_CASES << " clipped by " << max_tests << std::endl;
		}

		// execute the test
		int nrOfFailedTests = 0;
		TestType positive_minimum;
		SrcType dminpos = SrcType(minpos(positive_minimum));
		TestType negative_maximum;
		SrcType dmaxneg = SrcType(maxneg(negative_maximum));

		// NUT: number under test
		TestType nut;
		TestType x;
		x.set_raw_bits(0x0FC);

		for (size_t i = 0; i < NR_TEST_CASES && i < max_tests; i += 2) {
			TestType current, interval;
			SrcType testValue{ 0.0 };
			current.set_raw_bits(i);
			interval.set_raw_bits(i + 1);  // sets the ubit
			SrcType da = SrcType(current);
//			std::cout << "current : " << to_binary(current) << " : " << current << std::endl;
//			std::cout << "interval: " << to_binary(interval) << " : " << interval << std::endl;
			// da - delta = (prev, current)
			// da         = [current, current]
			// da + delta = (current, next)

			// b00'1111'1100
//			if (current == x) {
//				std::cout << "[1] " << x << std::endl;
//			}

			if (current.iszero()) {
				SrcType delta = dminpos / 4.0;  // the test value between 0 and minpos
				if (current.sign()) {
					// da         = [-0]
					testValue = da;
					nut = testValue;
					int zeroCmp = Compare(testValue, nut, current, bReportIndividualTestCases);
					if (zeroCmp == 1) {  // working around optimizing compilers ignoring sign on 0
						if (!nut.iszero()) {
							nrOfFailedTests += 1;
						}
					}
					// da - delta = (-0,-minpos)
					testValue = da - delta;
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, interval, bReportIndividualTestCases);
				}
				else {
					// da         = [0]
					testValue = da;
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, current, bReportIndividualTestCases);
					// da + delta = (0,minpos)
					testValue = da + delta;
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, interval, bReportIndividualTestCases);
				}
			}
			else if (current.isinf(INF_TYPE_NEGATIVE)) {

			}
			else if (current.isinf(INF_TYPE_POSITIVE)) {

			}
			else if (current.isinf(NAN_TYPE_SIGNALLING)) {  // sign is true

			}
			else if (current.isinf(NAN_TYPE_QUIET)) {       // sign is false

			}
			else {
				TestType previous, previousInterval;
				previous.set_raw_bits(i - 2);
				previousInterval.set_raw_bits(i - 1);
//				std::cout << "previous: " << to_binary(previous) << " : " << previous << std::endl;
//				std::cout << "interval: " << to_binary(previousInterval) << " : " << previousInterval << std::endl;
				SrcType delta = (da - SrcType(previous)) / 2.0;  // NOTE: the sign will flip the relationship between the enumeration and the values
//				std::cout << "delta   : " << delta << " : " << to_binary(delta, true) << std::endl;
															   // da - delta = (prev,current) == previous + ubit
				testValue = da - delta;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, previousInterval, bReportIndividualTestCases);
				// da         = [v]
				testValue = da;
//				if (testValue == 1.0) {
//					std::cout << "test value: " << testValue << std::endl;
//				}
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, current, bReportIndividualTestCases);
				// da + delta = (v+,next) == current + ubit
				testValue = da + delta;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, interval, bReportIndividualTestCases);
			}
		}
		return nrOfFailedTests;
	}

} // namespace sw::universal

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::string tag = "conversion: ";

#if MANUAL_TESTING

	bool bReportIndividualTestCases = false;

	// areal<> is organized as a set of exact samples and an interval to the next exact value
	//
	// vprev    exact value          ######-0     ubit = false     some value [vprev,vprev]
	//          interval value       ######-1     ubit = true      (vprev, v)
	// v        exact value          ######-0     ubit = false     some value [v,v]
	//          interval value       ######-1     ubit = true      (v, vnext)
	// vnext    exact value          ######-0     ubit = false     some value [vnext,vnext]
	//          interval value       ######-1     ubit = true      (vnext, vnextnext)
	//
	// the assignment test can thus be constructed by enumerating the exact values
	// and taking a -diff to obtain the interval value of vprev, 
	// and taking a +diff to obtain the interval value of v

//	GenerateArealTable<9, 6>(cout, true);  // ok
//	GenerateArealTable<10, 7>(cout, true); // fails

	areal<10, 7> a;
	a.set_raw_bits(0x1F6);  // b01'1111'0110;
	cout << to_binary(a) << " : " << a << endl;
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 7, uint8_t>, float >(tag, true), tag, "areal<10,7,uint8_t>");
//	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 7, uint16_t>, float >(tag, true), tag, "areal<10,7,uint16_t>");

	cout << "failed tests: " << nrOfFailedTestCases << endl;

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	cout << "AREAL conversion validation" << endl;

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<4, 1>, float >(tag, bReportIndividualTestCases), tag, "areal<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<5, 1>, float >(tag, bReportIndividualTestCases), tag, "areal<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 1>, float >(tag, true), tag, "areal<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 1>, float >(tag, bReportIndividualTestCases), tag, "areal<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 1>, float >(tag, bReportIndividualTestCases), tag, "areal<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<9, 1>, float >(tag, bReportIndividualTestCases), tag, "areal<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 1>, float >(tag, bReportIndividualTestCases), tag, "areal<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 1>, float >(tag, bReportIndividualTestCases), tag, "areal<12,1>");


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<5, 2>, float >(tag, bReportIndividualTestCases), tag, "areal<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 2>, float >(tag, bReportIndividualTestCases), tag, "areal<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 2>, float >(tag, bReportIndividualTestCases), tag, "areal<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 2>, float >(tag, bReportIndividualTestCases), tag, "areal<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 2>, float >(tag, bReportIndividualTestCases), tag, "areal<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 2>, float >(tag, bReportIndividualTestCases), tag, "areal<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 2>, float >(tag, bReportIndividualTestCases), tag, "areal<14,2>");


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 3>, float >(tag, bReportIndividualTestCases), tag, "areal<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 3>, float >(tag, bReportIndividualTestCases), tag, "areal<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 3>, float >(tag, bReportIndividualTestCases), tag, "areal<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 3>, float >(tag, bReportIndividualTestCases), tag, "areal<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 3>, float >(tag, bReportIndividualTestCases), tag, "areal<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 3>, float >(tag, bReportIndividualTestCases), tag, "areal<14,3>");


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 4>, float >(tag, bReportIndividualTestCases), tag, "areal<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 4>, float >(tag, bReportIndividualTestCases), tag, "areal<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 4>, float >(tag, bReportIndividualTestCases), tag, "areal<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 4>, float >(tag, bReportIndividualTestCases), tag, "areal<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 4>, float >(tag, bReportIndividualTestCases), tag, "areal<14,4>");


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 5>, float >(tag, bReportIndividualTestCases), tag, "areal<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 5>, float >(tag, bReportIndividualTestCases), tag, "areal<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 5>, float >(tag, bReportIndividualTestCases), tag, "areal<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 5>, float >(tag, bReportIndividualTestCases), tag, "areal<14,5>");


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<9, 6>, float >(tag, bReportIndividualTestCases), tag, "areal<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 6>, float >(tag, bReportIndividualTestCases), tag, "areal<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 6>, float >(tag, bReportIndividualTestCases), tag, "areal<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 6>, float >(tag, bReportIndividualTestCases), tag, "areal<14,6>");

#if LATER
	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 7>, float >(tag, true), tag, "areal<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 7>, float >(tag, bReportIndividualTestCases), tag, "areal<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 7>, float >(tag, bReportIndividualTestCases), tag, "areal<14,7>");


	// es = 8
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<11, 8>, float >(tag, bReportIndividualTestCases), tag, "areal<11,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 8>, float >(tag, bReportIndividualTestCases), tag, "areal<12,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 8>, float >(tag, bReportIndividualTestCases), tag, "areal<14,8>");
#endif // LATER

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_arithmetic_exception& err) {
	std::cerr << "Uncaught areal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_internal_exception& err) {
	std::cerr << "Uncaught areal internal exception: " << err.what() << std::endl;
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


/*

  To generate:
  	GenerateFixedPointComparisonTable<4, 0>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 1>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 2>(std::string("-"));
	

 */
