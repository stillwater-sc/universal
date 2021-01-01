#pragma once
// blocktriple.hpp: definition of a (sign, scale, fraction) representation of an approximation to a real value
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>

#include <universal/blockbin/blockbinary.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/native/bit_functions.hpp>
#include "trace_constants.hpp"

namespace sw { namespace universal {

// Forward definitions
template<size_t ebits, size_t fbits, typename bt> class blocktriple;
template<size_t ebits, size_t fbits, typename bt> blocktriple<ebits,fbits,bt> abs(const blocktriple<ebits,fbits,bt>& v);

template<size_t nbits, typename bt>
blockbinary<nbits,bt> extract_23b_fraction(uint32_t _23b_fraction_without_hidden_bit) {
	blockbinary<nbits, bt> _fraction;
	uint32_t mask = uint32_t(0x00400000ul);
	unsigned int ub = (nbits < 23 ? nbits : 23);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _23b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<size_t nbits, typename bt>
blockbinary<nbits, bt> extract_52b_fraction(uint64_t _52b_fraction_without_hidden_bit) {
	blockbinary<nbits, bt> _fraction;
	uint64_t mask = uint64_t(0x0008000000000000ull);
	unsigned int ub = (nbits < 52 ? nbits : 52);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _52b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

template<size_t nbits, typename bt>
blockbinary<nbits, bt> extract_63b_fraction(uint64_t _63b_fraction_without_hidden_bit) {
	blockbinary<nbits, bt> _fraction;
	uint64_t mask = uint64_t(0x4000000000000000ull);
	unsigned int ub = (nbits < 63 ? nbits : 63);
	for (unsigned int i = 0; i < ub; i++) {
		_fraction[nbits - 1 - i] = _63b_fraction_without_hidden_bit & mask;
		mask >>= 1;
	}
	return _fraction;
}

// template class representing a value in scientific notation
// using a template parameter for the number of exponent and fraction bits
template<size_t ebits, size_t fbits, typename bt = uint8_t>
class blocktriple {
public:
	static constexpr size_t fhbits = fbits + 1;    // number of fraction bits including the hidden bit

	blocktriple() : _sign(false), _scale(0), _nrOfBits(fbits), _fraction(), _inf(false), _zero(true), _nan(false) {}
	blocktriple(bool sign, int scale, const blockbinary<fbits>& fraction_without_hidden_bit, bool zero = true, bool inf = false) 
		: _sign(sign), _scale(scale), _nrOfBits(fbits), _fraction(fraction_without_hidden_bit), _inf(inf), _zero(zero), _nan(false) {}

	blocktriple(const signed char initial_value)        { *this = initial_value; }
	blocktriple(const short initial_value)              { *this = initial_value; }
	blocktriple(const int initial_value)                { *this = initial_value; }
	blocktriple(const long initial_value)               { *this = initial_value; }
	blocktriple(const long long initial_value)          { *this = initial_value; }
	blocktriple(const char initial_value)               { *this = initial_value; }
	blocktriple(const unsigned short initial_value)     { *this = initial_value; }
	blocktriple(const unsigned int initial_value)       { *this = initial_value; }
	blocktriple(const unsigned long initial_value)      { *this = initial_value; }
	blocktriple(const unsigned long long initial_value) { *this = initial_value; }
	blocktriple(const float initial_value)              { *this = initial_value; }
	blocktriple(const double initial_value)             { *this = initial_value; }
	blocktriple(const long double initial_value)        { *this = initial_value; }
	blocktriple(const blocktriple& rhs)                 { *this = rhs; }

	blocktriple& operator=(const blocktriple& rhs) {
		_sign	  = rhs._sign;
		_scale	  = rhs._scale;
		_fraction = rhs._fraction;
		_nrOfBits = rhs._nrOfBits;
		_inf      = rhs._inf;
		_zero     = rhs._zero;
		_nan      = rhs._nan;
		return *this;
	}
	blocktriple& operator=(const signed char rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	blocktriple& operator=(const short rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	blocktriple& operator=(const int rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	blocktriple& operator=(const long rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	blocktriple& operator=(const long long rhs) {
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
		if (rhs == 0) {
			setzero();
			return *this;
		}
		reset();
		_sign = (0x8000000000000000 & rhs);  // 1 is negative, 0 is positive
		if (_sign) {
			// process negative number: process 2's complement of the input
			_scale = findMostSignificantBit(-rhs) - 1;
			uint64_t _fraction_without_hidden_bit = _scale == 0 ? 0 : (-rhs << (64 - _scale));
			_fraction.set_raw_bits(_fraction_without_hidden_bit);
			//take_2s_complement();
			_nrOfBits = fbits;
			if (_trace_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		}
		else {
			// process positive number
			_scale = findMostSignificantBit(rhs) - 1;
			uint64_t _fraction_without_hidden_bit = _scale == 0 ? 0 : (rhs << (64 - _scale));
			_fraction.set_raw_bits(_fraction_without_hidden_bit);
			_nrOfBits = fbits;
			if (_trace_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		}
		return *this;
	}
	blocktriple& operator=(const char rhs) {
		*this = (unsigned long long)(rhs);
		return *this;
	}
	blocktriple& operator=(const unsigned short rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	blocktriple& operator=(const unsigned int rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	blocktriple& operator=(const unsigned long rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	blocktriple& operator=(const unsigned long long rhs) {
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
		if (rhs == 0) {
			setzero();
		}
		else {
			reset();
			_scale = findMostSignificantBit(rhs) - 1;
			uint64_t _fraction_without_hidden_bit = _scale == 0 ? 0ull : (rhs << (64 - _scale)); // the scale == -1 case is handled above
			_fraction = _fraction_without_hidden_bit; // TODO: check correct rounding
			_nrOfBits = fbits;
		}
		if (_trace_conversion) std::cout << "uint64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		return *this;
	}
	blocktriple& operator=(const float rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

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
			    float _fr{ 0.0f };
				unsigned int _23b_fraction_without_hidden_bit{ 0u };
				int _exponent{ 0 };
				extract_fp_components(rhs, _sign, _exponent, _fr, _23b_fraction_without_hidden_bit);
				_scale = _exponent - 1;
				_fraction = extract_23b_fraction<fbits>(_23b_fraction_without_hidden_bit);
				_nrOfBits = fbits;
				if (_trace_conversion) std::cout << "float " << rhs << " sign " << _sign << " scale " << _scale << " 23b fraction 0x" << std::hex << _23b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
			}
			break;
		}
		return *this;
	}
	blocktriple& operator=(const double rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

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
				double _fr{ 0.0 };
				unsigned long long _52b_fraction_without_hidden_bit{ 0ull };
				int _exponent{ 0 };
				extract_fp_components(rhs, _sign, _exponent, _fr, _52b_fraction_without_hidden_bit);
				_scale = _exponent - 1;
				_fraction = extract_52b_fraction<fbits>(_52b_fraction_without_hidden_bit);
				_nrOfBits = fbits;
				if (_trace_conversion) std::cout << "double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _52b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
			}
			break;
		}
		return *this;
	}
	blocktriple& operator=(const long double rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

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
				long double _fr{ 0.0l };
				unsigned long long _63b_fraction_without_hidden_bit{ 0ull };
				int _exponent{ 0 };
				extract_fp_components(rhs, _sign, _exponent, _fr, _63b_fraction_without_hidden_bit);
				_scale = _exponent - 1;
				// how to interpret the fraction bits: TODO: this should be a static compile-time code block
				if (sizeof(long double) == 8) {
					// we are just a double and thus only have 52bits of fraction
					_fraction = extract_52b_fraction<fbits,bt>(_63b_fraction_without_hidden_bit);
					if (_trace_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

				}
				else if (sizeof(long double) == 16) {
					// how to differentiate between 80bit and 128bit formats?
					_fraction = extract_63b_fraction<fbits,bt>(_63b_fraction_without_hidden_bit);
					if (_trace_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 63b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

				}
				_nrOfBits = fbits;
			}
			break;
		}
		return *this;
	}

	// conversion operators
	explicit operator float() const { return to_float(); }
	explicit operator double() const { return to_double(); }
	explicit operator long double() const { return to_long_double(); }

	// operators
	blocktriple operator-() const {				
		return blocktriple<ebits, fbits>(!_sign, _scale, _fraction, _zero, _inf);
	}

	// modifiers
	void reset() {
		_sign  = false;
		_scale = 0;
		_nrOfBits = 0;
		_inf = false;
		_zero = false;
		_nan = false;
		_fraction.clear();
	}
	void set(bool sign, int scale, blockbinary<fbits> fraction_without_hidden_bit, bool zero, bool inf, bool nan = false) {
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
		_fraction.clear();
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
	inline void setscale(int e) { _scale = e; }
	inline void set_raw_bits(uint64_t v) { _fraction.set_raw_bits(v); }
	inline bool isneg() const { return _sign; }
	inline bool ispos() const { return !_sign; }
	inline bool iszero() const { return _zero; }
	inline bool isinf() const { return _inf; }
	inline bool isnan() const { return _nan; }
	inline bool sign() const { return _sign; }
	inline int scale() const { return _scale; }
	blockbinary<fbits> fraction() const { return _fraction; }
	/// Normalized shift (e.g., for addition).
	template <size_t Size>
	blockbinary<Size> nshift(long shift) const {
		blockbinary<Size> number;

#if BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTIONS
		// Check range
		if (long(fbits) + shift >= long(Size))
			throw shift_too_large{};
#else
		// Check range
		if (long(fbits) + shift >= long(Size)) {
			std::cerr << "nshift: shift is too large\n";
			number.reset();
			return number;
		}
#endif // BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTIONS

		const long hpos = fbits + shift;       // position of hidden bit
												  
		if (hpos <= 0) {   // If hidden bit is LSB or beyond just set uncertainty bit and call it a day
			number[0] = true;
			return number;
		}
		number[hpos] = true;                   // hidden bit now safely set

											   // Copy fraction bits into certain part
		for (long npos = hpos - 1, fpos = long(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
			number[npos] = _fraction[fpos];

		// Set uncertainty bit
		bool uncertainty = false;
		for (long fpos = std::min(long(fbits) - 1, -shift); fpos >= 0 && !uncertainty; --fpos)
			uncertainty |= _fraction[fpos];
		number[0] = uncertainty;
		return number;
	}
	// get a fixed point number by making the hidden bit explicit: useful for multiply units
	blockbinary<fhbits, bt> get_fixed_point() const {
		blockbinary<fbits + 1, bt> fixed_point_number;
		fixed_point_number.set(fbits, true); // make hidden bit explicit
		for (unsigned int i = 0; i < fbits; i++) {
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
			if (_fraction.test(i)) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		return v;
	}
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
	double      to_double() const {
		return sign_value() * scale_value() * fraction_value<double>();
	}
	float       to_float() const {
		return float(sign_value() * scale_value() * fraction_value<float>());
	}

	// TODO: this does not implement a 'real' right extend. tgtbits need to be shorter than fbits
	template<size_t srcbits, size_t tgtbits>
	void right_extend(const blocktriple<ebits,srcbits,bt>& src) {
		_sign = src.sign();
		_scale = src.scale();
		_nrOfBits = tgtbits;
		_inf = src.isinf();
		_zero = src.iszero();
		_nan = src.isnan();
		blockbinary<srcbits> src_fraction = src.fraction();
		if (!_inf && !_zero && !_nan) {
			for (int s = srcbits - 1, t = tgtbits - 1; s >= 0 && t >= 0; --s, --t)
				_fraction[t] = src_fraction[s];
		}
	}
	template<size_t tgt_ebits, size_t tgt_fbits>
	blocktriple<tgt_ebits, tgt_fbits, bt> round_to() {
		blockbinary<tgt_fbits, bt> rounded_fraction;
		if (tgt_fbits == 0) {
			bool round_up = false;
			if (fbits >= 2) {
				bool blast = _fraction[int(fbits) - 1];
				bool sb = anyAfter(_fraction, int(fbits) - 2);
				if (blast && sb) round_up = true;
			}
			else if (fbits == 1) {
				round_up = _fraction[0];
			}
			return blocktriple<tgt_ebits,tgt_fbits,bt>(_sign, (round_up ? _scale + 1 : _scale), rounded_fraction, _zero, _inf);
		}
		else {
			if (!_zero || !_inf) {
				if (tgt_fbits < fbits) {
					int rb = int(tgt_fbits) - 1;
					int lb = int(fbits) - int(tgt_fbits) - 1;
					for (int i = int(fbits) - 1; i > lb; i--, rb--) {
						rounded_fraction[rb] = _fraction[i];
					}
					bool blast = _fraction[lb];
					bool sb = false;
					if (lb > 0) sb = anyAfter(_fraction, lb-1);
					if (blast || sb) rounded_fraction[0] = true;
				}
				else {
					int rb = int(tgt_fbits) - 1;
					for (int i = int(fbits) - 1; i >= 0; i--, rb--) {
						rounded_fraction[rb] = _fraction[i];
					}
				}
			}
		}
		return blocktriple<tgt_ebits, tgt_fbits, bt>(_sign, _scale, rounded_fraction, _zero, _inf);
	}

private:
	bool                _sign;
	int                 _scale;
	int                 _nrOfBits;  // in case the fraction is smaller than the full fbits
	bool                _inf;
	bool                _zero;
	bool                _nan;
	blockbinary<fbits, bt>  _fraction;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t eebits, size_t ffbits, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const blocktriple<eebits, ffbits, bbt>& r);
	template<size_t eebits, size_t ffbits, typename bbt>
	friend std::istream& operator>> (std::istream& istr, blocktriple<eebits, ffbits, bbt>& r);

	// logic operators
	template<size_t eebits, size_t ffbits, typename bbt>
	friend bool operator==(const blocktriple<eebits, ffbits, bbt>& lhs, const blocktriple<eebits, ffbits, bbt>& rhs);
	template<size_t eebits, size_t ffbits, typename bbt>
	friend bool operator!=(const blocktriple<eebits, ffbits, bbt>& lhs, const blocktriple<eebits, ffbits, bbt>& rhs);
	template<size_t eebits, size_t ffbits, typename bbt>
	friend bool operator< (const blocktriple<eebits, ffbits, bbt>& lhs, const blocktriple<eebits, ffbits, bbt>& rhs);
	template<size_t eebits, size_t ffbits, typename bbt>
	friend bool operator> (const blocktriple<eebits, ffbits, bbt>& lhs, const blocktriple<eebits, ffbits, bbt>& rhs);
	template<size_t eebits, size_t ffbits, typename bbt>
	friend bool operator<=(const blocktriple<eebits, ffbits, bbt>& lhs, const blocktriple<eebits, ffbits, bbt>& rhs);
	template<size_t eebits, size_t ffbits, typename bbt>
	friend bool operator>=(const blocktriple<eebits, ffbits, bbt>& lhs, const blocktriple<eebits, ffbits, bbt>& rhs);
};

////////////////////// operators
template<size_t ebits, size_t fbits, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<ebits, fbits, bt>& v) {
	if (v._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << (long double)v;
	}
	return ostr;
}

template<size_t ebits, size_t fbits, typename bt>
inline std::istream& operator>> (std::istream& istr, const blocktriple<ebits, fbits, bt>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t ebits, size_t fbits, typename bt>
inline blocktriple<ebits, fbits, bt> operator/(const blocktriple<ebits, fbits, bt>& lhs, const blocktriple<ebits, fbits, bt>& rhs) {
	return lhs;
}

template<size_t ebits, size_t fbits, typename bt>
inline bool operator==(const blocktriple<ebits, fbits, bt>& lhs, const blocktriple<ebits, fbits, bt>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._nrOfBits == rhs._nrOfBits && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }

template<size_t ebits, size_t fbits, typename bt>
inline bool operator!=(const blocktriple<ebits, fbits, bt>& lhs, const blocktriple<ebits, fbits, bt>& rhs) { return !operator==(lhs, rhs); }

template<size_t ebits, size_t fbits, typename bt>
inline bool operator< (const blocktriple<ebits, fbits, bt>& lhs, const blocktriple<ebits, fbits, bt>& rhs) {
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
	return false;
}

template<size_t ebits, size_t fbits, typename bt>
inline bool operator> (const blocktriple<ebits, fbits, bt>& lhs, const blocktriple<ebits, fbits, bt>& rhs) { return  operator< (rhs, lhs); }
template<size_t ebits, size_t fbits, typename bt>
inline bool operator<=(const blocktriple<ebits, fbits, bt>& lhs, const blocktriple<ebits, fbits, bt>& rhs) { return !operator> (lhs, rhs); }
template<size_t ebits, size_t fbits, typename bt>
inline bool operator>=(const blocktriple<ebits, fbits, bt>& lhs, const blocktriple<ebits, fbits, bt>& rhs) { return !operator< (lhs, rhs); }

template<size_t ebits, size_t fbits, typename bt>
inline std::string components(const blocktriple<ebits, fbits, bt>& v) {
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

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<size_t ebits, size_t fbits, typename bt>
blocktriple<ebits, fbits, bt> abs(const blocktriple<ebits, fbits, bt>& v) {
	return blocktriple<ebits, fbits, bt>(false, v.scale(), v.fraction(), v.iszero());
}

// add two values with fbits fraction bits, round them to abits, and return the abits+1 result value
template<size_t ebits, size_t fbits, size_t abits, typename bt>
void module_add(const blocktriple<ebits,fbits,bt>& lhs, const blocktriple<ebits,fbits,bt>& rhs, blocktriple<ebits,abits + 1,bt>& result) {
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
	blockbinary<abits,bt> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	blockbinary<abits,bt> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = rhs.sign();
	bool signs_are_different = r1_sign != r2_sign;

	if (signs_are_different && sw::universal::abs(lhs) < sw::universal::abs(rhs)) {
		std::swap(r1, r2);
		std::swap(r1_sign, r2_sign);
	}

	if (signs_are_different) r2 = twos_complement(r2);

	if (_trace_add) {
		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
		if (signs_are_different) {
			std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2 orig  " << twos_complement(r2) << std::endl;
		}
		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
	}

	blockbinary<abits + 1,bt> sum;
	const bool carry = add_unsigned(r1, r2, sum);

	if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

	long shift = 0;
	if (carry) {
		if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
			shift = -1;
		} 
		else {
			// the carry && signs!= implies ||result|| < ||r1||, must find MSB (in the complement)
			for (int i = abits - 1; i >= 0 && !sum[i]; i--) {
				shift++;
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
	const int hpos = abits - 1 - shift;         // position of the hidden bit 
	sum <<= abits - hpos + 1;
	if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
}

// subtract module: use ADDER
template<size_t ebits, size_t fbits, size_t abits, typename bt>
void module_subtract(const blocktriple<ebits,fbits,bt>& lhs, const blocktriple<ebits,fbits,bt>& rhs, blocktriple<ebits,abits + 1,bt>& result) {
	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

	// align the fractions
	blockbinary<abits,bt> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	blockbinary<abits,bt> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = !rhs.sign();
	bool signs_are_different = r1_sign != r2_sign;

	if (sw::universal::abs(lhs) < sw::universal::abs(rhs)) {
		std::swap(r1, r2);
		std::swap(r1_sign, r2_sign);
	}

	if (signs_are_different) r2 = twos_complement(r2);

	if (_trace_sub) {
		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
	}

	blockbinary<abits + 1,bt> sum;
	const bool carry = add_unsigned(r1, r2, sum);

	if (_trace_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

	long shift = 0;
	if (carry) {
		if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
			shift = -1;
		}
		else {
			// the carry && signs!= implies r2 is complement, result < r1, must find hidden bit (in the complement)
			for (int i = abits - 1; i >= 0 && !sum[i]; i--) {
				shift++;
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
	const int hpos = abits - 1 - shift;         // position of the hidden bit 
	sum <<= abits - hpos + 1;
	if (_trace_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
}

// subtract module using SUBTRACTOR: CURRENTLY BROKEN FOR UNKNOWN REASON
template<size_t ebits, size_t fbits, size_t abits, typename bt>
void module_subtract_BROKEN(const blocktriple<ebits,fbits,bt>& lhs, const blocktriple<ebits,fbits,bt>& rhs, blocktriple<ebits,abits + 1,bt>& result) {

	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

	// align the fractions
	blockbinary<abits,bt> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	blockbinary<abits,bt> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = rhs.sign();
	//bool signs_are_equal = r1_sign == r2_sign;

	if (r1_sign) r1 = twos_complement(r1);
	if (r1_sign) r2 = twos_complement(r2);

	if (_trace_sub) {
		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
	}

	blockbinary<abits + 1,bt> difference;
	const bool borrow = subtract_unsigned(r1, r2, difference);

	if (_trace_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " borrow" << std::setw(3) << (borrow ? 1 : 0) << " diff    " << difference << std::endl;

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
	if (_trace_sub) std::cout << (borrow ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " result  " << difference << std::endl;
	result.set(borrow, scale_of_result, difference, false, false, false);
}

// multiply module
template<size_t ebits, size_t fbits, size_t mbits, typename bt>
void module_multiply(const blocktriple<ebits,fbits,bt>& lhs, const blocktriple<ebits,fbits,bt>& rhs, blocktriple<ebits,mbits,bt>& result) {
	static constexpr size_t fhbits = fbits + 1;  // fraction + hidden bit
	if (_trace_mul) std::cout << "lhs  " << components(lhs) << std::endl << "rhs  " << components(rhs) << std::endl;

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
	blockbinary<mbits,bt> result_fraction;

	if (fbits > 0) {
		// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
		blockbinary<fhbits,bt> r1 = lhs.get_fixed_point();
		blockbinary<fhbits,bt> r2 = rhs.get_fixed_point();
		multiply_unsigned(r1, r2, result_fraction);

		if (_trace_mul) std::cout << "r1  " << r1 << std::endl << "r2  " << r2 << std::endl << "res " << result_fraction << std::endl;
		// check if the radix point needs to shift
		int shift = 2;
		if (result_fraction.test(mbits - 1)) {
			shift = 1;
			if (_trace_mul) std::cout << " shift " << shift << std::endl;
			new_scale += 1;
		}
		result_fraction <<= shift;    // shift hidden bit out	
	}
	else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
		// multiply the hidden bits together, i.e. 1*1: we know the answer a priori
	}
	if (_trace_mul) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

	result.set(new_sign, new_scale, result_fraction, false, false, false);
}

// divide module
template<size_t ebits, size_t fbits, size_t divbits, typename bt>
void module_divide(const blocktriple<ebits,fbits,bt>& lhs, const blocktriple<ebits,fbits,bt>& rhs, blocktriple<ebits,divbits,bt>& result) {
	static constexpr size_t fhbits = fbits + 1;  // fraction + hidden bit
	if (_trace_div) std::cout << "lhs  " << components(lhs) << std::endl << "rhs  " << components(rhs) << std::endl;

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
	blockbinary<divbits,bt> result_fraction;

	if (fbits > 0) {
		// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
		blockbinary<fhbits,bt> r1 = lhs.get_fixed_point();
		blockbinary<fhbits,bt> r2 = rhs.get_fixed_point();
		divide_with_fraction(r1, r2, result_fraction);
		if (_trace_div) std::cout << "r1     " << r1 << std::endl << "r2     " << r2 << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;
		// check if the radix point needs to shift
		// radix point is at divbits - fhbits
		int msb = divbits - fhbits;
		int shift = fhbits;
		if (!result_fraction.test(msb)) {
			msb--; shift++;
			while (!result_fraction.test(msb)) { // search for the first 1
				msb--; shift++;
			}
		}
		result_fraction <<= shift;    // shift hidden bit out
		new_scale -= (shift - fhbits);
		if (_trace_div) std::cout << "shift  " << shift << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;;
	}
	else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
			 // no need to multiply the hidden bits together, i.e. 1*1: we know the answer a priori
	}
	if (_trace_div) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

	result.set(new_sign, new_scale, result_fraction, false, false, false);
}

}}  // namespace sw::universal
