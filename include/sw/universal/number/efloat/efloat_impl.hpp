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
#include <universal/behavior/rounding.hpp>
#include <universal/behavior/status_flags.hpp>

namespace sw { namespace universal {

// =============================================================================
// Architectural Design Note on Rounding Modes (Issue #1091)
// =============================================================================
// The Universal Number Library implements efloat's rounding mode as a dynamic
// thread_local variable rather than a compile-time template parameter.
// This choice was made based on several key architectural and standard tenets:
//
// 1. IEEE-754 & MPFR Conformance:
//    The IEEE-754 standard and mature arbitrary-precision libraries like MPFR
//    model the rounding mode as a dynamic thread-local or global environment state
//    (analogous to the FPU control word managed via `fesetround()`). This allows
//    a single library body or compiled algorithm to execute under different
//    rounding modes without requiring code modification or separate compiles.
//
// 2. Runtime Flexibility & Numerical Analysis:
//    A thread_local runtime state allows developers to dynamically manipulate
//    the rounding behavior during execution, which is a prerequisite for:
//    - Interval Arithmetic: Dynamically toggling between RoundTowardPositive 
//      and RoundTowardNegative to compute strict upper and lower bounds.
//    - Sensitivity Analysis: Evaluating the same numeric algorithm under different
//      rounding modes to measure cumulative error and stability without recompilation.
//
// 3. Avoiding Type-System Proliferation:
//    If the rounding mode were a compile-time template parameter (e.g.,
//    efloat<nlimbs, Mode>), then efloat<16, RoundToNearest> and efloat<16, RoundToZero>
//    would be completely distinct C++ types. This would cause type-system
//    proliferation, complicate type-promotion rules, and require extensive
//    mixed-type operator overloading for every rounding mode permutation
//    (e.g., resolving the result type of adding a RoundToNearest to a RoundToZero).
//
// While a template parameter approach offers compile-time branch pruning (via
// `if constexpr`), the thread_local approach maximizes runtime utility, type
// safety, and compliance with established numeric standards.
// =============================================================================

inline thread_local RoundingMode efloat_rounding_mode = RoundingMode::RoundToNearest;

inline thread_local unsigned efloat_default_precision_bits = 0;

inline unsigned get_default_precision() noexcept { return efloat_default_precision_bits; }
inline void set_default_precision(unsigned bits) noexcept { efloat_default_precision_bits = bits; }

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
			_precision_limbs = (efloat_default_precision_bits > 0) ? std::max(1u, (efloat_default_precision_bits + 31) / 32) : nlimbs;
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
		if (!std::is_constant_evaluated()) {
			_precision_limbs = (efloat_default_precision_bits > 0) ? std::max(1u, (efloat_default_precision_bits + 31) / 32) : nlimbs;
		}
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

	// instance precision management
	unsigned get_precision() const noexcept { return _precision_limbs * 32; }
	void set_precision(unsigned bits) noexcept { _precision_limbs = std::max(1u, (bits + 31) / 32); normalize(); }

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
			if (rhs.isinf() && _sign != rhs._sign) {
				setnan();
				if (!std::is_constant_evaluated()) {
					efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
				}
			}
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
		_precision_limbs = std::max(_precision_limbs, rhs._precision_limbs);
		unsigned target_prec = _precision_limbs;

		std::vector<uint32_t> a_limbs = _limb;
		std::vector<uint32_t> b_limbs = rhs._limb;
		int64_t a_exp = _exponent;
		int64_t b_exp = rhs._exponent;
		
