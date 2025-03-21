//  api.cpp : test suite runner for adaptive precision decimal integers class API
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// configure the elastic decimal integer arithmetic class
#define EDECIMAL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult

namespace sw::universal {

static constexpr unsigned edecimal_TABLE_WIDTH = 15;

// report edecimal binary operator error
void ReportBinaryEdecimalError(const std::string& test_case, const std::string& op, const edecimal& lhs, const edecimal& rhs, const edecimal& dref, const long& ref) {
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(edecimal_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(edecimal_TABLE_WIDTH) << rhs
		<< " != "
		<< std::setw(edecimal_TABLE_WIDTH) << dref << " it should have been "
		<< std::setw(edecimal_TABLE_WIDTH) << ref
		<< std::setprecision(5)
		<< std::endl;
}

// report edecimal binary operator success
void ReportBinaryEdecimalSuccess(const std::string& test_case, const std::string& op, const edecimal& lhs, const edecimal& rhs, const edecimal& dref, const long& ref) {
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(edecimal_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(edecimal_TABLE_WIDTH) << rhs
		<< " == "
		<< std::setw(edecimal_TABLE_WIDTH) << dref << " equal to the reference "
		<< std::setw(edecimal_TABLE_WIDTH) << ref
		<< std::setprecision(5)
		<< std::endl;
}

// verification of addition
int VerifyAddition(long ub, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	for (long i = -ub; i <= ub; ++i) {
		edecimal d1 = i;
		for (long j = -ub; j <= ub; ++j) {
			edecimal d2 = j;
			long ref = i + j;
			edecimal dref = d1 + d2;
			if (dref != ref) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) ReportBinaryEdecimalError("FAIL", "add", d1, d2, dref, ref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryEdecimalSuccess("SUCCESS", "add", d1, d2, dref, ref);
			}
		}
	}
	return nrOfFailedTests;
}

// verification of subtraction
int VerifySubtraction(long ub, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	for (long i = -ub; i <= ub; ++i) {
		edecimal d1 = i;
		for (long j = -ub; j <= ub; ++j) {
			edecimal d2 = j;
			long ref = i - j;
			edecimal dref = d1 - d2;
			if (dref != ref) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) ReportBinaryEdecimalError("FAIL", "sub", d1, d2, dref, ref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryEdecimalSuccess("SUCCESS", "seb", d1, d2, dref, ref);
			}
		}
	}
	return nrOfFailedTests;
}

// verification of multiplication
int VerifyMultiplication(long ub, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	for (long i = -ub; i <= ub; ++i) {
		edecimal d1 = i;
		for (long j = -ub; j <= ub; ++j) {
			edecimal d2 = j;
			long ref = i * j;
			edecimal dref = d1 * d2;
			if (dref != ref) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) ReportBinaryEdecimalError("FAIL", "mul", d1, d2, dref, ref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryEdecimalSuccess("SUCCESS", "mul", d1, d2, dref, ref);
			}
		}
	}
	return nrOfFailedTests;
}

// verification of division
int VerifyDivision(long ub, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	edecimal dref;
	for (long i = -ub; i <= ub; ++i) {
		edecimal d1 = i;
		for (long j = -ub; j <= ub; ++j) {
			edecimal d2 = j;
			long ref = 0; // clang insists on initializing this to silence a warning
			if (j == 0) {
				try {
					dref = d1 / d2;
				}
				catch (edecimal_integer_divide_by_zero& e) {
					if (bReportIndividualTestCases) {
						std::cout << "properly caught divide by zero exception: " << e.what() << std::endl;
					}

					continue;
				}
				catch (...) {
					++nrOfFailedTests;
					continue;
				}
			}
			else {
				dref = d1 / d2;
				ref = i / j;
			}
			if (dref != ref) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) ReportBinaryEdecimalError("FAIL", "div", d1, d2, dref, ref);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryEdecimalSuccess("SUCCESS", "div", d1, d2, dref, ref);
			}
		}
	}
	return nrOfFailedTests;
}

bool less(const edecimal& lhs, const edecimal& rhs) {
	return lhs < rhs;
}


}  // namespace sw::universal


