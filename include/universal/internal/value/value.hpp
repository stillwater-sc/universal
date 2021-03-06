#pragma once
// value.hpp: definition of a (sign, scale, significant) representation of an approximation to a real value
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>
#include <tuple>
#include <algorithm> // std::max

#include <universal/native/ieee754.hpp>
#include <universal/native/bit_functions.hpp>
#include <universal/internal/bitblock/bitblock.hpp>

namespace sw::universal::internal {

using namespace sw::universal;

// Forward definitions
template<size_t fbits> class value;
template<size_t fbits> value<fbits> abs(const value<fbits>& v);

#ifdef VALUE_TRACE_CONVERSION
constexpr bool _trace_value_conversion = true;
#else
constexpr bool _trace_value_conversion = false;
#endif

#ifdef VALUE_TRACE_ADD
constexpr bool _trace_value_add = true;
#else
constexpr bool _trace_value_add = false;
#endif

#ifdef VALUE_TRACE_SUB
constexpr bool _trace_value_sub = true;
#else
constexpr bool _trace_value_sub = false;
#endif

#ifdef VALUE_TRACE_MUL
constexpr bool _trace_value_mul = true;
#else
constexpr bool _trace_value_mul = false;
#endif

#ifdef VALUE_TRACE_DIV
constexpr bool _trace_value_div = true;
#else
constexpr bool _trace_value_div = false;
#endif

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<size_t fbits>
class value {
public:
	static constexpr size_t fhbits = fbits + 1;    // number of fraction bits including the hidden bit
	constexpr value() 
          : _sign{false}, _scale{0}, _nrOfBits{fbits}, _fraction{}, _inf{false}, 
            _zero{true}, _nan{false} {}
	constexpr value(bool sign, int scale, const internal::bitblock<fbits>& fraction_without_hidden_bit, 
                        bool zero = true, bool inf = false) 
          : _sign{sign}, _scale{scale}, _nrOfBits{fbits}, _fraction{fraction_without_hidden_bit}, 
            _inf{inf}, _zero{zero}, _nan{false} {}


	// decorated constructors
	constexpr value(const value& initial_value)       { *this = initial_value; }
	constexpr value(signed char initial_value)        { *this = initial_value; }
	constexpr value(short initial_value)              { *this = initial_value; }
	constexpr value(int initial_value)                { *this = initial_value; }
	constexpr value(long initial_value)               { *this = initial_value; }
	constexpr value(long long initial_value)          { *this = initial_value; }
	constexpr value(char initial_value)               { *this = initial_value; }
	constexpr value(unsigned short initial_value)     { *this = initial_value; }
	constexpr value(unsigned int initial_value)       { *this = initial_value; }
	constexpr value(unsigned long initial_value)      { *this = initial_value; }
	constexpr value(unsigned long long initial_value) { *this = initial_value; }
	value(float initial_value)              { *this = initial_value; }
	value(double initial_value) : value{}   { *this = initial_value; }
	value(long double initial_value)        { *this = initial_value; }

