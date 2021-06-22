//  decimal.cpp : test suite runner for abitrary precision decimal integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the decimal integer arithmetic class
#define DECIMAL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/decimal/decimal_impl.hpp>
#include <universal/number/decimal/numeric_limits.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult

namespace sw::universal {

static constexpr unsigned DECIMAL_TABLE_WIDTH = 15;

// report decimal binary operator error
void ReportBinaryDecimalError(const std::string& test_case, const std::string& op, const decimal& lhs, const decimal& rhs, const decimal& dref, const long& ref) {
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(DECIMAL_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(DECIMAL_TABLE_WIDTH) << rhs
		<< " != "
		<< std::setw(DECIMAL_TABLE_WIDTH) << dref << " it should have been "
		<< std::setw(DECIMAL_TABLE_WIDTH) << ref
		<< std::setprecision(5)
		<< std::endl;
}

// report decimal binary operator success
void ReportBinaryDecimalSuccess(const std::string& test_case, const std::string& op, const decimal& lhs, const decimal& rhs, const decimal& dref, const long& ref) {
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(DECIMAL_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(DECIMAL_TABLE_WIDTH) << rhs
		<< " == "
		<< std::setw(DECIMAL_TABLE_WIDTH) << dref << " equal to the reference "
		<< std::setw(DECIMAL_TABLE_WIDTH) << ref
		<< std::setprecision(5)
		<< std::endl;
}

// verification of addition
int VerifyAddition(const std::string& tag, long ub, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	for (long i = -ub; i <= ub; ++i) {
		decimal d1 = i;
		for (long j = -ub; j <= ub; ++j) {
			decimal d2 = j;
			long ref = i + j;
			decimal dref = d1 + d2;
			if (dref != ref) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) ReportBinaryDecimalError("FAIL", "add", d1, d2, dref, ref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryDecimalSuccess("SUCCESS", "add", d1, d2, dref, ref);
			}
		}
	}
	return nrOfFailedTests;
}

// verification of subtraction
int VerifySubtraction(const std::string& tag, long ub, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	for (long i = -ub; i <= ub; ++i) {
		decimal d1 = i;
		for (long j = -ub; j <= ub; ++j) {
			decimal d2 = j;
			long ref = i - j;
			decimal dref = d1 - d2;
			if (dref != ref) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) ReportBinaryDecimalError("FAIL", "sub", d1, d2, dref, ref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryDecimalSuccess("SUCCESS", "seb", d1, d2, dref, ref);
			}
		}
	}
	return nrOfFailedTests;
}

// verification of multiplication
int VerifyMultiplication(const std::string& tag, long ub, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	for (long i = -ub; i <= ub; ++i) {
		decimal d1 = i;
		for (long j = -ub; j <= ub; ++j) {
			decimal d2 = j;
			long ref = i * j;
			decimal dref = d1 * d2;
			if (dref != ref) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) ReportBinaryDecimalError("FAIL", "mul", d1, d2, dref, ref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryDecimalSuccess("SUCCESS", "mul", d1, d2, dref, ref);
			}
		}
	}
	return nrOfFailedTests;
}

// verification of division
int VerifyDivision(const std::string& tag, long ub, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	decimal dref;
	for (long i = -ub; i <= ub; ++i) {
		decimal d1 = i;
		for (long j = -ub; j <= ub; ++j) {
			decimal d2 = j;
			long ref = 0; // clang insists on initializing this to silence a warning
			if (j == 0) {
				try {
					dref = d1 / d2;
				}
				catch (decimal_integer_divide_by_zero& e) {
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
				if (bReportIndividualTestCases) ReportBinaryDecimalError("FAIL", "div", d1, d2, dref, ref);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryDecimalSuccess("SUCCESS", "div", d1, d2, dref, ref);
			}
		}
	}
	return nrOfFailedTests;
}

bool less(const decimal& lhs, const decimal& rhs) {
	return lhs < rhs;
}


}  // namespace sw::universal


