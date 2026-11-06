#pragma once
#include <iosfwd>       // std::ostream/std::istream in the friend declarations (#1334)
#include <string>       // std::string, returned by the text layer's builders
// faithful_impl.hpp: definition of a faithfully rounded number system
//
// Copyright (C) 2023-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <cmath>        // std::isinf, std::isnan
#include <concepts>     // std::convertible_to
#include <limits>
#include <type_traits>  // std::is_floating_point_v

#include <universal/native/ieee754_core.hpp>          // the bit-manipulation half (#1334)
#include <universal/native/manipulators_core.hpp>   // scale(), without the text layer
#include <universal/internal/abstract/triple.hpp>
#include <universal/numerics/eft.hpp>

namespace sw {	namespace universal {
		
// Forward definitions
template<typename FloatingPointType> class faithful;
template<typename FloatingPointType> faithful<FloatingPointType> abs(const faithful<FloatingPointType>& v);

// convert a floating-point value to a specific faithful configuration. Semantically, p = v, return reference to p
template<unsigned nbits, typename BlockType, typename FloatingPointType>
inline faithful<FloatingPointType>& convert(const triple<nbits, BlockType>& v, faithful<FloatingPointType>& p) {
	if (v.iszero()) {
		p.setzero();
		return p;
	}
	if (v.isnan() || v.isinf()) {
		p.setnan();
		return p;
	}
	return p;
}

namespace faithful_detail {
	// 2^e in Real, exactly, by repeated doubling or halving: constexpr where std::ldexp is not.
	// Exact for every e from the smallest subnormal exponent to the largest finite one.
	template<typename Real> constexpr Real pow2(int e) {
		Real r(1);
		if (e >= 0) for (int i = 0; i < e; ++i) r *= Real(2);
		else        for (int i = 0; i < -e; ++i) r /= Real(2);
		return r;
	}
}

// The extremes of a faithful<Real> (#1949). A faithful is a value of Real plus an error, so the
// smallest positive value is Real's subnormal minimum with no error. The largest is Real's maximum
// plus the largest error that does not round the sum up to infinity: max * 2^-(digits + 1), just
// below half an ulp of max. dd's max() is built the same way (#1882). These used to return their
// argument unchanged, a default zero.
template<typename FloatingPointType>
constexpr faithful<FloatingPointType>& minpos(faithful<FloatingPointType>& lminpos) {
	lminpos = faithful<FloatingPointType>(std::numeric_limits<FloatingPointType>::denorm_min(), FloatingPointType(0));
	return lminpos;
}
template<typename FloatingPointType>
constexpr faithful<FloatingPointType>& maxpos(faithful<FloatingPointType>& lmaxpos) {
	constexpr FloatingPointType top = std::numeric_limits<FloatingPointType>::max();
	lmaxpos = faithful<FloatingPointType>(top, top * faithful_detail::pow2<FloatingPointType>(-(std::numeric_limits<FloatingPointType>::digits + 1)));
	return lmaxpos;
}
template<typename FloatingPointType>
constexpr faithful<FloatingPointType>& minneg(faithful<FloatingPointType>& lminneg) {
	faithful<FloatingPointType> p;
	lminneg = -minpos(p);
	return lminneg;
}
template<typename FloatingPointType>
constexpr faithful<FloatingPointType>& maxneg(faithful<FloatingPointType>& lmaxneg) {
	faithful<FloatingPointType> p;
	lmaxneg = -maxpos(p);
	return lmaxneg;
}

// The classification of a part, for a native FloatingPointType and for a Universal one such as
// cfloat. A native type uses <cmath>. A Universal type uses its own isinf() and isnan() members,
// which its core defines; its free isinf()/isnan() overloads live in its mathlib, which a caller
// of the core alone does not include. Anything else is found unqualified, by ADL. These are free
// functions because inside faithful an unqualified isinf() names faithful's own member.
namespace faithful_detail {
	template<typename Real> inline bool part_isinf(const Real& v) {
		if constexpr (std::is_floating_point_v<Real>) return std::isinf(v);
		else if constexpr (requires { { v.isinf() } -> std::convertible_to<bool>; }) return v.isinf();
		else { using std::isinf; return isinf(v); }
	}
	template<typename Real> inline bool part_isnan(const Real& v) {
		if constexpr (std::is_floating_point_v<Real>) return std::isnan(v);
		else if constexpr (requires { { v.isnan() } -> std::convertible_to<bool>; }) return v.isnan();
		else { using std::isnan; return isnan(v); }
	}
	template<typename Real> inline bool part_isfinite(const Real& v) { return !part_isinf(v) && !part_isnan(v); }
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<typename FloatingPointType = double>
class faithful {
public:
	constexpr faithful() : value{ 0 }, error{ 0 } {}
	constexpr faithful(FloatingPointType v, FloatingPointType e) : value{ v }, error{ e } {}

	constexpr faithful(const faithful&) = default;
	constexpr faithful(faithful&&) = default;

