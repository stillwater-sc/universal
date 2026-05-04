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
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <universal/utility/bit_cast.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/shared/nan_encoding.hpp>
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
	constexpr explicit e8m0(float iv) noexcept : _bits{} { from_float(iv); }
	constexpr explicit e8m0(double iv) noexcept : _bits{} { from_float(static_cast<float>(iv)); }
	constexpr e8m0(int iv) noexcept : _bits{} { from_float(static_cast<float>(iv)); }
	constexpr e8m0(unsigned iv) noexcept : _bits{} { from_float(static_cast<float>(iv)); }

	// assignment operators
	constexpr e8m0& operator=(float rhs) noexcept { from_float(rhs); return *this; }
	constexpr e8m0& operator=(double rhs) noexcept { from_float(static_cast<float>(rhs)); return *this; }
	constexpr e8m0& operator=(int rhs) noexcept { from_float(static_cast<float>(rhs)); return *this; }
	constexpr e8m0& operator=(unsigned rhs) noexcept { from_float(static_cast<float>(rhs)); return *this; }

	// conversion operators
	constexpr explicit operator float() const noexcept { return to_float(); }
	constexpr explicit operator double() const noexcept { return static_cast<double>(to_float()); }
	constexpr explicit operator int() const noexcept { return static_cast<int>(to_float()); }

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

	// Convert from float: find the closest power of 2.
	//
	// The runtime implementation uses std::log2 + std::round, which is
	// not constexpr.  At constant evaluation, extract the IEEE 754 fields
	// directly via sw::bit_cast and emulate round-to-nearest-power-of-2:
	//
	//   v = (1 + frac/2^23) * 2^E for normal floats, where E = rawExp - 127
	//   round(log2(v)) is E + 1 iff log2(1 + frac/2^23) >= 0.5
	//                          iff (1 + frac/2^23) >= sqrt(2)
	//                          iff frac >= (sqrt(2) - 1) * 2^23
	//                          iff frac >= 3474676  (precomputed)
	//
	// Subnormal floats (rawExp == 0): e8m0 minpos is 2^-127, which sits in
	// the float subnormal range.  Full conversion of arbitrary subnormals
	// requires log2 of the fraction; in constexpr context we coarsely
	// clamp them all to encoding 0.  This is acceptable for the OCP
	// scaling use case (powers of 2 typically in [-126, 127]).  Runtime
	// retains exact log2-based behavior.
	constexpr void from_float(float v) noexcept {
		// NaN: only value not equal to itself.
		if (v != v) {
			setnan();
			return;
		}
		// Infinity (either sign): clamp to maxpos, matching the
		// SpecificValue::infpos / SpecificValue::infneg construction path.
		// Must be checked BEFORE the v <= 0 clamp below, otherwise -inf
		// would silently encode as 0 (smallest representable) -- pre-PR
		// latent bug.  std::isinf is not constexpr; numeric_limits<float>::max()
		// is, so bracket against +/-fmax.
		constexpr float fmax = std::numeric_limits<float>::max();
		if (v > fmax || v < -fmax) {
			maxpos();
			return;
		}
		// Negative or zero: e8m0 has no representation; clamp to encoding 0.
		if (v <= 0.0f) {
			_bits = 0;
			return;
		}
		if (std::is_constant_evaluated()) {
			// Constexpr path via IEEE 754 bit-extraction.
			uint32_t bits_u = sw::bit_cast<uint32_t>(v);
			uint32_t rawExp  = (bits_u >> 23) & 0xFFu;
			uint32_t rawFrac = bits_u & 0x7FFFFFu;
			if (rawExp == 0u) {
				// subnormal -- coarse approximation, see comment above.
				_bits = 0;
				return;
			}
			// (sqrt(2) - 1) * 2^23 ~= 3474676; round half away from zero.
			constexpr uint32_t round_up_threshold = 3474676u;
			unsigned biased = rawExp;
			if (rawFrac >= round_up_threshold) ++biased;
			if (biased > 254u) {
				_bits = 254u;
			}
			else {
				_bits = static_cast<uint8_t>(biased);
			}
		}
		else {
			// Runtime path: legacy std::log2 + std::round implementation.
			// Infinity (either sign) is already handled above via the
			// numeric_limits<float>::max() bracket -- no need to redo
			// std::isinf here.
			float log2v = std::log2(v);
			int exp_int = static_cast<int>(std::round(log2v));
			int biased = exp_int + bias;
			if (biased < 0) {
				_bits = 0;
			}
			else if (biased > 254) {
				_bits = 254u;
			}
			else {
				_bits = static_cast<uint8_t>(biased);
			}
		}
	}

protected:
	uint8_t _bits;

private:
	friend constexpr bool operator==(e8m0 lhs, e8m0 rhs);
};

////////////////////////    functions   /////////////////////////////////

/// stream operators

inline std::ostream& operator<<(std::ostream& ostr, e8m0 v) {
	if (v.isnan()) return ostr << "NaN";
	return ostr << float(v);
}

inline std::istream& operator>>(std::istream& istr, e8m0& v) {
	float f;
	istr >> f;
	v = e8m0(f);
	return istr;
}

////////////////// string operators

inline std::string to_binary(e8m0 v, bool = false) {
	std::stringstream ss;
	uint8_t bits = v.bits();
	ss << "0b";
	for (int j = 7; j >= 0; --j) {
		ss << ((bits & (1u << j)) ? '1' : '0');
		if (j == 4) ss << '.'; // visual separator at nibble boundary
	}
	return ss.str();
}

// native semantic representation: radix-2, delegates to to_binary
inline std::string to_native(e8m0 v, bool nibbleMarker = false) {
	return to_binary(v, nibbleMarker);
}

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