	// assignment operators
	constexpr value& operator=(const value& rhs) {
		_sign	  = rhs._sign;
		_scale	  = rhs._scale;
		_fraction = rhs._fraction;
		_nrOfBits = rhs._nrOfBits;
		_inf      = rhs._inf;
		_zero     = rhs._zero;
		_nan      = rhs._nan;
		return *this;
	}
	constexpr value<fbits>& operator=(signed char rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr value<fbits>& operator=(short rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr value<fbits>& operator=(int rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr value<fbits>& operator=(long rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr value<fbits>& operator=(long long rhs) {
		if (_trace_value_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
		if (rhs == 0) {
			setzero();
			return *this;
		}
		reset();
		_sign = (0x8000000000000000 & rhs);  // 1 is negative, 0 is positive
		if (_sign) {
			// process negative number: process 2's complement of the input
			_scale = int(sw::universal::findMostSignificantBit(-rhs)) - 1;
			uint64_t _fraction_without_hidden_bit = uint64_t(_scale == 0 ? 0 : (-rhs << (64 - _scale)));
			_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
			//take_2s_complement();
			_nrOfBits = fbits;
			if (_trace_value_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		}
		else {
			// process positive number
			_scale = int(sw::universal::findMostSignificantBit(rhs)) - 1;
			uint64_t _fraction_without_hidden_bit = uint64_t(_scale == 0 ? 0 : (rhs << (64 - _scale)));
			_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
			_nrOfBits = fbits;
			if (_trace_value_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		}
		return *this;
	}
	constexpr value<fbits>& operator=(char rhs) {
		*this = (unsigned long long)(rhs);
		return *this;
	}
	constexpr value<fbits>& operator=(unsigned short rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr value<fbits>& operator=(unsigned int rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr value<fbits>& operator=(unsigned long rhs) {
		*this = static_cast<long long>(rhs);
		return *this;
	}
	constexpr value<fbits>& operator=(unsigned long long rhs) {
		if (_trace_value_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
		if (rhs == 0) {
			setzero();
		}
		else {
			reset();
			_scale = sw::universal::findMostSignificantBit(rhs) - 1;
			uint64_t _fraction_without_hidden_bit = _scale == 0 ? 0ull : (rhs << (64 - _scale)); // the scale == -1 case is handled above
			_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
			_nrOfBits = fbits;
		}
		if (_trace_value_conversion) std::cout << "uint64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		return *this;
	}
	value<fbits>& operator=(float rhs) {
		reset();
		if (_trace_value_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_nrOfBits = fbits;
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
				float _fr{0};
				unsigned int _23b_fraction_without_hidden_bit{0};
				int _exponent{0};
				extract_fp_components(rhs, _sign, _exponent, _fr, _23b_fraction_without_hidden_bit);
				_scale = _exponent - 1;
				_fraction = extract_23b_fraction<fbits>(_23b_fraction_without_hidden_bit);
				_nrOfBits = fbits;
				if (_trace_value_conversion) std::cout << "float " << rhs << " sign " << _sign << " scale " << _scale << " 23b fraction 0x" << std::hex << _23b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
			}
			break;
		}
		return *this;
	}
	value<fbits>& operator=(double rhs) {
		using std::get;
		reset();
		if (_trace_value_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_nrOfBits = fbits;
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
#if 1
				double _fr{0};
				unsigned long long _52b_fraction_without_hidden_bit{0};
				int _exponent{0};
				extract_fp_components(rhs, _sign, _exponent, _fr, _52b_fraction_without_hidden_bit);
#endif
#if 0
                                auto components= ieee_components(rhs);
                                _sign= get<0>(components);
                                int _exponent= get<1>(components);
                                unsigned long long _52b_fraction_without_hidden_bit= get<2>(components);
#endif
				_scale = _exponent - 1;
				_fraction = extract_52b_fraction<fbits>(_52b_fraction_without_hidden_bit);
				_nrOfBits = fbits;
				if (_trace_value_conversion) std::cout << "double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _52b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
			}
			break;
		}
		return *this;
	}
	value<fbits>& operator=(long double rhs) {
		reset();
		if (_trace_value_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_nrOfBits = fbits;
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
				long double _fr{0};
				unsigned long long _63b_fraction_without_hidden_bit{0};
				int _exponent{0};
				extract_fp_components(rhs, _sign, _exponent, _fr, _63b_fraction_without_hidden_bit);
				_scale = _exponent - 1;
				// how to interpret the fraction bits: TODO: this should be a static compile-time code block
				if (sizeof(long double) == 8) {
					// we are just a double and thus only have 52bits of fraction
					_fraction = extract_52b_fraction<fbits>(_63b_fraction_without_hidden_bit);
					if (_trace_value_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

				}
				else if (sizeof(long double) == 16) {
					// how to differentiate between 80bit and 128bit formats?
					_fraction = extract_63b_fraction<fbits>(_63b_fraction_without_hidden_bit);
					if (_trace_value_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 63b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

				}
				_nrOfBits = fbits;
			}
			break;
		}
		return *this;
	}

	// operators
	constexpr value<fbits> operator-() const {				
		return value<fbits>(!_sign, _scale, _fraction, _zero, _inf);
	}

	value<fbits>& operator++() {
		*this = *this + value<fbits>(1);
		return *this;
	}
	value<fbits> operator++(int) {
		value<fbits> tmp{ *this };
		operator++();
		return tmp;
	}

	// modifiers
	constexpr void reset() & {
		_sign  = false;
		_scale = 0;
		_nrOfBits = 0;
		_inf = false;
		_zero = false;
		_nan = false;
		_fraction.reset(); // not constexpr
                // _fraction= bitblock<fbits>{}; // work around
	}
	void set(bool sign, int scale, bitblock<fbits> fraction_without_hidden_bit, bool zero, bool inf, bool nan = false) {
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
		_nrOfBits = fbits;
		_fraction.reset();
	}
	void setinf() {      // this maps to NaR on the posit side, and that has a sign = 1
		_inf      = true;
		_sign     = true;
		_zero     = false;
		_nan      = false;
		_scale    = 0;
		_nrOfBits = fbits;
		_fraction.reset();
	}
	void setnan() {		// this will also map to NaR
		_nan      = true;
		_sign     = true;
		_zero     = false;
		_inf      = false;
		_scale    = 0;
		_nrOfBits = fbits;	
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
	bitblock<fbits> fraction() const { return _fraction; }
	/// Normalized shift (e.g., for addition).
	template <size_t Size>
	bitblock<Size> nshift(int shift) const {
	bitblock<Size> number;

#if VALUE_THROW_ARITHMETIC_EXCEPTION
		// Check range
		if (int(fbits) + shift >= int(Size))
			throw shift_too_large{};
#else
		// Check range
		if (int(fbits) + shift >= int(Size)) {
			std::cerr << "nshift: shift is too large\n";
			number.reset();
			return number;
		}
#endif // VALUE_THROW_ARITHMETIC_EXCEPTIONS

		int hpos = int(fbits) + shift;       // position of hidden bit
		if (hpos <= 0) {   // If hidden bit is LSB or beyond just set uncertainty bit and call it a day
			number[0] = true;
			return number;
		}
		number[size_t(hpos)] = true;                   // hidden bit now safely set

											   // Copy fraction bits into certain part
		for (int npos = hpos - 1, fpos = int(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
			number[size_t(npos)] = _fraction[size_t(fpos)];

		// Set uncertainty bit
		bool uncertainty = false;
		for (int fpos = std::min(int(fbits) - 1, -shift); fpos >= 0 && !uncertainty; --fpos)
			uncertainty |= _fraction[size_t(fpos)];
		number[0] = uncertainty;
		return number;
	}
	// get a fixed point number by making the hidden bit explicit: useful for multiply units
	bitblock<fhbits> get_fixed_point() const {
		bitblock<fbits + 1> fixed_point_number;
		fixed_point_number.set(fbits, true); // make hidden bit explicit
		for (size_t i = 0; i < fbits; i++) {
			fixed_point_number[i] = _fraction[i];
		}
		return fixed_point_number;
	}
	// get the fraction value including the implicit hidden bit (this is at an exponent level 1 smaller)
	template<typename Ty = double>
	Ty get_implicit_fraction_value() const {
		if (_zero) return (long double)0.0;
		Ty v = 1.0;
		Ty scale = 0.5;
		for (int i = int(fbits) - 1; i >= 0; i--) {
			if (_fraction.test(size_t(i))) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		return v;
	}
	int sign_value() const { return (_sign ? -1 : 1); }
	long double scale_value() const {
		if (_zero) return (long double)(0.0);
		return std::pow((long double)2.0, (long double)_scale);
	}
	template<typename Ty = double>
	Ty fraction_value() const {
		if (_zero) return (long double)0.0;
		Ty v = 1.0;
		Ty scale = 0.5;
		for (int i = int(fbits) - 1; i >= 0; i--) {
			if (_fraction.test(size_t(i))) v += scale;
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

	// TODO: this does not implement a 'real' right extend. tgtbits need to be shorter than fbits
	template<size_t srcbits, size_t tgtbits>
	void right_extend(const value<srcbits>& src) {
		_sign = src.sign();
		_scale = src.scale();
		_nrOfBits = tgtbits;
		_inf = src.isinf();
		_zero = src.iszero();
		_nan = src.isnan();
		bitblock<srcbits> src_fraction = src.fraction();
		if (!_inf && !_zero && !_nan) {
			for (int s = srcbits - 1, t = tgtbits - 1; s >= 0 && t >= 0; --s, --t)
				_fraction[t] = src_fraction[s];
		}
	}
	template<size_t tgt_size>
	value<tgt_size> round_to() {
		bitblock<tgt_size> rounded_fraction;
		if (tgt_size == 0) {
			bool round_up = false;
			if (fbits >= 2) {
				bool blast = _fraction[int(fbits) - 1];
				bool sb = anyAfter(_fraction, int(fbits) - 2);
				if (blast && sb) round_up = true;
			}
			else if (fbits == 1) {
				round_up = _fraction[0];
			}
			return value<tgt_size>(_sign, (round_up ? _scale + 1 : _scale), rounded_fraction, _zero, _inf);
		}
		else {
			if (!_zero || !_inf) {
				if (tgt_size < fbits) {
					int rb = int(tgt_size) - 1;
					int lb = int(fbits) - int(tgt_size) - 1;
					for (int i = int(fbits) - 1; i > lb; i--, rb--) {
						rounded_fraction[rb] = _fraction[i];
					}
					bool blast = _fraction[lb];
					bool sb = false;
					if (lb > 0) sb = anyAfter(_fraction, lb-1);
					if (blast || sb) rounded_fraction[0] = true;
				}
				else {
					int rb = int(tgt_size) - 1;
					for (int i = int(fbits) - 1; i >= 0; i--, rb--) {
						rounded_fraction[rb] = _fraction[i];
					}
				}
			}
		}
		return value<tgt_size>(_sign, _scale, rounded_fraction, _zero, _inf);
	}
private:
	bool                _sign;
	int                 _scale;
	int                 _nrOfBits;  // in case the fraction is smaller than the full fbits
	bitblock<fbits>	    _fraction;
	bool                _inf;
	bool                _zero;
	bool                _nan;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nfbits>
	friend std::ostream& operator<< (std::ostream& ostr, const value<nfbits>& r);
	template<size_t nfbits>
	friend std::istream& operator>> (std::istream& istr, value<nfbits>& r);

	template<size_t nfbits>
	friend bool operator==(const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator!=(const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator< (const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator> (const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator<=(const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator>=(const value<nfbits>& lhs, const value<nfbits>& rhs);
};

////////////////////// VALUE operators
template<size_t nfbits>
inline std::ostream& operator<<(std::ostream& ostr, const value<nfbits>& v) {
	if (v._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << (long double)v;
	}
	return ostr;
}

template<size_t nfbits>
inline std::istream& operator>> (std::istream& istr, const value<nfbits>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nfbits>
inline bool operator==(const value<nfbits>& lhs, const value<nfbits>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._nrOfBits == rhs._nrOfBits && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }
template<size_t nfbits>
inline bool operator!=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator==(lhs, rhs); }
template<size_t nfbits>
inline bool operator< (const value<nfbits>& lhs, const value<nfbits>& rhs) {
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
					// compare the fraction, which is an unsigned value
					if (lhs._fraction == rhs._fraction) return false; // they are the same value
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
					if (lhs._fraction == rhs._fraction) return false; // they are the same value
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
//	return false; // all paths are taken care of
}
template<size_t nfbits>
inline bool operator> (const value<nfbits>& lhs, const value<nfbits>& rhs) { return  operator< (rhs, lhs); }
template<size_t nfbits>
inline bool operator<=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator> (lhs, rhs); }
template<size_t nfbits>
inline bool operator>=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator< (lhs, rhs); }

template<size_t nbits>
inline std::string to_binary(const bitblock<nbits>& a, bool nibbleMarker = true) {
	std::stringstream s;
	s << 'b';
	for (int i = int(nbits - 1); i >= 0; --i) {
		s << (a[i] ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}
	return s.str();
}
template<size_t fbits>
inline std::string to_triple(const value<fbits>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << "(+, 0," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	else if (v.isinf()) {
		s << "(inf," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	s << (v.sign() ? "(-, " : "(+, ");
	s << v.scale() << ", ";
	s << to_binary(v.fraction(), true) << ')';
//	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ')';
	return s.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<size_t nfbits>
value<nfbits> abs(const value<nfbits>& v) {
	return value<nfbits>(false, v.scale(), v.fraction(), v.iszero());
}

// add two values with fbits fraction bits, round them to abits, and return the abits+1 result value
template<size_t fbits, size_t abits>
void module_add(const value<fbits>& lhs, const value<fbits>& rhs, value<abits + 1>& result) {
	// with sign/magnitude adders it is customary to organize the computation 
	// along the four quadrants of sign combinations
	//  + + = +
	//  + - =   lhs > rhs ? + : -
	//  - + =   lhs > rhs ? - : +
	//  - - = 
	// to simplify the result processing assign the biggest 
	// absolute value to R1, then the sign of the result will be sign of the value in R1.

	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

	// align the fractions
	bitblock<abits> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	bitblock<abits> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = rhs.sign();
	bool signs_are_different = r1_sign != r2_sign;

	if (signs_are_different && abs(lhs) < abs(rhs)) {
		std::swap(r1, r2);
		std::swap(r1_sign, r2_sign);
	}

	if (signs_are_different) r2 = twos_complement(r2);

	if (_trace_value_add) {
		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
		if (signs_are_different) {
			std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2 orig  " << twos_complement(r2) << std::endl;
		}
		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
	}

	bitblock<abits + 1> sum;
	const bool carry = add_unsigned(r1, r2, sum);

	if (_trace_value_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

	int shift = 0;
	if (carry) {
		if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
			shift = -1;
		} 
		else {
			// the carry && signs!= implies ||result|| < ||r1||, must find MSB (in the complement)
			for (int i = int(abits) - 1; i >= 0 && !sum[size_t(i)]; --i) {
				++shift;
			}
		}
	}
	assert(shift >= -1);

	if (shift >= long(abits)) {            // we have actual 0                            
		sum.reset();
		result.set(false, 0, sum, true, false, false);
		return;
	}

	scale_of_result -= shift;
	const int hpos = int(abits) - 1 - shift;         // position of the hidden bit 
	sum <<= abits - hpos + 1;
	if (_trace_value_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
}

// subtract module: use ADDER
template<size_t fbits, size_t abits>
void module_subtract(const value<fbits>& lhs, const value<fbits>& rhs, value<abits + 1>& result) {
	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

	// align the fractions
	bitblock<abits> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	bitblock<abits> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = !rhs.sign();
	bool signs_are_different = r1_sign != r2_sign;

	if (abs(lhs) < abs(rhs)) {
		std::swap(r1, r2);
		std::swap(r1_sign, r2_sign);
	}

	if (signs_are_different) r2 = twos_complement(r2);

	if (_trace_value_sub) {
		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
	}

	bitblock<abits + 1> sum;
	const bool carry = add_unsigned(r1, r2, sum);

	if (_trace_value_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

	int shift = 0;
	if (carry) {
		if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
			shift = -1;
		}
		else {
			// the carry && signs!= implies r2 is complement, result < r1, must find hidden bit (in the complement)
			for (int i = static_cast<int>(abits) - 1; i >= 0 && !sum[static_cast<size_t>(i)]; --i) {
				shift++;
			}
		}
	}
	assert(shift >= -1);

	if (shift >= static_cast<int>(abits)) {            // we have actual 0                            
		sum.reset();
		result.set(false, 0, sum, true, false, false);
		return;
	}

	scale_of_result -= shift;
	const int hpos = static_cast<int>(abits) - 1 - shift;         // position of the hidden bit 
	sum <<= abits - hpos + 1;
	if (_trace_value_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
}

// subtract module using SUBTRACTOR: CURRENTLY BROKEN FOR UNKNOWN REASON
template<size_t fbits, size_t abits>
void module_subtract_BROKEN(const value<fbits>& lhs, const value<fbits>& rhs, value<abits + 1>& result) {

	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

	// align the fractions
	bitblock<abits> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	bitblock<abits> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = rhs.sign();
	//bool signs_are_equal = r1_sign == r2_sign;

	if (r1_sign) r1 = twos_complement(r1);
	if (r1_sign) r2 = twos_complement(r2);

	if (_trace_value_sub) {
		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
	}

	bitblock<abits + 1> difference;
	const bool borrow = subtract_unsigned(r1, r2, difference);

	if (_trace_value_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " borrow" << std::setw(3) << (borrow ? 1 : 0) << " diff    " << difference << std::endl;

	long shift = 0;
	if (borrow) {   // we have a negative value result
		difference = twos_complement(difference);
	}
	// find hidden bit
	for (int i = abits - 1; i >= 0 && difference[i]; i--) {
		shift++;
	}
	assert(shift >= -1);

	if (shift >= long(abits)) {            // we have actual 0                            
		difference.reset();
		result.set(false, 0, difference, true, false, false);
		return;
	}

	scale_of_result -= shift;
	const int hpos = abits - 1 - shift;         // position of the hidden bit 
	difference <<= abits - hpos + 1;
	if (_trace_value_sub) std::cout << (borrow ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " result  " << difference << std::endl;
	result.set(borrow, scale_of_result, difference, false, false, false);
}

// multiply module
template<size_t fbits, size_t mbits>
void module_multiply(const value<fbits>& lhs, const value<fbits>& rhs, value<mbits>& result) {
	static constexpr size_t fhbits = fbits + 1;  // fraction + hidden bit
	if (_trace_value_mul) std::cout << "lhs  " << to_triple(lhs) << std::endl << "rhs  " << to_triple(rhs) << std::endl;

	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	if (lhs.iszero() || rhs.iszero()) {
		result.setzero();
		return;
	}

	bool new_sign = lhs.sign() ^ rhs.sign();
	int new_scale = lhs.scale() + rhs.scale();
	bitblock<mbits> result_fraction;

	if (fbits > 0) {
		// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
		bitblock<fhbits> r1 = lhs.get_fixed_point();
		bitblock<fhbits> r2 = rhs.get_fixed_point();
		multiply_unsigned(r1, r2, result_fraction);

		if (_trace_value_mul) std::cout << "r1  " << r1 << std::endl << "r2  " << r2 << std::endl << "res " << result_fraction << std::endl;
		// check if the radix point needs to shift
		int shift = 2;
		if (result_fraction.test(mbits - 1)) {
			shift = 1;
			if (_trace_value_mul) std::cout << " shift " << shift << std::endl;
			new_scale += 1;
		}
		result_fraction <<= static_cast<size_t>(shift);    // shift hidden bit out	
	}
	else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
		// multiply the hidden bits together, i.e. 1*1: we know the answer a priori
	}
	if (_trace_value_mul) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

	result.set(new_sign, new_scale, result_fraction, false, false, false);
}

// divide module
template<size_t fbits, size_t divbits>
void module_divide(const value<fbits>& lhs, const value<fbits>& rhs, value<divbits>& result) {
	static constexpr size_t fhbits = fbits + 1;  // fraction + hidden bit
	if (_trace_value_div) std::cout << "lhs  " << to_triple(lhs) << std::endl << "rhs  " << to_triple(rhs) << std::endl;

	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	if (lhs.iszero() || rhs.iszero()) {
		result.setzero();
		return;
	}

	bool new_sign = lhs.sign() ^ rhs.sign();
	int new_scale = lhs.scale() - rhs.scale();
	bitblock<divbits> result_fraction;

	if (fbits > 0) {
		// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
		bitblock<fhbits> r1 = lhs.get_fixed_point();
		bitblock<fhbits> r2 = rhs.get_fixed_point();
		divide_with_fraction(r1, r2, result_fraction);
		if (_trace_value_div) std::cout << "r1     " << r1 << std::endl << "r2     " << r2 << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;
		// check if the radix point needs to shift
		// radix point is at divbits - fhbits
		int msb = static_cast<int>(divbits - fhbits);
		int shift = fhbits;
		if (!result_fraction.test(static_cast<size_t>(msb))) {
			msb--; shift++;
			while (!result_fraction.test(static_cast<size_t>(msb))) { // search for the first 1
				msb--; shift++;
			}
		}
		result_fraction <<= static_cast<size_t>(shift);    // shift hidden bit out
		new_scale -= (shift - static_cast<int>(fhbits));
		if (_trace_value_div) std::cout << "shift  " << shift << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;;
	}
	else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
			 // no need to multiply the hidden bits together, i.e. 1*1: we know the answer a priori
	}
	if (_trace_value_div) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

	result.set(new_sign, new_scale, result_fraction, false, false, false);
}

template<size_t fbits>
value<fbits> operator+(const value<fbits>& lhs, const value<fbits>& rhs) {
	constexpr size_t abits = fbits + 5;
	value<abits+1> result;
	module_add<fbits,abits>(lhs, rhs, result);
#if defined(__GNUC__) || defined(__GNUG__)
	return value<fbits>(); // for some reason GCC doesn't want to compile result.round_to<fbits>()
#else
	return result.round_to<fbits>();
#endif
}
template<size_t fbits>
value<fbits> operator-(const value<fbits>& lhs, const value<fbits>& rhs) {
	constexpr size_t abits = fbits + 5;
	value<abits+1> result;
	module_subtract<fbits,abits>(lhs, rhs, result);
#if defined(__GNUC__) || defined(__GNUG__)
	return value<fbits>(); // for some reason GCC doesn't want to compile result.round_to<fbits>()
#else
	return result.round_to<fbits>();
#endif
}
template<size_t fbits>
value<fbits> operator*(const value<fbits>& lhs, const value<fbits>& rhs) {
	constexpr size_t mbits = 2*fbits + 2;
	value<mbits> result;
	module_multiply(lhs, rhs, result);
#if defined(__GNUC__) || defined(__GNUG__)
	return value<fbits>(); // for some reason GCC doesn't want to compile result.round_to<fbits>()
#else
	return result.round_to<fbits>();
#endif
}
template<size_t fbits>
value<fbits> operator/(const value<fbits>& lhs, const value<fbits>& rhs) {
	constexpr size_t divbits = 2 * fbits + 5;
	value<divbits> result;
	module_divide(lhs, rhs, result);
#if defined(__GNUC__) || defined(__GNUG__)
	return value<fbits>(); // for some reason GCC doesn't want to compile result.round_to<fbits>()
#else
	return result.round_to<fbits>();
#endif
}
template<size_t fbits>
value<fbits> sqrt(const value<fbits>& a) {
	return std::sqrt(double(a));
}

}  // namespace sw::universal::internal
