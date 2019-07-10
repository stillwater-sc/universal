//  decimal.cpp : test suite for abitrary precision decimal integers
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/decimal/decimal.hpp>
#include <universal/decimal/numeric_limits.hpp>
// test helpers
#include "../test_helpers.hpp"

namespace sw {
	namespace unum {

		static constexpr unsigned DECIMAL_TABLE_WIDTH = 15;

		void ReportBinaryDecimalError(std::string test_case, std::string op, const decimal& lhs, const decimal& rhs, const decimal& dref, const long& ref) {
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
		void ReportBinaryDecimalSuccess(std::string test_case, std::string op, const decimal& lhs, const decimal& rhs, const decimal& dref, const long& ref) {
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
		int VerifyAddition(std::string tag, long ub, bool bReportIndividualTestCases) {
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
		int VerifySubtraction(std::string tag, long ub, bool bReportIndividualTestCases) {
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
		int VerifyMultiplication(std::string tag, long ub, bool bReportIndividualTestCases) {
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
	}
}

void examples() {
	using namespace std;
	using namespace sw::unum;
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
	d2 = (short)2;
	d2 = (int)3;
	d2 = (long)4;
	d2 = (long long)5;
	// unsigned integers
	d2 = (unsigned char)6;
	d2 = (unsigned short)7;
	d2 = (unsigned int)8;
	d2 = (unsigned long)9;
	d2 = (unsigned long long)10;

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
#if MSVC
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

#define MANUAL_TESTING 0
#define STRESS_TESTING 1

int main()
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Decimal Arithmetic tests failed";

#if MANUAL_TESTING

	decimal d1, d2, d3;
	reportType(d1);

	d1.parse("50000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
	cout << d1 << endl;
	cout << d1 + d1 << endl;

	long rangeBound = 100;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition("addition", rangeBound, bReportIndividualTestCases), "decimal", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction("subtraction", rangeBound, bReportIndividualTestCases), "decimal", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication("multiplication", rangeBound, bReportIndividualTestCases), "decimal", "multiplication");

#else
	std::cout << "Decimal Arithmetic verfication" << std::endl;

#ifdef STRESS_TESTING

	long rangeBound = 500;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition("addition", rangeBound, bReportIndividualTestCases), "decimal", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction("subtraction", rangeBound, bReportIndividualTestCases), "decimal", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication("multiplication", rangeBound, bReportIndividualTestCases), "decimal", "multiplication");

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
