#pragma once
// triple.hpp: definition of an abstract (sign, scale, significant) representation of an approximation to a real triple
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>
#include <tuple>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {

// Forward definitions
template<size_t fbits, typename bt> class triple;
template<size_t fbits, typename bt> triple<fbits,bt> abs(const triple<fbits,bt>& v);

// template class representing an abstract real triple using a template size for the number of fraction bits
template<size_t fbits, typename bt>
class triple {
public:
	static constexpr size_t fhbits = fbits + 1;    // number of fraction bits including the hidden bit

	triple() : _sign{false}, _scale{0}, _fraction{0}, _inf{false}, _zero{true}, _nan{false} {}

	triple(bool sign, 
		   int scale, 
		   const blockbinary<fbits,bt>& fraction_without_hidden_bit, 
           bool zero = true, 
		   bool inf = false) 
      : _sign{sign}, 
		_scale{scale}, 
		_fraction{fraction_without_hidden_bit}, 
		_inf{inf}, 
		_zero{zero}, 
		_nan{false} {}


	triple(const triple&) = default;
	triple(triple&&) = default;

	triple& operator=(const triple&) = default;
	triple& operator=(triple&&) = default;

	// decorated constructors
	triple(signed char initial_triple)        { *this = initial_triple; }
	triple(short initial_triple)              { *this = initial_triple; }
	triple(int initial_triple)                { *this = initial_triple; }
	triple(long initial_triple)               { *this = initial_triple; }
	triple(long long initial_triple)          { *this = initial_triple; }
	triple(char initial_triple)               { *this = initial_triple; }
	triple(unsigned short initial_triple)     { *this = initial_triple; }
	triple(unsigned int initial_triple)       { *this = initial_triple; }
	triple(unsigned long initial_triple)      { *this = initial_triple; }
	triple(unsigned long long initial_triple) { *this = initial_triple; }
	triple(float initial_triple)              { *this = initial_triple; }
	triple(double initial_triple) : triple{}  { *this = initial_triple; }
	triple(long double initial_triple)        { *this = initial_triple; }

	constexpr triple& operator=(signed char rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr triple& operator=(short rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr triple& operator=(int rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr triple& operator=(long rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr triple& operator=(long long rhs) {
		return *this;
	}
	triple& operator=(char rhs) {
		return *this = (unsigned long long)(rhs);
	}
	triple& operator=(unsigned short rhs) {
		return *this = static_cast<long long>(rhs);
	}
	triple& operator=(unsigned int rhs) {
		return *this = static_cast<long long>(rhs);
	}
	triple& operator=(unsigned long rhs) {
		return *this = static_cast<long long>(rhs);
	}
	triple& operator=(unsigned long long rhs) {
		return *this;
	}
	triple& operator=(float rhs) {
		reset();

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_zero = true;
			break;
		case FP_INFINITE:
			_inf  = true;
			_sign = true;
			break;
		case FP_NAN:
			_nan = true;
			_sign = true;
			break;
		case FP_SUBNORMAL:
		case FP_NORMAL:
			{
			}
			break;
		}
		return *this;
	}
	triple& operator=(double rhs) {
                using std::get;
		reset();

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_zero = true;
			break;
		case FP_INFINITE:
			_inf = true;
			_sign = true;
			break;
		case FP_NAN:
			_nan = true;
			_sign = true;
			break;
		case FP_SUBNORMAL:
		case FP_NORMAL:
			{
			}
			break;
		}
		return *this;
	}
	triple& operator=(long double rhs) {
		reset();

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_zero = true;
			break;
		case FP_INFINITE:
			_inf = true;
			_sign = true;
			break;
		case FP_NAN:
			_nan = true;
			_sign = true;
			break;
		case FP_SUBNORMAL:
		case FP_NORMAL:
			{
			}
			break;
		}
		return *this;
	}

	// operators
	constexpr triple operator-() const {				
		return triple(!_sign, _scale, _fraction, _zero, _inf);
	}

	// modifiers
	constexpr void reset() & {
		_sign  = false;
		_scale = 0;
		_inf = false;
		_zero = false;
		_nan = false;
		_fraction.reset(); // not constexpr
                // _fraction= bitblock<fbits>{}; // work around
	}
	void set(bool sign, int scale, blockbinary<fbits,bt> fraction_without_hidden_bit, bool zero, bool inf, bool nan = false) {
		_sign     = sign;
		_scale    = scale;
		_fraction = fraction_without_hidden_bit;
		_zero     = zero;
		_inf      = inf;
		_nan      = nan;
	}
	void setzero() {
		_zero     = true;
		_sign     = false;
		_inf      = false;
		_nan      = false;
		_scale    = 0;
		_fraction.reset();
	}
	void setinf() {      // this maps to NaR on the posit side, and that has a sign = 1
		_inf      = true;
		_sign     = true;
		_zero     = false;
		_nan      = false;
		_scale    = 0;
		_fraction.reset();
	}
	void setnan() {		// this will also map to NaR
		_nan      = true;
		_sign     = true;
		_zero     = false;
		_inf      = false;
		_scale    = 0;
		_fraction.reset();
	}
	inline void setExponent(int e) { _scale = e; }
	inline bool isneg() const { return _sign; }
	inline bool ispos() const { return !_sign; }
	inline constexpr bool iszero() const { return _zero; }
	inline constexpr bool isinf() const { return _inf; }
	inline constexpr bool isnan() const { return _nan; }
	inline bool sign() const { return _sign; }
	inline int scale() const { return _scale; }
	blockbinary<fbits,bt> fraction() const { return _fraction; }

	int sign_value() const { return (_sign ? -1 : 1); }
	double scale_value() const {
		if (_zero) return (long double)(0.0);
		return std::pow((long double)2.0, (long double)_scale);
	}
	template<typename Ty = double>
	Ty fraction_value() const {
		if (_zero) return (long double)0.0;
		Ty v = 1.0;
		Ty scale = 0.5;
		for (int i = int(fbits) - 1; i >= 0; i--) {
			if (_fraction.test(i)) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		return v;
	}
	long double to_long_double() const {
		return sign_value() * scale_value() * fraction_value<long double>();
	}
	double to_double() const {
		return sign_value() * scale_value() * fraction_value<double>();
	}
	float to_float() const {
		return float(sign_value() * scale_value() * fraction_value<float>());
	}
	// Maybe remove explicit
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

private:
	bool                  _sign;
	int                   _scale;
	blockbinary<fbits,bt> _fraction;   // managing the significand as a 2's complement number
	bool                  _inf;
	bool                  _zero;
	bool                  _nan;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nfbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const triple<nfbits,nbt>& r);
	template<size_t nfbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, triple<nfbits,nbt>& r);

