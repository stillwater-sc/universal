#pragma once
// blockBinaryNumber.hpp: parameterized blocked binary number
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>

// compiler specific operators
#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw {
namespace unum {


	// forward references
	template<size_t nbits, typename StorageBlockType> class blockBinaryNumber;
	template<size_t nbits, typename StorageBlockType> blockBinaryNumber<nbits, StorageBlockType> twosComplement(const blockBinaryNumber<nbits, StorageBlockType>&);

	// generate the 2's complement of the block binary number
	template<size_t nbits, typename StorageBlockType>
	blockBinaryNumber<nbits, StorageBlockType> twosComplement(const blockBinaryNumber<nbits, StorageBlockType>& orig) {
		blockBinaryNumber<nbits, StorageBlockType> twosC;
		blockBinaryNumber<nbits, StorageBlockType> plusOne;
		plusOne = 1;
		twosC += plusOne;
		return twosC;
	}

/*
NOTES

for block arithmetic, we need to manage a carry bit.
This disqualifies using uint64_t as a block type as we can't catch the overflow condition
in the same way as the other native types, uint8_t, uint16_t, uint32_t.

We could use a sint64_t and then convert to uint64_t and observe the MSB. Very different 
logic though.
*/

// a block-based 2's complement binary number
template<size_t nbits, typename StorageBlockType = uint8_t>
class blockBinaryNumber {
public:
	static constexpr size_t bitsInByte = 8;
	static constexpr size_t bitsInBlock = sizeof(StorageBlockType) * bitsInByte;
	static_assert(bitsInBlock <= 32, "storage unit for block arithmetic needs to be <= uint32_t");

