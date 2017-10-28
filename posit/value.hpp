#pragma once
// posit.hpp: definition of arbitrary posit number configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// template class representing a value in scientific notation, using a template size for the fraction bits
template<size_t fbits>
class value {
public:
	value() : _sign(false), _scale(0), _nrOfBits(fbits) {}

	value(int8_t initial_value) {
		*this = initial_value;
	}
	value(int16_t initial_value) {
		*this = initial_value;
	}
	value(int32_t initial_value) {
		*this = initial_value;
	}
	value(int64_t initial_value) {
		*this = initial_value;
	}
	value(uint64_t initial_value) {
		*this = initial_value;
	}
	value(float initial_value) {
		*this = initial_value;
	}
	value(double initial_value) {
		*this = initial_value;
	}
	value(const value& rhs) {
		*this = rhs;
	}
	value& operator=(const value& rhs) {
		_sign	  = rhs._sign;
		_scale	  = rhs._scale;
		_fraction = rhs._fraction;
		_nrOfBits = rhs._nrOfBits;
		_inf      = rhs._inf;
		_zero     = rhs._zero;
		_nan      = rhs._nan;
		return *this;
	}
	value<fbits>& operator=(int8_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	value<fbits>& operator=(int16_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	value<fbits>& operator=(int32_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	value<fbits>& operator=(int64_t rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		_sign = (0x8000000000000000 & rhs);  // 1 is negative, 0 is positive
		if (_sign) {
			// process negative number: process 2's complement of the input
			_scale = findMostSignificantBit(-rhs) - 1;
			uint64_t _fraction_without_hidden_bit = (-rhs << (64 - _scale));
			_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
			//take_2s_complement();
			_nrOfBits = fbits;
			if (_trace_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		}
		else {
			// process positive number
			if (rhs != 0) {
				_scale = findMostSignificantBit(rhs) - 1;
				uint64_t _fraction_without_hidden_bit = (rhs << (64 - _scale));
				_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
				_nrOfBits = fbits;
				if (_trace_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;

			}
		}
		return *this;
	}
	value<fbits>& operator=(uint64_t rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		if (rhs == 0) {
			_zero = true;
		}
		else {
			_scale = findMostSignificantBit(rhs) - 1;
			uint64_t _fraction_without_hidden_bit = (rhs << (64 - _scale));
			_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
			_nrOfBits = fbits;
		}
		if (_trace_conversion) std::cout << "uint64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		return *this;
	}
	value<fbits>& operator=(float rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_nrOfBits = fbits;
			_zero = true;
			break;
		case FP_INFINITE:
			_inf  = true;
			break;
		case FP_NAN:
			_nan = true;
			break;
		case FP_SUBNORMAL:
			std::cerr << "TODO: subnormal number: returning 0" << std::endl;
			break;
		case FP_NORMAL:
		{
			_sign = extract_sign(rhs);
			_scale = extract_exponent(rhs) - 1;
			uint32_t _23b_fraction_without_hidden_bit = extract_fraction(rhs);
			_fraction = extract_float_fraction<fbits>(_23b_fraction_without_hidden_bit);
			_nrOfBits = fbits;
			if (_trace_conversion) std::cout << "float " << rhs << " sign " << _sign << " scale " << _scale << " 23b fraction 0x" << std::hex << _23b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
		}
		break;
		}
		return *this;
	}
	value<fbits>& operator=(double rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_nrOfBits = fbits;
			_zero = true;
			break;
		case FP_INFINITE:
			_inf = true;
			break;
		case FP_NAN:
			_nan = true;
			break;
		case FP_SUBNORMAL:
			std::cerr << "TODO: subnormal number: returning 0" << std::endl;
			break;
		case FP_NORMAL:
		{
			_sign = extract_sign(rhs);
			_scale = extract_exponent(rhs) - 1;
			uint64_t _52b_fraction_without_hidden_bit = extract_fraction(rhs);
			_fraction = extract_double_fraction<fbits>(_52b_fraction_without_hidden_bit);
			_nrOfBits = fbits;
			if (_trace_conversion) std::cout << "double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _52b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
		}
		break;
		}
		return *this;
	}
	void reset() {
		_sign  = false;
		_scale = 0;
		_nrOfBits = 0;
		_inf = false;
		_zero = false;
		_nan = false;
		_fraction.reset();
	}
	bool isNegative() {	return _sign; }
	bool isZero() { return _zero; }
	bool isInfinite() { return _inf; }
	bool isNaN() { return _nan; }
	bool sign() const { return _sign; }
	int scale() const { return _scale; }
	std::bitset<fbits> fraction() const { return _fraction; }
	double sign_value() const {	return (_sign ? -1.0 : 1.0); }
	double scale_value() const {
		double v = 0.0;
		if (_zero) return v;
		// TODO: breaks when _scale is larger than 64
		if (_scale >= 0) {
			v = double(uint64_t(1) << _scale);
		}
		else {
			v = double(1.0)/double(uint64_t(1) << -_scale);
		}
		return v;  
	}
	double fraction_value() const {
		// TODO: this fails when fbits > 64 and we cannot represent the fraction by a 64bit unsigned integer
		return double(_fraction.to_ullong()) / double(uint64_t(1) << (fbits));
	}
	double to_double() const {
		return sign_value() * scale_value() * (1.0 + fraction_value());
	}
private:
	bool				_sign;
	int					_scale;
	std::bitset<fbits>	_fraction;
	int					_nrOfBits;  // in case the fraction is smaller than the full fbits
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
