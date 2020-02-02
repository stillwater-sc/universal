//  arithmetic.cpp : arithmetic test suite for abitrary precision integers
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
// is representable
#include <universal/functions/isrepresentable.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/integer_test_helpers.hpp"

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

#include <typeinfo>
template<typename Scalar>
void GenerateDivTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::unum;
	z = x / y;
	std::cout << typeid(Scalar).name() << ": " << x << " / " << y << " = " << z << std::endl;
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

// ExamplePattern to check that short and integer<16> do exactly the same
void ExamplePattern() {
	short s = 0;
	GenerateDivTest<short>(2, 16, s);
	sw::unum::integer<16> z = 0;
	GenerateDivTest<sw::unum::integer<16> >(2, 16, z);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "Integer Arithmetic tests failed";

#if MANUAL_TESTING

	integer<12> a, b, c;
	a = 10000;
	b = 100;
	GenerateDivTest(a, b, c);

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

	// samples of number systems
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4>(tag, bReportIndividualTestCases), "integer<4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<6>(tag, bReportIndividualTestCases), "integer<6>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8>(tag, bReportIndividualTestCases), "integer<8>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<10>(tag, bReportIndividualTestCases), "integer<10>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<12>(tag, bReportIndividualTestCases), "integer<12>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<4>(tag, bReportIndividualTestCases), "integer<4>", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<6>(tag, bReportIndividualTestCases), "integer<6>", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<8>(tag, bReportIndividualTestCases), "integer<8>", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<10>(tag, bReportIndividualTestCases), "integer<10>", "remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<12>(tag, bReportIndividualTestCases), "integer<12>", "remainder");

#if STRESS_TESTING
	type = "integer<16>";
	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivision(tag, bReportIndividualTestCases), "integer<16>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyShortRemainder(tag, bReportIndividualTestCases), "integer<16>", "remainder");
#define NBITS 16
	// this is a 'standard' comparision against a native int64_t
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<NBITS>(tag, bReportIndividualTestCases), "integer<16>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<NBITS>(tag, bReportIndividualTestCases), "integer<16>", "remainder");
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
