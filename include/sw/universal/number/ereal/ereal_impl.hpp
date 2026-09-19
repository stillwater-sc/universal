#pragma once
// ereal_impl.hpp: implementation of an adaptive precision multi-component floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>    // std::fill, std::max
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>       // fprintf(stdout/stderr, ...) for the traces and diagnostics (#1334)
#include <cstdlib>      // std::abs(int)
#include <ios>          // std::streamsize, taken by to_string(); no stream is opened
#include <limits>
#include <string>
#include <vector>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// Defaults here rather than in the ereal.hpp umbrella, so that a translation unit which
/// includes core.hpp directly gets the same default (#1334, #1436). Defining it before
/// any ereal header still wins.

// enable throwing specific exceptions for ereal arithmetic errors
// left to application to enable
#if !defined(EREAL_THROW_ARITHMETIC_EXCEPTION)
// default is to return the IEEE-754 special value
#define EREAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

// supporting types and functions, from the I/O-free halves of the native support:
// sw::universal::isinf/isnan (ieee754_core.hpp) and scale(double)
// (manipulators_core.hpp). The ereal.hpp umbrella still brings the text halves.
#include <universal/native/ieee754_core.hpp>
#include <universal/native/manipulators_core.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/internal/expansion/expansion_ops.hpp>  // Shewchuk's expansion arithmetic

/*
The ereal arithmetic can be configured to:
- throw exceptions on invalid arguments and operations
- return a signaling NaN

Compile-time configuration flags are used to select the exception mode.

The exception types are defined, but you have the option to throw them
*/
#include <universal/number/ereal/exceptions.hpp>
#include <universal/number/ereal/ereal_fwd.hpp>   // abs(ereal), called before its definition

namespace sw { namespace universal {

/*
 * ALGORITHMIC CONSTRAINT FOR MULTI-COMPONENT FLOATING-POINT ARITHMETIC
 * =====================================================================
 *
 * The ereal type uses Shewchuk's expansion arithmetic (two_sum/two_product algorithms)
 * which requires all components and error terms to be representable as NORMAL values of
 * the limb type FpType. These algorithms break down when components underflow to
 * subnormal values or zero.
 *
 * Each limb adds p = numeric_limits<FpType>::digits bits of precision. After n limbs, the
 * smallest representable correction term is approximately 2^(-p*n). This must remain
 * >= the smallest normal, 2^emin, to maintain the non-overlapping property:
 *
 *   n <= |emin| / p          digits ~= n * p * log10(2)
 *
 *   limb        p     emin     max limbs   ~digits
 *   float      24     -126         5          36
 *   double     53    -1022        19         303
 *   x87        64   -16382       255        4913
 *   binary128 113   -16382       144        4898
 *
 * Therefore maxlimbs MUST be <= max_safe_limbs for algorithmically correct operations
 * (19 for the default double limbs).
 *
 * Violating this constraint causes:
 *   - two_sum/two_product to produce incorrect error terms (lost to underflow)
 *   - Non-overlapping invariant violations
 *   - Silent arithmetic incorrectness (not just unobservable precision)
 *
 * Reference: Shewchuk, "Adaptive Precision Floating-Point Arithmetic and
 *            Fast Robust Geometric Predicates", 1997
 */

// ereal is a multi-component arbitrary-precision arithmetic type
// Default to 8 limbs (approximately 127 decimal digits of precision) of type double.
//
// FpType is the limb: any p-bit IEEE-754 binary type (#1355) -- float, double, and long
// double where it is x87 extended or binary128. IBM double-double, the default long
// double on ppc64le, is refused: it is itself an expansion (see is_expansion_limb_v).
// The wider limbs are not faster: binary128 is software on every CPU that has it, and
// x87 has no FMA. What they buy is reach -- about 4900 digits against double's 303.
template<unsigned maxlimbs = 8, typename FpType = double>
class ereal {
public:
	static constexpr unsigned maxNrLimbs = maxlimbs;
	using limb_type = FpType;

	// the limb type's own constants, from which the representable range follows
	static constexpr int EXP_BIAS = std::numeric_limits<FpType>::max_exponent - 1;       // 1023 for double
	static constexpr int MAX_EXP = std::numeric_limits<FpType>::max_exponent;             // 1024
	static constexpr int MIN_EXP_NORMAL = std::numeric_limits<FpType>::min_exponent - 1;  // -1022
	static constexpr int MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - static_cast<int>(std::numeric_limits<FpType>::digits * maxlimbs);

	// the most limbs whose last correction term stays normal: see the note above
	static constexpr unsigned max_safe_limbs =
		static_cast<unsigned>((-(std::numeric_limits<FpType>::min_exponent - 1)) / std::numeric_limits<FpType>::digits);

	// decimal digits a limb carries, and the largest power of ten the limb type holds
	static constexpr unsigned digits10_per_limb = static_cast<unsigned>(std::numeric_limits<FpType>::digits10) + 1u;  // 16 for double
	static constexpr int      max_exponent10    = std::numeric_limits<FpType>::max_exponent10;                        // 308 for double

	static constexpr bool bTraceDecimalConversion = false;
	static constexpr bool bTraceDecimalRounding   = false;

	// Newton-reciprocal iteration count for division, scaled to maxlimbs.
	// expansion_reciprocal converges quadratically: from a one-limb, p-bit seed it carries
	// ~p*2^k bits after k iterations. To reach the full ~p*maxlimbs bits we need
	// 2^k >= maxlimbs, i.e. k = ceil(log2(maxlimbs)) whatever p is; +1 guard iteration absorbs
	// rounding in the intermediate products. A fixed iterations=3 (the historical
	// default) capped division -- and therefore every transcendental built on it --
	// at ~130 digits regardless of maxlimbs (issue #1002 deeper root cause). Floored
	// at 3 so small types keep their historical accuracy.
	static constexpr int reciprocal_iterations() {
		int iters = 1;
		unsigned cap = 2;  // 2^1
		while (cap < maxlimbs) { cap <<= 1; ++iters; }  // iters == ceil(log2(maxlimbs))
		++iters;  // guard iteration
		return iters < 3 ? 3 : iters;
	}

	// Enforce algorithmic validity. The limb must be a type the error-free transformations
	// are exact on, and the last limb's correction term must stay normal: for double
	// limbs that is maxlimbs <= 19 (approximately 303 decimal digits).
	static_assert(is_expansion_limb_v<FpType>,
		"ereal<maxlimbs, FpType>: FpType must be a p-bit IEEE-754 binary type (float, double, "
		"x87 extended or binary128 long double). IBM extended double-double -- the default "
		"long double on ppc64le -- is itself a two-component expansion and cannot serve as a "
		"limb; build with -mabi=ieeelongdouble for a binary128 long double there.");
	static_assert(maxlimbs <= max_safe_limbs,
		"ereal<maxlimbs, FpType>: maxlimbs must be <= max_safe_limbs = -(min_exponent - 1) / digits "
		"of the limb type (5 for float, 19 for double, 255 for x87, 144 for binary128). More limbs "
		"push the last one below the smallest normal, violating the non-overlapping property "
		"Shewchuk's expansion arithmetic requires, and two_sum/two_product silently lose bits.");