	constexpr faithful& operator=(const faithful&) = default;
	constexpr faithful& operator=(faithful&&) = default;

	// conversion from compensated to faithfully rounded
	explicit operator FloatingPointType() const { return value + error; }

	faithful(long long initial_value)          { *this = initial_value; }
	faithful(unsigned long long initial_value) { *this = initial_value; }
	faithful(FloatingPointType initial_value)  { *this = initial_value; }

	// assignment operators
	faithful& operator=(long long rhs) {
		value = static_cast<FloatingPointType>(rhs);
		error = static_cast<FloatingPointType>((long double)(rhs)-(long double)(value));
		return *this;
	}
	faithful& operator=(unsigned long long rhs) {
		value = static_cast<FloatingPointType>(rhs);
		error = static_cast<FloatingPointType>((long double)(rhs)-(long double)(value));
		return *this;
	}
	faithful& operator=(FloatingPointType rhs) {
		value = rhs;
		error = 0;
		return *this;
	}

	// arithmetic operators
	// prefix operator: the negation, leaving *this unchanged. It used to assign to the members of
	// a const object, so -x did not compile (#1930).
	constexpr faithful operator-() const {
		return faithful(-value, -error);
	}

	// in-place arithmetic assignment operators
	// The compensated operators carry each operand's error into the result's (#1930). With
	// x = a + ea and y = b + eb, and the exact roundoff r of the operation on the heads:
	//   x + y = s + (r + ea + eb)
	//   x - y = s + (r + ea - eb)
	//   x * y = p + (r + a*eb + b*ea)          dropping ea*eb, below the error's own precision
	//   x / y = d + (r + ea - d*eb) / b        to first order, with r = a - b*d from twoDiv
	// Those first-order forms need each error below half an ulp of its value. Cancellation breaks
	// that: (1 + 2^-24) - (1 - 2^-24) leaves value and error both 2^-24, and dividing by it gave
	// zero. So every result is renormalized, and the divisor of /= is too, since faithful(v, e)
	// can be built from any pair.
	faithful& operator+=(const faithful& rhs) {
		FloatingPointType a(value), b(rhs.value), s, r;
		twoSum(a, b, s, r);
		value = s;
		error += r + rhs.error;
		renormalize();
		return *this; 
	}
	faithful& operator+=(double rhs) { return *this += faithful(rhs); }
	faithful& operator-=(const faithful& rhs) {
		FloatingPointType a(value), b(-rhs.value), s, r;
		twoSum(a, b, s, r);
		value = s;
		error += r - rhs.error;
		renormalize();
		return *this; 
	}
	faithful& operator-=(double rhs) { return *this -= faithful(rhs); }
	faithful& operator*=(const faithful& rhs) {
		FloatingPointType a(value), b(rhs.value), p, r;
		twoProd(a, b, p, r);
		value = p;
		error = a*rhs.error + b*error + r;
		renormalize();
		return *this; 
	}
	faithful& operator*=(double rhs) { return *this *= faithful(rhs); }
	faithful& operator/=(const faithful& rhs) {
		faithful divisor(rhs);
		divisor.renormalize();
		FloatingPointType a(value), b(divisor.value), d, r;
		twoDiv(a, b, d, r);
		value = d;
		error = (error + r - d * divisor.error) / b;
		renormalize();
		return *this;
	}
	faithful& operator/=(double rhs) { return *this /= faithful(rhs); }

