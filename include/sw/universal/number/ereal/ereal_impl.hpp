#pragma once
// ereal_impl.hpp: implementation of an adaptive precision multi-component floating-point number system
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

	// Enforce algorithmic validity: two_sum/two_product require normal doubles
	// Maximum safe configuration is maxlimbs = 19 (approximately 303 decimal digits)
	static_assert(maxlimbs <= 19,
		"ereal<maxlimbs>: maxlimbs must be <= 19 to maintain algorithmic correctness. "
		"Larger values cause the last limb to underflow below DBL_MIN, violating the "
		"non-overlapping property required by Shewchuk's expansion arithmetic. "
		"This results in incorrect two_sum/two_product operations and silent arithmetic errors.");

	// constructor
	ereal() : _limb{ 0 } { }

	ereal(const ereal&) = default;
	ereal(ereal&&) = default;

	ereal& operator=(const ereal&) = default;
	ereal& operator=(ereal&&) = default;

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
	ereal& operator+=(const ereal& rhs) {
		using namespace expansion_ops;
		_limb = linear_expansion_sum(_limb, rhs._limb);
		return *this;
	}
	ereal& operator+=(double rhs) {
		using namespace expansion_ops;
		ereal<maxlimbs> rhs_expansion(rhs);
		_limb = linear_expansion_sum(_limb, rhs_expansion._limb);
		return *this;
	}
	ereal& operator-=(const ereal& rhs) {
		using namespace expansion_ops;
		// Negate rhs components and add
		std::vector<double> neg_rhs = rhs._limb;
		for (auto& v : neg_rhs) v = -v;
		_limb = linear_expansion_sum(_limb, neg_rhs);
		return *this;
	}
	ereal& operator-=(double rhs) {
		return operator-=(ereal<maxlimbs>(rhs));
	}
	ereal& operator*=(const ereal& rhs) {
		using namespace expansion_ops;
		_limb = expansion_product(_limb, rhs._limb);
		return *this;
	}
	ereal& operator*=(double rhs) {
		using namespace expansion_ops;
		_limb = scale_expansion(_limb, rhs);
		return *this;
	}
	ereal& operator/=(const ereal& rhs) {
		using namespace expansion_ops;
		_limb = expansion_quotient(_limb, rhs._limb);
		return *this;
	}
	ereal& operator/=(double rhs) {
		using namespace expansion_ops;
		ereal<maxlimbs> rhs_expansion(rhs);
		_limb = expansion_quotient(_limb, rhs_expansion._limb);
		return *this;
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
		while (pos < str.length() && std::isspace(str[pos])) ++pos;
		if (pos >= str.length()) return false;

		// Parse optional sign
		if (str[pos] == '+' || str[pos] == '-') {
			negative = (str[pos] == '-');
			++pos;
		}

		// Parse mantissa digits
		bool found_digit = false;
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
				++pos;  // Move past 'e'/'E'
				break;  // Start exponent parsing
			}
			else {
				return false;  // Invalid character
			}

			++pos;
		}

		if (!found_digit) return false;

		// Parse optional exponent
		if (pos < str.length() && (str[pos - 1] == 'e' || str[pos - 1] == 'E')) {
			bool exp_negative = false;

			// Parse exponent sign
			if (pos < str.length() && (str[pos] == '+' || str[pos] == '-')) {
				exp_negative = (str[pos] == '-');
				++pos;
			}

			// Parse exponent digits
			bool found_exp_digit = false;
			while (pos < str.length() && std::isdigit(str[pos])) {
				found_exp_digit = true;
				exponent = exponent * 10 + (str[pos] - '0');
				++pos;
			}

			if (!found_exp_digit) return false;
			if (exp_negative) exponent = -exponent;
		}

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

	// selectors
	bool iszero()  const noexcept { return _limb[0] == 0.0; }  // do we need to check that we should only have one limb?
	bool isone()   const noexcept { return _limb[0] == 1.0; }
	bool ispos()   const noexcept { return _limb[0] > 0.0; }
	bool isneg()   const noexcept { return _limb[0] < 0.0; }
	bool isinf()   const noexcept { return sw::universal::isinf(_limb[0]); }
	bool isnan()   const noexcept { return sw::universal::isnan(_limb[0]); }

	// value information selectors
	bool                       signbit()     const noexcept { return std::signbit(_limb[0]); }
	int                        sign()        const noexcept { return (isneg() ? -1 : 1); }
	int64_t                    scale()       const noexcept { return sw::universal::scale(_limb[0]); }
	double                     significant() const noexcept { return _limb[0]; }
	const std::vector<double>& limbs()       const noexcept { return _limb; }

protected:
	std::vector<double> _limb;     // components of the real value

	// HELPER methods

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
		// Sum all components to get the full value
		Real sum = 0.0;
		for (const auto& component : _limb) {
			sum += static_cast<Real>(component);
		}
		return sum;
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
	// to make certain that setw and left/right operators work properly
	// we need to transform the ereal into a string
	std::stringstream ss;

	if (rhs.isinf()) {
		ss << (rhs.sign() == -1 ? "-inf" : "+inf");
	}
	else if (rhs.isnan()) {
		ss << "nan";
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

// read an ASCII ereal format
template<unsigned nlimbs>
inline std::istream& operator>>(std::istream& istr, ereal<nlimbs>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a floating-point value\n";
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
