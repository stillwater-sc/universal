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
#include <limits>
#include <type_traits>

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
	constexpr dfixpnt(const SpecificValue code) : _sign{ false }, _block{} {
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
	constexpr dfixpnt(signed char iv)        : _sign{false}, _block{} { *this = static_cast<long long>(iv); }
	constexpr dfixpnt(short iv)              : _sign{false}, _block{} { *this = static_cast<long long>(iv); }
	constexpr dfixpnt(int iv)                : _sign{false}, _block{} { *this = static_cast<long long>(iv); }
	constexpr dfixpnt(long iv)               : _sign{false}, _block{} { *this = static_cast<long long>(iv); }
	constexpr dfixpnt(long long iv)          : _sign{false}, _block{} { *this = iv; }
	constexpr dfixpnt(char iv)               : _sign{false}, _block{} { *this = static_cast<long long>(iv); }
	constexpr dfixpnt(unsigned short iv)     : _sign{false}, _block{} { *this = static_cast<unsigned long long>(iv); }
	constexpr dfixpnt(unsigned int iv)       : _sign{false}, _block{} { *this = static_cast<unsigned long long>(iv); }
	constexpr dfixpnt(unsigned long iv)      : _sign{false}, _block{} { *this = static_cast<unsigned long long>(iv); }
	constexpr dfixpnt(unsigned long long iv) : _sign{false}, _block{} { *this = iv; }

	// constructors from floating-point types
	constexpr dfixpnt(float iv)  : _sign{false}, _block{} { *this = static_cast<double>(iv); }
	constexpr dfixpnt(double iv) : _sign{false}, _block{} { *this = iv; }

	/////////////////////////////////////////////////////////////////////////
	// assignment operators for native types

	constexpr dfixpnt& operator=(long long rhs) {
		clear();
		uint64_t value;
		if (rhs < 0) {
			_sign = true;
			// avoid -LLONG_MIN signed overflow: compute unsigned magnitude safely
			value = static_cast<uint64_t>(-(rhs + 1)) + 1ull;
		} else {
			_sign = false;
			value = static_cast<uint64_t>(rhs);
		}
		// the integer portion starts at digit position 'radix'
		for (unsigned i = 0; i < idigits && value > 0; ++i) {
			_block.setdigit(radix + i, static_cast<unsigned>(value % 10));
			value /= 10;
		}
		return *this;
	}

	constexpr dfixpnt& operator=(unsigned long long rhs) {
		clear();
		_sign = false;
		uint64_t value = rhs;
		for (unsigned i = 0; i < idigits && value > 0; ++i) {
			_block.setdigit(radix + i, static_cast<unsigned>(value % 10));
			value /= 10;
		}
		return *this;
	}

	constexpr dfixpnt& operator=(double rhs) {
		// IEEE 754 double's representable decimal exponent range is +/- ~308.
		// Once `ndigits` (or `radix`) exceeds that, the `scaled *= 10.0` and
		// `max_magnitude *= 10.0` loops below overflow to +inf -- and `inf >=
		// inf` is true, so the saturation check would silently coerce ANY
		// input (including 1.0) to all-nines.  Other constructors (string,
		// integer) don't go through this path, so the constraint is local to
		// double conversion -- the type itself remains usable for
		// arbitrarily-wide configurations.
		static_assert(ndigits <= static_cast<unsigned>(std::numeric_limits<double>::max_exponent10),
			"dfixpnt::operator=(double): ndigits exceeds double's decimal exponent range "
			"(~308); use a narrower instantiation for double conversion, or assign via a "
			"non-FP path (string / integer).");

		clear();
		// std::isnan is not constexpr in C++20; use a NaN-safe equivalent
		// (NaN is the only value not equal to itself).
		if (rhs != rhs || rhs == 0.0) return *this;
		if (rhs < 0) {
			_sign = true;
			rhs = -rhs;
		} else {
			_sign = false;
		}
		// scale up by 10^radix to get the fixed-point integer representation
		double scaled = rhs;
		for (unsigned i = 0; i < radix; ++i) scaled *= 10.0;
		// round to nearest, half away from zero (scaled >= 0 here)
		scaled += 0.5;

		// Compute representational ceiling = 10^ndigits in FP space.
		double max_magnitude = 1.0;
		for (unsigned i = 0; i < ndigits; ++i) max_magnitude *= 10.0;

		// Saturate at the storage limit before any out-of-range cast.
		// Pre-fix code materialized scaled into uint64_t, which overflows
		// (UB per C++20 [conv.fpint]) for ndigits >= 20 since 10^20 exceeds
		// UINT64_MAX (~1.84e19).  Now we extract digits in FP space, so the
		// only constraint is that scaled fit in double's exponent range
		// (covers ndigits up to ~308).
		if (scaled >= max_magnitude) {
			for (unsigned i = 0; i < ndigits; ++i) _block.setdigit(i, 9);
			return *this;
		}

		// Extract decimal digits LSD-first via FP-domain repeated div/mod by 10.
		//
		// Per iteration we want:
		//   digit_i = scaled mod 10
		//   scaled  = floor(scaled / 10)
		//
		// 10.0 is exactly representable in IEEE 754 double, but 0.1 is not,
		// so dividing by 10.0 is the closest correct primitive.  The floor
		// emulation handles two regimes:
		//
		//   * scaled / 10.0 < 2^53: q is in the dense-integer range of double,
		//     casting to unsigned long long truncates correctly.
		//   * scaled / 10.0 >= 2^53: q is already representable at integer
		//     (or coarser) granularity in double, so q is its own floor.
		//
		// For source values above 2^53 the lower decimal digits accumulate
		// FP rounding error -- intrinsic to converting from a double, which
		// carries only ~16 significant decimal digits regardless of which
		// extraction algorithm is used.
		constexpr double pow2_53 = 9007199254740992.0;  // 2^53
		double v = scaled;
		for (unsigned i = 0; i < ndigits; ++i) {
			if (v < 1.0) break;
			double q = v / 10.0;
			double q_floor = (q < pow2_53)
				? static_cast<double>(static_cast<unsigned long long>(q))
				: q;
			// digit = v - 10 * floor(v/10).  Clamp against any FP noise
			// from the subtraction so we never write an out-of-range digit.
			double d_d = v - 10.0 * q_floor;
			unsigned d = (d_d < 0.0) ? 0u
			           : (d_d >= 10.0) ? 9u
			           : static_cast<unsigned>(d_d);
			_block.setdigit(i, d);
			v = q_floor;
		}
		return *this;
	}

	constexpr dfixpnt& operator=(signed char rhs)        { return *this = static_cast<long long>(rhs); }
	constexpr dfixpnt& operator=(short rhs)              { return *this = static_cast<long long>(rhs); }
	constexpr dfixpnt& operator=(int rhs)                { return *this = static_cast<long long>(rhs); }
	constexpr dfixpnt& operator=(long rhs)               { return *this = static_cast<long long>(rhs); }
	constexpr dfixpnt& operator=(char rhs)               { return *this = static_cast<long long>(rhs); }
	constexpr dfixpnt& operator=(unsigned short rhs)     { return *this = static_cast<unsigned long long>(rhs); }
	constexpr dfixpnt& operator=(unsigned int rhs)       { return *this = static_cast<unsigned long long>(rhs); }
	constexpr dfixpnt& operator=(unsigned long rhs)      { return *this = static_cast<unsigned long long>(rhs); }
	constexpr dfixpnt& operator=(float rhs)              { return *this = static_cast<double>(rhs); }

	/////////////////////////////////////////////////////////////////////////
	// conversion operators

	constexpr explicit operator int() const { return static_cast<int>(to_int64()); }
	constexpr explicit operator long() const { return static_cast<long>(to_int64()); }
	constexpr explicit operator long long() const { return to_int64(); }
	constexpr explicit operator unsigned long long() const { return static_cast<unsigned long long>(to_int64()); }
	constexpr explicit operator float() const { return static_cast<float>(to_double()); }
	constexpr explicit operator double() const { return to_double(); }

	/////////////////////////////////////////////////////////////////////////
	// arithmetic operators

	// prefix increment
	// For dfixpnt<N, N> (idigits == 0, e.g. dfixpnt<3,3>), the type holds
	// values in (-1, 1) and 1 is not representable, so ++/-- on the integer
	// part are no-ops: setdigit(radix, 1) would write past the last digit
	// (radix == ndigits) and the storage no longer asserts.  Guarded with
	// if constexpr so the OOB path is removed at compile time.
	constexpr dfixpnt& operator++() {
		if constexpr (idigits > 0) {
			dfixpnt one;
			one._sign = false;
			one._block.clear();
			one._block.setdigit(radix, 1); // value = 1.0
			return *this += one;
		}
		// For idigits == 0, no integer-part-of-1 to add; ++ is a no-op.
		return *this;
	}
	// postfix increment
	constexpr dfixpnt operator++(int) {
		dfixpnt tmp(*this);
		++(*this);
		return tmp;
	}
	// prefix decrement
	constexpr dfixpnt& operator--() {
		if constexpr (idigits > 0) {
			dfixpnt one;
			one._sign = false;
			one._block.clear();
			one._block.setdigit(radix, 1);
			return *this -= one;
		}
		return *this;
	}
	// postfix decrement
	constexpr dfixpnt operator--(int) {
		dfixpnt tmp(*this);
		--(*this);
		return tmp;
	}

	// unary negation
	constexpr dfixpnt operator-() const {
		dfixpnt result(*this);
		if (!result.iszero()) result._sign = !result._sign;
		return result;
	}

	// addition
	constexpr dfixpnt& operator+=(const dfixpnt& rhs) {
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
	constexpr dfixpnt& operator-=(const dfixpnt& rhs) {
		dfixpnt neg(rhs);
		if (!neg.iszero()) neg._sign = !neg._sign;
		return *this += neg;
	}

	// multiplication
	constexpr dfixpnt& operator*=(const dfixpnt& rhs) {
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
	constexpr dfixpnt& operator/=(const dfixpnt& rhs) {
		if (rhs.iszero()) {
#if DFIXPNT_THROW_ARITHMETIC_EXCEPTION
			throw dfixpnt_divide_by_zero();
#else
			if (!std::is_constant_evaluated()) {
				std::cerr << "dfixpnt: division by zero\n";
			}
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

	constexpr unsigned digit(unsigned i) const { return _block.digit(i); }
	constexpr void setdigit(unsigned i, unsigned d) { _block.setdigit(i, d); }

	/////////////////////////////////////////////////////////////////////////
	// queries

	constexpr bool iszero() const { return _block.iszero(); }
	constexpr bool sign() const { return _sign; }
	constexpr bool ispos() const { return !_sign && !iszero(); }
	constexpr bool isneg() const { return _sign; }
	constexpr bool isinteger() const {
		for (unsigned i = 0; i < radix; ++i) {
			if (_block.digit(i) != 0) return false;
		}
		return true;
	}

	/////////////////////////////////////////////////////////////////////////
	// modifiers

	constexpr void setsign(bool s) { _sign = s; }

	constexpr void clear() {
		_sign = false;
		_block.clear();
	}
	constexpr void setzero() { clear(); }

	// set to specific extremes
	constexpr dfixpnt& zero() {
		clear();
		return *this;
	}
	constexpr dfixpnt& minpos() {
		clear();
		_block.setdigit(0, 1);
		return *this;
	}
	constexpr dfixpnt& maxpos() {
		clear();
		_block.maxval();
		return *this;
	}
	constexpr dfixpnt& minneg() {
		clear();
		_sign = true;
		_block.setdigit(0, 1);
		return *this;
	}
	constexpr dfixpnt& maxneg() {
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

	friend constexpr bool operator==(const dfixpnt& lhs, const dfixpnt& rhs) {
		if (lhs.iszero() && rhs.iszero()) return true; // +0 == -0
		if (lhs._sign != rhs._sign) return false;
		return lhs._block == rhs._block;
	}
	friend constexpr bool operator!=(const dfixpnt& lhs, const dfixpnt& rhs) {
		return !(lhs == rhs);
	}
	friend constexpr bool operator<(const dfixpnt& lhs, const dfixpnt& rhs) {
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
	friend constexpr bool operator>(const dfixpnt& lhs, const dfixpnt& rhs) {
		return rhs < lhs;
	}
	friend constexpr bool operator<=(const dfixpnt& lhs, const dfixpnt& rhs) {
		// dfixpnt has no NaN, but follow the #797 NaN-safe pattern for uniformity.
		return operator<(lhs, rhs) || operator==(lhs, rhs);
	}
	friend constexpr bool operator>=(const dfixpnt& lhs, const dfixpnt& rhs) {
		return operator>(lhs, rhs) || operator==(lhs, rhs);
	}

	// access to internal block (for testing/debug)
	constexpr const blockdecimal<ndigits, _encoding, bt>& block() const { return _block; }

private:
	bool _sign;
	blockdecimal<ndigits, _encoding, bt> _block;

	// Convert to long long (truncates fractional part), clamped to
	// [LLONG_MIN, LLONG_MAX].  The previous LSD-first accumulator with
	// scale *= 10 overflowed long long for idigits >= 19 (10^19 already
	// exceeds LLONG_MAX) -- UB at runtime, hard compile error in a
	// constant expression.  Switch to MSD-first Horner with per-step
	// overflow detection in unsigned long long, then clamp on cast.
	// Matches blockdecimal::to_long_long.
	constexpr long long to_int64() const {
		constexpr unsigned long long pos_max =
			static_cast<unsigned long long>(std::numeric_limits<long long>::max());
		constexpr unsigned long long neg_max = pos_max + 1ull;  // |LLONG_MIN|

		unsigned long long mag = 0;
		bool overflow = false;
		// MSD-first: iterate i from ndigits-1 down to radix (inclusive on radix).
		// Loop bound expressed as `i > radix` with `i - 1` index, so radix
		// itself is the last digit consumed.
		for (unsigned i = ndigits; i > radix; --i) {
			// Detect overflow before mag *= 10 (would wrap around in unsigned).
			if (mag > neg_max / 10ull) { overflow = true; break; }
			mag *= 10ull;
			unsigned d = _block.digit(i - 1);
			// Detect overflow before mag += d.
			if (mag > neg_max - d) { overflow = true; break; }
			mag += d;
		}
		if (_sign) {
			// Negative: |LLONG_MIN| = LLONG_MAX + 1 = neg_max, so values in
			// [0, neg_max] represent.  mag == neg_max is exactly LLONG_MIN.
			if (overflow || mag == neg_max) return std::numeric_limits<long long>::min();
			return -static_cast<long long>(mag);
		}
		// Positive: clamp at LLONG_MAX.
		if (overflow || mag > pos_max) return std::numeric_limits<long long>::max();
		return static_cast<long long>(mag);
	}

	// convert to double
	constexpr double to_double() const {
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
constexpr dfixpnt<ndigits, radix, encoding, arithmetic, bt>
operator+(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& lhs,
          const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& rhs) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(lhs);
	result += rhs;
	return result;
}

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
constexpr dfixpnt<ndigits, radix, encoding, arithmetic, bt>
operator-(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& lhs,
          const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& rhs) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(lhs);
	result -= rhs;
	return result;
}

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
constexpr dfixpnt<ndigits, radix, encoding, arithmetic, bt>
operator*(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& lhs,
          const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& rhs) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(lhs);
	result *= rhs;
	return result;
}

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
constexpr dfixpnt<ndigits, radix, encoding, arithmetic, bt>
operator/(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& lhs,
          const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& rhs) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(lhs);
	result /= rhs;
	return result;
}

}} // namespace sw::universal
