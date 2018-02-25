//  arithmetic.cpp :  test suite for bitblock-based arithmetic operators
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <sstream>

#include "../../posit/exceptions.hpp"
#include "../../bitblock/bitblock.hpp"
#include "../tests/test_helpers.hpp"
#include "../bitblock_test_helpers.hpp"

using namespace std;
using namespace sw::unum;


int Conversions() {
	const size_t nbits = 33;
	int nrOfFailedTestCases = 0;
	bitblock<nbits> a, b, ref, sum;
	bool carry = 0;

	cout << "Binary conversions" << endl;

	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0x155555555));
	a = flip_sign_bit(convert_to_bitblock<nbits,uint64_t>(uint64_t(0x55555555)));
	nrOfFailedTestCases += (a != ref ? 1 : 0);

	b = convert_to_bitblock<nbits,uint64_t>(uint64_t(0x5));

	cout << "1's complement of a = " << to_binary(ones_complement(a)) << endl;
	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0xAAAAAAAA));
	nrOfFailedTestCases += (ones_complement(a) != ref ? 1 : 0);
	cout << "1's complement of b = " << to_binary(ones_complement(b)) << endl;
	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0x1FFFFFFFA));
	nrOfFailedTestCases += (ones_complement(b) != ref ? 1 : 0);

	const size_t nnbits = 9;
	bitblock<nnbits> c, ref2;
	c = convert_to_bitblock<9,int8_t>(int8_t(-128));  // this looks like -1 for a 9bit posit
	cout << "c                   = " << to_binary(c) << endl;
	ref2 = convert_to_bitblock<nnbits, uint64_t>(uint64_t(0x180));
	nrOfFailedTestCases += (c != ref2 ? 1 : 0);

	c = twos_complement(c);							// this looks like  1 for a 9bit posit
	cout << "2's Complement      = " << to_binary(c) << endl;
	ref2 = convert_to_bitblock<nnbits, uint64_t>(uint64_t(0x080));
	nrOfFailedTestCases += (c != ref2 ? 1 : 0);

	bitblock<9> d;
	d = convert_to_bitblock<9,int64_t>(int64_t(int8_t(-128)));
	cout << "d                   = " << to_binary(d) << endl;
	d = twos_complement(d);
	cout << "2's complement      = " << to_binary(d) << endl;
	cout << endl;
	nrOfFailedTestCases += (c != d ? 1 : 0);

	return nrOfFailedTestCases;
}


// ? what is this trying to test TODO
int IncrementRightAdjustedBitset()
{
	const size_t nbits = 5;
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;

	bitblock<nbits> r1, ref;
	bool carry;

	cout << "Increments" << endl;
	for (std::size_t i = 0; i < nbits; i++) {
		r1.reset();
		r1.set(nbits - 1 - i, true);
		carry = false;
		cout << "carry " << (carry ? "1" : "0") << " r1 " << r1 << " <-- input" << endl;
		carry = increment_unsigned(r1, int(i));
		cout << "carry " << (carry ? "1" : "0") << " r1 " << r1 << " <-- result" << endl;
	}

	return nrOfFailedTestCases;
}

template<size_t src_size, size_t tgt_size>
int VerifyCopyInto(bool bReportIndividualTestCases = false) {
	int nrOfFailedTestCases = 0;

	bitblock<src_size> operand;
	bitblock<tgt_size> addend;
	bitblock<tgt_size> reference;
	
	// use a programmatic pattern of alternating bits
	// so it is easy to spot any differences
	for (int i = 0; i < src_size; i = i + 2) {
		reference.set(i, true);
		operand.set(i, true);
	}

	for (int i = 0; i < tgt_size - src_size; i++) {
		copy_into<src_size, tgt_size>(operand, i, addend);

		if (reference != addend) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) cout << "FAIL operand : " << operand << " at i=" << i << " result   : " << addend << " reference: " << reference << endl;
		}
		else {
			if (bReportIndividualTestCases) cout << "PASS operand : " << operand << " at i=" << i << " result   : " << addend << " reference: " << reference << endl;
		}


		reference <<= 1; // each time around the loop, shift left by 1	
	}

	return nrOfFailedTestCases;
}