void examples() {
	using namespace std;
	using namespace sw::universal;
	decimal d1, d2, d3;
	
//	d1.parse("0000"); cout << "bad parse: " << d1 << endl;

	d1 = -49;
	d2 =  50;
	d3 = d2 + d1;
	cout << d1 << " + " << d2 << " = " << d3 << endl;

	//cin >> d2;
	//cout << d2 << endl;



	std::string val = "1234567890";
	if (!d1.parse(val)) {
		cerr << "failed to parse the decimal value -" << val << "-\n";
	}
	cout << d1 << endl;

	val = "-123";
	if (!d2.parse(val)) {
		cerr << "failed to parse the decimal value -" << val << "-\n";
	}
	cout << d2 << endl;

	val = "+123";
	if (!d3.parse(val)) {
		cerr << "failed to parse the decimal value -" << val << "-\n";
	}
	cout << d3 << endl;

	d1.setzero();		cout << d1.iszero() << endl;
	d1.push_back(0);	cout << d1.iszero() << endl;

	cout << "Conversions\n";
	// signed integers
	d2 = (char)1;
	if (d2 != 1) cout << "assignment conversion (char) failed\n";
	d2 = (short)2;
	if (d2 != 2) cout << "assignment conversion (short) failed\n";
	d2 = (int)3;
	if (d2 != 3) cout << "assignment conversion (int) failed\n";
	d2 = (long)4;
	if (d2 != 4) cout << "assignment conversion (long) failed\n";
	d2 = (long long)5;
	if (d2 != 5) cout << "assignment conversion (long long) failed\n";
	// unsigned integers
	d2 = (unsigned char)6;
	if (d2 != 6) cout << "assignment conversion (unsigned char) failed\n";
	d2 = (unsigned short)7;
	if (d2 != 7) cout << "assignment conversion (unsigned short) failed\n";
	d2 = (unsigned int)8;
	if (d2 != 8) cout << "assignment conversion (unsigned int) failed\n";
	d2 = (unsigned long)9;
	if (d2 != 9) cout << "assignment conversion (unsigned long) failed\n";
	d2 = (unsigned long long)10;
	if (d2 != 10) cout << "assignment conversion (unsigned long long) failed\n";

	cout << "char type: " << numeric_limits<char>::digits << " max value " << (int)numeric_limits<char>::max() << endl;
	cout << "schar type : " << numeric_limits<signed char>::digits << " max value " << (int)numeric_limits<signed char>::max() << endl;

	unsigned char utest = 255;
	cout << " char       = " << (uint16_t)utest << endl;
	signed char test = 127;
	cout << "signed char = " << (int)test << endl;
}

template<typename Ty>
void reportType(Ty v) {
	using namespace std;

	cout << "Numeric limits for type " << typeid(v).name() << '\n';
	cout << "Type              : " << typeid(v).name() << endl;
#if _MSC_VER
	cout << "mangled C++ type  : " << typeid(v).raw_name() << endl;
#endif
	cout << "min()             : " << numeric_limits<Ty>::min() << '\n';
	cout << "max()             : " << numeric_limits<Ty>::max() << '\n';
	cout << "lowest()          : " << numeric_limits<Ty>::lowest() << '\n';
	cout << "epsilon()         : " << numeric_limits<Ty>::epsilon() << '\n';

	cout << "digits            : " << numeric_limits<Ty>::digits << '\n';
	cout << "digits10          : " << numeric_limits<Ty>::digits10 << '\n';
	cout << "max_digits10      : " << numeric_limits<Ty>::max_digits10 << '\n';
	cout << "is_signed         : " << numeric_limits<Ty>::is_signed << '\n';
	cout << "is_integer        : " << numeric_limits<Ty>::is_integer << '\n';
	cout << "is_exact          : " << numeric_limits<Ty>::is_exact << '\n';

	cout << "min_exponent      : " << numeric_limits<Ty>::min_exponent << '\n';
	cout << "min_exponent10    : " << numeric_limits<Ty>::min_exponent10 << '\n';
	cout << "max_exponent      : " << numeric_limits<Ty>::max_exponent << '\n';
	cout << "max_exponent10    : " << numeric_limits<Ty>::max_exponent10 << '\n';
	cout << "has_infinity      : " << numeric_limits<Ty>::has_infinity << '\n';
	cout << "has_quiet_NaN     : " << numeric_limits<Ty>::has_quiet_NaN << '\n';
	cout << "has_signaling_NaN : " << numeric_limits<Ty>::has_signaling_NaN << '\n';
	cout << "has_denorm        : " << numeric_limits<Ty>::has_denorm << '\n';
	cout << "has_denorm_loss   : " << numeric_limits<Ty>::has_denorm_loss << '\n';

	cout << "is_iec559         : " << numeric_limits<Ty>::is_iec559 << '\n';
	cout << "is_bounded        : " << numeric_limits<Ty>::is_bounded << '\n';
	cout << "is_modulo         : " << numeric_limits<Ty>::is_modulo << '\n';
	cout << "traps             : " << numeric_limits<Ty>::traps << '\n';
	cout << "tinyness_before   : " << numeric_limits<Ty>::tinyness_before << '\n';
	cout << "round_style       : " << numeric_limits<Ty>::round_style << '\n';
}