	template<size_t nfbits, typename nbt>
	friend bool operator==(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator!=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator< (const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator> (const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator<=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
	template<size_t nfbits, typename nbt>
	friend bool operator>=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs);
};

////////////////////// VALUE operators
template<size_t nfbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const triple<nfbits,nbt>& v) {
	if (v._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << (long double)v;
	}
	return ostr;
}

template<size_t nfbits, typename nbt>
inline std::istream& operator>> (std::istream& istr, const triple<nfbits,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nfbits, typename nbt>
inline bool operator==(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { 
	return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._zero == rhs._zero && lhs._inf == rhs._inf; 
}
template<size_t nfbits, typename nbt>
inline bool operator!=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nfbits, typename nbt>
inline bool operator< (const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) {
	if (lhs._inf) {
		if (rhs._inf) return false; else return true; // everything is less than -infinity
	}
	else {
		if (rhs._inf) return false;
	}

	if (lhs._zero) {
		if (rhs._zero) return false; // they are both 0
		if (rhs._sign) return false; else return true;
	}
	if (rhs._zero) {
		if (lhs._sign) return true; else return false;
	}
	if (lhs._sign) {
		if (rhs._sign) {	// both operands are negative
			if (lhs._scale > rhs._scale) {
				return true;	// lhs is more negative
			}
			else {
				if (lhs._scale == rhs._scale) {
					// compare the fraction, which is an unsigned triple
					if (lhs._fraction == rhs._fraction) return false; // they are the same triple
					if (lhs._fraction > rhs._fraction) {
						return true; // lhs is more negative
					}
					else {
						return false; // lhs is less negative
					}
				}
				else {
					return false; // lhs is less negative
				}
			}
		}
		else {
			return true; // lhs is negative, rhs is positive
		}
	}
	else {
		if (rhs._sign) {	
			return false; // lhs is positive, rhs is negative
		}
		else {
			if (lhs._scale > rhs._scale) {
				return false; // lhs is more positive
			}
			else {
				if (lhs._scale == rhs._scale) {
					// compare the fractions
					if (lhs._fraction == rhs._fraction) return false; // they are the same triple
					if (lhs._fraction > rhs._fraction) {
						return false; // lhs is more positive than rhs
					}
					else {
						return true; // lhs is less positive than rhs
					}
				}
				else {
					return true; // lhs is less positive
				}
			}
		}
	}
	return false;
}
template<size_t nfbits, typename nbt>
inline bool operator> (const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nfbits, typename nbt>
inline bool operator<=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nfbits, typename nbt>
inline bool operator>=(const triple<nfbits,nbt>& lhs, const triple<nfbits,nbt>& rhs) { return !operator< (lhs, rhs); }

template<size_t fbits, typename BlockType>
inline std::string components(const triple<fbits,BlockType>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << "(+,0," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	else if (v.isinf()) {
		s << "(inf," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ')';
	return s.str();
}

/// Magnitude of a scientific notation triple (equivalent to turning the sign bit off).
template<size_t nfbits, typename BlockType>
triple<nfbits,BlockType> abs(const triple<nfbits,BlockType>& v) {
	return triple<nfbits,BlockType>(false, v.scale(), v.fraction(), v.iszero());
}


}}  // namespace sw::universal
