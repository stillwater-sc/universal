//  integers.cpp : test suite for abitrary precision integers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer.hpp>
#include <universal/integer/numeric_limits.hpp>
// test helpers
#include "../test_helpers.hpp"

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw {
namespace unum {


#define INTEGER_TABLE_WIDTH 20
	template<size_t nbits>
	void ReportBinaryArithmeticError(std::string test_case, std::string op, const integer<nbits>& lhs, const integer<nbits>& rhs, const integer<nbits>& pref, const integer<nbits>& presult) {
		std::cerr << test_case << " "
			<< std::setprecision(20)
			<< std::setw(INTEGER_TABLE_WIDTH) << lhs
			<< " " << op << " "
			<< std::setw(INTEGER_TABLE_WIDTH) << rhs
			<< " != "
			<< std::setw(INTEGER_TABLE_WIDTH) << pref << " instead it yielded "
			<< std::setw(INTEGER_TABLE_WIDTH) << presult
			<< " " << to_binary(pref) << " vs " << to_binary(presult)
			<< std::setprecision(5)
			<< std::endl;
	}

	// enumerate all addition cases for an integer<16> configuration compared against native short
	int VerifyShortAddition(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		short i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = short(ib);
				iref = i64a + i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia + ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia + ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all subtraction cases for an integer<16> configuration compared against native short
	int VerifyShortSubtraction(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i16b = short(ib);
				iref = i16a - i16b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia - ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia - ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all multiplication cases for an integer<16> configuration compared against native short
	int VerifyShortMultiplication(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i16b = short(ib);
				iref = i16a * i16b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia * ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia * ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all division cases for an integer<16> configuration compared against native short
	int VerifyShortDivision(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i16b = short(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia / ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}
#else
				iresult = ia / ib;
#endif
				iref = i16a / i16b;
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "/", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all remainder cases for an integer<16> configuration compared against native short
	int VerifyShortRemainder(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i16b = short(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia % ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}
#else
				iresult = ia % ib;
#endif
				iref = i16a % i16b;
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "%", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "%", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}

	// enumerate all addition cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyAddition(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
				iref = i64a + i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia + ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception
	
					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia + ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}
	// enumerate all subtraction cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifySubtraction(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
				iref = i64a - i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia - ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia - ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}
	// enumerate all multiplication cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyMultiplication(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
				iref = i64a * i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia * ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia * ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}
	// enumerate all division cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyDivision(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia / ib;
				}
				catch (...) {
					if (ib == integer<nbits>(0)) {
						// correctly caught the exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia / ib;
#endif
				iref = i64a / i64b;
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "/", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}
	template<size_t nbits>
	int VerifyRemainder(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia % ib;
				}
				catch (...) {
					if (ib == integer<nbits>(0)) {
						// correctly caught the exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia % ib;
#endif
				iref = i64a % i64b;
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "%", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "%", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}
}
}

#include <typeinfo>
template<typename Scalar>
void GenerateAddTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::unum;
	z = x + y;
	std::cout << typeid(Scalar).name() << ": " << x << " + " << y << " = " << z << std::endl;
}
template<typename Scalar>
void GenerateMulTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::unum;
	z = x * y;
	std::cout << typeid(Scalar).name() << ": " << x << " * " << y << " = " << z << std::endl;
}

namespace fid {
// fast integer divide
class fastdiv {
public:
	// divisor != 0 
	fastdiv(int divisor = 0) : d(divisor) {
		generate_magic_constants();
	}

	fastdiv& operator =(int divisor) {
		this->d = divisor;
		generate_magic_constants();
		return *this;
	}

	operator int() const { return d; }

	void info() {
		std::cout << "d   : " << d << std::endl;
		std::cout << "M   : " << M << std::endl;
		std::cout << "s   : " << s << std::endl;
		std::cout << "n_add_sign : " << n_add_sign << std::endl;
	}

private:
	int d;
	int M;
	int s;
	int n_add_sign;

	// Hacker's Delight, Second Edition, Chapter 10, Integer Division By Constants
	void generate_magic_constants()	{
		if (d == 1)	{
			M = 0;
			s = -1;
			n_add_sign = 1;
			return;
		}
		else if (d == -1) {
			M = 0;
			s = -1;
			n_add_sign = -1;
			return;
		}

		int p;
		unsigned int ad, anc, delta, q1, r1, q2, r2, t;
		const unsigned two31 = 0x80000000;
		ad = (d == 0) ? 1 : abs(d);
		t = two31 + ((unsigned int)d >> 31);
		anc = t - 1 - t % ad;
		p = 31;
		q1 = two31 / anc;
		r1 = two31 - q1 * anc;
		q2 = two31 / ad;
		r2 = two31 - q2 * ad;
		do {
			++p;
			q1 = 2 * q1;
			r1 = 2 * r1;
			if (r1 >= anc) {
				++q1;
				r1 -= anc;
			}
			q2 = 2 * q2;
			r2 = 2 * r2;
			if (r2 >= ad) {
				++q2;
				r2 -= ad;
			}
			delta = ad - r2;
		} while (q1 < delta || (q1 == delta && r1 == 0));
		this->M = q2 + 1;
		if (d < 0)		this->M = -this->M;
		this->s = p - 32;

		if ((d > 0) && (M < 0)) {
			n_add_sign = 1;
		} 
		else if ((d < 0) && (M > 0)) {
			n_add_sign = -1;
		}
		else {
			n_add_sign = 0;
		}
	}

