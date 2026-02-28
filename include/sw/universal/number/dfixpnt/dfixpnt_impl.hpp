#pragma once
// dfixpnt_impl.hpp: implementation of a decimal fixed-point number type
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
#include <cmath>
#include <cassert>

// supporting types and functions
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/shared/decimal_encoding.hpp>
#include <universal/number/shared/decimal_bits.hpp>

// exceptions
#include <universal/number/dfixpnt/exceptions.hpp>

// the building block
#include <universal/internal/blockdecimal/blockdecimal.hpp>

namespace sw { namespace universal {

// arithmetic policy constants (same as fixpnt, constexpr bool with internal linkage)
// Modulo and Saturate are already defined in fixpnt_impl.hpp; guard against redefinition
#ifndef SW_UNIVERSAL_MODULO_SATURATE_DEFINED
#define SW_UNIVERSAL_MODULO_SATURATE_DEFINED
constexpr bool Modulo   = true;
constexpr bool Saturate = !Modulo;
#endif

// dfixpnt: a signed decimal fixed-point number
//
// _ndigits  : total number of decimal digits
// _radix    : number of fractional digits (digits after the decimal point)
// _encoding : decimal encoding format (BCD default)
// _arithmetic : Modulo (true) or Saturate (false) overflow behavior
// bt        : block type for underlying storage
//
// Value = (-1)^sign * _block * 10^(-radix)
// Example: dfixpnt<8,3> has 8 total digits, 3 fractional -> range +/-99999.999
template<unsigned _ndigits, unsigned _radix,
         DecimalEncoding _encoding = DecimalEncoding::BCD,
         bool _arithmetic = Modulo, typename bt = uint8_t>
class dfixpnt {
public:
	static_assert(_ndigits > 0, "dfixpnt requires at least 1 digit");
	static_assert(_ndigits >= _radix, "dfixpnt: ndigits must be >= radix (fractional digits)");

	static constexpr unsigned  ndigits    = _ndigits;
	static constexpr unsigned  radix      = _radix;
	static constexpr unsigned  idigits    = ndigits - radix;  // integer digits
	static constexpr DecimalEncoding encoding = _encoding;
	static constexpr bool      arithmetic = _arithmetic;
	using BlockType = bt;

	// constructors - trivial for trivial constructibility
	dfixpnt() = default;
	dfixpnt(const dfixpnt&) = default;
	dfixpnt(dfixpnt&&) = default;
	dfixpnt& operator=(const dfixpnt&) = default;
	dfixpnt& operator=(dfixpnt&&) = default;

	// specific value constructor
	dfixpnt(const SpecificValue code) : _sign{ false }, _block{} {
		switch (code) {
		case SpecificValue::infpos:
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::infneg:
		case SpecificValue::maxneg:
			maxneg();
			break;
		}
	}

	// constructors from native integer types
	dfixpnt(signed char iv)        { *this = static_cast<long long>(iv); }
	dfixpnt(short iv)              { *this = static_cast<long long>(iv); }
	dfixpnt(int iv)                { *this = static_cast<long long>(iv); }
	dfixpnt(long iv)               { *this = static_cast<long long>(iv); }
	dfixpnt(long long iv)          { *this = iv; }
	dfixpnt(char iv)               { *this = static_cast<long long>(iv); }
	dfixpnt(unsigned short iv)     { *this = static_cast<unsigned long long>(iv); }
	dfixpnt(unsigned int iv)       { *this = static_cast<unsigned long long>(iv); }
	dfixpnt(unsigned long iv)      { *this = static_cast<unsigned long long>(iv); }
	dfixpnt(unsigned long long iv) { *this = iv; }

	// constructors from floating-point types
	dfixpnt(float iv)  { *this = static_cast<double>(iv); }
	dfixpnt(double iv) { *this = iv; }

	/////////////////////////////////////////////////////////////////////////
	// assignment operators for native types

	dfixpnt& operator=(long long rhs) {
		clear();
		if (rhs < 0) {
			_sign = true;
			rhs = -rhs;
		} else {
			_sign = false;
		}
		uint64_t value = static_cast<uint64_t>(rhs);
		// the integer portion starts at digit position 'radix'
		for (unsigned i = 0; i < idigits && value > 0; ++i) {
			_block.setdigit(radix + i, static_cast<unsigned>(value % 10));
			value /= 10;
		}
		return *this;
	}

