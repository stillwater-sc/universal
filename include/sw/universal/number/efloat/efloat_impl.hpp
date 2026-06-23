#pragma once
// efloat_impl.hpp: implementation of an adaptive precision binary floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>

// supporting types and functions
#include <universal/native/ieee754.hpp>   // IEEE-754 decoders
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/utility/string_parse.hpp>   // scan_decimal_float
#include <universal/utility/decimal_to_binary.hpp>  // d2b convert (Phase B2a)

/*
The efloat arithmetic can be configured to:
- throw exceptions on invalid arguments and operations
- return a signaling NaN

Compile-time configuration flags are used to select the exception mode.

The exception types are defined, but you have the option to throw them
*/
#include <universal/number/efloat/exceptions.hpp>

namespace sw { namespace universal {

static inline constexpr int clz(uint32_t x) noexcept {
	if (x == 0) return 32;
	int n = 0;
	if (x <= 0x0000FFFF) { n += 16; x <<= 16; }
	if (x <= 0x00FFFFFF) { n += 8;  x <<= 8;  }
	if (x <= 0x0FFFFFFF) { n += 4;  x <<= 4;  }
	if (x <= 0x3FFFFFFF) { n += 2;  x <<= 2;  }
	if (x <= 0x7FFFFFFF) { n += 1;            }
	return n;
}

enum class FloatingPointState {
	Zero,
	Normal,
	SignalingNaN,             // let's use the US English spelling
	QuietNaN,
	Infinite
};

// efloat is an adaptive precision linear floating-point type
template<unsigned nlimbs = 1024>
class efloat {
public:
	static constexpr unsigned maxNrLimbs = nlimbs;

	// Partial-constexpr surface (issue #747): efloat carries a
	// std::vector<uint32_t> _limb member, so any non-empty digit storage
	// escapes constant evaluation under C++20's transient-allocation
	// rules.  The default ctor uses is_constant_evaluated() to keep two
	// parallel invariants: at constant evaluation _limb stays empty
	// (the canonical constexpr zero -- iszero() relies on _state == Zero,
	// not on _limb size); at runtime push_back(0) restores the historical
	// "_limb has one element of value 0" representation for callers that
	// inspect bits()/significant().  Sign-only and state-only selectors
	// and modifiers operate purely on the trivial members and are
	// constexpr-clean.  Compound arithmetic and free comparison operators
	// are currently stubs (returning *this or a constant) and are
	// therefore promoted to constexpr too -- the surface lights up at
	// constant evaluation today, and real arithmetic semantics will
	// inherit constexpr automatically when implemented.
	// Out of scope (heap-escape boundary): native-type ctors / operator=
	// (convert_ieee754 calls std::fpclassify which is not constexpr in
	// C++20), conversion-out (std::pow), parse() (std::regex).
	constexpr efloat() noexcept
		: _state{ FloatingPointState::Zero }, _sign{ false }, _exponent{ 0 }, _limb{} {
		if (!std::is_constant_evaluated()) {
			_limb.push_back(0);
		}
	}

	constexpr efloat(const efloat&) = default;
	constexpr efloat(efloat&&) = default;

	constexpr efloat& operator=(const efloat&) = default;
	constexpr efloat& operator=(efloat&&) = default;

	// specialized constructor for testing and verification
	constexpr efloat(bool sign, int64_t exponent, const std::vector<uint32_t>& limbs, bool nan = false, bool inf = false, bool zero = false) {
		_sign = sign;
		_exponent = exponent;
		_limb = limbs;
		if (nan) _state = FloatingPointState::QuietNaN;
		else if (inf) _state = FloatingPointState::Infinite;
		else if (zero) _state = FloatingPointState::Zero;
		else _state = FloatingPointState::Normal;
	}

	// initializers for native types
	efloat(signed char iv)                      noexcept { *this = iv; }
	efloat(short iv)                            noexcept { *this = iv; }
	efloat(int iv)                              noexcept { *this = iv; }
	efloat(long iv)                             noexcept { *this = iv; }
	efloat(long long iv)                        noexcept { *this = iv; }
	efloat(char iv)                             noexcept { *this = iv; }
	efloat(unsigned short iv)                   noexcept { *this = iv; }
	efloat(unsigned int iv)                     noexcept { *this = iv; }
	efloat(unsigned long iv)                    noexcept { *this = iv; }
	efloat(unsigned long long iv)               noexcept { *this = iv; }
	efloat(float iv)                            noexcept { *this = iv; }
	efloat(double iv)                           noexcept { *this = iv; }