	// Partial-constexpr surface (issue #750): ereal carries a
	// std::vector<FpType> _limb member, so any non-empty digit storage
	// escapes constant evaluation under C++20's transient-allocation
	// rule.  Default ctor uses is_constant_evaluated() dispatch: at
	// compile time, _limb stays empty (each selector below is empty-
	// vector-guarded so this behaves as canonical zero); at runtime,
	// _limb is initialized to a one-element vector containing 0.0 (the
	// historical representation that the arithmetic and conversion
	// paths rely on).
	//
	// Not marked noexcept: the runtime branch allocates, which can
	// throw std::bad_alloc.  (See comment block on the modifiers
	// for the same reasoning applied to clear/setzero/setnan.)
	//
	// Out of scope -- non-constexpr stdlib helpers / heap mutation:
	//   * isnan, isinf      - use std::fpclassify (not constexpr in C++20)
	//   * signbit, scale    - use std::signbit / sw::universal::scale
	//   * setzero, setnan,  - call clear() + push_back; heap escape
	//     setinf, max/min
	//   * arithmetic ops    - mutate via Shewchuk's expansion arithmetic
	//   * comparison ops    - call compare_adaptive which iterates the
	//                         expansion (also a non-constexpr path today)
	//   * conversion-out    - sums the limb vector at runtime
	//   * native-type ctors / operator= - convert_* allocate
	//   * parse() / to_string() / to_digits() - std::frexp, std::string
	constexpr ereal() : _limb{} {
		if (!std::is_constant_evaluated()) {
			_limb.push_back(0.0);
		}
	}

	constexpr ereal(const ereal&) = default;
	constexpr ereal(ereal&&) = default;

	constexpr ereal& operator=(const ereal&) = default;
	constexpr ereal& operator=(ereal&&) = default;

	// initializers for native types
	ereal(signed char iv)                      noexcept { *this = iv; }
	ereal(short iv)                            noexcept { *this = iv; }
	ereal(int iv)                              noexcept { *this = iv; }
	ereal(long iv)                             noexcept { *this = iv; }
	ereal(long long iv)                        noexcept { *this = iv; }
	ereal(char iv)                             noexcept { *this = iv; }
	ereal(unsigned short iv)                   noexcept { *this = iv; }
	ereal(unsigned int iv)                     noexcept { *this = iv; }
	ereal(unsigned long iv)                    noexcept { *this = iv; }
	ereal(unsigned long long iv)               noexcept { *this = iv; }
	ereal(float iv)                            noexcept { *this = iv; }
	ereal(double iv)                           noexcept { *this = iv; }

	// string constructor
	ereal(const std::string& str) {
		if (!parse(str)) {
			// Parse failed - set to zero (silent failure, matches cascade pattern)
			setzero();
		}
	}

	// specific value constructor
	ereal(const SpecificValue code) noexcept {
		switch (code) {
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
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::infpos:
			setinf(false);
			break;
		case SpecificValue::infneg:
			setinf(true);
			break;
		case SpecificValue::nar: // approximation as ereal doesn't have a NaR
		case SpecificValue::qnan:
			setnan();
			break;
		case SpecificValue::snan:
			setnan();
			break;
		}
	}