	// prefix/postfix operators: a compensated addition of one, so the roundoff of the head is kept
	// in the error (#1936)
	faithful& operator++() {
		return *this += faithful(FloatingPointType(1));
	}
	faithful operator++(int) {
		faithful tmp(*this);
		operator++();
		return tmp;
	}
	faithful& operator--() {
		return *this -= faithful(FloatingPointType(1));
	}
	faithful operator--(int) {
		faithful tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	void reset()   { value = 0; error = 0; }
	void setzero() { value = 0; error = 0; }
	void setnan()  { value = std::numeric_limits<FloatingPointType>::quiet_NaN(); error = 0; }
	void setinf(bool negative = false) {
		value = negative ? -std::numeric_limits<FloatingPointType>::infinity() : std::numeric_limits<FloatingPointType>::infinity();
		error = 0;
	}

	// selectors
	// The sign, the scale and zero are those of the represented value. Its rounded sum is the
	// value of the canonical pair, so a pair built with faithful(v, e) that is not canonical, such
	// as (1, -2), reports the sign of -1, not of its first part (#1936).
	inline bool isneg() const { return rounded() < FloatingPointType(0); }
	inline bool iszero() const { return rounded() == FloatingPointType(0); }
	// the head carries an infinity or a NaN; renormalize() keeps the error of such a value zero
	inline bool isinf() const { return faithful_detail::part_isinf(value); }
	inline bool isnan() const { return faithful_detail::part_isnan(value); }
	inline bool sign() const { return rounded() < FloatingPointType(0); }
	inline int scale() const { return sw::universal::scale(rounded()); }
	// the two parts: the rounded value and its compensation, as dd_cascade names them
	constexpr FloatingPointType high() const { return value; }
	constexpr FloatingPointType low()  const { return error; }

private:
	FloatingPointType value;
	FloatingPointType error;

	// make the pair canonical: value is value + error rounded, and error what remains, at most
	// half an ulp of value. twoSum is exact, so the represented value does not change.
	// An infinite or NaN value has no roundoff (#1936). twoSum(inf, x) leaves inf - inf = NaN as
	// its roundoff, which the operators add into the error before they get here; renormalizing
	// that pair would make the value NaN too, and value + error turned an infinity into a NaN.
	// So a non-finite value keeps itself and drops the error, and a sum that overflows does too.
	void renormalize() {
		if (!faithful_detail::part_isfinite(value)) {
			error = 0;
			return;
		}
		FloatingPointType s, r;
		twoSum(value, error, s, r);
		value = s;
		error = faithful_detail::part_isfinite(s) ? r : FloatingPointType(0);
	}

	// value + error rounded: the value of the canonical pair. A non-finite value keeps itself,
	// whatever the error of a pair built by hand.
	FloatingPointType rounded() const {
		return faithful_detail::part_isfinite(value) ? FloatingPointType(value + error) : value;
	}

	// the canonical form of a copy, for the comparisons
	faithful normalized() const {
		faithful c(*this);
		c.renormalize();
		return c;
	}

	// template parameters need names different from class template parameters (for gcc and clang)
	template<typename FPType>
	friend std::ostream& operator<< (std::ostream& ostr, const faithful<FPType>& r);
	template<typename FPType>
	friend std::istream& operator>> (std::istream& istr, faithful<FPType>& r);

	template<typename FPType>
	friend bool operator==(const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator!=(const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator< (const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator> (const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator<=(const faithful<FPType>& lhs, const faithful<FPType>& rhs);
	template<typename FPType>
	friend bool operator>=(const faithful<FPType>& lhs, const faithful<FPType>& rhs);
};

////////////////////// operators

// The comparisons order by the represented value, value + error (#1936). Both operands are
// renormalized first, which leaves each value the rounded sum and each error at most half an ulp
// of its value. Rounding preserves order, so a smaller value means a smaller sum, and only a tie
// of the values needs the errors. That is exact, with no rounding of value + error. A NaN
// compares unordered, as in IEEE-754: every comparison is false except !=.
template<typename FPType>
inline bool operator==(const faithful<FPType>& lhs, const faithful<FPType>& rhs) {
	const faithful<FPType> l = lhs.normalized(), r = rhs.normalized();
	return l.value == r.value && l.error == r.error;
}
template<typename FPType>
inline bool operator!=(const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return !operator==(lhs, rhs); }
template<typename FPType>
inline bool operator< (const faithful<FPType>& lhs, const faithful<FPType>& rhs) {
	const faithful<FPType> l = lhs.normalized(), r = rhs.normalized();
	if (l.value != r.value) return l.value < r.value;   // false when either is NaN
	return l.error < r.error;
}
template<typename FPType>
inline bool operator> (const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return  operator< (rhs, lhs); }
template<typename FPType>
inline bool operator<=(const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return operator< (lhs, rhs) || operator==(lhs, rhs); }
template<typename FPType>
inline bool operator>=(const faithful<FPType>& lhs, const faithful<FPType>& rhs) { return operator< (rhs, lhs) || operator==(lhs, rhs); }

// faithful - faithful binary arithmetic operators
// BINARY ADDITION
template<typename FloatingPointType>
inline faithful<FloatingPointType> operator+(const faithful<FloatingPointType>& lhs, const faithful<FloatingPointType>& rhs) {
	faithful<FloatingPointType> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<typename FloatingPointType>
inline faithful<FloatingPointType> operator-(const faithful<FloatingPointType>& lhs, const faithful<FloatingPointType>& rhs) {
	faithful<FloatingPointType> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<typename FloatingPointType>
inline faithful<FloatingPointType> operator*(const faithful<FloatingPointType>& lhs, const faithful<FloatingPointType>& rhs) {
	faithful<FloatingPointType> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<typename FloatingPointType>
inline faithful<FloatingPointType> operator/(const faithful<FloatingPointType>& lhs, const faithful<FloatingPointType>& rhs) {
	faithful<FloatingPointType> ratio(lhs);
	ratio /= rhs;
	return ratio;
}



/// Magnitude: both parts negated when the represented value is negative (#1936). sign() reads the
/// rounded value + error, which carries the sign of the represented value.
template<typename FloatingPointType>
faithful<FloatingPointType> abs(const faithful<FloatingPointType>& v) {
	if (v.iszero()) return faithful<FloatingPointType>(FloatingPointType(0), FloatingPointType(0));   // +0, also for -0
	return v.sign() ? -v : v;
}


}}  // namespace sw::universal
