#pragma once
// bitblock.hpp : bitblock class
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>

// universal type dependencies
#include <universal/internal/uint128/uint128.hpp>
#include <universal/utility/boolean_logic_operators.hpp>
#include <universal/internal/bitblock/exceptions.hpp>

namespace sw { namespace universal { namespace internal {

// bitblock is a template class implementing efficient multi-precision binary arithmetic and logic
template<unsigned nbits>
class bitblock : public std::bitset<nbits> {
    using base= std::bitset<nbits>;
public:
	constexpr bitblock() : base(0ull) {}

	constexpr bitblock(const bitblock&) = default;
	constexpr bitblock(bitblock&&) = default;

	constexpr bitblock& operator=(const bitblock&) = default;
	constexpr bitblock& operator=(bitblock&&) = default;

	constexpr bitblock& operator=(unsigned long long rhs) {
		return *this = (bitblock&)base::operator=(rhs);
	}

	constexpr base& reset() { *this= bitblock{}; return *this; }
	using base::reset; // make unary reset visible
	
	void setToZero() { std::bitset<nbits>::reset(); }
	bool load_bits(const std::string& string_of_bits) {
		if (string_of_bits.length() != nbits) return false;
		setToZero();
		int msb = nbits - 1;
		for (std::string::const_iterator it = string_of_bits.begin(); it != string_of_bits.end(); ++it) {
			if (*it == '0') {
				this->reset(msb--);
			}
			else if (*it == '1') {
				this->set(msb--);
			}
			else {
				return false;
			}
		}
		return true;
	}
};

// logic operators

// this comparison is for a two's complement number only
template<unsigned nbits>
bool twosComplementLessThan(const bitblock<nbits>& lhs, const bitblock<nbits>& rhs) {
	// comparison of the sign bit
	if (lhs[nbits - 1] == 0 && rhs[nbits - 1] == 1)	return false;
	if (lhs[nbits - 1] == 1 && rhs[nbits - 1] == 0) return true;
	// sign is equal, compare the remaining bits
	if constexpr (nbits > 1) {
		for (int i = static_cast<int>(nbits) - 2; i >= 0; --i) {
			if (lhs[unsigned(i)] == 0 && rhs[unsigned(i)] == 1)	return true;
			if (lhs[unsigned(i)] == 1 && rhs[unsigned(i)] == 0) return false;
		}
	}
	// numbers are equal
	return false;
}

// this comparison works for any number
template<unsigned nbits>
bool operator==(const bitblock<nbits>& lhs, const bitblock<nbits>& rhs) {
	// compare remaining bits
	if constexpr (nbits > 0) {
		for (unsigned i = 0; i < nbits; ++i) if (lhs[i] != rhs[i]) return false;
	}
	// numbers are equal
	return true;
}

// this comparison is for unsigned numbers only
template<unsigned nbits>
bool operator< (const bitblock<nbits>& lhs, const bitblock<nbits>& rhs) {
	// compare remaining bits
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		if (lhs[unsigned(i)] == 0 && rhs[unsigned(i)] == 1)	return true;
		if (lhs[unsigned(i)] == 1 && rhs[unsigned(i)] == 0) return false;
	}
	// numbers are equal
	return false;
}

// test less than or equal for unsigned numbers only
template<unsigned nbits>
bool operator<= (const bitblock<nbits>& lhs, const bitblock<nbits>& rhs) {
	// compare remaining bits
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		if (lhs[unsigned(i)] == 0 && rhs[unsigned(i)] == 1)	return true;
		if (lhs[unsigned(i)] == 1 && rhs[unsigned(i)] == 0) return false;
	}
	// numbers are equal
	return true;
}

// test greater than for unsigned numbers only
template<unsigned nbits>
bool operator> (const bitblock<nbits>& lhs, const bitblock<nbits>& rhs) {
	// compare remaining bits
	if constexpr (nbits > 0) {
		for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
			if (lhs[unsigned(i)] == 0 && rhs[unsigned(i)] == 1)	return false;
			if (lhs[unsigned(i)] == 1 && rhs[unsigned(i)] == 0) return true;
		}
	}
	// numbers are equal
	return false;
}

