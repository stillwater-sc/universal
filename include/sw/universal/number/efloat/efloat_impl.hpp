#pragma once
// efloat_impl.hpp: implementation of an adaptive precision binary floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>  // std::max, std::min, std::find_if
#include <cctype>     // std::isspace, std::tolower, in parse()
#include <cmath>      // std::fpclassify, std::frexp, std::ldexp (long double conversions)
#include <cstddef>    // std::size_t
#include <cstdint>
#include <ios>        // std::streamsize, taken by to_string(); no stream is opened
#include <limits>     // std::numeric_limits
#include <string>
#include <type_traits>
#include <utility>    // std::move
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// These default here rather than in the efloat.hpp umbrella, so that a translation
/// unit which includes core.hpp directly gets the same defaults (#1334, #1436).
/// Defining either before any efloat header still wins.

// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(EFLOAT_ENABLE_LITERALS)
// default is to enable them
#define EFLOAT_ENABLE_LITERALS 1
#endif

// enable throwing specific exceptions for efloat arithmetic errors
// left to application to enable
#if !defined(EFLOAT_THROW_ARITHMETIC_EXCEPTION)
// default is to not throw
#define EFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#endif

// supporting types and functions: the I/O-free halves of the native support --
// checkNaN from ieee754_core.hpp, scale(Real) and fractionBits(Real) from
// manipulators_core.hpp. The efloat.hpp umbrella still brings the text halves.
#include <universal/native/ieee754_core.hpp>
#include <universal/native/manipulators_core.hpp>
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
#include <universal/number/efloat/efloat_mpn.hpp>   // 64-bit limb products (#1762), quotients and square roots (#1763)
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
	static constexpr unsigned maxNrLimbs = nlimbs;  // limbs are 32-bit words, so 1024 limbs is 32k bits of precision

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
	// constexpr-clean.  Compound arithmetic and the free comparison
	// operators are fully implemented and used at runtime; they still
	// carry the constexpr specifier for a uniform interface, but because
	// they operate on the heap-allocated _limb expansion they escape
	// constant evaluation (the same #747 transient-allocation limit) and
	// so cannot be exercised in a constant expression.
	// Out of scope (heap-escape boundary): native-type ctors / operator=
	// (convert_ieee754 calls std::fpclassify which is not constexpr in
	// C++20), conversion-out (std::pow), parse() (std::string).
	constexpr efloat() noexcept
		: _state{ FloatingPointState::Zero }, _sign{ false }, _exponent{ 0 }, _limb{} {
		if (!std::is_constant_evaluated()) {
			_limb.push_back(0);
			_precision_limbs = default_precision_limbs();
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
			_precision_limbs = default_precision_limbs();
		}
	}

	// initializers for native types
	efloat(signed char iv)                      noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(short iv)                            noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(int iv)                              noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(long iv)                             noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(long long iv)                        noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(char iv)                             noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(unsigned short iv)                   noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(unsigned int iv)                     noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(unsigned long iv)                    noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(unsigned long long iv)               noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(float iv)                            noexcept : _precision_limbs{ 0 } { *this = iv; }
	efloat(double iv)                           noexcept : _precision_limbs{ 0 } { *this = iv; }

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
	efloat(long double iv)                      noexcept : _precision_limbs{ 0 } { *this = iv; }

	// convert_ieee754 only builds limbs for sizeof(Real) of 4 or 8, so a wide
	// long double (sizeof 16 on x86-64) produced 0. Decompose instead: frexp the
	// value into m * 2^e with m in [0.5,1) (safely inside double's range), split m
	// into an exact sum of doubles via the working double path, then apply e with
	// setexponent(). This preserves the full long double significand AND its
	// extended exponent range. Platforms where long double == double take the
	// direct path.
	efloat& operator=(long double rhs) noexcept {
		if constexpr (sizeof(long double) <= sizeof(double)) {
			return convert_ieee754(static_cast<double>(rhs));
		}
		else {
			switch (std::fpclassify(rhs)) {
			case FP_ZERO:
			case FP_NAN:
			case FP_INFINITE:
				return convert_ieee754(static_cast<double>(rhs));
			default:
				break;
			}
			int e = 0;
			long double m = std::frexp(rhs, &e);
			clear();
			// without a precision of its own the value is exact (#1757): sum at 4 limbs, which
			// hold any long double significand, and keep carrying none
			const bool carries = _precision_limbs != 0;
			if (!carries) _precision_limbs = 4u;
			long double r = m;
			for (int i = 0; i < 8 && r != 0.0L; ++i) {
				double hi = static_cast<double>(r);
				if (hi == 0.0) break;   // remainder fell below double's range
				*this += efloat(hi);
				r -= static_cast<long double>(hi);
			}
			if (!carries) _precision_limbs = 0u;
			if (!iszero()) setexponent(scale() + e);
			return *this;
		}
	}

	explicit operator long double() const noexcept {
		if constexpr (sizeof(long double) <= sizeof(double)) {
			return static_cast<long double>(convert_to_ieee754<double>());
		}
		else {
			switch (_state) {
			case FloatingPointState::Zero:         return _sign ? -0.0L : 0.0L;
			case FloatingPointState::QuietNaN:
			case FloatingPointState::SignalingNaN: return std::numeric_limits<long double>::quiet_NaN();
			case FloatingPointState::Infinite:     return _sign ? -std::numeric_limits<long double>::infinity()
			                                                    :  std::numeric_limits<long double>::infinity();
			case FloatingPointState::Normal:
			default:
				break;
			}
			// bring |x| into [1,2), extract as an exact sum of doubles, then scale
			// by the true (wide) exponent -- the dual of the assignment above.
			const std::int64_t k = scale();
			efloat n(*this);
			n.setsign(false);
			n.setexponent(0);
			long double s = 0.0L;
			efloat r(n);
			for (int i = 0; i < 4 && !r.iszero(); ++i) {
				double hi = static_cast<double>(r);
				if (hi == 0.0) break;
				s += static_cast<long double>(hi);
				r -= efloat(hi);
			}
			if (_sign) s = -s;
			return std::ldexp(s, static_cast<int>(k));
		}
	}
