#pragma once
// dd_impl.hpp: implementation of a fixed-size, arbitrary configuration decimal floating-point number system
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

// supporting types and functions
#include <universal/native/ieee754.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
// dd exception structure
#include <universal/number/dd/exceptions.hpp>
#include <universal/number/dd/dd_fwd.hpp>

namespace sw { namespace universal {

// dd is an unevaluated pair of IEEE-754 doubles that provides a (1,11,106) floating-point triple
class dd {
public:
	static constexpr unsigned nbits = 128;
	static constexpr unsigned es = 11;
	static constexpr unsigned fbits = 106; // number of fraction digits

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
	constexpr dd(long double iv)                    noexcept { *this = iv; }

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
	constexpr dd& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }

	// prefix operators
	dd operator-() const noexcept {
		dd negated(*this);
		return negated;
	}

	// conversion operators
	explicit operator float() const { return toNativeFloatingPoint<float>(); }
	explicit operator double() const { return toNativeFloatingPoint<double>(); }
	explicit operator long double() const { return toNativeFloatingPoint<long double>(); }

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
	constexpr void setinf(bool sign = true)                        noexcept { hi = INFINITY; lo = 0.0; }
	constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING)       noexcept { hi = std::numeric_limits<double>::quiet_NaN(); lo = 0.0; }
	constexpr void setsign(bool sign = true)                       noexcept { }
	constexpr void setexponent(const std::string& expDigits)       noexcept { }
	constexpr void setfraction(const std::string& fracDigits)      noexcept { }

	constexpr void setbit(unsigned index, bool b = true)           noexcept {
		if (index < 64) {
			// set bit in lower limb
		}
		else if (index < 128) {
			// set bit in upper limb
		}
		else {
			// NOP if index out of bounds
		}
	}
	constexpr void setbits(uint64_t value)                         noexcept {
		hi = static_cast<double>(value);
		lo = 0.0;
	}
	
	// create specific number system values of interest
	constexpr dd& maxpos() noexcept {
		hi = 1.7976931348623157e+308;
		lo = 1.9958403095347196e+292;
		return *this;
	}
	constexpr dd& minpos() noexcept {
		hi = 1.0f;
		lo = 0.0f;
		return *this;
	}
	constexpr dd& zero() noexcept {
		// the zero value
		clear();
		return *this;
	}
	constexpr dd& minneg() noexcept {
		hi = 1.0f;
		lo = 0.0f;
		return *this;
	}
	constexpr dd& maxneg() noexcept {
		hi = 1.0f;
		lo = 0.0f;
		return *this;
	}

	dd& assign(const std::string& txt) {
		dd v;
		if (parse(txt, v)) *this = v;
		return *this; // Is this what we want? when the string is not valid, keep the current value?
	}

