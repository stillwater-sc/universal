#pragma once
// microfloat_impl.hpp: definition of the microfloat number system for MX/OCP element types
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
#include <cstring>
#include <cmath>
#include <limits>
#include <type_traits>

#include <universal/utility/bit_cast.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/microfloat/microfloat_fwd.hpp>
#include <universal/number/microfloat/exceptions.hpp>

namespace sw { namespace universal {

// microfloat: a lightweight floating-point type for MX/OCP block formats
// Template parameters:
//   _nbits       - total number of bits (4, 6, or 8)
//   _es          - number of exponent bits
//   _hasInf      - whether the type supports IEEE-like infinity
//   _hasNaN      - whether the type supports NaN encoding
//   _isSaturating - whether overflow saturates to maxpos/maxneg
template<unsigned _nbits, unsigned _es, bool _hasInf, bool _hasNaN, bool _isSaturating>
class microfloat {
	static_assert(_nbits <= 8, "microfloat is limited to 8 bits");
	static_assert(_es < _nbits, "exponent bits must be less than total bits");
	static_assert(_es >= 1, "need at least 1 exponent bit");

	// HELPER methods
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	constexpr microfloat& convert_signed(SignedInt v) noexcept {
		from_float(static_cast<float>(v));
		return *this;
	}
	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	constexpr microfloat& convert_unsigned(UnsignedInt v) noexcept {
		from_float(static_cast<float>(v));
		return *this;
	}
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	constexpr microfloat& convert_ieee754(Real rhs) noexcept {
		from_float(static_cast<float>(rhs));
		return *this;
	}

public:
	static constexpr unsigned nbits  = _nbits;
	static constexpr unsigned es     = _es;
	static constexpr unsigned fbits  = nbits - 1u - es;   // fraction bits (without hidden bit)
	static constexpr int      bias   = (1 << (es - 1)) - 1;
	static constexpr bool     hasInf = _hasInf;
	static constexpr bool     hasNaN = _hasNaN;
	static constexpr bool     isSaturating = _isSaturating;
	static constexpr uint8_t  bitmask = static_cast<uint8_t>((1u << nbits) - 1u);

	// derived constants
	static constexpr uint8_t  sign_mask = static_cast<uint8_t>(1u << (nbits - 1u));
	static constexpr uint8_t  exponent_mask = static_cast<uint8_t>(((1u << es) - 1u) << fbits);
	static constexpr uint8_t  fraction_mask = static_cast<uint8_t>((1u << fbits) - 1u);
	static constexpr unsigned max_exp_code = (1u << es) - 1u;

	microfloat() = default;

	constexpr microfloat(const microfloat&) = default;
	constexpr microfloat(microfloat&&) = default;

	constexpr microfloat& operator=(const microfloat&) = default;
	constexpr microfloat& operator=(microfloat&&) = default;

	// specific value constructor
	constexpr microfloat(const SpecificValue code) noexcept : _bits{} {
		switch (code) {
		case SpecificValue::infpos:
			setinf(false);
			break;
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			setzero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::infneg:
			setinf(true);
			break;
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::qnan:
		case SpecificValue::nar:
			setnan(NAN_TYPE_QUIET);
			break;
		case SpecificValue::snan:
			setnan(NAN_TYPE_SIGNALLING);
			break;
		}
	}

	// initializers for native types
	constexpr microfloat(signed char iv)                    noexcept : _bits{} { *this = iv; }
	constexpr microfloat(short iv)                          noexcept : _bits{} { *this = iv; }
	constexpr microfloat(int iv)                            noexcept : _bits{} { *this = iv; }
	constexpr microfloat(long iv)                           noexcept : _bits{} { *this = iv; }
	constexpr microfloat(long long iv)                      noexcept : _bits{} { *this = iv; }
	constexpr microfloat(char iv)                           noexcept : _bits{} { *this = iv; }
	constexpr microfloat(unsigned short iv)                 noexcept : _bits{} { *this = iv; }
	constexpr microfloat(unsigned int iv)                   noexcept : _bits{} { *this = iv; }
	constexpr microfloat(unsigned long iv)                  noexcept : _bits{} { *this = iv; }
	constexpr microfloat(unsigned long long iv)             noexcept : _bits{} { *this = iv; }
	constexpr explicit microfloat(float iv)                 noexcept : _bits{} { *this = iv; }
	constexpr explicit microfloat(double iv)                noexcept : _bits{} { *this = iv; }

