//  arithmetic.cpp : bitset-based arithmetic tests
//

#include "stdafx.h"
#include <sstream>
#include "../../bitset/bitset_helpers.hpp"

using namespace std;

int main()
try
{
	const size_t nbits = 33;

	cout << "Bitset arithmetic tests" << endl;

	std::bitset<nbits> a, b, sum;
	bool carry = 0;
	a = assign<nbits>(uint64_t(0xfedcba98));
	b = assign<nbits>(uint64_t(1));

	carry = add(a, b, sum);

	cout << "a   = " << to_binary(a) << endl;
	cout << "b   = " << to_binary(b) << endl;
	cout << "sum = " << to_binary(sum) << " carry = " << carry << endl;

	cout << "1's complement of a = " << to_binary(ones_complement(a)) << endl;
	cout << "1's complement of b = " << to_binary(ones_complement(b)) << endl;

	std::bitset<9> c;
	c = assign<9>(int8_t(-128));
	cout << "c                = " << to_binary(c) << endl;
	c = twos_complement(c);
	cout << "2's Complement   = " << to_binary(c) << endl;

	std::bitset<9> d;
	d = assign<9>(int64_t(int8_t(-128)));
	cout << "d                = " << to_binary(d) << endl;
	d = twos_complement(d);
	cout << "2's complement   = " << to_binary(d) << endl;

	return 0;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
