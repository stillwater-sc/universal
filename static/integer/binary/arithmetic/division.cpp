//  divide.cpp : test suite runner for division operator on fixed-size abitrary precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// configure the integer arithmetic class
// we need to enable exceptions to validate divide by zero and overflow conditions
// however, we also need to make this work with exceptions turned off: TODO
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/verification/integer_test_suite.hpp>
#include <universal/native/integers.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

#include <typeinfo>
template<typename Scalar>
void GenerateDivTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::universal;
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
		unsigned ad, anc, delta{ 0 }, q1, r1, q2, r2, t;
		const unsigned two31 = 0x80000000u;
		ad = static_cast<unsigned>(d == 0) ? 1u : abs(d);
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
		this->M = static_cast<int>(q2 + 1u);
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

int operator/(int dividend, const fastdiv& divisor) {
	int q = static_cast<int>((static_cast<unsigned long long>(divisor.M) * static_cast<unsigned long long>(dividend)) >> 32ull);
	q += dividend * divisor.n_add_sign;
	if (divisor.s >= 0)	{
		q >>= divisor.s; // we rely on this to be implemented as arithmetic shift
		q += (((unsigned)q) >> 31);
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
	using namespace sw::universal;

	std::cout << "\nTestFastdiv\n";
	// fast integer division by transformation to multiply with magic constant followed by a shift
	fid::fastdiv dummy(1);
	std::cout << "size of fastdiv: " << sizeof(dummy) << '\n';
	dummy.info();

	// int q = dividend / divisor;
	// int q = hi32bits(dividend * M) >> s;
	for (int i = 0; i < 10; i++) {
		int divisor = rand();
		fid::fastdiv fast_divisor(divisor);
		std::cout << "divisor : " << divisor << '\n';
		fast_divisor.info();
	}

	fid::check();
}

// ExamplePattern to check that short and integer<16> do exactly the same
void ExamplePattern() {
	short s = 0;
	GenerateDivTest<short>(2, 16, s);
	sw::universal::integer<16> z = 0;
	GenerateDivTest<sw::universal::integer<16> >(2, 16, z);
}

namespace sw { namespace universal {

	// enumerate all division cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
	int VerifyLimbsDivision(bool reportTestCases) {
		using Integer = integer<nbits, BlockType, NumberType>;
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);

		Integer ia, ib, iresult, iref, ir;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult.reduce(ia, ib, ir);
				}
				catch (const integer_divide_by_zero& e) {
					if (ib.iszero()) {
						// correctly caught the exception
						continue;
					}
					else {
						std::cerr << "unexpected : " << e.what() << std::endl;
						nrOfFailedTests++;
					}
				}
				catch (const integer_overflow& e) {
					std::cerr << e.what() << std::endl;
					// TODO: how do you validate the overflow?
				}
				catch (...) {
					std::cerr << "unexpected exception" << std::endl;
					nrOfFailedTests++;
				}
#else
				iresult.reduce(ia, ib, ir);
#endif
				if (j == 0) {
					iref = 0; // or maxneg?
				}
				else {
					iref = i64a / i64b;
				}
				if (iresult != iref) {
					++nrOfFailedTests;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", ia, ib, iresult, iref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iresult, iref);
				}
				if (nrOfFailedTests > 4) return nrOfFailedTests;
			}
			//if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		//if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all division cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
	int VerifyIntegerDivision(bool reportTestCases) {
		using Integer = integer<nbits, BlockType, NumberType>;
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		size_t startValue = 0;
		if constexpr (NumberType == WholeNumber) {
			startValue = 1;
		}
		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = startValue; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = startValue; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia / ib;
					// std::cout << ia << " / " << ib << " = " << iresult << '\n';
				}
				catch (const integer_divide_by_zero& e) {
					if (ib.iszero()) {
						// correctly caught the exception
						continue;
					}
					else {
						std::cerr << "unexpected : " << e.what() << std::endl;
						nrOfFailedTests++;
					}
				}
				catch (const integer_overflow& e) {
					std::cerr << e.what() << std::endl;
					// TODO: how do you validate the overflow?
				}
				catch (const integer_encoding_exception& e) {
					if (i == 0 || j == 0 || ib > ia) {
						// correctly caught the encoding exception
						continue;
					}
					else {
						std::cerr << "unexpected : " << e.what() << std::endl;
						nrOfFailedTests++;
					}
				}
				catch (...) {
					std::cerr << "unexpected exception" << std::endl;
					nrOfFailedTests++;
				}
#else
				iresult.reduce(ia, ib, ir);
#endif
				if (j == 0) {
					iref = 0; // or maxneg?
				}
				else {
					iref = i64a / i64b;
				}
				if (iresult != iref) {
					++nrOfFailedTests;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", ia, ib, iresult, iref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iresult, iref);
				}
				if (nrOfFailedTests > 4) return nrOfFailedTests;
			}
			// if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