	friend int operator/(const int dividend, const fastdiv& divisor);
};

int operator/(const int dividend, const fastdiv& divisor) {
	int q = (((unsigned long long)((long long)divisor.M * (long long)dividend)) >> 32);
	q += dividend * divisor.n_add_sign;
	if (divisor.s >= 0)	{
		q >>= divisor.s; // we rely on this to be implemented as arithmetic shift
		q += (((unsigned int)q) >> 31);
	}
	return q;
}

int operator%(const int dividend, const fastdiv& divisor){
	int quotient = dividend / divisor;
	int remainder = dividend - quotient * divisor;
	return remainder;
}

int operator/(const unsigned int n, const fastdiv& divisor)		{	return ((int)n) / divisor; }
int operator%(const unsigned int n, const fastdiv& divisor)		{	return ((int)n) % divisor; }
int operator/(const short n, const fastdiv& divisor)			{	return ((int)n) / divisor; }
int operator%(const short n, const fastdiv& divisor)			{	return ((int)n) % divisor; }
int operator/(const unsigned short n, const fastdiv& divisor)	{	return ((int)n) / divisor; }
int operator%(const unsigned short n, const fastdiv& divisor)	{	return ((int)n) % divisor; }
int operator/(const char n, const fastdiv& divisor)				{	return ((int)n) / divisor; }
int operator%(const char n, const fastdiv& divisor)				{	return ((int)n) % divisor; }
int operator/(const unsigned char n, const fastdiv& divisor)	{	return ((int)n) / divisor; }
int operator%(const unsigned char n, const fastdiv& divisor)	{	return ((int)n) % divisor; }


int check() {
	const int divisor_count = 10000;
	const int dividend_count = 10000;
	std::cout << "Functional test on " << divisor_count << " divisors, with " << dividend_count << " dividends for each divisor" << std::endl;
	for (int d = 1; d < divisor_count; ++d) {
		for (int sign = 1; sign >= -1; sign -= 2) {
			int divisor = d * sign;
			fastdiv fast_divisor(divisor);

			for (int dd = 0; dd < dividend_count; ++dd) {
				for (int ss = 1; ss >= -1; ss -= 2) {
					int dividend = dd * ss;

					int quotient = dividend / divisor;
					int fast_quotient = dividend / fast_divisor;
					if (quotient != fast_quotient) {
						std::cout << "FAIL " << dividend << ", correct quotient = " << quotient << ", fast computed quotient = " << fast_quotient << std::endl;
						return 1;
					}
				}
			}
		}
	}

	return 0;
}
}

void TestSizeof() {
	using namespace std;
	using namespace sw::unum;

	cout << endl << "TestSizeof" << endl;
	bool pass = true;
	using int8 = integer<8>;
	using int64 = integer<64>;
	using int128 = integer<128>;
	using int1024 = integer<1024>;

	int8 a;
	int64 k;
	int128 m;
	int1024 o;

	constexpr int WIDTH = 30;
	cout << setw(WIDTH) << typeid(a).name() << "  size in bytes " << a.nrBytes << endl;
	cout << setw(WIDTH) << typeid(k).name() << "  size in bytes " << k.nrBytes << endl;
	cout << setw(WIDTH) << typeid(m).name() << "  size in bytes " << m.nrBytes << endl;
	cout << setw(WIDTH) << typeid(o).name() << "  size in bytes " << o.nrBytes << endl;
	if (a.nrBytes != sizeof(a)) pass = false;
	if (k.nrBytes != sizeof(k)) pass = false;
	if (m.nrBytes != sizeof(m)) pass = false;
	if (o.nrBytes != sizeof(o)) pass = false;

	cout << (pass ? "PASS" : "FAIL") << endl;
}

void TestConversion() {
	using namespace std;
	using namespace sw::unum;

	cout << endl << "TestConversion" << endl;

	integer<128> i1, i2, i3;

	bool pass = true;
	constexpr int iconst = 123456789;
	i1 = iconst;
	int64_t ll = int64_t(i1);
	cout << "integer  " << i1 << endl;
	if (iconst != ll) pass = false;
	i2 = 1.23456789e8;
	cout << "double   " << i2 << " TBD " << endl;
	//i3.parse("123456789");

	cout << (pass ? "PASS" : "FAIL") << endl;
}

