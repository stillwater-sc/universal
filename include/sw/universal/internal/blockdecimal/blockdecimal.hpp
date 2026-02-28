#pragma once
// blockdecimal.hpp: unsigned decimal integer with compact encoding backed by blockbinary
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

#include <universal/number/shared/decimal_encoding.hpp>
#include <universal/number/shared/decimal_bits.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/dfloat/dpd_codec.hpp>

namespace sw { namespace universal {

// blockdecimal: unsigned decimal integer with compact encoding
//
// ndigits  : number of decimal digits
// encoding : decimal encoding format (BCD, BID, or DPD)
// bt       : block type for underlying blockbinary storage
//
// BCD: 4 bits per digit (nibble access)
// BID: binary integer representation (value stored directly)
// DPD: 10 bits per 3 digits (declet access)
//
// The digit at index 0 is the least significant digit.
template<unsigned _ndigits, DecimalEncoding _encoding = DecimalEncoding::BCD, typename bt = uint8_t>
class blockdecimal {
public:
	static_assert(_ndigits > 0, "blockdecimal requires at least 1 digit");

	static constexpr unsigned ndigits  = _ndigits;
	static constexpr DecimalEncoding encoding = _encoding;
	using BlockType = bt;

	// compute the number of storage bits based on the encoding
	static constexpr unsigned nbits =
		(encoding == DecimalEncoding::BCD) ? bcd_bits(ndigits) :
		(encoding == DecimalEncoding::BID) ? bid_bits(ndigits) :
		                                     dpd_bits(ndigits);

	using StorageType = blockbinary<nbits, bt, BinaryNumberType::Unsigned>;

	// constructors - trivial default for trivial constructibility
	blockdecimal() = default;
	blockdecimal(const blockdecimal&) = default;
	blockdecimal(blockdecimal&&) = default;
	blockdecimal& operator=(const blockdecimal&) = default;
	blockdecimal& operator=(blockdecimal&&) = default;

	// construct from unsigned native integer
	blockdecimal(uint64_t value) { *this = value; }