// this comparison is for unsigned numbers only
template<unsigned nbits>
bool operator>= (const bitblock<nbits>& lhs, const bitblock<nbits>& rhs) {
	// compare remaining bits
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		if (lhs[unsigned(i)] == 0 && rhs[unsigned(i)] == 1)	return false;
		if (lhs[unsigned(i)] == 1 && rhs[unsigned(i)] == 0) return true;
	}
	// numbers are equal
	return true;
}

////////////////////////////// ARITHMETIC functions


//////////////////////////////////////////////////////////////////////////////////////
// increment and decrement

// increment the input bitset in place, and return true if there is a carry generated.
template<unsigned nbits>
bool increment_bitset(bitblock<nbits>& number) {
	bool carry = true;  // ripple carry
	for (unsigned i = 0; i < nbits; i++) {
		bool _a = number[i];
		number[i] = bxor(_a, carry);
		carry = carry && bxor(_a, false);
	}
	return carry;
}

// increment the input bitset in place, and return true if there is a carry generated.
// The input number is assumed to be right adjusted starting at nbits-nrBits
// [1 0 0 0] nrBits = 0 is a noop as there is no word to increment
// [1 0 0 0] nrBits = 1 is the word [1]
// [1 0 0 0] nrBits = 2 is the word [1 0]
// [1 1 0 0] nrBits = 3 is the word [1 1 0], etc.
template<unsigned nbits>
bool increment_unsigned(bitblock<nbits>& number, unsigned nrBits = nbits - 1) {
	if (nrBits > nbits - 1) nrBits = nbits - 1;  // check/fix argument
	bool carry = 1;  // ripple carry
	unsigned lsb = nbits - nrBits;
	for (unsigned i = lsb; i < nbits; i++) {
		bool _a = number[i];
		number[i] = bxor(_a, carry);
		carry = (_a && false) || (carry && bxor(_a, false));
	}
	return carry;
}

// decrement the input bitset in place, and return true if there is a borrow generated.
template<unsigned nbits>
bool decrement_bitset(bitblock<nbits>& number) {
	bool borrow = true;
	for (unsigned i = 0; i < nbits; i++) {
		bool _a = number[i];
		number[i] = bxor(_a, borrow);
		borrow = (!bxor(!_a, true) && borrow);
	}
	return borrow;
}

//////////////////////////////////////////////////////////////////////////////////////
// add and subtract

// add bitsets a and b and return result in bitset sum. Return true if there is a carry generated.
template<unsigned nbits>
bool add_unsigned(bitblock<nbits> a, bitblock<nbits> b, bitblock<nbits + 1>& sum) {
	bool carry = false;  // ripple carry
	for (unsigned i = 0; i < nbits; i++) {
		bool _a = a[i];
		bool _b = b[i];
		sum[i] = bxor(_a, bxor(_b, carry));
		carry = (_a && _b) || (carry && bxor(_a, _b));
	}
	sum.set(nbits, carry);
	return carry;
}

// subtract bitsets a and b and return result in bitset dif. Return true if there is a borrow generated.
template<unsigned nbits>
bool subtract_unsigned(bitblock<nbits> a, bitblock<nbits> b, bitblock<nbits + 1>& dif) {
	bool borrow = false;  // ripple borrow
	for (unsigned i = 0; i < nbits; i++) {
		bool _a = a[i];
		bool _b = b[i];
		dif[i] = bxor(bxor(_a, _b), borrow);
		borrow = (!_a && _b) || (!bxor(!_a, !_b) && borrow);
	}
	dif.set(nbits, borrow);
	return borrow;
}

template<unsigned nbits>
bool add_signed_magnitude(bitblock<nbits> a, bitblock<nbits> b, bitblock<nbits>& sum) {
	uint8_t carry = 0;
	if (nbits > 1) {  // need at least 1 bit of magnitude to add
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

		for (unsigned i = 0; i < nbits - 2; i++) {
			bool _a = a[i];
			bool _b = b[i];
			sum[i] = bxor(bxor(_a, _b), carry);
			carry = (_a && _b) || (carry && bxor(_a, _b));
		}
	}
	return carry;
}

template<unsigned nbits>
bool subtract_signed_magnitude(bitblock<nbits> a, bitblock<nbits> b, bitblock<nbits>& diff) {
	//bool sign_a = a.test(nbits - 1);
	//bool sign_b = b.test(nbits - 1);
	std::cerr << "subtract_signed_magnitude not implemented yet" << std::endl;
	return false;
}