void TestFindMsb() {
	using namespace std;
	using namespace sw::unum;

	cout << endl << "TestFindMsb" << endl;
	bool pass = true;
	integer<32> a = 0xD5555555;
	int golden_ref[] = { 31, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0, -1 };
	for (int i = 0; i < int(sizeof(golden_ref)/sizeof(int)); ++i) {
		int msb = findMsb(a);
		cout << "msb of " << to_binary(a) << " is " << msb << endl;
		if (msb >= 0) a.reset(msb);
		if (msb != golden_ref[i]) pass = false;
	}

	cout << (pass ? "PASS" : "FAIL") << endl;
}

template<size_t nbits>
void TestLessThan() {
	using namespace std;
	using namespace sw::unum;

	cout << endl << "TestLessThen" << endl;
	bool pass = true;
	constexpr int NR_INTS = 1 << nbits;
	integer<nbits> a, b;
	int ia, ib;
	for (int i = 0; i < NR_INTS; ++i) {
		a.set_raw_bits(i);
		ia = int(a);
		for (int j = 0; j < NR_INTS; ++j) {
			b.set_raw_bits(j);
			ib = int(b);
			if ((ia < ib) != (a < b)) {
				cout << "FAIL : " << a << " " << b << " yielded " << (a < b ? "true" : "false") << endl;
				pass = false;
				i = NR_INTS;
				break;
			}
		}
	}
	cout << (pass ? "PASS" : "FAIL") << endl;
}

void TestFastdiv() {
	using namespace std;
	using namespace sw::unum;

	cout << endl << "TestFastdiv" << endl;
	// fast integer division by transformation to multiply with magic constant followed by a shift
	fid::fastdiv fast_divisor(1);
	cout << "size of fastdiv: " << sizeof(fast_divisor) << endl;
	fast_divisor.info();

	// int q = dividend / divisor;
	// int q = hi32bits(dividend * M) >> s;
	for (int i = 0; i < 10; i++) {
		int divisor = rand();
		fid::fastdiv fast_divisor(divisor);
		cout << "divisor : " << divisor << std::endl;
		fast_divisor.info();
	}

	fid::check();
}

#include <chrono>
template<size_t nbits>
void ShiftPerformanceTest() {
	using namespace std;
	using namespace std::chrono;

	constexpr uint64_t NR_OPS = 1000000;

	integer<nbits> a = 0xFFFFFFFF;
	steady_clock::time_point begin = steady_clock::now();
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		a >>= 8;
		a <<= 8;
	}
	steady_clock::time_point end = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(end - begin);;
	double elapsed = time_span.count();
	cout << "performance is " << double(NR_OPS) / elapsed << " integer<" << nbits << "> shifts/sec" << endl;
}

// do we need to fix the performance of the shift operator?
void TestShiftOperatorPerformance() {
	using namespace std;

	cout << endl << "TestShiftOperatorPerformance" << endl;

	ShiftPerformanceTest<16>();
	ShiftPerformanceTest<32>();
	ShiftPerformanceTest<64>();
	ShiftPerformanceTest<128>();
	ShiftPerformanceTest<1024>();
	/*
	performance of the serial implementation of the shift operators
		performance is 1.99374e+07 integer<16> shifts / sec
		performance is 8.44852e+06 integer<32> shifts / sec
		performance is 3.85375e+06 integer<64> shifts / sec
		performance is 1.77301e+06 integer<128> shifts / sec
		performance is 219793 integer<1024> shifts / sec
	*/
}

template<size_t nbits>
void ArithmeticPerformanceTest() {
	using namespace std;
	using namespace std::chrono;

	constexpr uint64_t NR_OPS = 1000000;

	steady_clock::time_point begin, end;
	duration<double> time_span;
	double elapsed;

	integer<nbits> a, b, c, d;
	for (int i = 0; i < int(a.nrBytes); ++i) {
		a.setbyte(i, rand());
		b.setbyte(i, rand());
	}
	begin = steady_clock::now();
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a + b;
		a = c - b;
	}
	end = steady_clock::now();
	time_span = duration_cast<duration<double>>(end - begin);;
	 elapsed = time_span.count();
	cout << "performance is " << double(NR_OPS) / elapsed << " integer<" << nbits << "> additions/subtractions" << endl;

	begin = steady_clock::now();
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a * b;
		c.clear();
		d = c;
	}
	end = steady_clock::now();
	time_span = duration_cast<duration<double>>(end - begin);;
	elapsed = time_span.count();
	cout << "performance is " << double(NR_OPS) / elapsed << " integer<" << nbits << "> multiplications" << endl;

	begin = steady_clock::now();
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a / b;
		c.clear();
		d = c;
	}
	end = steady_clock::now();
	time_span = duration_cast<duration<double>>(end - begin);;
	elapsed = time_span.count();
	cout << "performance is " << double(NR_OPS) / elapsed << " integer<" << nbits << "> divisions" << endl;
}

