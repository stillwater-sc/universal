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
		blockBinaryNumber<nbits, StorageBlockType> twosC(orig);
		blockBinaryNumber<nbits, StorageBlockType> plusOne;
		plusOne = 1;
		twosC.flip();
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
	static constexpr StorageBlockType maxBlockValue = (uint64_t(1) << bitsInBlock) - 1;

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

	// logic operators
	blockBinaryNumber  operator~() {
		blockBinaryNumber<nbits, StorageBlockType> complement(*this);
		complement.flip();
		return complement;
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
		return operator+=(twosComplement(rhs));
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
	// shift left operator
	blockBinaryNumber& operator<<=(long bitsToShift) {
		if (bitsToShift < 0) return operator>>=(-bitsToShift);
		signed blockShift = 0;
		if (bitsToShift >= bitsInBlock) {
			blockShift = bitsToShift / bitsInBlock;
			for (signed i = signed(MSU); i >= blockShift; --i) {
				block[i] = block[i - blockShift];
			}
			std::cout << "after block shift: " << to_binary(*this, true) << std::endl;

			for (signed i = blockShift - 1; i >= 0; --i) {
				block[i] = StorageBlockType(0);
			}
			std::cout << "after null: " << to_binary(*this, true) << std::endl;

						// adjust the shift
			bitsToShift -= (long)(blockShift * bitsInBlock);
			if (bitsToShift == 0) return *this;
		}
		// construct the mask for the upper bits in the block that need to move to the higher word
		StorageBlockType mask = 0xFFFFFFFFFFFFFFFF << (bitsInBlock - bitsToShift);
		for (unsigned i = MSU; i > 0; --i) {
			block[i] <<= bitsToShift;
			// mix in the bits from the right
			StorageBlockType bits = (mask & block[i - 1]);
			block[i] |= (bits >> (bitsInBlock - bitsToShift));
		}
		block[0] <<= bitsToShift;
		return *this;
	}
	// shift right operator
	blockBinaryNumber& operator>>=(long bitsToShift) {
		if (bitsToShift < 0) return operator<<=(-bitsToShift);
		size_t blockShift = 0;
		if (bitsToShift >= bitsInBlock) {
			blockShift = bitsToShift / bitsInBlock;
			for (size_t i = 0; i <= MSU - blockShift; ++i) {
				block[i] = block[i + blockShift];
			}		
//			std::cout << "after block shift: " << to_binary(*this, true) << std::endl;

			for (size_t i = MSU - blockShift + 1; i <= MSU; ++i) {
				block[i] = StorageBlockType(0);
			}
//			std::cout << "after null: " << to_binary(*this, true) << std::endl;

			// adjust the shift
			bitsToShift -= (long)(blockShift * bitsInBlock);
			if (bitsToShift == 0) return *this;
		}
		StorageBlockType mask = 0xFFFFFFFFFFFFFFFF >> (64 - bitsInBlock);
		mask >>= (bitsInBlock - bitsToShift); // this is a mask for the lower bits in the block that need to move to the lower word
		for (unsigned i = 0; i < MSU; ++i) {
			block[i] >>= bitsToShift;
			// mix in the bits from the left
			StorageBlockType bits = (mask & block[i + 1]);
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
	// in-place one's complement
	inline blockBinaryNumber& flip() {
		for (unsigned i = 0; i < nrUnits; ++i) {
			block[i] = ~block[i];
		}
		// assert precondition of properly nulled leading non-bits
		block[MSU] = block[MSU] & MSU_MASK; 
		return *this;
	}
	// selectors
	inline bool sign() const { return block[MSU] & MSU_MASK; }
	inline bool at(size_t i) const {
		if (i < nbits) {
			StorageBlockType word = block[i / bitsInBlock];
			StorageBlockType mask = (StorageBlockType(1) << (i % bitsInBlock));
			return (word & mask);
		}
		throw "bit index out of bounds";
	}
	inline uint8_t nibble(size_t n) const {
		if (n < (1 + ((nbits - 1) >> 2))) {
			StorageBlockType word = block[(n * 4) / bitsInBlock];
			int nibbleIndexInWord = n % (bitsInBlock >> 2);
			StorageBlockType mask = 0xF << (nibbleIndexInWord*4);
			StorageBlockType nibblebits = mask & word;
			return (nibblebits >> (nibbleIndexInWord*4));
		}
		throw "nibble index out of bounds";
	}
	// determine the rounding mode: -1 round down, 0 tie, 1 round up
	int roundingMode(unsigned guardBitIndex) const {
		int rv = 0;

		return rv;
	}
private:
	StorageBlockType block[nrUnits];

	// integer - integer logic comparisons
	template<size_t nnbits, typename B>
	friend bool operator==(const blockBinaryNumber<nnbits, B>& lhs, const blockBinaryNumber<nnbits, B>& rhs);
	template<size_t nnbits, typename B>
	friend bool operator!=(const blockBinaryNumber<nnbits, B>& lhs, const blockBinaryNumber<nnbits, B>& rhs);

};

///////////////////////////////////////////////////////////////////////////////
// logic operators

template<size_t nnbits, typename B>
inline bool operator==(const blockBinaryNumber<nnbits, B>& lhs, const blockBinaryNumber<nnbits, B>& rhs) {
	for (size_t i = 0; i < lhs.nrUnits; ++i) {
		if (lhs.block[i] != rhs.block[i]) {
			return false;
		}
	}
	return true;
}
template<size_t nnbits, typename B>
inline bool operator!=(const blockBinaryNumber<nnbits, B>& lhs, const blockBinaryNumber<nnbits, B>& rhs) {
	return !operator==(lhs, rhs);
}

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
std::string to_binary(const blockBinaryNumber<nbits, StorageBlockType>& number, bool nibbleMarker = false) {
	std::stringstream ss;
	ss << 'b';
	for (int i = int(nbits - 1); i >= 0; --i) {
		ss << (number.at(i) ? '1' : '0');
		if (i > 0 && (i % 4) == 0) ss << '\'';
	}
	return ss.str();
}

// local helper to display the contents of a byte array
template<size_t nbits, typename StorageBlockType>
std::string to_hex(const blockBinaryNumber<nbits, StorageBlockType>& number, bool wordMarker = false) {
	static constexpr size_t bitsInByte = 8;
	static constexpr size_t bitsInBlock = sizeof(StorageBlockType) * bitsInByte;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	ss << "0x" << std::hex;
	int nrNibbles = int(1 + ((nbits - 1) >> 2));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = number.nibble(n);
		ss << hexChar[nibble];
		if (n > 0 && ((n * 4) % bitsInBlock) == 0) ss << '\'';
	}
	return ss.str();
}

// local helper to display the contents of a byte array
template<size_t nbits, typename StorageBlockType>
void displayByteArray(std::string tag, const blockBinaryNumber<nbits, StorageBlockType>& storage) {
	constexpr size_t bitsInBlock = sizeof(StorageBlockType) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInBlock);
	constexpr size_t nibblesInBlock = sizeof(StorageBlockType) * 2;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::cout << tag << "= 0x" << std::hex;
	for (int i = int(nrUnits - 1); i >= 0; --i) {
		StorageBlockType mask = (0xFF00000000000000 >> (64 - bitsInBlock));
		for (int j = int(nibblesInBlock - 1); j >= 0; --j) {
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
template<size_t nbits, typename StorageBlockType = uint8_t>
size_t multiplyBytes(const StorageUnit a[], const StorageUnit b[], StorageUnit accumulator[]) {
	constexpr size_t bitsInBlock = sizeof(StorageBlockType) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInBlock);
	constexpr size_t outbits = 2 * nbits;
	constexpr size_t outUnits = 1 + ((outbits - 1) / bitsInBlock);
	constexpr size_t MSU = nrUnits - 1; // Most Significant Unit
	constexpr StorageBlockType MSU_MASK = (StorageBlockType(0xFFFFFFFFFFFFFFFFul) >> (outUnits * bitsInBlock - outbits));

	bool signExtend = sign<nbits, StorageBlockType>(b);
	uint8_t multiplicant[outUnits]; // map multiplicant to accumulator size
	for (size_t i = 0; i < nrUnits; ++i) {
		multiplicant[i] = b[i];
		multiplicant[i + nrUnits] = (signExtend ? StorageBlockType(0xFF) : StorageBlockType(0x00)); // sign extend if needed
	}

	for (size_t i = 0; i < nbits; ++i) {
		uint8_t byte = b[i / 8];
		uint8_t mask = 1 << (i % 8);
		if (byte & mask) { // check the multiplication bit
			addBlockArray<outUnits, StorageBlockType>(accumulator, multiplicant);
		}
		shiftLeft<outbits, StorageBlockType>(multiplicant);
	}
	// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	accumulator[MSU] &= MSU_MASK;
	std::cout << "accu: " << to_hex<outbits, uint8_t>(accumulator) << std::endl;

	displayByteArray<outbits, StorageBlockType>("accu", accumulator);
	return outbits;
}
#endif

} // namespace unum
} // namespace sw