	dfixpnt& operator=(unsigned long long rhs) {
		clear();
		_sign = false;
		uint64_t value = rhs;
		for (unsigned i = 0; i < idigits && value > 0; ++i) {
			_block.setdigit(radix + i, static_cast<unsigned>(value % 10));
			value /= 10;
		}
		return *this;
	}

	dfixpnt& operator=(double rhs) {
		clear();
		if (rhs < 0) {
			_sign = true;
			rhs = -rhs;
		} else {
			_sign = false;
		}
		// scale up by 10^radix to get the fixed-point integer representation
		double scaled = rhs;
		for (unsigned i = 0; i < radix; ++i) scaled *= 10.0;
		// round to nearest
		uint64_t value = static_cast<uint64_t>(scaled + 0.5);
		for (unsigned i = 0; i < ndigits && value > 0; ++i) {
			_block.setdigit(i, static_cast<unsigned>(value % 10));
			value /= 10;
		}
		return *this;
	}

	dfixpnt& operator=(signed char rhs)        { return *this = static_cast<long long>(rhs); }
	dfixpnt& operator=(short rhs)              { return *this = static_cast<long long>(rhs); }
	dfixpnt& operator=(int rhs)                { return *this = static_cast<long long>(rhs); }
	dfixpnt& operator=(long rhs)               { return *this = static_cast<long long>(rhs); }
	dfixpnt& operator=(char rhs)               { return *this = static_cast<long long>(rhs); }
	dfixpnt& operator=(unsigned short rhs)     { return *this = static_cast<unsigned long long>(rhs); }
	dfixpnt& operator=(unsigned int rhs)       { return *this = static_cast<unsigned long long>(rhs); }
	dfixpnt& operator=(unsigned long rhs)      { return *this = static_cast<unsigned long long>(rhs); }
	dfixpnt& operator=(float rhs)              { return *this = static_cast<double>(rhs); }

	/////////////////////////////////////////////////////////////////////////
	// conversion operators

	explicit operator int() const { return static_cast<int>(to_int64()); }
	explicit operator long() const { return static_cast<long>(to_int64()); }
	explicit operator long long() const { return to_int64(); }
	explicit operator unsigned long long() const { return static_cast<unsigned long long>(to_int64()); }
	explicit operator float() const { return static_cast<float>(to_double()); }
	explicit operator double() const { return to_double(); }

	/////////////////////////////////////////////////////////////////////////
	// arithmetic operators

	// prefix increment
	dfixpnt& operator++() {
		dfixpnt one;
		one._sign = false;
		one._block.clear();
		one._block.setdigit(radix, 1); // value = 1.0
		return *this += one;
	}
	// postfix increment
	dfixpnt operator++(int) {
		dfixpnt tmp(*this);
		++(*this);
		return tmp;
	}
	// prefix decrement
	dfixpnt& operator--() {
		dfixpnt one;
		one._sign = false;
		one._block.clear();
		one._block.setdigit(radix, 1);
		return *this -= one;
	}
	// postfix decrement
	dfixpnt operator--(int) {
		dfixpnt tmp(*this);
		--(*this);
		return tmp;
	}

	// unary negation
	dfixpnt operator-() const {
		dfixpnt result(*this);
		if (!result.iszero()) result._sign = !result._sign;
		return result;
	}

	// addition
	dfixpnt& operator+=(const dfixpnt& rhs) {
		if (_sign == rhs._sign) {
			// same sign: add magnitudes
			_block += rhs._block;
			if constexpr (!arithmetic) {
				// Saturate mode: clamp to max
				blockdecimal<ndigits, _encoding, bt> maxblock;
				maxblock.maxval();
				if (maxblock < _block) _block = maxblock;
			}
		} else {
			// different signs: subtract smaller magnitude from larger
			if (_block < rhs._block) {
				// |rhs| > |this|: result takes rhs sign
				auto tmp = rhs._block;
				tmp -= _block;
				_block = tmp;
				_sign = rhs._sign;
			} else if (rhs._block < _block) {
				// |this| > |rhs|: result keeps this sign
				_block -= rhs._block;
			} else {
				// equal magnitudes: result is +0
				clear();
			}
		}
		return *this;
	}

	// subtraction
	dfixpnt& operator-=(const dfixpnt& rhs) {
		dfixpnt neg(rhs);
		if (!neg.iszero()) neg._sign = !neg._sign;
		return *this += neg;
	}