void TestArithmeticOperatorPerformance() {
	using namespace std;

	cout << endl << "TestShiftOperatorPerformance" << endl;

	ArithmeticPerformanceTest<16>();
	ArithmeticPerformanceTest<32>();
	ArithmeticPerformanceTest<64>();
	ArithmeticPerformanceTest<128>();
//	ArithmeticPerformanceTest<1024>();
	/*
		TestShiftOperatorPerformance
		performance is 1.01249e+08 integer<16> additions/subtractions
		performance is 1.45226e+06 integer<16> multiplications
		performance is 3.05808e+07 integer<16> divisions
		performance is 6.75147e+07 integer<32> additions/subtractions
		performance is 366806 integer<32> multiplications
		performance is 1.93706e+06 integer<32> divisions
		performance is 2.11016e+07 integer<64> additions/subtractions
		performance is 93139 integer<64> multiplications
		performance is 4.24692e+07 integer<64> divisions
		performance is 1.29312e+07 integer<128> additions/subtractions
		performance is 23545.5 integer<128> multiplications
		performance is 543714 integer<128> divisions
		performance is 2.06385e+06 integer<1024> additions/subtractions
		performance is 407.244 integer<1024> multiplications
		performance is 2.58264e+06 integer<1024> divisions
	*/
}

// ExamplePattern to check that short and integer<16> do exactly the same
void ExamplePattern() {
	short s = 0;
	GenerateMulTest<short>(2, 16, s);
	sw::unum::integer<16> z = 0;
	GenerateMulTest<sw::unum::integer<16> >(2, 16, z);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

std::string convert_to_string(const std::vector<char>& v) {
	std::stringstream ss;
	for (std::vector<char>::const_reverse_iterator rit = v.rbegin(); rit != v.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ss.str();
}

int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "Integer Arithmetic tests failed";

#if MANUAL_TESTING

	TestSizeof();
	TestConversion();
	TestFindMsb();
	TestLessThan<12>();
//	TestShiftOperatorPerformance();
//	TestArithmeticOperatorPerformance();
//	TestFastdiv();
	ReportTestResult(VerifyDivision<4>("manual test", true), "integer<4>", "divides");
	ReportTestResult(VerifyRemainder<4>("manual test", true), "integer<4>", "remainder");
	ReportTestResult(VerifyDivision<11>("manual test", true), "integer<11>", "divides");
	ReportTestResult(VerifyRemainder<11>("manual test", true), "integer<11>", "remainder");

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Arithmetic verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	// allocation is the only functionality of integer<N> at this time

	// TODO: implement parsing, assigment, conversion, arithmetic
	std::string type = "integer<4>";
#define NBITS 4
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<NBITS>(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<NBITS>(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<NBITS>(tag, bReportIndividualTestCases), type, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<NBITS>(tag, bReportIndividualTestCases), type, "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<NBITS>(tag, bReportIndividualTestCases), type, "remainder");
#undef NBITS

	type = "integer<8>";
#define NBITS 8
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<NBITS>(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<NBITS>(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<NBITS>(tag, bReportIndividualTestCases), type, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<NBITS>(tag, bReportIndividualTestCases), type, "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<NBITS>(tag, bReportIndividualTestCases), type, "remainder");
#undef NBITS

	type = "integer<12>";
#define NBITS 12
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<NBITS>(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<NBITS>(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<NBITS>(tag, bReportIndividualTestCases), type, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<NBITS>(tag, bReportIndividualTestCases), type, "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<NBITS>(tag, bReportIndividualTestCases), type, "remainder");
#undef NBITS

#if STRESS_TESTING
	type = "integer<16>";
	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
	nrOfFailedTestCases += ReportTestResult(VerifyShortAddition(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyShortSubtraction(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyShortMultiplication(tag, bReportIndividualTestCases), type, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivision(tag, bReportIndividualTestCases), type, "division");
	nrOfFailedTestCases += ReportTestResult(VerifyShortRemainder(tag, bReportIndividualTestCases), type, "remainder");
#define NBITS 16
	// this is a 'standard' comparision against a native int64_t
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<NBITS>(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<NBITS>(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<NBITS>(tag, bReportIndividualTestCases), type, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<NBITS>(tag, bReportIndividualTestCases), type, "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<NBITS>(tag, bReportIndividualTestCases), type, "remainder");
#undef NBITS

#endif // STRESS_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
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