//		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}
} } // namespace sw::universal

template<size_t nbits, typename BlockType>
void TestIntegerDivideAndRemainder() {
	sw::universal::integer<nbits, BlockType> a, b, c, r, iresult;
	a = 1;
	b = -1;
	c = a / b;
	std::cout << a << " / " << b << " = " << c << '\n';

	iresult.reduce(a, b, r);
	std::cout << a << " / " << b << " = " << c << " and " << r << '\n';
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

	std::string test_suite  = "Integer Arithmetic Division verfication";
	std::string test_tag    = "integer<> division";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using BlockType = std::uint8_t;

	{
		integer<9, uint8_t, IntegerNumber> a, b, c, r, iresult;
		a = -256;
		b = 1;
		c = a / b;
		r = a % b;
		std::cout << a << " / " << b << " = " << c << '\n';
		std::cout << a << " % " << b << " = " << r << '\n';

		iresult.reduce(a, b, r);
		std::cout << a << " / " << b << " = " << iresult << " and " << r << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision< 9, uint8_t, IntegerNumber >(reportTestCases), "integer< 9, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision< 9, uint16_t, IntegerNumber>(reportTestCases), "integer< 9, uint16_t>", test_tag);


	// The long division algorithm assumes that the number system has a zero representation
	// as it repeated subtracts to zero out msbs. This is creating a problem for
	// supporting the divide operator for WholeNumber encodings: TODO
#ifdef WHOLENUMBER_DIVISION
	{
		integer<4, uint8_t, WholeNumber> a, b, c, r, iresult;
		a = 1;
		b = 1;
		c = a / b;
		std::cout << a << " / " << b << " = " << c << '\n';

		iresult.reduce(a, b, r);
		std::cout << a << " / " << b << " = " << iresult << " and " << r << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<4, uint8_t, WholeNumber>(reportTestCases), "integer<4, uint8_t, wholenumber>", test_tag);
#endif

	{
		integer<4, uint8_t, IntegerNumber> a, b, c;
		a = 1;
		b = 1;
		c = a / b;
		std::cout << a << " / " << b << " = " << c << '\n';
	}
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<4, uint8_t, IntegerNumber>(reportTestCases), "integer<4, uint8_t, integernumber>", test_tag);

	{
		integer<32, BlockType> a, b, c;
		a.setbits(0x08040201);
		b = 1;
		GenerateDivTest(a, b, c);
	}

	{
		std::cout << "Whole number divide\n";
		integer<8, BlockType, IntegerNumberType::WholeNumber> a;
		a = 127;
		std::cout << to_binary(a, true) << " : " << a << '\n';
		a += 1;
		std::cout << to_binary(a, true) << " : " << a << '\n';
		a = 127;
		a *= 2;
		std::cout << to_binary(a, true) << " : " << a << '\n';
	}

	{
		std::cout << "Integer Number\n";
		integer<32, BlockType, IntegerNumberType::IntegerNumber> signedInt;
		signedInt = 8;
		for (unsigned i = 0; i < 8; ++i) {
			std::cout << signedInt.showLimbs() << " : " << signedInt << '\n';
			signedInt *= static_cast<BlockType>(16u);
		}
		signedInt.setbits(0xFFFF'FFFF);
		std::cout << signedInt.showLimbs() << " : " << signedInt << '\n';

		// double check that the native type does exactly the same thing
		int32_t i = 134217728;
		std::cout << "int32_t : " << 16 * i << '\n';
	}

	{
		std::cout << "Whole Number\n";
		integer<32, BlockType, IntegerNumberType::WholeNumber> unsignedInt;
		unsignedInt = 8;
		for (unsigned i = 0; i < 8; ++i) {
			std::cout << unsignedInt.showLimbs() << " : " << unsignedInt << '\n';
			unsignedInt *= static_cast<BlockType>(16u);
		}
		for (unsigned i = 0; i < 4; ++i) {
			unsignedInt.setblock(i, 0xFFu);
			std::cout << unsignedInt.showLimbs() << " : " << unsignedInt << '\n';
		}
	}

//	TestNLZ();

	{
		integer<32, BlockType> a, b, q, r;
		a.setbits(0x18040201);
		b.setbits(0x08040200);
		b.setbits(0x0804);
		for (size_t i = 0; i < 1; ++i) {
			std::cout << std::endl;
			std::cout << "a        : " << a.showLimbs() << " : " << a.showLimbValues() << " : " << a << '\n';
			std::cout << "b        : " << b.showLimbs() << " : " << b.showLimbValues() << " : " << b << '\n';
			q.reduce(a, b, r);
			std::cout << "result of division : " << q.showLimbValues() << " : " << q << '\n';
			std::cout << "reference  /       : " << (a / b) << '\n';
			std::cout << "result of division : " << r.showLimbValues() << " : " << r << '\n';
			std::cout << "reference  %       : " << (a % b) << '\n';
			b <<= 1;
		}
	}

	// m, n, u...,          v...,          cq...,  cr....
	// 3, 3, 0x00000003, 0x00000000, 0x80000000, 0x00000001, 0x00000000, 0x20000000, 0x00000003, 0, 0, 0x20000000, // Adding back step req'd.
	// 3, 3, 0x00000003, 0x00000000, 0x00008000, 0x00000001, 0x00000000, 0x00002000, 0x00000003, 0, 0, 0x00002000, // Adding back step req'd.
	// 4, 3, 0, 0, 0x00008000, 0x00007fff, 1, 0, 0x00008000, 0xfffe0000, 0, 0x00020000, 0xffffffff, 0x00007fff,  // Add back req'd.
	{
		integer<96, uint32_t, IntegerNumberType::WholeNumber> a, b, q, r;
		a.setblock(2, 0x3ul);
		a.setblock(1, 0ul);
		a.setblock(0, 0x8000'0000ul);
		b.setblock(2, 0x1ul);
		b.setblock(1, 0ul);
		b.setblock(0, 0x2000'0000ul);
		q = a / b;
		r = a % b;
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		std::cout << to_binary(q) << " : " << q << '\n';
		std::cout << to_binary(r) << " : " << r << '\n';

		q.reduce(a, b, r);
		std::cout << to_binary(q) << " : " << q << '\n';
		std::cout << to_binary(r) << " : " << r << '\n';
	}

	{
		integer<32, uint8_t> a, b, q, r;
		a = -10;
		b = 2;
		q.reduce(a, b, r);
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		std::cout << to_binary(q) << " : " << q << '\n';
		std::cout << to_binary(r) << " : " << r << '\n';
	}

//	TestFastdiv();
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<4, uint8_t, IntegerNumber>(reportTestCases), "integer<4, uint8_t, IntegerNumber>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<11, uint8_t, IntegerNumber>(reportTestCases), "integer<11, uint8_t, IntegerNumber>", test_tag);

//	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<16, uint8_t, IntegerNumber>(reportTestCases), "integer<16, uint8_t, IntegerNumber>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<16, uint16_t, IntegerNumber>(reportTestCases), "integer<16, uint16_t, IntegerNumber>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyShortDivision<uint8_t>(reportTestCases), "integer<16, uint8_t >", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<4, uint8_t, IntegerNumber>(reportTestCases), "integer<4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<6, uint8_t, IntegerNumber>(reportTestCases), "integer<6, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<8, uint8_t, IntegerNumber>(reportTestCases), "integer<8, uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<4, uint8_t, IntegerNumber>(reportTestCases), "integer<4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<6, uint8_t, IntegerNumber>(reportTestCases), "integer<6, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<8, uint8_t, IntegerNumber>(reportTestCases), "integer<8, uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision< 9, uint8_t, IntegerNumber >(reportTestCases), "integer< 9, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision< 9, uint16_t, IntegerNumber>(reportTestCases), "integer< 9, uint16_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision< 9, uint8_t, IntegerNumber >(reportTestCases), "integer< 9, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision< 9, uint16_t, IntegerNumber>(reportTestCases), "integer< 9, uint16_t>", test_tag);

#ifdef	WHOLENUMBER_DIVISION
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<4, uint8_t, WholeNumber>(reportTestCases), "integer<4, uint8_t, wholenumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<6, uint8_t, WholeNumber>(reportTestCases), "integer<6, uint8_t, wholenumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<8, uint8_t, WholeNumber>(reportTestCases), "integer<8, uint8_t, wholenumber>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<4, uint8_t, WholeNumber>(reportTestCases), "integer<4, uint8_t, wholenumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<6, uint8_t, WholeNumber>(reportTestCases), "integer<6, uint8_t, wholenumber>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<8, uint8_t, WholeNumber>(reportTestCases), "integer<8, uint8_t, wholenumber>", test_tag);
#endif // WHOLENUMBER_DIVISION

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<11, uint8_t,  IntegerNumber >(reportTestCases), "integer<11, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivision<11, uint16_t, IntegerNumber>(reportTestCases), "integer<11, uint16_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<11, uint8_t,  IntegerNumber >(reportTestCases), "integer<11, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<11, uint16_t, IntegerNumber>(reportTestCases), "integer<11, uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<13, uint8_t,  IntegerNumber >(reportTestCases), "integer<13, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbsDivision<13, uint16_t, IntegerNumber>(reportTestCases), "integer<13, uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivision<uint8_t >(reportTestCases), "integer<16, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivision<uint16_t>(reportTestCases), "integer<16, uint16_t>", test_tag);
	// this is a 'standard' comparision against a native int64_t
//	nrOfFailedTestCases += ReportTestResult(VerifyDivision<16, uint8_t>(reportTestCases), "integer<16, uint8_t>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
