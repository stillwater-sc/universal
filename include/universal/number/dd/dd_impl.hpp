#pragma once
// dd_impl.hpp: implementation of the double-double floating-point number system described in
// 
// Sherry Li, David Bailey, LBNL, "Library for Double-Double and Quad-Double Arithmetic", 2008
// https://www.researchgate.net/publication/228570156_Library_for_Double-Double_and_Quad-Double_Arithmetic
// 
// Adapted core subroutines from QD library by Yozo Hida
// 
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <vector>

// supporting types and functions
#include <universal/native/ieee754.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
// dd exception structure
#include <universal/number/dd/exceptions.hpp>
#include <universal/number/dd/dd_fwd.hpp>

namespace sw { namespace universal {

	// this is debug infrastructure: TODO: remove when decimal conversion is solved reliably
	constexpr bool bTraceDecimalConversion = false;
	constexpr bool bTraceDecimalRounding = false;
	std::ostream& operator<<(std::ostream& ostr, const std::vector<char>& s) {
		for (auto c : s) {
			ostr << c;
		}
		return ostr;
	}

// fwd references to free functions
dd operator-(const dd& lhs, const dd&);
dd operator*(const dd& lhs, const dd&);
std::ostream& operator<<(std::ostream&, const dd&);
dd pown(const dd&, int);
dd frexp(const dd&, int*);

// dd is an unevaluated pair of IEEE-754 doubles that provides a (1,11,106) floating-point triple
class dd {
public:
	static constexpr unsigned nbits = 128;
	static constexpr unsigned es = 11;
	static constexpr unsigned fbits = 106; // number of fraction digits
	// exponent characteristics are the same as native double precision floating-point
	static constexpr int      EXP_BIAS = ((1 << (es - 1u)) - 1l);
	static constexpr int      MAX_EXP = (es == 1) ? 1 : ((1 << es) - EXP_BIAS - 1);
	static constexpr int      MIN_EXP_NORMAL = 1 - EXP_BIAS;
	static constexpr int      MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP

	/// trivial constructor
	dd() = default;

	dd(const dd&) = default;
	dd(dd&&) = default;

	dd& operator=(const dd&) = default;
	dd& operator=(dd&&) = default;

	// converting constructors
	dd(const std::string& stringRep) : hi{0.0}, lo{0.0} { assign(stringRep); }

	// specific value constructor
	constexpr dd(const SpecificValue code) noexcept : hi{0.0}, lo{0.0} {
		switch (code) {
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			zero();
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
		case SpecificValue::nar: // approximation as dds don't have a NaR
		case SpecificValue::qnan:
			setnan(NAN_TYPE_QUIET);
			break;
		case SpecificValue::snan:
			setnan(NAN_TYPE_SIGNALLING);
			break;
		}
	}

	// raw limb constructor: no argument checking
	constexpr dd(double h, double l)                noexcept : hi{ h }, lo{ l } {}

	// initializers for native types
	constexpr dd(signed char iv)                    noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(short iv)                          noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(int iv)                            noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(long iv)                           noexcept { *this = iv; }
	constexpr dd(long long iv)                      noexcept { *this = iv; }
	constexpr dd(char iv)                           noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(unsigned short iv)                 noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(unsigned int iv)                   noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(unsigned long iv)                  noexcept { *this = iv; }
	constexpr dd(unsigned long long iv)             noexcept { *this = iv; }
	constexpr dd(float iv)                          noexcept : hi{ iv }, lo{ 0.0 } {}
	constexpr dd(double iv)                         noexcept : hi{ iv }, lo{ 0.0 } {}

	// assignment operators for native types
	constexpr dd& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr dd& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator int()                   const noexcept { return convert_to_signed<int>(); }
	explicit operator long()                  const noexcept { return convert_to_signed<long>(); }
	explicit operator long long()             const noexcept { return convert_to_signed<long long>(); }
	explicit operator unsigned int()          const noexcept { return convert_to_unsigned<unsigned int>(); }
	explicit operator unsigned long()         const noexcept { return convert_to_unsigned<unsigned long>(); }
	explicit operator unsigned long long()    const noexcept { return convert_to_unsigned<unsigned long long>(); }
	explicit operator float()                 const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double()                const noexcept { return convert_to_ieee754<double>(); }


#if LONG_DOUBLE_SUPPORT
	// can't be constexpr as remainder calculation requires volatile designation
			  dd(long double iv)                    noexcept { *this = iv; }
			  dd& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }
	explicit operator long double()           const noexcept { return convert_to_ieee754<long double>(); }
#endif

	// prefix operators
	constexpr dd operator-() const noexcept {
		dd negated(*this);
		negated.hi = -negated.hi;
		negated.lo = -negated.lo;
		return negated;
	}


