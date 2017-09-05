#pragma once

// number representation is sign-magnitude, sign is msb

template<size_t sign_magnitude>
std::bitset<sign_magnitude> ones_complement(std::bitset<sign_magnitude> number) {
	std::bitset<sign_magnitude> complement;
	for (int i = 0; i < sign_magnitude; i++) {
		complement.set(i, !number[i]);
	}
	return complement;
}

template<size_t sign_magnitude>
std::bitset<sign_magnitude> twos_complement(std::bitset<sign_magnitude> number) {
	std::bitset<sign_magnitude> complement;
	for (int i = 0; i < sign_magnitude; i++) {
		complement.set(i, !number[i]);
	}
	bool carry = increment(complement);
	return complement;
}

template<size_t sign_magnitude>
std::bitset<sign_magnitude> assign(uint64_t number) {
	std::bitset<sign_magnitude> _Bits;
	_Bits[sign_magnitude - 1] = 0;
	uint64_t mask = 1;
	for (int i = 0; i < sign_magnitude-1; i++) {
		_Bits[i] = mask & number;
		mask <<= 1;
	}
	return _Bits;
}

template<size_t sign_magnitude>
std::bitset<sign_magnitude> assign(int8_t number) {
	std::bitset<sign_magnitude> _Bits;
	_Bits[sign_magnitude - 1] = number >> 7;
	uint64_t mask = 1;
	for (int i = 0; i < sign_magnitude - 1; i++) {
		_Bits[i] = mask & number;
		mask <<= 1;
	}
	return _Bits;
}

// little endian, sign bit is at sign_magnitude-1
template<size_t sign_magnitude>
std::bitset<sign_magnitude> assign(int64_t number) {
	std::bitset<sign_magnitude> _Bits;
	_Bits[sign_magnitude - 1] = number >> 63;
	uint64_t mask = 1;
	for (int i = 0; i < sign_magnitude - 1; i++) {
		_Bits[i] = mask & number;
		mask <<= 1;
	}
	return _Bits;
}

template<size_t sign_magnitude>
std::string to_binary(std::bitset<sign_magnitude> bits) {
	std::stringstream ss;
	ss << (bits[sign_magnitude - 1] ? "n-" : "p-");
	for (int i = sign_magnitude - 2; i >= 0; --i) {
		ss << (bits[i] ? "1" : "0");
	}
	return ss.str();
}

template<size_t sign_magnitude>
bool increment(std::bitset<sign_magnitude>& number) {
	uint8_t carry = 0;  // ripple carry
	uint8_t _a = number[0];
	uint8_t _b = 1;
	uint8_t _slice = _a + _b;
	for (int i = 1; i < sign_magnitude - 1; i++) {
		_a = number[i];
		_slice = _a + 0 + carry;
		carry = _slice >> 1;
		number[i] = (0x1 & _slice);
	}
	return carry;
}

template<size_t sign_magnitude>
bool add(std::bitset<sign_magnitude> a, std::bitset<sign_magnitude> b, std::bitset<sign_magnitude>& sum) {
	uint8_t carry = 0;  // ripple carry
	for (int i = 0; i < sign_magnitude - 1; i++) {
		uint8_t _a = a[i];
		uint8_t _b = b[i];
		uint8_t _slice = _a + _b + carry;
		carry = _slice >> 1;
		sum[i] = (0x1 & _slice);
	}
	return carry;
}

template<size_t sign_magnitude>
bool subtract(std::bitset<sign_magnitude> a, std::bitset<sign_magnitude> b, std::bitset<sign_magnitude>& diff) {
	uint8_t borrow = 0;  // ripple carry
	for (int i = 0; i < sign_magnitude -1; i++) {
		uint8_t _a = a[i];
		uint8_t _b = b[i];
		uint8_t _slice = _a + _b + carry;
		carry = _slice >> 1;
		sum[i] = (0x1 & _slice);
	}
	return carry;
}