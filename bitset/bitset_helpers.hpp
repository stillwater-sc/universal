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

template<size_t nbits>
bool increment_signed_magnitude(std::bitset<nbits>& number) {
	uint8_t carry = 0;  // ripple carry
	uint8_t _a = number[0];
	uint8_t _b = 1;
	uint8_t _slice = _a + _b;
	for (int i = 1; i < nbits - 1; i++) {
		_a = number[i];
		_slice = _a + 0 + carry;
		carry = _slice >> 1;
		number[i] = (0x1 & _slice);
	}
	return carry;
}

template<size_t nbits>
std::bitset<nbits> twos_complement(std::bitset<nbits> number) {
	std::bitset<nbits> complement;
	uint8_t _slice = 0;
	uint8_t carry = 1;
	for (int i = 0; i < nbits; i++) {
		_slice = uint8_t(!number[i]) + carry;
		carry = _slice >> 1;
		complement[i] = (0x1 & _slice);
	}
	return complement;
}

// DANGER: this depends on the implicit type conversion of number to a uint64_t to sign extent a 2's complement number system
// if nbits > 64 then this code breaks.
template<size_t nbits, class Type>
std::bitset<nbits> convert_to_bitset(Type number) {
	std::bitset<nbits> _Bits; 
	uint64_t mask = 1;
	for (int i = 0; i < nbits; i++) {
		_Bits[i] = mask & number;
		mask <<= 1;
	}
	return _Bits;
}

template<size_t nbits>
std::string to_binary(std::bitset<nbits> bits) {
	std::stringstream ss;
	int msb = nbits; // compilation warning work-around for nbits = 0
	for (int i = msb - 1; i >= 0; --i) {
		ss << (bits[i] ? "1" : "0");
	}
	return ss.str();
}

template<size_t nbits>
std::string to_hex(std::bitset<nbits> bits) {
	char str[nbits];   // plenty of room
	const char* hexits = "0123456789ABCDEF";
	unsigned int max = (nbits >> 2) + (nbits % 4 ? 2 : 1);
	for (unsigned int i = 0; i < max; i++) {
		unsigned int hexit = (bits[3] << 3) + (bits[2] << 2) + (bits[1] << 1) + bits[0];
		str[max-1-i] = hexits[hexit];
		bits >>= 4;
	}
	str[max] = 0;
	return std::string(str);
}

template<size_t sign_magnitude>
std::string signed_magnitude_to_binary(std::bitset<sign_magnitude> bits) {
	std::stringstream ss;
	ss << (bits[sign_magnitude - 1] ? "n-" : "p-");
	if (sign_magnitude < 2) return ss.str();
	for (int i = sign_magnitude - 2; i >= 0; --i) {
		ss << (bits[i] ? "1" : "0");
	}
	return ss.str();
}

template<size_t nbits>
std::bitset<nbits> flip_sign_bit(std::bitset<nbits> number) {
	number.flip(nbits - 1);
	return number;
}

template<size_t nbits>
bool add_unsigned(std::bitset<nbits> a, std::bitset<nbits> b, std::bitset<nbits>& sum) {
	uint8_t carry = 0;  // ripple carry
	for (int i = 0; i < nbits; i++) {
		uint8_t _a = a[i];
		uint8_t _b = b[i];
		uint8_t _slice = _a + _b + carry;
		carry = _slice >> 1;
		sum[i] = (0x1 & _slice);
	}
	return carry;
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
	return false;
}
