#pragma once
// efloat_impl.hpp: implementation of an adaptive precision binary floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>  // std::max, std::min
#include <array>      // std::array, to_double_expansion()
#include <bit>        // std::bit_width, std::endian
#include <cstddef>    // std::size_t
#include <cstdint>
#include <cstring>    // std::memcpy, the long double encodings
#include <ios>        // std::streamsize, taken by to_string(); no stream is opened
#include <limits>     // std::numeric_limits
#include <string>
#include <string_view>
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
#include <universal/internal/limbs/limb_kernels.hpp>   // 64-bit limb products (#1762), quotients and square roots (#1763)
#include <universal/number/efloat/efloat_limbs.hpp> // the significand's storage (#1820)
#include <universal/number/efloat/double_math.hpp>  // the double estimates, without <cmath> (#1819)
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
// the 64-bit limbs efloat<nlimbs> holds inside the value before it takes a heap block (#1820):
// its precision and two limbs more, which the mathlib's working precisions use, up to
// EFLOAT_INLINE_LIMBS. Past that, a copy would move more than the limbs it holds.
#if !defined(EFLOAT_INLINE_LIMBS)
#define EFLOAT_INLINE_LIMBS 8
#endif
constexpr std::size_t efloat_inline_limbs(unsigned nlimbs) noexcept {
	return std::min<std::size_t>((nlimbs + 1u) / 2u + 2u, EFLOAT_INLINE_LIMBS);
}

template<unsigned nlimbs = 1024>
class efloat {
public:
	static constexpr unsigned maxNrLimbs = nlimbs;  // limbs are 32-bit words, so 1024 limbs is 32k bits of precision
	// the exponent range (#1786): a finite value's scale lies in [min_scale, max_scale]. The
	// int64_t exponent then holds the sum of two of them, so a product cannot overflow it, and a
	// result outside the range overflows or underflows (enforce_range). exp's range is the same.
	static constexpr int64_t max_scale = (int64_t(1) << 62) - 1;
	static constexpr int64_t min_scale = -max_scale;

	// the significand: 64-bit limbs, least significant first, in an inline buffer of
	// inline_limbs or on the heap past it (#1820)
	using limb_t = limb_kernels::limb_t;
	static constexpr std::size_t inline_limbs = efloat_inline_limbs(nlimbs);

	// Constant evaluation (#747, #1819): the integer, dyadic, float and double constructors, the
	// conversions to float and double, +, -, *, /, the compound operators, the comparisons, the
	// math library and parse() run in a constant expression, with the same results as at run
	// time. The limb_kernels kernels take their portable loops there instead
	// of the assembly, the scratch past the inline limbs comes from new and delete instead of the
	// thread's block cache, the rounding is to nearest and no status flag is raised: the thread's
	// mode and flags are out of a constant expression's reach (rounding_mode(), raise_flags()). A
	// value whose limbs fit the inline buffer can be a constexpr variable; a larger one cannot
	// keep its heap block. Not yet: long double, whose encodings are read and written with memcpy.
	constexpr efloat() noexcept
		: _state{ FloatingPointState::Zero }, _sign{ false }, _exponent{ 0 }, _limb{} {
		_limb.assign(1, 0u);   // zero holds one limb of 0, as bits() shows it
		_precision_limbs = default_precision_limbs();
	}

	// the copies and moves copy the scalar members one by one, the exponent first: as the
	// defaulted ones, clang copied the state, the sign and the exponent as one 16-byte vector,
	// which stalls on the separate stores an operation has just written them with (#1828)
	constexpr efloat(const efloat& rhs)
		: _state{ rhs._state }, _sign{ rhs._sign }, _exponent{ rhs._exponent }, _limb{ rhs._limb }, _precision_limbs{ rhs._precision_limbs } {}
	constexpr efloat(efloat&& rhs) noexcept
		: _state{ rhs._state }, _sign{ rhs._sign }, _exponent{ rhs._exponent }, _limb{ std::move(rhs._limb) }, _precision_limbs{ rhs._precision_limbs } {}

	// a value the binary operators overwrite at once (#1826): zero with no limbs and the type's
	// precision, without reading the thread's default precision
	struct uninitialized_t {};
	explicit constexpr efloat(uninitialized_t) noexcept
		: _state{ FloatingPointState::Zero }, _sign{ false }, _exponent{ 0 }, _limb{}, _precision_limbs{ nlimbs } {}

	constexpr efloat& operator=(const efloat& rhs) {
		_exponent        = rhs._exponent;
		_state           = rhs._state;
		_sign            = rhs._sign;
		_limb            = rhs._limb;
		_precision_limbs = rhs._precision_limbs;
		return *this;
	}
	constexpr efloat& operator=(efloat&& rhs) noexcept {
		_exponent        = rhs._exponent;
		_state           = rhs._state;
		_sign            = rhs._sign;
		_limb            = std::move(rhs._limb);
		_precision_limbs = rhs._precision_limbs;
		return *this;
	}

	// specialized constructor for testing and verification: limbs are 32-bit, most significant
	// first, the top bit of limbs[0] weighing 2^exponent
	constexpr efloat(bool sign, int64_t exponent, const std::vector<uint32_t>& limbs, bool nan = false, bool inf = false, bool zero = false) {
		_sign = sign;
		_exponent = exponent;
		import_limbs32(limbs.data(), limbs.size());
		if (nan) _state = FloatingPointState::QuietNaN;
		else if (inf) _state = FloatingPointState::Infinite;
		else if (zero) _state = FloatingPointState::Zero;
		else _state = FloatingPointState::Normal;
		_precision_limbs = default_precision_limbs();
	}

	// initializers for native types
	constexpr efloat(signed char iv)                      noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(short iv)                            noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(int iv)                              noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(long iv)                             noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(long long iv)                        noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(char iv)                             noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(unsigned short iv)                   noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(unsigned int iv)                     noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(unsigned long iv)                    noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(unsigned long long iv)               noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(float iv)                  noexcept : _precision_limbs{ 0 } { *this = iv; }
	constexpr efloat(double iv)                 noexcept : _precision_limbs{ 0 } { *this = iv; }

