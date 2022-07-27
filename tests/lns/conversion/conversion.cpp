// conversion.cpp : test suite runner for conversion operators to logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

		template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
		int Compare(double input, const lns<nbits, rbits, behavior, bt>& presult, double reference, bool reportTestCases) {
			int fail = 0;
			double result = double(presult);
			if (std::fabs(result - reference) > 0.000000001) {
				fail++;
				if (reportTestCases)	ReportConversionError("FAIL", "=", input, presult, reference);
			}
			else {
				// if (reportTestCases) ReportConversionSuccess("PASS", "=", input, reference, presult);
			}
			return fail;
		}

		// enumerate all conversion cases for a posit configuration
		template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
		int VerifyConversion(bool reportTestCases) {
			// we are going to generate a test set that consists of all lns configs and their midpoints
			// we do this by enumerating an lns that is 1-bit larger than the test configuration
			// These larger posits will be at the mid-point between the smaller sample values
			// and we'll enumerate the exact value, and a perturbation smaller and a perturbation larger
			// to test the rounding logic of the conversion.
			using TestType = lns<nbits, rbits, behavior, bt>;
			using ContainingType = lns<nbits + 1, rbits + 1, behavior, bt>;

			constexpr size_t max = nbits > 16 ? 16 : nbits;
			size_t NR_TEST_CASES = (size_t(1) << (max + 1));
			size_t HALF = (size_t(1) << max);

			if constexpr (nbits > 16) {
				std::cout << "VerifyConversion: " << type_tag(TestType()) << " : NR_TEST_CASES = " << NR_TEST_CASES << " constrained due to nbits > 16" << std::endl;
			}

			double halfMinpos = double(ContainingType(SpecificValue::minpos)) / 2.0;
			// execute the test
			int nrOfFailedTests = 0;
			for (size_t i = 0; i < NR_TEST_CASES; i++) {
				ContainingType ref, prev, next;

				ref.setbits(i);
				double da = double(ref);
				double eps = double(i == 0 ? halfMinpos : (da > 0 ? da * 1.0e-6 : da * -1.0e-6));
				double input;
				TestType a;
				if (i % 2) {
					if (i == 1) {
						// special case of projecting to +minpos
						// even the -delta goes to +minpos
						input = da - eps;
						a = input;
						next.setbits(i + 1);
						nrOfFailedTests += Compare(input, a, double(next), reportTestCases);
						input = da + eps;
						a = input;
						nrOfFailedTests += Compare(input, a, double(next), reportTestCases);
					}
					else if (i == HALF - 1) {
						// special case of projecting to +maxpos
						input = da - eps;
						a = input;
						prev.setbits(HALF - 2);
						nrOfFailedTests += Compare(input, a, double(prev), reportTestCases);
					}
					else if (i == HALF + 1) {
						// special case of projecting to -maxpos
						input = da - eps;
						a = input;
						prev.setbits(HALF + 2);
						nrOfFailedTests += Compare(input, a, double(prev), reportTestCases);
					}
					else if (i == NR_TEST_CASES - 1) {
						// special case of projecting to -minpos
						// even the +delta goes to -minpos
						input = da - eps;
						a = input;
						prev.setbits(i - 1);
						nrOfFailedTests += Compare(input, a, double(prev), reportTestCases);
						input = da + eps;
						a = input;
						nrOfFailedTests += Compare(input, a, double(prev), reportTestCases);
					}
					else {
						// for odd values, we are between posit values, so we create the round-up and round-down cases
						// round-down
						input = da - eps;
						a = input;
						prev.setbits(i - 1);
						nrOfFailedTests += Compare(input, a, double(prev), reportTestCases);
						// round-up
						input = da + eps;
						a = input;
						next.setbits(i + 1);
						nrOfFailedTests += Compare(input, a, double(next), reportTestCases);
					}
				}
				else {
					// for the even values, we generate the round-to-actual cases
					if (i == 0) {
						// special case of assigning to 0
						input = 0.0;
						a = input;
						nrOfFailedTests += Compare(input, a, da, reportTestCases);
						// special case of projecting to +minpos
						input = da + eps;
						a = input;
						next.setbits(i + 2);
						nrOfFailedTests += Compare(input, a, double(next), reportTestCases);
					}
					else if (i == NR_TEST_CASES - 2) {
						// special case of projecting to -minpos
						input = da - eps;
						a = input;
						prev.setbits(NR_TEST_CASES - 2);
						nrOfFailedTests += Compare(input, a, double(prev), reportTestCases);
					}
					else {
						// round-up
						input = da - eps;
						a = input;
						nrOfFailedTests += Compare(input, a, da, reportTestCases);
						// round-down
						input = da + eps;
						a = input;
						nrOfFailedTests += Compare(input, a, da, reportTestCases);
					}
				}
			}
			return nrOfFailedTests;
		}

		// enumerate all conversion cases for integers
		template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
		int VerifyIntegerConversion(bool reportTestCases) {
			using TestType = lns<nbits, rbits, behavior, bt>;

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

	std::string test_suite  = "lns<> conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using LNS5_2 = lns<5, 2, Saturating, std::uint8_t>;
		using LNS6_3 = lns<6, 3, Saturating, std::uint8_t>;

		constexpr size_t NR_SAMPLES = 32;
		LNS5_2 a;
		LNS6_3 b;
		for (size_t i = 0; i < NR_SAMPLES; ++i) {
			b.setbits(i);
			if (i % 2 == 0) {
				a.setbits(i);
				std::cout << to_binary(b) << " : " << std::setw(10) << b << " - " << std::setw(10) << a << " : " << to_binary(a) << '\n';
			}
			else {
				std::cout << to_binary(b) << " : " << std::setw(10) << b << '\n';
			}
		}
	}

	return 0; 
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<4, 1, Saturating, std::uint8_t>(true), "lns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<5, 2, Saturating, std::uint8_t>(true), "lns<5,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1, Saturating, std::uint8_t>(true), "lns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2, Saturating, std::uint8_t>(true), "lns<5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<6, 3, Saturating, std::uint8_t>(true), "lns<6,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1, Saturating, std::uint8_t>(true), "lns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2, Saturating, std::uint8_t>(true), "lns<5,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<3, 0, Saturating, std::uint8_t>(reportTestCases), "lns<3,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<4, 0, Saturating, std::uint8_t>(reportTestCases), "lns<4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<5, 0, Saturating, std::uint8_t>(reportTestCases), "lns<5,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<6, 0, Saturating, std::uint8_t>(reportTestCases), "lns<6,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<7, 0, Saturating, std::uint8_t>(reportTestCases), "lns<7,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<8, 0, Saturating, std::uint8_t>(reportTestCases), "lns<8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<9, 0, Saturating, std::uint8_t>(reportTestCases), "lns<9,0>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 3, 0, Saturating, std::uint8_t>(reportTestCases), "lns<3,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 0, Saturating, std::uint8_t>(reportTestCases), "lns<4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 0, Saturating, std::uint8_t>(reportTestCases), "lns<5,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 0, Saturating, std::uint8_t>(reportTestCases), "lns<6,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 0, Saturating, std::uint8_t>(reportTestCases), "lns<7,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 0, Saturating, std::uint8_t>(reportTestCases), "lns<8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 0, Saturating, std::uint8_t>(reportTestCases), "lns<9,0>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 1, Saturating, std::uint8_t>(reportTestCases), "lns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 1, Saturating, std::uint8_t>(reportTestCases), "lns<5,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 1, Saturating, std::uint8_t>(reportTestCases), "lns<6,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 1, Saturating, std::uint8_t>(reportTestCases), "lns<7,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 1, Saturating, std::uint8_t>(reportTestCases), "lns<8,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 1, Saturating, std::uint8_t>(reportTestCases), "lns<9,1>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 2, Saturating, std::uint8_t>(reportTestCases), "lns<5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 2, Saturating, std::uint8_t>(reportTestCases), "lns<6,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 2, Saturating, std::uint8_t>(reportTestCases), "lns<7,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 2, Saturating, std::uint8_t>(reportTestCases), "lns<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 2, Saturating, std::uint8_t>(reportTestCases), "lns<9,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 3, Saturating, std::uint8_t>(reportTestCases), "lns<6,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 3, Saturating, std::uint8_t>(reportTestCases), "lns<7,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 3, Saturating, std::uint8_t>(reportTestCases), "lns<8,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 3, Saturating, std::uint8_t>(reportTestCases), "lns<9,3>", test_tag);
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 0, Saturating, std::uint8_t>(reportTestCases), "lns<10,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 1, Saturating, std::uint8_t>(reportTestCases), "lns<10,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 2, Saturating, std::uint8_t>(reportTestCases), "lns<10,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 3, Saturating, std::uint8_t>(reportTestCases), "lns<10,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 0, Saturating, std::uint8_t>(reportTestCases), "lns<12,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 1, Saturating, std::uint8_t>(reportTestCases), "lns<12,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 2, Saturating, std::uint8_t>(reportTestCases), "lns<12,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 3, Saturating, std::uint8_t>(reportTestCases), "lns<12,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 0, Saturating, std::uint8_t>(reportTestCases), "lns<14,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 1, Saturating, std::uint8_t>(reportTestCases), "lns<14,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 2, Saturating, std::uint8_t>(reportTestCases), "lns<14,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 3, Saturating, std::uint8_t>(reportTestCases), "lns<14,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 0, Saturating, std::uint8_t>(reportTestCases), "lns<16,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 1, Saturating, std::uint8_t>(reportTestCases), "lns<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 2, Saturating, std::uint8_t>(reportTestCases), "lns<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 3, Saturating, std::uint8_t>(reportTestCases), "lns<16,3>", test_tag);
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

