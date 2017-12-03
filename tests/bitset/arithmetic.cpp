//  arithmetic.cpp : bitset-based arithmetic tests
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <sstream>
#include "../tests/test_helpers.hpp"
#include "../../bitset/bitset_arithmetic.hpp"
#include "../../bitset/bitset_helpers.hpp"

using namespace std;
using namespace sw::unum;

int Conversions() {
	const size_t nbits = 33;
	int nrOfFailedTestCases = 0;
	std::bitset<nbits> a, b, ref, sum;
	bool carry = 0;

	cout << "Binary conversions" << endl;

	ref = convert_to_bitset<nbits, uint64_t>(uint64_t(0x155555555));
	a = flip_sign_bit(convert_to_bitset<nbits,uint64_t>(uint64_t(0x55555555)));
	nrOfFailedTestCases += (a != ref ? 1 : 0);

	b = convert_to_bitset<nbits,uint64_t>(uint64_t(0x5));

	cout << "1's complement of a = " << to_binary(ones_complement(a)) << endl;
	ref = convert_to_bitset<nbits, uint64_t>(uint64_t(0xAAAAAAAA));
	nrOfFailedTestCases += (ones_complement(a) != ref ? 1 : 0);
	cout << "1's complement of b = " << to_binary(ones_complement(b)) << endl;
	ref = convert_to_bitset<nbits, uint64_t>(uint64_t(0x1FFFFFFFA));
	nrOfFailedTestCases += (ones_complement(b) != ref ? 1 : 0);

	const size_t nnbits = 9;
	std::bitset<nnbits> c, ref2;
	c = convert_to_bitset<9,int8_t>(int8_t(-128));  // this looks like -1 for a 9bit posit
	cout << "c                   = " << to_binary(c) << endl;
	ref2 = convert_to_bitset<nnbits, uint64_t>(uint64_t(0x180));
	nrOfFailedTestCases += (c != ref2 ? 1 : 0);

	c = twos_complement(c);							// this looks like  1 for a 9bit posit
	cout << "2's Complement      = " << to_binary(c) << endl;
	ref2 = convert_to_bitset<nnbits, uint64_t>(uint64_t(0x080));
	nrOfFailedTestCases += (c != ref2 ? 1 : 0);

	std::bitset<9> d;
	d = convert_to_bitset<9,int64_t>(int64_t(int8_t(-128)));
	cout << "d                   = " << to_binary(d) << endl;
	d = twos_complement(d);
	cout << "2's complement      = " << to_binary(d) << endl;
	cout << endl;
	nrOfFailedTestCases += (c != d ? 1 : 0);

	return nrOfFailedTestCases;
}

template<size_t nbits>
int ValidateBitsetAddition() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bool carry;
	std::bitset<nbits> a, b;
	std::bitset<nbits+1> bsum, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
			ref = i + j;
			bref = convert_to_bitset<nbits + 1, unsigned>(ref);
			carry = add_unsigned(a, b, bsum);
			if (bref != bsum) {
				nrOfFailedTestCases++;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits>
int ValidateBitsetMultiplication() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	std::bitset<nbits> a, b;
	std::bitset<2*nbits + 1> bmul, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
			ref = i * j;
			bref = convert_to_bitset<2*nbits + 1, unsigned>(ref);
			multiply_unsigned(a, b, bmul);
			if (bref != bmul) {
				nrOfFailedTestCases++;
			}
			//cout << "ref  " << ref << " = " << i << " * " << j << endl;
			//cout << "bref " << bref << endl;
			//cout << "bmul " << bmul << endl;
		}
	}
	return nrOfFailedTestCases;
}

// ? what is this trying to test TODO
int IncrementRightAdjustedBitset()
{
	const size_t nbits = 5;
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;

	std::bitset<nbits> r1, ref;
	bool carry;

	cout << "Increments" << endl;
	for (std::size_t i = 0; i < nbits; i++) {
		r1.reset();
		r1.set(nbits - 1 - i, true);
		carry = false;
		cout << "carry " << (carry ? "1" : "0") << " r1 " << r1 << " <-- input" << endl;
		carry = increment_unsigned(r1, i);
		cout << "carry " << (carry ? "1" : "0") << " r1 " << r1 << " <-- result" << endl;
	}

	return nrOfFailedTestCases;
}

template<size_t src_size, size_t tgt_size>
int VerifyCopyInto() {
	int nrOfFailedTestCases = 0;

	std::bitset<src_size> operand;
	std::bitset<tgt_size> addend;
	std::bitset<tgt_size> reference;
	
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
			cout << "result   : " << addend << endl;
			cout << "reference: " << reference << endl;
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

int main(int argc, char** argv)
try {
	int nrOfFailedTestCases = 0;

	cout << "Test of operators on bitsets" << endl;
	nrOfFailedTestCases += Conversions();

	cout << "Register management" << endl;
	nrOfFailedTestCases += VerifyCopyInto<3, 7>();
	nrOfFailedTestCases += VerifyCopyInto<4, 7>();
	nrOfFailedTestCases += VerifyCopyInto<8, 16>();

	cout << "Arithmetic: addition" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<3>(), "bitset<3>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<4>(), "bitset<4>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<5>(), "bitset<5>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<6>(), "bitset<6>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<7>(), "bitset<7>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<8>(), "bitset<8>", "+");

	cout << "Arithmetic: multiplication" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<3>(), "bitset<3>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<4>(), "bitset<4>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<5>(), "bitset<5>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<6>(), "bitset<6>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<7>(), "bitset<7>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<8>(), "bitset<8>", "*");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* e) {
	cerr << e << endl;
	return EXIT_FAILURE;
}