		// Align exponents
		if (a_exp < b_exp) {
			grow_for_shift(a_limbs, b_exp - a_exp, target_prec);
			shift_right(a_limbs, b_exp - a_exp);
			a_exp = b_exp;
		} else if (b_exp < a_exp) {
			grow_for_shift(b_limbs, a_exp - b_exp, target_prec);
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
	constexpr efloat& operator*=(const efloat& rhs) noexcept {
		_precision_limbs = std::max(_precision_limbs, rhs._precision_limbs);
		if (isnan() || rhs.isnan()) {
			setnan();
			return *this;
		}
		if (isinf()) {
			if (rhs.iszero()) {
				setnan();
				if (!std::is_constant_evaluated()) {
					efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
				}
			} else {
				_sign = (_sign != rhs._sign);
			}
			return *this;
		}
		if (rhs.isinf()) {
			if (iszero()) {
				setnan();
				if (!std::is_constant_evaluated()) {
					efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
				}
			} else {
				*this = rhs;
				_sign = (_sign != rhs._sign);
			}
			return *this;
		}
		if (iszero() || rhs.iszero()) {
			setzero();
			_sign = (_sign != rhs._sign);
			return *this;
		}

		// Optimization: if either operand is a power of 2, bypass long multiplication
		if (rhs._limb.size() == 1 && rhs._limb[0] == 0x80000000) {
			_exponent += rhs._exponent;
			_sign = (_sign != rhs._sign);
			return *this;
		}
		if (_limb.size() == 1 && _limb[0] == 0x80000000) {
			unsigned target_prec = _precision_limbs;
			int64_t old_exp = _exponent;
			bool original_sign = _sign;
			*this = rhs;
			_precision_limbs = target_prec;
			_exponent += old_exp;
			_sign = (original_sign != rhs._sign);
			return *this;
		}

		std::vector<uint32_t> product;
		multiply_limbs(product, _limb, rhs._limb);

		_limb = product;
		_exponent = _exponent + rhs._exponent + 1;
		_sign = (_sign != rhs._sign);

		normalize();
		return *this;
		}
		constexpr efloat& operator*=(double rhs) noexcept {
		return *this *= efloat(rhs);
		}
		constexpr efloat& operator/=(const efloat& rhs) noexcept {
		_precision_limbs = std::max(_precision_limbs, rhs._precision_limbs);
		if (isnan() || rhs.isnan()) {
			setnan();
			return *this;
		}
		if (rhs.iszero()) {
			if (iszero()) {
				setnan();
				if (!std::is_constant_evaluated()) {
					efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
				}
			} else {
				setinf(_sign != rhs._sign);
				if (!std::is_constant_evaluated()) {
					efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
				}
			}
			return *this;
		}
		if (iszero()) {
			return *this; // 0 / finite = 0
		}
		if (isinf()) {
			if (rhs.isinf()) {
				setnan();
				if (!std::is_constant_evaluated()) {
					efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
				}
			} else {
				_sign = (_sign != rhs._sign);
			}
			return *this;
		}
		if (rhs.isinf()) {
			setzero(); // finite / inf = 0
			_sign = (_sign != rhs._sign);
			return *this;
		}

		// Optimization: if rhs is a power of 2, bypass division
		if (rhs._limb.size() == 1 && rhs._limb[0] == 0x80000000) {
			_exponent -= rhs._exponent;
			_sign = (_sign != rhs._sign);
			return *this;
		}

		std::vector<uint32_t> quotient;
		bool remainder_non_zero = false;
		const bool result_sign = (_sign != rhs._sign);
		divide_limbs(quotient, _limb, rhs._limb, _precision_limbs + 1, remainder_non_zero); // generate _precision_limbs + 1 limbs

		if (round_limbs(quotient, _precision_limbs, efloat_rounding_mode, result_sign, remainder_non_zero)) {
			quotient.insert(quotient.begin(), 1u);
			_exponent += 32;
		}

		_limb = quotient;
		_exponent = _exponent - rhs._exponent;
		_sign = result_sign;

		normalize();
		return *this;
		}
	constexpr efloat& operator/=(double rhs) noexcept {
		return *this /= efloat(rhs);
	}

	// modifiers
	constexpr void clear() noexcept {
		_state = FloatingPointState::Normal;
		_sign = false;
		_exponent = 0;
		_limb.clear();
	}
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

	constexpr void normalize() {
		if (_state != FloatingPointState::Normal) {
			return;
		}

		int msb_pos = -1;
		for (size_t i = 0; i < _limb.size(); ++i) {
			if (_limb[i] != 0) {
				msb_pos = (_limb.size() - 1 - i) * 32 + (31 - clz(_limb[i]));
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
				for(int j = static_cast<int>(_limb.size()) - 1; j >= 0; --j) {
					uint64_t v = (uint64_t(_limb[j]) << 1) | carry;
					_limb[j] = static_cast<uint32_t>(v);
					carry = v >> 32;
				}
			}
		} else if (shift < 0) {
			shift_right(_limb, static_cast<unsigned>(-shift));
		}

		// Truncate trailing zero limbs at the LSB side (end of vector) to maintain minimal representation
		while (_limb.size() > 1 && _limb.back() == 0) {
			_limb.pop_back();
		}

		// Enforce precision limit and round
		if (_limb.size() > _precision_limbs) {
			if (round_limbs(_limb, _precision_limbs, efloat_rounding_mode, _sign)) {
				_limb.insert(_limb.begin(), 1u);
				_exponent += 32;
			}
			normalize(); // Recursive call to normalize the rounded/carried result
		}
	}

	static constexpr int compare_limbs(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) noexcept {
		size_t max_size = (a.size() > b.size() ? a.size() : b.size());
		for (size_t i = 0; i < max_size; ++i) {
			uint32_t val_a = (i < a.size() ? a[i] : 0u);
			uint32_t val_b = (i < b.size() ? b[i] : 0u);
			if (val_a > val_b) return 1;
			if (val_b > val_a) return -1;
		}
		return 0;
	}

protected:
	FloatingPointState    _state;    // exceptional state
	bool                  _sign;     // sign of the number: -1 if true, +1 if false, zero is positive
	int64_t               _exponent; // exponent of the number
	std::vector<uint32_t> _limb;     // limbs of the representation
	unsigned              _precision_limbs = nlimbs;

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
			// insert zeros at the MSB side (index 0)
			limbs.insert(limbs.begin(), limb_shift, 0u);
			// erase the LSBs that are shifted out (at the end)
			limbs.resize(limbs.size() - limb_shift);
		}

		if (bit_shift > 0) {
			uint32_t carry_mask = (1u << bit_shift) - 1;
			uint32_t carry = 0;
			for (size_t i = 0; i < limbs.size(); ++i) {
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
				limbs.resize(limbs.size() + growth, 0u); // appends zeros at the LSB side
			}
		}
	}

	static constexpr void align_sizes(std::vector<uint32_t>& a, std::vector<uint32_t>& b) noexcept {
		size_t max_limbs = std::max(a.size(), b.size());
		a.resize(max_limbs, 0u); // appends zeros at the LSB side
		b.resize(max_limbs, 0u);
	}

