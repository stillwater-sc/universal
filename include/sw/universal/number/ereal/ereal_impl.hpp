#pragma once
// ereal_impl.hpp: implementation of an adaptive precision multi-component floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cctype>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>
#include <type_traits>

// supporting types and functions
#include <universal/native/ieee754.hpp>   // IEEE-754 decoders
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

namespace sw { namespace universal {

/*
 * ALGORITHMIC CONSTRAINT FOR MULTI-COMPONENT FLOATING-POINT ARITHMETIC
 * =====================================================================
 *
 * The ereal type uses Shewchuk's expansion arithmetic (two_sum/two_product algorithms)
 * which requires all components and error terms to be representable as NORMAL IEEE-754
 * double-precision values. These algorithms break down when components underflow to
 * subnormal values or zero.
 *
 * Each limb adds approximately 53 bits of precision (one double's mantissa).
 * After n limbs, the smallest representable correction term is approximately 2^(-53n).
 * This must remain >= DBL_MIN (2^-1022) to maintain the non-overlapping property.
 *
 * Mathematical limit:
 *   2^(-53n) >= 2^(-1022)
 *   -53n >= -1022
 *   n <= 19.28
 *
 * Therefore: maxlimbs MUST be <= 19 for algorithmically correct operations.
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
// Default to 8 limbs (approximately 127 decimal digits of precision)
template<unsigned maxlimbs = 8>
class ereal {
public:
	static constexpr unsigned maxNrLimbs = maxlimbs;

	// IEEE-754 double precision constants for constructing special values
	static constexpr int EXP_BIAS = 1023;
	static constexpr int MAX_EXP = 1024;
	static constexpr int MIN_EXP_NORMAL = -1022;
	static constexpr int MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - static_cast<int>(53 * maxlimbs);

	static constexpr bool bTraceDecimalConversion = false;
	static constexpr bool bTraceDecimalRounding   = false;

	// Newton-reciprocal iteration count for division, scaled to maxlimbs.
	// expansion_reciprocal converges quadratically: from a 53-bit seed it carries
	// ~53*2^k bits after k iterations. To reach the full ~53*maxlimbs bits we need
	// 2^k >= maxlimbs, i.e. k = ceil(log2(maxlimbs)); +1 guard iteration absorbs
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

	// Enforce algorithmic validity: two_sum/two_product require normal doubles
	// Maximum safe configuration is maxlimbs = 19 (approximately 303 decimal digits)
	static_assert(maxlimbs <= 19,
		"ereal<maxlimbs>: maxlimbs must be <= 19 to maintain algorithmic correctness. "
		"Larger values cause the last limb to underflow below DBL_MIN, violating the "
		"non-overlapping property required by Shewchuk's expansion arithmetic. "
		"This results in incorrect two_sum/two_product operations and silent arithmetic errors.");

	// Partial-constexpr surface (issue #750): ereal carries a
	// std::vector<double> _limb member, so any non-empty digit storage
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
	//   * parse() / to_string() / to_digits() - std::frexp, regex,
	//                                           stringstream
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

	// Component access
	constexpr double  operator[](size_t i) const noexcept { return _limb[i]; }
	constexpr double& operator[](size_t i) { return _limb[i]; }

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
	ereal& operator+=(const ereal& rhs) {
		using namespace expansion_ops;
		if (apply_ieee754_add_special_values(rhs)) return *this;
		_limb = renormalize_expansion(linear_expansion_sum(_limb, rhs._limb));
		return *this;
	}
	ereal& operator+=(double rhs) {
		using namespace expansion_ops;
		ereal<maxlimbs> rhs_expansion(rhs);
		if (apply_ieee754_add_special_values(rhs_expansion)) return *this;
		_limb = renormalize_expansion(linear_expansion_sum(_limb, rhs_expansion._limb));
		return *this;
	}
	ereal& operator-=(const ereal& rhs) {
		using namespace expansion_ops;
		// Subtraction is a + (-b). Apply the special-value rules to the
		// effective sign-flipped RHS so e.g. (+Inf) - (-Inf) = +Inf + +Inf,
		// not (+Inf) + (-Inf) = NaN.
		ereal<maxlimbs> neg_rhs_e = -rhs;
		if (apply_ieee754_add_special_values(neg_rhs_e)) return *this;
		_limb = renormalize_expansion(linear_expansion_sum(_limb, neg_rhs_e._limb));
		return *this;
	}
	ereal& operator-=(double rhs) {
		return operator-=(ereal<maxlimbs>(rhs));
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
		return operator*=(ereal<maxlimbs>(rhs));
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
		return operator/=(ereal<maxlimbs>(rhs));
	}

	// modifiers
	//  After a move (or any shrink-to-fit), the vector can have zero capacity, 
	//  so that push_back may allocate and throw std::bad_alloc. 
	//  Therefore, we cannot mark the functions noexcept. 
	//  The std::bad_alloc exception would trigger std::terminate.`
	void clear()                   { _limb.clear(); _limb.push_back(0.0); }
	void setzero()                 { clear(); }
	void setnan()                  { clear(); _limb[0] = std::numeric_limits<double>::quiet_NaN(); }
	void setinf(bool sign = false) { clear(); _limb[0] = (sign ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity()); }

	// Special value setters for numeric_limits support
	ereal& maxpos() {
		clear();
		// Maximum positive value: DBL_MAX plus additional components following 2^-53 scaling
		_limb[0] = 1.7976931348623157e+308;  // DBL_MAX = 2^1024 - 2^971
		if (maxlimbs >= 2) _limb.push_back(9.9792015476735972e+291);  // ≈ 2^971
		if (maxlimbs >= 3) _limb.push_back(5.5395696628011126e+275);  // ≈ 2^918
		if (maxlimbs >= 4) _limb.push_back(3.0750789988826854e+259);  // ≈ 2^865
		// For maxlimbs > 4, additional components would need to be computed
		// Each component follows: limb[i] ≈ limb[i-1] × 2^-53
		return *this;
	}

	ereal& minpos() {
		clear();
		// Minimum positive normalized value
		_limb[0] = std::numeric_limits<double>::min();  // DBL_MIN = 2^-1022
		return *this;
	}

	ereal& minneg() {
		clear();
		// Minimum negative normalized value (closest to zero from below)
		_limb[0] = -std::numeric_limits<double>::min();  // -DBL_MIN = -2^-1022
		return *this;
	}

	ereal& maxneg() {
		clear();
		// Maximum negative value: negative of maxpos components
		_limb[0] = -1.7976931348623157e+308;  // -DBL_MAX
		if (maxlimbs >= 2) _limb.push_back(-9.9792015476735972e+291);
		if (maxlimbs >= 3) _limb.push_back(-5.5395696628011126e+275);
		if (maxlimbs >= 4) _limb.push_back(-3.0750789988826854e+259);
		return *this;
	}

	// parse: convert a decimal string to ereal
	// Returns true on success, false on parse error (leaves *this unchanged)
	// Supports formats: "123", "3.14", "-1.23e-45", "1E+10"
	bool parse(const std::string& str) {
		if (str.empty()) return false;

		ereal<maxlimbs> result;
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

		// Parse mantissa digits
		bool found_digit = false;
		bool saw_exponent_marker = false;
		ereal<maxlimbs> ten(10.0);

		while (pos < str.length()) {
			char c = str[pos];

			if (std::isdigit(c)) {
				found_digit = true;
				int digit = c - '0';

				// result = result * 10 + digit
				result = result * ten;
				result = result + ereal<maxlimbs>(static_cast<double>(digit));

				if (decimal_point_seen) {
					++decimal_position;
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

		// Apply decimal point adjustment
		if (decimal_point_seen) {
			exponent -= decimal_position;
		}

		// Apply exponent using pown(10, exp) for integer powers
		// pown uses repeated squaring and maintains full precision
		if (exponent != 0) {
			ereal<maxlimbs> power_of_ten = pown(ten, exponent);
			result = result * power_of_ten;
		}

		// Apply sign
		if (negative) {
			result = -result;
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

				// Adaptive buffer size: maxlimbs * 16 approximates available decimal digits
				int minBuffer = static_cast<int>(maxlimbs) * 16;
				int nrDigitsForFixedFormat = nrDigits;
				if (fixed)
					nrDigitsForFixedFormat = std::max(minBuffer, nrDigits);

				if constexpr (bTraceDecimalConversion) {
					std::cout << "powerOfTenScale  : " << powerOfTenScale << '\n';
					std::cout << "integerDigits    : " << integerDigits   << '\n';
					std::cout << "nrDigits         : " << nrDigits        << '\n';
					std::cout << "nrDigitsForFixedFormat  : " << nrDigitsForFixedFormat << '\n';
				}

				// a number in the range of [0.5, 1.0) to be printed with zero precision
				// must be rounded up to 1 to print correctly
				// Use full ereal magnitude (not just _limb[0]) for correct multi-limb rounding
				{
				double fullMagnitude = std::fabs(static_cast<double>(*this));
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
	constexpr double           significant() const noexcept { return _limb.empty() ? 0.0 : _limb[0]; }
	constexpr const std::vector<double>& limbs()       const noexcept { return _limb; }

protected:
	std::vector<double> _limb;     // components of the real value

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
		}
		else {
			// TBD
		}
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	ereal& convert_unsigned(UnsignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			// TBD
		}
		return *this;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	ereal& convert_ieee754(Real rhs) noexcept {
		clear();
		_limb[0] = rhs;
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
		ereal<maxlimbs> r = sw::universal::abs(*this);
		const ereal<maxlimbs> _ten(10.0);
		const ereal<maxlimbs> _one(1.0);

		if (e < 0) {
			if (e < -300) {
				r = ldexp(r, 53);
				r *= pown(_ten, -e);
				r = ldexp(r, -53);
			}
			else {
				r *= pown(_ten, -e);
			}
		}
		else {
			if (e > 0) {
				if (e > 300) {
					r = ldexp(r, -53);
					r /= pown(_ten, e);
					r = ldexp(r, 53);
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
			std::cerr << "ereal::to_digits() failed to compute exponent\n";
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
			if constexpr (bTraceDecimalConversion) std::cout << "to_digits  digit[" << i << "] : " << s.data() << '\n';
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
			std::cerr << "ereal::to_digits() non-positive leading digit\n";
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
			std::cout << "string       : " << s.data() << '\n';
			std::cout << "precision    : " << precision << '\n';
			std::cout << "decimalPoint : " << *decimalPoint << '\n';
		}

		int nrDigits = precision;
		// round decimal string and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			if constexpr (bTraceDecimalRounding) std::cout << "need to round\n";
			int i = nrDigits - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}

		// if first digit is 10, shift everything.
		if (s[0] > '9') {
			if constexpr (bTraceDecimalRounding) std::cout << "shift right to handle overflow\n";
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
	template<unsigned nnlimbs>
	friend signed findMsb(const ereal<nnlimbs>& v);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////    ereal functions   /////////////////////////////////

template<unsigned nlimbs>
inline ereal<nlimbs> abs(const ereal<nlimbs>& a) {
	return (a < 0 ? -a : a);
}

////////////////////////////////////////////////////////////////////////////////
/// stream operators

// read a ereal ASCII format and make a binary ereal out of it
template<unsigned nlimbs>
bool parse(const std::string& txt, ereal<nlimbs>& value) {
	return value.parse(txt);
}

// generate an ereal format ASCII format
template<unsigned nlimbs>
inline std::ostream& operator<<(std::ostream& ostr, const ereal<nlimbs>& rhs) {
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize precision = ostr.precision();
	std::streamsize width = ostr.width();
	char fillChar = ostr.fill();
	bool showpos    = fmt & std::ios_base::showpos;
	bool uppercase  = fmt & std::ios_base::uppercase;
	bool fixed      = fmt & std::ios_base::fixed;
	bool scientific = fmt & std::ios_base::scientific;
	bool internal   = fmt & std::ios_base::internal;
	bool left       = fmt & std::ios_base::left;
	return ostr << rhs.to_string(precision, width, fixed, scientific,
	                              internal, left, showpos, uppercase, fillChar);
}

// read an ASCII ereal format
template<unsigned nlimbs>
inline std::istream& operator>>(std::istream& istr, ereal<nlimbs>& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into an ereal value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

////////////////// string operators


//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - ereal binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<unsigned nlimbs>
inline bool operator==(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	using namespace expansion_ops;
	return compare_adaptive(lhs.limbs(), rhs.limbs()) == 0;
}
template<unsigned nlimbs>
inline bool operator!=(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	using namespace expansion_ops;
	return compare_adaptive(lhs.limbs(), rhs.limbs()) < 0;
}
template<unsigned nlimbs>
inline bool operator> (const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
inline bool operator<=(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nlimbs>
inline bool operator==(const ereal<nlimbs>& lhs, double rhs) {
	return operator==(lhs, ereal<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline bool operator!=(const ereal<nlimbs>& lhs, double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (const ereal<nlimbs>& lhs, double rhs) {
	return operator<(lhs, ereal<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline bool operator> (const ereal<nlimbs>& lhs, double rhs) {
	return operator< (ereal<nlimbs>(rhs), lhs);
}
template<unsigned nlimbs>
inline bool operator<=(const ereal<nlimbs>& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(const ereal<nlimbs>& lhs, double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - ereal binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned nlimbs>
inline bool operator==(double lhs, const ereal<nlimbs>& rhs) {
	return operator==(ereal<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline bool operator!=(double lhs, const ereal<nlimbs>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (double lhs, const ereal<nlimbs>& rhs) {
	return operator<(ereal<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline bool operator> (double lhs, const ereal<nlimbs>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
inline bool operator<=(double lhs, const ereal<nlimbs>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(double lhs, const ereal<nlimbs>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - ereal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline ereal<nlimbs> operator+(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	ereal<nlimbs> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline ereal<nlimbs> operator-(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	ereal<nlimbs> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline ereal<nlimbs> operator*(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	ereal<nlimbs> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nlimbs>
inline ereal<nlimbs> operator/(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	ereal<nlimbs> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - literal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline ereal<nlimbs> operator+(const ereal<nlimbs>& lhs, double rhs) {
	return operator+(lhs, ereal<nlimbs>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline ereal<nlimbs> operator-(const ereal<nlimbs>& lhs, double rhs) {
	return operator-(lhs, ereal<nlimbs>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline ereal<nlimbs> operator*(const ereal<nlimbs>& lhs, double rhs) {
	return operator*(lhs, ereal<nlimbs>(rhs));
}
// BINARY DIVISION
template<unsigned nlimbs>
inline ereal<nlimbs> operator/(const ereal<nlimbs>& lhs, double rhs) {
	return operator/(lhs, ereal<nlimbs>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - ereal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline ereal<nlimbs> operator+(double lhs, const ereal<nlimbs>& rhs) {
	return operator+(ereal<nlimbs>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline ereal<nlimbs> operator-(double lhs, const ereal<nlimbs>& rhs) {
	return operator-(ereal<nlimbs>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline ereal<nlimbs> operator*(double lhs, const ereal<nlimbs>& rhs) {
	return operator*(ereal<nlimbs>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nlimbs>
inline ereal<nlimbs> operator/(double lhs, const ereal<nlimbs>& rhs) {
	return operator/(ereal<nlimbs>(lhs), rhs);
}

}} // namespace sw::universal