	// assignment operators for native types
	ereal& operator=(signed char rhs)          noexcept { return convert_signed(rhs); }
	ereal& operator=(short rhs)                noexcept { return convert_signed(rhs); }
	ereal& operator=(int rhs)                  noexcept { return convert_signed(rhs); }
	ereal& operator=(long rhs)                 noexcept { return convert_signed(rhs); }
	ereal& operator=(long long rhs)            noexcept { return convert_signed(rhs); }
	ereal& operator=(char rhs)                 noexcept { return convert_unsigned(rhs); }
	ereal& operator=(unsigned short rhs)       noexcept { return convert_unsigned(rhs); }
	ereal& operator=(unsigned int rhs)         noexcept { return convert_unsigned(rhs); }
	ereal& operator=(unsigned long rhs)        noexcept { return convert_unsigned(rhs); }
	ereal& operator=(unsigned long long rhs)   noexcept { return convert_unsigned(rhs); }
	ereal& operator=(float rhs)                noexcept { return convert_ieee754(rhs); }
	ereal& operator=(double rhs)               noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float()            const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double()           const noexcept { return convert_to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	ereal(long double iv)                      noexcept { *this = iv; }
	ereal& operator=(long double rhs)          noexcept { return convert_ieee754(rhs); }
	explicit operator long double()      const noexcept { return convert_to_ieee754<long double>(); }
#endif 

	// An ereal always converts from and to its own limb type. For float and double the
	// overloads above do that; for a long double limb they exist only where
	// LONG_DOUBLE_SUPPORT is set, and it is not on MSVC or on a GCC whose long double is
	// double -- where long double is still a valid limb, and ereal<n, long double>(x)
	// would otherwise be ambiguous between the float and double overloads. These
	// templates cover that case; where the non-template overloads exist they win.
	template<typename Limb, std::enable_if_t<std::is_same_v<Limb, FpType> && !std::is_same_v<Limb, float> && !std::is_same_v<Limb, double>, int> = 0>
	ereal(Limb iv) noexcept { convert_ieee754(iv); }
	template<typename Limb, std::enable_if_t<std::is_same_v<Limb, FpType> && !std::is_same_v<Limb, float> && !std::is_same_v<Limb, double>, int> = 0>
	ereal& operator=(Limb rhs) noexcept { return convert_ieee754(rhs); }
	template<typename Limb, std::enable_if_t<std::is_same_v<Limb, FpType> && !std::is_same_v<Limb, float> && !std::is_same_v<Limb, double>, int> = 0>
	explicit operator Limb() const noexcept { return convert_to_ieee754<Limb>(); }

	// Component access
	constexpr FpType  operator[](size_t i) const noexcept { return _limb[i]; }
	constexpr FpType& operator[](size_t i) { return _limb[i]; }

	// prefix operators
	ereal operator-() const {
		ereal negated(*this);
		for (auto& v : negated._limb) v = -v;
		return negated;
	}

	// arithmetic operators
	//
	// `linear_expansion_sum` (Shewchuk Figure 7) returns a non-overlapping
	// expansion but does NOT guarantee a unique canonical representation:
	// the same real value can be produced as different limb sequences
	// depending on the order in which inputs are merged. Without a
	// canonicalisation pass, mathematically equal expressions like
	// `(a + b) + c` and `(c + b) + a` produce different limb vectors, and
	// `compare_adaptive`'s limb-by-limb test reports them as unequal --
	// breaking commutativity / associativity / equality contracts.
	//
	// `renormalize_expansion` rebuilds the expansion via grow_expansion so
	// that equal values produce equal limb sequences. This restores the
	// algebraic invariants at the cost of an extra O(m) pass per operation.
	//
	// Special-value short-circuit (issue #957): Shewchuk's EFT-based merge
	// computes residuals via expressions like `s - a` and `a - (s - bb)`,
	// which produce `Inf - Inf = NaN` when an operand is +/-Inf. The
	// resulting expansion has `Inf` in the leading limb and `NaN` in the
	// residual, and `convert_to_ieee754` sums them back to NaN. To avoid
	// this, we apply IEEE 754 addition rules to special values BEFORE the
	// EFT chain runs.
	// The special-value guards resolve NaN, Inf and zero OPERANDS. A result that overflows
	// from finite operands is handled inside the expansion operations themselves, which
	// scale near the top of the range and return a single signed infinity on a genuine
	// overflow rather than the NaN the error-free transformations produce (#1553).
	ereal& operator+=(const ereal& rhs) {
		using namespace expansion_ops;
		if (apply_ieee754_add_special_values(rhs)) return *this;
		_limb = expansion_sum_normalized(_limb, rhs._limb);
		return *this;
	}
	ereal& operator+=(double rhs) {
		using namespace expansion_ops;
		ereal<maxlimbs, FpType> rhs_expansion(rhs);
		if (apply_ieee754_add_special_values(rhs_expansion)) return *this;
		_limb = expansion_sum_normalized(_limb, rhs_expansion._limb);
		return *this;
	}
	ereal& operator-=(const ereal& rhs) {
		using namespace expansion_ops;
		// Subtraction is a + (-b). Apply the special-value rules to the
		// effective sign-flipped RHS so e.g. (+Inf) - (-Inf) = +Inf + +Inf,
		// not (+Inf) + (-Inf) = NaN.
		ereal<maxlimbs, FpType> neg_rhs_e = -rhs;
		if (apply_ieee754_add_special_values(neg_rhs_e)) return *this;
		_limb = expansion_sum_normalized(_limb, neg_rhs_e._limb);
		return *this;
	}
	ereal& operator-=(double rhs) {
		return operator-=(ereal<maxlimbs, FpType>(rhs));
	}
	ereal& operator*=(const ereal& rhs) {
		using namespace expansion_ops;
		// IEEE 754 special values (NaN/Inf/signed-zero) must be resolved before
		// the EFT product: expansion_product turns finite * Inf into Inf - Inf
		// = NaN and collapses any zero operand to +0 (issue #966).
		if (apply_ieee754_mul_special_values(rhs)) return *this;
		_limb = expansion_product(_limb, rhs._limb);
		return *this;
	}
	ereal& operator*=(double rhs) {
		// Delegate to the ereal overload so the IEEE 754 special-value table is
		// applied uniformly (matches operator-=(double)). The free operator*
		// overloads already construct an ereal for the scalar, so this keeps
		// in-place `*= scalar` consistent with `x = x * scalar`.
		return operator*=(ereal<maxlimbs, FpType>(rhs));
	}
	ereal& operator/=(const ereal& rhs) {
		using namespace expansion_ops;
		// IEEE 754 special values (NaN/Inf/zero) and divide-by-zero must be
		// resolved before the Newton-reciprocal quotient: reciprocal(0) is Inf
		// and a * Inf renormalises to NaN, and any zero operand collapses to +0
		// (issue #968).
		if (apply_ieee754_div_special_values(rhs)) return *this;
		_limb = expansion_quotient(_limb, rhs._limb, reciprocal_iterations());
		return *this;
	}
	ereal& operator/=(double rhs) {
		// Delegate to the ereal overload so the IEEE 754 special-value table and
		// divide-by-zero handling apply uniformly (matches operator*=(double)).
		return operator/=(ereal<maxlimbs, FpType>(rhs));
	}

	// modifiers
	//  After a move (or any shrink-to-fit), the vector can have zero capacity, 
	//  so that push_back may allocate and throw std::bad_alloc. 
	//  Therefore, we cannot mark the functions noexcept. 
	//  The std::bad_alloc exception would trigger std::terminate.`
	void clear()                   { _limb.clear(); _limb.push_back(0.0); }
	void setzero()                 { clear(); }
	void setnan()                  { clear(); _limb[0] = std::numeric_limits<FpType>::quiet_NaN(); }
	void setinf(bool sign = false) { clear(); _limb[0] = (sign ? -std::numeric_limits<FpType>::infinity() : std::numeric_limits<FpType>::infinity()); }

	// Special value setters for numeric_limits support
	ereal& maxpos() {
		clear();
		// Maximum positive value: the limb type's max plus up to three further components,
		// each the previous one scaled by 2^-(p+1), so that each sits just below half an ulp
		// of the one before and the whole is a non-overlapping expansion.
		//
		// For double these were decimal literals, 1.7976931348623157e+308 then
		// 9.9792015476735972e+291, 5.5395696628011126e+275 and 3.0750789988826854e+259. The
		// literals only approximated DBL_MAX * 2^-54k, and the fourth came out as
		// 0x1.00000093c6e94p+862 -- more than half an ulp of the third, an overlapping
		// component in the one value that is supposed to be the largest expansion (#1564).
		constexpr int p = std::numeric_limits<FpType>::digits;
		const FpType top = std::numeric_limits<FpType>::max();
		_limb[0] = top;
		for (unsigned i = 1; i < maxlimbs && i < 4; ++i) _limb.push_back(std::ldexp(top, -(p + 1) * static_cast<int>(i)));
		return *this;
	}

	ereal& minpos() {
		clear();
		// Minimum positive normalized value
		_limb[0] = std::numeric_limits<FpType>::min();  // the smallest normal: DBL_MIN for double
		return *this;
	}

	ereal& minneg() {
		clear();
		// Minimum negative normalized value (closest to zero from below)
		_limb[0] = -std::numeric_limits<FpType>::min();
		return *this;
	}

	ereal& maxneg() {
		// Maximum negative value: negative of maxpos components
		maxpos();
		for (auto& v : _limb) v = -v;
		return *this;
	}

	// parse: convert a decimal string to ereal
	// Returns true on success, false on parse error (leaves *this unchanged)
	// Supports formats: "123", "3.14", "-1.23e-45", "1E+10"
	bool parse(const std::string& str) {
		if (str.empty()) return false;

		ereal<maxlimbs, FpType> result;
		result.setzero();

		size_t pos = 0;
		bool negative = false;
		bool decimal_point_seen = false;
		int decimal_position = 0;
		int exponent = 0;

		// Skip leading whitespace
		while (pos < str.length() && std::isspace(static_cast<unsigned char>(str[pos]))) ++pos;
		if (pos >= str.length()) return false;

		// Parse optional sign
		if (str[pos] == '+' || str[pos] == '-') {
			negative = (str[pos] == '-');
			++pos;
		}

		// Detect nan / inf / infinity tokens (case-insensitive). The digit
		// loop below would otherwise reject any alphabetic character.
		{
			std::string lower;
			for (size_t q = pos; q < str.length(); ++q) {
				lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(str[q]))));
			}
			if (lower == "nan") {
				this->setnan();
				return true;
			}
			if (lower == "inf" || lower == "infinity") {
				this->setinf(negative);
				return true;
			}
		}

