#pragma once
// value.hpp: definition of a (sign, scale, significant) representation of an approximation to a real value
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>
#include <tuple>
#include <algorithm> // std::max

#include <universal/common/exceptions.hpp>
#include <universal/number/support/decimal.hpp>
#include <universal/utility/find_msb.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/native/nonconstexpr/extract_fp_components.hpp>
#include <universal/internal/bitblock/bitblock.hpp>

namespace sw { namespace universal { namespace internal {

struct value_internal_exception : public universal_internal_exception {
	value_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("value internal exception: ") + error) {};
};

struct value_shift_too_large : public value_internal_exception {
	value_shift_too_large() : value_internal_exception("shift value too large") {}
};

using namespace sw::universal;

// Forward definitions
template<unsigned fbits> class value;
template<unsigned fbits> value<fbits> abs(const value<fbits>& v);

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

// template class representing a value in scientific notation, using a template parameter to define the number of fraction bits
template<unsigned fbits>
class value {
public:
	static constexpr unsigned fhbits = fbits + 1;    // number of fraction bits including the hidden bit
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
			_scale = int(sw::universal::find_msb(-rhs)) - 1;
			uint64_t _fraction_without_hidden_bit = uint64_t(_scale == 0 ? 0 : (-rhs << (64 - _scale)));
			_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
			//take_2s_complement();
			_nrOfBits = fbits;
			if (_trace_value_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		}
		else {
			// process positive number
			_scale = int(sw::universal::find_msb(rhs)) - 1;
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
			_scale = static_cast<int>(sw::universal::find_msb(rhs)) - 1;
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
				int _exponent{0};
				// fraction without hidden bit moved into the if

				// how to interpret the fraction bits: TODO: this should be a static compile-time code block
				if constexpr (sizeof(long double) == 8) {
					// we are just a double and thus only have 52bits of fraction

					std::uint64_t _63b_fraction_without_hidden_bit{0};
					extract_fp_components(rhs, _sign, _exponent, _fr, _63b_fraction_without_hidden_bit);
					_scale = _exponent - 1;

					_fraction = extract_52b_fraction<fbits>(_63b_fraction_without_hidden_bit);
					if (_trace_value_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

				}
				else if constexpr (sizeof(long double) == 16 && std::numeric_limits<long double>::digits <= 64) {
					// how to differentiate between 80bit and 128bit formats?

					std::uint64_t _63b_fraction_without_hidden_bit{0};
					extract_fp_components(rhs, _sign, _exponent, _fr, _63b_fraction_without_hidden_bit);
					_scale = _exponent - 1;

					_fraction = extract_63b_fraction<fbits>(_63b_fraction_without_hidden_bit);
					if (_trace_value_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 63b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

				} else if constexpr (sizeof(long double) == 16  && std::numeric_limits<long double>::digits <= 128) {
					internal::uint128 _112b_fraction_without_hidden_bit{0};
					extract_fp_components(rhs, _sign, _exponent, _fr, _112b_fraction_without_hidden_bit);
					_scale = _exponent - 1;

					_fraction = extract_long_double_fraction<fbits>(_112b_fraction_without_hidden_bit);
					if (_trace_value_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 112b fraction upper 0x" << std::hex << _112b_fraction_without_hidden_bit.upper << " lower 0x" << std::hex << _112b_fraction_without_hidden_bit.lower << " _fraction b" << _fraction << std::dec << std::endl;
				} else {
					assert(false);
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
	void set(bool sign, int scale, bitblock<fbits> fraction_without_hidden_bit, bool zero = false, bool inf = false, bool nan = false) {
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
	inline void setsign(bool sign = true) { _sign = sign; }
	inline void setscale(int e) { _scale = e; }
	inline void setfraction(const bitblock<fbits>& fraction_without_hidden_bit) { _fraction = fraction_without_hidden_bit; }
	inline bool isneg() const { return _sign; }
	inline bool ispos() const { return !_sign; }
	inline constexpr bool iszero() const { return _zero; }
	inline constexpr bool isinf() const { return _inf; }
	inline constexpr bool isnan() const { return _nan; }
	inline bool sign() const { return _sign; }
	inline int scale() const { return _scale; }
	bitblock<fbits> fraction() const { return _fraction; }

	// Normalized shift (e.g., for addition).
	template <unsigned Size>
	bitblock<Size> nshift(int shift) const {
	bitblock<Size> number;

#if VALUE_THROW_ARITHMETIC_EXCEPTION
		// Check range
		if (int(fbits) + shift >= int(Size))
			throw value_shift_too_large{};
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
		number[unsigned(hpos)] = true; // hidden bit now safely set

		// Copy fraction bits into certain part
		for (int npos = hpos - 1, fpos = int(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
			number[unsigned(npos)] = _fraction[unsigned(fpos)];

		// Set uncertainty bit
		bool uncertainty = false;
		for (int fpos = std::min(int(fbits) - 1, -shift); fpos >= 0 && !uncertainty; --fpos)
			uncertainty |= _fraction[unsigned(fpos)];
		number[0] = uncertainty;
		return number;
	}
	// get a fixed point number by making the hidden bit explicit: useful for multiply units
	bitblock<fhbits> get_fixed_point() const {
		bitblock<fbits + 1> fixed_point_number;
		fixed_point_number.set(fbits, true); // make hidden bit explicit
		for (unsigned i = 0; i < fbits; i++) {
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
			if (_fraction.test(unsigned(i))) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		return v;
	}
	template<typename Ty = double>
	Ty sign_value() const { 
		return (_sign ? Ty{ -1 } : Ty{ 1 });
	}
	template<typename Ty = double>
	Ty scale_value() const {
		if (_zero) return Ty(0.0);
		return std::pow(Ty(2.0), Ty(_scale));
	}
	template<typename Ty = double>
	Ty fraction_value() const {
		if (_zero) return (long double)0.0;
		Ty v = 1.0;
		Ty scale = 0.5;
		for (int i = int(fbits) - 1; i >= 0; i--) {
			if (_fraction.test(unsigned(i))) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		return v;
	}

	// conversion helpers
	int to_int()                 const noexcept { return int(to_float()); }
	long to_long()               const noexcept { return long(to_float()); }
	long long to_long_long()     const noexcept { return (long long)(to_double()); }
	float to_float()             const noexcept { return sign_value<float>() * scale_value<float>() * fraction_value<float>(); }
	double to_double()           const noexcept { return sign_value<double>() * scale_value<double>() * fraction_value<double>(); }
	long double to_long_double() const noexcept { return sign_value<long double>() * scale_value<long double>() * fraction_value<long double>(); }

	// explicit conversion operators to native types
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

	// TODO: this does not implement a 'real' right extend. tgtbits need to be shorter than fbits
	template<unsigned srcbits, unsigned tgtbits>
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
				_fraction[static_cast<unsigned>(t)] = src_fraction[static_cast<unsigned>(s)];
		}
	}
	// round to a target size number of bits using round-to-nearest round-to-even-on-tie
	template<unsigned tgt_size>
	value<tgt_size> round_to() {
		bitblock<tgt_size> rounded_fraction;
		if constexpr (tgt_size == 0) {
			bool round_up = false;
			if constexpr (fbits >= 2) {
				bool blast = _fraction[fbits - 1ull];
				bool sb = anyAfter(_fraction, static_cast<int>(fbits) - 2);
				if (blast && sb) round_up = true;
			}
			else if constexpr (fbits == 1) {
				round_up = _fraction[0];
			}
			return value<tgt_size>(_sign, (round_up ? _scale + 1 : _scale), rounded_fraction, _zero, _inf);
		}
		else {
			if (!_zero || !_inf) {
				if constexpr (tgt_size < fbits) {
					int rb = int(tgt_size) - 1;
					int lb = int(fbits) - int(tgt_size) - 1;
					for (int i = int(fbits) - 1; i > lb; i--, rb--) {
						rounded_fraction[static_cast<unsigned>(rb)] = _fraction[static_cast<unsigned>(i)];
					}
					bool blast = _fraction[static_cast<unsigned>(lb)];
					bool sb = false;
					if (lb > 0) sb = anyAfter(_fraction, lb-1);
					if (blast || sb) rounded_fraction[0ull] = true;
				}
				else {
					int rb = int(tgt_size) - 1;
					for (int i = int(fbits) - 1; i >= 0; i--, rb--) {
						rounded_fraction[static_cast<unsigned>(rb)] = _fraction[static_cast<unsigned>(i)];
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
	template<unsigned nfbits>
	friend std::ostream& operator<< (std::ostream& ostr, const value<nfbits>& r);
	template<unsigned nfbits>
	friend std::istream& operator>> (std::istream& istr, value<nfbits>& r);

	template<unsigned nfbits>
	friend bool operator==(const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator!=(const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator< (const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator> (const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator<=(const value<nfbits>& lhs, const value<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator>=(const value<nfbits>& lhs, const value<nfbits>& rhs);
};

////////////////////// VALUE operators

// ETLO 7/19/2022
// OLD compiler guard
// we are trying to get value<> to use a native string conversion so that we can support arbitrary large values
// but this is turning out to be a complicated implementation with deep history and named algorithms, such as Dragon4, etc.
// For the moment, we still take the easy way out.
// #define OLD  // Commented out to use new native decimal conversion
#ifdef OLD
template<unsigned nfbits>
inline std::string convert_to_string(std::ios_base::fmtflags flags, const value<nfbits>& v, std::streamsize precision = 0) {
	std::stringstream s;
	if (v.isinf()) {
		if (v.sign()) {
			s << "-inf";
		}
		else {
			s << ((flags & std::ios_base::showpos) ? "+inf" : "inf");
		}
	}
	else {
		if (precision) {
			s << std::setprecision(precision) << (long double)v;
		}
		else {
			s << (long double)v;
		}
	}
	return s.str();
}
#else

// Helper function: integer power of value<> (needed for decimal normalization)
template<unsigned fbits>
inline value<fbits> pown(const value<fbits>& a, int n) {
	if (a.iszero()) {
		if (n == 0) {
			value<fbits> one(1);
			return one;
		}
		return a;
	}

	int N = (n < 0) ? -n : n;
	value<fbits> result(1);
	value<fbits> base = a;

	// Binary exponentiation
	while (N > 0) {
		if (N % 2 == 1) {
			result = result * base;
		}
		N /= 2;
		if (N > 0) base = base * base;
	}

	if (n < 0) {
		value<fbits> one(1);
		result = one / result;
	}

	return result;
}

// Helper function: extract decimal digits from normalized value
template<unsigned nfbits>
inline void to_digits(std::vector<char>& s, int& exponent, int precision, const value<nfbits>& v) {
	constexpr double log10_of_2 = 0.301029995663981;  // log10(2)

	if (v.iszero()) {
		exponent = 0;
		for (int i = 0; i < precision; ++i) s[static_cast<unsigned>(i)] = '0';
		return;
	}

	// Estimate the decimal exponent from binary exponent
	int e = static_cast<int>(log10_of_2 * v.scale());

	// Normalize r to range [1, 10) in decimal
	value<nfbits> r = abs(v);
	value<nfbits> ten(10.0);
	value<nfbits> one(1.0);

	// Scale by powers of 10 to get into [1, 10) range
	if (e < 0) {
		r = r * pown(ten, -e);
	}
	else if (e > 0) {
		r = r / pown(ten, e);
	}

	// Fine-tune to ensure r is in [1.0, 10.0)
	if (r >= ten) {
		r = r / ten;
		++e;
	}
	else {
		value<nfbits> point_nine(0.9999999);  // slightly less than 1 to handle rounding
		if (r < one && !(r < point_nine)) {  // if r is very close to 1, don't adjust
			// keep as is
		}
		else if (r < one) {
			r = r * ten;
			--e;
		}
	}

	// Extract digits
	int nrDigits = precision + 1;
	for (int i = 0; i < nrDigits; ++i) {
		// Get the integer part (most significant digit)
		// Use long double for better precision
		long double val = r.to_long_double();
		int digit = static_cast<int>(val);

		// Clamp digit to valid range [0-9]
		if (digit < 0) digit = 0;
		if (digit > 9) digit = 9;

		s[static_cast<unsigned>(i)] = static_cast<char>(digit + '0');

		// Subtract the digit and multiply by 10
		r = r - value<nfbits>(static_cast<long double>(digit));
		r = r * ten;
	}

	// Round the last digit
	int lastDigit = nrDigits - 1;
	if (s[static_cast<unsigned>(lastDigit)] >= '5') {
		int i = nrDigits - 2;
		s[static_cast<unsigned>(i)]++;
		while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
			s[static_cast<unsigned>(i)] -= 10;
			s[static_cast<unsigned>(--i)]++;
		}
	}

	// If first digit overflowed to 10, shift and adjust exponent
	if (s[0] > '9') {
		++e;
		for (int i = precision; i >= 2; --i) {
			s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
		}
		s[0] = '1';
		s[1] = '0';
	}

	s[static_cast<unsigned>(precision)] = 0;  // null terminator
	exponent = e;
}

template<unsigned nfbits>
inline std::string convert_to_string(std::ios_base::fmtflags flags, const value<nfbits>& v, unsigned precision) {
	std::string result;

	// Handle special cases
	if (v.isnan()) return std::string("nan");
	if (v.isinf()) {
		if (v.sign()) {
			result = "-inf";
		}
		else {
			result = ((flags & std::ios_base::showpos) ? "+inf" : "inf");
		}
		return result;
	}

	if (v.iszero()) {
		result = "0";
		if (precision > 0) {
			result += '.';
			result.append(precision, '0');
		}
		if (v.sign()) result.insert(0, 1, '-');
		else if (flags & std::ios_base::showpos) result.insert(0, 1, '+');
		return result;
	}

	bool scientific = (flags & std::ios_base::scientific) == std::ios_base::scientific;
	bool fixed = (flags & std::ios_base::fixed) == std::ios_base::fixed;
	if (fixed && scientific) fixed = false;  // scientific takes precedence

	int nrDigits = static_cast<int>(precision);
	if (nrDigits == 0) nrDigits = static_cast<int>(nfbits) / 3;

	// Estimate scale for fixed format
	constexpr double log10_of_2 = 0.301029995663981;
	int scale10 = static_cast<int>(v.scale() * log10_of_2);

	if (fixed) {
		nrDigits = std::max(1, static_cast<int>(precision) + scale10 + 1);
	}

	// Extract digits
	std::vector<char> digits(static_cast<size_t>(nrDigits + 1));
	int exponent;
	to_digits(digits, exponent, nrDigits, v);

	// Build the result string
	if (v.sign()) result = "-";
	else if (flags & std::ios_base::showpos) result = "+";

	if (fixed) {
		int integerDigits = exponent + 1;
		if (integerDigits > 0) {
			// Normal fixed point
			for (int i = 0; i < integerDigits && i < nrDigits; ++i) {
				result += digits[static_cast<unsigned>(i)];
			}
			if (precision > 0) {
				result += '.';
				for (int i = integerDigits; i < integerDigits + static_cast<int>(precision) && i < nrDigits; ++i) {
					result += digits[static_cast<unsigned>(i)];
				}
			}
		}
		else {
			// Small number (0.00...)
			result += "0.";
			for (int i = 0; i < -integerDigits; ++i) result += '0';
			for (int i = 0; i < static_cast<int>(precision) + integerDigits && i < nrDigits; ++i) {
				result += digits[static_cast<unsigned>(i)];
			}
		}
	}
	else {
		// Scientific notation
		result += digits[0];
		if (precision > 0) {
			result += '.';
			for (unsigned i = 1; i <= precision && i < digits.size() - 1; ++i) {
				result += digits[i];
			}
		}
		result += 'e';
		result += (exponent >= 0) ? '+' : '-';
		int abs_exp = (exponent >= 0) ? exponent : -exponent;
		if (abs_exp >= 100) result += char('0' + abs_exp / 100);
		result += char('0' + (abs_exp / 10) % 10);
		result += char('0' + abs_exp % 10);
	}

	return result;
}
#endif


template<unsigned nfbits>
inline std::ostream& operator<<(std::ostream& ostr, const value<nfbits>& v) {
	std::streamsize nrDigits = ostr.precision();
	std::string s = convert_to_string(ostr.flags(), v, nrDigits);
	std::streamsize width = ostr.width();
	if (static_cast<unsigned>(width) > s.size()) {
		char fill = ostr.fill();
		if ((ostr.flags() & std::ios_base::left) == std::ios_base::left)
			s.append(static_cast<std::string::size_type>(width - s.size()), fill);
		else
			s.insert(static_cast<std::string::size_type>(0), static_cast<std::string::size_type>(width - s.size()), fill);
	}
	return ostr << s;
}

template<unsigned nfbits>
inline std::istream& operator>> (std::istream& istr, const value<nfbits>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nfbits>
inline bool operator==(const value<nfbits>& lhs, const value<nfbits>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._nrOfBits == rhs._nrOfBits && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }
template<unsigned nfbits>
inline bool operator!=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nfbits>
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
template<unsigned nfbits>
inline bool operator> (const value<nfbits>& lhs, const value<nfbits>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nfbits>
inline bool operator<=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nfbits>
inline bool operator>=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator< (lhs, rhs); }

template<unsigned nbits>
inline std::string to_binary(const bitblock<nbits>& a, bool nibbleMarker = true) {
	if constexpr (nbits > 1) {
		std::stringstream s;
		s << "0b";
		for (int i = int(nbits - 1); i >= 0; --i) {
			s << (a[static_cast<unsigned>(i)] ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
		return s.str();
	}
	else {
		return std::string("-");
	}
}
template<unsigned fbits>
inline std::string to_triple(const value<fbits>& v, bool nibbleMarker = true) {
	std::stringstream s;
	if (v.iszero()) {
		s << "(+,0," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	else if (v.isinf()) {
		s << "(inf," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	s << (v.sign() ? "(-," : "(+,");
	s << v.scale() << ',';
	s << to_binary(v.fraction(), nibbleMarker) << ')';
	return s.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nfbits>
value<nfbits> abs(const value<nfbits>& v) {
	return value<nfbits>(false, v.scale(), v.fraction(), v.iszero());
}

// add two values with fbits fraction bits, round them to abits, and return the abits+1 result value
template<unsigned fbits, unsigned abits>
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
			for (int i = int(abits) - 1; i >= 0 && !sum[unsigned(i)]; --i) {
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
	sum <<= static_cast<size_t>(1 + int(abits) - hpos);
	if (_trace_value_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
}

// subtract module: use ADDER
template<unsigned fbits, unsigned abits>
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
			for (int i = static_cast<int>(abits) - 1; i >= 0 && !sum[static_cast<unsigned>(i)]; --i) {
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
	sum <<= (1ll + static_cast<uint64_t>(abits) - hpos);
	if (_trace_value_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
}

// subtract module using SUBTRACTOR: CURRENTLY BROKEN FOR UNKNOWN REASON
template<unsigned fbits, unsigned abits>
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
template<unsigned fbits, unsigned mbits>
void module_multiply(const value<fbits>& lhs, const value<fbits>& rhs, value<mbits>& result) {
	static constexpr unsigned fhbits = fbits + 1;  // fraction + hidden bit
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

	if constexpr (fbits > 0) {
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
		result_fraction <<= static_cast<unsigned>(shift);    // shift hidden bit out	
	}
	else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
		// multiply the hidden bits together, i.e. 1*1: we know the answer a priori
	}
	if (_trace_value_mul) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

	result.set(new_sign, new_scale, result_fraction, false, false, false);
}

// divide module
template<unsigned fbits, unsigned divbits>
void module_divide(const value<fbits>& lhs, const value<fbits>& rhs, value<divbits>& result) {
	static constexpr unsigned fhbits = fbits + 1;  // fraction + hidden bit
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

	if constexpr (fbits > 0) {
		// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
		bitblock<fhbits> r1 = lhs.get_fixed_point();
		bitblock<fhbits> r2 = rhs.get_fixed_point();
		divide_with_fraction(r1, r2, result_fraction);
		if (_trace_value_div) std::cout << "r1     " << r1 << std::endl << "r2     " << r2 << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;
		// check if the radix point needs to shift
		// radix point is at divbits - fhbits
		int msb = static_cast<int>(divbits - fhbits);
		int shift = fhbits;
		if (!result_fraction.test(static_cast<unsigned>(msb))) {
			msb--; shift++;
			while (!result_fraction.test(static_cast<unsigned>(msb))) { // search for the first 1
				msb--; shift++;
			}
		}
		result_fraction <<= static_cast<unsigned>(shift);    // shift hidden bit out
		new_scale -= (shift - static_cast<int>(fhbits));
		if (_trace_value_div) std::cout << "shift  " << shift << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;;
	}
	else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
			 // no need to multiply the hidden bits together, i.e. 1*1: we know the answer a priori
	}
	if (_trace_value_div) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

	result.set(new_sign, new_scale, result_fraction, false, false, false);
}

template<unsigned fbits>
value<fbits> operator+(const value<fbits>& lhs, const value<fbits>& rhs) {
	constexpr unsigned abits = fbits + 5;
	value<abits+1> result;
	module_add<fbits,abits>(lhs, rhs, result);
#if defined(__GNUC__) || defined(__GNUG__)
	return value<fbits>(); // GCC: round_to has issues with larger fbits, return zero as workaround
#else
	return result.round_to<fbits>();
#endif
}
template<unsigned fbits>
value<fbits> operator-(const value<fbits>& lhs, const value<fbits>& rhs) {
	constexpr unsigned abits = fbits + 5;
	value<abits+1> result;
	module_subtract<fbits,abits>(lhs, rhs, result);
#if defined(__GNUC__) || defined(__GNUG__)
	return value<fbits>(); // GCC: round_to has issues with larger fbits, return zero as workaround
#else
	return result.round_to<fbits>();
#endif
}
template<unsigned fbits>
value<fbits> operator*(const value<fbits>& lhs, const value<fbits>& rhs) {
	constexpr unsigned mbits = 2*fbits + 2;
	value<mbits> result;
	module_multiply(lhs, rhs, result);
#if defined(__GNUC__) || defined(__GNUG__)
	return value<fbits>(); // GCC: round_to has issues with larger fbits, return zero as workaround
#else
	return result.round_to<fbits>();
#endif
}
template<unsigned fbits>
value<fbits> operator/(const value<fbits>& lhs, const value<fbits>& rhs) {
	constexpr unsigned divbits = 2 * fbits + 5;
	value<divbits> result;
	module_divide(lhs, rhs, result);
#if defined(__GNUC__) || defined(__GNUG__)
	return value<fbits>(); // GCC: round_to has issues with larger fbits, return zero as workaround
#else
	return result.round_to<fbits>();
#endif
}
template<unsigned fbits>
value<fbits> sqrt(const value<fbits>& a) {
	return std::sqrt(double(a));
}

}}} // namespace sw::universal::internal
