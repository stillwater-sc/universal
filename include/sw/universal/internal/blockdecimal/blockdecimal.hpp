#pragma once
// blockdecimal.hpp: signed decimal integer with compact encoding backed by blockbinary
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
#include <limits>

#include <universal/number/shared/decimal_encoding.hpp>
#include <universal/number/shared/decimal_bits.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/dfloat/dpd_codec.hpp>

namespace sw { namespace universal {

// blockdecimal: signed decimal integer with compact encoding
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
// Sign is stored separately (sign-magnitude representation).
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
	// NOTE: _negative is NOT initialized here (triviality requirement)
	blockdecimal() = default;
	blockdecimal(const blockdecimal&) = default;
	blockdecimal(blockdecimal&&) = default;
	blockdecimal& operator=(const blockdecimal&) = default;
	blockdecimal& operator=(blockdecimal&&) = default;

	// construct from unsigned native integer
	blockdecimal(unsigned long long value) { convert_unsigned(value); }

	// construct from signed native integer
	blockdecimal(int value)       { convert_signed(static_cast<long long>(value)); }
	blockdecimal(long value)      { convert_signed(static_cast<long long>(value)); }
	blockdecimal(long long value) { convert_signed(value); }

	// assignment from native integer types
	blockdecimal& operator=(int rhs)                { return convert_signed(static_cast<long long>(rhs)); }
	blockdecimal& operator=(long rhs)               { return convert_signed(static_cast<long long>(rhs)); }
	blockdecimal& operator=(long long rhs)           { return convert_signed(rhs); }
	blockdecimal& operator=(unsigned int rhs)        { return convert_unsigned(static_cast<unsigned long long>(rhs)); }
	blockdecimal& operator=(unsigned long rhs)       { return convert_unsigned(static_cast<unsigned long long>(rhs)); }
	blockdecimal& operator=(unsigned long long rhs)  { return convert_unsigned(rhs); }

	// explicit conversion operators
	explicit operator long long() const noexcept { return to_long_long(); }
	explicit operator unsigned long long() const noexcept { return static_cast<unsigned long long>(to_long_long()); }
	explicit operator int() const noexcept { return static_cast<int>(to_long_long()); }
	explicit operator long() const noexcept { return static_cast<long>(to_long_long()); }
	explicit operator unsigned int() const noexcept { return static_cast<unsigned int>(to_long_long()); }
	explicit operator unsigned long() const noexcept { return static_cast<unsigned long>(to_long_long()); }
	explicit operator float() const noexcept { return static_cast<float>(to_double()); }
	explicit operator double() const noexcept { return to_double(); }

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
	bool sign() const { return _negative; }
	bool isneg() const { return _negative && !iszero(); }
	bool ispos() const { return !_negative || iszero(); }

	void clear() { _negative = false; _block.clear(); }

	void setsign(bool s) { _negative = s; }
	void setbits(uint64_t v) { convert_unsigned(v); }