	// assignment operators for native types
	efloat& operator=(signed char rhs)          noexcept { return convert_signed(rhs); }
	efloat& operator=(short rhs)                noexcept { return convert_signed(rhs); }
	efloat& operator=(int rhs)                  noexcept { return convert_signed(rhs); }
	efloat& operator=(long rhs)                 noexcept { return convert_signed(rhs); }
	efloat& operator=(long long rhs)            noexcept { return convert_signed(rhs); }
	efloat& operator=(char rhs)                 noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned short rhs)       noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned int rhs)         noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned long rhs)        noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned long long rhs)   noexcept { return convert_unsigned(rhs); }
	efloat& operator=(float rhs)                noexcept { return convert_ieee754(rhs); }
	efloat& operator=(double rhs)               noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float()             const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double()            const noexcept { return convert_to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	efloat(long double iv)                      noexcept { *this = iv; }
	efloat& operator=(long double rhs)          noexcept { return convert_ieee754(rhs); }
	explicit operator long double()       const noexcept { return convert_to_ieee754<long double>(); }
#endif 

	// prefix operators
	constexpr efloat operator-() const noexcept {
		if (iszero()) return *this;
		efloat negated(*this);
		negated.setsign(!_sign);
		return negated;
	}

	// arithmetic operators
	constexpr efloat& operator+=(const efloat& rhs) noexcept {
		// handle special cases
		if (isnan() || rhs.isnan()) {
			setnan();
			return *this;
		}
		if (isinf()) {
			if (rhs.isinf() && _sign != rhs._sign) setnan();
			return *this;
		}
		if (rhs.isinf()) {
			*this = rhs;
			return *this;
		}
		if (iszero()) {
			*this = rhs;
			return *this;
		}
		if (rhs.iszero()) {
			return *this;
		}

		// Make copies for manipulation
		std::vector<uint32_t> a_limbs = _limb;
		std::vector<uint32_t> b_limbs = rhs._limb;
		int64_t a_exp = _exponent;
		int64_t b_exp = rhs._exponent;
		
		// Align exponents
		if (a_exp < b_exp) {
			grow_for_shift(a_limbs, b_exp - a_exp, nlimbs);
			shift_right(a_limbs, b_exp - a_exp);
			a_exp = b_exp;
		} else if (b_exp < a_exp) {
			grow_for_shift(b_limbs, a_exp - b_exp, nlimbs);
			shift_right(b_limbs, a_exp - b_exp);
		}

		// Align sizes before any operation
		align_sizes(a_limbs, b_limbs);

		if (_sign == rhs._sign) {
			size_t old_size = a_limbs.size();
			add_limbs(a_limbs, b_limbs);
			if (a_limbs.size() > old_size) {
				a_exp += 32 * (a_limbs.size() - old_size);
			}
			_limb = a_limbs;
		} else {
			// Signs differ, perform subtraction
			int cmp = compare_limbs(a_limbs, b_limbs);
			if (cmp == 0) {
				setzero();
				return *this;
			}
			if (cmp > 0) {
				subtract_limbs(a_limbs, b_limbs);
				_limb = a_limbs;
				// sign is already correct
			} else { // cmp < 0
				subtract_limbs(b_limbs, a_limbs);
				_limb = b_limbs;
				_sign = rhs._sign;
			}
		}

		_exponent = a_exp;
		normalize();

		return *this;
	}
	constexpr efloat& operator+=(double rhs) noexcept {
		return *this += efloat(rhs);
	}
	constexpr efloat& operator-=(const efloat& rhs) noexcept {
		*this += -rhs;
		return *this;
	}
	constexpr efloat& operator-=(double rhs) noexcept {
		return *this -= efloat(rhs);
	}
	constexpr efloat& operator*=(const efloat& /* rhs */) noexcept {
		return *this;
	}
	constexpr efloat& operator*=(double /* rhs */) noexcept {
		return *this;
	}
	constexpr efloat& operator/=(const efloat& /* rhs */) noexcept {
		return *this;
	}
	constexpr efloat& operator/=(double /* rhs */) noexcept {
		return *this;
	}

	// modifiers
	constexpr void clear() noexcept { _state = FloatingPointState::Normal;  _sign = false; _exponent = 0; _limb.clear(); }
	constexpr void setzero() noexcept {
		clear();
		_state = FloatingPointState::Zero;
		// Match the default ctor's runtime representation (_limb = [0])
		// so isone(), bits(), significant() see identical state regardless
		// of how the zero was reached.  Empty _limb stays at constant
		// evaluation (heap-escape boundary).
		if (!std::is_constant_evaluated()) {
			_limb.push_back(0);
		}
	}
	constexpr void setinf(bool sign = false) noexcept {
		_state = FloatingPointState::Infinite;
		_sign = sign;
		_exponent = 0;
		_limb.clear();
	}
	constexpr void setnan(bool quiet = true) noexcept {
		_state = quiet ? FloatingPointState::QuietNaN : FloatingPointState::SignalingNaN;
		_sign = false;
		_exponent = 0;
		_limb.clear();
	}
	constexpr void setsign(bool sign) noexcept { _sign = sign; }
	constexpr void setexponent(std::int64_t e) noexcept { _exponent = e; }
	void setlimb(unsigned i, std::uint32_t value) {
		if (i >= _limb.size()) _limb.resize(i + 1, 0u);
		_limb[i] = value;
	}

	efloat& assign(const std::string& txt) {
		parse(txt, *this);
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept { return _state == FloatingPointState::Zero; }
	constexpr bool isone()  const noexcept { return (_state == FloatingPointState::Normal && !_sign && _exponent == 0 && _limb.size() == 1 && _limb[0] == 0x8000'000); }
	constexpr bool isodd()  const noexcept { return false; }
	constexpr bool iseven() const noexcept { return !isodd(); }
	constexpr bool ispos()  const noexcept { return (_state == FloatingPointState::Normal && !_sign); }
	constexpr bool isneg()  const noexcept { return (_state == FloatingPointState::Normal && _sign); }
	constexpr bool isinf()  const noexcept { return (_state == FloatingPointState::Infinite); }
	constexpr bool isnan()  const noexcept { return (_state == FloatingPointState::QuietNaN || _state == FloatingPointState::SignalingNaN); }
	constexpr bool isqnan()  const noexcept { return (_state == FloatingPointState::QuietNaN); }
	constexpr bool issnan()  const noexcept { return (_state == FloatingPointState::SignalingNaN); }


	// value information selectors
	constexpr int     sign()        const noexcept { return (_sign ? -1 : 1); }
	constexpr int64_t scale()       const noexcept { return _exponent; }
	double  significant() const noexcept {
		// efloat is a normalized floating-point, thus the significant falls in the range [1.0, 2.0)
		double v{ 0.0 };
		if (_state == FloatingPointState::Normal) {
			// build a 64-bit bit representation
			uint64_t raw{ 0 };
			switch (_limb.size()) {
			case 0:
				break;
			case 1:
				raw = _limb[0];
				raw <<= 32;
				break;
			case 2:
			default:
				raw = _limb[0];
				raw <<= 32;
				raw |= _limb[1];
				break;
			}
			raw &= 0x7FFF'FFFF'FFFF'FFFF; // remove hidden bit
			if (raw > 0) {
				v = double(raw)/ 9223372036854775808.0;
			}
			v += 1.0;
		}
		// else {
			// Zero, NaN or Infinity will return a significant value of 0.0
		// }

		return v;
	}
	std::vector<uint32_t> bits() const { return _limb; }

protected:
	FloatingPointState    _state;    // exceptional state
	bool                  _sign;     // sign of the number: -1 if true, +1 if false, zero is positive
	int64_t               _exponent; // exponent of the number
	std::vector<uint32_t> _limb;     // limbs of the representation

	// HELPER methods

	static constexpr void shift_right(std::vector<uint32_t>& limbs, unsigned k) noexcept {
		if (k == 0) return;
		if (k >= limbs.size() * 32) {
			limbs.assign(1, 0u);
			return;
		}
		const unsigned limb_shift = k / 32;
		const unsigned bit_shift = k % 32;
		
		if (limb_shift > 0) {
			// erase the LSBs that are shifted out
			limbs.erase(limbs.begin(), limbs.begin() + limb_shift);
			// insert zeros at the MSB side
			limbs.insert(limbs.end(), limb_shift, 0u);
		}

		if (bit_shift > 0) {
			uint32_t carry_mask = (1u << bit_shift) - 1;
			uint32_t carry = 0;
			for (int i = static_cast<int>(limbs.size()) - 1; i >= 0; --i) {
				uint32_t next_carry = limbs[i] & carry_mask;
				limbs[i] = (limbs[i] >> bit_shift) | (carry << (32 - bit_shift));
				carry = next_carry;
			}
		}
	}

	static constexpr void grow_for_shift(std::vector<uint32_t>& limbs, unsigned k, unsigned max_limbs) noexcept {
		const unsigned required_limbs = (k + 31) / 32;
		if (limbs.size() < max_limbs && required_limbs > 0) {
			unsigned growth = std::min(required_limbs, max_limbs - static_cast<unsigned>(limbs.size()));
			if (growth > 0) {
				limbs.insert(limbs.begin(), growth, 0u);
			}
		}
	}

	static constexpr void align_sizes(std::vector<uint32_t>& a, std::vector<uint32_t>& b) noexcept {
		size_t max_limbs = std::max(a.size(), b.size());
		if (a.size() < max_limbs) {
			size_t diff = max_limbs - a.size();
			a.insert(a.begin(), diff, 0u);
		}
		if (b.size() < max_limbs) {
			size_t diff = max_limbs - b.size();
			b.insert(b.begin(), diff, 0u);
		}
	}

	static constexpr void add_limbs(std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		uint64_t carry = 0;
		for (size_t i = 0; i < a.size(); ++i) {
			uint64_t sum = uint64_t(a[i]) + uint64_t(b[i]) + carry;
			a[i] = static_cast<uint32_t>(sum);
			carry = sum >> 32;
		}
		if (carry) {
			a.push_back(1);
		}
	}

	static constexpr void subtract_limbs(std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		uint64_t borrow = 0;
		for (size_t i = 0; i < a.size(); ++i) {
			uint64_t diff = (uint64_t(1) << 32) + uint64_t(a[i]) - uint64_t(b[i]) - borrow;
			a[i] = static_cast<uint32_t>(diff);
			borrow = (diff >> 32) ? 0 : 1;
		}
	}

	constexpr void normalize() {
		int msb_pos = -1;
		for (int i = _limb.size() - 1; i >= 0; --i) {
			if (_limb[i] != 0) {
				msb_pos = i * 32 + (31 - clz(_limb[i]));
				break;
			}
		}

		if (msb_pos == -1) {
			setzero();
			return;
		}

		int64_t shift = (int64_t)(_limb.size() * 32 - 1) - msb_pos;
		_exponent -= shift;

		if (shift > 0) {
			for(int64_t i = 0; i < shift; ++i) {
				uint64_t carry = 0;
				for(size_t j = 0; j < _limb.size(); ++j) {
					uint64_t v = (uint64_t(_limb[j]) << 1) | carry;
					_limb[j] = static_cast<uint32_t>(v);
					carry = v >> 32;
				}
			}
		} else if (shift < 0) {
			shift_right(_limb, static_cast<unsigned>(-shift));
		}

		// Truncate trailing zero limbs at the LSB side (index 0) to maintain minimal representation
		while (_limb.size() > 1 && _limb[0] == 0) {
			_limb.erase(_limb.begin());
		}
	}

	static constexpr int compare_limbs(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) noexcept {
		for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) {
			if (a[i] > b[i]) return 1;
			if (b[i] > a[i]) return -1;
		}
		return 0;
	}


	// convert arithmetic types into an elastic floating-point
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	efloat& convert_signed(SignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {

		}
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	efloat& convert_unsigned(UnsignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {

		}
		return *this;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	efloat& convert_ieee754(Real rhs) noexcept {
		clear();
		bool isSubnormal{ false };
		int nan_type{ NAN_TYPE_NEITHER };
		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_state = FloatingPointState::Zero;
			_sign = false;
			_exponent = 0;
			// stay limbless
			return *this;
		case FP_NAN:
			_sign = sw::universal::sign(rhs);
			// x86 specific: the top bit of the significand = 1 for quiet, 0 for signaling
			checkNaN(rhs, nan_type);
			if (nan_type == NAN_TYPE_QUIET) {
				_state = FloatingPointState::QuietNaN;
			}
			else {
				_state = FloatingPointState::SignalingNaN;
			}
			_exponent = 0;
			// stay limbless
			return *this;
		case FP_INFINITE:
			_state = FloatingPointState::Infinite;
			_sign = sw::universal::sign(rhs);
			_exponent = 0;
			// stay limbless
			return *this;
		case FP_SUBNORMAL:
			isSubnormal = true;
			break;
		case FP_NORMAL:
		default:
			break;
		}

		_sign = sw::universal::sign(rhs);
		_exponent = sw::universal::scale(rhs); // scale already deals with subnormal numbers
		if constexpr (sizeof(Real) == 4) {
			std::uint32_t bits{ 0 };
			if (isSubnormal) { // subnormal number
				bits = static_cast<std::uint32_t>(sw::universal::fractionBits(rhs));
				bits <<= 8; // 31 - 23 = 8 bits to get the hidden bit to land on bit 31
				std::uint32_t mask = 0x8000'0000;
				while ((mask & bits) == 0) {
					bits <<= 1;
				}
			}
			else {
				bits = static_cast<std::uint32_t>(sw::universal::significandBits(rhs));
				bits <<= 8; // 31 - 23 = 8 bits to get the hidden bit to land on bit 31
			}
			_limb.push_back(bits);
		}
		else if constexpr (sizeof(Real) == 8) {
			std::uint64_t bits{ 0 };
			if (isSubnormal) { // subnormal number
				bits = sw::universal::fractionBits(rhs);
				bits <<= 11; // 63 - 52 = 11 bits to get the hidden bit to land on bit 63
				std::uint64_t mask = 0x8000'0000'0000'0000;
				while ((mask & bits) == 0) {
					bits <<= 1;
				}
			}
			else {
				bits = sw::universal::significandBits(rhs);
				bits <<= 11; // 63 - 52 = 11 bits to get the hidden bit to land on bit 63
			}
			_limb.push_back(static_cast<uint32_t>(bits >> 32));
			_limb.push_back(static_cast<uint32_t>(bits & 0xFFFF'FFFF));
		}
		else {
			static_assert(true);
		}
		return *this;
	}


	// convert elastic floating-point to native ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	Real convert_to_ieee754() const noexcept {
		Real v{ 0.0 };
		switch (_state) {
		case FloatingPointState::Zero:
			break;
		case FloatingPointState::QuietNaN:
			v = std::numeric_limits<Real>::quiet_NaN();
			break;
		case FloatingPointState::SignalingNaN:
			v = std::numeric_limits<Real>::signaling_NaN();
			break;
		case FloatingPointState::Infinite:
			v = (_sign ? -std::numeric_limits<Real>::infinity() : +std::numeric_limits<Real>::infinity());
			break;
		case FloatingPointState::Normal:
			v = Real(sign()) * std::pow(Real(2.0), Real(scale())) * Real(significant());
		}
		return v;
	}

private:

	// efloat - efloat logic comparisons
	template<unsigned nnlimbs>
	friend constexpr bool operator==(const efloat<nnlimbs>& lhs, const efloat<nnlimbs>& rhs) noexcept;

	// efloat - literal logic comparisons
	template<unsigned nnlimbs>
	friend bool operator==(const efloat<nnlimbs>& lhs, long long rhs);

	// literal - efloat logic comparisons
	template<unsigned nnlimbs>
	friend bool operator==(long long lhs, const efloat<nnlimbs>& rhs);

	// find the most significant bit set
	template<unsigned nnlimbs>
	friend signed findMsb(const efloat<nnlimbs>& v);
	};

	// to_binary formatter for efloat to support test reporters
	template<unsigned nlimbs>
	inline std::string to_binary(const efloat<nlimbs>& number, bool nibbleMarker = false) {
		std::stringstream ss;
		if (number.isnan()) {
			ss << "nan";
		} else if (number.isinf()) {
			ss << (number.sign() == -1 ? "-inf" : "+inf");
		} else if (number.iszero()) {
			ss << "0b0.0.0";
		} else {
			ss << "0b" << (number.sign() == -1 ? "1" : "0") << "."
			   << number.scale() << ".";
			auto limbs = number.bits();
			for (int i = limbs.size() - 1; i >= 0; --i) {
				ss << std::setw(8) << std::setfill('0') << std::hex << limbs[i];
				if (i > 0) ss << "'";
			}
		}
		return ss.str();
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////    efloat functions   /////////////////////////////////

template<unsigned nlimbs>
inline efloat<nlimbs> abs(const efloat<nlimbs>& a) {
	return a; // (a < 0 ? -a : a);
}

////////////////////////////////////////////////////////////////////////////////
/// stream operators

// read an ASCII decimal literal and make a binary efloat out of it.
// Supports:
//   - "nan", "inf", "infinity" (case-insensitive, optional sign)
//   - decimal / scientific literals routed through decimal_to_binary::convert
//     ("1.5", "-3.14e2", "1e-100", "0x" is not accepted)
// The d2b target_mantissa_bits is sized to fill efloat's available limb
// storage, capped at 2040 bits (just under the d2b BigBits budget of 2048).
// For larger nlimbs the parsed value uses 2040 explicit bits of precision
// and lower limbs stay zero -- that is exact for any practical literal a
// user types (a decimal exponent within ~+/-600).
template<unsigned nlimbs>
bool parse(const std::string& txt, efloat<nlimbs>& value) {
	value.clear();

	std::string s = txt;
	// trim ASCII whitespace
	auto not_space = [](unsigned char c) { return !std::isspace(c); };
	auto first = std::find_if(s.begin(), s.end(), not_space);
	auto last  = std::find_if(s.rbegin(), s.rend(), not_space).base();
	s = (first < last) ? std::string(first, last) : std::string{};
	if (s.empty()) return false;

	// nan / inf / infinity tokens (case-insensitive, optional leading sign).
	{
		bool neg = false;
		std::size_t i = 0;
		if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
			neg = (s[i] == '-');
			++i;
		}
		std::string tok;
		tok.reserve(s.size() - i);
		for (std::size_t k = i; k < s.size(); ++k) {
			tok.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(s[k]))));
		}
		if (tok == "nan") {
			value.setnan(true);
			return true;
		}
		if (tok == "inf" || tok == "infinity") {
			value.setinf(neg);
			return true;
		}
	}

	// Decimal / scientific literal.
	constexpr unsigned cap_bits  = sw::universal::decimal_to_binary::default_big_bits - 8u;
	constexpr unsigned want_bits = static_cast<unsigned>(nlimbs) * 32u;
	constexpr unsigned target_bits = (want_bits == 0u) ? 1u
	                              : ((want_bits < cap_bits) ? want_bits : cap_bits);

	auto r = sw::universal::decimal_to_binary::convert(s, target_bits);
	if (!r.valid) return false;

	if (r.is_zero) {
		value.setzero();
		if (r.negative) value.setsign(true);
		return true;
	}

	// Round-to-nearest-even using d2b's guard/sticky.  If a carry escapes
	// the top of the mantissa (now occupies bit `target_bits`), shift right
	// by 1 and bump the binary scale.
	auto mantissa = r.mantissa;
	std::int64_t binary_scale = r.binary_scale;
	bool round_up = r.guard_bit && (r.sticky_bit || mantissa.at(0));
	if (round_up) {
		using Big = sw::universal::decimal_to_binary::big_integer<>;
		mantissa += Big(1);
		if (mantissa.at(target_bits)) {
			mantissa >>= 1;
			++binary_scale;
		}
	}

	value.setsign(r.negative);
	value.setexponent(binary_scale);

	// Pack the mantissa MSB-first into uint32 limbs: _limb[0] bit 31 = MSB,
	// _limb[0] bit 0 = mantissa bit (target_bits - 32), _limb[1] bit 31 =
	// mantissa bit (target_bits - 33), and so on.  Any leftover bits below
	// the last full chunk go into the top of the next limb.
	const unsigned full_limbs = target_bits / 32u;
	const unsigned leftover   = target_bits % 32u;

	for (unsigned i = 0; i < full_limbs; ++i) {
		std::uint32_t v = 0;
		const unsigned base = target_bits - 32u * (i + 1u);
		for (unsigned b = 0; b < 32u; ++b) {
			if (mantissa.at(base + b)) v |= (1u << b);
		}
		value.setlimb(i, v);
	}
	if (leftover > 0u) {
		std::uint32_t v = 0;
		for (unsigned b = 0; b < leftover; ++b) {
			// mantissa.at(b) (a low mantissa bit) goes into the (32 - leftover + b)
			// position of _limb[full_limbs] so the surviving bits stay packed
			// against the top of the limb (MSB-first per-limb convention).
			if (mantissa.at(b)) v |= (1u << (32u - leftover + b));
		}
		value.setlimb(full_limbs, v);
	}
	return true;
}

// generate an efloat format ASCII format
template<unsigned nlimbs>
inline std::ostream& operator<<(std::ostream& ostr, const efloat<nlimbs>& rhs) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the efloat into a string
	std::stringstream ss;

	if (rhs.isinf()) {
		ss << (rhs.sign() == -1 ? "-inf" : "+inf");
	}
	else if (rhs.isqnan()) {
		ss << "nan(qnan)";
	}
	else if (rhs.issnan()) {
		ss << "nan(snan)";
	}
	else {
		std::streamsize prec = ostr.precision();
		std::streamsize width = ostr.width();
		std::ios_base::fmtflags ff;
		ff = ostr.flags();
		ss.flags(ff);
		ss << std::setw(width) << std::setprecision(prec) << "TBD";
	}

	return ostr << ss.str();
}