template<size_t src_size, size_t tgt_size>
int VerifyAccumulation() {
	int nrOfFailedTestCases = 0;

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Bitblock arithmetic operation failed";

#if MANUAL_TESTING
	const size_t nbits = 8;
	bitblock<nbits> a = convert_to_bitblock<nbits, uint32_t>(55);
	bitblock<nbits> b = convert_to_bitblock<nbits, uint32_t>(5);
	bitblock<nbits> r = convert_to_bitblock<nbits, uint32_t>(11);
	bitblock<nbits+1> sum, diff;
	bool borrow = subtract_unsigned(a, b, diff);
	cout << diff << " borrow " << borrow << endl;
	bool carry = add_unsigned(a, twos_complement(b), diff);
	cout << diff << " carry  " << carry << endl;
	bitblock<2 * nbits> mul;
	multiply_unsigned(a, b, mul);
	cout << "mul " << mul << endl;
	cout << "a   " << a << endl;
	cout << "b   " << b << endl;
	cout << "ref " << r << endl;
	bitblock<2*nbits> div;
	integer_divide_unsigned(a, b, div);
	cout << "div " << div << endl;

	constexpr size_t result_size = 2 * nbits + 3;
	bitblock<result_size> div_with_fraction;
	a = convert_to_bitblock<nbits, uint32_t>(0x80);  // representing 1.0000000
	b = convert_to_bitblock<nbits, uint32_t>(0xA0);  // representing 1.0100000
	divide_with_fraction(a, b, div_with_fraction);
	cout << "a      " << a << endl;
	cout << "b      " << b << endl;
	cout << "div with fraction " << div_with_fraction << endl;
	// radix point comes out at at result_size - operand_size
	div_with_fraction <<= result_size - nbits;
	cout << "result " << div_with_fraction << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<3>(true), "bitblock<3>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<3>(true), "bitblock<3>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<3>(true), "bitblock<3>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<3>(true), "bitblock<3>", "/");

#else

	cout << "Test of operators on bitblocks" << endl;
	nrOfFailedTestCases += Conversions();

	cout << "Register management" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<3, 8>(bReportIndividualTestCases),   "bitblock<  5>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<4, 8>(bReportIndividualTestCases),   "bitblock<  8>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 16>(bReportIndividualTestCases),  "bitblock< 16>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 24>(bReportIndividualTestCases),  "bitblock< 24>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 32>(bReportIndividualTestCases),  "bitblock< 32>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 48>(bReportIndividualTestCases),  "bitblock< 48>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 64>(bReportIndividualTestCases),  "bitblock< 64>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 128>(bReportIndividualTestCases), "bitblock<128>", "copyInto");

	cout << "Arithmetic: addition" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<3>(bReportIndividualTestCases), "bitblock<3>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<4>(bReportIndividualTestCases), "bitblock<4>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<5>(bReportIndividualTestCases), "bitblock<5>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<6>(bReportIndividualTestCases), "bitblock<6>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<7>(bReportIndividualTestCases), "bitblock<7>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<8>(bReportIndividualTestCases), "bitblock<8>", "+");

	cout << "Arithmetic: subtraction" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<3>(bReportIndividualTestCases), "bitblock<3>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<4>(bReportIndividualTestCases), "bitblock<4>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<5>(bReportIndividualTestCases), "bitblock<5>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<6>(bReportIndividualTestCases), "bitblock<6>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<7>(bReportIndividualTestCases), "bitblock<7>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<8>(bReportIndividualTestCases), "bitblock<8>", "-");

	cout << "Arithmetic: multiplication" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<3>(bReportIndividualTestCases), "bitblock<3>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<4>(bReportIndividualTestCases), "bitblock<4>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<5>(bReportIndividualTestCases), "bitblock<5>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<6>(bReportIndividualTestCases), "bitblock<6>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<7>(bReportIndividualTestCases), "bitblock<7>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<8>(bReportIndividualTestCases), "bitblock<8>", "*");

	cout << "Arithmetic: division" << endl;
	bitblock<8> a, b;
	bitblock<16> c;
	try {
		integer_divide_unsigned(a, b, c); // divide by zero
	}
	catch (runtime_error& e) {
		cout << "Properly caught exception: " << e.what() << endl;
	}

	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<3>(bReportIndividualTestCases), "bitblock<3>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<4>(bReportIndividualTestCases), "bitblock<4>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<5>(bReportIndividualTestCases), "bitblock<5>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<6>(bReportIndividualTestCases), "bitblock<6>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<7>(bReportIndividualTestCases), "bitblock<7>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<8>(bReportIndividualTestCases), "bitblock<8>", "/");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<16>(bReportIndividualTestCases), "bitblock<8>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<16>(bReportIndividualTestCases), "bitblock<8>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<16>(bReportIndividualTestCases), "bitblock<8>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<16>(bReportIndividualTestCases), "bitblock<8>", "/");

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