	// selectors
	constexpr bool iszero()   const noexcept { return false; }
	constexpr bool isone()    const noexcept { return true;  }
	constexpr bool isodd()    const noexcept { return false; }
	constexpr bool iseven()   const noexcept { return !isodd(); }
	constexpr bool ispos()    const noexcept { return false; }
	constexpr bool isneg()    const noexcept { return false; }
	constexpr bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		bool isNaN = false; // (_bits & 0x7F80u) && (_bits & 0x007F);
		bool isNegNaN = isNaN && negative;
		bool isPosNaN = isNaN && !negative;
		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
			(NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
				(NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}
	constexpr bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		bool isInf = false; // (_bits & 0x7F80u);
		bool isNegInf = isInf && negative;
		bool isPosInf = isInf && !negative;
		return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
			(InfType == INF_TYPE_NEGATIVE ? isNegInf :
				(InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
	}

	constexpr bool sign()     const noexcept { return false; }
	constexpr int  scale()    const noexcept { return 0; }
	constexpr int  exponent() const noexcept { return 0; }
	constexpr int  fraction() const noexcept { return 0; }
	constexpr double high()   const noexcept { return hi; }
	constexpr double low()    const noexcept { return lo; }

	void round_string(char* s, int precision, int* offset) const {
		/*
		 Input string must be all digits or errors will occur.
		 */

		int i;
		int D = precision;

		/* Round, handle carry */
		if (s[D - 1] >= '5') {
			s[D - 2]++;

			i = D - 2;
			while (i > 0 && s[i] > '9') {
				s[i] -= 10;
				s[--i]++;
			}
		}

		/* If first digit is 10, shift everything. */
		if (s[0] > '9') {
			// e++; // don't modify exponent here
			for (i = precision; i >= 2; i--) s[i] = s[i - 1];
			s[0] = '1';
			s[1] = '0';

			(*offset)++; // now offset needs to be increased by one
			precision++;
		}

		s[precision] = 0; // add terminator for array
	}

	void append_expn(std::string& str, int expn) const {
		int k;

		str += (expn < 0 ? '-' : '+');
		expn = std::abs(expn);

		if (expn >= 100)
		{
			k = (expn / 100);
			str += static_cast<char>('0' + k);
			expn -= 100 * k;
		}

		k = (expn / 10);
		str += static_cast<char>('0' + k);
		expn -= 10 * k;

		str += static_cast<char>('0' + expn);
	}

	// convert to string containing digits number of digits
	std::string to_string(std::streamsize precision, std::streamsize width, std::ios_base::fmtflags fmt, bool showpos, bool uppercase, char fill) const
	{
		std::string s;
		bool fixed = (fmt & std::ios_base::fixed) != 0;
		bool sgn = true;
		int i, e = 0;

		if (isnan()) {
			s = uppercase ? "NAN" : "nan";
			sgn = false;
		}
		else {
			if (sign())
				s += '-';
			else if (showpos)
				s += '+';
			else
				sgn = false;

			if (isinf()) {
				s += uppercase ? "INF" : "inf";
			}
			else if (*this == 0.0) {
				/* Zero case */
				s += '0';
				if (precision > 0) {
					s += '.';
					s.append(static_cast<unsigned int>(precision), '0');
				}
			}
			else {
				/* Non-zero case */
				//int off = (fixed ? (1 + floor(log10(abs(*this)))).toInt() : 1);
				int off = (fixed ? (1 + static_cast<int>(std::log10(std::fabs(hi)))) : 1);
				int d = static_cast<int>(precision) + off;

				int d_with_extra = d;
				if (fixed)
					d_with_extra = std::max(60, d); // longer than the max accuracy for DD

				// highly special case - fixed mode, precision is zero, abs(*this) < 1.0
				// without this trap a number like 0.9 printed fixed with 0 precision prints as 0
				// should be rounded to 1.
//				if (fixed && (precision == 0) && (abs(*this) < 1.0)) {
//					if (abs(*this) >= 0.5)
				if (fixed && (precision == 0) && (std::abs(high()) < 1.0)) {
					if (std::abs(high()) >= 0.5)

						s += '1';
					else
						s += '0';

					return s;
				}

				// handle near zero to working precision (but not exactly zero)
				if (fixed && d <= 0) {
					s += '0';
					if (precision > 0) {
						s += '.';
						s.append(static_cast<unsigned int>(precision), '0');
					}
				}
				else { // default

					char* t; //  = new char[d+1];
					int j;

					if (fixed) {
						t = new char[d_with_extra + 1];
						to_digits(t, e, d_with_extra);
					}
					else {
						t = new char[d + 1];
						to_digits(t, e, d);
					}

					if (fixed) {
						// fix the string if it's been computed incorrectly
						// round here in the decimal string if required
						round_string(t, d + 1, &off);

						if (off > 0) {
							for (i = 0; i < off; i++) s += t[i];
							if (precision > 0) {
								s += '.';
								for (j = 0; j < precision; j++, i++) s += t[i];
							}
						}
						else {
							s += "0.";
							if (off < 0) s.append(-off, '0');
							for (i = 0; i < d; i++) s += t[i];
						}
					}
					else {
						s += t[0];
						if (precision > 0) s += '.';

						for (i = 1; i <= precision; i++)
							s += t[i];

					}
					delete[] t;
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
					for (std::string::size_type i = 1; i < s.length(); i++)
					{
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
				/* Fill in exponent part */
				s += uppercase ? 'E' : 'e';
				append_expn(s, e);
			}
		}

		/* Fill in the blanks */
		int len = s.length();
		if (len < width) {
			int delta = static_cast<int>(width) - len;
			if (fmt & std::ios_base::internal) {
				if (sgn)
					s.insert(static_cast<std::string::size_type>(1), delta, fill);
				else
					s.insert(static_cast<std::string::size_type>(0), delta, fill);
			}
			else if (fmt & std::ios_base::left) {
				s.append(delta, fill);
			}
			else {
				s.insert(static_cast<std::string::size_type>(0), delta, fill);
			}
		}

		return s;
	}

	int toInt() {
		return toLongLong();
	}
	long toLong() {
		return toLongLong();
	}
	long long toLongLong() {
		//auto x = trunc(*this);
		return static_cast<long long>(hi) + static_cast<long long>(lo);
	}

protected:
	double hi, lo;

	// HELPER methods

	// convert to native floating-point, use conversion rules to cast down to float and double
	template<typename NativeFloat>
	NativeFloat toNativeFloatingPoint() const {
		return NativeFloat(hi);
	}

	constexpr dd& convert_signed(int64_t v) {
		if (0 == v) {
			setzero();
		}
		else {
			hi = static_cast<double>(v);
			lo = static_cast<double>(v - static_cast<int64_t>(hi));
		}
		return *this;
	}

	constexpr dd& convert_unsigned(uint64_t v) {
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
	template<typename NativeFloat>
	constexpr dd& convert_ieee754(NativeFloat& rhs) {
		hi = rhs;
		lo = 0.0;
		return *this;
	}

	void to_digits(char* s, int& expn, int precision) const {
		//int D = precision + 1;  // number of digits to compute
		//dd r = abs(*this);
		//int e;
		//int d;

		if (iszero()) {
			expn = 0;
			for (int i = 0; i < precision; ++i)
				s[i] = '0';
			return;
		}
#ifdef NOW_TO_DIGITS
		/* First determine the (approximate) exponent. */
		std::frexp(*this, &e); // e is appropriate for 0.5 <= x < 1
		std::ldexp(r, 1);      // adjust e, r
		--e;
		e = (_log2 * (double)e).toInt();

		if (e < 0) {
			if (e < -300) {
				r = std::ldexp(r, 53);
				r *= pown(_ten, -e);
				r = std::ldexp(r, -53);
			}
			else {
				r *= pown(_ten, -e);
			}
		}
		else
			if (e > 0) {
				if (e > 300) {
					r = std::ldexp(r, -53);
					r /= pown(_ten, e);
					r = std::ldexp(r, +53);
				}
				else {
					r /= pown(_ten, e);
				}
			}

		// Fix exponent if we are off by one
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
			error("(dd_real::to_digits): can't compute exponent.");
			return;
		}

		// Extract the digits
		for (int i = 0; i < D; ++i) {
			d = static_cast<int>(r.x[0]);
			r -= d;
			r *= 10.0;

			s[i] = static_cast<char>(d + '0');
		}

		// Fix out of range digits
		for (int i = D - 1; i > 0; --i) {
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
			//error("(dd::to_digits): non-positive leading digit.");
			return;
		}

		// Round, handle carry
		if (s[D - 1] >= '5') {
			s[D - 2]++;

			int i = D - 2;
			while (i > 0 && s[i] > '9') {
				s[i] -= 10;
				s[--i]++;
			}
		}

		// If first digit is 10, shift everything.
		if (s[0] > '9') {
			++e;
			for (int i = precision; i >= 2; --i) {
				s[i] = s[i - 1];
			}
			s[0] = '1';
			s[1] = '0';
		}

		s[precision] = 0;
		expn = e;
#endif // NOW_TO_DIGITS
	}

private:

	// dd - dd logic comparisons
	friend bool operator==(const dd& lhs, const dd& rhs);

	// dd - literal logic comparisons
	friend bool operator==(const dd& lhs, const double rhs);

	// literal - dd logic comparisons
	friend bool operator==(const double lhs, const dd& rhs);

	friend bool operator<(const dd& lhs, const dd& rhs);

};


////////////////////////    helper functions   /////////////////////////////////


inline std::string to_binary(const dd& number, bool bNibbleMarker = false) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number.high();

	s << "0b";
	// print sign bit
	s << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x400;
		for (int i = 10; i >= 0; --i) {
			s << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print hi fraction bits
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
		mask >>= 1;
	}

	if (bNibbleMarker) s << '\'';

	// print lo fraction bits
	decoder.d = number.low();
	mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
		mask >>= 1;
	}

	return s.str();
}

////////////////////////    math functions   /////////////////////////////////

inline dd abs(dd a) {
	double hi = a.high();
	if (hi < 0) hi = -hi;
	double lo = a.low();
	return dd(hi, lo);
}

//
//	rounding and remainder functions
//
inline dd ceil(dd const& a)
{
	if (a.isnan()) return a;

	double hi = std::ceil(a.high());
	double lo = 0.0;

	if (hi == a.high())	{
		/* High word is integer already.  Round the low word. */
		lo = std::ceil(a.low());
		hi = quick_two_sum(hi, lo, lo);
	}

	return dd(hi, lo);
}

inline dd floor(dd const& a) {
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

// quad-double operators

// quad-double + double-double
void qd_add(double const a[4], dd const& b, double s[4]) {
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
void qd_mul(dd const& a, dd const& b, double p[4]) {
	double p4, p5, p6, p7;

	//	powers of e - 0, 1, 1, 1, 2, 2, 2, 3
	p[0] = two_prod(a.high(), b.low(), p[1]);
	if (std::isfinite(p[0])) {
		p[2] = two_prod(a.high(), b.low(), p4);
		p[3] = two_prod(a.low(), b.low(), p5);
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

inline dd fma(dd const& a, dd const& b, dd const& c) {
	double p[4];
	qd_mul(a, b, p);
	qd_add(p, c, p);
	p[0] = two_sum(p[0], p[1] + p[2] + p[3], p[1]);
	return dd(p[0], p[1]);
}

inline dd sqr(dd const& a) {
	if (a.isnan()) return a;

	double p2, p1 = two_sqr(a.high(), p2);
	p2 += 2.0 * a.high() * a.low();
	p2 += a.low() * a.low();

	double s2, s1 = quick_two_sum(p1, p2, s2);
	return dd(s1, s2);
}

inline dd reciprocal(dd const& a) {
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

inline dd pown(dd const& a, int n) {
	if (a.isnan()) return a;

	int N = (n < 0) ? -n : n;
	dd s;

	switch (N) {
	case 0:
		if (a.iszero()) {
//			error("(dd_real::pown): Invalid argument.");
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


// generate an dd format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, const dd& v) {
	return ostr << "( " << v.high() << ", " << v.low() << ')';
}

// read an ASCII dd format
inline std::istream& operator>>(std::istream& istr, dd& v) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators

// read a decimal ASCII format and make a doubledouble (dd) out of it
bool parse(const std::string& number, dd& value) {
	char const* p = number.c_str();
	char ch;
	int sign = 0;
	int point = -1;
	int nd = 0;
	int e = 0;
	bool done = false;
	dd r = 0.0;
	int nread;

	/* Skip any leading spaces */
	while (std::isspace(*p))
		++p;

	while (!done && (ch = *p) != '\0') {
		if (std::isdigit(ch)) {
			int d = ch - '0';
			r *= 10.0;
			r += static_cast<double>(d);
			nd++;
		}
		else
		{
			switch (ch)
			{
			case '.':
				if (point >= 0)
					return false;
				point = nd;
				break;

			case '-':
			case '+':
				if (sign != 0 || nd > 0)
					return false;
				sign = (ch == '-') ? -1 : 1;
				break;

			case 'E':
			case 'e':
				nread = std::sscanf(p + 1, "%d", &e);
				done = true;
				if (nread != 1)
					return false;
				break;

			default:
				return false;
			}
		}

		++p;
	}

	if (point >= 0) e -= (nd - point);
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
inline bool operator==(const dd& lhs, const double rhs) {
	return operator==(lhs, dd(rhs));
}

inline bool operator!=(const dd& lhs, const double rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const dd& lhs, const double rhs) {
	return operator<(lhs, dd(rhs));
}

inline bool operator> (const dd& lhs, const double rhs) {
	return operator< (dd(rhs), lhs);
}

inline bool operator<=(const dd& lhs, const double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const dd& lhs, const double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dd binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const double lhs, const dd& rhs) {
	return operator==(dd(lhs), rhs);
}

inline bool operator!=(const double lhs, const dd& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const double lhs, const dd& rhs) {
	return operator<(dd(lhs), rhs);
}

inline bool operator> (const double lhs, const dd& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const double lhs, const dd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const double lhs, const dd& rhs) {
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