void examples() {
	using namespace sw::universal;
	edecimal d1, d2, d3;
	
//	d1.parse("0000"); cout << "bad parse: " << d1 << endl;

	d1 = -49;
	d2 =  50;
	d3 = d2 + d1;
	std::cout << d1 << " + " << d2 << " = " << d3 << '\n';

	std::string val = "1234567890";
	if (!d1.parse(val)) {
		std::cerr << "failed to parse the edecimal value -" << val << "-\n";
	}
	std::cout << d1 << '\n';

	val = "-123";
	if (!d2.parse(val)) {
		std::cerr << "failed to parse the edecimal value -" << val << "-\n";
	}
	std::cout << d2 << '\n';

	val = "+123";
	if (!d3.parse(val)) {
		std::cerr << "failed to parse the edecimal value -" << val << "-\n";
	}
	std::cout << d3 << '\n';

	d1.setzero();		std::cout << d1.iszero() << '\n';
	d1.push_back(0);	std::cout << d1.iszero() << '\n';

	std::cout << "Conversions\n";
	// signed integers
	d2 = (char)1;
	if (d2 != 1) std::cout << "assignment conversion (char) failed\n";
	d2 = (short)2;
	if (d2 != 2) std::cout << "assignment conversion (short) failed\n";
	d2 = (int)3;
	if (d2 != 3) std::cout << "assignment conversion (int) failed\n";
	d2 = (long)4;
	if (d2 != 4) std::cout << "assignment conversion (long) failed\n";
	d2 = (long long)5;
	if (d2 != 5) std::cout << "assignment conversion (long long) failed\n";
	// unsigned integers
	d2 = (unsigned char)6;
	if (d2 != 6) std::cout << "assignment conversion (unsigned char) failed\n";
	d2 = (unsigned short)7;
	if (d2 != 7) std::cout << "assignment conversion (unsigned short) failed\n";
	d2 = (unsigned int)8;
	if (d2 != 8) std::cout << "assignment conversion (unsigned int) failed\n";
	d2 = (unsigned long)9;
	if (d2 != 9) std::cout << "assignment conversion (unsigned long) failed\n";
	d2 = (unsigned long long)10;
	if (d2 != 10) std::cout << "assignment conversion (unsigned long long) failed\n";

	std::cout << "char type: " << std::numeric_limits<char>::digits << " max value " << (int)std::numeric_limits<char>::max() << '\n';
	std::cout << "schar type : " << std::numeric_limits<signed char>::digits << " max value " << (int)std::numeric_limits<signed char>::max() << '\n';

	unsigned char utest = 255;
	std::cout << " char       = " << (uint16_t)utest << '\n';
	signed char test = 127;
	std::cout << "signed char = " << (int)test << '\n';
}

template<typename Ty>
void reportType(Ty v) {
	std::cout << "Numeric limits for type " << typeid(v).name() << '\n';
	std::cout << "Type              : " << typeid(v).name() << '\n';
#if _MSC_VER
	std::cout << "mangled C++ type  : " << typeid(v).raw_name() << '\n';
#endif
	std::cout << "min()             : " << std::numeric_limits<Ty>::min() << '\n';
	std::cout << "max()             : " << std::numeric_limits<Ty>::max() << '\n';
	std::cout << "lowest()          : " << std::numeric_limits<Ty>::lowest() << '\n';
	std::cout << "epsilon()         : " << std::numeric_limits<Ty>::epsilon() << '\n';

	std::cout << "digits            : " << std::numeric_limits<Ty>::digits << '\n';
	std::cout << "digits10          : " << std::numeric_limits<Ty>::digits10 << '\n';
	std::cout << "max_digits10      : " << std::numeric_limits<Ty>::max_digits10 << '\n';
	std::cout << "is_signed         : " << std::numeric_limits<Ty>::is_signed << '\n';
	std::cout << "is_integer        : " << std::numeric_limits<Ty>::is_integer << '\n';
	std::cout << "is_exact          : " << std::numeric_limits<Ty>::is_exact << '\n';

	std::cout << "min_exponent      : " << std::numeric_limits<Ty>::min_exponent << '\n';
	std::cout << "min_exponent10    : " << std::numeric_limits<Ty>::min_exponent10 << '\n';
	std::cout << "max_exponent      : " << std::numeric_limits<Ty>::max_exponent << '\n';
	std::cout << "max_exponent10    : " << std::numeric_limits<Ty>::max_exponent10 << '\n';
	std::cout << "has_infinity      : " << std::numeric_limits<Ty>::has_infinity << '\n';
	std::cout << "has_quiet_NaN     : " << std::numeric_limits<Ty>::has_quiet_NaN << '\n';
	std::cout << "has_signaling_NaN : " << std::numeric_limits<Ty>::has_signaling_NaN << '\n';
	std::cout << "has_denorm        : " << std::numeric_limits<Ty>::has_denorm << '\n';
	std::cout << "has_denorm_loss   : " << std::numeric_limits<Ty>::has_denorm_loss << '\n';

	std::cout << "is_iec559         : " << std::numeric_limits<Ty>::is_iec559 << '\n';
	std::cout << "is_bounded        : " << std::numeric_limits<Ty>::is_bounded << '\n';
	std::cout << "is_modulo         : " << std::numeric_limits<Ty>::is_modulo << '\n';
	std::cout << "traps             : " << std::numeric_limits<Ty>::traps << '\n';
	std::cout << "tinyness_before   : " << std::numeric_limits<Ty>::tinyness_before << '\n';
	std::cout << "round_style       : " << std::numeric_limits<Ty>::round_style << '\n';
}