#endif

	// instance precision management
	//
	// A default-constructed efloat, a copy, and the result of an operation carry a precision.  A
	// value constructed from a native integer or float carries none (#1757): it is exact, and an
	// operation runs at its other operand's precision, so x * efloat(3) and x * 3 both stay at
	// x's precision rather than widening to the type's.  When neither operand carries one, the
	// result has none either, and rounds at the default precision: set_default_precision() if
	// set, else nlimbs * 32 bits.  get_precision() reports that effective precision.
	unsigned get_precision() const noexcept { return precision_limbs() * 32; }
	void set_precision(unsigned bits) noexcept { _precision_limbs = std::max(1u, (bits + 31) / 32); normalize(); }
	// whether the value carries a precision of its own, and dropping it: the value rounds to
	// the default precision and then carries none, as one constructed from a native value
	bool carries_precision() const noexcept { return _precision_limbs != 0; }
	void clear_precision() noexcept { _precision_limbs = 0; normalize(); }

	// prefix operators
	constexpr efloat operator-() const noexcept {
		if (iszero()) return *this;
		efloat negated(*this);
		negated.setsign(!_sign);
		return negated;
	}

	// arithmetic operators
	constexpr efloat& operator+=(const efloat& rhs) noexcept { return add(rhs, false); }

	constexpr efloat& operator+=(double rhs) noexcept {
		return *this += efloat(rhs);
	}
	// an integer operand converts exactly (all 64 bits), not through double
	template<typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
	constexpr efloat& operator+=(Integer rhs) noexcept {
		if (_state == FloatingPointState::Normal && rhs != 0) {   // in place, no efloat for rhs (#1762)
			const native_limbs b = integer_limbs(rhs);
			add_magnitude(b.limb, b.n, b.exponent, b.sign);
			return *this;
		}
		return *this += efloat(rhs);
	}
	constexpr efloat& operator-=(const efloat& rhs) noexcept {
		return add(rhs, true);
	}
	constexpr efloat& operator-=(double rhs) noexcept {
		return *this -= efloat(rhs);
	}
	// an integer operand converts exactly (all 64 bits), not through double
	template<typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
	constexpr efloat& operator-=(Integer rhs) noexcept {
		if (_state == FloatingPointState::Normal && rhs != 0) {
			const native_limbs b = integer_limbs(rhs);
			add_magnitude(b.limb, b.n, b.exponent, !b.sign);
			return *this;
		}
		return *this -= efloat(rhs);
	}
	constexpr efloat& operator*=(const efloat& rhs) noexcept {
		_precision_limbs = merged_precision(_precision_limbs, rhs._precision_limbs);
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
		// (the merged precision can be below an operand's that carried none: round to it)
		if (rhs._limb.size() == 1 && rhs._limb[0] == 0x80000000) {
			_exponent += rhs._exponent;
			_sign = (_sign != rhs._sign);
			normalize();
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
			normalize();
			return *this;
		}

		multiply_magnitude(rhs._limb.data(), rhs._limb.size(), rhs._exponent, rhs._sign);
		return *this;
		}
		constexpr efloat& operator*=(double rhs) noexcept {
		return *this *= efloat(rhs);
		}
	// an integer operand converts exactly (all 64 bits), not through double
	template<typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
	constexpr efloat& operator*=(Integer rhs) noexcept {
		if (_state == FloatingPointState::Normal && rhs != 0) {
			const native_limbs b = integer_limbs(rhs);
			multiply_magnitude(b.limb, b.n, b.exponent, b.sign);
			return *this;
		}
		return *this *= efloat(rhs);
	}
		constexpr efloat& operator/=(const efloat& rhs) noexcept {
		_precision_limbs = merged_precision(_precision_limbs, rhs._precision_limbs);
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
			normalize();
			return *this;
		}

		divide_magnitude(rhs._limb.data(), rhs._limb.size(), rhs._exponent, rhs._sign);
		return *this;
		}
	constexpr efloat& operator/=(double rhs) noexcept {
		return *this /= efloat(rhs);
	}
	// an integer operand converts exactly (all 64 bits), not through double
	template<typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
	constexpr efloat& operator/=(Integer rhs) noexcept {
		// a divisor of one limb, such as the (2n-1)(2n) of a series term, divides in place (#1761),
		// a wider one without a temporary efloat; powers of 2, zero and the special values take
		// the general path
		if (_state == FloatingPointState::Normal) {
			bool     negative  = false;
			uint64_t magnitude = 0;
			if constexpr (std::is_signed_v<Integer>) {
				negative  = (rhs < 0);
				magnitude = negative ? (0ull - static_cast<uint64_t>(static_cast<int64_t>(rhs))) : static_cast<uint64_t>(rhs);
			}
			else {
				magnitude = static_cast<uint64_t>(rhs);
			}
			if ((magnitude & (magnitude - 1u)) != 0u) {
				if (magnitude <= 0xFFFF'FFFFull) {
					divide_by_limb(static_cast<uint32_t>(magnitude), negative);
				}
				else {
					const native_limbs b = integer_limbs(rhs);
					divide_magnitude(b.limb, b.n, b.exponent, b.sign);
				}
				return *this;
			}
		}
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
			// the leading bit is below the top: move whole limbs, then the remaining bits, in one
			// pass each, rather than one bit per pass -- a cancelling subtraction can leave
			// hundreds of leading zero bits (#1714)
			const size_t limb_shift = static_cast<size_t>(shift / 32);
			const unsigned bit_shift = static_cast<unsigned>(shift % 32);
			const size_t n = _limb.size();
			for (size_t j = 0; j < n; ++j) {
				const size_t from = j + limb_shift;
				uint64_t v = (from < n) ? (uint64_t(_limb[from]) << bit_shift) : 0u;
				if (bit_shift != 0u && from + 1 < n) v |= uint64_t(_limb[from + 1]) >> (32u - bit_shift);
				_limb[j] = static_cast<uint32_t>(v);
			}
		} else if (shift < 0) {
			shift_right(_limb, static_cast<unsigned>(-shift));
		}

		// Truncate trailing zero limbs at the LSB side (end of vector) to maintain minimal representation
		while (_limb.size() > 1 && _limb.back() == 0) {
			_limb.pop_back();
		}

		// Enforce precision limit and round
		if (_limb.size() > precision_limbs()) {
			if (round_limbs(_limb, precision_limbs(), efloat_rounding_mode, _sign)) {
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
	unsigned              _precision_limbs = nlimbs;   // 0: carries none, see get_precision()

	// HELPER methods

	// *this + rhs, or *this - rhs with negate_rhs: subtraction flips rhs's sign here instead of
	// building a negated copy of it (#1761)
	constexpr efloat& add(const efloat& rhs, bool negate_rhs) noexcept {
		const unsigned precision = merged_precision(_precision_limbs, rhs._precision_limbs);
		const bool rhs_sign = (rhs._sign != (negate_rhs && !rhs.iszero()));   // -0 is 0, as operator-()
		// handle special cases
		if (isnan() || rhs.isnan()) {
			setnan();
			return *this;
		}
		if (isinf()) {
			if (rhs.isinf() && _sign != rhs_sign) {
				setnan();
				if (!std::is_constant_evaluated()) {
					efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
				}
			}
			return *this;
		}
		if (rhs.isinf()) {
			*this = rhs;
			_sign = rhs_sign;
			return *this;
		}
		if (iszero()) {
			*this = rhs;
			_sign = rhs_sign;
			_precision_limbs = precision;
			normalize();
			return *this;
		}
		_precision_limbs = precision;
		if (rhs.iszero()) {
			normalize();
			return *this;
		}

		add_magnitude(rhs._limb.data(), rhs._limb.size(), rhs._exponent, rhs_sign);
		return *this;
	}

	// *this + B for a finite nonzero *this and B = (b[0, nb), eb, b_sign), normalized, at the
	// precision already merged into *this; b may be *this's own limbs.  The operand with the
	// smaller exponent, Y, is aligned to the other, X: it keeps two limbs past the precision, or
	// X's length if longer, and bits that fall below those are folded into a sticky bit at the
	// bottom, so the sum and the difference round correctly (#1757); with a gap of a bit or none
	// it keeps all of its bits.  The aligned sum is formed
	// in scratch space and rounded into _limb once, without copying either operand (#1762).
	void add_magnitude(const uint32_t* b, size_t nb, int64_t eb, bool b_sign) {
		const size_t    target = precision_limbs();
		const uint32_t* a      = _limb.data();
		const size_t    na     = _limb.size();
		const bool      a_is_x = (_exponent >= eb);
		const uint32_t* x      = a_is_x ? a : b;
		const uint32_t* y      = a_is_x ? b : a;
		const size_t    nx     = a_is_x ? na : nb;
		const size_t    ny     = a_is_x ? nb : na;
		const int64_t   ex     = a_is_x ? _exponent : eb;
		const bool      x_sign = a_is_x ? _sign : b_sign;
		const uint64_t  d      = static_cast<uint64_t>(a_is_x ? _exponent - eb : eb - _exponent);

		// Y keeps ly limbs of X's frame; its bits below them are lost to the sticky bit.  A gap of
		// two bits or more cancels at most one leading bit, so the sticky bit stays far below the
		// rounding point; a gap of one bit can cancel any number of them, so Y then keeps every
		// bit, as it does with no gap at all (a Y longer than the precision, which carries none,
		// lost its last bits here before)
		const uint64_t window = (d <= 1) ? uint64_t(ny) + 1u : std::max<uint64_t>(nx, uint64_t(target) + 2u);
		const size_t   ly     = static_cast<size_t>(std::max<uint64_t>(ny, std::min<uint64_t>(ny + (d + 31) / 32, window)));
		const size_t   lx     = std::max(nx, ly);
		bool lost = false;
		if (d >= 32ull * ly) {
			lost = true;   // all of Y, which is nonzero
		}
		else {
			const uint64_t first = 32ull * ly - d;   // Y's first lost bit, counted from its top
			const size_t   li    = static_cast<size_t>(first / 32);
			if (li < ny) {
				lost = (y[li] & (0xFFFF'FFFFu >> (first % 32))) != 0u;
				for (size_t k = li + 1; !lost && k < ny; ++k) lost = (y[k] != 0u);
			}
		}
		const size_t len = lx + (lost ? 1u : 0u);   // the frame, plus a carry limb in front

		// frame limb i, for dl <= i < iend, holds the low db bits of y[i - dl - 1] and the top
		// 32 - db bits of y[i - dl]; the frame limbs from iend on hold none of Y's bits
		const size_t   dl   = static_cast<size_t>(std::min<uint64_t>(d / 32, ly));
		const unsigned db   = static_cast<unsigned>(d % 32);
		const size_t   iend = std::min(ly, dl + ny + 1);
		auto y_at = [&](size_t i) -> uint32_t {
			const size_t   j    = i - dl;
			const uint64_t pair = (uint64_t((j >= 1u) ? y[j - 1u] : 0u) << 32) | ((j < ny) ? y[j] : 0u);
			return static_cast<uint32_t>(pair >> db);
		};

		scratch32 buffer;
		uint32_t* f  = buffer.get(len + 1);
		uint32_t* fr = f + 1;   // the frame; f[0] takes a carry out of it
		for (size_t i = 0; i < nx; ++i) fr[i] = x[i];
		for (size_t i = nx; i < len; ++i) fr[i] = 0u;
		if (_sign == b_sign) {
			if (lost) fr[len - 1] = 1u;   // X is zero below its own limbs
			uint64_t carry = 0;
			for (size_t i = iend; i-- > dl;) {
				const uint64_t sum = uint64_t(fr[i]) + y_at(i) + carry;
				fr[i] = static_cast<uint32_t>(sum);
				carry = sum >> 32;
			}
			for (size_t i = dl; carry != 0u && i-- > 0;) carry = (++fr[i] == 0u) ? 1u : 0u;
			f[0] = static_cast<uint32_t>(carry);
			assign_rounded([f](size_t i) { return f[i]; }, len + 1, ex + 32);
			return;
		}
		// X - Y; a borrow out of the top means Y was the larger: negate the difference
		uint64_t borrow = 0;
		if (lost) { fr[len - 1] = 0xFFFF'FFFFu; borrow = 1; }
		for (size_t i = lx; borrow != 0u && i-- > iend;) borrow = (fr[i]-- == 0u) ? 1u : 0u;
		for (size_t i = iend; i-- > dl;) {
			const uint64_t diff = (uint64_t(1) << 32) + fr[i] - y_at(i) - borrow;
			fr[i]  = static_cast<uint32_t>(diff);
			borrow = (diff >> 32) ? 0u : 1u;
		}
		for (size_t i = dl; borrow != 0u && i-- > 0;) borrow = (fr[i]-- == 0u) ? 1u : 0u;
		f[0] = 0u;
		bool sign = x_sign;
		if (borrow != 0u) {
			uint64_t c = 1;
			for (size_t i = len; i-- > 0;) {
				const uint64_t v = uint64_t(static_cast<uint32_t>(~fr[i])) + c;
				fr[i] = static_cast<uint32_t>(v);
				c     = v >> 32;
			}
			sign = !x_sign;
		}
		_sign = sign;
		assign_rounded([f](size_t i) { return f[i]; }, len + 1, ex + 32);
	}

	// *this * B for finite nonzero *this and B = (b[0, nb), eb, b_sign), normalized, at the
	// precision already merged into *this; b may be *this's own limbs, and when it is, *this is
	// squared.  The significands are packed into 64-bit limbs, least significant first, an odd
	// count padded with a zero limb at the bottom, multiplied by efloat_mpn (schoolbook below
	// its Karatsuba threshold), and the exact product is rounded into _limb once (#1762).
	void multiply_magnitude(const uint32_t* b, size_t nb, int64_t eb, bool b_sign) {
		using efloat_mpn::limb_t;
		const uint32_t* a      = _limb.data();
		const size_t    na     = _limb.size();
		const bool      square = (a == b && na == nb);
		const size_t    ma     = (na + 1) / 2;
		const size_t    mb     = (nb + 1) / 2;
		const size_t    mt     = ma + mb;
		scratch64 buffer;
		limb_t* pa   = buffer.get(ma + mb + mt + efloat_mpn::scratch_limbs(std::max(ma, mb)));
		limb_t* pb   = pa + ma;
		limb_t* prod = pb + mb;
		limb_t* work = prod + mt;
		auto pack = [](limb_t* p, const uint32_t* v, size_t n) {
			const size_t m = (n + 1) / 2;
			for (size_t j = 0; j < m; ++j) {
				const size_t hi = 2 * m - 2 - 2 * j;   // index of the high half in the padded vector
				const uint32_t h = v[hi];
				const uint32_t l = (hi + 1 < n) ? v[hi + 1] : 0u;
				p[j] = (limb_t(h) << 32) | l;
			}
		};
		pack(pa, a, na);
		if (square) {
			efloat_mpn::mul_n(prod, pa, pa, ma, work, true);
		}
		else {
			pack(pb, b, nb);
			efloat_mpn::mul(prod, pa, ma, pb, mb, work);
		}
		const int64_t top = _exponent + eb + 1;
		_sign = (_sign != b_sign);
		// the product's 32-bit limbs, most significant first: its first na + nb hold a * b, and
		// any padding limbs after them are zero
		assign_rounded([prod, mt](size_t i) {
			const limb_t w = prod[mt - 1 - i / 2];
			return (i & 1u) ? static_cast<uint32_t>(w) : static_cast<uint32_t>(w >> 32);
		}, na + nb, top);
	}

	// Rounds the frame s(0), ..., s(len - 1), 32-bit limbs most significant first whose first bit
	// weighs 2^top, to the precision in the current mode and with *this's sign, and stores it as
	// normalize() would leave it: the leading bit at the top of _limb[0], at most precision_limbs()
	// limbs, no trailing zero limbs.  One pass, and _limb never grows past the precision.
	template<typename LimbAt>
	void assign_rounded(LimbAt s, size_t len, int64_t top) {
		size_t i0 = 0;
		while (i0 < len && s(i0) == 0u) ++i0;
		if (i0 == len) {
			setzero();
			return;
		}
		const unsigned sh       = static_cast<unsigned>(clz(s(i0)));
		const size_t   n        = len - i0;
		const int64_t  exponent = top - 32 * static_cast<int64_t>(i0) - sh;
		if (sh == 0u) {
			store_rounded([&](size_t k) { return s(i0 + k); }, n, exponent);
		}
		else {
			store_rounded([&](size_t k) -> uint32_t {   // limb k of the frame shifted to its leading bit
				uint32_t v = s(i0 + k) << sh;
				if (i0 + k + 1 < len) v |= s(i0 + k + 1) >> (32u - sh);
				return v;
			}, n, exponent);
		}
	}
	// assign_rounded for a frame q(0), ..., q(n - 1) whose leading bit is the top bit of q(0)
	template<typename LimbAt>
	void store_rounded(LimbAt q, size_t n, int64_t exponent) {
		const size_t target = precision_limbs();
		size_t keep = n;
		bool   up   = false;
		if (n > target) {
			keep = target;
			const uint32_t g      = q(target);
			const bool     guard  = (g & 0x8000'0000u) != 0u;
			bool           sticky = (g & 0x7FFF'FFFFu) != 0u;
			for (size_t k = target + 1; !sticky && k < n; ++k) sticky = (q(k) != 0u);
			if (guard || sticky) efloat_exception_flags.set(ExceptionFlag::Inexact);
			up = rounds_up(efloat_rounding_mode, _sign, (q(target - 1) & 1u) != 0u, guard, sticky);
		}
		_limb.resize(keep);
		for (size_t k = 0; k < keep; ++k) _limb[k] = q(k);
		if (up) {
			size_t k = keep;
			while (k > 0 && ++_limb[k - 1] == 0u) --k;
			if (k == 0) {   // carried out of the top: the value is a power of 2
				_limb.assign(1, 0x8000'0000u);
				++exponent;
			}
		}
		while (_limb.size() > 1 && _limb.back() == 0u) _limb.pop_back();
		_exponent = exponent;
		_state    = FloatingPointState::Normal;
	}

	// whether a value whose last kept bit is lsb, and whose discarded bits are guard (the first)
	// and sticky (any other), rounds away from zero in mode
	static constexpr bool rounds_up(RoundingMode mode, bool sign, bool lsb, bool guard, bool sticky) noexcept {
		switch (mode) {
		case RoundingMode::RoundToNearest:      return guard && (sticky || lsb);
		case RoundingMode::RoundToZero:         return false;
		case RoundingMode::RoundTowardPositive: return !sign && (guard || sticky);
		case RoundingMode::RoundTowardNegative: return sign && (guard || sticky);
		}
		return false;
	}

	// scratch space on the stack, or on the heap when an operation needs more
	template<typename T, size_t N>
	struct scratch_buffer {
		T              local[N];
		std::vector<T> heap;
		T* get(size_t n) {
			if (n <= N) return local;
			heap.resize(n);
			return heap.data();
		}
	};
	using scratch32 = scratch_buffer<uint32_t, 512>;
	using scratch64 = scratch_buffer<std::uint64_t, 768>;

	// a nonzero native integer as one or two normalized limbs, as efloat(v) would hold it
	struct native_limbs {
		uint32_t limb[2];
		size_t   n;
		int64_t  exponent;
		bool     sign;
	};
	template<typename Integer>
	static constexpr native_limbs integer_limbs(Integer v) noexcept {
		native_limbs r{};
		uint64_t magnitude = 0;
		if constexpr (std::is_signed_v<Integer>) {
			r.sign    = (v < 0);
			magnitude = r.sign ? (0ull - static_cast<uint64_t>(static_cast<int64_t>(v))) : static_cast<uint64_t>(v);
		}
		else {
			magnitude = static_cast<uint64_t>(v);
		}
		const uint32_t high = static_cast<uint32_t>(magnitude >> 32);
		const int      lz   = (high != 0u) ? clz(high) : 32 + clz(static_cast<uint32_t>(magnitude));
		magnitude <<= lz;
		r.limb[0]  = static_cast<uint32_t>(magnitude >> 32);
		r.limb[1]  = static_cast<uint32_t>(magnitude);
		r.n        = (r.limb[1] != 0u) ? 2u : 1u;
		r.exponent = 63 - lz;
		return r;
	}


	// the default precision in limbs: set_default_precision() if set, else the type's nlimbs
	static constexpr unsigned default_precision_limbs() noexcept {
		if (std::is_constant_evaluated()) return nlimbs;
		return (efloat_default_precision_bits > 0) ? std::max(1u, (efloat_default_precision_bits + 31) / 32) : nlimbs;
	}
	// the precision this value rounds at, in limbs
	constexpr unsigned precision_limbs() const noexcept {
		return (_precision_limbs != 0) ? _precision_limbs : default_precision_limbs();
	}
	// A native value is exact (#1757): without a precision of its own it is not rounded at the
	// default, which can be narrower than its 64 bits, but by the operation that uses it.
	constexpr void normalize_native() {
		if (_precision_limbs != 0) { normalize(); return; }
		_precision_limbs = 2u;   // 64 bits hold every native integer and float significand
		normalize();
		_precision_limbs = 0u;
	}
	// the precision of a result: the wider of the operands' that carry one, else none
	static constexpr unsigned merged_precision(unsigned a, unsigned b) noexcept {
		if (a == 0) return b;
		if (b == 0) return a;
		return (a > b) ? a : b;
	}

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

	// *this /= d for a finite nonzero *this and a d of one limb that is not a power of 2, rounded
	// once in the current mode (#1761): the quotient as operator/= forms it, without building
	// an efloat for d or a general long division.  The significand A (na limbs, read as a
	// fraction) is divided a limb at a time by d, normalized to dn = d 2^s by feeding A 2^s, with
	// Moller and Granlund's reciprocal (as efloat_mpn::divrem_1).  The quotient A/d has 2 to 33
	// leading zero bits (d >= 3), so target + 3 quotient limbs leave the guard limb exact after the
	// leading bit is moved to the top; the remainder is the sticky bit.  A correctly rounded
	// quotient is unique, so the result is the one the general path returns.
	constexpr void divide_by_limb(uint32_t d, bool negative) {
		const unsigned target = precision_limbs();
		const size_t   na     = _limb.size();
		const size_t   len    = std::max<size_t>(na, size_t(target) + 3u);
		const unsigned s      = static_cast<unsigned>(clz(d));
		const uint32_t dn     = d << s;
		const uint32_t inv    = static_cast<uint32_t>(~uint64_t(0) / dn - (uint64_t(1) << 32));
		std::vector<uint32_t> q(len, 0u);
		auto digit = [&](size_t j) -> uint32_t { return (j < na) ? _limb[j] : 0u; };
		uint32_t r = (s != 0u) ? (digit(0) >> (32u - s)) : 0u;   // below dn: the top quotient digit is 0
		for (size_t j = 0; j < len; ++j) {
			const uint32_t u = (s != 0u) ? ((digit(j) << s) | (digit(j + 1) >> (32u - s))) : digit(j);
			const uint64_t prod = uint64_t(inv) * r + ((uint64_t(r) << 32) | u);
			uint32_t qj = static_cast<uint32_t>(prod >> 32) + 1u;
			uint32_t rj = u - qj * dn;
			if (rj > static_cast<uint32_t>(prod)) { --qj; rj += dn; }
			if (rj >= dn) { ++qj; rj -= dn; }
			q[j] = qj;
			r = rj;
		}
		// bring the leading bit to the top: the bits that enter at the bottom lie below the guard
		// limb, where only the sticky remainder counts
		size_t lead = 0;
		while (q[lead] == 0u) ++lead;
		const unsigned bits = static_cast<unsigned>(clz(q[lead]));
		for (size_t j = 0; j < len; ++j) {
			const size_t from = j + lead;
			uint64_t v = (from < len) ? (uint64_t(q[from]) << bits) : 0u;
			if (bits != 0u && from + 1 < len) v |= uint64_t(q[from + 1]) >> (32u - bits);
			q[j] = static_cast<uint32_t>(v);
		}
		_exponent -= static_cast<int64_t>(32 * lead + bits);
		_sign = (_sign != negative);
		if (round_limbs(q, target, efloat_rounding_mode, _sign, r != 0u)) {
			q.insert(q.begin(), 1u);
			_exponent += 32;
		}
		_limb = std::move(q);
		normalize();
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

		const bool round_up = rounds_up(mode, sign, lsb, guard, sticky);

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

	// *this = *this / b for finite nonzero operands, rounded once in the current mode: the
	// significands as integers A and B (trailing zero limbs dropped, so a divisor such as 7, padded
	// out to the dividend's length, costs one limb), Q = floor(A 2^shift / B) with as many bits as
	// the precision plus a limb, and the remainder for the sticky bit.  Both are exact: efloat_mpn
	// divides on 64-bit limbs, schoolbook with a 3-by-2 reciprocal below its threshold and
	// divide-and-conquer above it, which costs a few multiplications (#1763).  Before, Knuth's
	// Algorithm D ran on 32-bit limbs, a quotient digit at a time.
	void divide_magnitude(const uint32_t* b, size_t nb, int64_t eb, bool b_sign) {
		using efloat_mpn::limb_t;
		const uint32_t* a  = _limb.data();
		size_t          na = _limb.size();
		while (na > 1 && a[na - 1] == 0u) --na;
		while (nb > 1 && b[nb - 1] == 0u) --nb;
		const size_t max_limbs = precision_limbs() + 1;   // 32-bit quotient limbs, the first bit weighing 2^0

		// the bits of an MSB-first limb vector as an integer, 64 at a time from bit p; p may be
		// negative, and the bits below 0 and above the top are zero
		auto bits_at = [](const uint32_t* v, size_t n, int64_t p) -> limb_t {
			auto limb32 = [v, n](int64_t j) -> limb_t {
				return (j >= 0 && j < static_cast<int64_t>(n)) ? limb_t(v[n - 1 - static_cast<size_t>(j)]) : 0u;
			};
			if (p <= -64) return 0u;
			if (p < 0) {
				const limb_t w = limb32(0) | (limb32(1) << 32);
				return w << static_cast<unsigned>(-p);
			}
			const int64_t  j = p / 32;
			const unsigned r = static_cast<unsigned>(p % 32);
			const limb_t   w = limb32(j) | (limb32(j + 1) << 32);
			return (r == 0u) ? w : ((w >> r) | (limb32(j + 2) << (64u - r)));
		};

		// the divisor B, normalized to dn limbs with its top bit set; a / b = (A / B) 2^(32 (nb - na))
		size_t lead = 0;
		while (lead + 1 < nb && b[lead] == 0u) ++lead;
		const int64_t bits_b = 32 * static_cast<int64_t>(nb - lead) - clz(b[lead]);
		const size_t  dn     = static_cast<size_t>((bits_b + 63) / 64);
		const int64_t sb     = 64 * static_cast<int64_t>(dn) - bits_b;
		// the dividend N = A 2^(shift + sb): Q = floor(N / (B 2^sb)) has its first bit at 2^(32 max_limbs - 1)
		const int64_t shift  = 32 * (static_cast<int64_t>(max_limbs) + static_cast<int64_t>(nb) - static_cast<int64_t>(na)) - 1;
		const int64_t total  = shift + sb;
		const int64_t bits_n = 32 * static_cast<int64_t>(na) + total;   // N < 2^bits_n
		size_t nn = (bits_n > 0) ? static_cast<size_t>((bits_n + 63) / 64) : 0u;
		if (nn < dn) nn = dn;
		const size_t qn = nn - dn + 1;          // with the top limb div_qr returns
		const size_t ql = (max_limbs + 1) / 2;  // the 64-bit limbs the quotient's bits fill

		scratch_buffer<limb_t, 1024> buffer;
		const size_t qlen = std::max(qn, ql);
		limb_t* d    = buffer.get(dn + nn + qlen + efloat_mpn::div_scratch_limbs(dn));
		limb_t* num  = d + dn;
		limb_t* q    = num + nn;
		limb_t* work = q + qlen;
		for (size_t k = 0; k < dn; ++k) d[k] = bits_at(b, nb, 64 * static_cast<int64_t>(k) - sb);
		for (size_t k = 0; k < nn; ++k) num[k] = bits_at(a, na, 64 * static_cast<int64_t>(k) - total);
		bool sticky = false;
		if (total < 0) {   // more dividend bits than the quotient can use: those below are sticky
			for (int64_t p = 0; !sticky && p < -total; p += 64) {
				limb_t w = bits_at(a, na, p);
				if (-total - p < 64) w &= (limb_t(1) << static_cast<unsigned>(-total - p)) - 1u;
				sticky = (w != 0u);
			}
		}
		for (size_t k = 0; k < qlen; ++k) q[k] = 0u;
		q[nn - dn] = efloat_mpn::div_qr(q, num, nn, d, dn, work);
		for (size_t k = 0; !sticky && k < dn; ++k) sticky = (num[k] != 0u);

		_sign = (_sign != b_sign);
		// the quotient's 32-bit limbs, most significant first, then a limb that only the sticky
		// bit sets: it lies below the guard bit whether or not the leading bit needs a shift
		assign_rounded([q, max_limbs](size_t i) -> uint32_t {
			if (i >= max_limbs) return 1u;
			const size_t k = max_limbs - 1 - i;
			return static_cast<uint32_t>(q[k / 2] >> (32u * (k & 1u)));
		}, max_limbs + (sticky ? 1u : 0u), _exponent - eb);
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
		normalize_native();
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
		normalize_native();
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
		normalize_native(); // canonical, minimal representation; exact without a precision (#1757)
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
	template<unsigned nnlimbs>
	friend constexpr bool operator< (const efloat<nnlimbs>& lhs, const efloat<nnlimbs>& rhs) noexcept;

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
/// string parsing

namespace efloat_parse_detail {

// bits converted below the target. Rounding to the target looks at them, and when parse()
// has dropped digits from a long literal they also absorb what was dropped.
constexpr unsigned guard_bits = 64u;

enum class parse_outcome { done, undecided, refused };

// Convert `d` to target_bits + guard_bits bits, in a result W bits wide, round it to
// target_bits by round-to-nearest-even, and store it in value.
//
// `inexact` says nonzero digits were dropped from the end of the text behind `d`. The value
// then lies strictly above d's magnitude, by less than one unit in the last guard bit, so the
// guard bits are the truncated value's bits or one unit more. The rounding is the same either
// way unless they read 0111...1, where one more unit makes them the midpoint 1000...0: that
// case is `undecided`, and parse() looks at more digits. `refused` is a literal whose exact
// conversion needs more working bits than the converter has.
template<unsigned W, unsigned nlimbs>
parse_outcome round_into(const sw::universal::string_parse::decimal_float_scan& d, unsigned target_bits,
                         bool inexact, efloat<nlimbs>& value) {
	using Big = sw::universal::decimal_to_binary::big_integer<W>;
	const unsigned work = target_bits + guard_bits;
	// past the widest width the converter estimates the mantissa from the magnitude: not a
	// value at this precision
	if (sw::universal::decimal_to_binary::detail::required_working_bits(d, work)
	    > sw::universal::decimal_to_binary::widest_working_bits) {
		return parse_outcome::refused;
	}
	auto r = sw::universal::decimal_to_binary::convert<W>(d, work);
	if (!r.valid) return parse_outcome::refused;
	if (r.is_zero) {
		value.setzero();
		if (r.negative) value.setsign(true);
		return parse_outcome::done;
	}

	Big mantissa = r.mantissa;
	const bool half = mantissa.at(guard_bits - 1u);
	bool below_half_all_ones = true;
	bool below = r.guard_bit || r.sticky_bit;
	for (unsigned b = 0; b + 1u < guard_bits; ++b) {
		const bool bit = mantissa.at(b);
		below = below || bit;
		below_half_all_ones = below_half_all_ones && bit;
	}
	if (inexact) {
		if (!half && below_half_all_ones) return parse_outcome::undecided;
		below = true;
	}
	mantissa >>= static_cast<int>(guard_bits);

	std::int64_t binary_scale = r.binary_scale;
	if (half && (below || mantissa.at(0))) {
		mantissa += Big(1);
		if (mantissa.at(target_bits)) {   // the carry escaped the top
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
	return parse_outcome::done;
}

}  // namespace efloat_parse_detail

// read an ASCII decimal literal and make a binary efloat out of it.
// Supports:
//   - "nan", "inf", "infinity" (case-insensitive, optional sign)
//   - decimal / scientific literals routed through decimal_to_binary::convert
//     ("1.5", "-3.14e2", "1e-100", "0x" is not accepted)
//
// The literal is correctly rounded to the value's precision, however many digits it has:
// digits beyond those that can affect the rounding are dropped and kept as a sticky bit, and
// the converter widens its intermediates as the remaining digits and the exponent need (#1713).
// BigBits is the narrowest result width tried first; it no longer limits the precision.
// The one limit is the converter's widest working width, 32768 bits: past about 16000 bits
// (efloat<>'s default precision is 32768) a literal is correctly rounded to the most it can reach.
//
// Returns false, and leaves a zero, for text that is not a number. A well-formed literal the
// converter cannot reach -- a decimal exponent beyond about +/-13000 at ordinary precision --
// returns false and leaves a NaN.
template<unsigned BigBits, unsigned nlimbs>
bool parse(const std::string& txt, efloat<nlimbs>& value) {
	// clear() leaves the Normal state with NO limbs, which is the state the digit loop
	// below builds on -- but it is also a value no arithmetic produces: not zero to
	// iszero() or operator==, printing as 0.000000e-01, with a truncated to_binary. Every
	// `return false` used to hand that half-built value back, and assign() ignores the
	// result, so a failed assign() left it in the caller's object (#1458). Each failure
	// path now leaves the canonical zero instead.
	value.clear();

	std::string s = txt;
	// trim ASCII whitespace
	auto not_space = [](unsigned char c) { return !std::isspace(c); };
	auto first = std::find_if(s.begin(), s.end(), not_space);
	auto last  = std::find_if(s.rbegin(), s.rend(), not_space).base();
	s = (first < last) ? std::string(first, last) : std::string{};
	if (s.empty()) {
		value.setzero();   // empty or whitespace-only input
		return false;
	}

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
	auto scan = sw::universal::string_parse::scan_decimal_float(s);
	if (!scan.valid) {
		value.setzero();   // malformed digits
		return false;
	}

	// The target is the value's precision, with no ceiling taken from BigBits: the converter
	// widens its intermediates as the literal needs, and the result is chosen wide enough to
	// hold target + guard bits.
	using namespace efloat_parse_detail;
	constexpr unsigned widest = sw::universal::decimal_to_binary::widest_working_bits;
	unsigned target_bits = std::max(1u, value.get_precision());
	if (target_bits + guard_bits + 2u > widest) target_bits = widest - guard_bits - 2u;

	// The converter's intermediates grow with the target, the digits and the exponent, and the
	// widest working width bounds them. Near that width -- efloat<>'s default 32768 bits is past
	// it -- the target is lowered to the most the literal can reach: about 32700 bits for a short
	// literal, and about 16000 for a long one, whose kept digits need as many bits again.
	// reachable() returns the largest target up to t that fits, or 0 if not even one bit does.
	auto reachable = [&](const sw::universal::string_parse::decimal_float_scan& d, unsigned t) -> unsigned {
		using sw::universal::decimal_to_binary::detail::required_working_bits;
		if (required_working_bits(d, 1u + guard_bits) > widest) return 0u;
		for (;;) {
			const std::uint64_t need = required_working_bits(d, t + guard_bits);
			if (need <= widest) return t;
			const std::uint64_t excess = need - widest;   // need rises at most one bit per target bit
			t = (excess < t) ? static_cast<unsigned>(t - excess) : 1u;
		}
	};

	auto attempt = [&](const sw::universal::string_parse::decimal_float_scan& d, unsigned t, bool inexact) {
		const unsigned w = t + guard_bits;
		if (w + 2u <= BigBits) return round_into<BigBits>(d, t, inexact, value);
		if constexpr (BigBits < 8192u) {
			if (w + 2u <= 8192u) return round_into<8192u>(d, t, inexact, value);
		}
		if constexpr (BigBits < widest) {
			return round_into<widest>(d, t, inexact, value);
		}
		else {
			return round_into<BigBits>(d, t, inexact, value);
		}
	};

	// A literal longer than the target needs only its leading digits. Past the first
	// `keep` significant digits the rest move the value by less than one unit in the last
	// guard bit, so they are dropped and stand in as a sticky bit (#1713). Without this the
	// converter's intermediates grow with the length of the literal, and a long reference
	// constant used to fail, or come back with fewer bits than a short one.
	std::string digits;
	digits.reserve(scan.int_part.size() + scan.frac_part.size());
	digits.append(scan.int_part.data(), scan.int_part.size());
	digits.append(scan.frac_part.data(), scan.frac_part.size());
	const std::size_t leading = std::min(digits.find_first_not_of('0'), digits.size());
	const std::size_t significant = digits.size() - leading;

	// a dropped digit moves the value by less than 10^-(keep-1) of it, and one unit in the
	// last guard bit is more than 2^-(target + guard_bits) of it: keep >= 1 + (target + guard_bits) * log10(2)
	auto keep_for = [](unsigned t) { return static_cast<std::size_t>(t + guard_bits) * 30103u / 100000u + 2u; };
	std::size_t widen = 1u;   // doubles each time the dropped digits sit on a rounding boundary
	parse_outcome outcome = parse_outcome::refused;
	for (;;) {
		const std::size_t keep = keep_for(target_bits) * widen;
		if (keep >= significant) {
			// every digit is converted, so the rounding is exact at whatever target fits
			const unsigned t = reachable(scan, target_bits);
			if (t != 0u) outcome = attempt(scan, t, false);
			break;
		}
		const bool inexact = digits.find_first_not_of('0', leading + keep) != std::string::npos;
		// the decimal exponent of the last digit kept
		const std::int64_t last = static_cast<std::int64_t>(scan.int_part.size())
		                        - static_cast<std::int64_t>(leading + keep)
		                        + static_cast<std::int64_t>(scan.exp10);
		std::string text;
		text.reserve(keep + 24u);
		if (scan.negative) text.push_back('-');
		text.append(digits, leading, keep);
		text.push_back('e');
		text.append(std::to_string(last));
		auto truncated = sw::universal::string_parse::scan_decimal_float(text);
		if (!truncated.valid) break;   // the exponent is out of the scanner's range
		const unsigned t = reachable(truncated, target_bits);
		if (t < target_bits) {
			// the kept digits were sized for more bits than fit: fewer bits need fewer digits
			if (target_bits == 1u) break;
			target_bits = (t != 0u) ? t : target_bits / 2u;
			continue;
		}
		outcome = attempt(truncated, t, inexact);
		if (outcome != parse_outcome::undecided) break;
		widen *= 2u;   // the dropped digits sit on a rounding boundary: look at more of them
	}

	if (outcome != parse_outcome::done) {
		// a well-formed literal this converter cannot reach, such as 1e100000: say so with a
		// NaN rather than a zero that looks like a value
		value.setnan(true);
		return false;
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
	if (lhs._limb.size() != rhs._limb.size()) return false;

	return efloat<nlimbs>::compare_limbs(lhs._limb, rhs._limb) == 0;
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
			abs_cmp = efloat<nlimbs>::compare_limbs(lhs._limb, rhs._limb);
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
	if (&lhs == &rhs) mul *= mul;   // x * x squares (#1761)
	else mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> ratio = lhs;
	ratio /= rhs;
	return ratio;
}
// a temporary left operand is reused, its limbs and all, rather than copied (#1762)
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator+(efloat<nlimbs>&& lhs, const efloat<nlimbs>& rhs) noexcept {
	lhs += rhs;
	return std::move(lhs);
}
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator-(efloat<nlimbs>&& lhs, const efloat<nlimbs>& rhs) noexcept {
	lhs -= rhs;
	return std::move(lhs);
}
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator*(efloat<nlimbs>&& lhs, const efloat<nlimbs>& rhs) noexcept {
	if (&lhs == &rhs) lhs *= lhs;
	else lhs *= rhs;
	return std::move(lhs);
}
template<unsigned nlimbs>
constexpr efloat<nlimbs> operator/(efloat<nlimbs>&& lhs, const efloat<nlimbs>& rhs) noexcept {
	lhs /= rhs;
	return std::move(lhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - literal binary arithmetic operators
// A native operand carries no precision (#1757): the result has the efloat operand's.  An
// integer operand converts exactly, all 64 bits, rather than through double.
// BINARY ADDITION
template<unsigned nlimbs>
inline efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, double rhs) {
	return operator+(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, Integer rhs) {
	efloat<nlimbs> sum(lhs);
	sum += rhs;   // in place, without an efloat for rhs (#1762)
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, double rhs) {
	return operator-(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, Integer rhs) {
	efloat<nlimbs> difference(lhs);
	difference -= rhs;   // in place, without an efloat for rhs (#1762)
	return difference;
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, double rhs) {
	return operator*(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, Integer rhs) {
	efloat<nlimbs> product(lhs);
	product *= rhs;   // in place, without an efloat for rhs (#1762)
	return product;
}
// BINARY DIVISION
template<unsigned nlimbs>
inline efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, double rhs) {
	return operator/(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, Integer rhs) {
	efloat<nlimbs> quotient(lhs);
	quotient /= rhs;   // a one-limb divisor divides in place (#1761)
	return quotient;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline efloat<nlimbs> operator+(double lhs, const efloat<nlimbs>& rhs) {
	return operator+(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline efloat<nlimbs> operator+(Integer lhs, const efloat<nlimbs>& rhs) {
	efloat<nlimbs> sum(rhs);   // + commutes, and is correctly rounded either way round
	sum += lhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline efloat<nlimbs> operator-(double lhs, const efloat<nlimbs>& rhs) {
	return operator-(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline efloat<nlimbs> operator-(Integer lhs, const efloat<nlimbs>& rhs) {
	return operator-(efloat<nlimbs>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline efloat<nlimbs> operator*(double lhs, const efloat<nlimbs>& rhs) {
	return operator*(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline efloat<nlimbs> operator*(Integer lhs, const efloat<nlimbs>& rhs) {
	if (lhs == 0 || rhs.iszero()) return operator*(efloat<nlimbs>(lhs), rhs);   // the sign of a zero product is not symmetric
	efloat<nlimbs> product(rhs);   // * commutes, and is correctly rounded either way round
	product *= lhs;
	return product;
}
// BINARY DIVISION
template<unsigned nlimbs>
inline efloat<nlimbs> operator/(double lhs, const efloat<nlimbs>& rhs) {
	return operator/(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline efloat<nlimbs> operator/(Integer lhs, const efloat<nlimbs>& rhs) {
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

	// append a signed decimal exponent, minimum 2 digits (efloat's exponent range
	// far exceeds double's, so 4+ digit exponents are reachable -- e.g. 1e1000).
	inline void append_exponent(std::string& str, int e) {
		str += (e < 0 ? '-' : '+');
		std::string digits = std::to_string((e < 0) ? -e : e);
		if (digits.size() < 2) digits.insert(0, 2u - digits.size(), '0');
		str += digits;
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

	if (value.isnan()) {
		s = uppercase ? "NAN" : "nan";
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
			int e               = 0;   // decimal exponent, filled in by to_digits below
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
					// compute extra guard digits (nrDigitsForFixedFormat) for accuracy,
					// but round and print exactly nrDigits (= integerDigits + precision).
					t.resize(static_cast<size_t>(nrDigitsForFixedFormat + 1));
					efloat_detail::to_digits(value, t, e, nrDigitsForFixedFormat);
					efloat_detail::round_string(t, nrDigits + 1, &integerDigits);
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
						for (int i = 0; i < nrDigits; ++i) s += t[static_cast<unsigned>(i)];
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