	static constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInBlock);
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFF >> (64 - bitsInBlock));
	static constexpr StorageBlockType maxBlockValue = (StorageBlockType(1) << bitsInBlock) - 1;

	static constexpr size_t MSU = nrUnits - 1; // Most Significant Unit
	static constexpr size_t MSU_MASK = (StorageBlockType(0xFFFFFFFFFFFFFFFFul) >> (nrUnits * bitsInBlock - nbits));

	// constructors
	blockBinaryNumber() { setzero(); }

	blockBinaryNumber(const blockBinaryNumber&) = default;
	blockBinaryNumber(blockBinaryNumber&&) = default;

	blockBinaryNumber& operator=(const blockBinaryNumber&) = default;
	blockBinaryNumber& operator=(blockBinaryNumber&&) = default;

	// initializer for long long
	blockBinaryNumber(const long long initial_value) { *this = initial_value; }

	blockBinaryNumber& operator=(long long rhs) {
		for (unsigned i = 0; i < nrUnits; ++i) {
			block[i] = rhs & storageMask;
			rhs >>= bitsInBlock;
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		block[MSU] &= MSU_MASK;
		return *this;
	}

	// arithmetic operators
	blockBinaryNumber& operator+=(const blockBinaryNumber& rhs) {
		bool carry = false;
		for (unsigned i = 0; i < nrUnits; ++i) {
			// cast up so we can test for overflow
			uint64_t l = uint64_t(block[i]);
			uint64_t r = uint64_t(rhs.block[i]);
			uint64_t s = l + r + (carry ? uint64_t(1) : uint64_t(0));
			carry = (s > maxBlockValue ? true : false);
			block[i] = StorageBlockType(s);
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		block[MSU] &= MSU_MASK;
		return *this;
	}
	blockBinaryNumber& operator-=(const blockBinaryNumber& rhs) {
		return *this;
	}
	blockBinaryNumber& operator*=(const blockBinaryNumber& rhs) {
		return *this;
	}
	blockBinaryNumber& operator/=(const blockBinaryNumber& rhs) {
		return *this;
	}
	blockBinaryNumber& operator%=(const blockBinaryNumber& rhs) {
		return *this;
	}
	blockBinaryNumber& operator<<=(const signed shift) {
		// hardcoded shift by one bit
		constexpr size_t msbMask = (StorageBlockType(1) << (bitsInBlock - 1));

		int i = int(nrUnits - 1);
		block[i] <<= 1;
		if (nrUnits > 1) {
			block[i] |= ((msbMask & block[i - 1]) >> (bitsInBlock - 1));
			for (i = int(nrUnits - 2); i > 0; --i) {
				block[i] <<= 1;
				block[i] |= ((msbMask & block[i - 1]) >> (bitsInBlock - 1));
			}
		}
		block[0] <<= 1;
		return *this;
	}
	blockBinaryNumber& operator>>=(long long bitsToShift) {
		size_t blockShift = 0;
		if (bitsToShift >= bitsInBlock) {
			blockShift = bitsToShift / bitsInBlock;
			for (size_t i = 0; i < MSU - blockShift; ++i) {
				block[i] = block[i + blockShift];
			}
			for (size_t i = MSU - blockShift; i < MSU; ++i) {
				block[i] = StorageBlockType(0);
			}
			// adjust the shift
			bitsToShift -= (long long)(blockShift * bitsInBlock);
			if (bitsToShift == 0) return *this;
		}
		StorageBlockType mask = 0xFFFFFFFFFFFFFFFF > (64 - bitsInBlock);
		mask >>= (bitsInBlock - bitsToShift); // this is a mask for the lower bits in the block that need to move to the previous byte
		for (unsigned i = 0; i < MSU; ++i) {
			block[i] >>= bitsToShift;
			// mix in the bits from the left
			uint8_t bits = (mask & block[i + 1]);
			block[i] |= (bits << (bitsInBlock - bitsToShift));
		}
		block[MSU] >>= bitsToShift;
		return *this;
	}

	// modifiers
	 // clear a block binary number
	inline void clear() {
		for (size_t i = 0; i < nrUnits; ++i) {
			block[i] = StorageBlockType(0);
		}
	}
	inline void setzero() { clear(); }
	void set_raw_bits(uint64_t value) {
		for (unsigned i = 0; i < nrUnits; ++i) {
			block[i] = value & storageMask;
			value >>= bitsInBlock;
		}
		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		block[MSU] &= MSU_MASK;
	}

	// selectors
	inline bool sign() const { return block[MSU] & MSU_MASK; }
	inline bool at(size_t i) {
		if (i < nbits) {
			StorageBlockType word = block[i / bitsInBlock];
			StorageBlockType mask = (StorageBlockType(1) << (i % bitsInBlock));
			return (word & mask);
		}
		throw "bit index out of bounds";
	}
	inline uint8_t nibble(size_t n) {
		if (n < (1 + ((nbits - 1) >> 2))) {
			StorageBlockType word = block[(nibble * 4) / bitsInBlock];
			uint8_t nibblebits = 0;
			return nibblebits;

		}
		throw "nibble index out of bounds";
	}
	// test if the value is equal to a native signed integer type
	inline bool isEqual(const blockBinaryNumber& lhs, const blockBinaryNumber& rhs) const {
		for (size_t i = 0; i < nrUnits; ++i) {
			if (lhs[i] != rhs[i]) {
				return false;
			}
		}
		return true;
	}

	// determine the rounding mode: -1 round down, 0 tie, 1 round up
	int roundingMode(unsigned guardBitIndex) const {
		int rv = 0;

		return rv;
	}
private:
	StorageBlockType block[nrUnits];
};

///////////////////////////////////////////////////////////////////////////////
// binary operators

template<size_t nbits, typename StorageBlockType>
inline blockBinaryNumber<nbits, StorageBlockType> operator+(const blockBinaryNumber<nbits, StorageBlockType>& a, const blockBinaryNumber<nbits, StorageBlockType>& b) {
	blockBinaryNumber<nbits, StorageBlockType> c(a);
	return c += b;
}
template<size_t nbits, typename StorageBlockType>
inline blockBinaryNumber<nbits, StorageBlockType> operator-(const blockBinaryNumber<nbits, StorageBlockType>& a, const blockBinaryNumber<nbits, StorageBlockType>& b) {
	blockBinaryNumber<nbits, StorageBlockType> c(a);
	return c -= b;
}
template<size_t nbits, typename StorageBlockType>
inline blockBinaryNumber<nbits, StorageBlockType> operator*(const blockBinaryNumber<nbits, StorageBlockType>& a, const blockBinaryNumber<nbits, StorageBlockType>& b) {
	blockBinaryNumber<nbits, StorageBlockType> c(a);
	return c *= b;
}
template<size_t nbits, typename StorageBlockType>
inline blockBinaryNumber<nbits, StorageBlockType> operator/(const blockBinaryNumber<nbits, StorageBlockType>& a, const blockBinaryNumber<nbits, StorageBlockType>& b) {
	blockBinaryNumber<nbits, StorageBlockType> c(a);
	return c /= b;
}
template<size_t nbits, typename StorageBlockType>
inline blockBinaryNumber<nbits, StorageBlockType> operator%(const blockBinaryNumber<nbits, StorageBlockType>& a, const blockBinaryNumber<nbits, StorageBlockType>& b) {
	blockBinaryNumber<nbits, StorageBlockType> c(a);
	return c %= b;
}

//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the storage
template<size_t nbits, typename StorageBlockType>
std::string to_binary(const blockBinaryNumber<nbits, StorageBlockType>& number) {
	std::stringstream ss;
	ss << 'b';
	for (size_t i = nbits - 1; i >= 0; --i) {
		ss << (number.at(i) ? '1' : '0');
	}
	return ss.str();
}

// local helper to display the contents of a byte array
template<size_t nbits, typename StorageBlockType>
std::string to_hex(const blockBinaryNumber<nbits, StorageBlockType>& storage) {
	constexpr size_t bitsInBlock = sizeof(StorageBlockType) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInBlock);
	constexpr size_t nibblesInStorageUnit = sizeof(StorageBlockType) * 2;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	ss << "0x" << std::hex;
	StorageBlockType nibble;
	for (int i = int(nrUnits - 1); i >= 0; --i) {
		StorageUnit mask = (0xF000000000000000 >> (64 - bitsInBlock));
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
template<size_t nbits, typename StorageBlockType>
void displayByteArray(std::string tag, const blockBinaryNumber<nbits, StorageBlockType>& storage) {
	constexpr size_t bitsInBlock = sizeof(StorageBlockType) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInBlock);
	constexpr size_t nibblesInStorageUnit = sizeof(StorageBlockType) * 2;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::cout << tag << "= 0x" << std::hex;
	for (int i = int(nrUnits - 1); i >= 0; --i) {
		StorageUnit mask = (0xFF00000000000000 >> (64 - bitsInBlock));
		for (int j = int(nibblesInStorageUnit - 1); j >= 0; --j) {
			unsigned nibble = (mask & storage[i]) >> (j * 4);
			std::cout << hexChar[nibble];
		}
	}
	std::cout << std::endl;
}


#ifdef later
// multiply two byte arrays, a * b, return unrounded result in c and the size of outbits as value
// precondition: 
//   - a and b are in two's complement form
//   - argument c can be 0 or a partial result from a chained multiplication
template<size_t nbits, typename StorageUnit = uint8_t>
size_t multiplyBytes(const StorageUnit a[], const StorageUnit b[], StorageUnit accumulator[]) {
	constexpr size_t bitsInBlock = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInBlock);
	constexpr size_t outbits = 2 * nbits;
	constexpr size_t outUnits = 1 + ((outbits - 1) / bitsInBlock);
	constexpr size_t MSU = nrUnits - 1; // Most Significant Unit
	constexpr StorageUnit MSU_MASK = (StorageUnit(0xFFFFFFFFFFFFFFFFul) >> (outUnits * bitsInBlock - outbits));

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
			addBlockArray<outUnits, StorageUnit>(accumulator, multiplicant);
		}
		shiftLeft<outbits, StorageUnit>(multiplicant);
	}
	// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	accumulator[MSU] &= MSU_MASK;
	std::cout << "accu: " << to_hex<outbits, uint8_t>(accumulator) << std::endl;

	displayByteArray<outbits, StorageUnit>("accu", accumulator);
	return outbits;
}
#endif

} // namespace unum
} // namespace sw