	// assignment from unsigned native integer
	blockdecimal& operator=(uint64_t value) {
		if constexpr (encoding == DecimalEncoding::BID) {
			from_uint64(value);
		} else {
			clear();
			for (unsigned i = 0; i < ndigits && value > 0; ++i) {
				setdigit(i, static_cast<unsigned>(value % 10));
				value /= 10;
			}
		}
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////
	// digit access (encoding-aware)

	// get digit at position i (0 = least significant)
	unsigned digit(unsigned i) const {
		assert(i < ndigits);
		if constexpr (encoding == DecimalEncoding::BCD) {
			return extract_nibble(i);
		} else if constexpr (encoding == DecimalEncoding::BID) {
			uint64_t val = to_uint64();
			for (unsigned j = 0; j < i; ++j) val /= 10;
			return static_cast<unsigned>(val % 10);
		} else { // DPD
			return dpd_extract_digit(i);
		}
	}

	// set digit at position i (0 = least significant)
	void setdigit(unsigned i, unsigned d) {
		assert(i < ndigits);
		assert(d <= 9);
		if constexpr (encoding == DecimalEncoding::BCD) {
			set_nibble(i, d);
		} else if constexpr (encoding == DecimalEncoding::BID) {
			// read-modify-write through uint64_t
			uint64_t val = to_uint64();
			uint64_t p = pow10(i);
			unsigned old_digit = static_cast<unsigned>((val / p) % 10);
			val = val - old_digit * p + d * p;
			from_uint64(val);
		} else { // DPD
			dpd_set_digit(i, d);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// queries

	bool iszero() const { return _block.iszero(); }

	void clear() { _block.clear(); }

	// set all digits to 9 (max representable value)
	void maxval() {
		if constexpr (encoding == DecimalEncoding::BID) {
			uint64_t max_v = pow10(ndigits) - 1;
			from_uint64(max_v);
		} else {
			for (unsigned i = 0; i < ndigits; ++i) setdigit(i, 9);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// conversion

	// convert to uint64_t (may overflow for large ndigits)
	uint64_t to_uint64() const {
		if constexpr (encoding == DecimalEncoding::BID) {
			return bb_to_uint64();
		} else {
			uint64_t result = 0;
			uint64_t scale = 1;
			for (unsigned i = 0; i < ndigits; ++i) {
				result += digit(i) * scale;
				scale *= 10;
			}
			return result;
		}
	}

	// convert to double
	double to_double() const {
		double result = 0.0;
		double scale = 1.0;
		for (unsigned i = 0; i < ndigits; ++i) {
			result += digit(i) * scale;
			scale *= 10.0;
		}
		return result;
	}

	// convert to string
	std::string to_string() const {
		std::string s;
		bool leading = true;
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			unsigned d = digit(static_cast<unsigned>(i));
			if (leading && d == 0 && i > 0) continue;
			leading = false;
			s += static_cast<char>('0' + d);
		}
		return s;
	}

	/////////////////////////////////////////////////////////////////////////
	// arithmetic operators

	// addition with decimal adjust
	blockdecimal& operator+=(const blockdecimal& rhs) {
		unsigned carry = 0;
		for (unsigned i = 0; i < ndigits; ++i) {
			unsigned sum = digit(i) + rhs.digit(i) + carry;
			if (sum > 9) {
				sum -= 10;
				carry = 1;
			} else {
				carry = 0;
			}
			setdigit(i, sum);
		}
		return *this;
	}

	// subtraction: assumes *this >= rhs (unsigned subtraction)
	blockdecimal& operator-=(const blockdecimal& rhs) {
		int borrow = 0;
		for (unsigned i = 0; i < ndigits; ++i) {
			int diff = static_cast<int>(digit(i)) - static_cast<int>(rhs.digit(i)) - borrow;
			if (diff < 0) {
				diff += 10;
				borrow = 1;
			} else {
				borrow = 0;
			}
			setdigit(i, static_cast<unsigned>(diff));
		}
		return *this;
	}

	// multiplication: schoolbook digit-by-digit
	blockdecimal& operator*=(const blockdecimal& rhs) {
		blockdecimal result;
		result.clear();
		for (unsigned i = 0; i < ndigits; ++i) {
			unsigned rd = rhs.digit(i);
			if (rd == 0) continue;
			unsigned carry = 0;
			for (unsigned j = 0; j < ndigits; ++j) {
				if (i + j >= ndigits) break;
				unsigned prod = digit(j) * rd + result.digit(i + j) + carry;
				result.setdigit(i + j, prod % 10);
				carry = prod / 10;
			}
		}
		*this = result;
		return *this;
	}

	// division by single digit (helper)
	blockdecimal& divide_by(unsigned divisor, unsigned& remainder) {
		assert(divisor > 0 && divisor <= 9);
		remainder = 0;
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			unsigned cur = remainder * 10 + digit(static_cast<unsigned>(i));
			setdigit(static_cast<unsigned>(i), cur / divisor);
			remainder = cur % divisor;
		}
		return *this;
	}

	// long division by another blockdecimal
	blockdecimal& operator/=(const blockdecimal& rhs) {
		if (rhs.iszero()) return *this;
		blockdecimal quotient;
		blockdecimal remainder;
		quotient.clear();
		remainder.clear();
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			for (int j = static_cast<int>(ndigits) - 1; j > 0; --j) {
				remainder.setdigit(static_cast<unsigned>(j), remainder.digit(static_cast<unsigned>(j - 1)));
			}
			remainder.setdigit(0, digit(static_cast<unsigned>(i)));
			unsigned q = 0;
			while (!less_than(remainder, rhs)) {
				remainder -= rhs;
				++q;
			}
			quotient.setdigit(static_cast<unsigned>(i), q);
		}
		*this = quotient;
		return *this;
	}

	blockdecimal& operator%=(const blockdecimal& rhs) {
		if (rhs.iszero()) return *this;
		blockdecimal quotient;
		blockdecimal remainder;
		quotient.clear();
		remainder.clear();
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			for (int j = static_cast<int>(ndigits) - 1; j > 0; --j) {
				remainder.setdigit(static_cast<unsigned>(j), remainder.digit(static_cast<unsigned>(j - 1)));
			}
			remainder.setdigit(0, digit(static_cast<unsigned>(i)));
			while (!less_than(remainder, rhs)) {
				remainder -= rhs;
			}
		}
		*this = remainder;
		return *this;
	}

	// multiply by a power of 10 (shift left by decimal positions)
	void shift_left(unsigned positions) {
		if (positions == 0) return;
		if (positions >= ndigits) { clear(); return; }
		for (int i = static_cast<int>(ndigits) - 1; i >= static_cast<int>(positions); --i) {
			setdigit(static_cast<unsigned>(i), digit(static_cast<unsigned>(i - static_cast<int>(positions))));
		}
		for (unsigned i = 0; i < positions; ++i) {
			setdigit(i, 0);
		}
	}

	// divide by a power of 10 (shift right by decimal positions)
	void shift_right(unsigned positions) {
		if (positions == 0) return;
		if (positions >= ndigits) { clear(); return; }
		for (unsigned i = 0; i < ndigits - positions; ++i) {
			setdigit(i, digit(i + positions));
		}
		for (unsigned i = ndigits - positions; i < ndigits; ++i) {
			setdigit(i, 0);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// comparison operators

	friend bool operator==(const blockdecimal& lhs, const blockdecimal& rhs) {
		return lhs._block == rhs._block;
	}
	friend bool operator!=(const blockdecimal& lhs, const blockdecimal& rhs) {
		return !(lhs == rhs);
	}
	friend bool operator<(const blockdecimal& lhs, const blockdecimal& rhs) {
		return less_than(lhs, rhs);
	}
	friend bool operator>(const blockdecimal& lhs, const blockdecimal& rhs) {
		return rhs < lhs;
	}
	friend bool operator<=(const blockdecimal& lhs, const blockdecimal& rhs) {
		return !(rhs < lhs);
	}
	friend bool operator>=(const blockdecimal& lhs, const blockdecimal& rhs) {
		return !(lhs < rhs);
	}

	/////////////////////////////////////////////////////////////////////////
	// stream I/O

	friend std::ostream& operator<<(std::ostream& os, const blockdecimal& v) {
		return os << v.to_string();
	}

	// access to underlying bit storage
	const StorageType& bits() const { return _block; }
	StorageType& bits() { return _block; }

private:
	StorageType _block;

	/////////////////////////////////////////////////////////////////////////
	// power of 10 helper
	static constexpr uint64_t pow10(unsigned n) {
		uint64_t result = 1;
		for (unsigned i = 0; i < n; ++i) result *= 10;
		return result;
	}

	/////////////////////////////////////////////////////////////////////////
	// blockbinary <-> uint64_t helpers (for BID and DPD)

	uint64_t bb_to_uint64() const {
		uint64_t value = 0;
		constexpr unsigned maxbit = (nbits < 64) ? nbits : 64;
		for (unsigned i = 0; i < maxbit; ++i) {
			if (_block.test(i)) value |= (uint64_t(1) << i);
		}
		return value;
	}

	void from_uint64(uint64_t value) {
		_block.clear();
		constexpr unsigned maxbit = (nbits < 64) ? nbits : 64;
		for (unsigned i = 0; i < maxbit; ++i) {
			if (value & (uint64_t(1) << i)) _block.setbit(i);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// BCD nibble access helpers

	unsigned extract_nibble(unsigned digit_pos) const {
		unsigned bit_pos = digit_pos * 4;
		unsigned nibble = 0;
		for (unsigned b = 0; b < 4; ++b) {
			if (_block.test(bit_pos + b))
				nibble |= (1u << b);
		}
		return nibble;
	}

	void set_nibble(unsigned digit_pos, unsigned value) {
		unsigned bit_pos = digit_pos * 4;
		for (unsigned b = 0; b < 4; ++b) {
			_block.setbit(bit_pos + b, (value >> b) & 1);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// DPD declet access helpers

	// extract a 10-bit declet starting at bit position 'bit_start'
	uint16_t extract_declet(unsigned bit_start) const {
		uint16_t declet = 0;
		for (unsigned b = 0; b < 10; ++b) {
			if (bit_start + b < nbits && _block.test(bit_start + b))
				declet |= static_cast<uint16_t>(1u << b);
		}
		return declet;
	}

	// store a 10-bit declet starting at bit position 'bit_start'
	void store_declet(unsigned bit_start, uint16_t declet) {
		for (unsigned b = 0; b < 10; ++b) {
			if (bit_start + b < nbits)
				_block.setbit(bit_start + b, (declet >> b) & 1);
		}
	}

	// extract 4-bit BCD digit from DPD remainder bits
	unsigned extract_dpd_remainder_digit(unsigned bit_start) const {
		unsigned val = 0;
		for (unsigned b = 0; b < 4; ++b) {
			if (bit_start + b < nbits && _block.test(bit_start + b))
				val |= (1u << b);
		}
		return val;
	}

	// store 4-bit BCD digit into DPD remainder bits
	void store_dpd_remainder_digit(unsigned bit_start, unsigned d) {
		for (unsigned b = 0; b < 4; ++b) {
			if (bit_start + b < nbits)
				_block.setbit(bit_start + b, (d >> b) & 1);
		}
	}

	// DPD digit extraction: extract the i-th decimal digit
	unsigned dpd_extract_digit(unsigned i) const {
		// digits are organized as: groups of 3 from LSB, with remainder at top
		unsigned group = i / 3;
		unsigned pos_in_group = i % 3;  // 0=units, 1=tens, 2=hundreds of the group
		unsigned full_groups = ndigits / 3;
		unsigned remainder_digits = ndigits % 3;

		if (group < full_groups) {
			// this digit is in a full declet
			unsigned bit_start = group * 10;
			uint16_t declet = extract_declet(bit_start);
			unsigned value = dpd_decode(declet); // returns d0*100 + d1*10 + d2
			// d2 = units (pos 0), d1 = tens (pos 1), d0 = hundreds (pos 2)
			if (pos_in_group == 0) return value % 10;
			if (pos_in_group == 1) return (value / 10) % 10;
			return (value / 100) % 10;
		} else {
			// this digit is in the remainder portion
			unsigned rem_pos = i - full_groups * 3;  // position within remainder
			unsigned bit_start = full_groups * 10;
			if (remainder_digits == 1) {
				return extract_dpd_remainder_digit(bit_start);
			} else { // remainder_digits == 2
				if (rem_pos == 0) {
					return extract_dpd_remainder_digit(bit_start) & 0xF;
				} else {
					return extract_dpd_remainder_digit(bit_start + 4) & 0xF;
				}
			}
		}
	}

	// DPD digit setting: set the i-th decimal digit
	void dpd_set_digit(unsigned i, unsigned d) {
		unsigned group = i / 3;
		unsigned pos_in_group = i % 3;
		unsigned full_groups = ndigits / 3;
		unsigned remainder_digits = ndigits % 3;

		if (group < full_groups) {
			unsigned bit_start = group * 10;
			uint16_t declet = extract_declet(bit_start);
			unsigned value = dpd_decode(declet);
			unsigned d2 = value % 10;
			unsigned d1 = (value / 10) % 10;
			unsigned d0 = (value / 100) % 10;
			if (pos_in_group == 0) d2 = d;
			else if (pos_in_group == 1) d1 = d;
			else d0 = d;
			store_declet(bit_start, dpd_encode(d0 * 100 + d1 * 10 + d2));
		} else {
			unsigned rem_pos = i - full_groups * 3;
			unsigned bit_start = full_groups * 10;
			if (remainder_digits == 1) {
				store_dpd_remainder_digit(bit_start, d);
			} else {
				if (rem_pos == 0) {
					store_dpd_remainder_digit(bit_start, d);
				} else {
					store_dpd_remainder_digit(bit_start + 4, d);
				}
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// comparison

	// digit-by-digit comparison (MSD to LSD)
	static bool less_than(const blockdecimal& lhs, const blockdecimal& rhs) {
		if constexpr (encoding == DecimalEncoding::BID) {
			return lhs._block < rhs._block;
		} else {
			for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
				unsigned ld = lhs.digit(static_cast<unsigned>(i));
				unsigned rd = rhs.digit(static_cast<unsigned>(i));
				if (ld < rd) return true;
				if (ld > rd) return false;
			}
			return false; // equal
		}
	}
};

// free function: wide multiply returning 2*ndigits result
template<unsigned ndigits, DecimalEncoding encoding, typename bt>
blockdecimal<2 * ndigits, encoding, bt> wide_mul(
	const blockdecimal<ndigits, encoding, bt>& lhs,
	const blockdecimal<ndigits, encoding, bt>& rhs) {
	blockdecimal<2 * ndigits, encoding, bt> result;
	result.clear();
	for (unsigned i = 0; i < ndigits; ++i) {
		unsigned rd = rhs.digit(i);
		if (rd == 0) continue;
		unsigned carry = 0;
		for (unsigned j = 0; j < ndigits; ++j) {
			unsigned prod = lhs.digit(j) * rd + result.digit(i + j) + carry;
			result.setdigit(i + j, prod % 10);
			carry = prod / 10;
		}
		if (i + ndigits < 2 * ndigits) {
			result.setdigit(i + ndigits, carry);
		}
	}
	return result;
}

}} // namespace sw::universal
