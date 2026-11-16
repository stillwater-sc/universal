#pragma once
// e8m0_impl.hpp: definition of the e8m0 exponent-only scale type for MX/OCP formats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// e8m0 is an 8-bit exponent-only type used as the shared scale factor in
// OCP Microscaling (MX) block floating-point formats.
//
// Properties:
// - No sign bit, no mantissa bits
// - 8-bit unsigned exponent with bias of 127
// - Value = 2^(encoding - 127)
// - Encoding 0xFF = NaN
// - Encoding 0 = 2^(-127) (smallest positive value)
// - Encoding 127 = 2^0 = 1.0
// - Encoding 254 = 2^127 (largest value)
// - All values are positive powers of 2

#include <string>
#include <iosfwd>       // std::ostream/std::istream in the stream-operator declarations (#1334)
#include <string>       // std::string, returned by the text layer's builders
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <universal/utility/bit_cast.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/integer_cast.hpp>
#include <universal/number/e8m0/e8m0_fwd.hpp>
#include <universal/number/e8m0/exceptions.hpp>

namespace sw { namespace universal {

class e8m0 {
public:
	static constexpr unsigned nbits = 8;
	static constexpr int bias = 127;

	e8m0() = default;

	constexpr e8m0(const e8m0&) = default;
	constexpr e8m0(e8m0&&) = default;

	constexpr e8m0& operator=(const e8m0&) = default;
	constexpr e8m0& operator=(e8m0&&) = default;

	// specific value constructor
	constexpr e8m0(const SpecificValue code) noexcept : _bits{} {
		switch (code) {
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			// e8m0 has no zero; use encoding for 1.0 (2^0)
			_bits = 127;
			break;
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:
			setnan();
			break;
		case SpecificValue::infpos:
		case SpecificValue::infneg:
			// e8m0 has no infinity; saturate to max
			maxpos();
			break;
		case SpecificValue::minneg:
		case SpecificValue::maxneg:
			// e8m0 has no negative values; use minpos
			minpos();
			break;
		}
	}

	// construct from native types
	// every source goes through a double, which holds float, int and unsigned exactly, so the
	// rounding to a power of two happens once (#2106: a double or int was first rounded to float)
	constexpr explicit e8m0(float iv) noexcept : _bits{} { from_double(static_cast<double>(iv)); }
	constexpr explicit e8m0(double iv) noexcept : _bits{} { from_double(iv); }
	constexpr e8m0(int iv) noexcept : _bits{} { from_double(static_cast<double>(iv)); }
	constexpr e8m0(unsigned iv) noexcept : _bits{} { from_double(static_cast<double>(iv)); }

	// assignment operators
	constexpr e8m0& operator=(float rhs) noexcept { from_double(static_cast<double>(rhs)); return *this; }
	constexpr e8m0& operator=(double rhs) noexcept { from_double(rhs); return *this; }
	constexpr e8m0& operator=(int rhs) noexcept { from_double(static_cast<double>(rhs)); return *this; }
	constexpr e8m0& operator=(unsigned rhs) noexcept { from_double(static_cast<double>(rhs)); return *this; }

	// conversion operators
	constexpr explicit operator float() const noexcept { return to_float(); }
	constexpr explicit operator double() const noexcept { return static_cast<double>(to_float()); }
	// the library's integer-cast rule (shared/integer_cast.hpp, #2104): truncate, saturate to
	// max(), NaN gives the integer NaR. It cast the float: (int)e8m0(maxpos) = (int)2^127 was UB
	// and came out INT_MIN (#2106).
	constexpr explicit operator int() const noexcept { return integer_from_native<int>(to_float()); }

	// prefix operators
	constexpr e8m0& operator++() noexcept {
		if (_bits < 254u) ++_bits;
		return *this;
	}
	constexpr e8m0 operator++(int) noexcept {
		e8m0 tmp(*this);
		operator++();
		return tmp;
	}
	constexpr e8m0& operator--() noexcept {
		if (_bits > 0u && _bits != 0xFFu) --_bits;
		return *this;
	}
	constexpr e8m0 operator--(int) noexcept {
		e8m0 tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear() noexcept { _bits = 0; }
	constexpr void setnan() noexcept { _bits = 0xFFu; }
	constexpr void setbits(unsigned value) noexcept { _bits = static_cast<uint8_t>(value & 0xFFu); }
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		if (i < 8) {
			uint8_t bit = static_cast<uint8_t>(1u << i);
			if (v) _bits |= bit;
			else _bits &= static_cast<uint8_t>(~bit);
		}
	}

	constexpr e8m0& minpos() noexcept { _bits = 0x00u; return *this; }
	constexpr e8m0& maxpos() noexcept { _bits = 0xFEu; return *this; }

	// selectors
	constexpr bool isnan() const noexcept { return _bits == 0xFFu; }
	constexpr bool iszero() const noexcept { return false; } // e8m0 cannot represent zero
	constexpr bool isone() const noexcept { return _bits == 127u; }
	constexpr bool sign() const noexcept { return false; } // always positive
	constexpr int scale() const noexcept { return static_cast<int>(_bits) - bias; }
	constexpr uint8_t bits() const noexcept { return _bits; }
	constexpr int exponent() const noexcept { return static_cast<int>(_bits) - bias; }