void findLargestMultipleTest() {
	sw::universal::edecimal d;
	int fails = 0;
	int numerator = 9;
	d = numerator;
	for (int i = 0; i < 100; ++i) {
		sw::universal::edecimal multiple = sw::universal::findLargestMultiple(i, d);
		if (multiple != (i / 9)) {
			std::cout << d << " into " << i << " yields multiplier " << multiple << " but should have been " << (i/numerator) << std::endl;
			++fails;
		}	
	}
	if (fails == 0) {
		std::cout << "PASS  : findLargestMultipleTest" << std::endl;
	}
	else {
		std::cout << fails << " FAILURES in findLargestMultipleTest"  << std::endl;
	}
}

int BigNumberComputation() {
	using namespace sw::universal;

	std::cout << "big number computation\n";
	int nrOfFailedTestCases = 0;
	edecimal a, b, c, d, e, f;
	a.parse("1234567890"); std::cout << a << '\n';
	b.parse("5432109876"); std::cout << b << '\n';
	c = edecimal(1) << 9; std::cout << c << '\n';
	d = a * b * c; std::cout << d << '\n';
	e = d / a;  std::cout << e << '\n';
	f = e / b; std::cout << f << '\n';
	if (c != f) {
		++nrOfFailedTestCases;
		std::cout << "FAIL: " << c << " is not equal to " << f << '\n';
	}
	return nrOfFailedTestCases;
}

#include <universal/verification/test_status.hpp>

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

#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "edecimal arithmetic";

#if MANUAL_TESTING

	edecimal d1, d2, d3;

	d1 = -1'234'567'890;
	d2 = +1'234'567'890;
	d3 = d1 + d2;
	std::cout << d1 << " + " << d2 << " = " << d3 << '\n';

	// ieee754 conversion
	d1 = 0.5f;
	std::cout << d1 << '\n';
	d1 = 1.0f;
	std::cout << d1 << '\n';
	d1 = 2.5f;
	std::cout << d1 << '\n';
	d1 = 123456789.5;
	std::cout << d1 << '\n';
	d1 = 1.234567895e10;
	std::cout << d1 << '\n';
	d1 = 1.234567895e18;
	std::cout << d1 << '\n';
	d1 = 1.234567895e20;
	std::cout << d1 << '\n';
	d1 = 1.234567895e30;
	std::cout << d1 << '\n';
	d1 = 1.234567895e10;
	d2 = 1.0e20;
	std::cout << d1 * d2 << '\n';

	//reportType(d1);

	findLargestMultipleTest();

	d1.parse("50000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
	std::cout << "big number :  " << d1 << '\n';
	std::cout << "doubled    : " << d1 + d1 << std::endl;

	long rangeBound = 10; // 100;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition(rangeBound, bReportIndividualTestCases), "edecimal", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction(rangeBound, bReportIndividualTestCases), "edecimal", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication(rangeBound, bReportIndividualTestCases), "edecimal", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision(rangeBound, bReportIndividualTestCases), "edecimal", "division");

	nrOfFailedTestCases += BigNumberComputation();

	nrOfFailedTestCases = 0; // in manual testing ignore failures

#else
	std::cout << "edecimal Arithmetic verfication\n";

	long rangeBound = 100;
	std::cout << "quick sample test with range bound: " << rangeBound << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyAddition(rangeBound, bReportIndividualTestCases), "edecimal", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction(rangeBound, bReportIndividualTestCases), "edecimal", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication(rangeBound, bReportIndividualTestCases), "edecimal", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision(rangeBound, bReportIndividualTestCases), "edecimal", "division");

	nrOfFailedTestCases += BigNumberComputation();

#if STRESS_TESTING

	long stressRangeBound = (1 << 9);
	std::cout << "stress testing with range bound: " << stressRangeBound << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyAddition(stressRangeBound, bReportIndividualTestCases), "edecimal", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction(stressRangeBound, bReportIndividualTestCases), "edecimal", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication(stressRangeBound, bReportIndividualTestCases), "edecimal", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision(stressRangeBound, bReportIndividualTestCases), "edecimal", "division");

#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