		// Parse mantissa digits.
		//
		// Significant digits are accumulated into `result` as an integer via
		// Horner (result = result*10 + digit); the decimal point and any explicit
		// exponent are applied afterward as a power of ten. Accumulation stops
		// after MAX_SIG_DIGITS significant digits because (a) ereal<maxlimbs, FpType>
		// cannot represent more than ~maxlimbs*16 decimal digits anyway, and
		// (b) `result`'s leading component must stay below DBL_MAX, so the integer
		// can hold at most ~307 digits. Past the cap, integer digits scale the
		// exponent and fraction digits (below precision) are dropped (#1006).
		bool found_digit = false;
		bool saw_exponent_marker = false;
		bool sig_started = false;   // seen the first nonzero significant digit
		unsigned numSig = 0;        // significant digits accumulated into result
		int dropped_integer = 0;    // integer digits dropped past the cap (scale up)
		// (16 digits per double limb, and double's 10^307 ceiling)
		constexpr unsigned sigCap = static_cast<unsigned>(max_exponent10 - 1);
		const unsigned MAX_SIG_DIGITS = (maxlimbs * digits10_per_limb + digits10_per_limb < sigCap) ? (maxlimbs * digits10_per_limb + digits10_per_limb) : sigCap;
		ereal<maxlimbs, FpType> ten(10.0);

		while (pos < str.length()) {
			char c = str[pos];

			if (std::isdigit(c)) {
				found_digit = true;
				int digit = c - '0';
				if (digit != 0) sig_started = true;

				if (!sig_started) {
					// Leading zero: not a significant digit. A leading zero in
					// the fraction still shifts the scale (e.g. 0.001).
					if (decimal_point_seen) ++decimal_position;
				}
				else if (numSig < MAX_SIG_DIGITS) {
					// result = result * 10 + digit
					result = result * ten;
					result = result + ereal<maxlimbs, FpType>(static_cast<double>(digit));
					++numSig;
					if (decimal_point_seen) ++decimal_position;
				}
				else {
					// Past the precision cap: an integer digit scales the value
					// by ten; a fraction digit is below precision and dropped.
					if (!decimal_point_seen) ++dropped_integer;
				}
			}
			else if (c == '.' && !decimal_point_seen) {
				decimal_point_seen = true;
			}
			else if (c == 'e' || c == 'E') {
				saw_exponent_marker = true;
				++pos;  // Move past 'e'/'E'
				break;  // Start exponent parsing
			}
			else {
				return false;  // Invalid character
			}

			++pos;
		}

		if (!found_digit) return false;

		// Parse exponent digits when the mantissa loop stopped on an 'e'/'E'
		// marker.  Tracking saw_exponent_marker (instead of inspecting
		// str[pos-1]) is the only correct signal because the mantissa loop
		// may have reached end-of-string normally.  A trailing 'e' with no
		// digits ("1e") and a non-digit after the exponent ("1e3.5") both
		// reject below.
		if (saw_exponent_marker) {
			bool exp_negative = false;

			if (pos < str.length() && (str[pos] == '+' || str[pos] == '-')) {
				exp_negative = (str[pos] == '-');
				++pos;
			}

			bool found_exp_digit = false;
			while (pos < str.length() && std::isdigit(str[pos])) {
				found_exp_digit = true;
				exponent = exponent * 10 + (str[pos] - '0');
				++pos;
			}

			if (!found_exp_digit) return false;
			if (exp_negative) exponent = -exponent;
		}

		// Reject trailing garbage: any character left over after the
		// mantissa + optional exponent run means the input wasn't a
		// valid decimal literal.
		if (pos != str.length()) return false;

		// Net power of ten = explicit exponent - fraction digits + dropped
		// integer digits (digits past the cap that were not stored in result).
		if (decimal_point_seen) {
			exponent -= decimal_position;
		}
		exponent += dropped_integer;

		// Apply the power-of-ten scale.
		//
		// A negative exponent is applied by DIVIDING by the normal-range 10^e,
		// NOT by multiplying by 10^-e. The latter is a subnormal-range value that
		// cannot hold more than ~1 normal component, which would cap the result
		// near 16 digits regardless of maxlimbs (#1006). expansion_quotient scales
		// the divisor to near-unit magnitude, so the (normal-range) quotient keeps
		// full precision. 10^e overflows DBL_MAX for e > 308, so |exponent| is
		// applied in chunks of at most 10^308; for an out-of-range value the chain
		// saturates to inf (overflow) or 0 (underflow) rather than producing NaN.
		if (!result.iszero()) {
			if (exponent > 0) {
				int e = exponent;
				while (e > 0 && !result.isinf() && !result.isnan()) {
					int step = (e > max_exponent10) ? max_exponent10 : e;
					result = result * pown(ten, step);
					e -= step;
				}
				// A product that overflows DBL_MAX leaves a NaN error term; the
				// input was a valid finite decimal, so saturate to +inf (the sign
				// is applied below).
				if (result.isnan()) result.setinf(false);
			}
			else if (exponent < 0) {
				int e = -exponent;
				while (e > max_exponent10 && !result.iszero()) { result = result / pown(ten, max_exponent10); e -= max_exponent10; }
				if (e > 0 && !result.iszero()) result = result / pown(ten, e);
			}
		}

		// Apply sign
		if (negative) {
			result = -result;
		}

		// Limit to the type's component budget. Components past maxlimbs lie below
		// ereal<maxlimbs, FpType>'s representable precision and would be subnormal, which
		// violates the normal-double invariant that Shewchuk's two_sum/two_product
		// require. The expansion is in canonical decreasing-magnitude order, so the
		// leading maxlimbs components carry the full representable value (#1006).
		if (result._limb.size() > maxlimbs) {
			result._limb.resize(maxlimbs);
		}

		// Success - assign to *this
		*this = result;
		return true;
	}

	ereal& assign(const std::string& txt) {
		parse(txt);  // If parse fails, *this remains unchanged
		return *this;
	}

