#pragma once
// dfixpnt_impl.hpp: implementation of a decimal fixed-point number type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <array>
#include <string>       // std::string: assign() and to_string() stay in the core
#include <cstdint>
#include <cstdio>       // fprintf(stderr, ...) for the division-by-zero diagnostic (#1334)
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

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// These default here rather than in the dfixpnt.hpp umbrella, so that a translation unit
/// which includes core.hpp directly gets the same defaults (#1334, #1436). They MUST stay
/// above the blockdecimal include below: the blockdecimal building block reads its own
/// BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION, and dfixpnt forwards its choice to it.
/// Defining either macro before any dfixpnt header still wins.
#if !defined(DFIXPNT_THROW_ARITHMETIC_EXCEPTION)
// default is to write a diagnostic to stderr for signalling an error
#define DFIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#if !defined(BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION)
#define BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION 0
#endif
#else
// for the blockdecimal building block assume the same behavior as requested for dfixpnt
#if !defined(BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION)
#define BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION DFIXPNT_THROW_ARITHMETIC_EXCEPTION
#endif
#endif
#include <universal/utility/exceptions_check.hpp>  // reject a throw switch without exceptions (#1925)

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

	// a double is exactly m * 2^e, so it has a finite decimal expansion: the result is that value rounded
	// once to radix fraction digits, to nearest with ties away from zero, on the magnitude. A magnitude of
	// 10^idigits or more saturates to all nines with the sign, as does an infinity; NaN gives 0. The digits
	// come from exact decimal arithmetic on m * 10^radix (doubling it e times, or halving it -e times with
	// the dropped bits kept for the rounding), not from a double product, which rounds once v * 10^radix
	// reaches 2^53 (#2076).
	constexpr dfixpnt& operator=(double rhs) {
		clear();
		// std::isnan is not constexpr in C++20; use a NaN-safe equivalent
		// (NaN is the only value not equal to itself).
		if (rhs != rhs || rhs == 0.0) return *this;
		const bool negative = (rhs < 0.0);
		if (negative) rhs = -rhs;
		if (rhs > std::numeric_limits<double>::max()) return saturated(negative);   // an infinity

		const dyadic d = decompose(rhs);
		decimal_work w{};
		std::uint64_t m = d.m;                              // m < 2^53: at most 16 digits, above the radix
		for (unsigned i = radix; m != 0; ++i) { w[i] = static_cast<std::uint8_t>(m % 10u); m /= 10u; }
		if (d.e >= 0) {
			if (beyond_storage(w)) return saturated(negative);
			for (int k = 0; k < d.e; ++k) {
				double_digits(w);
				if (beyond_storage(w)) return saturated(negative);
			}
		}
		else {
			bool half = false;                              // the last bit dropped: the rounding bit
			for (int k = 0; k < -d.e; ++k) {
				if (all_zero(w)) { half = false; break; }   // only zeros are left to drop
				half = halve_digits(w);
			}
			if (half) {                                     // ties away from zero: the sticky bits do not matter
				for (unsigned i = 0; i < work_digits; ++i) {
					if (w[i] < 9u) { ++w[i]; break; }
					w[i] = 0;
				}
			}
			if (beyond_storage(w)) return saturated(negative);   // halving could leave it above storage
		}
		for (unsigned i = 0; i < ndigits; ++i) _block.setdigit(i, w[i]);
		_sign = negative && !_block.iszero();               // a magnitude that rounds to 0 is +0
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
	// correctly rounded, to nearest with ties to even; float is rounded once, not through double
	constexpr explicit operator float() const { return static_cast<float>(to_binary<24, -126, 127>()); }
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
				std::fprintf(stderr, "dfixpnt: division by zero\n");
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

	// operator<< and operator>> are in iostream.hpp: they need only the public to_string()
	// and assign(), so they are free function templates, not friends (#1334)

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

	// convert to double, correctly rounded
	constexpr double to_double() const { return to_binary<53, -1022, 1023>(); }

	// Exact decimal arithmetic for the conversions to and from binary floating-point (#2076). The work
	// register holds ndigits + 20 digits, least significant first: room for m * 10^radix with m < 2^53
	// (16 digits above the radix), for a carry, and for a 63-bit integer part above the radix.
	static constexpr unsigned work_digits = ndigits + 20u;
	using decimal_work = std::array<std::uint8_t, work_digits>;

	// a finite positive double as m * 2^e with an integer m in [2^52, 2^53). A product with a power of two
	// is exact while it stays a normal double, and a doubling is always exact, subnormals included.
	struct dyadic { std::uint64_t m; int e; };
	static constexpr dyadic decompose(double v) {
		constexpr double two52 = 4503599627370496.0;       // 2^52
		constexpr double two53 = 9007199254740992.0;       // 2^53
		constexpr double two32 = 4294967296.0;              // 2^32
		int e = 0;
		while (v >= two53 * two32) { v /= two32; e += 32; } // stays >= 2^53: exact
		while (v >= two53) { v *= 0.5; ++e; }
		while (v < two52 / two32) { v *= two32; e -= 32; }  // stays < 2^52: exact
		while (v < two52) { v *= 2.0; --e; }
		return { static_cast<std::uint64_t>(v), e };
	}

	// v * 2^k, exactly, for a result that is representable: every intermediate keeps the bits of v, and its
	// last bit weighs no less than the result's
	static constexpr double scale2(double v, int k) {
		constexpr double two32 = 4294967296.0;
		for (; k >= 32; k -= 32) v *= two32;
		for (; k <= -32; k += 32) v /= two32;
		for (; k > 0; --k) v *= 2.0;
		for (; k < 0; ++k) v *= 0.5;
		return v;
	}

	static constexpr bool all_zero(const decimal_work& w) {
		for (unsigned i = 0; i < work_digits; ++i) if (w[i] != 0) return false;
		return true;
	}
	// a magnitude of 10^ndigits or more
	static constexpr bool beyond_storage(const decimal_work& w) {
		for (unsigned i = ndigits; i < work_digits; ++i) if (w[i] != 0) return true;
		return false;
	}
	static constexpr void double_digits(decimal_work& w) {
		unsigned carry = 0;
		for (unsigned i = 0; i < work_digits; ++i) {
			const unsigned v = 2u * w[i] + carry;
			w[i] = static_cast<std::uint8_t>(v % 10u);
			carry = v / 10u;
		}
	}
	// w = floor(w / 2); returns the bit dropped
	static constexpr bool halve_digits(decimal_work& w) {
		unsigned rem = 0;
		for (unsigned i = work_digits; i-- > 0;) {
			const unsigned v = 10u * rem + w[i];
			w[i] = static_cast<std::uint8_t>(v / 2u);
			rem = v % 2u;
		}
		return rem != 0;
	}
	// the digits from position radix up, as an integer, when there are at most 19 of them
	static constexpr std::uint64_t integer_part(const decimal_work& w) {
		std::uint64_t v = 0;
		for (unsigned i = work_digits; i-- > radix;) v = 10u * v + w[i];
		return v;
	}
	static constexpr unsigned top_digit(const decimal_work& w) {      // the index of the highest nonzero digit
		for (unsigned i = work_digits; i-- > 0;) if (w[i] != 0) return i;
		return 0;
	}

	constexpr dfixpnt& saturated(bool negative) {
		clear();
		for (unsigned i = 0; i < ndigits; ++i) _block.setdigit(i, 9);
		_sign = negative;
		return *this;
	}

	// The value rounded to nearest, ties to even, into a binary format with p significand bits, normal
	// exponents emin .. emax and subnormals below 2^emin; returned as the double that holds it exactly, or
	// an infinity beyond the format's largest value. The magnitude N 10^-radix is scaled by a power of two
	// into an integer Q in [2^62, 2^63) plus a fraction, exactly, in decimal: N is doubled s times, or its
	// integer part halved; then Q is rounded to the format's grid with the fraction as sticky bits.
	template<unsigned p, int emin, int emax>
	constexpr double to_binary() const {
		static_assert(p >= 2u && p <= 53u && emin >= -1022 && emax <= 1023, "to_binary: a format a double holds");
		if (_block.iszero()) return 0.0;
		decimal_work w{};
		for (unsigned i = 0; i < ndigits; ++i) w[i] = static_cast<std::uint8_t>(_block.digit(i));
		constexpr std::uint64_t two62 = 1ull << 62;
		constexpr std::uint64_t two63 = 1ull << 63;
		int s = 0;                                          // the magnitude is (Q + fraction) * 2^-s
		bool sticky = false;
		if (top_digit(w) >= radix + 19u || integer_part(w) >= two63) {
			// at least 2^63: drop the fraction digits, then halve the integer part below 2^63
			for (unsigned i = 0; i < radix; ++i) {
				if (w[i] != 0) sticky = true;
				w[i] = 0;
			}
			while (top_digit(w) >= radix + 19u || integer_part(w) >= two63) {
				decimal_work q{};                           // halve the integer part, which starts at radix
				unsigned rem = 0;
				for (unsigned i = work_digits; i-- > radix;) {
					const unsigned v = 10u * rem + w[i];
					q[i] = static_cast<std::uint8_t>(v / 2u);
					rem = v % 2u;
				}
				if (rem != 0) sticky = true;
				w = q;
				--s;
			}
		}
		else {
			// below 2^63: double until the integer part reaches 2^62
			while (integer_part(w) < two62) {
				double_digits(w);
				++s;
			}
			for (unsigned i = 0; i < radix; ++i) if (w[i] != 0) { sticky = true; break; }
		}
		const std::uint64_t Q = integer_part(w);            // in [2^62, 2^63)
		const int E = 62 - s;                               // the exponent of the leading bit
		if (E > emax) return _sign ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
		const int lsb = ((E > emin) ? E : emin) - static_cast<int>(p) + 1;   // the weight of the format's last bit
		const int shift = lsb + s;                          // the bits of Q below that weight, at least 9
		std::uint64_t R = 0;
		if (shift <= 63) {
			R = Q >> shift;
			const bool round = ((Q >> (shift - 1)) & 1u) != 0;
			const bool rest = sticky || (Q & ((1ull << (shift - 1)) - 1u)) != 0;
			if (round && (rest || (R & 1u) != 0)) ++R;
		}
		// else Q 2^-s is below half of the smallest subnormal: it rounds to 0
		if (R == 0) return _sign ? -0.0 : 0.0;
		int top = lsb;                                      // the exponent of R's leading bit
		for (std::uint64_t r = R; r > 1u; r >>= 1) ++top;
		if (top > emax) return _sign ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
		const double v = scale2(static_cast<double>(R), lsb);   // R <= 2^p: exact
		return _sign ? -v : v;
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