void findLargestMultipleTest() {
	sw::universal::decimal d;
	int fails = 0;
	int numerator = 9;
	d = numerator;
	for (int i = 0; i < 100; ++i) {
		sw::universal::decimal multiple = sw::universal::findLargestMultiple(i, d);
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
	using namespace std;
	using namespace sw::universal;

	cout << "big number computation\n";
	int nrOfFailedTestCases = 0;
	decimal a, b, c, d, e, f;
	a.parse("1234567890"); cout << a << endl;
	b.parse("5432109876"); cout << b << endl;
	c = decimal(1) << 9; cout << c << endl;
	d = a * b * c; cout << d << endl;
	e = d / a;  cout << e << endl;
	f = e / b; cout << f << endl;
	if (c != f) {
		++nrOfFailedTestCases;
		cout << "FAIL: " << c << " is not equal to " << f << endl;
	}
	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "decimal arithmetic";

#if MANUAL_TESTING

	decimal d1, d2, d3;

	d1 = -1'234'567'890;
	d2 = +1'234'567'890;
	d3 = d1 + d2;
	cout << d1 << " + " << d2 << " = " << d3 << endl;

	// double conversion is not implemented yet
	d1 = -0.25;
	cout << d1 << endl;
	d1 = 2.5;
	cout << d1 << endl;
	d1 = 123456789.5;
	cout << d1 << endl;
	d1 = 1.234567895e10;
	cout << d1 << endl;
	d1 = 1.234567895e100;
	cout << d1 << endl;
	return 0;
	//reportType(d1);

	findLargestMultipleTest();

	d1.parse("50000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
	cout << d1 << endl;
	cout << d1 + d1 << endl;

	long rangeBound = 10; // 100;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition("addition", rangeBound, bReportIndividualTestCases), "decimal", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction("subtraction", rangeBound, bReportIndividualTestCases), "decimal", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication("multiplication", rangeBound, bReportIndividualTestCases), "decimal", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision("division", rangeBound, bReportIndividualTestCases), "decimal", "division");

	nrOfFailedTestCases += BigNumberComputation();

	nrOfFailedTestCases = 0; // in manual testing ignore failures

#else
	std::cout << "Decimal Arithmetic verfication" << std::endl;

	long rangeBound = 100;
	cout << "quick sample test with range bound: " << rangeBound << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition("addition", rangeBound, bReportIndividualTestCases), "decimal", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction("subtraction", rangeBound, bReportIndividualTestCases), "decimal", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication("multiplication", rangeBound, bReportIndividualTestCases), "decimal", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision("division", rangeBound, bReportIndividualTestCases), "decimal", "division");

	nrOfFailedTestCases += BigNumberComputation();

#if STRESS_TESTING

	long stressRangeBound = (1 << 9);
	cout << "stress testing with range bound: " << stressRangeBound << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition("addition", stressRangeBound, bReportIndividualTestCases), "decimal", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction("subtraction", stressRangeBound, bReportIndividualTestCases), "decimal", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication("multiplication", stressRangeBound, bReportIndividualTestCases), "decimal", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision("division", stressRangeBound, bReportIndividualTestCases), "decimal", "division");

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