	// convert to string containing digits number of digits
	std::string to_string(std::streamsize precision = 7, std::streamsize width = 15,
		bool fixed = false, bool scientific = true, bool internal = false,
		bool left = false, bool showpos = false, bool uppercase = false,
		char fill = ' ') const
	{
		std::string s;

		// Guard: empty _limb vector is semantically zero
		// (renormalize_expansion can prune all components to empty)
		if (_limb.empty()) {
			if (showpos) s += '+';
			s += '0';
			if (precision > 0) {
				s += '.';
				s.append(static_cast<unsigned int>(precision), '0');
			}
			if (!fixed) {
				s += uppercase ? "E+00" : "e+00";
			}
			// apply width/fill padding
			if (width > 0 && s.length() < static_cast<size_t>(width)) {
				size_t pad = static_cast<size_t>(width) - s.length();
				if (left) { s.append(pad, fill); }
				else { s.insert(0, pad, fill); }
			}
			return s;
		}

		bool negative = isneg();
		int  e{ 0 };
		if (fixed && scientific) fixed = false; // scientific format takes precedence
		if (isnan()) {
			s = uppercase ? "NAN" : "nan";
			negative = false;
		}
		else {
			if (negative) { s += '-'; } else { if (showpos) s += '+'; }

			if (isinf()) {
				s += uppercase ? "INF" : "inf";
			}
			else if (iszero()) {
				s += '0';
				if (precision > 0) {
					s += '.';
					s.append(static_cast<unsigned int>(precision), '0');
				}
			}
			else {
				int powerOfTenScale = static_cast<int>(std::floor(std::log10(std::fabs(_limb[0]))));
				int integerDigits = (fixed ? (powerOfTenScale + 1) : 1);
				int nrDigits = integerDigits + static_cast<int>(precision);

				// Adaptive buffer size: the decimal digits the limbs carry
				int minBuffer = static_cast<int>(maxlimbs * digits10_per_limb);
				int nrDigitsForFixedFormat = nrDigits;
				if (fixed)
					nrDigitsForFixedFormat = std::max(minBuffer, nrDigits);

				if constexpr (bTraceDecimalConversion) {
					std::fprintf(stdout, "powerOfTenScale  : %d\n", powerOfTenScale);
					std::fprintf(stdout, "integerDigits    : %d\n", integerDigits);
					std::fprintf(stdout, "nrDigits         : %d\n", nrDigits);
					std::fprintf(stdout, "nrDigitsForFixedFormat  : %d\n", nrDigitsForFixedFormat);
				}

				// a number in the range of [0.5, 1.0) to be printed with zero precision
				// must be rounded up to 1 to print correctly
				// Use full ereal magnitude (not just _limb[0]) for correct multi-limb rounding
				{
				FpType fullMagnitude = std::fabs(convert_to_ieee754<FpType>());
				if (fixed && (precision == 0) && (fullMagnitude < 1.0)) {
					s += (fullMagnitude >= 0.5) ? '1' : '0';
				}
				else if (fixed && nrDigits <= 0) {
					// process values that are near zero
					s += '0';
					if (precision > 0) {
						s += '.';
						s.append(static_cast<unsigned int>(precision), '0');
					}
				}
				else {
					std::vector<char> t;

					if (fixed) {
						t.resize(static_cast<size_t>(nrDigitsForFixedFormat + 1));
						to_digits(t, e, nrDigitsForFixedFormat);
					}
					else {
						t.resize(static_cast<size_t>(nrDigits + 1));
						to_digits(t, e, nrDigits);
					}

					if (fixed) {
						// round the decimal string
						round_string(t, nrDigits + 1, &integerDigits);

						if (integerDigits > 0) {
							int i;
							for (i = 0; i < integerDigits; ++i) s += t[static_cast<unsigned>(i)];
							if (precision > 0) {
								s += '.';
								for (int j = 0; j < precision; ++j, ++i) s += t[static_cast<unsigned>(i)];
							}
						}
						else {
							s += "0.";
							if (integerDigits < 0) s.append(static_cast<size_t>(-integerDigits), '0');
							for (int i = 0; i < nrDigits; ++i) s += t[static_cast<unsigned>(i)];
						}
					}
					else {
						s += t[0ull];
						if (precision > 0) s += '.';

						for (int i = 1; i <= precision; ++i)
							s += t[static_cast<unsigned>(i)];
					}
				}
				} // fullMagnitude scope
			}

			if (!fixed && !isinf()) {
				// construct the exponent
				s += uppercase ? 'E' : 'e';
				append_exponent(s, e);
			}
		}

		// process any fill
		size_t strLength = s.length();
		if (width > 0 && strLength < static_cast<size_t>(width)) {
			size_t nrCharsToFill = (width - strLength);
			if (internal) {
				const bool hasSign = !s.empty() && (s[0] == '-' || s[0] == '+');
				s.insert(hasSign ? static_cast<std::string::size_type>(1)
				                 : static_cast<std::string::size_type>(0),
				         nrCharsToFill, fill);
			}
			else if (left) {
				s.append(nrCharsToFill, fill);
			}
			else {
				s.insert(static_cast<std::string::size_type>(0), nrCharsToFill, fill);
			}
		}

		return s;
	}

	// selectors (empty-_limb-guarded so the constexpr default ctor's
	// empty-vector path doesn't dereference out of bounds; the runtime
	// path always has _limb.size() >= 1 so the guards are pure
	// constexpr-evaluation safety)
	constexpr bool iszero()  const noexcept { return _limb.empty() || _limb[0] == 0.0; }
	constexpr bool isone()   const noexcept { return !_limb.empty() && _limb[0] == 1.0; }
	constexpr bool ispos()   const noexcept { return !_limb.empty() && _limb[0] > 0.0; }
	constexpr bool isneg()   const noexcept { return !_limb.empty() && _limb[0] < 0.0; }
	// isinf, isnan use sw::universal::isinf/isnan which call std::fpclassify
	// (not constexpr in C++20); not promoted.  Empty guard added for
	// runtime safety against zero-capacity vectors after move (see
	// modifier comment block below).
	bool isinf()   const noexcept { return !_limb.empty() && sw::universal::isinf(_limb[0]); }
	bool isnan()   const noexcept { return !_limb.empty() && sw::universal::isnan(_limb[0]); }

	// value information selectors
	bool                       signbit()     const noexcept { return !_limb.empty() && std::signbit(_limb[0]); }
	constexpr int              sign()        const noexcept { return (isneg() ? -1 : 1); }
	int64_t                    scale()       const noexcept { return _limb.empty() ? 0 : sw::universal::scale(_limb[0]); }
	constexpr FpType           significant() const noexcept { return _limb.empty() ? FpType(0) : _limb[0]; }
	constexpr const std::vector<FpType>& limbs()       const noexcept { return _limb; }

protected:
	std::vector<FpType> _limb;     // components of the real value

	// HELPER methods

	// apply_ieee754_add_special_values: IEEE 754 special-value rules for
	// addition (resolves issue #957). If either operand is NaN or +/-Inf,
	// canonicalise `*this` to the single-limb IEEE 754 result and return
	// true. Otherwise return false and let the EFT-based merge handle the
	// finite-finite case.
	//
	// Rules:
	//   NaN + anything           = NaN
	//   anything + NaN           = NaN
	//   +Inf + +Inf              = +Inf
	//   -Inf + -Inf              = -Inf
	//   +Inf + -Inf              = NaN
	//   +/-Inf + finite          = +/-Inf
	//   finite + +/-Inf          = +/-Inf
	//
	// All special-value branches set *this to a canonical single-limb
	// representation via setinf()/setnan() so any pre-existing multi-limb
	// state is normalised, regardless of which operand is the special value.
	bool apply_ieee754_add_special_values(const ereal& rhs) {
		bool a_nan = this->isnan();
		bool b_nan = rhs.isnan();
		if (a_nan || b_nan) {
			this->setnan();
			return true;
		}
		bool a_inf = this->isinf();
		bool b_inf = rhs.isinf();
		if (a_inf && b_inf) {
			// Same sign -> keep that infinity; opposite sign -> NaN
			if (this->signbit() == rhs.signbit()) {
				this->setinf(this->signbit());
				return true;
			}
			this->setnan();
			return true;
		}
		if (a_inf) {
			// *this is +/-Inf, rhs is finite: result is +/-Inf with this's sign.
			this->setinf(this->signbit());
			return true;
		}
		if (b_inf) {
			// *this is finite, rhs is +/-Inf: result is +/-Inf with rhs's sign.
			this->setinf(rhs.signbit());
			return true;
		}
		// Signed zero: when both operands are zero, IEEE 754 round-to-nearest
		// gives -0 only if both addends are -0, otherwise +0. operator-= routes
		// here on the sign-flipped RHS, so this single rule yields the correct
		// subtraction table (e.g. -0 - +0 = -0 + -0 = -0). The general sum path
		// would otherwise renormalise any zero result to +0.
		if (this->iszero() && rhs.iszero()) {
			bool negzero = this->signbit() && rhs.signbit();
			clear();
			_limb[0] = negzero ? -0.0 : 0.0;
			return true;
		}
		return false;
	}

