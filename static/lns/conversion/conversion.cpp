// conversion.cpp : test suite runner for conversion operators to logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
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
			// we are going to generate a test set that consists of all lns configs and their midpoints
			// we do this by enumerating an lns that is 1-bit larger than the test configuration
			// These larger posits will be at the mid-point between the smaller sample values
			// and we'll enumerate the exact value, and a perturbation smaller and a perturbation larger
			// to test the rounding logic of the conversion.
			using TestType = lns<nbits, rbits, bt, x...>;
			using ContainingType = lns<nbits + 1, rbits + 1, bt, x...>;

			constexpr size_t max = nbits > 16 ? 16 : nbits;
			size_t NR_TEST_CASES = (size_t(1) << (max + 1));
			size_t QUARTER = (size_t(1) << (max - 1));
			size_t HALF = (size_t(1) << max);

			if constexpr (nbits > 16) {
				std::cout << "VerifyConversion: " << type_tag(TestType()) << " : NR_TEST_CASES = " << NR_TEST_CASES << " constrained due to nbits > 16" << std::endl;
			}

//			ContainingType halfMinpos(SpecificValue::minpos);  // in the more precise type
//			double dhalfMinpos = double(halfMinpos);
//			std::cerr << "half minpos : " << halfMinpos << " : " << to_binary(halfMinpos) << '\n';
	
			// execute the test
			int nrOfFailedTests = 0;
			for (size_t i = 0; i < NR_TEST_CASES; i++) {
				ContainingType ref, prev, next;
				// std::cerr << "i : " << i << '\n';
				ref.setbits(i);
				double da = double(ref);
				double eps = da * 1.0e-6; //  (da > 0 ? da * 1.0e-6 : da * -1.0e-6);
				double input;
				TestType a;
				if (i % 2) {
					if (i == QUARTER - 1) {
						if (reportTestCases) std::cerr << " odd-1: special case of project to maxpos\n";
						input = da - eps;
						a = input;
						prev.setbits(i - 1);
						nrOfFailedTests += Compare(input, a, prev, "round down to maxpos", reportTestCases);
						input = da + eps;
						a = input;
						nrOfFailedTests += Compare(input, a, prev, "project down to maxpos", reportTestCases);
					}
					else if (i == HALF - 1) {
						if (reportTestCases) std::cerr << " odd-2: special case of project to 1.0\n";
						input = da - eps;
						a = input;
						prev.setbits(i - 1);
						nrOfFailedTests += Compare(input, a, prev, "round down to 1.0", reportTestCases);
						input = da + eps;
						a = input;
						next.setbits(0);   // encoding of 1.0
						nrOfFailedTests += Compare(input, a, next, "round up to 1.0", reportTestCases);
					}
					else if (i == NR_TEST_CASES - 1) {
						if (reportTestCases) std::cerr << " odd-3: special case of project to -1.0\n";
						input = da - eps;
						a = input;
						prev.setbits(i - 1);
						nrOfFailedTests += Compare(input, a, prev, "round down to -1.0", reportTestCases);
						input = da + eps;
						a = input;
						next.setbits(0);
						next.setsign();       // encoding of -1.0
						nrOfFailedTests += Compare(input, a, next, "round up to -1.0", reportTestCases);
					}
					else {
						// for odd values, we are between lns values, so we create the round-up and round-down cases
						// std::cerr << " odd-4: between value case\n";
						// round-down
						input = da - eps;
						a = input;
						prev.setbits(i - 1);
						//next.setbits(i + 1);
						//std::cout << "da       : " << da << '\n';
						//std::cout << "eps      : " << eps << '\n';
						//std::cout << "input    : " << input << '\n';
						//std::cout << "previous : " << to_binary(prev) << " : " << prev << '\n';
						//std::cout << "midpoint : " << to_binary(ref) << " : " << ref << '\n';
						//std::cout << "next     : " << to_binary(next) << " : " << next << '\n';
						//std::cout << "sample   : " << to_binary(a) << " : " << a << '\n';
						//std::cout << input << " : " << float(ref) << " : " << float(next) << '\n';
						nrOfFailedTests += Compare(input, a, prev, "round down", reportTestCases);
						// round-up
						input = da + eps;
						a = input;
						next.setbits(i + 1);
						nrOfFailedTests += Compare(input, a, next, "round up", reportTestCases);
					}
				}
				else {
					// for the even values, we generate the round-to-actual cases
					if (i == QUARTER) {
						if (reportTestCases) std::cerr << "even-1: special case of rounding to 0\n";
						input = eps;
						a = input;
						nrOfFailedTests += Compare(input, a, ref, "round down", reportTestCases);
						input = 0.0;
						a = input;
						nrOfFailedTests += Compare(input, a, ref, " == ", reportTestCases);
						input = -eps;
						a = input;
						nrOfFailedTests += Compare(input, a, ref, "round up", reportTestCases);
					}
					else {
						// std::cerr << "even-2: same value case\n";
						// round-up
						input = da - eps;
						a = input;
						nrOfFailedTests += Compare(input, a, ref, "round up", reportTestCases);
						a = da;
						nrOfFailedTests += Compare(input, a, ref, " == ", reportTestCases);
						// round-down
						input = da + eps;
						a = input;
						nrOfFailedTests += Compare(input, a, ref, "round down", reportTestCases);
					}
				}
			}
			return nrOfFailedTests;
		}

		// enumerate all conversion cases for integers
		template<size_t nbits, size_t rbits, typename bt, auto... x>
		int VerifyIntegerConversion(bool reportTestCases) {
			using TestType = lns<nbits, rbits, bt, x...>;

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
					ref = (long)v; // obtain the integer cast of this lns
					result = ref;		// assign this integer to a lns				
					if (ref != result) { // compare the integer cast to the reference lns
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

	std::string test_suite  = "lns<> conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using LNS5_2 = lns<5, 2, std::uint8_t>;
		LNS5_2 minpos(SpecificValue::minpos);
		double mp = double(minpos);
		LNS5_2 result = mp;
		GenerateTestCase<LNS5_2, double>(mp, mp, result);
		double halfMinpos = mp / 2.0;
		result = halfMinpos;
		GenerateTestCase<LNS5_2, double>(halfMinpos, 0.0, result);
		double quarterMinpos = halfMinpos / 2.0;
		result = quarterMinpos;
		GenerateTestCase<LNS5_2, double>(quarterMinpos, 0.0, result);
		double threeQuarterMinpos = halfMinpos + quarterMinpos;
		result = threeQuarterMinpos;
		GenerateTestCase<LNS5_2, double>(threeQuarterMinpos, mp, result);

		using LNS6_3 = lns<6, 3, std::uint8_t>;
		LNS6_3 ref;
		ref.setbits(17);
		std::cout << to_binary(ref) << " : " << ref << '\n';
		double input = double(ref);
		result = input;
		std::cout << to_binary(ref) << " : " << ref << " -> " << result << " : " << to_binary(result) << '\n';
		GenerateTestCase<LNS5_2, double>(input, double(LNS5_2(SpecificValue::minpos)), result);
	}

	{
		using LNS5_2 = lns<5, 2, std::uint8_t>;
		using LNS6_3 = lns<6, 3, std::uint8_t>;

		constexpr size_t NR_SAMPLES = 32;
		LNS5_2 a;
		LNS6_3 b;
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
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<4, 1, std::uint8_t>(true), "lns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<5, 2, std::uint8_t>(true), "lns<5,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1, std::uint8_t>(true), "lns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2, std::uint8_t>(true), "lns<5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<6, 3, std::uint8_t>(true), "lns<6,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1, std::uint8_t>(true), "lns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2, std::uint8_t>(true), "lns<5,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 3, 0, std::uint8_t>(reportTestCases), "lns<3,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 3, 1, std::uint8_t>(reportTestCases), "lns<3,1>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 0, std::uint8_t>(reportTestCases), "lns<4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 1, std::uint8_t>(reportTestCases), "lns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 2, std::uint8_t>(reportTestCases), "lns<4,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 0, std::uint8_t>(reportTestCases), "lns<6,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 1, std::uint8_t>(reportTestCases), "lns<6,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 2, std::uint8_t>(reportTestCases), "lns<6,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 3, std::uint8_t>(reportTestCases), "lns<6,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 4, std::uint8_t>(reportTestCases), "lns<6,4>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 0, std::uint8_t>(reportTestCases), "lns<8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 1, std::uint8_t>(reportTestCases), "lns<8,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 2, std::uint8_t>(reportTestCases), "lns<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 3, std::uint8_t>(reportTestCases), "lns<8,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 4, std::uint8_t>(reportTestCases), "lns<8,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 5, std::uint8_t>(reportTestCases), "lns<8,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 6, std::uint8_t>(reportTestCases), "lns<8,6>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 0, std::uint8_t>(reportTestCases), "lns<9,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 1, std::uint8_t>(reportTestCases), "lns<9,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 2, std::uint8_t>(reportTestCases), "lns<9,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 3, std::uint8_t>(reportTestCases), "lns<9,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 4, std::uint8_t>(reportTestCases), "lns<9,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 5, std::uint8_t>(reportTestCases), "lns<9,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 6, std::uint8_t>(reportTestCases), "lns<9,6>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 7, std::uint8_t>(reportTestCases), "lns<9,7>", test_tag);


#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 0, std::uint8_t>(reportTestCases), "lns<10,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 1, std::uint8_t>(reportTestCases), "lns<10,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 2, std::uint8_t>(reportTestCases), "lns<10,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 3, std::uint8_t>(reportTestCases), "lns<10,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 0, std::uint8_t>(reportTestCases), "lns<12,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 1, std::uint8_t>(reportTestCases), "lns<12,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 2, std::uint8_t>(reportTestCases), "lns<12,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 3, std::uint8_t>(reportTestCases), "lns<12,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 0, std::uint8_t>(reportTestCases), "lns<14,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 1, std::uint8_t>(reportTestCases), "lns<14,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 2, std::uint8_t>(reportTestCases), "lns<14,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 3, std::uint8_t>(reportTestCases), "lns<14,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 0, std::uint8_t>(reportTestCases), "lns<16,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 1, std::uint8_t>(reportTestCases), "lns<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 2, std::uint8_t>(reportTestCases), "lns<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 3, std::uint8_t>(reportTestCases), "lns<16,3>", test_tag);
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
Generate Value table for an LNS<5,2> in TXT format
   #           Binary    sign   scale                         value          format
   0:        0b0.00.00       0       0                             1                1
   1:        0b0.00.01       0       0                       1.18921          1.18921
   2:        0b0.00.10       0       0                       1.41421          1.41421
   3:        0b0.00.11       0       0                       1.68179          1.68179
   4:        0b0.01.00       0       1                             2                2
   5:        0b0.01.01       0       1                       2.37841          2.37841
   6:        0b0.01.10       0       1                       2.82843          2.82843
   7:        0b0.01.11       0       1                       3.36359          3.36359
   8:        0b0.10.00       0      -2                             0                0
   9:        0b0.10.01       0      -2                      0.297302         0.297302
  10:        0b0.10.10       0      -2                      0.353553         0.353553
  11:        0b0.10.11       0      -2                      0.420448         0.420448
  12:        0b0.11.00       0      -1                           0.5              0.5
  13:        0b0.11.01       0      -1                      0.594604         0.594604
  14:        0b0.11.10       0      -1                      0.707107         0.707107
  15:        0b0.11.11       0      -1                      0.840896         0.840896
  16:        0b1.00.00       1       0                            -1               -1
  17:        0b1.00.01       1       0                      -1.18921         -1.18921
  18:        0b1.00.10       1       0                      -1.41421         -1.41421
  19:        0b1.00.11       1       0                      -1.68179         -1.68179
  20:        0b1.01.00       1       1                            -2               -2
  21:        0b1.01.01       1       1                      -2.37841         -2.37841
  22:        0b1.01.10       1       1                      -2.82843         -2.82843
  23:        0b1.01.11       1       1                      -3.36359         -3.36359
  24:        0b1.10.00       1      -2                     -nan(ind)        -nan(ind)
  25:        0b1.10.01       1      -2                     -0.297302        -0.297302
  26:        0b1.10.10       1      -2                     -0.353553        -0.353553
  27:        0b1.10.11       1      -2                     -0.420448        -0.420448
  28:        0b1.11.00       1      -1                          -0.5             -0.5
  29:        0b1.11.01       1      -1                     -0.594604        -0.594604
  30:        0b1.11.10       1      -1                     -0.707107        -0.707107
  31:        0b1.11.11       1      -1                     -0.840896        -0.840896
 */