// integral type to bitblock transformations

// we are using a full nbits sized bitset even though nbits-3 is the maximum fraction
// a posit would contain. However, we need an extra bit after the cut-off to make the
// round up/down decision. The <nbits-something> size created a lot of sw complexity
// that isn't worth the trouble, so we are simplifying and simply manage a full nbits
// of fraction bits.

template<unsigned nbits>
bitblock<nbits> extract_23b_fraction(uint32_t _23b_fraction_without_hidden_bit) {
	bitblock<nbits> _fraction;
	uint32_t mask = uint32_t(0x00400000ul);
	unsigned int ub = (nbits < 23 ? nbits : 23);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _23b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<unsigned nbits>
bitblock<nbits> extract_52b_fraction(uint64_t _52b_fraction_without_hidden_bit) {
	bitblock<nbits> _fraction;
	uint64_t mask = uint64_t(0x0008000000000000ull);
	unsigned int ub = (nbits < 52 ? nbits : 52);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _52b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<unsigned nbits>
bitblock<nbits> extract_63b_fraction(uint64_t _63b_fraction_without_hidden_bit) {
	bitblock<nbits> _fraction;
	uint64_t mask = uint64_t(0x4000000000000000ull);
	unsigned int ub = (nbits < 63 ? nbits : 63);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _63b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

// take in a long double mapped to two uint64_t elements
template<unsigned nbits>
bitblock<nbits> extract_long_double_fraction(uint128& _112b_fraction_without_hidden_bit) {
	bitblock<nbits> _fraction;
	int msb = nbits - 1;
	uint64_t mask = uint64_t(0x0000800000000000ull);
	unsigned int ub = (nbits < 48 ? nbits : 48); // 48 bits in the upper half
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[msb--] = _112b_fraction_without_hidden_bit.upper & mask;
		mask >>= 1;
	}
	mask = uint64_t(0x8000000000000000ull);
	ub = (nbits < 112 - 48 ? nbits : 112 - 48); // max 64 bits in the lower half
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[msb--] = _112b_fraction_without_hidden_bit.lower & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<unsigned nbits>
bitblock<nbits> copy_integer_fraction(unsigned long long _fraction_without_hidden_bit) {
	bitblock<nbits> _fraction;
	uint64_t mask = uint64_t(0x8000000000000000ull);
	unsigned int ub = (nbits < 64 ? nbits : 64);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

////////////////////////////////////////////////////////////////////////////////////////
// bitset copy and slice operators

// copy a bitset into a bigger bitset starting at position indicated by the shift value
template<unsigned src_size, unsigned tgt_size>
void copy_into(const bitblock<src_size>& src, unsigned shift, bitblock<tgt_size>& tgt) {
	tgt.reset();
	for (unsigned i = 0; i < src_size; i++)
		tgt.set(i + shift, src[i]);
}

#if BITBLOCK_THROW_ARITHMETIC_EXCEPTION
// copy a slice of a bitset into a bigger bitset starting at position indicated by the shift value
template<unsigned src_size, unsigned tgt_size>
void copy_slice_into(bitblock<src_size>& src, bitblock<tgt_size>& tgt, unsigned begin = 0, unsigned end = src_size, unsigned shift = 0) {
	// do NOT reset the target!!!
	if (end <= src_size) throw iteration_bound_too_large{};
	if (end + shift < tgt_size) throw iteration_bound_too_large{};
	for (unsigned i = begin; i < end; i++)
		tgt.set(i + shift, src[i]);
}
#else // !BITBLOCK_THROW_ARITHMETIC_EXCEPTION
// copy a slice of a bitset into a bigger bitset starting at position indicated by the shift value
template<unsigned src_size, unsigned tgt_size>
void copy_slice_into(bitblock<src_size>& src, bitblock<tgt_size>& tgt, unsigned begin = 0, unsigned end = src_size, unsigned shift = 0) {
	// do NOT reset the target!!!
	if (end <= src_size) return;
	if (end + shift < tgt_size) return;
	for (unsigned i = begin; i < end; i++)
		tgt.set(i + shift, src[i]);
}
#endif // !BITBLOCK_THROW_ARITHMETIC_EXCEPTION

template<unsigned from, unsigned to, unsigned src_size>
bitblock<to - from> fixed_subset(const bitblock<src_size>& src) {
	static_assert(from <= to, "from cannot be larger than to");
	static_assert(to <= src_size, "to is larger than src_size");

	bitblock<to - from> result;
	for (unsigned i = 0, end = to - from; i < end; ++i)
		result[i] = src[i + from];
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////
// multiply and divide

// accumulate the addend to a running accumulator
template<unsigned src_size, unsigned tgt_size>
bool accumulate(const bitblock<src_size>& addend, bitblock<tgt_size>& accumulator) {
	bool carry = 0;  // ripple carry
	for (unsigned i = 0; i < src_size; i++) {
		bool _a = addend[i];
		bool _b = accumulator[i];
		accumulator[i] = bxor(bxor(_a, _b), carry);
		carry = (_a && _b) || (carry && bxor(_a, _b));
	}
	return carry;
}

// multiply bitsets a and b and return result in bitset result.
template<unsigned operand_size>
void multiply_unsigned(const bitblock<operand_size>& a, const bitblock<operand_size>& b, bitblock<2 * operand_size>& result) {
	constexpr unsigned result_size = 2 * operand_size;
	bitblock<result_size> addend;
	result.reset();
	if (a.test(0)) {
		copy_into<operand_size, result_size>(b, 0, result);
	}
	for (unsigned i = 1; i < operand_size; i++) {
		if (a.test(i)) {
			copy_into<operand_size, result_size>(b, i, addend);
#ifdef DEBUG
			bool carry = accumulate(addend, result);   // we should never have a carry
			assert(carry == false);
#else
			accumulate(addend, result);   // we should never have a carry
#endif
		}
	}
}

// subtract a subtractand from a running accumulator
template<unsigned src_size, unsigned tgt_size>
bool subtract(bitblock<tgt_size>& accumulator, const bitblock<src_size>& subtractand) {
	bool borrow = 0;  // ripple borrow
	for (unsigned i = 0; i < src_size; i++) {
		bool _a = accumulator[i];
		bool _b = subtractand[i];
		accumulator[i] = bxor(bxor(_a, _b), borrow);
		borrow = (!_a && _b) || (!bxor(!_a, !_b) && borrow);
	}
	return borrow;
}

// divide bitsets a and b and return result in bitset result.
template<unsigned operand_size>
void integer_divide_unsigned(const bitblock<operand_size>& a, const bitblock<operand_size>& b, bitblock<2 * operand_size>& result) {
	bitblock<operand_size> subtractand, accumulator;
	result.reset();
	accumulator = a;
	int msb = findMostSignificantBit(b);
	if (msb < 0) {
#if BITBLOCK_THROW_ARITHMETIC_EXCEPTION
		throw bitblock_divide_by_zero{};
#else
		std::cerr << "bitblock_divide_by_zero\n";
#endif // BITBLOCK_THROW_ARITHMETIC_EXCEPTION
	}
	else {
		int shift = static_cast<int>(operand_size) - msb - 1;
		// prepare the subtractand
		subtractand = b;
		subtractand <<= static_cast<unsigned>(shift);
//		for (int i = operand_size - msb - 1; i >= 0; --i) {
		for (int i = shift; i >= 0; --i) {

			if (subtractand <= accumulator) {
#ifdef DEBUG
				bool borrow = subtract(accumulator, subtractand);
				assert(borrow == true);
#else
				subtract(accumulator, subtractand);
#endif
				result.set(static_cast<unsigned>(i));
			}
			else {
				result.reset(static_cast<unsigned>(i));
			}
			subtractand >>= 1ull;
		}
	}
}

// divide bitsets a and b and return result in bitset result. 
// By providing more bits in the result, the algorithm will fill these with fraction bits if available.
// Radix point must be maintained by calling function.
template<unsigned operand_size, unsigned result_size>
void divide_with_fraction(const bitblock<operand_size>& a, const bitblock<operand_size>& b, bitblock<result_size>& result) {
	bitblock<result_size> subtractand, accumulator;
	result.reset();
	copy_into<operand_size, result_size>(a, result_size - operand_size, accumulator);
	int msb = findMostSignificantBit(b);
	if (msb < 0) {
#if BITBLOCK_THROW_ARITHMETIC_EXCEPTION
		throw bitblock_divide_by_zero{};
#else
		std::cerr << "bitblock_divide_by_zero\n";
#endif // BITBLOCK_THROW_ARITHMETIC_EXCEPTION
	}
	else {
		int shift = static_cast<int>(operand_size) - msb - 1;
		// prepare the subtractand
		copy_into<operand_size, result_size>(b, result_size - operand_size, subtractand);
		subtractand <<= static_cast<unsigned>(shift);
		for (int i = static_cast<int>(result_size) - msb - 1; i >= 0; --i) {
			//std::cout << "accumulator " << accumulator << std::endl;
			//std::cout << "subtractand " << subtractand << std::endl;
			if (subtractand <= accumulator) {
#ifdef DEBUG
				bool borrow = subtract(accumulator, subtractand);
				assert(borrow == false);
#else
				subtract(accumulator, subtractand);
#endif
				result.set(static_cast<unsigned>(i));
			}
			else {
				result.reset(static_cast<unsigned>(i));
			}
			//std::cout << "result      " << result << std::endl;
			subtractand >>= 1u;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// truncating and rounding

// truncate right-side
template<unsigned src_size, unsigned tgt_size>
void truncate(bitblock<src_size>& src, bitblock<tgt_size>& tgt) {
	tgt.reset();
	for (unsigned i = 0; i < tgt_size; i++)
		tgt.set(tgt_size - 1 - i, src[src_size - 1 - i]);
}

// round
template<unsigned tgt_size, unsigned src_size>
struct round_t
{
	static bitblock<tgt_size> eval(const bitblock<src_size>& src, unsigned n)
	{
		static_assert(src_size > 0 && tgt_size > 0, "We don't bother with empty sets.");
#if BITBLOCK_THROW_ARITHMETIC_EXCEPTION
		if (n >= src_size)
			throw round_off_all{};
		// look for cut-off leading bits
		for (unsigned leading = tgt_size + n; leading < src_size; ++leading)
			if (src[leading])
				throw cut_off_leading_bit{};
#else
		if (n >= src_size) {
			bitblock<tgt_size> result;
			result.reset();
			return result;
		}
		for (unsigned leading = tgt_size + n; leading < src_size; ++leading) {
			if (src[leading]) {
				std::cerr << "cut_off_leading_bit\n";
				bitblock<tgt_size> result;
				result.reset();
				return result;
			}
		}
#endif // BITBLOCK_THROW_ARITHMETIC_EXCEPTION

		bitblock<tgt_size> result((src >> n).to_ullong()); // convert to unsigned to deal with different sizes

		if (n > 0 && src[n - 1]) {                         // round up potentially if first cut-off bit is true
#ifdef BITBLOCK_ROUND_TIES_AWAY_FROM_ZERO					   // TODO: Evil hack to be consistent with assign_fraction, for testing only
			result = result.to_ullong() + 1;
#else            

			bool more_bits = false;
			for (long i = 0; i + 1 < n && !more_bits; ++i)
				more_bits |= src[i];
			if (more_bits) {
				result = result.to_ullong() + 1;           // increment_unsigned is ambiguous 
			}
			else {                                         // tie: round up odd number
#ifndef BITBLOCK_ROUND_TIES_TO_ZERO                        // TODO: evil hack to be removed later
				if (result[0])
					result = result.to_ullong() + 1;
#endif
			}
#endif
		}
		return result;
	}
};

template<unsigned src_size>
struct round_t<0, src_size>
{
	static bitblock<0> eval(const bitblock<src_size>&, unsigned)
	{
		return {};
	}
};



/** Round off \p n last bits of bitset \p src. Round to nearest resulting in potentially smaller bitset.
*  Doesn't return carry bit in case of overflow while rounding up! TODO: Check whether we need carry or we require an extra bit for this case.
*/
template<unsigned tgt_size, unsigned src_size>
bitblock<tgt_size> round(const bitblock<src_size>& src, unsigned n)
{
	return round_t<tgt_size, src_size>::eval(src, n);
}


////////////////////////////// HELPER functions

// find the MSB, return position if found, return -1 if no bits are set
template<unsigned nbits>
int findMostSignificantBit(const bitblock<nbits>& bits) {
	int msb = -1; // indicative of no bits set
	for (int i = nbits - 1; i >= 0; i--) {
		if (bits.test(static_cast<unsigned>(i))) {
			msb = i;
			break;
		}
	}
	return msb;
}

// calculate the 1's complement of a sign-magnitude encoded number
template<unsigned nbits>
bitblock<nbits> ones_complement(bitblock<nbits> number) {
	bitblock<nbits> complement;
	for (unsigned i = 0; i < nbits; i++) {
		complement.set(i, !number[i]);
	}
	return complement;
}

// calculate the 2's complement of a 2's complement encoded number
template<unsigned nbits>
bitblock<nbits> twos_complement(bitblock<nbits> number) {
	bitblock<nbits> complement;
	uint8_t _slice = 0;
	uint8_t carry = 1;
	for (unsigned i = 0; i < nbits; i++) {
		uint8_t not_bit_at_i = number[i] ? uint8_t(0) : uint8_t(1);
		_slice = uint8_t(not_bit_at_i + carry);
//		_slice = uint8_t(!number[i]) + carry;
		carry = uint8_t(_slice >> 1);
		complement[i] = (0x1 & _slice);
	}
	return complement;
}

// DANGER: this depends on the implicit type conversion of number to a uint64_t to sign extent a 2's complement number system
// if nbits > 64 then this code breaks.
template<unsigned nbits, class Type>
bitblock<nbits> convert_to_bitblock(Type number) {
	bitblock<nbits> _Bits;
	uint64_t mask = 1ull;
	for (unsigned i = 0; i < nbits; i++) {
		_Bits[i] = mask & number;
		mask <<= 1;
	}
	return _Bits;
}

template<unsigned nbits>
std::string to_bit_string(bitblock<nbits> bits, bool separator = true) {
	std::stringstream ss;
	int msb = nbits; // compilation warning work-around for nbits = 0
	for (int i = msb - 1; i >= 0; --i) {
		ss << (bits[i] ? "1" : "0");
		if (separator && i % 4 == 0 && i != 0) ss << "'";
	}
	return ss.str();
}

template<unsigned nbits>
std::string to_hex(bitblock<nbits> bits, bool nibbleMarker = false, bool hexPrefix = true) {
	std::string hexStr;

	//const char* hexits = "0123456789ABCDEF";
	const char* hexits = "0123456789abcdef";
	unsigned hexit;
	switch (nbits) {
	case 1:
		hexit = bits[0];
		hexStr = hexits[hexit];
		break;
	case 2:
		hexit = static_cast<unsigned int>((bits[1] << 1u) + bits[0]);
		hexStr = hexits[hexit];
		break;
	case 3:
		hexit = static_cast<unsigned int>((bits[2] << 2u) + (bits[1] << 1u) + bits[0]);
		hexStr = hexits[hexit];
		break;
	default:
		{
			unsigned nrHexits = (nbits >> 2) + (nbits % 4 ? 0 : 1);
			for (unsigned i = 0; i < nrHexits; i++) {
				hexit = static_cast<unsigned>((bits[3] << 3u) + (bits[2] << 2u) + (bits[1] << 1u) + bits[0]);
				hexStr = hexits[hexit] + hexStr;
				if (nibbleMarker && (i % 4) == 0 && i != 0) hexStr = '\'' + hexStr;
				bits >>= 4;
			}
		}
	}
	return (hexPrefix ? std::string("0x") : std::string("")) + hexStr;
}

// convert a sign/magnitude number to a string
template<unsigned nbits>
std::string sign_magnitude_to_string(bitblock<nbits> bits) {
	std::stringstream ss;
	ss << (bits[nbits - 1] ? "n-" : "p-");
	if (nbits < 2) return ss.str();
	for (int i = nbits - 2; i >= 0; --i) {
		ss << (bits[i] ? "1" : "0");
	}
	return ss.str();
}

// return a new bitset with the sign flipped as compared to the input bitset
template<unsigned nbits>
bitblock<nbits> flip_sign_bit(bitblock<nbits> number) {
	number.flip(nbits - 1);
	return number;
}

// sticky bit representation of all the bits from [msb, lsb], that is, msb is included
template<unsigned nbits>
bool anyAfter(const bitblock<nbits>& bits, int msb) {
	if (msb < 0) return false;	// bad input
	bool running = false;
	for (int i = msb; i >= 0; i--) {
		running |= bits.test(unsigned(i));
	}
	return running;
}

}}} // namespace sw::universal::internal