	// apply_ieee754_mul_special_values: IEEE 754 special-value rules for
	// multiplication (resolves issue #966). If either operand is NaN, +/-Inf,
	// or zero, canonicalise `*this` to the single-limb IEEE 754 result and
	// return true. Otherwise return false and let expansion_product handle the
	// finite-nonzero * finite-nonzero case.
	//
	// The sign of a product is signbit(a) XOR signbit(b) -- including for zero
	// and infinite results. The general expansion_product path cannot express
	// this: it would turn finite * Inf into Inf - Inf = NaN, and short-circuit
	// any zero operand to a positive +0 limb.
	//
	// Rules:
	//   NaN * anything           = NaN
	//   anything * NaN           = NaN
	//   Inf * 0  (either order)  = NaN
	//   Inf * Inf                = Inf, sign = a ^ b
	//   Inf * finite-nonzero     = Inf, sign = a ^ b
	//   finite-nonzero * Inf     = Inf, sign = a ^ b
	//   0 * finite / finite * 0  = 0,   sign = a ^ b   (signed-zero rule)
	bool apply_ieee754_mul_special_values(const ereal& rhs) {
		if (this->isnan() || rhs.isnan()) {
			this->setnan();
			return true;
		}
		bool a_inf  = this->isinf();
		bool b_inf  = rhs.isinf();
		bool a_zero = this->iszero();
		bool b_zero = rhs.iszero();
		bool resultSign = this->signbit() != rhs.signbit();  // XOR of signs
		if (a_inf || b_inf) {
			// Inf times zero is NaN regardless of order; otherwise signed Inf.
			if ((a_inf && b_zero) || (b_inf && a_zero)) {
				this->setnan();
				return true;
			}
			this->setinf(resultSign);
			return true;
		}
		if (a_zero || b_zero) {
			// Product with a zero operand is a zero whose sign is the XOR of the
			// operand signs; expansion_product would otherwise always yield +0.
			clear();
			_limb[0] = resultSign ? -0.0 : 0.0;
			return true;
		}
		return false;
	}

	// apply_ieee754_div_special_values: IEEE 754 special-value rules for
	// division (resolves issue #968). If the operands are special (NaN/Inf/
	// zero), canonicalise `*this` to the single-limb IEEE 754 result and return
	// true. Otherwise return false and let expansion_quotient handle the
	// finite-nonzero / finite-nonzero case.
	//
	// The sign of a quotient is signbit(a) XOR signbit(b) -- including for zero
	// and infinite results. The Newton-reciprocal path cannot express this:
	// reciprocal(0) yields Inf, then a * Inf renormalises to NaN, and any zero
	// operand collapses to a positive +0.
	//
	// Rules (a / b), default (non-throwing) mode:
	//   NaN / x, x / NaN          = NaN
	//   x / 0 (x != 0)            = Inf, sign = a ^ b
	//   0 / 0                     = NaN
	//   Inf / Inf                 = NaN
	//   Inf / finite-nonzero      = Inf, sign = a ^ b
	//   finite / Inf              = 0,   sign = a ^ b
	//   0 / finite-nonzero        = 0,   sign = a ^ b
	//
	// When EREAL_THROW_ARITHMETIC_EXCEPTION is enabled, a zero divisor throws
	// ereal_divide_by_zero instead of returning a non-finite value.
	bool apply_ieee754_div_special_values(const ereal& rhs) {
		if (this->isnan() || rhs.isnan()) {
			this->setnan();
			return true;
		}
		bool a_inf  = this->isinf();
		bool b_inf  = rhs.isinf();
		bool a_zero = this->iszero();
		bool b_zero = rhs.iszero();
		bool resultSign = this->signbit() != rhs.signbit();  // XOR of signs
		if (b_zero) {
#if EREAL_THROW_ARITHMETIC_EXCEPTION
			throw ereal_divide_by_zero();
#else
			// 0/0 is NaN; any other x/0 is a signed infinity.
			if (a_zero) { this->setnan(); return true; }
			this->setinf(resultSign);
			return true;
#endif
		}
		if (a_inf) {
			// Inf / Inf is NaN; Inf / finite-nonzero is a signed infinity.
			if (b_inf) { this->setnan(); return true; }
			this->setinf(resultSign);
			return true;
		}
		if (b_inf || a_zero) {
			// finite / Inf and 0 / finite-nonzero are signed zeros; the quotient
			// path would otherwise drop the sign.
			clear();
			_limb[0] = resultSign ? -0.0 : 0.0;
			return true;
		}
		return false;
	}