	// assignment operators for native types
	constexpr microfloat& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr microfloat& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr microfloat& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr microfloat& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr microfloat& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr microfloat& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	constexpr microfloat& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr microfloat& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr microfloat& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr microfloat& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr microfloat& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr microfloat& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators
	constexpr explicit operator float()                       const noexcept { return to_float(); }
	constexpr explicit operator double()                      const noexcept { return static_cast<double>(to_float()); }
	constexpr explicit operator signed char()                 const noexcept { return static_cast<signed char>(to_float()); }
	constexpr explicit operator short()                       const noexcept { return static_cast<short>(to_float()); }
	constexpr explicit operator int()                         const noexcept { return static_cast<int>(to_float()); }
	constexpr explicit operator long()                        const noexcept { return static_cast<long>(to_float()); }
	constexpr explicit operator long long()                   const noexcept { return static_cast<long long>(to_float()); }
	constexpr explicit operator char()                        const noexcept { return static_cast<char>(to_float()); }
	constexpr explicit operator unsigned short()              const noexcept { return static_cast<unsigned short>(to_float()); }
	constexpr explicit operator unsigned int()                const noexcept { return static_cast<unsigned int>(to_float()); }
	constexpr explicit operator unsigned long()               const noexcept { return static_cast<unsigned long>(to_float()); }
	constexpr explicit operator unsigned long long()          const noexcept { return static_cast<unsigned long long>(to_float()); }

#if LONG_DOUBLE_SUPPORT
	constexpr explicit microfloat(long double iv)           noexcept : _bits{} { *this = iv; }
	constexpr microfloat& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }
	constexpr explicit operator long double()                 const noexcept { return static_cast<long double>(to_float()); }
#endif

	// prefix operators
	constexpr microfloat operator-() const noexcept {
		microfloat tmp;
		tmp.setbits(_bits ^ sign_mask);
		return tmp;
	}

	constexpr microfloat& operator++() noexcept {
		// increment to the next encoding
		if (_bits & sign_mask) {
			// negative
			uint8_t magnitude = _bits & static_cast<uint8_t>(~sign_mask);
			if (magnitude == 1u) {
				_bits = 0; // go to +0
			}
			else if (magnitude > 0u) {
				--_bits;
			}
			// if magnitude == 0 (negative zero), stay at zero
		}
		else {
			// positive: increment unless at max encoding
			uint8_t magnitude = _bits & static_cast<uint8_t>(~sign_mask);
			uint8_t maxMagnitude = static_cast<uint8_t>(bitmask >> 1);
			if (magnitude < maxMagnitude) {
				++_bits;
			}
		}
		return *this;
	}
	constexpr microfloat operator++(int) noexcept {
		microfloat tmp(*this);
		operator++();
		return tmp;
	}
	constexpr microfloat& operator--() noexcept {
		if (_bits & sign_mask) {
			// negative: increment magnitude
			uint8_t magnitude = _bits & static_cast<uint8_t>(~sign_mask);
			uint8_t maxMagnitude = static_cast<uint8_t>(bitmask >> 1);
			if (magnitude < maxMagnitude) {
				++_bits;
			}
		}
		else {
			// positive
			if (_bits == 0u) {
				_bits = sign_mask | 0x01u; // go to minneg
			}
			else {
				--_bits;
			}
		}
		return *this;
	}
	constexpr microfloat operator--(int) noexcept {
		microfloat tmp(*this);
		operator--();
		return tmp;
	}

	// arithmetic operators
	constexpr microfloat& operator+=(const microfloat& rhs) {
		float result = to_float() + rhs.to_float();
		from_float(result);
		return *this;
	}
	constexpr microfloat& operator-=(const microfloat& rhs) {
		float result = to_float() - rhs.to_float();
		from_float(result);
		return *this;
	}
	constexpr microfloat& operator*=(const microfloat& rhs) {
		float result = to_float() * rhs.to_float();
		from_float(result);
		return *this;
	}
	constexpr microfloat& operator/=(const microfloat& rhs) {
		float result = to_float() / rhs.to_float();
		from_float(result);
		return *this;
	}

	// modifiers
	constexpr void clear() noexcept { _bits = 0; }
	constexpr void setzero() noexcept { clear(); }

	constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING) noexcept {
		if constexpr (hasNaN) {
			if constexpr (nbits == 8 && es == 4) {
				// e4m3: NaN encodings are 0x7F (positive) and 0xFF (negative)
				// S.1111.111 pattern
				_bits = (NaNType == NAN_TYPE_SIGNALLING) ? 0xFFu : 0x7Fu;
			}
			else {
				// e5m2 (IEEE-like): all-ones exponent with non-zero fraction
				// quiet NaN: fraction MSB = 1, signaling NaN: fraction MSB = 0 with non-zero fraction
				if (NaNType == NAN_TYPE_SIGNALLING) {
					_bits = static_cast<uint8_t>(sign_mask | exponent_mask | 0x01u);
				}
				else {
					_bits = static_cast<uint8_t>(exponent_mask | (1u << (fbits - 1u)));
				}
			}
		}
		else {
			// no NaN support: set to zero
			_bits = 0;
		}
		_bits &= bitmask;
	}

	constexpr void setinf(bool sign = false) noexcept {
		if constexpr (hasInf) {
			// e5m2 (IEEE-like): all-ones exponent, zero fraction
			_bits = exponent_mask;
			if (sign) _bits |= sign_mask;
			_bits &= bitmask;
		}
		else if constexpr (isSaturating) {
			// saturate to maxpos/maxneg
			if (sign) maxneg(); else maxpos();
		}
		else {
			_bits = 0;
		}
	}

	constexpr void setbit(unsigned i, bool v = true) noexcept {
		if (i < nbits) {
			uint8_t bit = static_cast<uint8_t>(1u << i);
			if (v) {
				_bits |= bit;
			}
			else {
				_bits &= static_cast<uint8_t>(~bit);
			}
			_bits &= bitmask;
		}
	}
	constexpr void setbits(unsigned value) noexcept { _bits = static_cast<uint8_t>(value & bitmask); }

	constexpr microfloat& minpos() noexcept { _bits = 0x01u; return *this; }

	constexpr microfloat& maxpos() noexcept {
		if constexpr (hasNaN && hasInf) {
			// e5m2: all-ones exponent is Inf/NaN, so max normal = exponent_mask - 1 step
			// max = S.11110.11 = 0x7B
			_bits = static_cast<uint8_t>(exponent_mask - (1u << fbits) + fraction_mask + (1u << fbits));
			// Actually: max normal for e5m2: exp=11110, frac=11 -> 0b0.11110.11 = 0x7B
			_bits = static_cast<uint8_t>((max_exp_code - 1u) << fbits | fraction_mask);
		}
		else if constexpr (hasNaN && !hasInf) {
			// e4m3: NaN is all-ones exp + all-ones fraction, max = all-ones exp + (fraction_mask - 1)
			// max = 0b0.1111.110 = 0x7E
			_bits = static_cast<uint8_t>(exponent_mask | (fraction_mask - 1u));
		}
		else {
			// No NaN, no Inf: all encodings are valid numbers
			// max = 0.111...1 (all bits except sign set)
			_bits = static_cast<uint8_t>(bitmask >> 1);
		}
		return *this;
	}

	constexpr microfloat& zero() noexcept { _bits = 0x00u; return *this; }

	constexpr microfloat& minneg() noexcept { _bits = static_cast<uint8_t>(sign_mask | 0x01u); return *this; }