	// arithmetic operators
	dd& operator+=(const dd& rhs) {
		double s2;
		hi = two_sum(hi, rhs.hi, s2);
		if (std::isfinite(hi)) {
			double t2, t1 = two_sum(lo, rhs.lo, t2);
			lo = two_sum(s2, t1, t1);
			t1 += t2;
			three_sum(hi, lo, t1);
		}
		else {
			lo = 0.0;
		}
		return *this;
	}
	dd& operator+=(double rhs) {
		return operator+=(dd(rhs));
	}
	dd& operator-=(const dd& rhs) {
		double s2;
		hi = two_sum(hi, -rhs.hi, s2);
		if (std::isfinite(hi)) {
			double t2, t1 = two_sum(lo, -rhs.lo, t2);
			lo = two_sum(s2, t1, t1);
			t1 += t2;
			three_sum(hi, lo, t1);
		}
		else {
			lo = 0.0;
		}
		return *this;
	}
	dd& operator-=(double rhs) {
		return operator-=(dd(rhs));
	}
	dd& operator*=(const dd& rhs) {
		double p[7];
		//	e powers in p = 0, 1, 1, 1, 2, 2, 2
		p[0] = two_prod(hi, rhs.hi, p[1]);
		if (std::isfinite(p[0])) {
			p[2] = two_prod(hi, rhs.lo, p[4]);
			p[3] = two_prod(lo, rhs.hi, p[5]);
			p[6] = lo * rhs.lo;

			//	e powers in p = 0, 1, 2, 3, 2, 2, 2
			three_sum(p[1], p[2], p[3]);

			//	e powers in p = 0, 1, 2, 3, 2, 3, 4
			p[2] += p[4] + p[5] + p[6];

			three_sum(p[0], p[1], p[2]);

			hi = p[0];
			lo = p[1];
		}
		else {
			hi = p[0];
			lo = 0.0;
		}
		return *this;
	}
	dd& operator*=(double rhs) {
		return operator*=(dd(rhs));
	}
	dd& operator/=(const dd& rhs) {
		if (isnan()) return *this;

		if (rhs.isnan()) {
			*this = rhs;
			return *this;
		}

		if (rhs.iszero()) {
			if (iszero()) {
				*this = dd(SpecificValue::qnan);
			}
			else {
				// auto signA = std::copysign(1.0, hi);
				// auto signB = std::copysign(1.0, rhs.hi);
				// *this = (signA * signB) * dd(SpecificValue::infpos);
				*this = dd(SpecificValue::infpos);
			}
			return *this;
		}

		double q1 = hi / rhs.hi;  // approximate quotient
		if (std::isfinite(q1)) {
			dd r = fma(-q1, rhs, *this);

			double q2 = r.hi / rhs.hi;
			r = fma(-q2, rhs, r);

			double q3 = r.hi / rhs.hi;

			three_sum(q1, q2, q3);
			hi = q1;
			lo = q2;
		}
		else {
			hi = q1;
			lo = 0.0;
		}

		return *this;
	}
	dd& operator/=(double rhs) {
		return operator/=(dd(rhs));
	}

	// unary operators
	dd& operator++() {
		return *this;
	}
	dd operator++(int) {
		dd tmp(*this);
		operator++();
		return tmp;
	}
	dd& operator--() {
		return *this;
	}
	dd operator--(int) {
		dd tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear()                                         noexcept { hi = 0.0; lo = 0.0; }
	constexpr void setzero()                                       noexcept { hi = 0.0; lo = 0.0; }
	constexpr void setinf(bool sign = true)                        noexcept { hi = (sign ? -INFINITY : INFINITY); lo = 0.0; }
	constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING)       noexcept { hi = (NaNType == NAN_TYPE_SIGNALLING ? std::numeric_limits<double>::signaling_NaN() : std::numeric_limits<double>::quiet_NaN()); lo = 0.0; }
	constexpr void setsign(bool sign = true)                       noexcept { if (sign && hi > 0.0) hi = -hi; }

	constexpr void setbit(unsigned index, bool b = true)           noexcept {
		if (index < 64) { // set bit in lower limb
			sw::universal::setbit(lo, index, b);
		}
		else if (index < 128) { // set bit in upper limb
			sw::universal::setbit(hi, index-64, b);
		}
		else {
			// NOP if index out of bounds
		}
	}
	constexpr void setbits(uint64_t value)                         noexcept {
		hi = static_cast<double>(value);
		lo = 0.0;
	}
	
	// argument is not protected for speed
	double operator[](int index) const { return (index == 0 ? hi : lo); }
	double& operator[](int index) { return (index == 0 ? hi : lo); }

	// create specific number system values of interest
	constexpr dd& maxpos() noexcept {
		hi = 1.7976931348623157e+308;
		lo = 1.9958403095347196e+292;
		return *this;
	}
	constexpr dd& minpos() noexcept {
		hi = std::numeric_limits<double>::min();
		lo = 0.0f;
		return *this;
	}
	constexpr dd& zero() noexcept {
		// the zero value
		clear();
		return *this;
	}
	constexpr dd& minneg() noexcept {
		hi = -std::numeric_limits<double>::min();
		lo = 0.0f;
		return *this;
	}
	constexpr dd& maxneg() noexcept {
		hi = -1.7976931348623157e+308;
		lo = -1.9958403095347196e+292;
		return *this;
	}