	// convert arithmetic types into an elastic floating-point
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	ereal& convert_signed(SignedInt v) noexcept {
		if (0 == v) {
			setzero();
			return *this;
		}
		// The magnitude is taken in unsigned arithmetic, so the most negative value of
		// the type -- whose negation does not fit it -- converts like any other.
		const bool negative = v < 0;
		const std::uint64_t magnitude = negative
			? std::uint64_t(0) - static_cast<std::uint64_t>(v)
			: static_cast<std::uint64_t>(v);
		convert_unsigned(magnitude);
		if (negative) {
			for (auto& limb : _limb) limb = -limb;
		}
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	ereal& convert_unsigned(UnsignedInt v) noexcept {
		// Both of these used to be `// TBD` for any non-zero value, so every ereal built
		// or assigned from a non-zero integer was silently zero -- ereal(5), ereal(-7),
		// `e = 42` all compared equal to 0 (#1460).
		//
		// operator=(char) routes here, and whether plain char is signed is up to the
		// implementation. Send a signed type to the signed path, or a negative char
		// would be read as a value near 2^64.
		if constexpr (std::is_signed_v<UnsignedInt>) {
			return convert_signed(v);
		}
		else {
			if (0 == v) {
				setzero();
				return *this;
			}
			const std::uint64_t u = static_cast<std::uint64_t>(v);
			clear();
			constexpr int p = std::numeric_limits<FpType>::digits;
			if constexpr (p >= 64) {
				_limb[0] = static_cast<FpType>(u);     // every 64-bit integer is exact in such a limb
				return *this;
			}
			else {
				if (u < (std::uint64_t(1) << p)) {
					_limb[0] = static_cast<FpType>(u); // every integer below 2^p is exact in a limb
					return *this;
				}
			}
			// Otherwise a limb cannot hold every integer, so split it into chunks a limb does
			// hold exactly: 32 bits for double (hi and lo halves), 16 for float. Each scaled
			// chunk occupies bits the ones below it cannot reach, so the chunks form an
			// expansion whose sum is exactly u; adding them through the ordinary path leaves
			// it renormalized.
			constexpr int chunk = (p >= 32) ? 32 : 16;
			constexpr std::uint64_t mask = (std::uint64_t(1) << chunk) - 1u;
			bool leading = true;
			for (int shift = 64 - chunk; shift >= 0; shift -= chunk) {
				const std::uint64_t c = (u >> shift) & mask;
				if (c == 0) continue;
				const FpType part = std::ldexp(static_cast<FpType>(c), shift);   // exact
				if (leading) { _limb[0] = part; leading = false; continue; }
				ereal tail;
				tail._limb[0] = part;
				*this += tail;
			}
			return *this;
		}
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	ereal& convert_ieee754(Real rhs) noexcept {
		clear();
		constexpr bool fits = std::numeric_limits<Real>::digits <= std::numeric_limits<FpType>::digits
		                   && std::numeric_limits<Real>::max_exponent <= std::numeric_limits<FpType>::max_exponent
		                   && std::numeric_limits<Real>::min_exponent >= std::numeric_limits<FpType>::min_exponent;
		if constexpr (fits) {
			_limb[0] = static_cast<FpType>(rhs);   // exact: the limb type holds every value of Real
		}
		else {
			// A value wider than one limb -- a double into float limbs, or an x87 long double
			// into double limbs -- is split into an expansion: each limb is the rounded
			// remainder, and the remainder after it is exact in Real. The pieces come out
			// non-overlapping and in decreasing order. Bits below the limb type's range are
			// lost, and a value above it is an infinity, as any narrowing conversion would give.
			if (!std::isfinite(rhs) || rhs == Real(0)) { _limb[0] = static_cast<FpType>(rhs); return *this; }
			// Overflow is decided before any cast: converting a finite value the limb type
			// cannot represent is undefined behaviour by the letter of [conv.double] (#1570).
			// The threshold is max + ulp(max)/2, not max: a value between the two rounds DOWN
			// to max and is kept exactly, as {max, remainder}. At the threshold itself the tie
			// goes to the even neighbour, 2^max_exponent, which is infinity. Only the first
			// piece can overflow; every remainder after it is below half an ulp of it.
			if constexpr (std::numeric_limits<Real>::max_exponent > std::numeric_limits<FpType>::max_exponent) {
				const Real limit = static_cast<Real>(std::numeric_limits<FpType>::max())
				                 + std::ldexp(Real(1), std::numeric_limits<FpType>::max_exponent - std::numeric_limits<FpType>::digits - 1);
				if (std::fabs(rhs) >= limit) {
					_limb[0] = (rhs < Real(0)) ? -std::numeric_limits<FpType>::infinity() : std::numeric_limits<FpType>::infinity();
					return *this;
				}
			}
			Real rest = rhs;
			for (unsigned i = 0; i < maxlimbs && rest != Real(0); ++i) {
				const FpType piece = static_cast<FpType>(rest);
				if (piece == FpType(0)) break;                                             // below range
				if (i == 0) _limb[0] = piece; else _limb.push_back(piece);
				rest -= static_cast<Real>(piece);
			}
		}
		return *this;
	}


	// convert elastic floating-point to native ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	Real convert_to_ieee754() const noexcept {
		if (_limb.empty()) return Real(0);
		// Seed the accumulator with the leading (largest-magnitude) component
		// rather than +0. Starting from +0 would discard the sign of a signed
		// zero (IEEE 754: +0 + -0 == +0), turning a stored -0 into +0. Seeding
		// from _limb[0] is numerically identical for non-zero expansions since
		// _limb[0] is the dominant term.
		Real sum = static_cast<Real>(_limb[0]);
		for (std::size_t i = 1; i < _limb.size(); ++i) {
			sum += static_cast<Real>(_limb[i]);
		}
		return sum;
	}

	// to_digits generates the decimal digits representing this ereal value
	// Ported from dd_impl.hpp, adapted for multi-component expansion arithmetic
	void to_digits(std::vector<char>& s, int& exponent, int precision) const {
		constexpr double _log2(0.301029995663981);

		if (iszero()) {
			exponent = 0;
			for (int i = 0; i < precision; ++i) s[static_cast<unsigned>(i)] = '0';
			return;
		}

		// First determine the (approximate) exponent.
		int e;
		(void)std::frexp(_limb[0], &e);  // Only need exponent, not mantissa
		--e; // adjust e as frexp gives a binary e that is 1 too big
		e = static_cast<int>(_log2 * e); // estimate the power of ten exponent

		// r = abs(*this) - use the free function abs() which is defined earlier in the file
		ereal<maxlimbs, FpType> r = sw::universal::abs(*this);
		const ereal<maxlimbs, FpType> _ten(10.0);
		const ereal<maxlimbs, FpType> _one(1.0);

		// exact power-of-two scaling of every limb; used near the ends of the range, where
		// 10^|e| would overflow the limb type (e beyond 300 for double limbs)
		constexpr int p = std::numeric_limits<FpType>::digits;
		constexpr int nearRangeEnd = max_exponent10 - 8;
		auto scaled = [](ereal<maxlimbs, FpType> x, int k) {
			for (auto& v : x._limb) v = std::ldexp(v, k);
			return x;
		};

		if (e < 0) {
			if (e < -nearRangeEnd) {
				r = scaled(r, p);
				r *= pown(_ten, -e);
				r = scaled(r, -p);
			}
			else {
				r *= pown(_ten, -e);
			}
		}
		else {
			if (e > 0) {
				if (e > nearRangeEnd) {
					r = scaled(r, -p);
					r /= pown(_ten, e);
					r = scaled(r, p);
				}
				else {
					r /= pown(_ten, e);
				}
			}
		}

		// Fix exponent if we have gone too far
		if (r >= _ten) {
			r /= _ten;
			++e;
		}
		else {
			if (r < 1.0) {
				r *= _ten;
				--e;
			}
		}

		if ((r >= _ten) || (r < _one)) {
			std::fprintf(stderr, "ereal::to_digits() failed to compute exponent\n");
			std::fill(s.begin(), s.end(), '0');
			if (!s.empty()) s.back() = 0;
			exponent = 0;
			return;
		}

		// at this point the value is normalized to a decimal value between (0, 10)
		// generate the digits
		int nrDigits = precision + 1;
		for (int i = 0; i < nrDigits; ++i) {
			if (r.limbs().empty() || r.iszero()) {
				// fill remaining digits with zeros
				for (int j = i; j < nrDigits; ++j) s[static_cast<unsigned>(j)] = '0';
				break;
			}
			int mostSignificantDigit = static_cast<int>(r[0]);
			r -= static_cast<double>(mostSignificantDigit);
			r *= 10.0;

			s[static_cast<unsigned>(i)] = static_cast<char>(mostSignificantDigit + '0');
			// the buffer is not NUL-terminated until the loop ends: format a bounded copy, as
			// dd does, rather than s.data(), which read past the vector on the last digit
			if constexpr (bTraceDecimalConversion) std::fprintf(stdout, "to_digits  digit[%d] : %s\n", i, std::string(s.begin(), s.end()).c_str());
		}

		// Fix out of range digits
		for (int i = nrDigits - 1; i > 0; --i) {
			if (s[static_cast<unsigned>(i)] < '0') {
				s[static_cast<unsigned>(i - 1)]--;
				s[static_cast<unsigned>(i)] += 10;
			}
			else {
				if (s[static_cast<unsigned>(i)] > '9') {
					s[static_cast<unsigned>(i - 1)]++;
					s[static_cast<unsigned>(i)] -= 10;
				}
			}
		}

		if (s[0] <= '0') {
			std::fprintf(stderr, "ereal::to_digits() non-positive leading digit\n");
			std::fill(s.begin(), s.end(), '0');
			if (!s.empty()) s.back() = 0;
			exponent = 0;
			return;
		}

		// Round and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			int i = nrDigits - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}

		// If first digit is 10, shift left and increment exponent
		if (s[0] > '9') {
			++e;
			for (int i = precision; i >= 2; --i) {
				s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
			}
			s[0] = '1';
			s[1] = '0';
		}

		s[static_cast<unsigned>(precision)] = 0;  // termination null
		exponent = e;
	}

	// precondition: string s must be all digits
	void round_string(std::vector<char>& s, int precision, int* decimalPoint) const {
		if constexpr (bTraceDecimalRounding) {
			std::fprintf(stdout, "string       : %s\n", std::string(s.begin(), s.end()).c_str());
			std::fprintf(stdout, "precision    : %d\n", precision);
			std::fprintf(stdout, "decimalPoint : %d\n", *decimalPoint);
		}

		int nrDigits = precision;
		// round decimal string and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			if constexpr (bTraceDecimalRounding) std::fprintf(stdout, "need to round\n");
			int i = nrDigits - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}

		// if first digit is 10, shift everything.
		if (s[0] > '9') {
			if constexpr (bTraceDecimalRounding) std::fprintf(stdout, "shift right to handle overflow\n");
			for (int i = precision; i >= 2; --i) s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
			s[0u] = '1';
			s[1u] = '0';

			(*decimalPoint)++; // increment decimal point
			++precision;
		}
	}

