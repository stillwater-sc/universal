//  arithmetic.cpp : bitset-based arithmetic operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// this comparison is for a two's complement number only
template<size_t nbits>
bool operator< (const std::bitset<nbits>& lhs, const std::bitset<nbits>& rhs) {
	// comparison of the sign bit
	if (lhs[nbits - 1] == 0 && rhs[nbits - 1] == 1)	return false;
	if (lhs[nbits - 1] == 1 && rhs[nbits - 1] == 0) return true;
	// sign is equal, compare the remaining bits
	for (int i = nbits - 2; i >= 0; --i) {
		if (lhs[i] == 0 && rhs[i] == 1)	return true;
		if (lhs[i] == 1 && rhs[i] == 0) return false;
	}
	// numbers are equal
	return false;
}

// add bitsets a and b and return in bitset sum. Return true if there is a carry generated.
template<size_t nbits>
bool add_unsigned(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<nbits+1>& sum) {
	uint8_t carry = 0;  // ripple carry
	for (int i = 0; i < nbits; i++) {
		uint8_t _a = a[i];
		uint8_t _b = b[i];
		uint8_t _slice = _a + _b + carry;
		carry = _slice >> 1;
		sum[i] = (0x1 & _slice);
	}
	sum.set(nbits, carry);
	return carry;
}

template<size_t src_size, size_t tgt_size>
void copy_into(std::bitset<src_size>& src, unsigned int shift, std::bitset<tgt_size>& tgt) {
	tgt.reset();
	for (int i = 0; i < src_size; i++) { tgt.set(i+shift, src[i]); }
}

template<size_t src_size, size_t tgt_size>
bool accumulate(const std::bitset<src_size>& addend, std::bitset<tgt_size>& accumulator) {
	uint8_t carry = 0;  // ripple carry
	for (int i = 0; i < src_size; i++) {
		uint8_t _a = addend[i];
		uint8_t _b = accumulator[i];
		uint8_t _slice = _a + _b + carry;
		carry = _slice >> 1;
		accumulator[i] = (0x1 & _slice);
	}
	accumulator.set(src_size, carry);
	return carry;
}

// multiply bitsets a and b and return in bitset mul. Return true if there is a carry generated.
template<size_t nbits>
void multiply_unsigned(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<2*nbits + 1>& accumulator) {
	bool carry = false;
	std::bitset<2 * nbits> addend;
	accumulator.reset();
	if (a.test(0)) {
		copy_into<nbits, 2 * nbits+1>(b, 0, accumulator);
	}
	for (int i = 1; i < nbits; i++) {
		if (a.test(i)) {
			copy_into<nbits, 2 * nbits>(b, i, addend);
			accumulate(addend, accumulator);
		}
	}
}

// increment the input bitset in place, and return true if there is a carry generated.
template<size_t nbits>
bool increment_unsigned(std::bitset<nbits>& number) {
	uint8_t carry = 1;  // ripple carry
	uint8_t _a, _slice;
	for (int i = 0; i < nbits; i++) {
		_a = number[i];
		_slice = _a + 0 + carry;
		carry = _slice >> 1;
		number[i] = (0x1 & _slice);
	}
	return carry;
}

// increment the input bitset in place, and return true if there is a carry generated.
// The input number is assumed to be right adjusted starting at nbits-nrBits
// [1 0 0 0] nrBits = 0 is a noop as there is no word to increment
// [1 0 0 0] nrBits = 1 is the word [1]
// [1 0 0 0] nrBits = 2 is the word [1 0]
// [1 1 0 0] nrBits = 3 is the word [1 1 0], etc.
template<size_t nbits>
bool increment_unsigned(std::bitset<nbits>& number, int nrBits = nbits - 1) {
	uint8_t carry = 1;  // ripple carry
	uint8_t _a, _slice;
	int lsb = nbits - nrBits;
	for (int i = lsb; i < nbits; i++) {
		_a = number[i];
		_slice = _a + 0 + carry;
		carry = _slice >> 1;
		number[i] = (0x1 & _slice);
	}
	return carry;
}

// increment the input bitset in place, assuming it is representing a 2's complement number
template<size_t nbits>
void increment_twos_complement(std::bitset<nbits>& number) {
	uint8_t carry = 1;  // ripple carry
	uint8_t _a, _slice;
	for (int i = 0; i < nbits; i++) {
		_a = number[i];
		_slice = _a + 0 + carry;
		carry = _slice >> 1;
		number[i] = (0x1 & _slice);
	}
	// ignore any carry bits
}

// decrement the input bitset in place, assuming it is representing a 2's complement number
template<size_t nbits>
void decrement_twos_complement(std::bitset<nbits>& number) {
	std::bitset<nbits> minus_one;
	minus_one.set();
	uint8_t carry = 0;  // ripple carry
	uint8_t _a, _b, _slice;
	for (int i = 0; i < nbits; i++) {
		_a = number[i];
		_b = minus_one[i];
		_slice = _a + _b + carry;
		carry = _slice >> 1;
		number[i] = (0x1 & _slice);
	}
	// ignore any carry bits
}

template<size_t nbits>
bool add_signed_magnitude(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<nbits>& sum) {
	uint8_t carry = 0;
	bool sign_a = a.test(nbits - 1);
	if (sign_a) {
		a = a.flip();
		carry += 1;
	}
	bool sign_b = b.test(nbits - 1);
	if (sign_b) {
		b = b.flip();
		carry += 1;
	}

	for (int i = 0; i < nbits - 2; i++) {
		uint8_t _a = a[i];
		uint8_t _b = b[i];
		uint8_t _slice = _a + _b + carry;
		carry = _slice >> 1;
		sum[i] = (0x1 & _slice);
	}

	return carry;
}

template<size_t nbits>
bool subtract_signed_magnitude(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<nbits>& diff) {
	bool sign_a = a.test(nbits - 1);
	bool sign_b = b.test(nbits - 1);
	std::cerr << "subtract_signed_magnitude not implemented yet" << std::endl;
	return false;
}