	constexpr microfloat& maxneg() noexcept {
		maxpos();
		_bits |= sign_mask;
		_bits &= bitmask;
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept {
		return (_bits == 0x00u) || (_bits == sign_mask);
	}
	constexpr bool isone() const noexcept {
		// 1.0 = sign=0, exponent=bias, fraction=0
		uint8_t one_encoding = static_cast<uint8_t>(static_cast<unsigned>(bias) << fbits);
		return _bits == one_encoding;
	}
	constexpr bool isodd() const noexcept { return (_bits & 0x01u); }
	constexpr bool iseven() const noexcept { return !isodd(); }
	constexpr bool ispos() const noexcept { return !isneg(); }
	constexpr bool isneg() const noexcept { return (_bits & sign_mask) != 0; }

	constexpr bool isnan(int NaNType = NAN_TYPE_EITHER) const noexcept {
		if constexpr (!hasNaN) return false;

		if constexpr (nbits == 8 && es == 4 && !hasInf && hasNaN) {
			// e4m3: NaN is S.1111.111 -> encodings 0x7F and 0xFF
			bool isNaN = (_bits & 0x7Fu) == 0x7Fu;
			if (NaNType == NAN_TYPE_EITHER) return isNaN;
			if (NaNType == NAN_TYPE_SIGNALLING) return isNaN && ((_bits & sign_mask) != 0);
			if (NaNType == NAN_TYPE_QUIET) return isNaN && ((_bits & sign_mask) == 0);
			return false;
		}
		else {
			// IEEE-like (e5m2): NaN = all-ones exponent + non-zero fraction
			uint8_t exp = (_bits & exponent_mask);
			uint8_t frac = (_bits & fraction_mask);
			bool isNaN = (exp == exponent_mask) && (frac != 0);
			if (NaNType == NAN_TYPE_EITHER) return isNaN;
			bool isQuietNaN = isNaN && ((frac & (1u << (fbits - 1u))) != 0);
			bool isSignalNaN = isNaN && ((frac & (1u << (fbits - 1u))) == 0);
			if (NaNType == NAN_TYPE_QUIET) return isQuietNaN;
			if (NaNType == NAN_TYPE_SIGNALLING) return isSignalNaN;
			return false;
		}
	}

	constexpr bool isinf(int InfType = INF_TYPE_EITHER) const noexcept {
		if constexpr (!hasInf) return false;

		// IEEE-like: all-ones exponent + zero fraction
		uint8_t exp = (_bits & exponent_mask);
		uint8_t frac = (_bits & fraction_mask);
		bool inf = (exp == exponent_mask) && (frac == 0);
		if (!inf) return false;
		bool negative = isneg();
		if (InfType == INF_TYPE_EITHER) return true;
		if (InfType == INF_TYPE_NEGATIVE) return negative;
		if (InfType == INF_TYPE_POSITIVE) return !negative;
		return false;
	}

	constexpr bool   sign()   const noexcept { return isneg(); }
	constexpr int    scale()  const noexcept {
		int e = static_cast<int>((_bits & exponent_mask) >> fbits);
		return e - bias;
	}
	constexpr uint8_t bits()  const noexcept { return _bits; }

	constexpr bool test(unsigned bitIndex) const noexcept { return at(bitIndex); }
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex < nbits) {
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
	constexpr uint8_t exponent() const noexcept {
		return static_cast<uint8_t>((_bits & exponent_mask) >> fbits);
	}
	constexpr uint8_t fraction() const noexcept {
		return static_cast<uint8_t>(_bits & fraction_mask);
	}

	// Constexpr-safe ldexp emulation: x * 2^exp via power-of-2 multiplication.
	// For microfloat the |exp| range is small (at most |bias - 1|, which for
	// e5m2 is 14, for e4m3 is 6), so the linear loop is well-bounded.
	// Used only on the constexpr code path; runtime keeps std::ldexp.
	static constexpr float cx_ldexp(float x, int exp) noexcept {
		if (exp >= 0) {
			for (int i = 0; i < exp; ++i) x *= 2.0f;
		}
		else {
			for (int i = 0; i < -exp; ++i) x *= 0.5f;
		}
		return x;
	}

	// Convert to float.
	// Constexpr-safe via std::is_constant_evaluated() dispatch: at runtime
	// we use std::ldexp (fast, exact); at constant evaluation we substitute
	// cx_ldexp (a power-of-2 loop).  Both produce bit-identical results
	// because the 2.0/0.5 multiplications are exactly representable in
	// IEEE 754 float.
	constexpr float to_float() const noexcept {
		if (iszero()) return isneg() ? -0.0f : 0.0f;
		if constexpr (hasNaN) {
			if (isnan()) return std::numeric_limits<float>::quiet_NaN();
		}
		if constexpr (hasInf) {
			if (isinf()) {
				return isneg() ? -std::numeric_limits<float>::infinity()
				               :  std::numeric_limits<float>::infinity();
			}
		}

		bool     s = isneg();
		unsigned e = exponent();
		unsigned f = fraction();

		float value;
		if (e == 0) {
			// subnormal: value = (-1)^s * 2^(1-bias) * (0.fraction)
			float frac = static_cast<float>(f) / static_cast<float>(1u << fbits);
			if (std::is_constant_evaluated()) {
				value = cx_ldexp(frac, 1 - bias);
			}
			else {
				value = std::ldexp(frac, 1 - bias);
			}
		}
		else {
			// normal: value = (-1)^s * 2^(e-bias) * (1.fraction)
			float frac = 1.0f + static_cast<float>(f) / static_cast<float>(1u << fbits);
			if (std::is_constant_evaluated()) {
				value = cx_ldexp(frac, static_cast<int>(e) - bias);
			}
			else {
				value = std::ldexp(frac, static_cast<int>(e) - bias);
			}
		}
		return s ? -value : value;
	}

	// Constexpr-safe extraction of an IEEE 754 float's sign / biased
	// exponent / fraction fields via sw::bit_cast.  Used only on the
	// constexpr code path; runtime uses std::signbit / std::frexp etc.
	struct float_fields { bool sign; int rawExp; uint32_t rawFrac; };
	static_assert(sizeof(float) == sizeof(uint32_t) && std::numeric_limits<float>::is_iec559,
		"microfloat constexpr conversion requires IEEE 754 binary32 float layout "
		"(1 sign / 8 exp / 23 fraction bits).  On platforms where float is not "
		"32-bit IEC 559 the constexpr path's bit-field positions would be wrong; "
		"the runtime path (std::frexp / std::signbit) remains portable.");
	static constexpr float_fields extract_float_fields(float v) noexcept {
		uint32_t bits_u = sw::bit_cast<uint32_t>(v);
		return float_fields{
			(bits_u >> 31) != 0u,
			static_cast<int>((bits_u >> 23) & 0xFFu),
			bits_u & 0x7FFFFFu
		};
	}

	// Convert from float with RNE rounding.
	// Constexpr-safe via std::is_constant_evaluated() dispatch:
	//   * at runtime  -- std::signbit / std::isinf / std::frexp / std::ldexp
	//   * at constexpr -- IEEE 754 field extraction via sw::bit_cast,
	//                     plus cx_ldexp for the subnormal divisor
	// Both paths converge on the same RNE-rounded encoding.
	constexpr void from_float(float v) noexcept {
		if (v != v) { // NaN check
			if constexpr (hasNaN) {
				setnan(NAN_TYPE_QUIET);
			}
			else {
				setzero();
			}
			return;
		}

		bool s;
		bool is_inf;
		if (std::is_constant_evaluated()) {
			float_fields ff = extract_float_fields(v);
			s = ff.sign;
			is_inf = (ff.rawExp == 0xFF) && (ff.rawFrac == 0u);
		}
		else {
			s = std::signbit(v);
			is_inf = std::isinf(v);
		}
		if (s) v = -v;

		if (is_inf) {
			if constexpr (hasInf) {
				setinf(s);
			}
			else if constexpr (isSaturating) {
				if (s) maxneg(); else maxpos();
			}
			else {
				setzero();
			}
			return;
		}

		if (v == 0.0f) {
			// Preserve the sign bit for signed zero -- microfloat models
			// +0 / -0 distinctly (iszero() accepts both encodings, unary -
			// flips the sign on zero, to_float() emits the matching float
			// sign).  Pre-fix code unconditionally collapsed -0.0f to +0,
			// breaking the round-trip contract.  CodeRabbit catch on PR #811.
			_bits = s ? sign_mask : 0x00u;
			return;
		}

		// Compute the maxpos value for clamping
		microfloat mp;
		mp.maxpos();
		float maxval = mp.to_float();

		if (v >= maxval) {
			// Check if we need to round to maxpos or to inf
			if constexpr (hasInf) {
				// Compute the tie-point between maxpos and inf
				// For IEEE-like types, values > maxval round to inf or stay at maxval
				// The tie point is maxval + 0.5 ULP above maxval
				// For simplicity with these small types: if v > maxval, go to inf
				if (v > maxval) {
					setinf(s);
					return;
				}
			}
			if constexpr (isSaturating) {
				if (s) maxneg(); else maxpos();
				return;
			}
			// non-saturating without inf: clamp to max
			if (s) maxneg(); else maxpos();
			return;
		}

		// Extract exponent and fraction from the float value.
		// frexp returns frac in [0.5, 1.0), exp such that v = frac * 2^exp.
		// At constant evaluation we substitute IEEE 754 bit-extraction,
		// which yields the same (frac, exp) for normal floats:
		//   frac    = (1 + rawFrac/2^23) / 2     in [0.5, 1)
		//   exp     = rawExp - 127 + 1            (frexp adds 1 to make
		//                                          frac < 1)
		// For subnormal floats the bit-extraction would need a leading-zero
		// scan; subnormals here are within microfloat's subnormal range only
		// when the source float is itself subnormal -- a rare path.  We
		// route the constexpr subnormal-float case through the existing
		// ldexp-based subnormal branch by computing exp via the raw
		// exponent and frac via shifted mantissa, exactly as the runtime
		// std::frexp does.
		int exp;
		float frac;
		if (std::is_constant_evaluated()) {
			float_fields ff = extract_float_fields(v);
			if (ff.rawExp == 0) {
				// Source is a subnormal float.  Reconstruct via cx_ldexp
				// to match std::frexp semantics: find the leading 1 of
				// rawFrac, shift to align as 0.5, set exp accordingly.
				// rawFrac > 0 here (zero already handled).
				int leading = 22;  // top fraction bit index
				while (leading >= 0 && ((ff.rawFrac >> leading) & 1u) == 0u) --leading;
				int shift = 22 - leading;
				uint32_t aligned = ff.rawFrac << shift;
				// aligned now has its top bit at position 22, representing
				// the implicit leading 1 of a frexp-style mantissa.
				// frac = aligned / 2^23 + 0.5; equivalently aligned has
				// bit 22 set, lower 22 bits are the fractional part.
				// Construct frac directly via float bit pattern: exp_field=126
				// (which gives 2^-1 = 0.5), mantissa = aligned bits 21..0
				// shifted into the 23-bit field.
				uint32_t frac_bits = (uint32_t(126) << 23) | (aligned & 0x7FFFFFu);
				frac = sw::bit_cast<float>(frac_bits);
				exp = -126 - shift + 1;  // frexp's exp for subnormal source
			}
			else {
				// Normal float: frexp(v) = ((1 + rawFrac/2^23) / 2, rawExp - 127 + 1)
				// Construct frac with exp_field=126 (-> 2^-1 = 0.5 base),
				// mantissa = rawFrac.  This equals (1 + rawFrac/2^23)/2
				// bit-exactly.
				uint32_t frac_bits = (uint32_t(126) << 23) | ff.rawFrac;
				frac = sw::bit_cast<float>(frac_bits);
				exp = ff.rawExp - 127 + 1;
			}
		}
		else {
			frac = std::frexp(v, &exp);
		}
		// We want: v = 1.mantissa * 2^(exp-1)
		// so our biased_exp = exp - 1 + bias
		exp -= 1; // now v = (2*frac) * 2^exp, and 2*frac in [1.0, 2.0)
		float significand = 2.0f * frac; // in [1.0, 2.0)

		int biased_exp = exp + bias;

		if (biased_exp <= 0) {
			// Subnormal range
			// subnormal: v = f * 2^(1-bias) where f = 0.mantissa in [0, 1)
			float subnormal_divisor;
			if (std::is_constant_evaluated()) {
				subnormal_divisor = cx_ldexp(1.0f, 1 - bias);
			}
			else {
				subnormal_divisor = std::ldexp(1.0f, 1 - bias);
			}
			float subnormal_frac = v / subnormal_divisor;
			// subnormal_frac is in [0, 1)
			// Quantize to fbits bits with RNE
			float scaled = subnormal_frac * static_cast<float>(1u << fbits);
			unsigned f_int = rne_round(scaled);
			if (f_int >= (1u << fbits)) {
				// Rounded up to smallest normal
				biased_exp = 1;
				f_int = 0;
				_bits = static_cast<uint8_t>((static_cast<unsigned>(biased_exp) << fbits) | f_int);
			}
			else {
				_bits = static_cast<uint8_t>(f_int);
			}
		}
		else {
			// Normal range
			// significand is in [1.0, 2.0), we need the fractional part
			float mantissa = significand - 1.0f; // in [0, 1)
			float scaled = mantissa * static_cast<float>(1u << fbits);
			unsigned f_int = rne_round(scaled);
			if (f_int >= (1u << fbits)) {
				// Carry into exponent
				f_int = 0;
				biased_exp += 1;
			}
			// Check for overflow after rounding
			if constexpr (hasNaN && hasInf) {
				// e5m2: max biased exp for normal = max_exp_code - 1
				if (static_cast<unsigned>(biased_exp) >= max_exp_code) {
					setinf(s);
					return;
				}
			}
			else if constexpr (hasNaN && !hasInf) {
				// e4m3: max biased exp = max_exp_code, but all-ones exp + all-ones frac = NaN
				if (static_cast<unsigned>(biased_exp) > max_exp_code) {
					if (s) maxneg(); else maxpos();
					return;
				}
				if (static_cast<unsigned>(biased_exp) == max_exp_code && f_int >= fraction_mask) {
					if (s) maxneg(); else maxpos();
					return;
				}
			}
			else {
				// No NaN, no Inf: all encodings valid
				if (static_cast<unsigned>(biased_exp) > max_exp_code) {
					if (s) maxneg(); else maxpos();
					return;
				}
			}
			_bits = static_cast<uint8_t>((static_cast<unsigned>(biased_exp) << fbits) | f_int);
		}

		if (s) _bits |= sign_mask;
		_bits &= bitmask;
	}

protected:
	uint8_t _bits;

private:
	// Round-to-nearest-even helper
	static constexpr unsigned rne_round(float v) noexcept {
		unsigned truncated = static_cast<unsigned>(v);
		float remainder = v - static_cast<float>(truncated);
		if (remainder > 0.5f) return truncated + 1u;
		if (remainder < 0.5f) return truncated;
		// Exactly 0.5: round to even
		return (truncated & 1u) ? truncated + 1u : truncated;
	}

	// microfloat - microfloat logic comparisons
	template<unsigned n, unsigned e, bool i, bool na, bool s>
	friend constexpr bool operator==(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs);

	// microfloat - literal logic comparisons
	template<unsigned n, unsigned e, bool i, bool na, bool s>
	friend constexpr bool operator==(microfloat<n,e,i,na,s> lhs, float rhs);

	// literal - microfloat logic comparisons
	template<unsigned n, unsigned e, bool i, bool na, bool s>
	friend constexpr bool operator==(float lhs, microfloat<n,e,i,na,s> rhs);
};

////////////////////////    functions   /////////////////////////////////

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> abs(microfloat<n,e,i,na,s> a) {
	return (a.isneg() ? -a : a);
}

/// stream operators

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline std::ostream& operator<<(std::ostream& ostr, microfloat<n,e,i,na,s> mf) {
	return ostr << float(mf);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline std::istream& operator>>(std::istream& istr, microfloat<n,e,i,na,s>& p) {
	float f;
	istr >> f;
	p = microfloat<n,e,i,na,s>(f);
	return istr;
}

////////////////// string operators

template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
inline std::string to_binary(microfloat<nbits, es, hasInf, hasNaN, isSaturating> mf, bool bNibbleMarker = false) {
	constexpr unsigned fbits = nbits - 1u - es;
	std::stringstream ss;
	uint8_t bits = mf.bits();
	uint8_t mask = static_cast<uint8_t>(1u << (nbits - 1u));

	ss << (bits & mask ? "0b1." : "0b0.");
	mask >>= 1;
	// exponent bits
	for (unsigned j = 0; j < es; ++j) {
		if (bNibbleMarker && j > 0 && (j % 4) == 0) ss << '\'';
		ss << ((bits & mask) ? '1' : '0');
		mask >>= 1;
	}
	ss << '.';
	// fraction bits
	for (unsigned j = 0; j < fbits; ++j) {
		if (bNibbleMarker && j > 0 && (j % 4) == 0) ss << '\'';
		ss << ((bits & mask) ? '1' : '0');
		mask >>= 1;
	}
	return ss.str();
}

// native semantic representation: radix-2, delegates to to_binary
template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
inline std::string to_native(microfloat<nbits, es, hasInf, hasNaN, isSaturating> mf, bool nibbleMarker = false) {
	return to_binary(mf, nibbleMarker);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// microfloat - microfloat binary logic operators

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator==(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	// +0 == -0
	if (lhs.iszero() && rhs.iszero()) return true;
	return lhs._bits == rhs._bits;
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator!=(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator<(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	return (float(lhs) - float(rhs)) < 0;
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator>(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	return operator<(rhs, lhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator<=(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator>=(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	// Cannot just be `!operator<`: operator< returns false for any NaN
	// operand, so negating it would yield true for NaN >= x.  Build from
	// operator> and operator==, both of which return false on NaN.
	return operator>(lhs, rhs) || operator==(lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// microfloat - literal binary logic operators
//
// All overloads short-circuit a float NaN operand BEFORE narrowing to
// microfloat.  For instantiations with hasNaN == false, from_float(NaN)
// collapses to zero, which would erase the IEEE 754 NaN-comparison
// contract (the "all relational ops false / != true" rule) -- mf == NaN
// would silently become mf == 0.  CodeRabbit catch on PR #811.

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator==(microfloat<n,e,i,na,s> lhs, float rhs) {
	if (rhs != rhs) return false;  // float NaN: == always false
	return operator==(lhs, microfloat<n,e,i,na,s>(rhs));
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator!=(microfloat<n,e,i,na,s> lhs, float rhs) {
	if (rhs != rhs) return true;   // float NaN: != always true
	return !operator==(lhs, microfloat<n,e,i,na,s>(rhs));
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator<(microfloat<n,e,i,na,s> lhs, float rhs) {
	if (rhs != rhs) return false;  // float NaN: ordering always false
	return operator<(lhs, microfloat<n,e,i,na,s>(rhs));
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator>(microfloat<n,e,i,na,s> lhs, float rhs) {
	if (rhs != rhs) return false;
	return operator<(microfloat<n,e,i,na,s>(rhs), lhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator<=(microfloat<n,e,i,na,s> lhs, float rhs) {
	if (rhs != rhs) return false;
	return operator<(lhs, microfloat<n,e,i,na,s>(rhs)) || operator==(lhs, microfloat<n,e,i,na,s>(rhs));
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator>=(microfloat<n,e,i,na,s> lhs, float rhs) {
	if (rhs != rhs) return false;
	// NaN-safe via operator> + operator== (see microfloat-microfloat
	// operator>= for the !operator< trap rationale).
	return operator>(lhs, microfloat<n,e,i,na,s>(rhs)) || operator==(lhs, microfloat<n,e,i,na,s>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - microfloat binary logic operators (symmetric NaN-safety)

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator==(float lhs, microfloat<n,e,i,na,s> rhs) {
	if (lhs != lhs) return false;
	return operator==(microfloat<n,e,i,na,s>(lhs), rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator!=(float lhs, microfloat<n,e,i,na,s> rhs) {
	if (lhs != lhs) return true;
	return !operator==(microfloat<n,e,i,na,s>(lhs), rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator<(float lhs, microfloat<n,e,i,na,s> rhs) {
	if (lhs != lhs) return false;
	return operator<(microfloat<n,e,i,na,s>(lhs), rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator>(float lhs, microfloat<n,e,i,na,s> rhs) {
	if (lhs != lhs) return false;
	return operator<(rhs, microfloat<n,e,i,na,s>(lhs));
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator<=(float lhs, microfloat<n,e,i,na,s> rhs) {
	if (lhs != lhs) return false;
	return operator<(microfloat<n,e,i,na,s>(lhs), rhs) || operator==(microfloat<n,e,i,na,s>(lhs), rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr bool operator>=(float lhs, microfloat<n,e,i,na,s> rhs) {
	if (lhs != lhs) return false;
	// NaN-safe (see microfloat-microfloat operator>= for the rationale).
	return operator>(microfloat<n,e,i,na,s>(lhs), rhs) || operator==(microfloat<n,e,i,na,s>(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// microfloat - microfloat binary arithmetic operators

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator+(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	microfloat<n,e,i,na,s> sum = lhs;
	sum += rhs;
	return sum;
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator-(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	microfloat<n,e,i,na,s> diff = lhs;
	diff -= rhs;
	return diff;
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator*(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	microfloat<n,e,i,na,s> mul = lhs;
	mul *= rhs;
	return mul;
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator/(microfloat<n,e,i,na,s> lhs, microfloat<n,e,i,na,s> rhs) {
	microfloat<n,e,i,na,s> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// microfloat - literal binary arithmetic operators

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator+(microfloat<n,e,i,na,s> lhs, float rhs) {
	return operator+(lhs, microfloat<n,e,i,na,s>(rhs));
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator-(microfloat<n,e,i,na,s> lhs, float rhs) {
	return operator-(lhs, microfloat<n,e,i,na,s>(rhs));
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator*(microfloat<n,e,i,na,s> lhs, float rhs) {
	return operator*(lhs, microfloat<n,e,i,na,s>(rhs));
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator/(microfloat<n,e,i,na,s> lhs, float rhs) {
	return operator/(lhs, microfloat<n,e,i,na,s>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - microfloat binary arithmetic operators

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator+(float lhs, microfloat<n,e,i,na,s> rhs) {
	return operator+(microfloat<n,e,i,na,s>(lhs), rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator-(float lhs, microfloat<n,e,i,na,s> rhs) {
	return operator-(microfloat<n,e,i,na,s>(lhs), rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator*(float lhs, microfloat<n,e,i,na,s> rhs) {
	return operator*(microfloat<n,e,i,na,s>(lhs), rhs);
}

template<unsigned n, unsigned e, bool i, bool na, bool s>
inline constexpr microfloat<n,e,i,na,s> operator/(float lhs, microfloat<n,e,i,na,s> rhs) {
	return operator/(microfloat<n,e,i,na,s>(lhs), rhs);
}

}} // namespace sw::universal