	void append_exponent(std::string& str, int e) const {
		str += (e < 0 ? '-' : '+');
		e = std::abs(e);
		int k;
		if (e >= 100) {
			k = (e / 100);
			str += static_cast<char>('0' + k);
			e -= 100 * k;
		}

		k = (e / 10);
		str += static_cast<char>('0' + k);
		e -= 10 * k;

		str += static_cast<char>('0' + e);
	}

private:

	// find the most significant bit set
	template<unsigned nnlimbs, typename FFpType>
	friend signed findMsb(const ereal<nnlimbs, FFpType>& v);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////    ereal functions   /////////////////////////////////

template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> abs(const ereal<nlimbs, FpType>& a) {
	return (a < 0 ? -a : a);
}

// pown returns x raised to the integer power n
// Adaptive-precision repeated squaring (no double conversion)
template<unsigned maxlimbs, typename FpType>
inline ereal<maxlimbs, FpType> pown(const ereal<maxlimbs, FpType>& x, int n) {
	using Real = ereal<maxlimbs, FpType>;

	// Special cases
	if (n == 0) return Real(1.0);
	if (n == 1) return x;
	if (x.iszero()) {
		if (n < 0) return Real(std::numeric_limits<double>::quiet_NaN());
		return Real(0.0);
	}
	if (x.isone()) return Real(1.0);

	// The magnitude of the exponent is taken in unsigned arithmetic. Negating n first,
	// as this used to, is undefined for INT_MIN, whose negation does not fit an int
	// (#1466). x^(-n) = 1 / x^n is applied once, at the end.
	const bool negativeExponent = n < 0;
	unsigned int exp = negativeExponent
		? 0u - static_cast<unsigned int>(n)
		: static_cast<unsigned int>(n);

	// Positive integer power using repeated squaring
	// This algorithm is O(log n) and maintains full precision
	Real result(1.0);
	Real base = x;

	while (exp > 0) {
		if (exp & 1) {
			result = result * base;  // Uses ereal multiplication, maintains precision
		}
		base = base * base;
		exp >>= 1;
	}

	return negativeExponent ? Real(1.0) / result : result;
}

////////////////////////////////////////////////////////////////////////////////
/// string parsing

// read a ereal ASCII format and make a binary ereal out of it
template<unsigned nlimbs, typename FpType>
bool parse(const std::string& txt, ereal<nlimbs, FpType>& value) {
	return value.parse(txt);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - ereal binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<unsigned nlimbs, typename FpType>
inline bool operator==(const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	using namespace expansion_ops;
	return compare_adaptive(lhs.limbs(), rhs.limbs()) == 0;
}
template<unsigned nlimbs, typename FpType>
inline bool operator!=(const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator< (const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	using namespace expansion_ops;
	return compare_adaptive(lhs.limbs(), rhs.limbs()) < 0;
}
template<unsigned nlimbs, typename FpType>
inline bool operator> (const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator<=(const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator>=(const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nlimbs, typename FpType>
inline bool operator==(const ereal<nlimbs, FpType>& lhs, double rhs) {
	return operator==(lhs, ereal<nlimbs, FpType>(rhs));
}
template<unsigned nlimbs, typename FpType>
inline bool operator!=(const ereal<nlimbs, FpType>& lhs, double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator< (const ereal<nlimbs, FpType>& lhs, double rhs) {
	return operator<(lhs, ereal<nlimbs, FpType>(rhs));
}
template<unsigned nlimbs, typename FpType>
inline bool operator> (const ereal<nlimbs, FpType>& lhs, double rhs) {
	return operator< (ereal<nlimbs, FpType>(rhs), lhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator<=(const ereal<nlimbs, FpType>& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator>=(const ereal<nlimbs, FpType>& lhs, double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - ereal binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned nlimbs, typename FpType>
inline bool operator==(double lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator==(ereal<nlimbs, FpType>(lhs), rhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator!=(double lhs, const ereal<nlimbs, FpType>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator< (double lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator<(ereal<nlimbs, FpType>(lhs), rhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator> (double lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator<=(double lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs, typename FpType>
inline bool operator>=(double lhs, const ereal<nlimbs, FpType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - ereal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator+(const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	ereal<nlimbs, FpType> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator-(const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	ereal<nlimbs, FpType> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator*(const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	ereal<nlimbs, FpType> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator/(const ereal<nlimbs, FpType>& lhs, const ereal<nlimbs, FpType>& rhs) {
	ereal<nlimbs, FpType> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - literal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator+(const ereal<nlimbs, FpType>& lhs, double rhs) {
	return operator+(lhs, ereal<nlimbs, FpType>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator-(const ereal<nlimbs, FpType>& lhs, double rhs) {
	return operator-(lhs, ereal<nlimbs, FpType>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator*(const ereal<nlimbs, FpType>& lhs, double rhs) {
	return operator*(lhs, ereal<nlimbs, FpType>(rhs));
}
// BINARY DIVISION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator/(const ereal<nlimbs, FpType>& lhs, double rhs) {
	return operator/(lhs, ereal<nlimbs, FpType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - ereal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator+(double lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator+(ereal<nlimbs, FpType>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator-(double lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator-(ereal<nlimbs, FpType>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator*(double lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator*(ereal<nlimbs, FpType>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nlimbs, typename FpType>
inline ereal<nlimbs, FpType> operator/(double lhs, const ereal<nlimbs, FpType>& rhs) {
	return operator/(ereal<nlimbs, FpType>(lhs), rhs);
}

}} // namespace sw::universal