// read an ASCII efloat format
template<unsigned nlimbs>
inline std::istream& operator>>(std::istream& istr, efloat<nlimbs>& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into an efloat value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

////////////////// string operators


//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - efloat binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<unsigned nlimbs>
constexpr bool operator==(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	// handle special cases
	if (lhs.isnan() || rhs.isnan()) {
		return false; // NaN != NaN per IEEE 754
	}
	if (lhs.isinf()) {
		return rhs.isinf() && (lhs.sign() == rhs.sign());
	}
	if (rhs.isinf()) {
		return false;
	}
	if (lhs.iszero()) {
		return rhs.iszero(); // positive zero == negative zero
	}
	if (rhs.iszero()) {
		return false;
	}

	// normal numbers
	if (lhs.sign() != rhs.sign()) return false;
	if (lhs.scale() != rhs.scale()) return false;
	if (lhs.bits().size() != rhs.bits().size()) return false;

	return efloat<nlimbs>::compare_limbs(lhs.bits(), rhs.bits()) == 0;
}
template<unsigned nlimbs>
constexpr bool operator!=(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
constexpr bool operator< (const efloat<nlimbs>& /* lhs */, const efloat<nlimbs>& /* rhs */) noexcept {
	return false; // lhs and rhs are the same
}
template<unsigned nlimbs>
constexpr bool operator> (const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
constexpr bool operator<=(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
constexpr bool operator>=(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nlimbs>
inline bool operator==(const efloat<nlimbs>& lhs, double rhs) {
	return operator==(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline bool operator!=(const efloat<nlimbs>& lhs, double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (const efloat<nlimbs>& lhs, double rhs) {
	return operator<(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline bool operator> (const efloat<nlimbs>& lhs, double rhs) {
	return operator< (efloat<nlimbs>(rhs), lhs);
}
template<unsigned nlimbs>
inline bool operator<=(const efloat<nlimbs>& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(const efloat<nlimbs>& lhs, double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned nlimbs>
inline bool operator==(double lhs, const efloat<nlimbs>& rhs) {
	return operator==(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline bool operator!=(double lhs, const efloat<nlimbs>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (double lhs, const efloat<nlimbs>& rhs) {
	return operator<(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline bool operator> (double lhs, const efloat<nlimbs>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
inline bool operator<=(double lhs, const efloat<nlimbs>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(double lhs, const efloat<nlimbs>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - efloat binary arithmetic operators
// (compose from constexpr-marked compound operators)
// BINARY ADDITION
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - literal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, double rhs) {
	return operator+(lhs, efloat<nlimbs>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, double rhs) {
	return operator-(lhs, efloat<nlimbs>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, double rhs) {
	return operator*(lhs, efloat<nlimbs>(rhs));
}
// BINARY DIVISION
template<unsigned nlimbs>
inline efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, double rhs) {
	return operator/(lhs, efloat<nlimbs>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline efloat<nlimbs> operator+(double lhs, const efloat<nlimbs>& rhs) {
	return operator+(efloat<nlimbs>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline efloat<nlimbs> operator-(double lhs, const efloat<nlimbs>& rhs) {
	return operator-(efloat<nlimbs>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline efloat<nlimbs> operator*(double lhs, const efloat<nlimbs>& rhs) {
	return operator*(efloat<nlimbs>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nlimbs>
inline efloat<nlimbs> operator/(double lhs, const efloat<nlimbs>& rhs) {
	return operator/(efloat<nlimbs>(lhs), rhs);
}

}} // namespace sw::universal
