//  arithmetic.cpp : bitset-based arithmetic tests
//

#include "stdafx.h"
#include <sstream>
#include "../../bitset/bitset_helpers.hpp"

using namespace std;

void BasicArithmeticTests() {
	const size_t nbits = 33;

	cout << "Bitset arithmetic tests" << endl;

	std::bitset<nbits> a, b, sum;
	bool carry = 0;
	a = flip_sign_bit(assign_unsigned<nbits>(uint64_t(0x55555555)));
	b = assign_unsigned<nbits>(uint64_t(0x5));

	carry = add_signed_magnitude(a, b, sum);

	cout << "a   = " << a << endl;
	cout << "b   = " << to_binary(b) << endl;
	cout << "sum = " << to_binary(sum) << " carry = " << carry << endl;

	cout << "1's complement of a = " << to_binary(ones_complement(a)) << endl;
	cout << "1's complement of b = " << to_binary(ones_complement(b)) << endl;

	std::bitset<9> c;
	c = assign_signed_magnitude<9>(int8_t(-128));
	cout << "c                = " << to_binary(c) << endl;
	c = twos_complement(c);
	cout << "2's Complement   = " << to_binary(c) << endl;

	std::bitset<9> d;
	d = assign_signed_magnitude<9>(int64_t(int8_t(-128)));
	cout << "d                = " << to_binary(d) << endl;
	d = twos_complement(d);
	cout << "2's complement   = " << to_binary(d) << endl;
}

template<size_t nbits>
void add_fractions(int f1_scale, std::bitset<nbits> f1, int f2_scale, std::bitset<nbits> f2, int& sum_scale, std::bitset<nbits>& sum) {
	// fraction operations that are part of adding posits

	cout << "f1 scale " << f1_scale << " value " << to_hex(f1) << endl;
	cout << "f2 scale " << f2_scale << " value " << to_hex(f2) << endl;

	// scale the smaller number to the bigger number by right shifting the difference
	// fractions are right extended, and the hidden bit becomes the msb
	f1.set(nbits - 1);
	f2.set(nbits - 1);
	int diff = f1_scale - f2_scale;
	cout << "scale difference is " << diff << endl;

	if (diff < 0) {
		f1 >>= -diff;	// denormalize
	}
	else {
		f2 >>= diff;	// denormalize
	}
	cout << "f1  : " << f1 << endl;
	cout << "f2  : " << f2 << endl;

	bool carry = add_unsigned(f1, f2, sum);
	cout << "sum : " << sum << " carry : " << (carry ? 1 : 0) << endl; 

	sum_scale = (f1_scale > f2_scale ? f1_scale : f2_scale);
	sum_scale = f1_scale + (carry ? 1 : 0);
	sum >>= (carry ? 1 : 0);
}

int main()
try
{
	// BasicArithmeticTests();
	const size_t posit_nbits = 16;
	const size_t fraction_nbits = posit_nbits - 2;
	int f1_scale = 5;
	std::bitset<fraction_nbits> f1 = assign_unsigned<fraction_nbits>(uint64_t(0x1fff));
	int f2_scale = 3;
	std::bitset<fraction_nbits> f2 = assign_unsigned<fraction_nbits>(uint64_t(0xf));
	int sum_scale = 0;
	std::bitset<fraction_nbits> sum;
	add_fractions<fraction_nbits>(f1_scale, f1, f2_scale, f2, sum_scale, sum);
	cout << "sum : " << sum << " sum_scale : " << sum_scale << endl;

	return 0;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
