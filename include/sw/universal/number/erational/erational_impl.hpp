#pragma once
// erational_impl.hpp: implementation of adaptive precision decimal erational arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <cmath>
#include <sstream>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <limits>
#include <regex>
#include <algorithm>

#include <universal/native/ieee754.hpp>
#include <universal/string/strmanip.hpp>
#include <universal/number/erational/exceptions.hpp>
#include <universal/number/edecimal/edecimal.hpp>

namespace sw { namespace universal {

/// <summary>
/// Adaptive precision rational number system type
/// </summary>
/// The erational is comprised of two adaptive precision decimals representing the numerator and denominator.
/// The digits of both are managed as a vector with the digit for 10^0 stored at index 0, 10^1 stored at index 1, etc.
class erational {
public:
	// Partial-constexpr surface (issue #749): erational is composed of
	// two edecimal members (numerator and denominator), each carrying a
	// std::vector<uint8_t> digit storage.  Under C++20's transient-
	// allocation rule, any non-empty digit storage escapes constant
	// evaluation, so the constexpr surface mirrors edecimal's (per
	// PR #824): default ctor + selectors that read trivial / empty-
	// constexpr-clean members + sign-only modifiers.  Arithmetic and
	// free comparison operators chain through edecimal arithmetic,
	// which itself remains non-constexpr (depends on push_back/insert),
	// so they are out of scope here.
	//
	// Default ctor uses std::is_constant_evaluated() to keep parallel
	// invariants: at constant evaluation the edecimal members stay
	// default-constructed (empty digit vectors, recognized as zero by
	// edecimal::iszero); at runtime setzero() restores the historical
	// "0/1" representation that arithmetic and the existing comparison
	// operators rely on.
	// Not noexcept: the runtime branch calls setzero() -> edecimal::operator=
	// which calls push_back, and that may throw std::bad_alloc.  Constexpr
	// invocation never throws (the empty-vector path doesn't allocate),
	// but the function signature must reflect the runtime contract.
	constexpr erational() : negative{ false }, numerator{}, denominator{} {
		if (!std::is_constant_evaluated()) {
			setzero();
		}
	}

	constexpr erational(const erational&) = default;
	constexpr erational(erational&&) = default;

	constexpr erational& operator=(const erational&) = default;
	constexpr erational& operator=(erational&&) = default;

	erational(std::int64_t n, std::uint64_t d) : negative{ false }, numerator { n }, denominator{ d } {
		negative = ((n < 0) ^ (d < 0));
	}

	// initializers for native types
	erational(char initial_value)               { *this = initial_value; }
	erational(short initial_value)              { *this = initial_value; }
	erational(int initial_value)                { *this = initial_value; }
	erational(long initial_value)               { *this = initial_value; }
	erational(long long initial_value)          { *this = initial_value; }
	erational(unsigned char initial_value)      { *this = initial_value; }
	erational(unsigned short initial_value)     { *this = initial_value; }
	erational(unsigned int initial_value)       { *this = initial_value; }
	erational(unsigned long initial_value)      { *this = initial_value; }
	erational(unsigned long long initial_value) { *this = initial_value; }
	erational(float initial_value)              { *this = initial_value; }
	erational(double initial_value)             { *this = initial_value; }


	// assignment operators for native types
	erational& operator=(const std::string& digits) {
		parse(digits);
		return *this;
	}
	erational& operator=(signed char rhs)        { return convert_signed(rhs); }
	erational& operator=(short rhs)              { return convert_signed(rhs); }
	erational& operator=(int rhs)                { return convert_signed(rhs); }
	erational& operator=(long rhs)               { return convert_signed(rhs); }
	erational& operator=(long long rhs)          { return convert_signed(rhs); }
	erational& operator=(unsigned char rhs)      { return convert_unsigned(rhs); }
	erational& operator=(unsigned short rhs)     { return convert_unsigned(rhs); }
	erational& operator=(unsigned int rhs)       { return convert_unsigned(rhs); }
	erational& operator=(unsigned long rhs)      { return convert_unsigned(rhs); }
	erational& operator=(unsigned long long rhs) { return convert_unsigned(rhs); }
	erational& operator=(float rhs)              { return convert_ieee754(rhs); }
	erational& operator=(double rhs)             { return convert_ieee754(rhs); }

