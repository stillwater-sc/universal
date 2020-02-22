#pragma once
// byteArray.hpp: manipulators for byte arrays
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>

// byte array arithmetic is NOT a class, but a set of functions that implement
// the basic arithmetic functions on binary numbers represented as byte arrays.
// Why not a class: the different operators, add/sub, mul, and div yield
// different data types than the original input. Add/sub yields a binary
// number one bit larger than the original, mul yields a number twice the 
// size, and div can yield an arbitrary large return result.
// Depending on the context of the arithmetic different actions are taken
// to map the result back into the original number system discretization.
// This set of functions only provides the raw binary arithmetic and 
// leaves the mapping to the calling number system.

namespace sw {
namespace unum {

// clear a storage block
template<size_t nbits, typename StorageUnit = uint8_t>
void clear(StorageUnit su[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	for (size_t i = 0; i < nrUnits; ++i) {
		su[i] = StorageUnit(0);
	}
}

// for testing suites where we can satisfy the constraint that v is less than 64 bits
template<size_t nbits, typename StorageUnit = uint8_t>
void setRawBits(StorageUnit storage[], uint64_t value) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFF >> (64 - bitsInStorageUnit));
	constexpr size_t MSU = nrUnits - 1; // Most Significant Unit
	constexpr StorageUnit MSU_MASK = (StorageUnit(0xFFFFFFFFFFFFFFFFul) >> (nrUnits * bitsInStorageUnit - nbits));
//	std::cout << std::hex << "value = " << value << std::dec << std::endl;
//	std::cout << std::hex << "mask  = " << uint64_t(MSU_MASK) << std::dec << std::endl;
	for (unsigned i = 0; i < nrUnits; ++i) {
		storage[i] = value & storageMask;
		value >>= bitsInStorageUnit;
//		std::cout << "storage[" << i << "] = " << std::hex << int(storage[i]) << " current value bits: " << value << std::dec << std::endl;
	}
	// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	storage[MSU] &= MSU_MASK;
}

// copy rhs into lhs
template<size_t nbits, typename StorageUnit = uint8_t>
void copy(StorageUnit lhs[], const StorageUnit rhs[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	for (size_t i = 0; i < nrUnits; ++i) {
		lhs[i] = rhs[i];
	}
}

// test if the value is equal to a native signed integer type
template<size_t nbits, typename StorageUnit = uint8_t>
bool isEqual(const StorageUnit lhs[], const StorageUnit rhs[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	for (size_t i = 0; i < nrUnits; ++i) {
		if (lhs[i] != rhs[i]) {
			return false;
		}
	}
	return true;
}

// local helper to display the contents of a byte array
template<size_t nbits, typename StorageUnit = uint8_t>
std::string to_hex(const StorageUnit storage[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	constexpr size_t nibblesInStorageUnit = sizeof(StorageUnit) * 2;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	ss << "0x" << std::hex;
	StorageUnit nibble;
	for (int i = int(nrUnits - 1); i >= 0; --i) {
		StorageUnit mask = (0xF000000000000000 >> (64 - bitsInStorageUnit));
		for (int j = int(nibblesInStorageUnit - 1); j >= 0; --j) {
			// check if this nibble is part of the number
			size_t lsbOfNibble = (size_t(i)*nibblesInStorageUnit + size_t(j)) * 4;
			if (lsbOfNibble < nbits) {
				nibble = (mask & storage[i]) >> (j * 4);
				//std::cout << "mask = " << std::hex << int64_t(mask) << " storage[" << i << "] = " << int64_t(storage[i]) << std::dec << std::endl;
				//std::cout << "nibble[" << j << "] = " << std::hex << int(nibble) << std::dec << std::endl;
				ss << hexChar[nibble];
			}
			mask >>= 4;
		}
	}
	return ss.str();
}

// local helper to display the contents of a byte array
template<size_t nbits, typename StorageUnit = uint8_t>
void displayByteArray(std::string tag, const StorageUnit storage[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	constexpr size_t nibblesInStorageUnit = sizeof(StorageUnit) * 2;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::cout << tag << "= 0x" << std::hex;
	for (int i = int(nrUnits - 1); i >= 0; --i) {
		StorageUnit mask = (0xFF00000000000000 >> (64 - bitsInStorageUnit));
		for (int j = int(nibblesInStorageUnit - 1); j >= 0; --j) {
			unsigned nibble = (mask & storage[i]) >> (j * 4);
			std::cout << hexChar[nibble];
		}
	}
	std::cout << std::endl;
}

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


// addition of two byte arrays: semanticly: a = a + b
template<size_t nbits, typename StorageUnit = uint8_t>
void addBytes(StorageUnit a[], const StorageUnit b[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);

	bool carry = false;
	for (unsigned i = 0; i < nrUnits; ++i) {
		// cast up so we can test for overflow
		uint16_t l = uint16_t(a[i]);
		uint16_t r = uint16_t(b[i]);
		uint16_t s = l + r + (carry ? uint16_t(0x0001) : uint16_t(0x0000));
		carry = (s > 255 ? true : false);
		a[i] = (uint8_t)(s & 0xFF);
	}
}

template<size_t nbits, typename StorageUnit = uint8_t>
inline bool sign(const StorageUnit b[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	constexpr StorageUnit bitIndex = nbits % bitsInStorageUnit;
	constexpr StorageUnit mask = (StorageUnit(1) << bitIndex);
	return b[nrUnits - 1] & mask;
}

// shift left by one bit
template<size_t nbits, typename StorageUnit = uint8_t>
void shiftLeft(StorageUnit multiplicant[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	// hardcoded shift by one bit
	int i = int(nrUnits - 1);
	multiplicant[i] <<= 1;
	if (nrUnits > 1) {
		multiplicant[i] |= ((0x80 & multiplicant[i - 1]) >> 7);
		for (i = int(nrUnits - 2); i > 0; --i) {
			multiplicant[i] <<= 1;
			multiplicant[i] |= ((0x80 & multiplicant[i - 1]) >> 7);
		}
	}
	multiplicant[0] <<= 1;
}

// multiply two byte arrays, a * b, return unrounded result in c and the size of outbits as value
// precondition: 
//   - a and b are in two's complement form
//   - argument c can be 0 or a partial result from a chained multiplication
template<size_t nbits, typename StorageUnit = uint8_t>
size_t multiplyBytes(const StorageUnit a[], const StorageUnit b[], StorageUnit accumulator[]) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	constexpr size_t outbits = 2 * nbits;
	constexpr size_t outUnits = 1 + ((outbits - 1) / bitsInStorageUnit);
	constexpr size_t MSU = nrUnits - 1; // Most Significant Unit
	constexpr StorageUnit MSU_MASK = (StorageUnit(0xFFFFFFFFFFFFFFFFul) >> (outUnits * bitsInStorageUnit - outbits));

	bool signExtend = sign<nbits, StorageUnit>(b);
	uint8_t multiplicant[outUnits]; // map multiplicant to accumulator size
	for (size_t i = 0; i < nrUnits; ++i) {
		multiplicant[i] = b[i];
		multiplicant[i + nrUnits] = (signExtend ? StorageUnit(0xFF) : StorageUnit(0x00)); // sign extend if needed
	}

	for (size_t i = 0; i < nbits; ++i) {
		uint8_t byte = b[i / 8];
		uint8_t mask = 1 << (i % 8);
		if (byte & mask) { // check the multiplication bit
			addBytes<outUnits, StorageUnit>(accumulator, multiplicant);
		}
		shiftLeft<outbits, StorageUnit>(multiplicant);
	}
	// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	accumulator[MSU] &= MSU_MASK;
	std::cout << "accu: " << to_hex<outbits, uint8_t>(accumulator) << std::endl;

	displayByteArray<outbits, StorageUnit>("accu", accumulator);
	return outbits;
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




} // namespace unum
} // namespace sw