	// multiplication
	dfixpnt& operator*=(const dfixpnt& rhs) {
		bool result_sign = _sign != rhs._sign;

		// wide multiply: 2*ndigits product
		auto wide_result = wide_mul(_block, rhs._block);

		// the product has 2*radix fractional digits; we need to shift right by 'radix'
		// to get back to ndigits.radix format
		wide_result.shift_right(radix);

		// extract the lower ndigits
		blockdecimal<ndigits, _encoding, bt> result;
		result.clear();
		for (unsigned i = 0; i < ndigits; ++i) {
			result.setdigit(i, wide_result.digit(i));
		}

		if constexpr (!arithmetic) {
			// Saturate: check for overflow (any non-zero digits beyond ndigits)
			bool overflow = false;
			for (unsigned i = ndigits; i < 2 * ndigits; ++i) {
				if (wide_result.digit(i) != 0) { overflow = true; break; }
			}
			if (overflow) {
				result.maxval();
			}
		}

		_block = result;
		_sign = result_sign;
		if (_block.iszero()) _sign = false;
		return *this;
	}

	// division
	dfixpnt& operator/=(const dfixpnt& rhs) {
		if (rhs.iszero()) {
#if DFIXPNT_THROW_ARITHMETIC_EXCEPTION
			throw dfixpnt_divide_by_zero();
#else
			std::cerr << "dfixpnt: division by zero\n";
			return *this;
#endif
		}
		bool result_sign = _sign != rhs._sign;

		// upscale dividend by 10^radix: multiply _block by 10^radix
		// use a wider blockdecimal to hold the scaled value
		blockdecimal<2 * ndigits, _encoding, bt> scaled_dividend;
		scaled_dividend.clear();
		for (unsigned i = 0; i < ndigits; ++i) {
			scaled_dividend.setdigit(i + radix, _block.digit(i));
		}

		// convert divisor to same width
		blockdecimal<2 * ndigits, _encoding, bt> divisor;
		divisor.clear();
		for (unsigned i = 0; i < ndigits; ++i) {
			divisor.setdigit(i, rhs._block.digit(i));
		}

		// long division
		scaled_dividend /= divisor;

		// extract result
		blockdecimal<ndigits, _encoding, bt> result;
		result.clear();
		for (unsigned i = 0; i < ndigits; ++i) {
			result.setdigit(i, scaled_dividend.digit(i));
		}

		_block = result;
		_sign = result_sign;
		if (_block.iszero()) _sign = false;
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////
	// digit access (public for manipulators)

	unsigned digit(unsigned i) const { return _block.digit(i); }
	void setdigit(unsigned i, unsigned d) { _block.setdigit(i, d); }

	/////////////////////////////////////////////////////////////////////////
	// queries

	bool iszero() const { return _block.iszero(); }
	bool sign() const { return _sign; }
	bool ispos() const { return !_sign && !iszero(); }
	bool isneg() const { return _sign; }
	bool isinteger() const {
		for (unsigned i = 0; i < radix; ++i) {
			if (_block.digit(i) != 0) return false;
		}
		return true;
	}

	/////////////////////////////////////////////////////////////////////////
	// modifiers

	void setsign(bool s) { _sign = s; }

	void clear() {
		_sign = false;
		_block.clear();
	}
	void setzero() { clear(); }

	// set to specific extremes
	dfixpnt& zero() {
		clear();
		return *this;
	}
	dfixpnt& minpos() {
		clear();
		_block.setdigit(0, 1);
		return *this;
	}
	dfixpnt& maxpos() {
		clear();
		_block.maxval();
		return *this;
	}
	dfixpnt& minneg() {
		clear();
		_sign = true;
		_block.setdigit(0, 1);
		return *this;
	}
	dfixpnt& maxneg() {
		clear();
		_sign = true;
		_block.maxval();
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////
	// string I/O

	// parse a decimal string like "123.456" or "-0.01"
	dfixpnt& assign(const std::string& str) {
		clear();
		if (str.empty()) return *this;

		size_t pos = 0;
		if (str[0] == '-') {
			_sign = true;
			pos = 1;
		} else if (str[0] == '+') {
			pos = 1;
		}

		// find decimal point
		size_t dot_pos = str.find('.', pos);

		// parse integer part
		std::string int_part;
		if (dot_pos != std::string::npos) {
			int_part = str.substr(pos, dot_pos - pos);
		} else {
			int_part = str.substr(pos);
		}

		// parse fractional part
		std::string frac_part;
		if (dot_pos != std::string::npos) {
			frac_part = str.substr(dot_pos + 1);
		}

		// fill integer digits (from LSD to MSD)
		for (int i = static_cast<int>(int_part.size()) - 1, d = 0; i >= 0 && d < static_cast<int>(idigits); --i, ++d) {
			unsigned digit_val = static_cast<unsigned>(int_part[static_cast<size_t>(i)] - '0');
			_block.setdigit(radix + static_cast<unsigned>(d), digit_val);
		}

		// fill fractional digits (from MSD to LSD)
		for (unsigned i = 0; i < radix && i < frac_part.size(); ++i) {
			unsigned digit_val = static_cast<unsigned>(frac_part[i] - '0');
			_block.setdigit(radix - 1 - i, digit_val);
		}

		if (_block.iszero()) _sign = false;
		return *this;
	}

	// convert to string with decimal point
	std::string to_string() const {
		std::string s;
		if (_sign) s += '-';

		// integer part
		bool leading = true;
		for (int i = static_cast<int>(ndigits) - 1; i >= static_cast<int>(radix); --i) {
			unsigned d = _block.digit(static_cast<unsigned>(i));
			if (leading && d == 0 && i > static_cast<int>(radix)) continue;
			leading = false;
			s += static_cast<char>('0' + d);
		}
		if (leading) s += '0'; // all integer digits were zero

		if (radix > 0) {
			s += '.';
			for (int i = static_cast<int>(radix) - 1; i >= 0; --i) {
				s += static_cast<char>('0' + _block.digit(static_cast<unsigned>(i)));
			}
		}
		return s;
	}

	/////////////////////////////////////////////////////////////////////////
	// stream I/O

	friend std::ostream& operator<<(std::ostream& os, const dfixpnt& v) {
		return os << v.to_string();
	}

	friend std::istream& operator>>(std::istream& is, dfixpnt& v) {
		std::string s;
		is >> s;
		v.assign(s);
		return is;
	}

	/////////////////////////////////////////////////////////////////////////
	// comparison operators

	friend bool operator==(const dfixpnt& lhs, const dfixpnt& rhs) {
		if (lhs.iszero() && rhs.iszero()) return true; // +0 == -0
		if (lhs._sign != rhs._sign) return false;
		return lhs._block == rhs._block;
	}
	friend bool operator!=(const dfixpnt& lhs, const dfixpnt& rhs) {
		return !(lhs == rhs);
	}
	friend bool operator<(const dfixpnt& lhs, const dfixpnt& rhs) {
		if (lhs.iszero() && rhs.iszero()) return false;
		if (lhs._sign && !rhs._sign) return true;   // neg < pos
		if (!lhs._sign && rhs._sign) return false;   // pos > neg
		if (!lhs._sign) {
			// both positive: compare magnitudes
			return lhs._block < rhs._block;
		}
		// both negative: larger magnitude is smaller value
		return rhs._block < lhs._block;
	}
	friend bool operator>(const dfixpnt& lhs, const dfixpnt& rhs) {
		return rhs < lhs;
	}
	friend bool operator<=(const dfixpnt& lhs, const dfixpnt& rhs) {
		return !(rhs < lhs);
	}
	friend bool operator>=(const dfixpnt& lhs, const dfixpnt& rhs) {
		return !(lhs < rhs);
	}

	// access to internal block (for testing/debug)
	const blockdecimal<ndigits, _encoding, bt>& block() const { return _block; }

private:
	bool _sign;
	blockdecimal<ndigits, _encoding, bt> _block;

	// convert to int64_t (truncates fractional part)
	long long to_int64() const {
		long long result = 0;
		long long scale = 1;
		for (unsigned i = radix; i < ndigits; ++i) {
			result += _block.digit(i) * scale;
			scale *= 10;
		}
		return _sign ? -result : result;
	}

	// convert to double
	double to_double() const {
		double result = 0.0;
		double scale = 1.0;
		for (unsigned i = 0; i < ndigits; ++i) {
			result += _block.digit(i) * scale;
			scale *= 10.0;
		}
		// scale down by 10^radix
		double divisor = 1.0;
		for (unsigned i = 0; i < radix; ++i) divisor *= 10.0;
		result /= divisor;
		return _sign ? -result : result;
	}
};

/////////////////////////////////////////////////////////////////////////
// binary arithmetic operators (non-member)

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt>
operator+(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& lhs,
          const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& rhs) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(lhs);
	result += rhs;
	return result;
}

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt>
operator-(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& lhs,
          const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& rhs) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(lhs);
	result -= rhs;
	return result;
}

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt>
operator*(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& lhs,
          const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& rhs) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(lhs);
	result *= rhs;
	return result;
}

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt>
operator/(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& lhs,
          const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& rhs) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(lhs);
	result /= rhs;
	return result;
}

}} // namespace sw::universal