	// assignment operators for native types
	constexpr efloat& operator=(signed char rhs)          noexcept { return convert_signed(rhs); }
	constexpr efloat& operator=(short rhs)                noexcept { return convert_signed(rhs); }
	constexpr efloat& operator=(int rhs)                  noexcept { return convert_signed(rhs); }
	constexpr efloat& operator=(long rhs)                 noexcept { return convert_signed(rhs); }
	constexpr efloat& operator=(long long rhs)            noexcept { return convert_signed(rhs); }
	constexpr efloat& operator=(char rhs)                 noexcept { return convert_unsigned(rhs); }
	constexpr efloat& operator=(unsigned short rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr efloat& operator=(unsigned int rhs)         noexcept { return convert_unsigned(rhs); }
	constexpr efloat& operator=(unsigned long rhs)        noexcept { return convert_unsigned(rhs); }
	constexpr efloat& operator=(unsigned long long rhs)   noexcept { return convert_unsigned(rhs); }
	constexpr efloat& operator=(float rhs)      noexcept { return convert_ieee754(rhs); }
	constexpr efloat& operator=(double rhs)     noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit constexpr operator float()   const noexcept { return convert_to_ieee754<float>(); }
	explicit constexpr operator double()  const noexcept { return convert_to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	efloat(long double iv)                      noexcept : _precision_limbs{ 0 } { *this = iv; }

	// A long double is taken apart by its bits, as float and double are, in each of the formats
	// it has (#1819): x87's 64-bit significand with its explicit leading bit, IEEE binary128's
	// 113 bits, and IBM's double-double, which is the exact sum of two doubles. Where long double
	// is double, the double path. Its zeros, infinities and NaNs go through double, as before.
	efloat& operator=(long double rhs) noexcept {
		constexpr int digits = std::numeric_limits<long double>::digits;
		if constexpr (digits == 53) {
			return convert_ieee754(static_cast<double>(rhs));
		}
		else if constexpr (digits == 64 || digits == 113) {
			// the encoding's two 64-bit words, low first on a little-endian target
			uint64_t w[2]{ 0u, 0u };
			std::memcpy(w, &rhs, (digits == 64) ? 10u : 16u);
			if constexpr (std::endian::native == std::endian::big) {
				const uint64_t t = w[0];
				w[0] = w[1];
				w[1] = t;
			}
			// x87: the significand in w[0], sign and biased exponent in the low 16 bits of w[1];
			// binary128: sign, exponent and the top 48 fraction bits in w[1], the rest in w[0]
			const unsigned raw_exponent = (digits == 64) ? static_cast<unsigned>(w[1] & 0x7FFFu)
			                                             : static_cast<unsigned>((w[1] >> 48) & 0x7FFFu);
			uint64_t hi = (digits == 64) ? 0u : (w[1] & 0xFFFF'FFFF'FFFFull);
			uint64_t lo = w[0];
			if (digits == 113 && raw_exponent != 0) hi |= uint64_t(1) << 48;   // the hidden bit
			if (raw_exponent == 0x7FFFu || (hi == 0u && lo == 0u)) return convert_ieee754(static_cast<double>(rhs));
			clear();
			_sign = (digits == 64) ? ((w[1] >> 15) & 1u) != 0u : (w[1] >> 63) != 0u;
			_state = FloatingPointState::Normal;
			// the value is (hi, lo) 2^(e - bias - (digits - 1)), with e = 1 for a subnormal
			const int64_t lsb_weight = ((raw_exponent != 0) ? static_cast<int64_t>(raw_exponent) : 1) - 16383 - (digits - 1);
			const int     width      = (hi != 0u) ? 64 + static_cast<int>(std::bit_width(hi)) : static_cast<int>(std::bit_width(lo));
			_exponent = lsb_weight + width - 1;
			const unsigned up = static_cast<unsigned>(128 - width);   // to the top of two limbs
			if (up >= 64u) {
				hi = lo << (up - 64u);
				lo = 0u;
			}
			else if (up > 0u) {
				hi = (hi << up) | (lo >> (64u - up));
				lo <<= up;
			}
			const limb_t limbs[2]{ lo, hi };
			_limb.assign(limbs, 2);
		}
		else {
			// double-double: rhs = hi + lo exactly, |lo| <= ulp(hi) / 2, and lo can lie any distance
			// below hi (1 + 2^-1000 is one)
			const double hi = static_cast<double>(rhs);
			if (hi == 0.0 || hi - hi != 0.0) return convert_ieee754(hi);   // zero, inf, nan
			const double lo = static_cast<double>(rhs - static_cast<long double>(hi));
			const bool carries = _precision_limbs != 0;
			convert_ieee754(hi);
			if (lo != 0.0) {
				// summed exactly, over the bits from hi's leading one to lo's last, unless the
				// value carries a precision of its own
				if (!carries) {
					const int span = efloat_detail::fp::ilogb(hi) - efloat_detail::fp::ilogb(lo) + 54;
					_precision_limbs = std::max(4u, static_cast<unsigned>(span + 31) / 32u);
				}
				*this += efloat(lo);
				if (!carries) _precision_limbs = 0u;
			}
			return *this;
		}
		// without a precision of its own the value is exact (#1757): normalized at 4 limbs,
		// which hold any long double significand, and keeping none
		if (_precision_limbs != 0) {
			normalize();
		}
		else {
			_precision_limbs = 4u;
			normalize();
			_precision_limbs = 0u;
		}
		return *this;
	}

	explicit operator long double() const noexcept {
		constexpr int digits = std::numeric_limits<long double>::digits;
		if constexpr (digits == 53) {
			return static_cast<long double>(convert_to_ieee754<double>());
		}
		else if constexpr (digits == 64 || digits == 113) {
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
			// rounded once to the format, and assembled from its fields
			const native_rounding r = round_to_native<static_cast<unsigned>(digits), -16382, 16383>();
			if (r.overflow) {
				if (r.to_inf) return _sign ? -std::numeric_limits<long double>::infinity() : std::numeric_limits<long double>::infinity();
				return _sign ? -std::numeric_limits<long double>::max() : std::numeric_limits<long double>::max();
			}
			const uint64_t exp_field = r.normal ? static_cast<uint64_t>(r.scale + 16383) : 0u;
			const uint64_t sign_bit  = _sign ? 1u : 0u;
			uint64_t w[2]{ 0u, 0u };
			if constexpr (digits == 64) {
				w[0] = r.lo;   // the significand, its leading bit explicit
				w[1] = (sign_bit << 15) | exp_field;
			}
			else {
				w[0] = r.lo;
				w[1] = (sign_bit << 63) | (exp_field << 48) | (r.hi & 0xFFFF'FFFF'FFFFull);
			}
			if constexpr (std::endian::native == std::endian::big) {
				const uint64_t t = w[0];
				w[0] = w[1];
				w[1] = t;
			}
			long double v = 0.0L;
			std::memcpy(&v, w, (digits == 64) ? 10u : 16u);
			return v;
		}
		else {
			// double-double: the double nearest the value, and the double nearest what it leaves
			const double hi = convert_to_ieee754<double>();
			if (hi == 0.0 || hi - hi != 0.0) return static_cast<long double>(hi);   // zero, inf, nan
			efloat rest(*this);
			rest -= efloat(hi);
			const double lo = rest.template convert_to_ieee754<double>();
			return static_cast<long double>(hi) + static_cast<long double>(lo);
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
	constexpr unsigned get_precision() const noexcept { return precision_limbs() * 32; }
	// precision is held in whole 32-bit limbs: set_precision(24) keeps 32 bits and
	// set_precision(100) keeps 128, so a reference to compare against must round at
	// get_precision(), not at the bits requested
	constexpr void set_precision(unsigned bits) noexcept { _precision_limbs = std::max(1u, (bits + 31) / 32); normalize(); }
	// whether the value carries a precision of its own, and dropping it: the value rounds to
	// the default precision and then carries none, as one constructed from a native value
	constexpr bool carries_precision() const noexcept { return _precision_limbs != 0; }
	constexpr void clear_precision() noexcept { _precision_limbs = 0; normalize(); }

	// prefix operators
	constexpr efloat operator-() const noexcept {
		efloat negated(*this);   // -(+0) = -0, as IEEE-754 negation (#1768)
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
				raise_flags(ExceptionFlag::InvalidOperation);
			} else {
				_sign = (_sign != rhs._sign);
			}
			return *this;
		}
		if (rhs.isinf()) {
			if (iszero()) {
				setnan();
				raise_flags(ExceptionFlag::InvalidOperation);
			} else {
				const bool negative = (_sign != rhs._sign);   // before the assignment replaces it (#1768)
				*this = rhs;
				_sign = negative;
			}
			return *this;
		}
		if (iszero() || rhs.iszero()) {
			const bool negative = (_sign != rhs._sign);   // before setzero() clears it (#1768)
			setzero();
			_sign = negative;
			return *this;
		}

		// Optimization: if either operand is a power of 2, bypass long multiplication
		// (the merged precision can be below an operand's that carried none: round to it)
		if (rhs.is_power_of_2()) {
			_exponent += rhs._exponent;
			_sign = (_sign != rhs._sign);
			normalize();
			return *this;
		}
		if (is_power_of_2()) {
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
				raise_flags(ExceptionFlag::InvalidOperation);
			} else {
				setinf(_sign != rhs._sign);
				raise_flags(ExceptionFlag::DivisionByZero);
			}
			return *this;
		}
		if (iszero()) {
			_sign = (_sign != rhs._sign);   // 0 / finite = 0, signed as a quotient (#1768)
			return *this;
		}
		if (isinf()) {
			if (rhs.isinf()) {
				setnan();
				raise_flags(ExceptionFlag::InvalidOperation);
			} else {
				_sign = (_sign != rhs._sign);
			}
			return *this;
		}
		if (rhs.isinf()) {
			const bool negative = (_sign != rhs._sign);   // before setzero() clears it (#1768)
			setzero(); // finite / inf = 0
			_sign = negative;
			return *this;
		}

		// Optimization: if rhs is a power of 2, bypass division
		if (rhs.is_power_of_2()) {
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
				divide_by_limb(magnitude, negative);   // any divisor of up to 64 bits (#1811)
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
		_limb.assign(1, 0u);   // as the default ctor holds it, so bits() is the same however zero was reached
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
	// sets 32-bit limb i of the significand, counted from the top as bits() lists them; the
	// value is not normalized
	constexpr void setlimb(unsigned i, std::uint32_t value) {
		const std::size_t m    = _limb.size();
		const std::size_t need = i / 2u + 1u;
		if (need > m) {   // the significand grows at the bottom: its top limb keeps its weight
			_limb.resize(need);
			limb_t* w = _limb.data();
			const std::size_t d = need - m;
			for (std::size_t k = need; k-- > d;) w[k] = w[k - d];
			for (std::size_t k = 0; k < d; ++k) w[k] = 0u;
		}
		limb_t& w = _limb[_limb.size() - 1u - i / 2u];
		w = (i & 1u) ? ((w & 0xFFFF'FFFF'0000'0000ull) | value) : ((w & 0xFFFF'FFFFull) | (limb_t(value) << 32));
	}
	// sets the significand to w[0, n), 64-bit limbs least significant first, the top bit of
	// w[n - 1] weighing 2^scale(); the value is not normalized
	constexpr void set_significand(const limb_t* w, std::size_t n) { _limb.assign(w, n); }
	// For the mathlib's exact kernels (#1802): *this = (-1)^sign (R + s) 2^(top - 64 n + 1), R the
	// nonzero integer r[0, n) of 64-bit limbs, least significant first, whose bit 64 n - 1 weighs
	// 2^top, and s in (0, 1) when sticky, else 0: rounded once in the current mode at
	// precision_limbs limbs, which the value then carries. r must not be this value's limbs.
	constexpr void assign_rounded_limbs(bool sign, const limb_t* r, std::size_t n, std::int64_t top, bool sticky, unsigned precision_limbs) {
		_precision_limbs = precision_limbs;
		_sign = sign;
		round_into(r, n, top, sticky);
	}
	// T + (below + s) / B rounded at precision_limbs, 2 M - 1 or 2 M, for T = t[0, M) with its top
	// bit set and its leading bit weighing 2^exponent, below the limb under T and s in (0, 1) when
	// rest, else 0; t is overwritten (#1828). Inlined, as store_short is, so that t can stay in
	// registers: clang left it a call (#1830).
	template<std::size_t M>
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((always_inline))
#endif
	inline constexpr void assign_rounded_short(bool sign, limb_t* t, limb_t below, bool rest, std::int64_t exponent, unsigned precision_limbs) {
		_precision_limbs = precision_limbs;
		store_short<M>(t, below, rest, exponent, sign);
	}

	constexpr efloat& assign(std::string_view txt) {
		parse(txt, *this);
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept { return _state == FloatingPointState::Zero; }
	constexpr bool isone()  const noexcept { return (_state == FloatingPointState::Normal && !_sign && _exponent == 0 && is_power_of_2()); }
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
			uint64_t raw = _limb.empty() ? 0u : _limb.back();   // the top 64 bits
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
	// the significand as 32-bit limbs, most significant first, the leading bit at the top of
	// limb 0 and no trailing zero limbs: a copy, converted from the 64-bit limbs
	std::vector<uint32_t> bits() const {
		const std::size_t n32 = limb32_count();
		std::vector<uint32_t> v(n32);
		for (std::size_t i = 0; i < n32; ++i) v[i] = limb32(i);
		return v;
	}
	// the same limbs as bits() (#1783); the mathlib reads limb_data() instead, without the copy
	std::vector<uint32_t> limb_vector() const { return bits(); }
	// the significand as it is held (#1820): limb_count() 64-bit limbs, least significant first,
	// the top bit of the last weighing 2^scale() in a normalized value, an odd count of 32-bit
	// limbs padded with zeros at the bottom
	constexpr const limb_t* limb_data()  const noexcept { return _limb.data(); }
	constexpr std::size_t   limb_count() const noexcept { return _limb.size(); }
	// the number of 32-bit limbs bits() lists
	constexpr std::size_t limb32_count() const noexcept {
		const std::size_t m = _limb.size();
		if (m == 0) return 0;
		return 2 * m - (((_limb[0] & 0xFFFF'FFFFull) == 0u) ? 1u : 0u);
	}
	// 32-bit limb i, counted from the top, for i < 2 limb_count()
	constexpr std::uint32_t limb32(std::size_t i) const noexcept {
		const limb_t w = _limb[_limb.size() - 1u - i / 2u];
		return (i & 1u) ? static_cast<std::uint32_t>(w) : static_cast<std::uint32_t>(w >> 32);
	}

	// the leading bit at the top of the top limb, no trailing zero limbs, rounded to the
	// precision, and inside the exponent range (enforce_range)
	constexpr void normalize() {
		normalize_significand();
		if (!in_range(_exponent)) [[unlikely]] enforce_range();
	}

	// normalize() without the range check, which stays outside: inside the rounding, it cost
	// exp 8% at 128 bits (#1786). The top bit of the top limb weighs 2^_exponent before and after.
	constexpr void normalize_significand() {
		if (_state != FloatingPointState::Normal) {
			return;
		}
		std::size_t m = _limb.size();
		limb_t* w = _limb.data();
		std::size_t t = m;
		while (t > 0 && w[t - 1] == 0u) --t;
		if (t == 0) {
			setzero();
			return;
		}
		// move the leading bit to the top and drop the zero limbs below the last nonzero one, in
		// one pass each: a cancelling subtraction can leave hundreds of leading zero bits (#1714)
		const unsigned sh = static_cast<unsigned>(std::countl_zero(w[t - 1]));
		_exponent -= 64 * static_cast<int64_t>(m - t) + sh;
		std::size_t z = 0;
		while (z + 1 < t && w[z] == 0u) ++z;   // w[t - 1] != 0
		m = t - z;
		if (z > 0) {
			for (std::size_t k = 0; k < m; ++k) w[k] = w[k + z];
		}
		if (sh != 0u) {
			limb_kernels::lshift(w, w, m, sh);   // the top limb has sh leading zeros: nothing is lost
			if (m > 1 && w[0] == 0u) {   // the bottom limb's bits all moved up
				--m;
				for (std::size_t k = 0; k < m; ++k) w[k] = w[k + 1];
			}
		}
		_limb.resize_for_overwrite(m);
		round_to_precision();
	}

	// whether e lies in [min_scale, max_scale], in one unsigned comparison: e + max_scale wraps
	// past 2 max_scale on both sides
	static constexpr bool in_range(int64_t e) noexcept {
		return static_cast<uint64_t>(e) + static_cast<uint64_t>(max_scale) <= 2u * static_cast<uint64_t>(max_scale);
	}

	// A finite value outside [min_scale, max_scale] overflows or underflows in the current mode, as
	// IEEE-754 does without subnormals (#1786). It has already been rounded to its precision, so
	// overflow is decided after rounding. Overflow is inf, or the largest value at the precision
	// when the mode rounds toward zero from it; underflow is 0, or min() when the mode rounds away
	// from zero, and to nearest when the value lies above min() / 2, the midpoint rounding to 0.
	// The check is inline at each call site and this, the rare path, is out of line.
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noinline, cold))
#endif
	constexpr void enforce_range() noexcept {
		if (_state != FloatingPointState::Normal || in_range(_exponent)) return;
		RoundingMode mode = rounding_mode();   // not const: see rounding_mode()
		const bool upward = (mode == RoundingMode::RoundTowardPositive && !_sign) || (mode == RoundingMode::RoundTowardNegative && _sign);
		if (_exponent > max_scale) {
			if (mode == RoundingMode::RoundToNearest || upward) {
				setinf(_sign);
			}
			else {
				const unsigned p = precision_limbs();   // the largest value: 32 p ones
				_limb.assign((p + 1u) / 2u, ~limb_t(0));
				if (p & 1u) _limb[0] = 0xFFFF'FFFF'0000'0000ull;
				_exponent = max_scale;
			}
			raise_flags(ExceptionFlag::Overflow | ExceptionFlag::Inexact);
			return;
		}
		const bool to_min = upward || (mode == RoundingMode::RoundToNearest && _exponent == min_scale - 1 && !is_power_of_2());
		if (to_min) {
			_limb.assign(1, limb_t(1) << 63);
			_exponent = min_scale;
		}
		else {
			const bool negative = _sign;
			setzero();
			_sign = negative;
		}
		raise_flags(ExceptionFlag::Underflow | ExceptionFlag::Inexact);
	}

	// the significands of two normalized values compared, most significant limbs first, the
	// shorter one extended with zeros
	static constexpr int compare_limbs(const efloat& a, const efloat& b) noexcept {
		const std::size_t na = a._limb.size(), nb = b._limb.size();
		const std::size_t n  = std::max(na, nb);
		for (std::size_t i = 0; i < n; ++i) {
			const limb_t va = (i < na) ? a._limb[na - 1 - i] : 0u;
			const limb_t vb = (i < nb) ? b._limb[nb - 1 - i] : 0u;
			if (va != vb) return (va > vb) ? 1 : -1;
		}
		return 0;
	}

	// a significand of one bit: the value is a power of 2
	constexpr bool is_power_of_2() const noexcept { return _limb.size() == 1 && _limb[0] == (limb_t(1) << 63); }

protected:
	FloatingPointState          _state;    // exceptional state
	bool                        _sign;     // sign of the number: -1 if true, +1 if false, zero is positive
	int64_t                     _exponent; // exponent of the number: the weight of the significand's top bit
	efloat_limbs<inline_limbs>  _limb;     // the significand (#1820), see limb_data()
	unsigned                    _precision_limbs = nlimbs;   // in 32-bit limbs; 0: carries none, see get_precision()

	// HELPER methods

	// *this + rhs, or *this - rhs with negate_rhs: subtraction flips rhs's sign here instead of
	// building a negated copy of it (#1761)
	constexpr efloat& add(const efloat& rhs, bool negate_rhs) noexcept {
		const unsigned precision = merged_precision(_precision_limbs, rhs._precision_limbs);
		const bool rhs_sign = (rhs._sign != negate_rhs);
		// handle special cases
		if (isnan() || rhs.isnan()) {
			setnan();
			return *this;
		}
		if (isinf()) {
			if (rhs.isinf() && _sign != rhs_sign) {
				setnan();
				raise_flags(ExceptionFlag::InvalidOperation);
			}
			return *this;
		}
		if (rhs.isinf()) {
			*this = rhs;
			_sign = rhs_sign;
			return *this;
		}
		if (iszero()) {
			// IEEE-754 6.3: a sum of two zeros of opposite signs is +0, -0 rounding toward -inf (#1768)
			const bool negative = rhs.iszero() ? exact_zero_sum_sign(_sign, rhs_sign) : rhs_sign;
			*this = rhs;
			_sign = negative;
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

public:
	// *this = a b, and *this = a / b, without copying a first (#1826): finite nonzero operands go
	// straight to the limbs; zeros, infinities, NaNs and a power-of-2 b take a copy of a and the
	// compound operator, which keeps them usable in constant evaluation
	constexpr efloat& assign_product(const efloat& a, const efloat& b) {
		if (a._state == FloatingPointState::Normal && b._state == FloatingPointState::Normal && this != &a && this != &b) {
			_precision_limbs = merged_precision(a._precision_limbs, b._precision_limbs);
			multiply_magnitudes(a._limb.data(), a._limb.size(), a._exponent, a._sign, b._limb.data(), b._limb.size(), b._exponent, b._sign);
			return *this;
		}
		*this = a;
		if (&a == &b) return *this *= *this;   // x * x squares (#1761)
		return *this *= b;
	}
	constexpr efloat& assign_quotient(const efloat& a, const efloat& b) {
		if (a._state == FloatingPointState::Normal && b._state == FloatingPointState::Normal && !b.is_power_of_2() && this != &a && this != &b) {
			_precision_limbs = merged_precision(a._precision_limbs, b._precision_limbs);
			divide_magnitudes(a._limb.data(), a._limb.size(), a._exponent, a._sign, b._limb.data(), b._limb.size(), b._exponent, b._sign);
			return *this;
		}
		*this = a;
		return *this /= b;
	}
	// *this = a + b, or a - b with negate_b, without copying a first (#1826): finite nonzero
	// operands go straight to add_magnitudes, which writes the sum into this value's limbs; zeros,
	// infinities and NaNs take a copy of a and add(), as a += b does
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((always_inline))
#endif
	constexpr efloat& assign_sum(const efloat& a, const efloat& b, bool negate_b) {
		if (a._state == FloatingPointState::Normal && b._state == FloatingPointState::Normal && this != &a && this != &b) [[likely]] {
			_precision_limbs = merged_precision(a._precision_limbs, b._precision_limbs);
			add_magnitudes(a._limb.data(), a._limb.size(), a._exponent, a._sign, b._limb.data(), b._limb.size(), b._exponent, b._sign != negate_b);
			return *this;
		}
		return assign_sum_special(a, b, negate_b);
	}

protected:

	// assign_sum's other cases, out of line so that the operators keep only the finite ones (#1830)
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noinline))
#endif
	constexpr efloat& assign_sum_special(const efloat& a, const efloat& b, bool negate_b) {
		*this = a;
		return add(b, negate_b);
	}

	// *this + B for a finite nonzero *this and B = (b[0, nb), eb, b_sign), normalized, at the
	// precision already merged into *this; b may be *this's own limbs. The operand with the
	// smaller exponent, Y, is shifted into the frame of the other, X: L limbs below X's top,
	// enough for all of Y, or for 64 bits past the precision when Y reaches further. Y's bits
	// below the frame are lost to a sticky bit, held by a limb below the frame, so the sum and
	// the difference round correctly (#1757): a gap of two bits or more cancels at most one
	// leading bit, so the sticky bit stays far below the rounding point, and with a gap of one
	// bit or none the frame holds every bit of Y. The sum is formed by limb_kernels::add_n or
	// sub_n on 64-bit limbs and rounded into _limb once (#1762, #1820).
	constexpr void add_magnitude(const limb_t* b, std::size_t nb, int64_t eb, bool b_sign) {
		add_magnitudes(_limb.data(), _limb.size(), _exponent, _sign, b, nb, eb, b_sign);
	}
	// *this = A + B for finite nonzero, normalized A = (a[0, na), ea, a_sign) and B, at the
	// precision already in *this; a and b may be *this's own limbs, or another value's (#1826).
	// Inlined into its callers, which then call the sum's kernel directly (#1830).
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((always_inline))
#endif
	inline constexpr void add_magnitudes(const limb_t* a, std::size_t na, int64_t ea, bool a_sign, const limb_t* b, std::size_t nb, int64_t eb, bool b_sign) {
		const std::size_t target = precision_limbs();
		_sign = a_sign;
		const bool        a_is_x = (ea >= eb);
		const limb_t*     x      = a_is_x ? a : b;
		const limb_t*     y      = a_is_x ? b : a;
		const std::size_t nx     = a_is_x ? na : nb;
		const std::size_t ny     = a_is_x ? nb : na;
		const int64_t     ex     = a_is_x ? ea : eb;
		const bool        x_sign = a_is_x ? a_sign : b_sign;
		const uint64_t    d      = static_cast<uint64_t>(a_is_x ? ea - eb : eb - ea);
		if (nx == ny && nx == (target + 1) / 2 && d < 64 * uint64_t(nx) && (a_sign == b_sign || d >= 1)) {
			// up to four limbs as straight-line code, longer operands in loops (#1826)
			switch (nx) {
			case 1:  add_short<1>(x, y, ex, d, a_sign != b_sign, x_sign); break;
			case 2:  add_short<2>(x, y, ex, d, a_sign != b_sign, x_sign); break;
			case 3:  add_short<3>(x, y, ex, d, a_sign != b_sign, x_sign); break;
			case 4:  add_short<4>(x, y, ex, d, a_sign != b_sign, x_sign); break;
			default: add_full_length(x, y, nx, ex, d, a_sign != b_sign, x_sign); break;
			}
			return;
		}
		add_frame(x, nx, y, ny, ex, x_sign, d, a_sign == b_sign);
	}

	// add_magnitudes for operands that are not both as long as the precision, or a difference
	// with no gap: Y shifted into a frame of 64 bits past the precision, as described above. Out
	// of line, so that its scratch frame is not set up for the full-length sums.
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noinline))
#endif
	constexpr void add_frame(const limb_t* x, std::size_t nx, const limb_t* y, std::size_t ny, int64_t ex, bool x_sign, uint64_t d, bool same_sign) {
		const std::size_t target = precision_limbs();
		std::size_t L = 0;
		if (d <= 1) {
			L = std::max(nx, ny + static_cast<std::size_t>(d));
		}
		else {
			const uint64_t all  = uint64_t(ny) + (d + 63) / 64;           // the limbs that hold all of Y
			const uint64_t keep = (32 * uint64_t(target) + 64 + 63) / 64;  // 64 bits past the precision
			L = static_cast<std::size_t>(std::max<uint64_t>(nx, std::min(all, keep)));
		}

		// g = Y' B + lost: limb 0 below the frame, then the frame's L limbs; f = X B +- g, with
		// f's limb L + 1 for the carry out of a sum. Y is shifted once, into g, and X meets it in
		// one pass of add_n or sub_n over X's limbs; g's limbs below X go to f as they are, or
		// negated
		scratch64 buffer;
		limb_t* f = buffer.get(2 * L + 3);
		limb_t* g = f + L + 2;
		bool lost = false;
		limb_t* gy = g + 1;
		// Y lands at 2^(room - d) of its own weight: shifted up when it is shorter than the frame
		// and the gap leaves it room, else down, by d plus the limbs it has past the frame
		const uint64_t room = (L >= ny) ? 64 * uint64_t(L - ny) : 0u;
		const uint64_t over = (L >= ny) ? 0u : 64 * uint64_t(ny - L);
		if (over == 0u && d <= room) {
			const std::size_t q = static_cast<std::size_t>((room - d) / 64);
			const unsigned    c = static_cast<unsigned>((room - d) % 64);
			efloat_storage::zero_limbs(gy, q);
			if (c == 0u) {
				efloat_storage::copy_limbs(gy + q, y, ny);
				efloat_storage::zero_limbs(gy + q + ny, L - q - ny);
			}
			else {
				const limb_t out = limb_kernels::lshift(gy + q, y, ny, c);
				efloat_storage::zero_limbs(gy + q + ny, L - q - ny);
				if (q + ny < L) gy[q + ny] = out;   // else out is 0: Y's top lies below X's
			}
		}
		else {
			const uint64_t    sh = d + over - room;   // Y's bits below 2^sh of it fall below the frame
			if (sh >= 64 * uint64_t(ny)) {
				lost = true;   // all of Y, which is nonzero
				efloat_storage::zero_limbs(gy, L);
			}
			else {
				const std::size_t k = static_cast<std::size_t>(sh / 64);
				const unsigned    c = static_cast<unsigned>(sh % 64);
				for (std::size_t i = 0; !lost && i < k; ++i) lost = (y[i] != 0u);
				if (!lost && c != 0u) lost = (y[k] & ((limb_t(1) << c) - 1u)) != 0u;
				const std::size_t m = ny - k;
				if (c == 0u) efloat_storage::copy_limbs(gy, y + k, m);
				else         limb_kernels::rshift(gy, y + k, m, c);
				efloat_storage::zero_limbs(gy + m, L - m);
			}
		}
		g[0] = lost ? 1u : 0u;

		const std::size_t low = 1 + L - nx;   // f's limbs below X
		if (same_sign) {
			efloat_storage::copy_limbs(f, g, low);
			f[L + 1] = limb_kernels::add_n(f + low, x, g + low, nx);
			round_into(f, L + 2, ex + 64, false);
			return;
		}
		// X - Y; a borrow out of the top means Y was the larger: negate the difference
		limb_t borrow = 0;
		for (std::size_t i = 0; i < low; ++i) {
			const limb_t gi = g[i];
			f[i]   = limb_t(0) - gi - borrow;
			borrow = ((gi | borrow) != 0u) ? 1u : 0u;   // 0 - g[i] - borrow borrows unless both are 0
		}
		const limb_t below = borrow;
		borrow = limb_kernels::sub_n(f + low, x, g + low, nx);
		if (limb_kernels::sub_1(f + low, nx, below) != 0u) borrow = 1u;
		bool sign = x_sign;
		if (borrow != 0u) {
			limb_t c = 1;
			for (std::size_t i = 0; i <= L; ++i) {
				const limb_t v = ~f[i] + c;
				c    = (c != 0u && v == 0u) ? 1u : 0u;
				f[i] = v;
			}
			sign = !x_sign;
		}
		_sign = sign;
		round_into(f, L + 1, ex, false);
		if (iszero()) _sign = exact_zero_sum_sign(false, true);   // x - x: +0, -0 rounding toward -inf (#1768)
	}

	// X + Y, or X - Y with subtract, for operands both m limbs long at a precision of 64 m or
	// 64 m - 32 bits, Y below X by d < 64 m bits, and d >= 1 for a difference: the common case,
	// as MPFR's mpfr_add1sp and mpfr_sub1sp take it (#1822, #1826). Y is shifted down into the
	// result's m limbs once; the 64 bits it shifts out below them are kept as a limb, and any
	// below that as a sticky bit. X is added to it, or it is subtracted from X with a borrow out
	// of the low limb: a carry shifts the sum down one bit, and a difference, which a gap of two
	// bits or more leaves with at most one leading zero, is shifted up one. A gap of one bit can
	// cancel any number of them, but then the low limb holds all of Y's bits, and the exact
	// difference rounds through round_into. The result rounds
	// from the low limb and the sticky bit, or at bit 32 of the bottom limb. No frame, and no
	// second pass over the limbs but the one-bit shifts. Past four limbs: add_short below has
	// the one to four
	constexpr void add_full_length(const limb_t* x, const limb_t* y, std::size_t m, int64_t ex, uint64_t d, bool subtract, bool x_sign) {
		// the result's limbs: this value's own when they are neither operand's, as in c = a + b
		// (#1828), else scratch
		scratch_buffer<128> buffer;
		const bool in_place = (_limb.data() != x && _limb.data() != y);
		limb_t* t = in_place ? _limb.resize_for_overwrite(m) : buffer.get(m);
		// a difference one bit apart that cancels a leading bit, which the top limbs decide, is
		// formed as 2 X - Y, twice the difference, in one pass of limb_kernels::twice_sub_n: no
		// shift of Y before and none of the difference after (#1828)
		uint64_t e     = d;   // the shift of Y
		bool     twice = false;
		if (subtract && d == 1u) {
			const limb_t top = x[m - 1] - (y[m - 1] >> 1);   // X > Y / 2
			twice = (top >> 63) == 0u;                       // cancels, whatever borrows from below
			if (twice) e = 0;
		}
		const std::size_t q = static_cast<std::size_t>(e / 64);
		const unsigned    c = static_cast<unsigned>(e % 64);
		// t = Y >> d; o = the 64 bits of Y just below t's last; sticky = whether any below those
		limb_t o      = 0;
		bool   sticky = false;
		if (twice) {
			// t is formed by the subtraction below
		}
		else if (c == 0u) {
			efloat_storage::copy_limbs(t, y + q, m - q);
		}
		else {
			limb_kernels::rshift(t, y + q, m - q, c);
		}
		if (!twice) efloat_storage::zero_limbs(t + (m - q), q);
		if (c == 0u) {
			if (q > 0) o = y[q - 1];
		}
		else {
			o = y[q] << (64u - c);
			if (q > 0) {
				o |= y[q - 1] >> c;
				sticky = (y[q - 1] & ((limb_t(1) << c) - 1u)) != 0u;
			}
		}
		for (std::size_t i = 0; !sticky && i + 1 < q; ++i) sticky = (y[i] != 0u);
		int64_t exponent = ex;
		bool    sign     = x_sign;
		if (!subtract) {
			if (limb_kernels::add_n(t, x, t, m) != 0u) {   // the sum's top bit is 2^(ex + 1): shift it down
				sticky = sticky || ((o & 1u) != 0u);
				o      = (o >> 1) | (t[0] << 63);
				limb_kernels::rshift(t, t, m, 1u);
				t[m - 1] |= limb_t(1) << 63;
				++exponent;
			}
		}
		else {
			// X B - (t B + o + s), s in [0, 1) the sticky part: the low limb borrows unless o and s are 0
			const limb_t borrow = (o != 0u || sticky) ? 1u : 0u;
			o = limb_t(0) - o - (sticky ? 1u : 0u);
			if (twice) {
				// 2 X - Y < B^m, its bits above B^m 0: exact, o = 0 and nothing is sticky, and any
				// number more may cancel
				limb_kernels::twice_sub_n(t, x, y, m);
				--exponent;
				if (t[m - 1] == 0u) [[unlikely]] {
					_sign = sign;
					round_cancelled(t, m, o, ex - 1);
					return;
				}
				const unsigned k = static_cast<unsigned>(std::countl_zero(t[m - 1]));
				if (k != 0u) {
					limb_kernels::lshift(t, t, m, k);
					exponent -= k;
				}
			}
			else {
				limb_kernels::sub_n(t, x, t, m);
				limb_kernels::sub_1(t, m, borrow);   // X > Y: no borrow out of the top
				if ((t[m - 1] >> 62) == 0u && t[m - 1] != 0u) {
					// two to 63 leading bits cancelled, which only a gap of one bit does: the difference
					// is exact in t and o, nothing is sticky, and one shift brings its leading bit to the top
					const unsigned k = static_cast<unsigned>(std::countl_zero(t[m - 1]));
					limb_kernels::lshift(t, t, m, k);
					t[0] |= o >> (64u - k);
					o <<= k;
					exponent -= k;
				}
				else if ((t[m - 1] >> 62) == 0u) [[unlikely]] {
					// a whole limb or more cancelled: the exact difference in t and o rounds as any other
					_sign = sign;
					round_cancelled(t, m, o, ex);
					return;
				}
				else if ((t[m - 1] >> 63) == 0u) {   // one leading bit cancelled: shift up, the low limb's top bit in
					limb_kernels::lshift(t, t, m, 1u);
					t[0] |= o >> 63;
					o <<= 1;   // the bit shifted in below o is the sticky part's, which sticky covers
					--exponent;
				}
			}
		}
		// round at the precision: the low limb's top bit is the guard bit, or bit 31 of t[0]
		const unsigned drop = static_cast<unsigned>(64 * m - 32 * static_cast<std::size_t>(precision_limbs()));   // 0 or 32
		bool guard = false;
		if (drop == 0u) {
			guard  = (o >> 63) != 0u;
			sticky = sticky || ((o << 1) != 0u);
		}
		else {
			guard  = ((t[0] >> 31) & 1u) != 0u;
			sticky = sticky || o != 0u || (t[0] & 0x7FFF'FFFFu) != 0u;
			t[0] &= 0xFFFF'FFFF'0000'0000ull;
		}
		_sign = sign;
		bool power_of_2 = false;   // rounded up out of the top
		if (guard || sticky) {
			raise_flags(ExceptionFlag::Inexact);
			if (rounds_up(rounding_mode(), sign, ((t[0] >> drop) & 1u) != 0u, guard, sticky)) {
				limb_t carry = limb_t(1) << drop;
				for (std::size_t i = 0; carry != 0u && i < m; ++i) {
					t[i] += carry;
					carry = (t[i] < carry) ? 1u : 0u;
				}
				power_of_2 = (carry != 0u);
			}
		}
		if (power_of_2) {
			_limb.assign(1, limb_t(1) << 63);
			++exponent;
		}
		else if (in_place) {
			if (t[0] == 0u) trim_low_limbs(m);
		}
		else {
			efloat_storage::copy_limbs(_limb.resize_for_overwrite(m), t, m);
			if (t[0] == 0u) trim_low_limbs(m);
		}
		_exponent = exponent;
		_state    = FloatingPointState::Normal;
		if (!in_range(exponent)) [[unlikely]] enforce_range();
	}

	// add_full_length for M = m of one to four limbs (#1830), in code whose loops have fixed
	// bounds and unroll: the result's limbs are a local array whose address is never taken, so
	// that they stay in registers, as MPFR's mpfr_add1sp1 to mpfr_sub1sp3 keep theirs, and are
	// stored once, by store_short. A difference that cancels a whole limb or more, which only a
	// gap of one bit gives, goes to add_frame, which forms it exactly and rounds it once. The
	// operands are read from memory and the result written after the last read of them, so x or
	// y may be this value's own limbs.
	template<std::size_t M>
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noinline))
#endif
	constexpr void add_short(const limb_t* x, const limb_t* y, int64_t ex, uint64_t d, bool subtract, bool x_sign) {
		limb_t t[M];
		// a difference one bit apart whose top limbs say it cancels a leading bit is formed as
		// 2 X - Y in one pass (#1828)
		bool twice = false;
		if (subtract && d == 1u) twice = ((x[M - 1] - (y[M - 1] >> 1)) >> 63) == 0u;
		int64_t exponent = ex;
		limb_t  o        = 0;       // the 64 bits below t's last
		bool    sticky   = false;   // whether any bit below those is set
		if (twice) {
			limb_t borrow = 0;
			EFLOAT_UNROLLED_LOOP
			for (std::size_t i = 0; i < M; ++i) {
				const limb_t xi = (x[i] << 1) | ((i > 0) ? (x[i - 1] >> 63) : 0u);
				t[i] = limb_kernels::sub_borrow(xi, y[i], borrow);
			}
			if (t[M - 1] == 0u) [[unlikely]] {   // a limb or more cancelled
				add_frame(x, M, y, M, ex, x_sign, d, false);
				return;
			}
			--exponent;
			const unsigned k = static_cast<unsigned>(std::countl_zero(t[M - 1]));
			if (k != 0u) {
				EFLOAT_UNROLLED_LOOP
				for (std::size_t i = M - 1; i > 0; --i) t[i] = (t[i] << k) | (t[i - 1] >> (64u - k));
				t[0] <<= k;
				exponent -= k;
			}
			store_short<M>(t, 0u, false, exponent, x_sign);   // exact: 2 X - Y fits in M limbs
			return;
		}
		// t = Y >> d; o = the 64 bits of Y just below t's last; sticky = whether any below those
		if (d < 64u) [[likely]] {
			// the common case: a shift within the limbs, nothing sticky; a limb's bits that move to
			// the one below are shifted in two steps, which leaves 0 rather than a shift by 64 for d = 0
			const unsigned c = static_cast<unsigned>(d);
			EFLOAT_UNROLLED_LOOP
			for (std::size_t i = 0; i < M; ++i) t[i] = (y[i] >> c) | ((i + 1 < M) ? ((y[i + 1] << 1) << (63u - c)) : 0u);
			o = (y[0] << 1) << (63u - c);
		}
		else {
			const std::size_t q = static_cast<std::size_t>(d / 64);
			const unsigned    c = static_cast<unsigned>(d % 64);
			EFLOAT_UNROLLED_LOOP
			for (std::size_t i = 0; i < M; ++i) {
				const std::size_t j = i + q;
				const limb_t lo = (j < M) ? y[j] : 0u;
				const limb_t hi = (j + 1 < M) ? y[j + 1] : 0u;
				t[i] = (c == 0u) ? lo : ((lo >> c) | (hi << (64u - c)));
			}
			if (c == 0u) {
				o = y[q - 1];
			}
			else {
				o = (y[q] << (64u - c)) | (y[q - 1] >> c);
				sticky = (y[q - 1] & ((limb_t(1) << c) - 1u)) != 0u;
			}
			for (std::size_t i = 0; !sticky && i + 1 < q; ++i) sticky = (y[i] != 0u);
		}
		if (!subtract) {
			limb_t carry = 0;
			EFLOAT_UNROLLED_LOOP
			for (std::size_t i = 0; i < M; ++i) t[i] = limb_kernels::add_carry(x[i], t[i], carry);
			if (carry != 0u) {   // the sum's top bit is 2^(ex + 1): shift it down
				sticky = sticky || ((o & 1u) != 0u);
				o      = (o >> 1) | (t[0] << 63);
				EFLOAT_UNROLLED_LOOP
				for (std::size_t i = 0; i + 1 < M; ++i) t[i] = (t[i] >> 1) | (t[i + 1] << 63);
				t[M - 1] = (t[M - 1] >> 1) | (limb_t(1) << 63);
				++exponent;
			}
			store_short<M>(t, o, sticky, exponent, x_sign);
			return;
		}
		// X B - (t B + o + s), s in [0, 1) the sticky part: the low limb borrows unless o and s are 0
		limb_t borrow = (o != 0u || sticky) ? 1u : 0u;
		o = limb_t(0) - o - (sticky ? 1u : 0u);
		EFLOAT_UNROLLED_LOOP
		for (std::size_t i = 0; i < M; ++i) t[i] = limb_kernels::sub_borrow(x[i], t[i], borrow);
		if ((t[M - 1] >> 62) == 0u) {
			if (t[M - 1] == 0u) [[unlikely]] {   // a whole limb or more cancelled
				add_frame(x, M, y, M, ex, x_sign, d, false);
				return;
			}
			// two to 63 leading bits cancelled, which only a gap of one bit does: the difference is
			// exact in t and o, nothing is sticky, and one shift brings its leading bit to the top
			const unsigned k = static_cast<unsigned>(std::countl_zero(t[M - 1]));
			EFLOAT_UNROLLED_LOOP
			for (std::size_t i = M - 1; i > 0; --i) t[i] = (t[i] << k) | (t[i - 1] >> (64u - k));
			t[0] = (t[0] << k) | (o >> (64u - k));
			o <<= k;
			exponent -= k;
		}
		else if ((t[M - 1] >> 63) == 0u) {   // one leading bit cancelled: shift up, the low limb's top bit in
			EFLOAT_UNROLLED_LOOP
			for (std::size_t i = M - 1; i > 0; --i) t[i] = (t[i] << 1) | (t[i - 1] >> 63);
			t[0] = (t[0] << 1) | (o >> 63);
			o <<= 1;   // the bit shifted in below o is the sticky part's, which sticky covers
			--exponent;
		}
		store_short<M>(t, o, sticky, exponent, x_sign);
	}

	// rounds the exact difference t[0, m) B + o, its bit 64 (m + 1) - 1 weighing 2^ex, after a
	// cancellation of a limb or more; out of line, as it is rare
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noinline))
#endif
	constexpr void round_cancelled(const limb_t* t, std::size_t m, limb_t o, int64_t ex) {
		scratch_buffer<129> exact;
		limb_t* f = exact.get(m + 1);
		f[0] = o;
		for (std::size_t i = 0; i < m; ++i) f[1 + i] = t[i];
		round_into(f, m + 1, ex, false);
	}

