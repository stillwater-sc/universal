#pragma once
// areal.hpp: definition of a variable float representation that mimics the posit configuration
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee-754.hpp>
#include <universal/blockbin/blockbinary.hpp>
#include <universal/posit/value.hpp>         // TODO: this should be extracted to its own place

namespace sw {	namespace unum {
		
// Forward definitions
template<size_t nbits, size_t es, typename BlockType> class areal;
template<size_t nbits, size_t es, typename BlockType> areal<nbits,es,BlockType> abs(const areal<nbits,es,BlockType>& v);

template<size_t nbits, size_t es, typename BlockType>
void extract_fields(const blockbinary<nbits, BlockType>& raw_bits, bool& _sign, blockbinary<es, BlockType>& _exponent, blockbinary<nbits - es - 1, BlockType>& _fraction) {

}

// needed to avoid double rounding situations: TODO: does that mean the condensed version above should be removed?
template<size_t nbits, size_t es, typename BlockType, size_t fbits>
inline areal<nbits, es, BlockType>& convert_(bool _sign, int _scale, const blockbinary<fbits, BlockType>& fraction_in, areal<nbits, es, BlockType>& r) {
	if (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
	if (_trace_conversion) std::cout << "sign " << (_sign ? "-1 " : " 1 ") << "scale " << std::setw(3) << _scale << " fraction " << fraction_in << std::endl;

	r.reset();

	return r;
}

// convert a floating point value to a specific areal configuration. Semantically, p = v, return reference to p
template<size_t nbits, size_t es, typename BlockType>
inline areal<nbits, es, BlockType>& convert(const value<nbits - es - 1>& v, areal<nbits, es, BlockType>& p) {
	constexpr size_t fbits = nbits - es - 1;
	if (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
	if (_trace_conversion) std::cout << "sign " << (v.sign() ? "-1 " : " 1 ") << "scale " << std::setw(3) << v.scale() << " fraction " << v.fraction() << std::endl;

	if (v.iszero()) {
		p.setzero();
		return p;
	}
	if (v.isnan() || v.isinf()) {
		p.setnan();
		return p;
	}
	return convert_<nbits, es, fbits>(v.sign(), v.scale(), v.fraction(), p);
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<size_t nbits, size_t es, typename BlockType = uint8_t>
class areal {
public:
	static constexpr size_t fbits  = nbits - 1 - es;    // number of fraction bits excluding the hidden bit
	static constexpr size_t fhbits = fbits + 1;         // number of fraction bits including the hidden bit
	static constexpr size_t abits = fhbits + 3;         // size of the addend
	static constexpr size_t mbits = 2 * fhbits;         // size of the multiplier output
	static constexpr size_t divbits = 3 * fhbits + 4;   // size of the divider output

	areal() {}

	areal(signed char initial_value)        { *this = initial_value; }
	areal(short initial_value)              { *this = initial_value; }
	areal(int initial_value)                { *this = initial_value; }
	areal(long long initial_value)          { *this = initial_value; }
	areal(unsigned long long initial_value) { *this = initial_value; }
	areal(float initial_value)              { *this = initial_value; }
	areal(double initial_value)             { *this = initial_value; }
	areal(long double initial_value)        { *this = initial_value; }
	areal(const areal& rhs)                 { *this = rhs; }

	// assignment operators
	areal& operator=(signed char rhs) {
		return *this = (long long)(rhs);
	}
	areal& operator=(short rhs) {
		return *this = (long long)(rhs);
	}
	areal& operator=(int rhs) {
		return *this = (long long)(rhs);
	}
	areal& operator=(long long rhs) {
		return *this;
	}
	areal& operator=(unsigned long long rhs) {
		return *this;
	}
	areal& operator=(float rhs) {

		return *this;
	}
	areal& operator=(double rhs) {

		return *this;
	}
	areal& operator=(long double rhs) {

		return *this;
	}

	// arithmetic operators
	// prefix operator
	areal operator-() const {				
		return areal<nbits,es>(!_sign, _scale, _fraction, _zero, _inf);
	}

	areal& operator+=(const areal& rhs) {
		return *this;
	}
	areal& operator+=(double rhs) {
		return *this += areal(rhs);
	}
	areal& operator-=(const areal& rhs) {

		return *this;
	}
	areal& operator-=(double rhs) {
		return *this -= areal<nbits, es>(rhs);
	}
	areal& operator*=(const areal& rhs) {

		return *this;
	}
	areal& operator*=(double rhs) {
		return *this *= areal<nbits, es>(rhs);
	}
	areal& operator/=(const areal& rhs) {

		return *this;
	}
	areal& operator/=(double rhs) {
		return *this /= areal<nbits, es>(rhs);
	}
	areal& operator++() {
		return *this;
	}
	areal operator++(int) {
		areal tmp(*this);
		operator++();
		return tmp;
	}
	areal& operator--() {
		return *this;
	}
	areal operator--(int) {
		areal tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	void reset() {	}

	// selectors
	inline bool isneg() const { return _sign; }
	inline bool iszero() const { return _zero; }
	inline bool isinf() const { return _inf; }
	inline bool isnan() const { return _nan; }
	inline bool sign() const { return _sign; }
	inline int scale() const { return _scale; }


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

	// currently, size is tied to fbits size of areal config. Is there a need for a case that captures a user-defined sized fraction?
	value<fbits> to_value() const {
		bool		     	 _sign;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
		decode(_raw_bits, _sign, _exponent, _fraction);
		return value<fbits>(_sign, _exponent.scale(), _fraction.get(), iszero(), isnan());
	}
	void normalize(value<fbits>& v) const {
		bool		     	 _sign;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
		decode(_raw_bits, _sign, _exponent, _fraction);
		v.set(_sign, _exponent.scale(), _fraction.get(), iszero(), isnan());
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

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t nes, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const areal<nnbits,nes,nbt>& r);
	template<size_t nnbits, size_t nes, typename nbt>
	friend std::istream& operator>> (std::istream& istr, areal<nnbits,nes,nbt>& r);

	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator==(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator!=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator< (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator> (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator<=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator>=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
};

////////////////////// operators
template<size_t nnbits, size_t nes, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const areal<nnbits,nes,nbt>& v) {

	return ostr;
}

template<size_t nnbits, size_t nes, typename nbt>
inline std::istream& operator>>(std::istream& istr, const areal<nnbits,nes,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nnbits, size_t nes, typename nbt>
inline bool operator==(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return false; }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator!=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator< (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return false; }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator> (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator<=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator>=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator< (lhs, rhs); }

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t es, typename BlockType>
inline areal<nbits, es, BlockType> operator+(const areal<nbits, es, BlockType>& lhs, const areal<nbits, es, BlockType>& rhs) {
	areal<nbits, es> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t es, typename BlockType>
inline areal<nbits, es, BlockType> operator-(const areal<nbits, es, BlockType>& lhs, const areal<nbits, es, BlockType>& rhs) {
	areal<nbits, es> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t es, typename BlockType>
inline areal<nbits, es, BlockType> operator*(const areal<nbits, es, BlockType>& lhs, const areal<nbits, es, BlockType>& rhs) {
	areal<nbits, es> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t es, typename BlockType>
inline areal<nbits, es, BlockType> operator/(const areal<nbits, es, BlockType>& lhs, const areal<nbits, es, BlockType>& rhs) {
	areal<nbits, es> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<size_t nbits, size_t es, typename BlockType>
inline std::string components(const areal<nbits,es,BlockType>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << " zero b" << std::setw(nbits) << v.fraction();
		return s.str();
	}
	else if (v.isinf()) {
		s << " infinite b" << std::setw(nbits) << v.fraction();
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
	return s.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<size_t nbits, size_t es, typename BlockType>
areal<nbits,es> abs(const areal<nbits,es,BlockType>& v) {
	return areal<nbits,es>(false, v.scale(), v.fraction(), v.isZero());
}


}}  // namespace sw::unum