	// set all digits to 9 (max representable value)
	void maxval() {
		_negative = false;
		if constexpr (encoding == DecimalEncoding::BID) {
			uint64_t max_v = pow10(ndigits) - 1;
			from_uint64(max_v);
		} else {
			for (unsigned i = 0; i < ndigits; ++i) setdigit(i, 9);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// conversion

	// convert magnitude to uint64_t (may overflow for large ndigits)
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

	// convert to long long (signed)
	long long to_long_long() const noexcept {
		long long v = static_cast<long long>(to_uint64());
		return _negative ? -v : v;
	}

	// convert to double (signed)
	double to_double() const {
		double result = 0.0;
		double scale = 1.0;
		for (unsigned i = 0; i < ndigits; ++i) {
			result += digit(i) * scale;
			scale *= 10.0;
		}
		return _negative ? -result : result;
	}

	// convert to string (signed)
	std::string to_string() const {
		std::string s;
		if (_negative && !iszero()) s += '-';
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
	// unary negation

	blockdecimal operator-() const {
		blockdecimal tmp(*this);
		if (!tmp.iszero()) tmp._negative = !tmp._negative;
		return tmp;
	}

	/////////////////////////////////////////////////////////////////////////
	// arithmetic operators (sign-magnitude)

	// addition
	blockdecimal& operator+=(const blockdecimal& rhs) {
		if (_negative != rhs._negative) {
			// different signs: subtract magnitude of rhs
			blockdecimal tmp(rhs);
			tmp._negative = !tmp._negative;
			return operator-=(tmp);
		}
		// same sign: add magnitudes, sign stays the same
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

	// subtraction
	blockdecimal& operator-=(const blockdecimal& rhs) {
		if (_negative != rhs._negative) {
			// different signs: add magnitude of rhs
			blockdecimal tmp(rhs);
			tmp._negative = !tmp._negative;
			return operator+=(tmp);
		}
		// same sign: subtract magnitudes
		int cmp = compare_magnitude(rhs);
		if (cmp == 0) {
			clear();
			return *this;
		}
		if (cmp < 0) {
			// |this| < |rhs|: result = rhs magnitude - this magnitude, flip sign
			blockdecimal tmp(rhs);
			int borrow = 0;
			for (unsigned i = 0; i < ndigits; ++i) {
				int diff = static_cast<int>(tmp.digit(i)) - static_cast<int>(digit(i)) - borrow;
				if (diff < 0) {
					diff += 10;
					borrow = 1;
				} else {
					borrow = 0;
				}
				setdigit(i, static_cast<unsigned>(diff));
			}
			_negative = !_negative;
			if (iszero()) _negative = false;
			return *this;
		}
		// |this| > |rhs|: subtract rhs from this, sign unchanged
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
		if (iszero()) _negative = false;
		return *this;
	}

	// multiplication: schoolbook digit-by-digit
	blockdecimal& operator*=(const blockdecimal& rhs) {
		if (iszero() || rhs.iszero()) {
			clear();
			return *this;
		}
		bool resultSign = (_negative != rhs._negative);
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
		result._negative = resultSign;
		*this = result;
		return *this;
	}

	// division by single digit (helper) - magnitude only
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
		bool resultSign = (_negative != rhs._negative);
		// work with magnitudes
		blockdecimal a(*this); a._negative = false;
		blockdecimal b(rhs);   b._negative = false;
		blockdecimal quotient;
		blockdecimal remainder;
		quotient.clear();
		remainder.clear();
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			for (int j = static_cast<int>(ndigits) - 1; j > 0; --j) {
				remainder.setdigit(static_cast<unsigned>(j), remainder.digit(static_cast<unsigned>(j - 1)));
			}
			remainder.setdigit(0, a.digit(static_cast<unsigned>(i)));
			unsigned q = 0;
			while (!less_than_magnitude(remainder, b)) {
				remainder -= b;
				++q;
			}
			quotient.setdigit(static_cast<unsigned>(i), q);
		}
		quotient._negative = resultSign;
		if (quotient.iszero()) quotient._negative = false;
		*this = quotient;
		return *this;
	}

	blockdecimal& operator%=(const blockdecimal& rhs) {
		if (rhs.iszero()) return *this;
		bool remSign = _negative;
		// work with magnitudes
		blockdecimal a(*this); a._negative = false;
		blockdecimal b(rhs);   b._negative = false;
		blockdecimal remainder;
		remainder.clear();
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			for (int j = static_cast<int>(ndigits) - 1; j > 0; --j) {
				remainder.setdigit(static_cast<unsigned>(j), remainder.digit(static_cast<unsigned>(j - 1)));
			}
			remainder.setdigit(0, a.digit(static_cast<unsigned>(i)));
			while (!less_than_magnitude(remainder, b)) {
				remainder -= b;
			}
		}
		remainder._negative = remSign;
		if (remainder.iszero()) remainder._negative = false;
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

	// digit shift operators (matching blockdigit interface)
	blockdecimal& operator<<=(int shift) {
		if (shift < 0) return operator>>=(-shift);
		shift_left(static_cast<unsigned>(shift));
		return *this;
	}
	blockdecimal& operator>>=(int shift) {
		if (shift < 0) return operator<<=(-shift);
		shift_right(static_cast<unsigned>(shift));
		if (iszero()) _negative = false;
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////
	// comparison operators (signed)

	friend bool operator==(const blockdecimal& lhs, const blockdecimal& rhs) {
		if (lhs.iszero() && rhs.iszero()) return true; // +0 == -0
		if (lhs._negative != rhs._negative) return false;
		return lhs._block == rhs._block;
	}
	friend bool operator!=(const blockdecimal& lhs, const blockdecimal& rhs) {
		return !(lhs == rhs);
	}
	friend bool operator<(const blockdecimal& lhs, const blockdecimal& rhs) {
		bool lzero = lhs.iszero();
		bool rzero = rhs.iszero();
		if (lzero && rzero) return false;
		if (lhs._negative && !rhs._negative) return !rzero || !lzero;
		if (!lhs._negative && rhs._negative) return false;
		// same sign
		if (!lhs._negative) {
			return less_than_magnitude(lhs, rhs);
		} else {
			return less_than_magnitude(rhs, lhs);
		}
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
	friend std::istream& operator>>(std::istream& is, blockdecimal& v) {
		std::string s;
		is >> s;
		v.clear();
		if (s.empty()) return is;
		unsigned start = 0;
		if (s[0] == '-') { v._negative = true; start = 1; }
		else if (s[0] == '+') { start = 1; }
		unsigned len = static_cast<unsigned>(s.size()) - start;
		for (unsigned i = 0; i < len && i < ndigits; ++i) {
			char c = s[s.size() - 1 - i];
			if (c >= '0' && c <= '9') v.setdigit(i, static_cast<unsigned>(c - '0'));
		}
		if (v.iszero()) v._negative = false;
		return is;
	}

	// access to underlying bit storage
	const StorageType& bits() const { return _block; }
	StorageType& bits() { return _block; }

private:
	bool _negative;        // sign-magnitude sign bit (NOT initialized by default — triviality)
	StorageType _block;    // unsigned magnitude

	/////////////////////////////////////////////////////////////////////////
	// conversion helpers

	blockdecimal& convert_signed(long long rhs) {
		clear();
		if (rhs < 0) {
			_negative = true;
			unsigned long long v;
			if (rhs == std::numeric_limits<long long>::min()) {
				v = static_cast<unsigned long long>(-(rhs + 1)) + 1ull;
			} else {
				v = static_cast<unsigned long long>(-rhs);
			}
			store_magnitude(v);
		} else {
			store_magnitude(static_cast<unsigned long long>(rhs));
		}
		return *this;
	}

	blockdecimal& convert_unsigned(unsigned long long rhs) {
		clear();
		store_magnitude(rhs);
		return *this;
	}

	void store_magnitude(unsigned long long value) {
		if constexpr (encoding == DecimalEncoding::BID) {
			from_uint64(static_cast<uint64_t>(value));
		} else {
			for (unsigned i = 0; i < ndigits && value > 0; ++i) {
				setdigit(i, static_cast<unsigned>(value % 10));
				value /= 10;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// magnitude comparison (ignoring sign), returns -1, 0, +1
	int compare_magnitude(const blockdecimal& rhs) const {
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			unsigned ld = digit(static_cast<unsigned>(i));
			unsigned rd = rhs.digit(static_cast<unsigned>(i));
			if (ld < rd) return -1;
			if (ld > rd) return +1;
		}
		return 0;
	}

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
	// magnitude-only comparison (unsigned)

	// digit-by-digit comparison of magnitude (MSD to LSD)
	static bool less_than_magnitude(const blockdecimal& lhs, const blockdecimal& rhs) {
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

/////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////
// binary arithmetic operators

template<unsigned N, DecimalEncoding E, typename BT>
inline blockdecimal<N, E, BT> operator+(const blockdecimal<N, E, BT>& lhs, const blockdecimal<N, E, BT>& rhs) {
	blockdecimal<N, E, BT> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned N, DecimalEncoding E, typename BT>
inline blockdecimal<N, E, BT> operator-(const blockdecimal<N, E, BT>& lhs, const blockdecimal<N, E, BT>& rhs) {
	blockdecimal<N, E, BT> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned N, DecimalEncoding E, typename BT>
inline blockdecimal<N, E, BT> operator*(const blockdecimal<N, E, BT>& lhs, const blockdecimal<N, E, BT>& rhs) {
	blockdecimal<N, E, BT> product(lhs);
	product *= rhs;
	return product;
}
template<unsigned N, DecimalEncoding E, typename BT>
inline blockdecimal<N, E, BT> operator/(const blockdecimal<N, E, BT>& lhs, const blockdecimal<N, E, BT>& rhs) {
	blockdecimal<N, E, BT> quotient(lhs);
	quotient /= rhs;
	return quotient;
}
template<unsigned N, DecimalEncoding E, typename BT>
inline blockdecimal<N, E, BT> operator%(const blockdecimal<N, E, BT>& lhs, const blockdecimal<N, E, BT>& rhs) {
	blockdecimal<N, E, BT> remainder(lhs);
	remainder %= rhs;
	return remainder;
}

/////////////////////////////////////////////////////////////////////////
// manipulation functions

// Generate a type tag for blockdecimal
template<unsigned N, DecimalEncoding E, typename BT>
inline std::string type_tag(const blockdecimal<N, E, BT>& = {}) {
	std::stringstream s;
	s << "blockdecimal<" << N << '>';
	return s.str();
}

// to_binary: show internal digit storage
template<unsigned N, DecimalEncoding E, typename BT>
inline std::string to_binary(const blockdecimal<N, E, BT>& v) {
	std::stringstream s;
	s << (v.sign() ? '-' : '+') << "[ ";
	for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
		s << v.digit(static_cast<unsigned>(i));
		if (i > 0) s << '.';
	}
	s << " ]";
	return s.str();
}

}} // namespace sw::universal