	constexpr bool test(unsigned bitIndex) const noexcept { return at(bitIndex); }
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex < 8) {
			return (_bits & (1u << bitIndex)) != 0;
		}
		return false;
	}
	constexpr uint8_t nibble(unsigned n) const noexcept {
		if (n < 2) {
			return static_cast<uint8_t>((_bits >> (n * 4u)) & 0x0Fu);
		}
		return 0;
	}

	// Convert to float: value = 2^(encoding - 127).
	// Constexpr-safe via direct IEEE 754 bit construction:
	//   encoding 1..254 -> normal float, exp_field = encoding (bias matches),
	//                      mantissa = 0
	//   encoding 0      -> 2^-127 = subnormal float, mantissa MSB only set
	//   encoding 255    -> NaN
	// At runtime, std::ldexp is faster; dispatched via std::is_constant_evaluated().
	constexpr float to_float() const noexcept {
		if (_bits == 0xFFu) return std::numeric_limits<float>::quiet_NaN();
		if (std::is_constant_evaluated()) {
			if (_bits == 0u) {
				// 2^-127 as a subnormal float: exp_field = 0, mantissa MSB set.
				return sw::bit_cast<float>(uint32_t(0x00400000u));
			}
			uint32_t bits_u = uint32_t(_bits) << 23;
			return sw::bit_cast<float>(bits_u);
		}
		else {
			return std::ldexp(1.0f, static_cast<int>(_bits) - bias);
		}
	}

	// Convert from a double: the nearest power of two in the log domain, round(log2 v) with
	// halves away from zero, the rule e8m0 has always used.
	//
	// The midpoint between 2^E and 2^(E+1) is 2^(E+1/2): v = (1 + f) * 2^E rounds up exactly
	// when 1 + f >= sqrt(2). sqrt(2) is irrational, so no input is a tie, and the comparison is
	// done on the fraction bits against ceil((sqrt(2) - 1) * 2^52) = 1865452045155277. This is
	// the same at run time and at constant evaluation (#2106). The run-time path used
	// std::round(std::log2(v)) on a float, whose rounding could land on the other side of the
	// midpoint, and the constant-evaluated path flushed every float subnormal to encoding 0,
	// while 2^-127 (encoding 0) itself is a float subnormal.
	// NaN gives NaN; +/-inf and anything above 2^127.5 give maxpos; zero, negative values and
	// anything below 2^-127 give encoding 0, the smallest value e8m0 has.
	constexpr void from_double(double v) noexcept {
		if (v != v) {
			setnan();
			return;
		}
		constexpr double dmax = std::numeric_limits<double>::max();
		if (v > dmax || v < -dmax) {
			maxpos();
			return;
		}
		if (v <= 0.0) {
			_bits = 0;
			return;
		}
		const uint64_t bits_u = sw::bit_cast<uint64_t>(v);
		const int rawExp = static_cast<int>((bits_u >> 52) & 0x7FFu);
		if (rawExp == 0) {          // a subnormal double: far below 2^-127
			_bits = 0;
			return;
		}
		const uint64_t rawFrac = bits_u & ((uint64_t(1) << 52) - 1u);
		constexpr uint64_t round_up_threshold = 1865452045155277ull;   // (sqrt(2) - 1) * 2^52, rounded up
		int biased = (rawExp - 1023) + bias + ((rawFrac >= round_up_threshold) ? 1 : 0);
		if (biased < 0) biased = 0;
		if (biased > 254) biased = 254;
		_bits = static_cast<uint8_t>(biased);
	}
	// kept for callers that name it: a float converts exactly to a double
	constexpr void from_float(float v) noexcept { from_double(static_cast<double>(v)); }

protected:
	uint8_t _bits;

private:
	friend constexpr bool operator==(e8m0 lhs, e8m0 rhs);
};

////////////////////////    functions   /////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// e8m0 - e8m0 binary logic operators

inline constexpr bool operator==(e8m0 lhs, e8m0 rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	return lhs._bits == rhs._bits;
}

inline constexpr bool operator!=(e8m0 lhs, e8m0 rhs) {
	return !operator==(lhs, rhs);
}

inline constexpr bool operator<(e8m0 lhs, e8m0 rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	return lhs.bits() < rhs.bits();
}

inline constexpr bool operator>(e8m0 lhs, e8m0 rhs) {
	return operator<(rhs, lhs);
}

inline constexpr bool operator<=(e8m0 lhs, e8m0 rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}

inline constexpr bool operator>=(e8m0 lhs, e8m0 rhs) {
	// Cannot just be `!operator<(lhs, rhs)`: operator< returns false for
	// any NaN operand, so negating it would yield true for NaN >= x.
	// Build from operator> and operator==, both of which return false on NaN.
	return operator>(lhs, rhs) || operator==(lhs, rhs);
}

}} // namespace sw::universal
