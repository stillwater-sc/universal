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
#include <limits>

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
	explicit e8m0(float iv) noexcept : _bits{} { from_float(iv); }
	explicit e8m0(double iv) noexcept : _bits{} { from_float(static_cast<float>(iv)); }
	e8m0(int iv) noexcept : _bits{} { from_float(static_cast<float>(iv)); }
	e8m0(unsigned iv) noexcept : _bits{} { from_float(static_cast<float>(iv)); }

	// assignment operators
	e8m0& operator=(float rhs) noexcept { from_float(rhs); return *this; }
	e8m0& operator=(double rhs) noexcept { from_float(static_cast<float>(rhs)); return *this; }
	e8m0& operator=(int rhs) noexcept { from_float(static_cast<float>(rhs)); return *this; }
	e8m0& operator=(unsigned rhs) noexcept { from_float(static_cast<float>(rhs)); return *this; }

	// conversion operators
	explicit operator float() const noexcept { return to_float(); }
	explicit operator double() const noexcept { return static_cast<double>(to_float()); }
	explicit operator int() const noexcept { return static_cast<int>(to_float()); }

	// prefix operators
	e8m0& operator++() noexcept {
		if (_bits < 254u) ++_bits;
		return *this;
	}
	e8m0 operator++(int) noexcept {
		e8m0 tmp(*this);
		operator++();
		return tmp;
	}
	e8m0& operator--() noexcept {
		if (_bits > 0u && _bits != 0xFFu) --_bits;
		return *this;
	}
	e8m0 operator--(int) noexcept {
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

	// convert to float: value = 2^(encoding - 127)
	float to_float() const noexcept {
		if (_bits == 0xFFu) return std::numeric_limits<float>::quiet_NaN();
		return std::ldexp(1.0f, static_cast<int>(_bits) - bias);
	}

	// convert from float: find the closest power of 2
	void from_float(float v) noexcept {
		if (v != v) { // NaN
			setnan();
			return;
		}
		if (v <= 0.0f) {
			// e8m0 cannot represent zero or negative values
			// clamp to smallest representable value
			_bits = 0;
			return;
		}
		if (std::isinf(v)) {
			maxpos();
			return;
		}

		// Find the exponent: v ≈ 2^exp
		// Use log2 to find the closest power of 2
		float log2v = std::log2(v);
		int exp = static_cast<int>(std::round(log2v));
		int biased = exp + bias;

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

protected:
	uint8_t _bits;

private:
	friend bool operator==(e8m0 lhs, e8m0 rhs);
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

//////////////////////////////////////////////////////////////////////////////////////////////////////
// e8m0 - e8m0 binary logic operators

inline bool operator==(e8m0 lhs, e8m0 rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	return lhs._bits == rhs._bits;
}

inline bool operator!=(e8m0 lhs, e8m0 rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator<(e8m0 lhs, e8m0 rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	return lhs.bits() < rhs.bits();
}

inline bool operator>(e8m0 lhs, e8m0 rhs) {
	return operator<(rhs, lhs);
}

inline bool operator<=(e8m0 lhs, e8m0 rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(e8m0 lhs, e8m0 rhs) {
	return !operator<(lhs, rhs);
}

}} // namespace sw::universal