	static constexpr void add_limbs(std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		uint64_t carry = 0;
		for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) {
			uint64_t sum = uint64_t(a[i]) + uint64_t(b[i]) + carry;
			a[i] = static_cast<uint32_t>(sum);
			carry = sum >> 32;
		}
		if (carry) {
			a.insert(a.begin(), 1u); // insert carry-out at index 0 (MSB side)
		}
	}

	static constexpr void subtract_limbs(std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		uint64_t borrow = 0;
		for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) {
			uint64_t diff = (uint64_t(1) << 32) + uint64_t(a[i]) - uint64_t(b[i]) - borrow;
			a[i] = static_cast<uint32_t>(diff);
			borrow = (diff >> 32) ? 0 : 1;
		}
	}

	static constexpr void multiply_limbs(std::vector<uint32_t>& product, const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
		std::vector<uint32_t> rev_a = a;
		std::vector<uint32_t> rev_b = b;
		std::reverse(rev_a.begin(), rev_a.end()); // converts to LSB-first
		std::reverse(rev_b.begin(), rev_b.end());

		size_t m = a.size();
		size_t n = b.size();
		std::vector<uint32_t> rev_product(m + n, 0u);

		for (size_t i = 0; i < m; ++i) {
			uint64_t carry = 0;
			for (size_t j = 0; j < n; ++j) {
				uint64_t sum = uint64_t(rev_product[i + j]) + uint64_t(rev_a[i]) * uint64_t(rev_b[j]) + carry;
				rev_product[i + j] = static_cast<uint32_t>(sum);
				carry = sum >> 32;
			}
			rev_product[i + n] = static_cast<uint32_t>(carry);
		}

		std::reverse(rev_product.begin(), rev_product.end()); // converts back to MSB-first
		product = rev_product;
	}

	static constexpr bool round_limbs(std::vector<uint32_t>& limbs, size_t target_size, RoundingMode mode, bool sign, bool sticky_remainder = false) noexcept {
		if (limbs.size() <= target_size) return false;

		bool guard = (limbs[target_size] & 0x80000000) != 0;
		bool lsb = (limbs[target_size - 1] & 1) != 0;
		bool sticky = sticky_remainder || ((limbs[target_size] & 0x7FFFFFFF) != 0);
		for (size_t i = target_size + 1; i < limbs.size(); ++i) {
			if (limbs[i] != 0) {
				sticky = true;
				break;
			}
		}

		if (guard || sticky) {
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::Inexact);
			}
		}

		bool round_up = false;
		switch (mode) {
		case RoundingMode::RoundToNearest:
			if (guard) {
				if (sticky || lsb) {
					round_up = true;
				}
			}
			break;
		case RoundingMode::RoundToZero:
			break;
		case RoundingMode::RoundTowardPositive:
			if (!sign && (guard || sticky)) {
				round_up = true;
			}
			break;
		case RoundingMode::RoundTowardNegative:
			if (sign && (guard || sticky)) {
				round_up = true;
			}
			break;
		}

		limbs.resize(target_size);

		if (round_up) {
			uint64_t carry = 1;
			for (int i = static_cast<int>(target_size) - 1; i >= 0; --i) {
				uint64_t sum = uint64_t(limbs[i]) + carry;
				limbs[i] = static_cast<uint32_t>(sum);
				carry = sum >> 32;
				if (!carry) break;
			}
			if (carry) {
				return true; // carry-out occurred during rounding!
			}
		}
		return false;
	}

	static constexpr void divide_limbs(std::vector<uint32_t>& quotient, const std::vector<uint32_t>& a, const std::vector<uint32_t>& b, unsigned max_limbs, bool& remainder_non_zero) {
		quotient.assign(max_limbs, 0u);
		std::vector<uint32_t> div = b;
		std::vector<uint32_t> dvd = a;
		align_sizes(dvd, div);

		// Insert leading zero limb to prevent shift overflow bit-loss!
		dvd.insert(dvd.begin(), 0u);
		div.insert(div.begin(), 0u);
		remainder_non_zero = false;

		for (unsigned bit = 0; bit < max_limbs * 32; ++bit) {
			if (compare_limbs(dvd, div) >= 0) {
				// Set bit in quotient
				unsigned limb_idx = bit / 32;
				unsigned bit_idx = 31 - (bit % 32);
				quotient[limb_idx] |= (1u << bit_idx);
				subtract_limbs(dvd, div);
			}
			// Shift dividend left by 1 bit
			uint64_t carry = 0;
			for (int j = static_cast<int>(dvd.size()) - 1; j >= 0; --j) {
				uint64_t v = (uint64_t(dvd[j]) << 1) | carry;
				dvd[j] = static_cast<uint32_t>(v);
				carry = v >> 32;
			}
			remainder_non_zero = remainder_non_zero || (carry != 0);
		}
		// check if there are any non-zero bits left in dvd (remainder)
		if (!remainder_non_zero) {
			for (size_t i = 0; i < dvd.size(); ++i) {
				if (dvd[i] != 0) {
					remainder_non_zero = true;
					break;
				}
			}
		}
	}


	// convert arithmetic types into an elastic floating-point
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	efloat& convert_signed(SignedInt v) noexcept {
		clear();
		if (0 == v) {
			setzero();
			return *this;
		}
		bool neg = (v < 0);
		uint64_t magnitude = neg
			? (0ull - static_cast<uint64_t>(static_cast<int64_t>(v)))
			: static_cast<uint64_t>(v);

		uint32_t high = static_cast<uint32_t>(magnitude >> 32);
		uint32_t low = static_cast<uint32_t>(magnitude & 0xFFFFFFFFu);

		_state = FloatingPointState::Normal;
		_sign = neg;
		if (high != 0) {
			_limb = { high, low };
			_exponent = 63;
		} else {
			_limb = { low };
			_exponent = 31;
		}
		normalize();
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	efloat& convert_unsigned(UnsignedInt v) noexcept {
		clear();
		if (0 == v) {
			setzero();
			return *this;
		}
		uint64_t magnitude = static_cast<uint64_t>(v);
		uint32_t high = static_cast<uint32_t>(magnitude >> 32);
		uint32_t low = static_cast<uint32_t>(magnitude & 0xFFFFFFFFu);

		_state = FloatingPointState::Normal;
		_sign = false;
		if (high != 0) {
			_limb = { high, low };
			_exponent = 63;
		} else {
			_limb = { low };
			_exponent = 31;
		}
		normalize();
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
			_sign = std::signbit(rhs);
			_exponent = 0;
			// stay limbless
			return *this;
		case FP_NAN:
			_sign = std::signbit(rhs);
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
			_sign = std::signbit(rhs);
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

		_sign = std::signbit(rhs);
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
		normalize(); // Ensure canonical, minimal representation (truncating trailing zeros)
		return *this;
	}


	// convert elastic floating-point to native ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	Real convert_to_ieee754() const noexcept {
		switch (_state) {
		case FloatingPointState::Zero:
			return (_sign ? -Real(0.0) : +Real(0.0));
		case FloatingPointState::QuietNaN:
			return std::numeric_limits<Real>::quiet_NaN();
		case FloatingPointState::SignalingNaN:
			return std::numeric_limits<Real>::signaling_NaN();
		case FloatingPointState::Infinite:
			return (_sign ? -std::numeric_limits<Real>::infinity() : +std::numeric_limits<Real>::infinity());
		case FloatingPointState::Normal:
			break;
		}

		if constexpr (std::is_same_v<Real, float>) {
			constexpr unsigned S_bits = 1;
			constexpr unsigned F_bits = 23;
			constexpr unsigned E_bits = 8;
			constexpr int E_max = 127;
			constexpr int E_min = -126;
			constexpr int E_bias = 127;
			constexpr unsigned K = F_bits + 1; // 24
			constexpr unsigned shift_amt = 64 - K; // 40

			uint64_t raw_sig = 0;
			if (_limb.size() >= 1) {
				raw_sig |= (uint64_t(_limb[0]) << 32);
			}
			if (_limb.size() >= 2) {
				raw_sig |= _limb[1];
			}
			bool sticky_limbs = false;
			for (size_t i = 2; i < _limb.size(); ++i) {
				if (_limb[i] != 0) {
					sticky_limbs = true;
					break;
				}
			}

			int64_t exp = _exponent;
			uint64_t sig = raw_sig;
			unsigned eff_shift = shift_amt;
			bool is_subnormal = (exp < E_min);

			bool lsb = false;
			bool guard = false;
			bool sticky = false;

			if (is_subnormal) {
				int64_t sub_shift = E_min - exp;
				if (sub_shift >= static_cast<int64_t>(K)) {
					sig = 0;
					eff_shift = 64;
					sticky = sticky_limbs || (raw_sig != 0);
					guard = false;
					lsb = false;
				} else {
					eff_shift = shift_amt + static_cast<unsigned>(sub_shift);
					lsb = (sig & (1ULL << eff_shift)) != 0;
					guard = (sig & (1ULL << (eff_shift - 1))) != 0;
					sticky = sticky_limbs || ((sig & ((1ULL << (eff_shift - 1)) - 1)) != 0);
				}
			} else {
				lsb = (sig & (1ULL << shift_amt)) != 0;
				guard = (sig & (1ULL << (shift_amt - 1))) != 0;
				sticky = sticky_limbs || ((sig & ((1ULL << (shift_amt - 1)) - 1)) != 0);
			}

			if (guard || sticky) {
				efloat_exception_flags.set(ExceptionFlag::Inexact);
				if (is_subnormal) {
					efloat_exception_flags.set(ExceptionFlag::Underflow);
				}
			}

			bool round_up = false;
			switch (efloat_rounding_mode) {
			case RoundingMode::RoundToNearest:
				if (guard && (sticky || lsb)) round_up = true;
				break;
			case RoundingMode::RoundToZero:
				break;
			case RoundingMode::RoundTowardPositive:
				if (!_sign && (guard || sticky)) round_up = true;
				break;
			case RoundingMode::RoundTowardNegative:
				if (_sign && (guard || sticky)) round_up = true;
				break;
			}

			if (round_up) {
				if (eff_shift >= 64) {
					// Round up from full underflow to smallest subnormal
					sig = (1ULL << 63);
					exp = E_min;
					is_subnormal = true;
				} else {
					uint64_t prev_sig = sig;
					sig += (1ULL << eff_shift);
					if (sig < prev_sig) {
						if (is_subnormal) {
							sig = (1ULL << 63);
							exp = E_min;
							is_subnormal = false;
						} else {
							sig = (1ULL << 63);
							exp++;
						}
					}
				}
			}

			if (exp > E_max) {
				efloat_exception_flags.set(ExceptionFlag::Overflow | ExceptionFlag::Inexact);
				bool to_inf = true;
				switch (efloat_rounding_mode) {
				case RoundingMode::RoundToZero:
					to_inf = false;
					break;
				case RoundingMode::RoundTowardPositive:
					if (_sign) to_inf = false;
					break;
				case RoundingMode::RoundTowardNegative:
					if (!_sign) to_inf = false;
					break;
				default:
					break;
				}
				if (to_inf) {
					return (_sign ? -std::numeric_limits<float>::infinity() : +std::numeric_limits<float>::infinity());
				} else {
					return (_sign ? -std::numeric_limits<float>::max() : +std::numeric_limits<float>::max());
				}
			}

			uint32_t sign_bit = (_sign ? 1u : 0u);
			uint32_t exp_field = 0;
			uint32_t frac_field = 0;

			if (sig == 0) {
				exp_field = 0;
				frac_field = 0;
			} else if (is_subnormal) {
				exp_field = 0;
				frac_field = static_cast<uint32_t>((sig >> eff_shift) & ((1ULL << F_bits) - 1));
			} else {
				exp_field = static_cast<uint32_t>(exp + E_bias);
				frac_field = static_cast<uint32_t>((sig >> shift_amt) & ((1ULL << F_bits) - 1));
			}

			uint32_t bits = (sign_bit << (E_bits + F_bits)) | (exp_field << F_bits) | frac_field;
			return sw::bit_cast<float>(bits);

		} else if constexpr (std::is_same_v<Real, double>) {
			constexpr unsigned S_bits = 1;
			constexpr unsigned F_bits = 52;
			constexpr unsigned E_bits = 11;
			constexpr int E_max = 1023;
			constexpr int E_min = -1022;
			constexpr int E_bias = 1023;
			constexpr unsigned K = F_bits + 1; // 53
			constexpr unsigned shift_amt = 64 - K; // 11

			uint64_t raw_sig = 0;
			if (_limb.size() >= 1) {
				raw_sig |= (uint64_t(_limb[0]) << 32);
			}
			if (_limb.size() >= 2) {
				raw_sig |= _limb[1];
			}
			bool sticky_limbs = false;
			for (size_t i = 2; i < _limb.size(); ++i) {
				if (_limb[i] != 0) {
					sticky_limbs = true;
					break;
				}
			}

			int64_t exp = _exponent;
			uint64_t sig = raw_sig;
			unsigned eff_shift = shift_amt;
			bool is_subnormal = (exp < E_min);

			bool lsb = false;
			bool guard = false;
			bool sticky = false;

			if (is_subnormal) {
				int64_t sub_shift = E_min - exp;
				if (sub_shift >= static_cast<int64_t>(K)) {
					sig = 0;
					eff_shift = 64;
					sticky = sticky_limbs || (raw_sig != 0);
					guard = false;
					lsb = false;
				} else {
					eff_shift = shift_amt + static_cast<unsigned>(sub_shift);
					lsb = (sig & (1ULL << eff_shift)) != 0;
					guard = (sig & (1ULL << (eff_shift - 1))) != 0;
					sticky = sticky_limbs || ((sig & ((1ULL << (eff_shift - 1)) - 1)) != 0);
				}
			} else {
				lsb = (sig & (1ULL << shift_amt)) != 0;
				guard = (sig & (1ULL << (shift_amt - 1))) != 0;
				sticky = sticky_limbs || ((sig & ((1ULL << (shift_amt - 1)) - 1)) != 0);
			}

			if (guard || sticky) {
				efloat_exception_flags.set(ExceptionFlag::Inexact);
				if (is_subnormal) {
					efloat_exception_flags.set(ExceptionFlag::Underflow);
				}
			}

			bool round_up = false;
			switch (efloat_rounding_mode) {
			case RoundingMode::RoundToNearest:
				if (guard && (sticky || lsb)) round_up = true;
				break;
			case RoundingMode::RoundToZero:
				break;
			case RoundingMode::RoundTowardPositive:
				if (!_sign && (guard || sticky)) round_up = true;
				break;
			case RoundingMode::RoundTowardNegative:
				if (_sign && (guard || sticky)) round_up = true;
				break;
			}

			if (round_up) {
				if (eff_shift >= 64) {
					// Round up from full underflow to smallest subnormal
					sig = (1ULL << 63);
					exp = E_min;
					is_subnormal = true;
				} else {
					uint64_t prev_sig = sig;
					sig += (1ULL << eff_shift);
					if (sig < prev_sig) {
						if (is_subnormal) {
							sig = (1ULL << 63);
							exp = E_min;
							is_subnormal = false;
						} else {
							sig = (1ULL << 63);
							exp++;
						}
					}
				}
			}

			if (exp > E_max) {
				efloat_exception_flags.set(ExceptionFlag::Overflow | ExceptionFlag::Inexact);
				bool to_inf = true;
				switch (efloat_rounding_mode) {
				case RoundingMode::RoundToZero:
					to_inf = false;
					break;
				case RoundingMode::RoundTowardPositive:
					if (_sign) to_inf = false;
					break;
				case RoundingMode::RoundTowardNegative:
					if (!_sign) to_inf = false;
					break;
				default:
					break;
				}
				if (to_inf) {
					return (_sign ? -std::numeric_limits<double>::infinity() : +std::numeric_limits<double>::infinity());
				} else {
					return (_sign ? -std::numeric_limits<double>::max() : +std::numeric_limits<double>::max());
				}
			}

			uint64_t sign_bit = (_sign ? 1ULL : 0ULL);
			uint64_t exp_field = 0;
			uint64_t frac_field = 0;

			if (sig == 0) {
				exp_field = 0;
				frac_field = 0;
			} else if (is_subnormal) {
				exp_field = 0;
				frac_field = (sig >> eff_shift) & ((1ULL << F_bits) - 1);
			} else {
				exp_field = static_cast<uint64_t>(exp + E_bias);
				frac_field = (sig >> shift_amt) & ((1ULL << F_bits) - 1);
			}

			uint64_t bits = (sign_bit << (E_bits + F_bits)) | (exp_field << F_bits) | frac_field;
			return sw::bit_cast<double>(bits);

		} else {
			return static_cast<Real>(convert_to_ieee754<double>());
		}
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

	// mathematical truncation friend functions
	template<unsigned nnlimbs>
	friend constexpr efloat<nnlimbs> trunc(const efloat<nnlimbs>& x);
	template<unsigned nnlimbs>
	friend constexpr efloat<nnlimbs> floor(const efloat<nnlimbs>& x);
	template<unsigned nnlimbs>
	friend constexpr efloat<nnlimbs> ceil(const efloat<nnlimbs>& x);
	template<unsigned nnlimbs>
	friend constexpr efloat<nnlimbs> round(const efloat<nnlimbs>& x);
	template<unsigned nnlimbs>
	friend constexpr efloat<nnlimbs> rint(const efloat<nnlimbs>& x);
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

// abs(a): absolute value. Clears the sign on a copy, which is correct for
// every state: a negative normal becomes positive, -inf becomes +inf, -0
// becomes +0, and a NaN stays a NaN (its sign is irrelevant to isnan()).
template<unsigned nlimbs>
inline efloat<nlimbs> abs(const efloat<nlimbs>& a) {
	efloat<nlimbs> result(a);
	result.setsign(false);
	return result;
}

// fabs(a): floating-point absolute value (C <cmath> spelling); same as abs.
template<unsigned nlimbs>
inline efloat<nlimbs> fabs(const efloat<nlimbs>& a) {
	return abs(a);
}

////////////////////////////////////////////////////////////////////////////////
/// stream operators

// read an ASCII decimal literal and make a binary efloat out of it.
// Supports:
//   - "nan", "inf", "infinity" (case-insensitive, optional sign)
//   - decimal / scientific literals routed through decimal_to_binary::convert
//     ("1.5", "-3.14e2", "1e-100", "0x" is not accepted)
//
// The d2b target precision is bounded by the working budget BigBits (default
// decimal_to_binary::default_big_bits): convert<BigBits>'s reduction shifts the
// digit-integer left by ~(target + 3*neg_E) bits, which must fit in BigBits, so
// target is sized overflow-safely below that (a request beyond the ceiling is
// correctly rounded to the ceiling rather than returning garbage -- issue
// #1141). To parse to very high precision (e.g. a 1000-digit constant), pass a
// larger budget: parse<16384>(text, value). The default (2048) comfortably
// covers any literal a user types at ordinary precision.
template<unsigned BigBits, unsigned nlimbs>
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
	//
	// convert<BigBits>'s intermediate must fit in BigBits, and it grows two ways
	// depending on the effective base-10 exponent E (= exp10 - #fractional-digits):
	//   - E < 0 (sub-unit places): left-shifts the digit-integer by roughly
	//     (target_bits + 3*neg_E) bits, so target cannot approach BigBits.
	//   - E > 0 (large magnitude): multiplies by 5^E, growing the digit-integer
	//     by ~E*log2(5) < 3*E bits, independent of target.
	// Both are bounded by ~3 bits per "decimal place away from the unit". If even
	// the digit-integer plus that growth cannot fit in BigBits, the literal is too
	// large/small for this budget and we report failure (rather than returning the
	// garbage a silent overflow would produce -- issue #1141). Otherwise target is
	// sized to leave headroom; a request beyond the safe ceiling is correctly
	// rounded to that ceiling. Callers needing more precision or a wider magnitude
	// pass a larger BigBits, e.g. parse<16384>(...).
	auto scan = sw::universal::string_parse::scan_decimal_float(s);
	if (!scan.valid) return false;

	const std::int64_t  E        = static_cast<std::int64_t>(scan.exp10)
	                             - static_cast<std::int64_t>(scan.frac_part.size());
	const std::uint64_t neg_E    = (E < 0) ? static_cast<std::uint64_t>(-E) : 0ull;
	const std::uint64_t pos_E    = (E > 0) ? static_cast<std::uint64_t>( E) : 0ull;
	const std::uint64_t mag      = (neg_E > pos_E) ? neg_E : pos_E;        // one is 0
	const std::uint64_t sig      = scan.int_part.size() + scan.frac_part.size();
	const std::uint64_t sig_bits = sig * 34ull / 10ull + 8ull;            // ~3.33 bits/digit + guard

	// If the digit-integer plus its 5^|E| growth cannot fit, this budget is too
	// small for the literal's magnitude -- fail rather than overflow to garbage.
	if (sig_bits + 3ull * mag + 64ull > BigBits) return false;

	// Overflow-safe target. For E<0 the shift is target-relative, so cap target
	// below BigBits - (3*neg_E + sig_bits). For E>=0 the growth is target-
	// independent (already checked to fit), so target may go up to BigBits - 64.
	unsigned safe_ceiling;
	if (E < 0) {
		const std::uint64_t overhead = 3ull * neg_E + sig_bits + 64ull;
		safe_ceiling = (BigBits > overhead) ? static_cast<unsigned>(BigBits - overhead) : 1u;
	}
	else {
		safe_ceiling = (BigBits > 64u) ? (BigBits - 64u) : 1u;
	}

	unsigned want_bits   = value.get_precision();
	unsigned target_bits = (want_bits == 0u) ? 1u
	                     : (want_bits < safe_ceiling ? want_bits : safe_ceiling);
	if (target_bits == 0u) target_bits = 1u;

	auto r = sw::universal::decimal_to_binary::convert<BigBits>(scan, target_bits);
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
		using Big = sw::universal::decimal_to_binary::big_integer<BigBits>;
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
// forward reference: full decimal formatter, defined after the arithmetic
// operators it relies on (see below).
template<unsigned nlimbs>
std::string to_string(const efloat<nlimbs>& value, std::streamsize precision, std::streamsize width,
                      bool fixed, bool scientific, bool internal, bool left, bool showpos,
                      bool uppercase, char fill);

template<unsigned nlimbs>
inline std::ostream& operator<<(std::ostream& ostr, const efloat<nlimbs>& rhs) {
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize precision   = ostr.precision();
	std::streamsize width       = ostr.width();
	char fillChar               = ostr.fill();
	bool showpos    = (fmt & std::ios_base::showpos)    != 0;
	bool uppercase  = (fmt & std::ios_base::uppercase)  != 0;
	bool fixed      = (fmt & std::ios_base::fixed)      != 0;
	bool scientific = (fmt & std::ios_base::scientific) != 0;
	bool internal   = (fmt & std::ios_base::internal)   != 0;
	bool left       = (fmt & std::ios_base::left)       != 0;
	return ostr << to_string(rhs, precision, width, fixed, scientific, internal, left, showpos, uppercase, fillChar);
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
constexpr bool operator< (const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	if (lhs.isnan() || rhs.isnan()) return false;
	if (lhs.iszero() && rhs.iszero()) return false;
	if (lhs.iszero()) {
		return rhs.sign() != -1;
	}
	if (rhs.iszero()) {
		return lhs.sign() == -1;
	}

	int lhs_sign = lhs.sign();
	int rhs_sign = rhs.sign();
	if (lhs_sign != rhs_sign) {
		return lhs_sign == -1;
	}

	// Signs are equal. Compare absolute magnitudes.
	int abs_cmp = 0;
	if (lhs.isinf()) {
		if (rhs.isinf()) abs_cmp = 0;
		else abs_cmp = 1;
	} else if (rhs.isinf()) {
		abs_cmp = -1;
	} else {
		if (lhs.scale() < rhs.scale()) {
			abs_cmp = -1;
		} else if (lhs.scale() > rhs.scale()) {
			abs_cmp = 1;
		} else {
			// exponents are equal, compare limbs
			abs_cmp = efloat<nlimbs>::compare_limbs(lhs.bits(), rhs.bits());
		}
	}

	if (lhs_sign == -1) {
		return abs_cmp > 0; // for negative, larger absolute magnitude is smaller
	} else {
		return abs_cmp < 0; // for positive, smaller absolute magnitude is smaller
	}
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

////////////////////////////////////////////////////////////////////////////////
/// decimal formatting (binary -> decimal string at arbitrary precision)
///
/// Ported from ereal's to_string/to_digits: digits are extracted one at a time
/// using efloat's OWN arithmetic (scale to [1,10) via *10 / /10, then repeatedly
/// take floor and multiply by 10). Accuracy is limited by the operand's working
/// precision. (issue #1150)

namespace efloat_detail {

	// append a 2-or-3 digit signed decimal exponent
	inline void append_exponent(std::string& str, int e) {
		str += (e < 0 ? '-' : '+');
		e = (e < 0) ? -e : e;
		int k;
		if (e >= 100) { k = e / 100; str += static_cast<char>('0' + k); e -= 100 * k; }
		k = e / 10; str += static_cast<char>('0' + k); e -= 10 * k;
		str += static_cast<char>('0' + e);
	}

	// round the digit string in place, propagating a carry; bump *decimalPoint on overflow
	inline void round_string(std::vector<char>& s, int precision, int* decimalPoint) {
		int lastDigit = precision - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			int i = precision - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}
		if (s[0] > '9') {
			for (int i = precision - 1; i >= 2; --i) s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
			s[0u] = '1';
			s[1u] = '0';
			(*decimalPoint)++;
		}
	}

	// generate `precision`+1 decimal digits of |value| into s, with the decimal
	// exponent returned in `exponent` (value ~= 0.s[0]s[1]... * 10^(exponent+1),
	// i.e. s[0] is the leading significant digit at 10^exponent).
	template<unsigned nlimbs>
	void to_digits(const efloat<nlimbs>& value, std::vector<char>& s, int& exponent, int precision) {
		constexpr double log10_2 = 0.301029995663981;
		if (value.iszero()) {
			exponent = 0;
			for (int i = 0; i < precision; ++i) s[static_cast<unsigned>(i)] = '0';
			return;
		}

		// estimate the power-of-ten exponent from the binary scale, then correct
		int e = static_cast<int>(log10_2 * static_cast<double>(value.scale()));

		efloat<nlimbs> r(value); r.setsign(false);   // |value|
		const efloat<nlimbs> ten(10.0), one(1.0);
		if (e < 0)      { for (int k = 0; k < -e; ++k) r = r * ten; }
		else if (e > 0) { for (int k = 0; k <  e; ++k) r = r / ten; }
		if (r >= ten)     { r = r / ten; ++e; }
		else if (r < one) { r = r * ten; --e; }

		const int nrDigits = precision + 1;
		for (int i = 0; i < nrDigits; ++i) {
			if (r.iszero()) { for (int j = i; j < nrDigits; ++j) s[static_cast<unsigned>(j)] = '0'; break; }
			int digit = static_cast<int>(double(r));           // r in [0,10): leading digit
			r = r - efloat<nlimbs>(static_cast<double>(digit));
			r = r * ten;
			s[static_cast<unsigned>(i)] = static_cast<char>(digit + '0');
		}

		// repair any digit that fell just outside [0,9] from rounding
		for (int i = nrDigits - 1; i > 0; --i) {
			if (s[static_cast<unsigned>(i)] < '0')      { s[static_cast<unsigned>(i - 1)]--; s[static_cast<unsigned>(i)] += 10; }
			else if (s[static_cast<unsigned>(i)] > '9') { s[static_cast<unsigned>(i - 1)]++; s[static_cast<unsigned>(i)] -= 10; }
		}

		// round to `precision` digits, propagate carry
		int lastDigit = nrDigits - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			int i = nrDigits - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}
		if (s[0] > '9') {   // carry made the leading digit 10 -> shift
			++e;
			for (int i = precision; i >= 2; --i) s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
			s[0u] = '1';
			s[1u] = '0';
		}
		s[static_cast<unsigned>(precision)] = 0;
		exponent = e;
	}

}  // namespace efloat_detail

template<unsigned nlimbs>
std::string to_string(const efloat<nlimbs>& value, std::streamsize precision, std::streamsize width,
                      bool fixed, bool scientific, bool internal, bool left, bool showpos,
                      bool uppercase, char fill) {
	std::string s;
	if (fixed && scientific) fixed = false;   // scientific takes precedence
	if (precision < 0) precision = 6;         // default stream precision

	bool negative = (value.sign() == -1);   // sign(), not isneg(): isneg() is false for -inf
	int  e = 0;

	if (value.isnan()) {
		s = uppercase ? "NAN" : "nan";
		negative = false;
	}
	else {
		if (negative) s += '-'; else if (showpos) s += '+';

		if (value.isinf()) {
			s += uppercase ? "INF" : "inf";
		}
		else if (value.iszero()) {
			s += '0';
			if (precision > 0) { s += '.'; s.append(static_cast<unsigned>(precision), '0'); }
			if (!fixed) s += (uppercase ? "E+00" : "e+00");
		}
		else {
			int powerOfTenScale = static_cast<int>(std::floor(static_cast<double>(value.scale()) * 0.301029995663981));
			int integerDigits   = (fixed ? (powerOfTenScale + 1) : 1);
			int nrDigits        = integerDigits + static_cast<int>(precision);

			int minBuffer = static_cast<int>(nlimbs) * 16;
			int nrDigitsForFixedFormat = fixed ? std::max(minBuffer, nrDigits) : nrDigits;

			double fullMagnitude = std::fabs(static_cast<double>(value));
			if (fixed && (precision == 0) && (fullMagnitude < 1.0)) {
				s += (fullMagnitude >= 0.5) ? '1' : '0';
			}
			else if (fixed && nrDigits <= 0) {
				s += '0';
				if (precision > 0) { s += '.'; s.append(static_cast<unsigned>(precision), '0'); }
			}
			else {
				std::vector<char> t;
				if (fixed) {
					t.resize(static_cast<size_t>(nrDigitsForFixedFormat + 1));
					efloat_detail::to_digits(value, t, e, nrDigitsForFixedFormat);
					efloat_detail::round_string(t, nrDigitsForFixedFormat + 1, &integerDigits);
					if (integerDigits > 0) {
						int i;
						for (i = 0; i < integerDigits; ++i) s += t[static_cast<unsigned>(i)];
						if (precision > 0) {
							s += '.';
							for (int j = 0; j < static_cast<int>(precision); ++j, ++i) s += t[static_cast<unsigned>(i)];
						}
					}
					else {
						s += "0.";
						if (integerDigits < 0) s.append(static_cast<size_t>(-integerDigits), '0');
						for (int i = 0; i < nrDigitsForFixedFormat; ++i) s += t[static_cast<unsigned>(i)];
					}
				}
				else {
					t.resize(static_cast<size_t>(nrDigits + 1));
					efloat_detail::to_digits(value, t, e, nrDigits);
					s += t[0ull];
					if (precision > 0) s += '.';
					for (int i = 1; i <= static_cast<int>(precision); ++i) s += t[static_cast<unsigned>(i)];
				}
			}

			if (!fixed) { s += (uppercase ? 'E' : 'e'); efloat_detail::append_exponent(s, e); }
		}
	}

	// width / fill padding
	size_t strLength = s.length();
	if (width > 0 && strLength < static_cast<size_t>(width)) {
		size_t pad = static_cast<size_t>(width) - strLength;
		if (internal) {
			const bool hasSign = !s.empty() && (s[0] == '-' || s[0] == '+');
			s.insert(hasSign ? std::string::size_type(1) : std::string::size_type(0), pad, fill);
		}
		else if (left) s.append(pad, fill);
		else s.insert(std::string::size_type(0), pad, fill);
	}
	return s;
}

}} // namespace sw::universal
