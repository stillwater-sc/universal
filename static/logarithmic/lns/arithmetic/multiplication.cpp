// multiplication.cpp: test suite runner for multiplication arithmetic of fixed-sized, arbitrary precision logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// configure the number system
#define LNS_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/table.hpp>
#include <universal/verification/test_reporters.hpp>
//#include <universal/verification/test_suite.hpp>   // the generic VerifyMultiplication doesn't deal with the LNS special cases
//#include <universal/verification/lns_test_suite.hpp>  // is that the right solution to specialize?

namespace sw {
	namespace universal {
		namespace local {

			template<typename LnsType, std::enable_if_t<is_lns<LnsType>, bool> = true>
			int VerifyMultiplication(bool reportTestCases) {
				constexpr size_t nbits = LnsType::nbits;
				//constexpr size_t rbits = LnsType::rbits;
				//constexpr Behavior behavior = LnsType::behavior;
				//using bt = typename LnsType::BlockType;
				constexpr size_t NR_ENCODINGS = (1ull << nbits);

				int nrOfFailedTestCases = 0;

				LnsType a{}, b{}, c{}, cref{};
				for (size_t i = 0; i < NR_ENCODINGS; ++i) {
					a.setbits(i);
					double da = double(a);
					for (size_t j = 0; j < NR_ENCODINGS; ++j) {
						b.setbits(j);
						double db = double(b);

						double ref = da * db;
						if (reportTestCases && !isInRange<LnsType>(ref)) {
							std::cerr << da << " * " << db << " = " << ref << " which is not in range " << range(a) << '\n';
						}
						c = a * b;
						cref = ref;
						//				std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
						//				std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
						if (c != cref) {
							if (c.isnan() && cref.isnan()) continue; // NaN non-equivalence
							++nrOfFailedTestCases;
							if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", a, b, c, cref);
							//					std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
							//					std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
						}
						else {
							//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
						}
						if (nrOfFailedTestCases > 25) return nrOfFailedTestCases;
					}
				}
				return nrOfFailedTestCases;
			}

		}
	}
}

