#pragma once
// byteArray.hpp: manipulators for byte arrays
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
namespace sw {
namespace native {

// determine the rounding mode: -1 round down, 0 tie, 1 round up
// N sets the size of the byteArray, and bit indicats the location of the guard bit to round at
int round(const uint8_t byteArray[], unsigned N, int bit) {
	int rv = 0;
	if (bit >= 0) {
		bool guard, round, sticky;
		// byte in which the guard bit resides
		uint8_t byte = byteArray[bit / 8];
		uint8_t mask = 1 << (bit % 8);
		guard = (byte & mask) ? true : false;
		if (guard == false) return -1; // round down
		rv = 1; // round up, unless it is a tie
		--bit;
		if (bit >= 0) {
			byte = byteArray[bit / 8];
			mask = 1 << (bit % 8);
			round = (byte & mask) ? true : false;
			--bit;
			if (bit >= 0) {
				sticky = false;
				int msByte = bit / 8; // most significant byte of the sticky bit calculation
				byte = byteArray[msByte];
				mask = 0xFF >> (8 - (bit % 8));
				if ((byte & mask) == 0x00 && msByte > 0) {
					// for the remaining bytes check if there is any bit set
					for (int bb = msByte - 1; bb >= 0; --bb) {
						if (byteArray[bb]) {
							sticky = true;
							break;
						}
					}
				}
				rv = (!round && !sticky) ? 0 : -1;
			}
		}
	}
	return rv;
}

// local helper to display the contents of a byte array
void displayByteArray(std::string tag, const uint8_t byteArray[], unsigned N) {
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::cout << tag << "= 0x" << std::hex;
	for (int i = N - 1; i >= 0; --i) {
		unsigned msNibble = ((0xF0 & byteArray[i]) >> 4);
		unsigned lsNibble = (0x0F & byteArray[i]);
		//		std::cout << "nibbles: " << msNibble << " " << lsNibble << std::endl;
		//		std::cout << '-' << hexChar[msNibble] << '-' << std::endl << hexChar[lsNibble];
		std::cout << hexChar[msNibble] << hexChar[lsNibble];
	}
	std::cout << std::endl;
}

// byte arithmetic for mul and div
void addBytes(uint8_t accumulator[], uint8_t y[], unsigned mulBytes) {
	bool carry = false;
	for (unsigned i = 0; i < mulBytes; ++i) {
		// cast up so we can test for overflow
		uint16_t l = uint16_t(accumulator[i]);
		uint16_t r = uint16_t(y[i]);
		uint16_t s = l + r + (carry ? uint16_t(0x0001) : uint16_t(0x0000));
		carry = (s > 255 ? true : false);
		accumulator[i] = (uint8_t)(s & 0xFF);
	}
}

// shift left by one bit
void shiftLeft(uint8_t multiplicant[], unsigned N) {
	// hardcoded shift by one bit
	unsigned i = N - 1;
	multiplicant[i] <<= 1;
	if (N > 1) {
		multiplicant[i] |= ((0x80 & multiplicant[i - 1]) >> 7);
		for (int i = N - 2; i > 0; --i) {
			multiplicant[i] <<= 1;
			multiplicant[i] |= ((0x80 & multiplicant[i - 1]) >> 7);
		}
	}
	multiplicant[0] <<= 1;
}

// shift right by bitsToShift
void shiftRight(uint8_t byteArray[], unsigned N, unsigned bitsToShift) {
	int byteShift = 0;
	if (bitsToShift >= 8) {
		byteShift = bitsToShift / 8;
		for (unsigned i = 0; i < N - 1 - byteShift; ++i) {
			byteArray[i] = byteArray[i + byteShift];
		}
		for (unsigned i = N - 1 - byteShift; i < N - 1; ++i) {
			byteArray[i] = uint8_t(0x00);
		}
		// adjust the shift
		bitsToShift -= byteShift * 8;
		if (bitsToShift == 0) return;
	}
	uint8_t mask = 0xFF;
	mask >>= (8 - bitsToShift); // this is a mask for the lower bits in the byte that need to move to the previous byte
	for (unsigned i = 0; i < N - 1; ++i) {
		byteArray[i] >>= bitsToShift;
		// mix in the bits from the left
		uint8_t bits = (mask & byteArray[i + 1]);
		byteArray[i] |= (bits << (8 - bitsToShift));
	}
	byteArray[N - 1] >>= bitsToShift;
}



} // namespace native
} // namespace sw
