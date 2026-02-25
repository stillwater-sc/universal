// conversion.cpp : test suite runner for conversion operators to arbitrary precision, fixed-size double-base logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <vector>
#include <algorithm>
#include <universal/number/dbns/dbns.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

		template<typename TestType, typename RefType>
		void ReportConversionError(const std::string& test_case, const std::string& op, double input, const TestType& result, RefType ref, const std::string& rounding) {
			constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
			auto old_precision = std::cerr.precision();
			std::cerr << std::setprecision(10);
			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(NUMBER_COLUMN_WIDTH) << input
				<< " did not convert to "
				<< std::setw(NUMBER_COLUMN_WIDTH) << ref << " instead it yielded  "
				<< std::setw(NUMBER_COLUMN_WIDTH) << double(result)
				<< "  encoding " << std::setw(nbits) << to_binary(result) << " converted from " << to_binary(ref) << " " << rounding;
			std::cerr << '\n';
			std::cerr << std::setprecision(old_precision);
		}

		template<typename TestType, typename RefType>
		void ReportConversionSuccess(const std::string& test_case, const std::string& op, double input, const TestType& result, RefType ref, const std::string& rounding) {
			constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
			auto old_precision = std::cerr.precision();
			std::cerr << std::setprecision(10); 
			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(NUMBER_COLUMN_WIDTH) << input
				<< " success            "
				<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
				<< std::setw(NUMBER_COLUMN_WIDTH) << ref
				<< "  encoding " << std::setw(nbits) << to_binary(result) << " converted from " << to_binary(ref) << " " << rounding;
			std::cerr << '\n';
			std::cerr << std::setprecision(old_precision);
		}

		template<typename TestType, typename RefType>
		int Compare(double input, const TestType& result, const RefType& ref, const std::string& rounding, bool reportTestCases) {
			int fail = 0;
			double dresult = double(result);
			double dref = double(ref);
			if (std::fabs(dresult - dref) > 0.000000001) {
				fail++;
				if (reportTestCases) ReportConversionError("FAIL", "=", input, result, ref, rounding);
			}
			else {
				//if (reportTestCases) ReportConversionSuccess("PASS", "=", input, result, ref, rounding);
			}
			return fail;
		}

		// enumerate all conversion cases for a posit configuration
		template<size_t nbits, size_t rbits, typename bt, auto... x>
		int VerifyConversion(bool reportTestCases) {
			// we are going to generate a test set that consists of all dbns configs and their midpoints
			// As DBNS is a logarithmic number system, we need to generate these midpoints in 
			// logarithmic space.
			// the values traced out by DBNS are wicket, jumping all over the place.
			// One idea is to set up the set, sort the values and than use those values
			// to generate the +- deltas.
			using TestType = dbns<nbits, rbits, bt, x...>;

			constexpr size_t max = nbits > 16 ? 16 : nbits;
			size_t NR_TEST_CASES = (size_t(1) << max);
			if constexpr (nbits > 16) {
				std::cout << "VerifyConversion: " << type_tag(TestType()) << " : NR_TEST_CASES = " << NR_TEST_CASES << " constrained due to nbits > 16" << std::endl;
			}

			int nrOfFailedTests = 0;
			TestType a;
			std::vector<TestType> dbnsBits;
			std::vector<double> samples;
			for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
				a.setbits(i);
				if (a.isnan()) continue;
				dbnsBits.push_back(a);
				samples.push_back(double(a));
			}
			std::cout << "size of samples is " << size(samples) << '\n';
			for (auto e : samples) {
				std::cout << std::setw(20) << e << " : " << to_binary(a = e) << '\n';
			}
			std::cout << '\n';
			std::sort(samples.begin(), samples.end());
			for (auto e : samples) {
				std::cout << std::setw(20) << e << " : " << to_binary(a = e) << '\n';
			}

			return nrOfFailedTests;
		}

		// enumerate all conversion cases for integers
		template<size_t nbits, size_t rbits, typename bt, auto... x>
		int VerifyIntegerConversion(bool reportTestCases) {
			using TestType = dbns<nbits, rbits, bt, x...>;

			// we generate numbers from 1 via maxpos to -1 and through the special case of 0 back to 1
			constexpr unsigned max = nbits > 20 ? 20 : nbits;
			size_t NR_TEST_CASES = (size_t(1) << (max - 1)) + 1;
			int nrOfFailedTestCases = 0;
			// special cases in case we are clipped by the nbits > 20
			long ref = 0x80000000;  // -2147483648
			TestType result(ref);
			if (ref != result) {
				std::cout << " FAIL long(" << ref << ") != long(" << result << ") : reference = -2147483648" << std::endl;
				nrOfFailedTestCases++;
			}
			TestType v(1);
			for (size_t i = 0; i < NR_TEST_CASES; ++i) {
				if (!v.isnan()) {
					ref = (long)v; // obtain the integer cast of this dbns
					result = ref;		// assign this integer to a dbns				
					if (ref != result) { // compare the integer cast to the reference dbns
						if (reportTestCases) std::cout << " FAIL long(" << v << ") != long(" << result << ") : reference = " << ref << std::endl;
						nrOfFailedTestCases++;
					}
					else {
						//if (reportTestCases) std::cout << " PASS " << v << " casts to " << result << " : reference = " << ref << std::endl;
					}
				}
				++v;
			}
			return nrOfFailedTestCases;
		}

} } // namespace sw::universal

