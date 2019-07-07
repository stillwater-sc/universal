//  decimal.cpp : test suite for abitrary precision decimal integers
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include "../../decimal/decimal.hpp"

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
				<< std::setw(DECIMAL_TABLE_WIDTH) << ref << " instead it yielded "
				<< std::setw(DECIMAL_TABLE_WIDTH) << dref
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
				<< std::setw(DECIMAL_TABLE_WIDTH) << ref << " correctly yielded "
				<< std::setw(DECIMAL_TABLE_WIDTH) << dref
				<< std::setprecision(5)
				<< std::endl;
		}
		int ValidateAddition(std::string tag, long ub, bool bReportIndividualTestCases) {
			int nrOfFailedTests = 0;
			for (long i = -ub; i < ub; ++i) {
				decimal d1 = i;
				for (long j = -ub; j < ub; ++j) {
					decimal d2 = j;
					long ref = i + j;
					decimal dref = d1 + d2;
					if (dref != ref) {
						++nrOfFailedTests;
						if (bReportIndividualTestCases) ReportBinaryDecimalError("FAIL", "add", d1, d2, dref, ref);
					}
					else {
						//if (bReportIndividualTestCases) ReportBinaryDecimalSuccess("SUCCESS", "add", d1, d2, dref, ref);
					}
				}
			}
			return nrOfFailedTests;
		}
	}
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Decimal Arithmetic tests failed";

#if MANUAL_TESTING
	decimal d1, d2, d3;
	
//	d1.parse("0000"); cout << "bad parse: " << d1 << endl;

	d1 = 12345;
	d2 =   -78;
	d3 = d2 + d1;
	cout << d1 << " + " << d2 << " = " << d3 << endl;

	nrOfFailedTestCases += ValidateAddition("addition", 10, true);

	exit(0);
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


#else
	std::cout << "Decimal Arithmetic verfication" << std::endl;

#ifdef STRESS_TESTING


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