	// explicit conversion operators 
	explicit operator unsigned short()     const noexcept { return to_unsigned<unsigned short>(); }
	explicit operator unsigned int()       const noexcept { return to_unsigned<unsigned int>(); }
	explicit operator unsigned long()      const noexcept { return to_unsigned<unsigned long>(); }
	explicit operator unsigned long long() const noexcept { return to_unsigned<unsigned long long>(); }
	explicit operator short()              const noexcept { return to_signed<short>(); }
	explicit operator int()                const noexcept { return to_signed<int>(); }
	explicit operator long()               const noexcept { return to_signed<long>(); }
	explicit operator long long()          const noexcept { return to_signed<long long>(); }
	explicit operator float()              const noexcept { return to_ieee754<float>(); }
	explicit operator double()             const noexcept { return to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	erational(long double initial_value) { *this = initial_value; }
	erational& operator=(long double rhs) { return convert_ieee754(rhs); }
	explicit operator long double()        const noexcept { return to_ieee754<long double>(); }
#endif

	// unitary operators
	erational operator-() const {
		erational tmp(*this);
		tmp.setsign(!tmp.sign());
		return tmp;
	}
	erational operator++(int) { // postfix
		erational tmp(*this);
		++numerator;
		return tmp;
	}
	erational& operator++() { // prefix
		++numerator;
		return *this;
	}
	erational operator--(int) { // postfix
		erational tmp(*this);
		--numerator;
		return tmp;
	}
	erational& operator--() { // prefix
		--numerator;
		return *this;
	}

	// arithmetic operators
	erational& operator+=(const erational& rhs) {
		edecimal a = (negative ? -numerator : numerator);
		edecimal b = denominator;
		edecimal c = (rhs.negative ? -rhs.numerator : rhs.numerator);
		edecimal d = rhs.denominator;
		if (denominator == rhs.denominator) {
			edecimal num = a + c;
			negative = num.isneg();
			numerator = (negative ? -num : num);
		}
		else {
			edecimal e = a * d + b * c;
			edecimal f = b * d;
			negative = e.isneg();
			numerator = (negative ? -e : e);
			denominator = f;
		}
		normalize();
		return *this;
	}
	erational& operator-=(const erational& rhs) {
		edecimal a = (negative ? -numerator : numerator);
		edecimal b = denominator;
		edecimal c = (rhs.negative ? -rhs.numerator : rhs.numerator);
		edecimal d = rhs.denominator;
		if (denominator == rhs.denominator) {
			edecimal num = a - c;
			negative = num.isneg();
			numerator = (negative ? -num : num);
		}
		else {
			edecimal e = a * d - b * c;
			edecimal f = b * d;
			negative = e.isneg();
			numerator = (negative ? -e : e);
			denominator = f;
		}
		normalize();
		return *this;
	}
	erational& operator*=(const erational& rhs) {
		numerator *= rhs.numerator;
		denominator *= rhs.denominator;
		negative = !((negative && rhs.negative) || (!negative && !rhs.negative));
		normalize();
		return *this;
	}
	erational& operator/=(const erational& rhs) {
		if (rhs.iszero()) {
#if ERATIONAL_THROW_ARITHMETIC_EXCEPTION
			throw erational_divide_by_zero();
#else
			std::cerr << "erational_divide_by_zero\n";
#endif
		}
		negative = !((negative && rhs.negative) || (!negative && !rhs.negative));
		numerator *= rhs.denominator;
		denominator *= rhs.numerator;
		normalize();
		return *this;
	}

	// selectors (delegate to edecimal selectors, which are constexpr-clean
	// on empty digit vectors per #824, or read trivial bool member)
	constexpr bool iszero()         const noexcept { return numerator.iszero(); }
	constexpr bool isneg()          const noexcept { return negative; }   // <  0
	constexpr bool ispos()          const noexcept { return !negative; }  // >= 0
	constexpr bool isinf()          const noexcept { return false; }
	constexpr bool isnan()          const noexcept { return numerator.iszero() && denominator.iszero(); }
	constexpr bool sign()           const noexcept { return negative; }
	constexpr edecimal top()        const noexcept { return numerator; }
	constexpr edecimal bottom()     const noexcept { return denominator; }
	std::pair<int64_t, int64_t> toPair() const noexcept {
		return { int64_t(numerator), int64_t(denominator) };
	}

	// modifiers
	void setzero() { 
		negative    = false;
		numerator   = 0;
		denominator = 1;
	}
	constexpr void setsign(bool sign) noexcept { negative = sign; }
	constexpr void setneg() noexcept { negative = true; }
	constexpr void setpos() noexcept { negative = false; }
	void setnumerator(const edecimal& num) { numerator = num; }
	void setdenominator(const edecimal& denom) { denominator = denom; }
	void setbits(uint64_t v) { *this = v; } // API to be consistent with the other number systems

	// read a erational ASCII format and make a erational type out of it
	// Parse a rational literal in any of these forms:
	//   integer:     "42",   "-1000"            -> 42/1, -1000/1
	//   p/q:         "1/2",  "-22/7", "355/113" -> simplified to lowest terms
	//   decimal:     "3.14", "-0.5"             -> 157/50, -1/2
	//   scientific:  "1.5e2", "1.5e-1"          -> 150/1, 3/20
	//   mixed p/q:   "3.14/2"                   -> 157/100
	// Rejects malformed input, division by zero ("1/0"), and inputs whose
	// expansion would exceed the underlying edecimal parse cap.
	bool parse(const std::string& _digits) {
		std::string digits(_digits);
		trim(digits);
		if (digits.empty()) return false;

		// Detect rational p/q split. A single '/' divides the input; any
		// further '/' is rejected by the per-half scan_decimal_float check.
		auto slash = digits.find('/');
		if (slash != std::string::npos) {
			std::string p_str = digits.substr(0, slash);
			std::string q_str = digits.substr(slash + 1);
			edecimal p_num, p_den, q_num, q_den;
			bool p_neg = false, q_neg = false;
			if (!parse_decimal_to_fraction(p_str, p_num, p_den, p_neg)) return false;
			if (!parse_decimal_to_fraction(q_str, q_num, q_den, q_neg)) return false;
			// q == 0 (e.g. "1/0", "1/0.0") is rejected: erational has no
			// NaR encoding, and silently representing infinity would mask
			// downstream divide-by-zero detection.
			if (q_num.iszero()) return false;
			// (p_num/p_den) / (q_num/q_den) = (p_num*q_den)/(p_den*q_num)
			numerator   = p_num * q_den;
			denominator = p_den * q_num;
			negative    = p_neg ^ q_neg;
		}
		else {
			edecimal num, den;
			bool neg = false;
			if (!parse_decimal_to_fraction(digits, num, den, neg)) return false;
			numerator   = num;
			denominator = den;
			negative    = neg;
		}

		// Reduce to lowest terms via GCD.
		normalize();
		// No negative zero.
		if (numerator.iszero()) negative = false;
		return true;
	}

private:
	// Tokenize a decimal/scientific literal and split it into a (numerator,
	// denominator) edecimal pair without precision loss.  Returns false on
	// malformed input or if expansion would exceed the safe digit cap.
	static bool parse_decimal_to_fraction(const std::string& s,
	                                      edecimal& num,
	                                      edecimal& den,
	                                      bool& neg) {
		// Same defensive cap as edecimal::parse uses internally.  We pre-check
		// here so we never allocate a giant intermediate string before
		// handing it off.
		constexpr std::size_t MAX_DIGITS = 1u << 20;  // 1,048,576

		std::string trimmed = s;
		trim(trimmed);
		if (trimmed.empty()) return false;

		auto scan = sw::universal::string_parse::scan_decimal_float(trimmed);
		if (!scan.valid) return false;
		neg = scan.negative;

		// Combined significand = int_part || frac_part (high-to-low digits).
		std::string sig;
		sig.reserve(scan.int_part.size() + scan.frac_part.size());
		sig.append(scan.int_part);
		sig.append(scan.frac_part);
		if (sig.empty()) return false;  // defensive: scan requires >=1 digit

		// Effective exponent: positive means trailing zeros on the numerator,
		// negative means the denominator is 10^|eff|.
		std::int64_t eff_exp = static_cast<std::int64_t>(scan.exp10)
		                     - static_cast<std::int64_t>(scan.frac_part.size());

		if (eff_exp >= 0) {
			std::uint64_t total = static_cast<std::uint64_t>(sig.size())
			                    + static_cast<std::uint64_t>(eff_exp);
			if (total > MAX_DIGITS) return false;
			sig.append(static_cast<std::size_t>(eff_exp), '0');
			if (!num.parse(sig)) return false;
			den = edecimal(1);
		}
		else {
			// numerator = sig (no expansion);
			// denominator = 10^|eff_exp| (a single 1 followed by |eff| zeros).
			std::uint64_t den_digits = static_cast<std::uint64_t>(-eff_exp) + 1u;
			if (sig.size() > MAX_DIGITS || den_digits > MAX_DIGITS) return false;
			if (!num.parse(sig)) return false;
			std::string den_str;
			den_str.reserve(static_cast<std::size_t>(den_digits));
			den_str.push_back('1');
			den_str.append(static_cast<std::size_t>(-eff_exp), '0');
			if (!den.parse(den_str)) return false;
		}
		return true;
	}

public:

protected:
	// HELPER methods

	// remove greatest common divisor out of the numerator/denominator pair
	void normalize() {
		edecimal a, b, r;
		a = numerator; b = denominator;  // precondition is numerator and denominator are positive

		if (b.iszero()) {
#if ERATIONAL_THROW_ARITHMETIC_EXCEPTION
			throw erational_divide_by_zero();
#else
			std::cerr << "erational_divide_by_zero\n";
			denominator = 0;
			numerator = 0;
#endif
		}
		while (a % b > 0) {
			r = a % b;
			a = b;
			b = r;
		}
		numerator /= b;
		denominator /= b;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////
	// conversion helpers

	// convert to signed int
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	SignedInt to_signed() const { return static_cast<SignedInt>(numerator / denominator); }
	// convert to unsigned int
	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	UnsignedInt to_unsigned() const { return static_cast<UnsignedInt>(numerator / denominator); }
	// convert to ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	Real to_ieee754() const { return Real(numerator) / Real(denominator); }

	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	erational& convert_signed(SignedInt& rhs) {
		if (rhs < 0) {
			negative = true;
			numerator = -rhs;
		}
		else {
			negative = false;
			numerator = rhs;
		}
		denominator = 1;
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type >
	erational& convert_unsigned(UnsignedInt& rhs) {
		negative  = false;
		numerator = rhs;
		denominator = 1;
		return *this;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	erational& convert_ieee754(Real rhs) noexcept {
		// A binary floating-point value is the exact dyadic rational
		//     (-1)^s * significand * 2^(e - bias - fbits)
		// where, for a normal value, significand = (fraction | hidden bit) and
		// e is the biased exponent; for a subnormal value there is no hidden
		// bit and the exponent is the minimum (e == 0 -> 1 - bias). Build the
		// numerator/denominator exactly as significand over a power of two,
		// then reduce. (Previously the exponent scaling was dropped entirely,
		// so every value collapsed into [1,2) and subnormals/zero were
		// unhandled -- issue #986.)
		numerator = 0;
		denominator = 1;
		negative = false;
		if (rhs == 0) return *this;                       // +/-0 -> 0/1
		if (std::isinf(rhs) || std::isnan(rhs)) return *this;  // not representable; map to 0

		uint64_t bits{ 0 }, e{ 0 }, f{ 0 };
		bool s{ false };
		extractFields(rhs, s, e, f, bits);
		negative = s;

		constexpr int bias  = ieee754_parameter<Real>::bias;
		constexpr int fbits = ieee754_parameter<Real>::fbits;
		uint64_t significand;
		int exp_pow;
		if (e == 0) { // subnormal: no hidden bit, exponent fixed at the minimum
			significand = f;
			exp_pow     = 1 - bias - fbits;
		}
		else {        // normal: restore the hidden bit
			significand = f | ieee754_parameter<Real>::hmask;
			exp_pow     = static_cast<int>(e) - bias - fbits;
		}

		// 2^k as an exact edecimal via square-and-multiply (k >= 0).
		auto pow2 = [](int k) {
			edecimal result(1), base(2);
			while (k > 0) {
				if (k & 1) result *= base;
				base *= base;
				k >>= 1;
			}
			return result;
		};

		edecimal sig(static_cast<long long>(significand));
		if (exp_pow >= 0) {
			numerator   = sig * pow2(exp_pow);
			denominator = 1;
		}
		else {
			numerator   = sig;
			denominator = pow2(-exp_pow);
		}
		normalize();
		return *this;
	}


private:
	// sign-magnitude number: indicate if number is positive or negative
	bool negative;
	edecimal numerator; // will be managed as a positive number
	edecimal denominator; // will be managed as a positive number

	friend std::ostream& operator<<(std::ostream& ostr, const erational& d);
	friend std::istream& operator>>(std::istream& istr, erational& d);

	// erational - erational logic operators
	friend bool operator==(const erational& lhs, const erational& rhs);
	friend bool operator!=(const erational& lhs, const erational& rhs);
	friend bool operator<(const erational& lhs, const erational& rhs);
	friend bool operator>(const erational& lhs, const erational& rhs);
	friend bool operator<=(const erational& lhs, const erational& rhs);
	friend bool operator>=(const erational& lhs, const erational& rhs);
};

////////////////// helper functions


////////////////// erational operators

/// stream operators

// generate an ASCII erational string
inline std::string to_string(const erational& d) {
	std::stringstream str;
	if (d.isneg()) str << '-';
	str << "TBD";
	return str.str();
}

// generate an ASCII erational format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const erational& d) {
	// make certain that setw and left/right operators work properly
	std::stringstream str;
	if (d.isneg()) str << '-';
	str << d.numerator << '/' << d.denominator;
	return ostr << str.str();
}

// read an ASCII erational format from an istream
inline std::istream& operator>>(std::istream& istr, erational& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into an erational value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

/// erational binary arithmetic operators

// binary addition of erational numbers
inline erational operator+(const erational& lhs, const erational& rhs) {
	erational sum = lhs;
	sum += rhs;
	return sum;
}
// binary subtraction of erational numbers
inline erational operator-(const erational& lhs, const erational& rhs) {
	erational diff = lhs;
	diff -= rhs;
	return diff;
}
// binary mulitplication of erational numbers
inline erational operator*(const erational& lhs, const erational& rhs) {
	erational mul = lhs;
	mul *= rhs;
	return mul;
}
// binary division of erational numbers
inline erational operator/(const erational& lhs, const erational& rhs) {
	erational ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////
/// logic operators

/// erational - erational logic operators

// equality test
bool operator==(const erational& lhs, const erational& rhs) {
	return lhs.numerator == rhs.numerator && lhs.denominator == rhs.denominator;
}
// inequality test
bool operator!=(const erational& lhs, const erational& rhs) {
	return !operator==(lhs, rhs);
}
// less-than test
bool operator<(const erational& lhs, const erational& rhs) {
	// a/b < c/d  => ad / bd < cb / bd => ad < cb
	edecimal ad = lhs.numerator * rhs.denominator;
	edecimal bc = lhs.denominator * rhs.numerator;
	return ad < bc;
}
// greater-than test
bool operator>(const erational& lhs, const erational& rhs) {
	return operator<(rhs, lhs);
}
// less-or-equal test
bool operator<=(const erational& lhs, const erational& rhs) {
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}
// greater-or-equal test
bool operator>=(const erational& lhs, const erational& rhs) {
	return !operator<(lhs, rhs);
}

// erational - long logic operators
inline bool operator==(const erational& lhs, long rhs) {
	return lhs == erational(rhs);
}
inline bool operator!=(const erational& lhs, long rhs) {
	return !operator==(lhs, erational(rhs));
}
inline bool operator< (const erational& lhs, long rhs) {
	return operator<(lhs, erational(rhs));
}
inline bool operator> (const erational& lhs, long rhs) {
	return operator< (erational(rhs), lhs);
}
inline bool operator<=(const erational& lhs, long rhs) {
	return operator< (lhs, erational(rhs)) || operator==(lhs, erational(rhs));
}
inline bool operator>=(const erational& lhs, long rhs) {
	return !operator<(lhs, erational(rhs));
}

// long - erational logic operators
inline bool operator==(long lhs, const erational& rhs) {
	return erational(lhs) == rhs;
}
inline bool operator!=(long lhs, const erational& rhs) {
	return !operator==(erational(lhs), rhs);
}
inline bool operator< (long lhs, const erational& rhs) {
	return operator<(erational(lhs), rhs);
}
inline bool operator> (long lhs, const erational& rhs) {
	return operator< (erational(lhs), rhs);
}
inline bool operator<=(long lhs, const erational& rhs) {
	return operator< (erational(lhs), rhs) || operator==(erational(lhs), rhs);
}
inline bool operator>=(long lhs, const erational& rhs) {
	return !operator<(erational(lhs), rhs);
}

///////////////////////////////////////////////////////////////////////


// find largest multiplier of rhs being less or equal to lhs by subtraction; assumes 0*rhs <= lhs <= 9*rhs 
erational findLargestMultiple_(const erational& lhs, const erational& rhs) {
	erational multiplier;

	return multiplier;
}


///////////////////////
// erationalntdiv_t for erational to capture quotient and remainder during long division
struct erationalintdiv {
	erational quot; // quotient
	erational rem;  // remainder
};

// divide integer erational a and b and return result argument
erationalintdiv erational_divide(const erational& lhs, const erational& rhs) {
	if (rhs.iszero()) {
#if ERATIONAL_THROW_ARITHMETIC_EXCEPTION
		throw erational_divide_by_zero{};
#else
		std::cerr << "erational_divide_by_zero\n";
#endif
	}

	// a/b / c/d => ad / bc
	erationalintdiv divresult;
	divresult.quot.setnumerator(lhs.top() * rhs.bottom());
	divresult.quot.setdenominator(lhs.bottom() * rhs.top());
	return divresult;
}

// return quotient of a erational integer division
erational quotient(const erational& _a, const erational& _b) {
	return erational_divide(_a, _b).quot;
}
// return remainder of a erational integer division
erational remainder(const erational& _a, const erational& _b) {
	return erational_divide(_a, _b).rem;
}

}} // namespace sw::universal