	dd& assign(const std::string& txt) {
		dd v;
		if (parse(txt, v)) *this = v;
		return *this; // Is this what we want? when the string is not valid, keep the current value?
	}

	// selectors
	constexpr bool iszero()   const noexcept { return hi == 0.0; }
	constexpr bool isone()    const noexcept { return hi == 1.0 && lo == 0.0; }
	constexpr bool ispos()    const noexcept { return hi > 0.0; }
	constexpr bool isneg()    const noexcept { return hi < 0.0; }
	BIT_CAST_CONSTEXPR bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		int nan_type;
		bool isNaN = checkNaN(hi, nan_type);
		bool isNegNaN = isNaN && negative;
		bool isPosNaN = isNaN && !negative;
		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
			(NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
				(NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}
	BIT_CAST_CONSTEXPR bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		int inf_type;
		bool isInf = checkInf(hi, inf_type);
		bool isNegInf = isInf && negative;
		bool isPosInf = isInf && !negative;
		return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
			(InfType == INF_TYPE_NEGATIVE ? isNegInf :
				(InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
	}

	constexpr bool sign()          const noexcept { return (hi < 0.0); }
	constexpr int  scale()         const noexcept { return _extractExponent<std::uint64_t, double>(hi); }
	constexpr int  exponent()      const noexcept { return _extractExponent<std::uint64_t, double>(hi); }
	constexpr double high()        const noexcept { return hi; }
	constexpr double low()         const noexcept { return lo; }

	// convert to string containing digits number of digits
	std::string to_string(std::streamsize precision = 7, std::streamsize width = 15, bool fixed = false, bool scientific = true, bool internal = false, bool left = false, bool showpos = false, bool uppercase = false, char fill = ' ') const {
		std::string s;
		bool negative = sign() ? true : false;
		int  e{ 0 };
		if (fixed && scientific) fixed = false; // scientific format takes precedence
		if (isnan()) {
			s = uppercase ? "NAN" : "nan";
			negative = false;
		}
		else {
			if (negative) {	s += '-'; } else { if (showpos) s += '+'; }

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
				int powerOfTenScale = static_cast<int>(std::log10(std::fabs(hi)));
				int integerDigits = (fixed ? (powerOfTenScale + 1) : 1);
				int nrDigits = integerDigits + static_cast<int>(precision);

				int nrDigitsForFixedFormat = nrDigits;
				if (fixed)
					nrDigitsForFixedFormat = std::max(60, nrDigits); // can be much longer than the max accuracy for double-double

				if constexpr (bTraceDecimalConversion) {
					std::cout << "powerOfTenScale  : " << powerOfTenScale << '\n';
					std::cout << "integerDigits    : " << integerDigits   << '\n';
					std::cout << "nrDigits         : " << nrDigits        << '\n';
					std::cout << "nrDigitsForFixedFormat  : " << nrDigitsForFixedFormat << '\n';
				}


				// a number in the range of [0.5, 1.0) to be printed with zero precision 
				// must be rounded up to 1 to print correctly
				if (fixed && (precision == 0) && (std::abs(hi) < 1.0)) {
					s += (std::abs(hi) >= 0.5) ? '1' : '0';
					return s;
				}

				if (fixed && nrDigits <= 0) {
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
						t.resize(nrDigitsForFixedFormat+1);
						to_digits(t, e, nrDigitsForFixedFormat);
					}
					else {
						t.resize(nrDigits+1);
						to_digits(t, e, nrDigits);
					}

					if (fixed) {
						// round the decimal string
						round_string(t, nrDigits+1, &integerDigits);

						if (integerDigits > 0) {
							int i;
							for (i = 0; i < integerDigits; ++i) s += t[i];
							if (precision > 0) {
								s += '.';
								for (int j = 0; j < precision; ++j, ++i) s += t[i];
							}
						}
						else {
							s += "0.";
							if (integerDigits < 0) s.append(static_cast<size_t>(-integerDigits), '0');
							for (int i = 0; i < nrDigits; ++i) s += t[i];
						}
					}
					else {
						s += t[0];
						if (precision > 0) s += '.';

						for (int i = 1; i <= precision; ++i)
							s += t[i];

					}
				}
			}

			// trap for improper offset with large values
			// without this trap, output of values of the for 10^j - 1 fail for j > 28
			// and are output with the point in the wrong place, leading to a dramatically off value
			if (fixed && (precision > 0)) {
				// make sure that the value isn't dramatically larger
				double from_string = atof(s.c_str());

				// if this ratio is large, then we've got problems
				if (std::fabs(from_string / this->hi) > 3.0) {

					// loop on the string, find the point, move it up one
					// don't act on the first character
					for (std::string::size_type i = 1; i < s.length(); ++i) {
						if (s[i] == '.') {
							s[i] = s[i - 1];
							s[i - 1] = '.';
							break;
						}
					}

					from_string = atof(s.c_str());
					// if this ratio is large, then the string has not been fixed
					if (std::fabs(from_string / this->hi) > 3.0) {
						//error("Re-rounding unsuccessful in large number fixed point trap.");
					}
				}
			}

			if (!fixed && !isinf()) {
				// construct the exponent
				s += uppercase ? 'E' : 'e';
				append_exponent(s, e);
			}
		}

		// process any fill
		size_t strLength = s.length();
		if (strLength < static_cast<size_t>(width)) {
			size_t nrCharsToFill = (width - strLength);
			if (internal) {
				if (negative)
					s.insert(static_cast<std::string::size_type>(1), nrCharsToFill, fill);
				else
					s.insert(static_cast<std::string::size_type>(0), nrCharsToFill, fill);
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

protected:
	double hi, lo;

	// HELPER methods

	constexpr dd& convert_signed(int64_t v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			hi = static_cast<double>(v);
			lo = static_cast<double>(v - static_cast<int64_t>(hi));
		}
		return *this;
	}

	constexpr dd& convert_unsigned(uint64_t v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			hi = static_cast<double>(v);
			lo = static_cast<double>(v - static_cast<uint64_t>(hi));  // difference is always positive
		}
		return *this;
	}

	// no need to SFINAE this as it is an internal method that we ONLY call when we know the argument type is a native float
	constexpr dd& convert_ieee754(float rhs) noexcept {
		hi = double(rhs);
		lo = 0.0;
		return *this;
	}
	constexpr dd& convert_ieee754(double rhs) noexcept {
		hi = rhs;
		lo = 0.0;
		return *this;
	}
#if LONG_DOUBLE_SUPPORT
	dd& convert_ieee754(long double rhs) {
		volatile long double truncated = static_cast<long double>(double(rhs));
		volatile double remainder = static_cast<double>(rhs - truncated);
		hi = static_cast<double>(truncated);
		lo = remainder;
		return *this;
	}
#endif

	// convert to native unsigned integer, use C++ conversion rules to cast down to float and double
	template<typename Unsigned>
	Unsigned convert_to_unsigned() const noexcept {
		int64_t h = static_cast<int64_t>(hi);
		int64_t l = static_cast<int64_t>(lo);
		return Unsigned(h + l);
	}
	
	// convert to native unsigned integer, use C++ conversion rules to cast down to float and double
	template<typename Signed>
	Signed convert_to_signed() const noexcept {
		int64_t h = static_cast<int64_t>(hi);
		int64_t l = static_cast<int64_t>(lo);
		return Signed(h + l);
	}

	// convert to native floating-point, use C++ conversion rules to cast down to float and double
	template<typename Real>
	Real convert_to_ieee754() const noexcept {
		return Real(hi + lo);
	}

	// precondition: string s must be all digits
	void round_string(std::vector<char>& s, int precision, int* decimalPoint) const {
		if constexpr(bTraceDecimalRounding) {
			std::cout << "string       : " << s << '\n';
			std::cout << "precision    : " << precision << '\n';
			std::cout << "decimalPoint : " << *decimalPoint << '\n';
		}

		int nrDigits = precision;
		// round decimal string and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[lastDigit] >= '5') {
			if constexpr(bTraceDecimalRounding) std::cout << "need to round\n";
			int i = nrDigits - 2;
			s[i]++;
			while (i > 0 && s[i] > '9') {
				s[i] -= 10;
				s[--i]++;
			}
		}

		// if first digit is 10, shift everything.
		if (s[0] > '9') {
			if constexpr(bTraceDecimalRounding) std::cout << "shift right to handle overflow\n";
			for (int i = precision; i >= 2; --i) s[i] = s[i - 1];
			s[0] = '1';
			s[1] = '0';

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

	/// <summary>
	/// to_digits generates the decimal digits representing
	/// </summary>
	/// <param name="s"></param>
	/// <param name="exponent"></param>
	/// <param name="precision"></param>
	//void to_digits(char* s, int& exponent, int precision) const {
	void to_digits(std::vector<char>& s, int& exponent, int precision) const {
		constexpr dd _one(1.0), _ten(10.0);
		constexpr double _log2(0.301029995663981);

		if (iszero()) {
			exponent = 0;
			for (int i = 0; i < precision; ++i) s[i] = '0';
			return;
		}

		// First determine the (approximate) exponent.
		// std::frexp(*this, &e);   // e is appropriate for 0.5 <= x < 1
		int e;
		std::frexp(hi, &e);	
		--e; // adjust e as frexp gives a binary e that is 1 too big
		e = static_cast<int>(_log2 * e); // estimate the power of ten exponent 
		dd r = abs(*this);
		if (e < 0) {
			if (e < -300) {
				r = dd(std::ldexp(r.high(), 53), std::ldexp(r.low(), 53));
				r *= pown(_ten, -e);
				r = dd(std::ldexp(r.high(), -53), std::ldexp(r.low(), -53));
			}
			else {
				r *= pown(_ten, -e);
			}
		}
		else {
			if (e > 0) {
				if (e > 300) {
					r = dd(std::ldexp(r.high(), -53), std::ldexp(r.low(), -53));
					r /= pown(_ten, e);
					r = dd(std::ldexp(r.high(), 53), std::ldexp(r.low(), 53));
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
			std::cerr << "to_digits() failed to compute exponent\n";
			return;
		}

		// at this point the value is normalized to a decimal value between (0, 10)
		// generate the digits
		int nrDigits = precision + 1;
		for (int i = 0; i < nrDigits; ++i) {
			int mostSignificantDigit = static_cast<int>(r.hi);
			r -= mostSignificantDigit;
			r *= 10.0;

			s[i] = static_cast<char>(mostSignificantDigit + '0');
			if constexpr (bTraceDecimalConversion) std::cout << "to_digits  digit[" << i << "] : " << s << '\n';
		}

		// Fix out of range digits
		for (int i = nrDigits - 1; i > 0; --i) {
			if (s[i] < '0') {
				s[i - 1]--;
				s[i] += 10;
			}
			else {
				if (s[i] > '9') {
					s[i - 1]++;
					s[i] -= 10;
				}
			}
		}

		if (s[0] <= '0') {
			std::cerr << "to_digits() non-positive leading digit\n";
			return;
		}

		// Round and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[lastDigit] >= '5') {
			int i = nrDigits - 2;
			s[i]++;
			while (i > 0 && s[i] > '9') {
				s[i] -= 10;
				s[--i]++;
			}
		}

		// If first digit is 10, shift left and increment exponent
		if (s[0] > '9') {
			++e;
			for (int i = precision; i >= 2; --i) {
				s[i] = s[i - 1];
			}
			s[0] = '1';
			s[1] = '0';
		}

		s[precision] = 0;  // termination null
		exponent = e;
	}

private:

	// dd - dd logic comparisons
	friend bool operator==(const dd& lhs, const dd& rhs);
	friend bool operator!=(const dd& lhs, const dd& rhs);
	friend bool operator<=(const dd& lhs, const dd& rhs);
	friend bool operator>=(const dd& lhs, const dd& rhs);
	friend bool operator<(const dd& lhs, const dd& rhs);
	friend bool operator>(const dd& lhs, const dd& rhs);

	// dd - literal logic comparisons
	friend bool operator==(const dd& lhs, const double rhs);

	// literal - dd logic comparisons
	friend bool operator==(const double lhs, const dd& rhs);

	friend bool operator<(const dd& lhs, const dd& rhs);

};

////////////////////////  precomputed constants of note  /////////////////////////////////

// precomputed double-double constants courtesy Scibuilders, Jack Poulson

constexpr dd dd_2pi   (6.283185307179586232e+00,  2.449293598294706414e-16);
constexpr dd dd_pi    (3.141592653589793116e+00,  1.224646799147353207e-16);
constexpr dd dd_pi2   (1.570796326794896558e+00,  6.123233995736766036e-17);
constexpr dd dd_pi4   (7.853981633974482790e-01,  3.061616997868383018e-17);
constexpr dd dd_3pi4  (2.356194490192344837e+00,  9.1848509936051484375e-17);
constexpr dd dd_e     (2.718281828459045091e+00,  1.445646891729250158e-16);
constexpr dd dd_log2  (6.931471805599452862e-01,  2.319046813846299558e-17);
constexpr dd dd_log10 (2.302585092994045901e+00, -2.170756223382249351e-16);

constexpr double dd_eps = 4.93038065763132e-32;  // 2^-104
constexpr double dd_min_normalized = 2.0041683600089728e-292;  // = 2^(-1022 + 53)
constexpr dd dd_max     (1.79769313486231570815e+308, 9.97920154767359795037e+291);
constexpr dd dd_safe_max(1.7976931080746007281e+308, 9.97920154767359795037e+291);


// precomputed double-double constants courtesy of constants example program, Theodore Omtzigt

constexpr dd dd_ln2      (0.69314718055994529e+00,  2.3190468138462996e-17);
constexpr dd dd_ln10     (2.30258509299404590e+00, -2.1707562233822494e-16);
constexpr dd dd_lge      (1.44269504088896340e+00,  2.0355273740931027e-17);
constexpr dd dd_lg10     (3.32192809488736220e+00,  1.6616175169735918e-16);
constexpr dd dd_loge     (0.43429448190325182e+00,  1.0983196502167652e-17);

constexpr dd dd_sqrt2    (1.41421356237309510e+00, -9.6672933134529122e-17);

constexpr dd dd_inv_pi   (0.31830988618379069e+00, -1.9678676675182486e-17);
constexpr dd dd_inv_pi2  (0.63661977236758138e+00, -3.9357353350364972e-17);
constexpr dd dd_inv_e    (0.36787944117144233e+00, -1.2428753672788364e-17);
constexpr dd dd_inv_sqrt2(0.70710678118654757e+00, -4.8336466567264561e-17);

////////////////////////    helper functions   /////////////////////////////////

inline std::string to_pair(const dd& v, int precision = 17) {
	std::stringstream s;
	// 53 bits = 16 decimal digits, 17 to include last, 15 typical valid digits
	s << std::setprecision(precision) << "( " << v.high() << ", " << v.low() << ')';
	return s.str();
}

inline std::string to_triple(const dd& v, int precision = 17) {
	std::stringstream s;
	bool isneg = v.isneg();
	int scale = v.scale();
	int exponent;
	dd fraction = frexp(v, &exponent);
	s << '(' << (isneg ? '1' : '0') << ", " << scale << ", " << std::setprecision(precision) << fraction << ')';
	return s.str();
}

inline std::string to_binary(const dd& number, bool bNibbleMarker = false) {
	std::stringstream s;
	constexpr int nrLimbs = 2;
	for (int i = 0; i < nrLimbs; ++i) {
		double_decoder decoder;
		decoder.d = number[i];

		std::string label = "x[" + std::to_string(i) + "]";
		s << label << " : ";
		s << "0b";
		// print sign bit
		s << (decoder.parts.sign ? '1' : '0') << '.';

		// print exponent bits
		{
			uint64_t mask = 0x400;
			for (int bit = 10; bit >= 0; --bit) {
				s << ((decoder.parts.exponent & mask) ? '1' : '0');
				if (bNibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
				mask >>= 1;
			}
		}

		s << '.';

		// print hi fraction bits
		uint64_t mask = (uint64_t(1) << 51);
		for (int bit = 51; bit >= 0; --bit) {
			s << ((decoder.parts.fraction & mask) ? '1' : '0');
			if (bNibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
			mask >>= 1;
		}

		if (i < 1) s << '\n';
	}

	return s.str();
}

////////////////////////    math functions   /////////////////////////////////

inline dd ulp(const dd& a) {
	double hi{ a.high() };
	double nlo = std::nextafter(a.low(), INFINITY);
	dd n(hi, nlo);

	return n - a;
}

inline dd abs(dd a) {
	double hi = a.high();
	double lo = a.low();
	if (hi < 0) { // flip the pair with respect to 0
		hi = -hi;
		lo = -lo;
	}
	return dd(hi, lo);
}

inline dd ceil(const dd& a)
{
	if (a.isnan()) return a;

	double hi = std::ceil(a.high());
	double lo = 0.0;

	if (hi == a.high())	{ // High segment was already an integer, thus just round the low segment
		lo = std::ceil(a.low());
		hi = quick_two_sum(hi, lo, lo);
	}

	return dd(hi, lo);
}

inline dd floor(const dd& a) {
	if (a.isnan()) return a;

	double hi = std::floor(a.high());
	double lo = 0.0;

	if (hi == a.high()) {
		// High word is integer already.  Round the low word.
		//
		lo = std::floor(a.low());
		hi = quick_two_sum(hi, lo, lo);
	}

	return dd(hi, lo);
}

// Round to Nearest integer
inline dd nint(const dd& a) {
	double hi = nint(a.high());
	double lo;

	if (hi == a.high()) {
		/* High word is an integer already.  Round the low word.*/
		lo = nint(a.low());

		/* Renormalize. This is needed if x[0] = some integer, x[1] = 1/2.*/
		hi = quick_two_sum(hi, lo, lo);
	}
	else {
		/* High word is not an integer. */
		lo = 0.0;
		if (std::abs(hi - a.high()) == 0.5 && a.low() < 0.0) {
			/* There is a tie in the high word, consult the low word
			   to break the tie. */
			hi -= 1.0;      /* NOTE: This does not cause INEXACT. */
		}
	}

	return dd(hi, lo);
}

// double plus double yielding a double-double
inline dd add(double a, double b) {
	if (std::isnan(a) || std::isnan(b)) return dd(SpecificValue::snan);
	double s, e;
	s = two_sum(a, b, e);
	return dd(s, e);
}

// double minus double yielding a double-double
inline dd sub(double a, double b) {
	if (std::isnan(a) || std::isnan(b)) return dd(SpecificValue::snan);
	double s, e;
	s = two_sum(a, -b, e);
	return dd(s, e);
}

// double times double yielding a double-double
inline dd mul(double a, double b) {
	if (std::isnan(a) || std::isnan(b)) return dd(SpecificValue::snan);
	double p, e;
	p = two_prod(a, b, e);
	return dd(p, e);
}

// double divide by double yielding a double-double
inline dd div(double a, double b) {
	if (std::isnan(a) || std::isnan(b)) return dd(SpecificValue::snan);

	if (b == 0.0) return (sign(a) ? dd(SpecificValue::infneg) : dd(SpecificValue::infpos));

	double q1 = a / b; // initial approximation

	// Compute residual: a - q1 * b
	volatile double p2;
	double p1 = two_prod(q1, b, p2);
	volatile double e;
	double s = two_diff(a, p1, e);
	e -= p2;

	// get next approximation
	double q2 = (s + e) / b;

	//	normalize
	s = quick_two_sum(q1, q2, e);
	return dd(s, e);
}

// double-double * double, where double is a power of 2
inline dd mul_pwr2(const dd& a, double b) {
	return dd(a.high() * b, a.low() * b);
}

// quad-double operators

// quad-double + double-double
void qd_add(double const a[4], const dd& b, double s[4]) {
	double t[5];
	s[0] = two_sum(a[0], b.high(), t[0]);		//	s0 - O( 1 ); t0 - O( e )
	s[1] = two_sum(a[1], b.low(), t[1]);		//	s1 - O( e ); t1 - O( e^2 )

	s[1] = two_sum(s[1], t[0], t[0]);		//	s1 - O( e ); t0 - O( e^2 )

	s[2] = a[2];									//	s2 - O( e^2 )
	three_sum(s[2], t[0], t[1]);		//	s2 - O( e^2 ); t0 - O( e^3 ); t1 = O( e^4 )

	s[3] = two_sum(a[3], t[0], t[0]);			//	s3 - O( e^3 ); t0 - O( e^4 )
	t[0] += t[1];									//	fl( t0 + t1 ) - accuracy less important

	renorm(s[0], s[1], s[2], s[3], t[0]);
}

// quad-double = double-double * double-double
void qd_mul(const dd& a, const dd& b, double p[4]) {
	double p4, p5, p6, p7;

	//	powers of e - 0, 1, 1, 1, 2, 2, 2, 3
	p[0] = two_prod(a.high(), b.high(), p[1]);
	if (std::isfinite(p[0])) {
		p[2] = two_prod(a.high(), b.low(), p4);
		p[3] = two_prod(a.low(), b.high(), p5);
		p6 = two_prod(a.low(), b.low(), p7);

		//	powers of e - 0, 1, 2, 3, 2, 2, 2, 3
		three_sum(p[1], p[2], p[3]);

		//	powers of e - 0, 1, 2, 3, 2, 3, 4, 3
		three_sum(p4, p5, p6);

		//	powers of e - 0, 1, 2, 3, 3, 3, 4, 3
		p[2] = two_sum(p[2], p4, p4);

		//	powers of e - 0, 1, 2, 3, 4, 5, 4, 3
		three_sum(p[3], p4, p5);

		//	powers of e - 0, 1, 2, 3, 4, 5, 4, 4
		p[3] = two_sum(p[3], p7, p7);

		p4 += (p6 + p7);

		renorm(p[0], p[1], p[2], p[3], p4);
	}
	else {
		p[1] = p[2] = p[3] = 0.0;
	}
}

inline dd fma(const dd& a, const dd& b, const dd& c) {
	double p[4];
	qd_mul(a, b, p);
	qd_add(p, c, p);
	p[0] = two_sum(p[0], p[1] + p[2] + p[3], p[1]);
	return dd(p[0], p[1]);
}

inline dd sqr(const dd& a) {
	if (a.isnan()) return a;

	double p2, p1 = two_sqr(a.high(), p2);
	p2 += 2.0 * a.high() * a.low();
	p2 += a.low() * a.low();

	double s2, s1 = quick_two_sum(p1, p2, s2);
	return dd(s1, s2);
}

inline dd reciprocal(const dd& a) {
	if (a.iszero()) return dd(SpecificValue::infpos);

	if (a.isinf()) return dd(0.0);

	double q1 = 1.0 / a.high();  /* approximate quotient */
	if (std::isfinite(q1)) {
		dd r = fma(-q1, a, 1.0);

		double q2 = r.high() / a.high();
		r = fma(-q2, a, r);

		double q3 = r.high() / a.high();
		three_sum(q1, q2, q3);
		return dd(q1, q2);
	}
	else {
		return dd(q1, 0.0);
	}
}

inline dd pown(const dd& a, int n) {
	if (a.isnan()) return a;

	int N = (n < 0) ? -n : n;
	dd s;

	switch (N) {
	case 0:
		if (a.iszero()) {
			std::cerr << "pown: invalid argument\n";
			errno = EDOM;
			return dd(SpecificValue::qnan);
		}
		return 1.0;

	case 1:
		s = a;
		break;

	case 2:
		s = sqr(a);

	default: // Use binary exponentiation
	{
		dd r{ a };

		s = 1.0;
		while (N > 0) {
			if (N % 2 == 1) {
				s *= r;
			}
			N /= 2;
			if (N > 0) r = sqr(r);
		}
	}
	break;
	}

	// Compute the reciprocal if n is negative.
	return n < 0 ? reciprocal(s) : s;
}

////////////////////////  stream operators   /////////////////////////////////

// stream out a decimal floating-point representation of the double-double
inline std::ostream& operator<<(std::ostream& ostr, const dd& v) {
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize precision = ostr.precision();
	std::streamsize width = ostr.width();
	char fillChar = ostr.fill();
	bool showpos = fmt & std::ios_base::showpos;
	bool uppercase = fmt & std::ios_base::uppercase;
	bool fixed = fmt & std::ios_base::fixed;
	bool scientific = fmt & std::ios_base::scientific;
	bool internal = fmt & std::ios_base::internal;
	bool left = fmt & std::ios_base::left;
	return ostr << v.to_string(precision, width, fixed, scientific, internal, left, showpos, uppercase, fillChar);
}

// stream in an ASCII decimal floating-point format and assign it to a double-double
inline std::istream& operator>>(std::istream& istr, dd& v) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a double-double value\n";
	}
	return istr;
}

////////////////// string operators

// parse a decimal ASCII floating-point format and make a doubledouble (dd) out of it
bool parse(const std::string& number, dd& value) {
	char const* p = number.c_str();

	// Skip any leading spaces
	while (std::isspace(*p)) ++p;

	dd r{ 0.0 };
	int nrDigits{ 0 };
	int decimalPoint{ -1 };
	int sign{ 0 }, eSign{ 1 };
	int e{ 0 };
	bool done{ false }, parsingMantissa{ true };
	char ch;
	while (!done && (ch = *p) != '\0') {
		if (std::isdigit(ch)) {
			if (parsingMantissa) {
				int digit = ch - '0';
				r *= 10.0;
				r += static_cast<double>(digit);
				++nrDigits;
			}
			else { // parsing exponent section
				int digit = ch - '0';
				e *= 10;
				e += digit;
			}
		}
		else {
			switch (ch) {
			case '.':
				if (decimalPoint >= 0) return false;
				decimalPoint = nrDigits;
				break;

			case '-':
			case '+':
				if (parsingMantissa) {
					if (sign != 0 || nrDigits > 0) return false;
					sign = (ch == '-' ? -1 : 1);
				}
				else {
					eSign = (ch == '-' ? -1 : 1);
				}
				break;

			case 'E':
			case 'e':
				parsingMantissa = false;
				break;

			default:
				return false;
			}
		}

		++p;
	}
	e *= eSign;

	if (decimalPoint >= 0) e -= (nrDigits - decimalPoint);
	dd _ten(10.0, 0.0);
	if (e > 0) {
		r *= pown(_ten, e);
	}
	else {
		if (e < 0) r /= pown(_ten, -e);
	}
	value = (sign == -1) ? -r : r;
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - dd binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
inline bool operator==(const dd& lhs, const dd& rhs) {
	return (lhs.hi == rhs.hi) && (lhs.lo == rhs.lo);
}

inline bool operator!=(const dd& lhs, const dd& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const dd& lhs, const dd& rhs) {
	if (lhs.hi < rhs.hi) {
		return true;
	}
	else if (lhs.hi > rhs.hi) {
		return false;
	}
	else {
		// hi limbs are the same
		if (lhs.lo < rhs.lo) {
			return true;
		}
		else if (lhs.lo > rhs.lo) {
			return false;
		}
		else {
			// lhs and rhs are the same
			return false;
		}
	}
}

inline bool operator> (const dd& lhs, const dd& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const dd& lhs, const dd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const dd& lhs, const dd& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
inline bool operator==(const dd& lhs, double rhs) {
	return operator==(lhs, dd(rhs));
}

inline bool operator!=(const dd& lhs, double rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const dd& lhs, double rhs) {
	return operator<(lhs, dd(rhs));
}

inline bool operator> (const dd& lhs, double rhs) {
	return operator< (dd(rhs), lhs);
}

inline bool operator<=(const dd& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const dd& lhs, double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dd binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(double lhs, const dd& rhs) {
	return operator==(dd(lhs), rhs);
}

inline bool operator!=(double lhs, const dd& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (double lhs, const dd& rhs) {
	return operator<(dd(lhs), rhs);
}

inline bool operator> (double lhs, const dd& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(double lhs, const dd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(double lhs, const dd& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - dd binary arithmetic operators
// BINARY ADDITION
inline dd operator+(const dd& lhs, const dd& rhs) {
	dd sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
inline dd operator-(const dd& lhs, const dd& rhs) {
	dd diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
inline dd operator*(const dd& lhs, const dd& rhs) {
	dd mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
inline dd operator/(const dd& lhs, const dd& rhs) {
	dd ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - literal binary arithmetic operators
// BINARY ADDITION
inline dd operator+(const dd& lhs, double rhs) {
	return operator+(lhs, dd(rhs));
}
// BINARY SUBTRACTION
inline dd operator-(const dd& lhs, double rhs) {
	return operator-(lhs, dd(rhs));
}
// BINARY MULTIPLICATION
inline dd operator*(const dd& lhs, double rhs) {
	return operator*(lhs, dd(rhs));
}
// BINARY DIVISION
inline dd operator/(const dd& lhs, double rhs) {
	return operator/(lhs, dd(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dd binary arithmetic operators
// BINARY ADDITION
inline dd operator+(double lhs, const dd& rhs) {
	return operator+(dd(lhs), rhs);
}
// BINARY SUBTRACTION
inline dd operator-(double lhs, const dd& rhs) {
	return operator-(dd(lhs), rhs);
}
// BINARY MULTIPLICATION
inline dd operator*(double lhs, const dd& rhs) {
	return operator*(dd(lhs), rhs);
}
// BINARY DIVISION
inline dd operator/(double lhs, const dd& rhs) {
	return operator/(dd(lhs), rhs);
}

}} // namespace sw::universal