	// the sign of an exact zero sum of addends of the signs a and b (IEEE-754 6.3): their common
	// sign if they agree, else +0, or -0 rounding toward -inf
	static constexpr bool exact_zero_sum_sign(bool a, bool b) noexcept {
		if (a == b) return a;
		return rounding_mode() == RoundingMode::RoundTowardNegative;
	}
	// The thread's rounding mode and status flags, which a constant expression can neither read
	// nor write (#1819): constant evaluation rounds to nearest and raises no flags. The mode must
	// not initialize a const variable of its own: the compiler first tries such an initializer as
	// a constant expression, where this returns RoundToNearest, and keeps it at run time.
	static constexpr RoundingMode rounding_mode() noexcept {
		if (std::is_constant_evaluated()) return RoundingMode::RoundToNearest;
		return efloat_rounding_mode;
	}
	static constexpr void raise_flags(uint32_t flags) noexcept {
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(flags);
	}

	// *this * B for finite nonzero *this and B = (b[0, nb), eb, b_sign), normalized, at the
	// precision already merged into *this; b may be *this's own limbs, and when it is, *this is
	// squared. The significands are the kernels' own limbs (#1820), multiplied by limb_kernels,
	// and the exact product is rounded into _limb once (#1762).
	// Operands of one length from mulhigh_min_limbs up first try the product's high half
	// (#1807): with a zero limb below each, n = m + 1 limbs, limb_kernels::mulhigh_n leaves
	// R = r[n - 1, 2n) below the exact high part by under n units of r[n], 2^(64 + bit_width(n))
	// units of R's last bit, so R is right to 64 m + 62 - bit_width(n) bits, about 55 past a
	// product rounded to the operands' length. Where Ziv's test passes on R, it rounds from R
	// as from the exact product; where it does not, the exact product decides, as before.
	static constexpr size_t mulhigh_min_limbs = 16;   // by measurement: below, the padded short product does not pay
	constexpr void multiply_magnitude(const limb_t* b, std::size_t mb, int64_t eb, bool b_sign) {
		multiply_magnitudes(_limb.data(), _limb.size(), _exponent, _sign, b, mb, eb, b_sign);
	}
	// *this = A B for finite nonzero, normalized A = (a[0, ma), ea, a_sign) and B, at the
	// precision already in *this; a and b may be *this's own limbs, or another value's (#1826).
	// Operands of one to three limbs at the full precision multiply and round in registers.
	constexpr void multiply_magnitudes(const limb_t* a, std::size_t ma, int64_t ea, bool a_sign, const limb_t* b, std::size_t mb, int64_t eb, bool b_sign) {
		const bool        square = (a == b && ma == mb);
		const int64_t     top    = ea + eb + 1;
		const std::size_t target = precision_limbs();
		_sign = a_sign;
		if (ma == mb && ma == (target + 1) / 2 && ma <= 3) {
			switch (ma) {
			case 1:  multiply_short<1>(a, b, top, a_sign != b_sign); break;
			case 2:  multiply_short<2>(a, b, top, a_sign != b_sign); break;
			default: multiply_short<3>(a, b, top, a_sign != b_sign); break;
			}
			return;
		}
		multiply_long(a, ma, a_sign, b, mb, b_sign, top, square);
	}
	// multiply_magnitudes past the short products; out of line, so that its scratch frame is not
	// set up for them
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noinline))
#endif
	constexpr void multiply_long(const limb_t* a, std::size_t ma, bool a_sign, const limb_t* b, std::size_t mb, bool b_sign, int64_t top, bool square) {
		const std::size_t mt     = ma + mb;
		const std::size_t target = precision_limbs();
		if (ma == mb && ma >= mulhigh_min_limbs && 32 * target + 24 <= 64 * ma + 62 - static_cast<size_t>(std::bit_width(ma + 1))) {
			const size_t n = ma + 1;
			scratch64 high;
			limb_t* qa = high.get(4 * n + limb_kernels::scratch_limbs(n));
			limb_t* qb = qa + n;
			limb_t* r  = qb + n;
			qa[0] = 0u;
			for (std::size_t i = 0; i < ma; ++i) qa[1 + i] = a[i];
			if (!square) {
				qb[0] = 0u;
				for (std::size_t i = 0; i < mb; ++i) qb[1 + i] = b[i];
			}
			limb_kernels::mulhigh_n(r, qa, square ? qa : qb, n, r + 2 * n, square);
			const limb_t*  f       = r + n - 1;   // R, n + 1 limbs; its top bit weighs 2^top
			const unsigned lead    = static_cast<unsigned>(64 * (n + 1) - 1 - static_cast<size_t>(std::countl_zero(f[n])));
			const unsigned correct = lead - 64u - static_cast<unsigned>(std::bit_width(n));
			if (limb_kernels::can_round(f, n + 1, static_cast<unsigned>(32 * target), correct)) {
				// R and a sticky bit: the exact product lies past R, and Ziv's test says on which side
				// of every boundary
				_sign = (a_sign != b_sign);
				round_into(f, n + 1, top, true);
				return;
			}
		}
		scratch64 buffer;
		limb_t* prod = buffer.get(mt + limb_kernels::scratch_limbs(std::max(ma, mb)));
		limb_t* work = prod + mt;
		if (square) limb_kernels::mul_n(prod, a, a, ma, work, true);
		else        limb_kernels::mul(prod, a, ma, b, mb, work);
		_sign = (a_sign != b_sign);
		round_into(prod, mt, top, false);   // the exact product
	}

	// A B for operands of M = 1 to 3 limbs at a precision of 64 M or 64 M - 32 bits (#1826): the
	// 2 M limbs of the product by schoolbook on 128-bit partial products, normalized by at most
	// one bit, and rounded from them; the product's bit 128 M - 1 weighs 2^top
	template<std::size_t M>
	constexpr void multiply_short(const limb_t* a, const limb_t* b, int64_t top, bool sign) {
		limb_t p[2 * M] = {};
		for (std::size_t i = 0; i < M; ++i) {
			limb_t carry = 0;
			for (std::size_t j = 0; j < M; ++j) {
				limb_t hi = 0, lo = 0;
				limb_kernels::umul(a[j], b[i], hi, lo);
				lo += carry;
				hi += (lo < carry) ? 1u : 0u;
				const limb_t r = p[i + j] + lo;
				hi += (r < lo) ? 1u : 0u;
				p[i + j] = r;
				carry = hi;
			}
			p[i + M] = carry;
		}
		int64_t exponent = top;
		if ((p[2 * M - 1] >> 63) == 0u) {   // the product of two normalized values lacks at most one bit
			for (std::size_t i = 2 * M; i-- > 1;) p[i] = (p[i] << 1) | (p[i - 1] >> 63);
			p[0] <<= 1;
			--exponent;
		}
		// the top M limbs are kept, the limb below them gives the guard bit
		bool rest = false;
		for (std::size_t i = 0; !rest && i + 1 < M; ++i) rest = (p[i] != 0u);
		store_short<M>(p + M, p[M - 1], rest, exponent, sign);
	}

	// Stores T + (below + s) / B rounded to the precision, for T = t[0, M) with its top bit set
	// and its leading bit weighing 2^exponent, below the limb under T, and s in (0, 1) when rest,
	// else 0; the precision is 64 M or 64 M - 32 bits. The rounding of the short products,
	// quotients and roots (#1826, #1828), in code whose loops have fixed bounds and unroll.
	// Inlined into each: out of line, as g++ left it, the caller's limbs went through memory
	// and a 64-bit product took 9 ns instead of 7.
	template<std::size_t M>
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((always_inline))
#endif
	inline constexpr void store_short(limb_t* t, limb_t below, bool rest, int64_t exponent, bool sign) {
		const unsigned drop = static_cast<unsigned>(64 * M - 32 * static_cast<std::size_t>(precision_limbs()));   // 0 or 32
		bool guard = false, sticky = false;
		if (drop == 0u) {
			guard  = (below >> 63) != 0u;
			sticky = (below << 1) != 0u || rest;
		}
		else {
			guard  = ((t[0] >> 31) & 1u) != 0u;
			sticky = (t[0] & 0x7FFF'FFFFu) != 0u || below != 0u || rest;
			t[0] &= 0xFFFF'FFFF'0000'0000ull;
		}
		_sign = sign;
		bool power_of_2 = false;
		if (guard || sticky) {
			raise_flags(ExceptionFlag::Inexact);
			if (rounds_up(rounding_mode(), sign, ((t[0] >> drop) & 1u) != 0u, guard, sticky)) {
				// all M limbs, without an exit once the carry is spent: a loop the compiler unrolls
				// whole, which keeps t in registers (#1830)
				limb_t carry = limb_t(1) << drop;
				EFLOAT_UNROLLED_LOOP
				for (std::size_t i = 0; i < M; ++i) {
					t[i] += carry;
					carry = (t[i] < carry) ? 1u : 0u;
				}
				power_of_2 = (carry != 0u);
			}
		}
		if (power_of_2) {
			_limb.assign(1, limb_t(1) << 63);
			++exponent;
		}
		else {
			limb_t* w = _limb.resize_for_overwrite(M);
			EFLOAT_UNROLLED_LOOP
			for (std::size_t k = 0; k < M; ++k) w[k] = t[k];
			if (t[0] == 0u) trim_low_limbs(M);
		}
		_exponent = exponent;
		_state    = FloatingPointState::Normal;
		if (!in_range(exponent)) [[unlikely]] enforce_range();
	}

	// Rounds R + s, R the integer r[0, n) of 64-bit limbs least significant first whose bit
	// 64 n - 1 weighs 2^top and s in (0, 1) when sticky, else 0, to the precision in the current
	// mode and with *this's sign, and stores it as normalize() leaves a value: the leading bit at
	// the top of the top limb, at most 32 precision_limbs() bits, no zero limbs at the bottom.
	// One pass, and _limb never grows past the precision. r must not be _limb's.
	constexpr void round_into(const limb_t* r, std::size_t n, int64_t top, bool sticky) {
		std::size_t hi = n;
		while (hi > 0 && r[hi - 1] == 0u) --hi;
		const uint64_t tb = 32u * uint64_t(precision_limbs());
		if (hi == 0) [[unlikely]] {
			round_short(r, n, top, sticky);
			return;
		}
		const unsigned sh   = static_cast<unsigned>(std::countl_zero(r[hi - 1]));
		const uint64_t lead = 64 * uint64_t(hi) - 1 - sh;   // R's leading bit
		if (lead < tb) [[unlikely]] {
			round_short(r, n, top, sticky);
			return;
		}
		// the kept bits are the tb from the leading bit down; the guard bit is the next
		const uint64_t    g     = lead - tb;
		const std::size_t gw    = static_cast<std::size_t>(g / 64);
		const unsigned    gb    = static_cast<unsigned>(g % 64);
		const bool        guard = ((r[gw] >> gb) & 1u) != 0u;
		bool below = sticky || (r[gw] & ((limb_t(1) << gb) - 1u)) != 0u;
		for (std::size_t i = 0; !below && i < gw; ++i) below = (r[i] != 0u);
		const uint64_t k   = g + 1;
		const bool     lsb = ((r[k / 64] >> (k % 64)) & 1u) != 0u;
		// the kept bits, shifted so that the leading bit lands at the top of the top limb
		std::size_t       mo   = static_cast<std::size_t>((tb + 63) / 64);
		limb_t*           w    = _limb.resize_for_overwrite(mo);
		const std::size_t base = hi - mo;   // r[base, hi) shifted up by sh, with the top bits of r[base - 1]
		if (sh == 0u) {
			efloat_storage::copy_limbs(w, r + base, mo);
		}
		else {
			limb_kernels::lshift(w, r + base, mo, sh);   // shifts out r[hi - 1]'s leading zeros
			if (base > 0) w[0] |= r[base - 1] >> (64u - sh);
		}
		const unsigned drop = static_cast<unsigned>(64 * mo - tb);   // bits below the precision: 0 or 32
		if (drop != 0u) w[0] &= ~((limb_t(1) << drop) - 1u);
		int64_t exponent = top - static_cast<int64_t>(64 * uint64_t(n) - 1 - lead);
		if (guard || below) {
			raise_flags(ExceptionFlag::Inexact);
			if (rounds_up(rounding_mode(), _sign, lsb, guard, below) && limb_kernels::add_1(w, mo, limb_t(1) << drop) != 0u) {
				w[0] = limb_t(1) << 63;   // carried out of the top: the value is a power of 2
				_limb.resize_for_overwrite(1);
				mo = 1;
				++exponent;
			}
		}
		if (w[0] == 0u) trim_low_limbs(mo);
		_exponent = exponent;
		_state    = FloatingPointState::Normal;
		if (!in_range(exponent)) [[unlikely]] enforce_range();
	}

	// round_into for an R no longer than the precision: kept whole, or, with a sticky bit, R and
	// a 32-bit limb of 1 below it, as the 32-bit frame always held them; and a zero R
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noinline))
#endif
	constexpr void round_short(const limb_t* r, std::size_t n, int64_t top, bool sticky) {
		if (sticky) {
			scratch_buffer<64> frame;
			limb_t* fr = frame.get(n + 1);
			fr[0] = limb_t(1) << 32;
			efloat_storage::copy_limbs(fr + 1, r, n);
			round_into(fr, n + 1, top, false);
			return;
		}
		std::size_t hi = n;
		while (hi > 0 && r[hi - 1] == 0u) --hi;
		if (hi == 0) {
			setzero();
			return;
		}
		const unsigned sh = static_cast<unsigned>(std::countl_zero(r[hi - 1]));
		limb_t* w = _limb.resize_for_overwrite(hi);
		if (sh == 0u) efloat_storage::copy_limbs(w, r, hi);
		else          limb_kernels::lshift(w, r, hi, sh);
		if (w[0] == 0u) trim_low_limbs(hi);
		const int64_t exponent = top - 64 * static_cast<int64_t>(n - hi) - sh;
		_exponent = exponent;
		_state    = FloatingPointState::Normal;
		if (!in_range(exponent)) [[unlikely]] enforce_range();
	}

	// rounds a normalized significand to the precision in place, as round_into rounds R
	constexpr void round_to_precision() {
		std::size_t   m = _limb.size();
		const int64_t g = 64 * static_cast<int64_t>(m) - 1 - 32 * static_cast<int64_t>(precision_limbs());   // the guard bit
		if (g < 0) return;
		limb_t* w = _limb.data();
		const int64_t last = std::countr_zero(w[0]);   // the last 1 bit: w[0] != 0
		if (last > g) return;   // nothing past the precision
		const bool guard  = ((w[g / 64] >> (g % 64)) & 1u) != 0u;
		const bool sticky = last < g;
		raise_flags(ExceptionFlag::Inexact);
		const int64_t     k   = g + 1;   // the last bit kept
		const std::size_t lw  = static_cast<std::size_t>(k / 64);
		const unsigned    c   = static_cast<unsigned>(k % 64);
		const bool        lsb = ((w[lw] >> c) & 1u) != 0u;
		const bool        up  = rounds_up(rounding_mode(), _sign, lsb, guard, sticky);
		m -= lw;
		for (std::size_t i = 0; i < m; ++i) w[i] = w[i + lw];
		if (c != 0u) w[0] &= ~((limb_t(1) << c) - 1u);
		if (up && limb_kernels::add_1(w, m, limb_t(1) << c) != 0u) {
			w[0] = limb_t(1) << 63;   // carried out of the top: the value is a power of 2
			m = 1;
			++_exponent;
		}
		trim_low_limbs(m);
	}

	// the m limbs in use, less the zero limbs at the bottom of a nonzero significand
	constexpr void trim_low_limbs(std::size_t m) noexcept {
		limb_t* w = _limb.data();
		std::size_t z = 0;
		while (z + 1 < m && w[z] == 0u) ++z;
		if (z > 0) {
			m -= z;
			for (std::size_t i = 0; i < m; ++i) w[i] = w[i + z];
		}
		_limb.resize_for_overwrite(m);
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

	// scratch limbs on the stack, or from the thread's block cache when an operation needs more
	template<size_t N>
	using scratch_buffer = efloat_storage::scratch_limbs<N>;
	using scratch64 = scratch_buffer<768>;

	// the 32-bit limbs v[0, n), most significant first, as the 64-bit limbs that hold them: an
	// odd count padded with zeros at the bottom
	constexpr void import_limbs32(const std::uint32_t* v, std::size_t n) {
		const std::size_t m = (n + 1) / 2;
		limb_t* w = _limb.resize_for_overwrite(m);
		for (std::size_t j = 0; j < m; ++j) {
			const std::size_t hi = 2 * m - 2 - 2 * j;
			w[j] = (limb_t(v[hi]) << 32) | ((hi + 1 < n) ? v[hi + 1] : 0u);
		}
	}

	// a nonzero native integer as one normalized limb, as efloat(v) would hold it
	struct native_limbs {
		limb_t  limb[1];
		size_t  n;
		int64_t exponent;
		bool    sign;
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
		const int lz = std::countl_zero(magnitude);
		r.limb[0]  = magnitude << lz;
		r.n        = 1;
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

	// *this /= d for a finite nonzero *this and a d of up to 64 bits that is not a power of 2,
	// rounded once in the current mode (#1761, #1811): the quotient as operator/= forms it,
	// without building an efloat for d or a general long division. The significand P, its m
	// 64-bit limbs given k zero limbs below so that the quotient carries the precision and a guard
	// bit, is divided by d normalized to its top bit, a limb at a time with Moller and Granlund's
	// reciprocal (limb_kernels::divrem_1); the remainder is the sticky bit, and the quotient rounds
	// once. Exact, so the result is the one the general path returns.
	constexpr void divide_by_limb(uint64_t d, bool negative) {
		const unsigned    target = precision_limbs();
		const limb_t*     a      = _limb.data();
		const std::size_t m      = _limb.size();
		// Q = floor(P B^k / d) >= 2^(64 (m + k) - 1 - bit_width(d)): k limbs below give it the
		// 32 target + 2 bits the rounding reads, the guard bit among them; the remainder is exact
		const int64_t  need   = 32 * static_cast<int64_t>(target) + 3 + static_cast<int64_t>(std::bit_width(d)) - 64 * static_cast<int64_t>(m);
		const size_t   k      = (need > 0) ? static_cast<size_t>((need + 63) / 64) : 0u;
		const size_t   nn     = m + k + 1;   // a top limb for the normalizing shift
		const unsigned sh     = static_cast<unsigned>(std::countl_zero(d));
		const limb_t   dn     = d << sh;
		scratch64 buffer;
		limb_t* num = buffer.get(2 * nn);
		limb_t* q   = num + nn;
		// P B^k 2^sh, shifted up by d's leading zeros as d is
		for (size_t i = 0; i < k; ++i) num[i] = 0u;
		if (sh == 0u) {
			for (size_t j = 0; j < m; ++j) num[k + j] = a[j];
			num[nn - 1] = 0u;
		}
		else {
			num[nn - 1] = limb_kernels::lshift(num + k, a, m, sh);
		}
		q[nn - 1] = limb_kernels::divrem_1(q, num, nn, dn, limb_kernels::invert_limb(dn));
		const bool sticky = (num[0] != 0u);
		// the quotient weighs 2^(scale - 64 (m + k) + 1) a unit, its limb nn - 1 included
		const int64_t top = _exponent - 64 * static_cast<int64_t>(m + k) + 1 + 64 * static_cast<int64_t>(nn) - 1;
		_sign = (_sign != negative);
		round_into(q, nn, top, sticky);
	}
	// *this = A / d 2^-shift for a finite nonzero, normalized A = (a[0, m), ea, a_sign), at the
	// precision already in *this: divide_by_limb's quotient, its exponent moved by shift before it
	// rounds, so that a divisor of one limb, d 2^(eb - 63), divides the same way (#1826). A quotient
	// of one or two limbs is the common case here, and two changes pay for it, where they cost the
	// integer divisors of divide_by_limb more than they save (a/7 5-15% slower from 64 to 384 bits)
	constexpr void divide_limbs_by(const limb_t* a, std::size_t m, int64_t ea, bool a_sign, uint64_t d, bool negative, int64_t shift) {
		const unsigned    target = precision_limbs();
		// Q = floor(P B^k / d) >= 2^(64 (m + k) - 1 - bit_width(d)): k limbs below give it the
		// 32 target + 2 bits the rounding reads, the guard bit among them, and the remainder is the
		// sticky bit. Where a limb fewer gives it the 32 target bits alone, the remainder R decides
		// the rest instead, as MPFR's mpfr_div_1 does (#1826): the next bit is whether 2 R >= d,
		// and the sticky bit whether anything remains past it. A 64-bit quotient by a one-limb
		// divisor is then one 2-by-1 step, where it was three; elsewhere the limb is kept, which
		// costs less than the bit.
		const int64_t  base   = static_cast<int64_t>(std::bit_width(d)) - 64 * static_cast<int64_t>(m);
		const int64_t  wide   = 32 * static_cast<int64_t>(target) + 3 + base;
		const int64_t  tight  = 32 * static_cast<int64_t>(target) + base;
		const size_t   k_wide  = (wide > 0) ? static_cast<size_t>((wide + 63) / 64) : 0u;
		const size_t   k_tight = (tight > 0) ? static_cast<size_t>((tight + 63) / 64) : 0u;
		const bool     by_bit = (k_tight < k_wide);
		const size_t   k      = by_bit ? k_tight : k_wide;
		const unsigned sh     = static_cast<unsigned>(std::countl_zero(d));
		const size_t   nn     = m + k + ((sh != 0u) ? 1u : 0u);   // and a top limb for a normalizing shift
		const limb_t   dn     = d << sh;
		scratch64 buffer;
		limb_t* num = buffer.get(2 * nn + 1);
		limb_t* q   = num + nn;   // q[0] the bit from the remainder, q[1, nn + 1) the quotient
		// P B^k 2^sh, shifted up by d's leading zeros as d is
		for (size_t i = 0; i < k; ++i) num[i] = 0u;
		if (sh == 0u) {
			for (size_t j = 0; j < m; ++j) num[k + j] = a[j];   // a normalized d: no shift, and no top limb
		}
		else {
			num[nn - 1] = limb_kernels::lshift(num + k, a, m, sh);
		}
		// a few limbs divide faster in hardware than through a reciprocal
		q[nn] = (limb_kernels::hardware_division && nn <= 3) ? limb_kernels::divrem_1_short(q + 1, num, nn, dn)
		                                                     : limb_kernels::divrem_1(q + 1, num, nn, dn, limb_kernels::invert_limb(dn));
		// the quotient weighs 2^(scale - 64 (m + k) + 1) a unit, its limb nn - 1 included
		const int64_t top = ea - 64 * static_cast<int64_t>(m + k) + 1 + 64 * static_cast<int64_t>(nn) - 1 + shift;
		_sign = (a_sign != negative);
		const limb_t r = num[0];   // R 2^sh, against dn = d 2^sh
		if (!by_bit) {
			round_into(q + 1, nn, top, r != 0u);
			return;
		}
		const bool half = (r >= dn - r);   // 2 R >= d: the next bit
		const bool rest = half ? (r != dn - r) : (r != 0u);
		q[0] = half ? (limb_t(1) << 63) : 0u;
		round_into(q, nn + 1, top, rest);
	}

	// *this = *this / b for finite nonzero operands, rounded once in the current mode: the
	// significands as integers A and B, both normalized, Q = floor(A 2^shift / B) with
	// 32 (precision + 2) bits, and the remainder for the sticky bit; a dividend longer than the
	// quotient needs has its low bits dropped into the sticky bit. A divisor such as 7 is one
	// limb. limb_kernels divides on 64-bit limbs, schoolbook with a 3-by-2 reciprocal below its
	// threshold and divide-and-conquer above it, which costs a few multiplications (#1763).
	// From the divide-and-conquer threshold up, the quotient comes first from limb_kernels::div_appr
	// (#1811), without the remainders of its low halves: the exact floor or above it by at most
	// divappr_error units, against 64 bits past the precision, so Ziv's test on it decides the
	// rounding wherever it passes. Where it does not -- a quotient next to a rounding boundary,
	// and every exact one, whose low bits are zero -- the exact quotient and remainder do.
	constexpr void divide_magnitude(const limb_t* b, std::size_t dn, int64_t eb, bool b_sign) {
		divide_magnitudes(_limb.data(), _limb.size(), _exponent, _sign, b, dn, eb, b_sign);
	}
	// *this = A / B for finite nonzero, normalized A = (a[0, na), ea, a_sign) and B, at the
	// precision already in *this; a and b may be *this's own limbs, or another value's (#1826).
	// A divisor of one limb divides as an integer divisor does, a limb at a time.
	constexpr void divide_magnitudes(const limb_t* a, std::size_t na, int64_t ea, bool a_sign, const limb_t* b, std::size_t dn, int64_t eb, bool b_sign) {
		const std::size_t target = precision_limbs();
		if (target <= 4 && 2 * std::max(na, dn) <= target + 1) {   // operands of at most M = (target + 1) / 2 limbs
			if (target <= 2) divide_short<1>(a, na, ea, b, dn, eb, a_sign != b_sign);
			else             divide_short<2>(a, na, ea, b, dn, eb, a_sign != b_sign);
			return;
		}
		if (dn == 1) {   // B = b[0] 2^(eb - 63): A / B = (A / b[0]) 2^(63 - eb)
			divide_limbs_by(a, na, ea, a_sign, b[0], b_sign, 63 - eb);
			return;
		}
		divide_long(a, na, ea, a_sign, b, dn, eb, b_sign);
	}
	// A / D for operands of at most M = 1 or 2 limbs at a precision of 64 M or 64 M - 32 bits
	// (#1828), as MPFR's mpfr_div_1 and mpfr_div_2 do it: A and D as integers of M limbs, their top
	// bits set, Q = floor(A 2^(64 M) / D) of M limbs and a top bit, and the remainder R, in
	// fixed-size code without scratch frames. A quotient of 64 M + 1 bits gives its own guard bit
	// and R the sticky bit; one of 64 M bits has the guard bit from R, whether 2 R >= D, and the
	// sticky bit from whether R is 0 or D / 2, as divide_exact_short.
	template<std::size_t M>
	constexpr void divide_short(const limb_t* a, std::size_t na, int64_t ea, const limb_t* b, std::size_t dn, int64_t eb, bool sign) {
		// the operands at the top of their arrays, limb by limb from the top: fixed indices, where
		// a loop of na or dn limbs became a call to memcpy (#1830)
		limb_t np[2 * M] = {};
		limb_t dp[M]     = {};
		EFLOAT_UNROLLED_LOOP
		for (std::size_t k = 0; k < M; ++k) {
			np[2 * M - 1 - k] = (k < na) ? a[na - 1 - k] : 0u;
			dp[M - 1 - k]     = (k < dn) ? b[dn - 1 - k] : 0u;
		}
		limb_t q[M + 1] = {};   // q[M] the top bit
		if constexpr (M == 1) {
			limb_t x = np[1];
			if (x >= dp[0]) { x -= dp[0]; q[1] = 1u; }
			limb_kernels::udiv_qrnnd(q[0], np[0], x, 0u, dp[0]);
		}
		else if constexpr (limb_kernels::hardware_division) {
			q[M] = limb_kernels::sb_div_qr_short(q, np, 2 * M, dp, M);   // as div_qr would, without its dispatch
		}
		else {
			limb_t work[limb_kernels::div_scratch_limbs(M)];
			q[M] = limb_kernels::div_qr(q, np, 2 * M, dp, M, work);
		}
		const limb_t* r = np;   // R = r[0, M) < D
		bool r_zero = true;
		for (std::size_t i = 0; i < M; ++i) r_zero = r_zero && (r[i] == 0u);
		limb_t t[M] = {};
		limb_t below = 0;
		bool   rest  = false;
		int64_t exponent = ea - eb;
		if (q[M] != 0u) {   // Q has 64 M + 1 bits: its last is the guard bit
			for (std::size_t i = 0; i < M; ++i) t[i] = (q[i] >> 1) | (q[i + 1] << 63);
			below = q[0] << 63;
			rest  = !r_zero;
		}
		else {
			for (std::size_t i = 0; i < M; ++i) t[i] = q[i];
			--exponent;
			// 2 R >= D: R >= D - R, which does not borrow, R < D
			limb_t h[M] = {};
			limb_t borrow = 0;
			for (std::size_t i = 0; i < M; ++i) {
				const limb_t x = dp[i] - r[i];
				const limb_t y = x - borrow;
				borrow = ((dp[i] < r[i]) || (x < borrow)) ? 1u : 0u;
				h[i] = y;
			}
			int cmp = 0;
			for (std::size_t i = M; cmp == 0 && i-- > 0;) cmp = (r[i] > h[i]) ? 1 : (r[i] < h[i]) ? -1 : 0;
			below = (cmp >= 0) ? (limb_t(1) << 63) : 0u;
			rest  = (cmp >= 0) ? (cmp != 0) : !r_zero;
		}
		store_short<M>(t, below, rest, exponent, sign);
	}

	// divide_magnitudes for a divisor of two limbs or more; out of line, so that its scratch
	// frame is not set up for a divisor of one
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noinline))
#endif
	constexpr void divide_long(const limb_t* a, std::size_t na, int64_t ea, bool a_sign, const limb_t* b, std::size_t dn, int64_t eb, bool b_sign) {
		const unsigned    target = precision_limbs();
		const int64_t     qbits  = 32 * (static_cast<int64_t>(target) + 2);   // Q < 2^qbits, its leading bit at 2^(qbits - 1) or 2^(qbits - 2)

		// a / b = (A / B) 2^(scale(a) - scale(b) - 64 (na - dn)); N = A 2^shift
		const int64_t shift  = qbits - 1 - 64 * (static_cast<int64_t>(na) - static_cast<int64_t>(dn));
		const int64_t bits_n = 64 * static_cast<int64_t>(na) + shift;   // N < 2^bits_n
		size_t nn = (bits_n > 0) ? static_cast<size_t>((bits_n + 63) / 64) : 0u;
		if (nn < dn) nn = dn;
		const size_t qn = nn - dn + 1;                                  // with the top limb div_qr returns
		const size_t ql = static_cast<size_t>((qbits + 63) / 64);       // the limbs the quotient's bits fill
		// without an approximate quotient, the exact one need only carry the precision: its
		// remainder decides the rest (#1826), when no bit of the dividend is lost to the sticky bit
		if (limb_kernels::divappr_error(nn, dn) == 0u && 32 * static_cast<int64_t>(target) >= 64 * (static_cast<int64_t>(na) - static_cast<int64_t>(dn))) {
			divide_exact_short(a, na, ea, a_sign, b, dn, eb, b_sign);
			return;
		}

		scratch_buffer<1024> buffer;
		const size_t qlen = std::max(qn, ql);
		limb_t* num  = buffer.get(2 * nn + qlen + limb_kernels::div_scratch_limbs(dn));
		limb_t* keep = num + nn;   // N again, for the exact quotient after an approximate one
		limb_t* q    = keep + nn;
		limb_t* work = q + qlen;
		// N = A 2^shift: shifted up, as it is for a dividend no longer than the quotient needs, or
		// with the bits below the quotient's last dropped into the sticky bit
		bool sticky = false;
		if (shift >= 0 && static_cast<std::size_t>(shift / 64) + na <= nn) {
			const std::size_t q = static_cast<std::size_t>(shift / 64);
			const unsigned    c = static_cast<unsigned>(shift % 64);
			efloat_storage::zero_limbs(num, q);
			std::size_t used = q + na;
			if (c == 0u) {
				efloat_storage::copy_limbs(num + q, a, na);
			}
			else {
				const limb_t out = limb_kernels::lshift(num + q, a, na, c);
				if (used < nn) num[used++] = out;   // else out is 0: N < 2^bits_n
			}
			efloat_storage::zero_limbs(num + used, nn - used);
		}
		else {
			sticky = limb_kernels::load_window(num, nn, a, na, shift);
		}
		// the quotient rounds once from its 64-bit limbs, the bit qbits - 1 weighing
		// 2^(scale(a) - scale(b)), with the sticky bit below it
		const int64_t qtop = ea - eb + 64 * static_cast<int64_t>(qlen) - qbits;
		const bool    sign = (a_sign != b_sign);
		auto round_from = [&](bool sticky_bit) { _sign = sign; round_into(q, qlen, qtop, sticky_bit); };
		const size_t error = limb_kernels::divappr_error(nn, dn);
		if (error != 0u) {
			for (size_t k = 0; k < nn; ++k) keep[k] = num[k];
			for (size_t k = 0; k < qlen; ++k) q[k] = 0u;
			q[nn - dn] = limb_kernels::div_appr(q, num, nn, b, dn, work);
			// the quotient must stay below 2^qbits, its leading bit within 2 of the top
			bool fits = true;
			for (size_t k = static_cast<size_t>(qbits / 64); fits && k < qlen; ++k) {
				const unsigned from = (k == static_cast<size_t>(qbits / 64)) ? static_cast<unsigned>(qbits % 64) : 0u;
				fits = (from == 64u) || ((q[k] >> from) == 0u);
			}
			if (fits) {
				size_t top = qlen;
				while (top > 0 && q[top - 1] == 0u) --top;
				const unsigned lead_bit = static_cast<unsigned>(64 * top - 1 - static_cast<size_t>(std::countl_zero(q[top - 1])));
				const unsigned lost     = static_cast<unsigned>(std::bit_width(error + 1));
				if (lead_bit > lost && limb_kernels::can_round(q, qlen, 32 * target, lead_bit - lost)) {
					round_from(true);
					return;
				}
			}
			for (size_t k = 0; k < nn; ++k) num[k] = keep[k];
		}
		for (size_t k = 0; k < qlen; ++k) q[k] = 0u;
		q[nn - dn] = limb_kernels::div_qr(q, num, nn, b, dn, work);
		for (size_t k = 0; !sticky && k < dn; ++k) sticky = (num[k] != 0u);
		round_from(sticky);
	}

	// A / B as divide_long forms it without an approximate quotient, the exact quotient Q of at
	// least 32 precision_limbs() bits and no more, N = A 2^shift with shift >= 0 so that no bit of
	// A is lost: Q + R / B is the exact quotient, and the remainder R gives the bit after Q's last,
	// whether 2 R >= B, and the sticky bit, whether R is not 0 or B / 2. It is the bit and the
	// sticky bit two limbs of quotient used to give, at one or two division steps fewer.
	constexpr void divide_exact_short(const limb_t* a, std::size_t na, int64_t ea, bool a_sign, const limb_t* b, std::size_t dn, int64_t eb, bool b_sign) {
		const unsigned target = precision_limbs();
		const int64_t  qbits  = 32 * static_cast<int64_t>(target) + 1;   // Q < 2^qbits, its leading bit at 2^(qbits - 1) or 2^(qbits - 2)
		const int64_t  shift  = qbits - 1 - 64 * (static_cast<int64_t>(na) - static_cast<int64_t>(dn));   // >= 0
		const int64_t  bits_n = 64 * static_cast<int64_t>(na) + shift;
		const size_t   nn     = std::max<size_t>(dn, static_cast<size_t>((bits_n + 63) / 64));
		const size_t   qn     = nn - dn + 1;
		scratch_buffer<256> buffer;
		limb_t* num  = buffer.get(nn + qn + 1 + dn + limb_kernels::div_scratch_limbs(dn));
		limb_t* q    = num + nn;   // q[0] the bit from the remainder, q[1, qn + 1) the quotient
		limb_t* half = q + qn + 1; // B - R
		limb_t* work = half + dn;
		const std::size_t w = static_cast<std::size_t>(shift / 64);
		const unsigned    c = static_cast<unsigned>(shift % 64);
		efloat_storage::zero_limbs(num, w);
		std::size_t used = w + na;
		if (c == 0u) {
			efloat_storage::copy_limbs(num + w, a, na);
		}
		else {
			const limb_t out = limb_kernels::lshift(num + w, a, na, c);
			if (used < nn) num[used++] = out;   // else out is 0: N < 2^bits_n
		}
		efloat_storage::zero_limbs(num + used, nn - used);
		q[qn] = limb_kernels::div_qr(q + 1, num, nn, b, dn, work);
		// R = num[0, dn) < B: the next bit is whether R >= B - R
		limb_kernels::sub_n(half, b, num, dn);
		int cmp = 0;
		for (std::size_t i = dn; cmp == 0 && i-- > 0;) cmp = (num[i] > half[i]) ? 1 : (num[i] < half[i]) ? -1 : 0;
		bool zero = true;
		for (std::size_t i = 0; zero && i < dn; ++i) zero = (num[i] == 0u);
		q[0] = (cmp >= 0) ? (limb_t(1) << 63) : 0u;
		const bool rest = (cmp >= 0) ? (cmp != 0) : !zero;
		// Q's bit qbits - 1 weighs 2^(scale(a) - scale(b)); the frame's top is Q's limb qn - 1
		const int64_t qtop = ea - eb + 64 * static_cast<int64_t>(qn) - qbits;
		_sign = (a_sign != b_sign);
		round_into(q, qn + 1, qtop, rest);
	}

	// convert arithmetic types into an elastic floating-point
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	constexpr efloat& convert_signed(SignedInt v) noexcept {
		clear();
		if (0 == v) {
			setzero();
			return *this;
		}
		bool neg = (v < 0);
		uint64_t magnitude = neg
			? (0ull - static_cast<uint64_t>(static_cast<int64_t>(v)))
			: static_cast<uint64_t>(v);

		_state = FloatingPointState::Normal;
		_sign = neg;
		_limb.assign(1, magnitude);
		_exponent = 63;
		normalize_native();
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	constexpr efloat& convert_unsigned(UnsignedInt v) noexcept {
		clear();
		if (0 == v) {
			setzero();
			return *this;
		}
		uint64_t magnitude = static_cast<uint64_t>(v);
		_state = FloatingPointState::Normal;
		_sign = false;
		_limb.assign(1, magnitude);
		_exponent = 63;
		normalize_native();
		return *this;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	constexpr efloat& convert_ieee754(Real rhs) noexcept {
		// taken apart by its fields (extractFields), which a constant expression can read
		bool     s{ false };
		uint64_t raw_exponent{ 0 }, fraction{ 0 }, upper{ 0 };
		extractFields(rhs, s, raw_exponent, fraction, upper);
		using parameter = ieee754_parameter<Real>;
		clear();
		_sign = s;
		if (raw_exponent == parameter::eallset) {
			if (fraction == 0) {
				_state = FloatingPointState::Infinite;
			}
			else {
				// x86 specific: the top bit of the significand = 1 for quiet, 0 for signaling
				int nan_type{ NAN_TYPE_NEITHER };
				checkNaN(rhs, nan_type);
				_state = (nan_type == NAN_TYPE_QUIET) ? FloatingPointState::QuietNaN : FloatingPointState::SignalingNaN;
			}
			_exponent = 0;
			return *this;   // stay limbless
		}
		if (raw_exponent == 0 && fraction == 0) {
			_state = FloatingPointState::Zero;
			_exponent = 0;
			return *this;   // stay limbless
		}
		// a normal value is (2^fbits + fraction) 2^(raw - bias - fbits), a subnormal one
		// fraction 2^(1 - bias - fbits): the significand moves to the top of a 64-bit limb
		const uint64_t significand = (raw_exponent != 0) ? (fraction | (uint64_t(1) << parameter::fbits)) : fraction;
		const int      width       = static_cast<int>(std::bit_width(significand));
		const int64_t  lsb_weight  = ((raw_exponent != 0) ? static_cast<int64_t>(raw_exponent) : 1) - parameter::bias - parameter::fbits;
		_exponent = lsb_weight + width - 1;
		_limb.assign(1, significand << (64 - width));
		normalize_native(); // canonical, minimal representation; exact without a precision (#1757)
		return *this;
	}

	// The rounding of a finite nonzero value to a binary format of K significand bits, the
	// leading one included, normal scales E_min .. E_max and subnormals below them, in the
	// current mode and with the flags IEEE-754 raises (#1819): the format's bits come from the
	// top 128 bits of the significand, and the limbs below those are the sticky bit. A result
	// past E_max overflows, to infinity where to_inf, else to the largest finite value; a normal
	// one has a scale and K significand bits, its leading one at bit K - 1; a subnormal or zero
	// one has the fraction field.
	struct native_rounding {
		bool     overflow;
		bool     to_inf;
		bool     normal;
		int64_t  scale;
		uint64_t hi, lo;   // the significand, right-aligned, its bits 64 and up in hi
	};

	template<unsigned K, int E_min, int E_max>
	constexpr native_rounding round_to_native() const noexcept {
		static_assert(K >= 2 && K <= 120, "the 128-bit window keeps the guard bit below the K bits");
		constexpr unsigned shift = 128u - K;   // the bits below the significand
		// the 128-bit value (h, l) and its bits
		auto bit = [](uint64_t h, uint64_t l, unsigned p) -> bool {
			return ((p < 64u) ? (l >> p) : (h >> (p - 64u))) & 1u;
		};
		auto any_below = [](uint64_t h, uint64_t l, unsigned p) -> bool {   // bits [0, p) of (h, l)
			if (p == 0u) return false;
			if (p < 64u) return (l & ((uint64_t(1) << p) - 1u)) != 0u;
			if (l != 0u) return true;
			return (p > 64u) && (h & ((p >= 128u) ? ~uint64_t(0) : ((uint64_t(1) << (p - 64u)) - 1u))) != 0u;
		};

		const std::size_t m = _limb.size();
		const uint64_t raw_hi = _limb[m - 1];   // the leading one at bit 63
		const uint64_t raw_lo = (m >= 2) ? _limb[m - 2] : 0u;
		bool sticky_limbs = false;
		for (std::size_t i = 0; i + 2 < m; ++i) {
			if (_limb[i] != 0) {
				sticky_limbs = true;
				break;
			}
		}

		int64_t  exp       = _exponent;
		uint64_t sh        = raw_hi, sl = raw_lo;
		unsigned eff_shift = shift;
		bool is_subnormal  = (exp < E_min);
		bool lsb = false, guard = false, sticky = false;
		if (is_subnormal) {
			const int64_t sub_shift = E_min - exp;
			if (sub_shift >= static_cast<int64_t>(K)) {
				// below half the smallest subnormal, except at sub_shift == K, where the leading
				// bit is the guard bit of the smallest subnormal 2^(E_min - K + 1)
				sh = 0;
				sl = 0;
				eff_shift = 128;
				guard  = (sub_shift == static_cast<int64_t>(K)) && (raw_hi >> 63) != 0;
				sticky = sticky_limbs || (guard ? ((raw_hi << 1) | raw_lo) != 0 : (raw_hi | raw_lo) != 0);
			}
			else {
				eff_shift = shift + static_cast<unsigned>(sub_shift);
				lsb    = bit(sh, sl, eff_shift);
				guard  = bit(sh, sl, eff_shift - 1);
				sticky = sticky_limbs || any_below(sh, sl, eff_shift - 1);
			}
		}
		else {
			lsb    = bit(sh, sl, shift);
			guard  = bit(sh, sl, shift - 1);
			sticky = sticky_limbs || any_below(sh, sl, shift - 1);
		}

		if (guard || sticky) {
			raise_flags(ExceptionFlag::Inexact);
			if (is_subnormal) raise_flags(ExceptionFlag::Underflow);
		}

		bool round_up = false;
		switch (rounding_mode()) {
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
			if (eff_shift >= 128u) {
				// round up from full underflow to the smallest subnormal 2^(E_min - K + 1)
				sh = uint64_t(1) << 63;
				sl = 0;
				exp = E_min - static_cast<int64_t>(K) + 1;
				eff_shift = 127;
				is_subnormal = true;
			}
			else {
				// add 2^eff_shift; a carry out of the 128 bits doubles the value: one binade up,
				// and for a subnormal one bit less shift, a normal number once it reaches E_min
				const uint64_t add_lo = (eff_shift < 64u) ? (uint64_t(1) << eff_shift) : 0u;
				const uint64_t add_hi = (eff_shift < 64u) ? 0u : (uint64_t(1) << (eff_shift - 64u));
				const uint64_t new_lo = sl + add_lo;
				const uint64_t carry  = (new_lo < sl) ? 1u : 0u;
				const uint64_t new_hi = sh + add_hi + carry;
				const bool     out    = (new_hi < sh) || (new_hi == sh && (add_hi | carry) != 0u);
				sl = new_lo;
				sh = new_hi;
				if (out) {
					sh = uint64_t(1) << 63;
					sl = 0;
					exp++;
					if (is_subnormal) {
						eff_shift--;
						is_subnormal = (exp < E_min);
					}
				}
			}
		}

		native_rounding result{ false, false, false, 0, 0u, 0u };
		if (exp > E_max) {
			raise_flags(ExceptionFlag::Overflow | ExceptionFlag::Inexact);
			bool to_inf = true;
			switch (rounding_mode()) {
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
			result.overflow = true;
			result.to_inf   = to_inf;
			return result;
		}
		// (sh, sl) >> s, for s < 128
		auto shifted = [](uint64_t h, uint64_t l, unsigned s, uint64_t& rh, uint64_t& rl) {
			if (s == 0u)      { rh = h; rl = l; }
			else if (s < 64u) { rh = h >> s; rl = (l >> s) | (h << (64u - s)); }
			else              { rh = 0u; rl = h >> (s - 64u); }
		};
		if (sh == 0u && sl == 0u) return result;   // a zero, below the subnormals
		if (is_subnormal) {
			shifted(sh, sl, eff_shift, result.hi, result.lo);
			return result;
		}
		result.normal = true;
		result.scale  = exp;
		shifted(sh, sl, shift, result.hi, result.lo);
		return result;
	}

	// convert elastic floating-point to native ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	constexpr Real convert_to_ieee754() const noexcept {
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

		if constexpr (std::is_same_v<Real, float> || std::is_same_v<Real, double>) {
			using parameter = ieee754_parameter<Real>;
			using bits_t    = std::conditional_t<std::is_same_v<Real, float>, uint32_t, uint64_t>;
			constexpr unsigned F_bits = parameter::fbits;
			constexpr int      E_bias = parameter::bias;
			const native_rounding r = round_to_native<F_bits + 1, 1 - E_bias, E_bias>();
			if (r.overflow) {
				if (r.to_inf) return (_sign ? -std::numeric_limits<Real>::infinity() : +std::numeric_limits<Real>::infinity());
				return (_sign ? -std::numeric_limits<Real>::max() : +std::numeric_limits<Real>::max());
			}
			const uint64_t exp_field  = r.normal ? static_cast<uint64_t>(r.scale + E_bias) : 0u;
			const uint64_t frac_field = r.lo & ((uint64_t(1) << F_bits) - 1u);
			const uint64_t sign_bit   = (_sign ? 1u : 0u);
			const uint64_t bits       = (sign_bit << (parameter::ebits + F_bits)) | (exp_field << F_bits) | frac_field;
			return sw::bit_cast<Real>(static_cast<bits_t>(bits));
		}
		else {
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
inline constexpr efloat<nlimbs> abs(const efloat<nlimbs>& a) {
	efloat<nlimbs> result(a);
	result.setsign(false);
	return result;
}

// fabs(a): floating-point absolute value (C <cmath> spelling); same as abs.
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> fabs(const efloat<nlimbs>& a) {
	return abs(a);
}

// to_double_expansion<N>(v): v as N doubles, each the conversion of what the ones before it
// left, so that their sum is v to about 53 N bits: hi and lo for a dd, four for a qd. In
// round-to-nearest, which a constant evaluation always uses, the components do not overlap.
// Each residual is exact at v's precision. Past the double range the first component is the
// infinity or the zero the conversion gives and the rest are 0; a NaN gives NaN, 0, ... 0.
// constexpr, so a constant evaluation can hand an efloat result to a fixed-size type (#1819).
template<std::size_t N, unsigned nlimbs>
inline constexpr std::array<double, N> to_double_expansion(const efloat<nlimbs>& v) {
	std::array<double, N> out{};
	efloat<nlimbs> rest(v);
	for (std::size_t i = 0; i < N; ++i) {
		const double c = double(rest);
		out[i] = c;
		constexpr double largest = std::numeric_limits<double>::max();
		if (rest.isnan() || rest.isinf() || c > largest || c < -largest) break;   // NaN, infinity, or an overflow to one
		rest -= efloat<nlimbs>(c);
		if (rest.iszero()) break;
	}
	return out;
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
inline constexpr parse_outcome round_into(const sw::universal::string_parse::decimal_float_scan& d, unsigned target_bits,
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

	// the mantissa as the significand's 64-bit limbs, least significant first: its top bit at
	// the top of the last limb, and the zero bits below its last up to a whole number of 32-bit
	// limbs, and one more when that number is odd, at the bottom
	const std::size_t limbs32 = (target_bits + 31u) / 32u;
	const std::size_t m       = (limbs32 + 1u) / 2u;
	const unsigned    pad     = static_cast<unsigned>(64u * m - target_bits);
	std::vector<std::uint64_t> w(m, 0u);
	for (unsigned b = 0; b < target_bits; ++b) {
		if (mantissa.at(b)) {
			const unsigned p = b + pad;
			w[p / 64u] |= std::uint64_t(1) << (p % 64u);
		}
	}
	value.set_significand(w.data(), m);
	value.normalize();   // no zero limbs below the last 1 bit, so the literal compares as a value does
	return parse_outcome::done;
}

// ASCII whitespace and case, as std::isspace and std::tolower see them in the "C" locale.
// Those two are not constexpr.
inline constexpr bool is_space(char c) noexcept {
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}
inline constexpr char to_lower(char c) noexcept {
	return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// append the decimal digits of e, with a '-' when it is negative (std::to_string is not constexpr)
inline constexpr void append_decimal(std::string& text, std::int64_t e) {
	const bool negative = e < 0;
	std::uint64_t u = negative ? std::uint64_t(0) - static_cast<std::uint64_t>(e) : static_cast<std::uint64_t>(e);
	char buffer[24]{};
	std::size_t n = 0;
	do {
		buffer[n++] = static_cast<char>('0' + u % 10u);
		u /= 10u;
	} while (u != 0u);
	if (negative) text.push_back('-');
	while (n > 0) text.push_back(buffer[--n]);
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
//
// parse() is constexpr, and a constant evaluation reads a literal as the run time does (#1819).
template<unsigned BigBits, unsigned nlimbs>
inline constexpr bool parse(std::string_view txt, efloat<nlimbs>& value) {
	// clear() leaves the Normal state with NO limbs, which is the state the digit loop
	// below builds on -- but it is also a value no arithmetic produces: not zero to
	// iszero() or operator==, printing as 0.000000e-01, with a truncated to_binary. Every
	// `return false` used to hand that half-built value back, and assign() ignores the
	// result, so a failed assign() left it in the caller's object (#1458). Each failure
	// path now leaves the canonical zero instead.
	value.clear();

	using namespace efloat_parse_detail;

	// trim ASCII whitespace
	std::string_view s = txt;
	while (!s.empty() && is_space(s.front())) s.remove_prefix(1);
	while (!s.empty() && is_space(s.back())) s.remove_suffix(1);
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
			tok.push_back(to_lower(s[k]));
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
		append_decimal(text, last);
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

	return efloat<nlimbs>::compare_limbs(lhs, rhs) == 0;
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
			abs_cmp = efloat<nlimbs>::compare_limbs(lhs, rhs);
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
inline constexpr bool operator==(const efloat<nlimbs>& lhs, double rhs) {
	return operator==(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline constexpr bool operator!=(const efloat<nlimbs>& lhs, double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline constexpr bool operator< (const efloat<nlimbs>& lhs, double rhs) {
	return operator<(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline constexpr bool operator> (const efloat<nlimbs>& lhs, double rhs) {
	return operator< (efloat<nlimbs>(rhs), lhs);
}
template<unsigned nlimbs>
inline constexpr bool operator<=(const efloat<nlimbs>& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline constexpr bool operator>=(const efloat<nlimbs>& lhs, double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned nlimbs>
inline constexpr bool operator==(double lhs, const efloat<nlimbs>& rhs) {
	return operator==(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline constexpr bool operator!=(double lhs, const efloat<nlimbs>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline constexpr bool operator< (double lhs, const efloat<nlimbs>& rhs) {
	return operator<(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline constexpr bool operator> (double lhs, const efloat<nlimbs>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
inline constexpr bool operator<=(double lhs, const efloat<nlimbs>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline constexpr bool operator>=(double lhs, const efloat<nlimbs>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - efloat binary arithmetic operators
// (compose from constexpr-marked compound operators); always inlined, so that the result is built
// in the caller's frame and its move follows without a call (#1830)
// BINARY ADDITION
template<unsigned nlimbs>
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
constexpr efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> sum{ typename efloat<nlimbs>::uninitialized_t{} };
	sum.assign_sum(lhs, rhs, false);   // into sum's limbs, without a copy of lhs (#1826)
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
constexpr efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> diff{ typename efloat<nlimbs>::uninitialized_t{} };
	diff.assign_sum(lhs, rhs, true);
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
constexpr efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> mul{ typename efloat<nlimbs>::uninitialized_t{} };
	mul.assign_product(lhs, rhs);   // into mul's limbs; x * x squares (#1761, #1826)
	return mul;
}
// BINARY DIVISION
template<unsigned nlimbs>
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
constexpr efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) noexcept {
	efloat<nlimbs> ratio{ typename efloat<nlimbs>::uninitialized_t{} };
	ratio.assign_quotient(lhs, rhs);   // into ratio's limbs (#1826)
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
inline constexpr efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, double rhs) {
	return operator+(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline constexpr efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, Integer rhs) {
	efloat<nlimbs> sum(lhs);
	sum += rhs;   // in place, without an efloat for rhs (#1762)
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, double rhs) {
	return operator-(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline constexpr efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, Integer rhs) {
	efloat<nlimbs> difference(lhs);
	difference -= rhs;   // in place, without an efloat for rhs (#1762)
	return difference;
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, double rhs) {
	return operator*(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline constexpr efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, Integer rhs) {
	efloat<nlimbs> product(lhs);
	product *= rhs;   // in place, without an efloat for rhs (#1762)
	return product;
}
// BINARY DIVISION
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, double rhs) {
	return operator/(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline constexpr efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, Integer rhs) {
	efloat<nlimbs> quotient(lhs);
	quotient /= rhs;   // a one-limb divisor divides in place (#1761)
	return quotient;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> operator+(double lhs, const efloat<nlimbs>& rhs) {
	return operator+(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline constexpr efloat<nlimbs> operator+(Integer lhs, const efloat<nlimbs>& rhs) {
	efloat<nlimbs> sum(rhs);   // + commutes, and is correctly rounded either way round
	sum += lhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> operator-(double lhs, const efloat<nlimbs>& rhs) {
	return operator-(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline constexpr efloat<nlimbs> operator-(Integer lhs, const efloat<nlimbs>& rhs) {
	return operator-(efloat<nlimbs>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> operator*(double lhs, const efloat<nlimbs>& rhs) {
	return operator*(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline constexpr efloat<nlimbs> operator*(Integer lhs, const efloat<nlimbs>& rhs) {
	efloat<nlimbs> product(rhs);   // * commutes, and is correctly rounded either way round
	product *= lhs;
	return product;
}
// BINARY DIVISION
template<unsigned nlimbs>
inline constexpr efloat<nlimbs> operator/(double lhs, const efloat<nlimbs>& rhs) {
	return operator/(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs, typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
inline constexpr efloat<nlimbs> operator/(Integer lhs, const efloat<nlimbs>& rhs) {
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
			int powerOfTenScale = static_cast<int>(efloat_detail::fp::floor(static_cast<double>(value.scale()) * 0.301029995663981));
			int integerDigits   = (fixed ? (powerOfTenScale + 1) : 1);
			int nrDigits        = integerDigits + static_cast<int>(precision);

			int minBuffer = static_cast<int>(nlimbs) * 16;
			int nrDigitsForFixedFormat = fixed ? std::max(minBuffer, nrDigits) : nrDigits;

			double fullMagnitude = efloat_detail::fp::fabs(static_cast<double>(value));
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