template<typename TestType, typename Real>
void GenerateTestCase(double input, double reference, const TestType& result) {
	if (std::fabs(double(result) - reference) > 0.000000001)
		ReportConversionError("FAIL", "=", input, result, reference, std::string("faithful x = x"));
	else
		ReportConversionSuccess("PASS", "=", input, result, reference, std::string("faithful x = x"));
	std::cout << std::endl;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "dbns<> conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		nrOfFailedTestCases += VerifyConversion<5, 2, std::uint8_t>(true);
	}
	return 0;
	{
		using DBNS5_2 = dbns<5, 2, std::uint8_t>;
		DBNS5_2 minpos(SpecificValue::minpos);
		double mp = double(minpos);
		DBNS5_2 result = mp;
		GenerateTestCase<DBNS5_2, double>(mp, mp, result);
		double halfMinpos = mp / 2.0;
		result = halfMinpos;
		GenerateTestCase<DBNS5_2, double>(halfMinpos, 0.0, result);
		double quarterMinpos = halfMinpos / 2.0;
		result = quarterMinpos;
		GenerateTestCase<DBNS5_2, double>(quarterMinpos, 0.0, result);
		double threeQuarterMinpos = halfMinpos + quarterMinpos;
		result = threeQuarterMinpos;
		GenerateTestCase<DBNS5_2, double>(threeQuarterMinpos, mp, result);

		using DBNS6_3 = dbns<6, 3, std::uint8_t>;
		DBNS6_3 ref;
		ref.setbits(17);
		std::cout << to_binary(ref) << " : " << ref << '\n';
		double input = double(ref);
		result = input;
		std::cout << to_binary(ref) << " : " << ref << " -> " << result << " : " << to_binary(result) << '\n';
		GenerateTestCase<DBNS5_2, double>(input, double(DBNS5_2(SpecificValue::minpos)), result);
	}

	{
		using DBNS5_2 = dbns<5, 2, std::uint8_t>;
		using DBNS6_3 = dbns<6, 3, std::uint8_t>;

		constexpr size_t NR_SAMPLES = 32;
		DBNS5_2 a;
		DBNS6_3 b;
		for (size_t i = 0; i < NR_SAMPLES; ++i) {
			b.setbits(i);
			if (i % 2 == 0) {
				a.setbits(i / 2);
				std::cout << to_binary(b) << " : " << std::setw(10) << b << " - " << std::setw(10) << a << " : " << to_binary(a) << '\n';
			}
			else {
				std::cout << to_binary(b) << " : " << std::setw(10) << b << '\n';
			}
		}
	}

	nrOfFailedTestCases += VerifyConversion<5, 2, std::uint8_t>(true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return 0; 
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<4, 1, std::uint8_t>(true), "dbns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<5, 2, std::uint8_t>(true), "dbns<5,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1, std::uint8_t>(true), "dbns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2, std::uint8_t>(true), "dbns<5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<6, 3, std::uint8_t>(true), "dbns<6,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1, std::uint8_t>(true), "dbns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2, std::uint8_t>(true), "dbns<5,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 1, std::uint8_t>(reportTestCases), "dbns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 2, std::uint8_t>(reportTestCases), "dbns<4,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 2, std::uint8_t>(reportTestCases), "dbns<6,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 3, std::uint8_t>(reportTestCases), "dbns<6,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 4, std::uint8_t>(reportTestCases), "dbns<6,4>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 2, std::uint8_t>(reportTestCases), "dbns<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 3, std::uint8_t>(reportTestCases), "dbns<8,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 4, std::uint8_t>(reportTestCases), "dbns<8,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 5, std::uint8_t>(reportTestCases), "dbns<8,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 6, std::uint8_t>(reportTestCases), "dbns<8,6>", test_tag);

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 3, std::uint8_t>(reportTestCases), "dbns<10,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 4, std::uint8_t>(reportTestCases), "dbns<10,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 5, std::uint8_t>(reportTestCases), "dbns<10,5>", test_tag);
#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

/*
Generate Value table for an dbns<6,3> in TXT format

 */