/*
Generate Value table for an LNS<4,1> in TXT format
   #           Binary    sign   scale                         value          format
   0:         0b0.00.0       0       0                             1                1
   1:         0b0.00.1       0       0                       1.41421          1.41421
   2:         0b0.01.0       0       1                             2                2
   3:         0b0.01.1       0       1                       2.82843          2.82843
   4:         0b0.10.0       0      -2                             0                0
   5:         0b0.10.1       0      -2                      0.353553         0.353553
   6:         0b0.11.0       0      -1                           0.5              0.5
   7:         0b0.11.1       0      -1                      0.707107         0.707107
   8:         0b1.00.0       1       0                            -1               -1
   9:         0b1.00.1       1       0                      -1.41421         -1.41421
  10:         0b1.01.0       1       1                            -2               -2
  11:         0b1.01.1       1       1                      -2.82843         -2.82843
  12:         0b1.10.0       1      -2                     -nan(ind)        -nan(ind)
  13:         0b1.10.1       1      -2                     -0.353553        -0.353553
  14:         0b1.11.0       1      -1                          -0.5             -0.5
  15:         0b1.11.1       1      -1                     -0.707107        -0.707107

Generate Value table for an LNS<4,2> in TXT format
   #           Binary    sign   scale                         value          format
   0:         0b0.0.00       0       0                             1                1
   1:         0b0.0.01       0       0                       1.18921          1.18921
   2:         0b0.0.10       0       0                       1.41421          1.41421
   3:         0b0.0.11       0       0                       1.68179          1.68179
   4:         0b0.1.00       0      -1                             0                0
   5:         0b0.1.01       0      -1                      0.594604         0.594604
   6:         0b0.1.10       0      -1                      0.707107         0.707107
   7:         0b0.1.11       0      -1                      0.840896         0.840896
   8:         0b1.0.00       1       0                            -1               -1
   9:         0b1.0.01       1       0                      -1.18921         -1.18921
  10:         0b1.0.10       1       0                      -1.41421         -1.41421
  11:         0b1.0.11       1       0                      -1.68179         -1.68179
  12:         0b1.1.00       1      -1                     -nan(ind)        -nan(ind)
  13:         0b1.1.01       1      -1                     -0.594604        -0.594604
  14:         0b1.1.10       1      -1                     -0.707107        -0.707107
  15:         0b1.1.11       1      -1                     -0.840896        -0.840896

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

	std::string test_suite  = "lns multiplication validation";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);

#if MANUAL_TESTING

	using LNS4_1_mod = lns<4, 1, std::uint8_t, Behavior::Wrapping>;
	using LNS4_1_sat = lns<4, 1, std::uint8_t>;
	using LNS4_2     = lns<4, 2, std::uint8_t>;
	using LNS4_3_sat = lns<4, 3, std::uint8_t>;
	using LNS5_2     = lns<5, 2, std::uint8_t>;
	using LNS8_3     = lns<8, 3, std::uint8_t>;
	using LNS9_4     = lns<9, 4, std::uint8_t>;
	using LNS9_8_sat = lns<9, 8, std::uint8_t>;
	using LNS16_5    = lns<16, 5, std::uint16_t>;

	// generate individual testcases to hand trace/debug
	TestCase<LNS4_1_sat, float>(TestCaseOperator::MUL, 0.353f, -0.353f);
	TestCase<LNS16_5, double>(TestCaseOperator::MUL, INFINITY, INFINITY);
	TestCase<LNS8_3, float>(TestCaseOperator::MUL, 0.5f, -0.5f);

	// GenerateLnsTable<5, 2>(std::cout);
	LNS4_3_sat a(0);
	a.debugConstexprParameters();
	LNS9_8_sat b(0);
	b.debugConstexprParameters();

//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<LNS4_1_mod>(false), "lns<4,1,uint8_t,Behavior::Wrapping>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<LNS4_1_sat>(reportTestCases), "lns<4,1, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<LNS4_2>(reportTestCases), "lns<4,2, uint8_t>", test_tag);
   	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<LNS4_3_sat>(reportTestCases), "lns<4,3, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<LNS5_2>(reportTestCases), "lns<5,2, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<LNS8_3>(reportTestCases), "lns<8,3, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<LNS9_4>(reportTestCases), "lns<9,4, uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	using LNS4_0_sat = lns<4, 0, std::uint8_t>;
	using LNS4_1_sat = lns<4, 1, std::uint8_t>;
	using LNS4_2_sat = lns<4, 2, std::uint8_t>;
	using LNS4_3_sat = lns<4, 3, std::uint8_t>;
	using LNS5_2_sat = lns<5, 2, std::uint8_t>;
	using LNS8_1_sat = lns<8, 1, std::uint8_t>;
	using LNS8_4_sat = lns<8, 4, std::uint8_t>;
	using LNS8_6_sat = lns<8, 6, std::uint8_t>;
	using LNS9_0_sat = lns<9, 0, std::uint8_t>;
	using LNS9_4_sat = lns<9, 4, std::uint8_t>;
	using LNS9_7_sat = lns<9, 7, std::uint8_t>;
	using LNS9_8_sat = lns<9, 8, std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS4_0_sat>(true), "lns<4,0, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS4_1_sat>(reportTestCases), "lns<4,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS4_2_sat>(reportTestCases), "lns<4,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS4_3_sat>(reportTestCases), "lns<4,3, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS5_2_sat>(reportTestCases), "lns<5,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS8_1_sat>(reportTestCases), "lns<8,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS8_4_sat>(reportTestCases), "lns<8,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS8_6_sat>(reportTestCases), "lns<8,6, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS9_0_sat>(reportTestCases), "lns<9,0, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS9_4_sat>(reportTestCases), "lns<9,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS9_7_sat>(reportTestCases), "lns<9,7, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS9_8_sat>(reportTestCases), "lns<9,8, uint8_t>", test_tag);

#endif

#if REGRESSION_LEVEL_2
	using LNS10_0_sat = lns<10, 0, std::uint8_t>;
	using LNS10_4_sat = lns<10, 4, std::uint8_t>;
	using LNS10_8_sat = lns<10, 8, std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS10_0_sat>(reportTestCases), "lns<10,0, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS10_4_sat>(reportTestCases), "lns<10,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<LNS10_8_sat>(reportTestCases), "lns<10,8, uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
