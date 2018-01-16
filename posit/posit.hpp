#pragma once
// posit.hpp: definition of arbitrary posit number configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>
#include <cassert>
#include <iostream>
#include <limits>

#include "../bitset/bitset_helpers.hpp"
#include "../bitset/bitset_logic.hpp"
#include "../bitset/bitset_arithmetic.hpp"
#include "exceptions.hpp"
#include "bit_functions.hpp"
#include "trace_constants.hpp"
#include "posit_functions.hpp"
#include "value.hpp"
#include "fraction.hpp"
#include "exponent.hpp"
#include "regime.hpp"

namespace sw {
	namespace unum {

// Forward definitions
template<size_t nbits, size_t es> class posit;
template<size_t nbits, size_t es> posit<nbits, es> abs(const posit<nbits, es>& p);




/*
 class posit represents arbitrary configuration posits and their basic arithmetic operations (add/sub, mul/div)
 */
template<size_t nbits, size_t es>
class posit 
{
//	static_assert(es + 3 <= nbits, "Value for 'es' is too large for this 'nbits' value");

	template <typename T>
	posit<nbits, es>& float_assign(const T& rhs) {
		constexpr int dfbits = std::numeric_limits<T>::digits - 1;
		value<dfbits> v(rhs);

		// special case processing
		if (v.isZero()) {
			setToZero();
			return *this;
		}
		if (v.isInfinite() || v.isNaN()) {  // posit's encode NaN as -inf
			setToInfinite();
			return *this;
		}

		convert_to_posit(v);
		return *this;
	}
    
public:
	static constexpr size_t rbits = nbits - 1;
	static constexpr size_t ebits = es;
	static constexpr size_t fbits = nbits - 3 - es;  
	static constexpr size_t abits = fbits + 4;       // size of the addend
	static constexpr size_t fhbits = fbits + 1;      // size of fraction + hidden bit
	static constexpr size_t mbits  = 2 * fhbits;     // size of the multiplier output

	posit<nbits, es>() : _sign(false) {}
	
	posit(const posit&) = default;
	posit(posit&&) = default;
	
	posit& operator=(const posit&) = default;
	posit& operator=(posit&&) = default;
	
	/// Construct posit from its components
	posit(bool sign, const regime<nbits, es>& r, const exponent<nbits, es>& e, const fraction<fbits>& f)
          : _sign(sign), _regime(r), _exponent(e), _fraction(f) {
		// generate raw bit representation
		_raw_bits = _sign ? twos_complement(collect()) : collect();
		_raw_bits.set(nbits - 1, _sign);
	}
	/// Construct posit from raw bits
	posit(const std::bitset<nbits>& raw_bits) {
		*this = set(raw_bits);
	}
	posit<nbits, es>(int64_t initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(uint64_t initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(int32_t initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(float initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(double initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>& operator=(int8_t rhs) {
		setToZero();
		value<8> v(rhs);
		if (v.isZero()) {
			return *this;
		}
		else if (v.isNegative()) {
			convert_to_posit(v);
			take_2s_complement();
		}
		else {
			convert_to_posit(v);
		}
		return *this;
	}
	posit<nbits, es>& operator=(int16_t rhs) {
		setToZero();
		value<16> v(rhs);
		if (v.isZero()) {
			return *this;
		}
		else if (v.isNegative()) {
			convert_to_posit(v);
			take_2s_complement();
		}
		else {
			convert_to_posit(v);
		}
		return *this;
	}
	posit<nbits, es>& operator=(int32_t rhs) {
		setToZero();
		value<32> v(rhs);
		if (v.isZero()) {
			return *this;
		}
		else if (v.isNegative()) {
			convert_to_posit(v);
			take_2s_complement();
		}
		else {
			convert_to_posit(v);
		}
		return *this;
	}
	posit<nbits, es>& operator=(int64_t rhs) {
		setToZero();
		value<64> v(rhs);
		if (v.isZero()) {
			return *this;
		}
		else if (v.isNegative()) {
			convert_to_posit(v);
			take_2s_complement();
		}
		else {
			convert_to_posit(v);
		}
		return *this;
	}
	posit<nbits, es>& operator=(uint64_t rhs) {
		setToZero();
		value<64> v(rhs);
		if (v.isZero()) {
			return *this;
		}
		convert_to_posit(v);
		return *this;
	}
	posit<nbits, es>& operator=(float rhs) {
		return float_assign(rhs);
	}
	posit<nbits, es>& operator=(double rhs) {
                return float_assign(rhs);
	}
	posit<nbits, es> operator-() const {
		if (isZero()) {
			return *this;
		}
		if (isInfinite()) {
			return *this;
		}
		posit<nbits, es> negated;
		negated.decode(twos_complement(_raw_bits));
		return negated;
	}
	
	posit<nbits, es>& operator+=(const posit& rhs) 
	{
		// with sign/magnitude adders it is customary to organize the computation 
		// along the four quadrants of sign combinations
		//  + + = +
		//  + - =   lhs > rhs ? + : -
		//  - + =   lhs > rhs ? - : +
		//  - - = -
		// to simplify the result processing
		// By assigning the biggest absolute value to R1, the sign of the result will be sign of lhs.
     
		if (_trace_add) std::cout << "---------------------- ADD -------------------" << std::endl;
		if (isZero()) {
			*this = rhs;
			return *this;
		} else if (rhs.isZero()) {
			return *this;
		} else if (isInfinite()) {
			return *this;
		} else if (rhs.isInfinite()) {
			*this = rhs;
			return *this;
		}
		
		int lhs_scale = scale(), rhs_scale = rhs.scale(), scale_of_result= std::max(lhs_scale, rhs_scale);
		
		// align the fractions
        std::bitset<abits> r1 = _fraction.template nshift<abits>(lhs_scale - scale_of_result + 3), 
                           r2 = rhs._fraction.template nshift<abits>(rhs_scale - scale_of_result + 3);
        bool r1_sign = _sign, r2_sign = rhs._sign;
                

        if (sw::unum::abs(*this) < sw::unum::abs(rhs)) {
            std::swap(r1, r2);
            std::swap(r1_sign, r2_sign);
        } 

		if (_trace_add) {
			std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
			std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
		}
		
		if (r1_sign != r2_sign) r2 = twos_complement(r2);
        
		std::bitset<abits+1> sum;
		const bool carry = add_unsigned(r1, r2, sum);

		if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum      " << sum << std::endl;
                
		long shift = 0;
		if (carry) {
			if (r1_sign == r2_sign)   // the carry && signs= implies that we have a number bigger than r1
				shift = -1;
			else 
				// the carry && signs!= implies r2 is complement, result < r1, must find hidden bit (in the complement)
				for (int i = abits - 1; i >= 0 && !sum[i]; i--)
					shift++;
		}
		assert(shift >= -1);
                
		if (shift >= long(abits)) {            // we have actual 0                            
			setToZero();
			return *this;
		}
                
		scale_of_result -= shift;
		const int hpos = abits - 1 - shift;         // position hidden bit 
#ifdef ALGO1
		convert(r1_sign, scale_of_result, sum, hpos);
#else
		sum <<= abits - hpos + 1;
		if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum      " << sum << std::endl;
		convert(r1_sign, scale_of_result, sum);
#endif
		return *this;                
	}
	posit<nbits, es>& operator-=(const posit& rhs) {
                return *this += -rhs;
	}
	posit<nbits, es>& operator*=(const posit& rhs) {
		if (_trace_mul) std::cout << "---------------------- MUL -------------------" << std::endl;
		if (_trace_mul) std::cout << *this << " * " << rhs << std::endl;
		// since we are encoding error conditions as -inf, we need to process -inf condition first
		if (isInfinite()) {
			return *this;
		}
		else if (rhs.isInfinite()) {
			*this = rhs;
			return *this;
		}
		else if (isZero()) {
			return *this;
		}
		else if (rhs.isZero()) {
			*this = rhs;
			return *this;
		}
		value<fbits> v1, v2;
		v1 = convert_to_scientific_notation();
		v2 = rhs.convert_to_scientific_notation();
		value<mbits> result;
		multiply(v1, v2, result);
		// this path rounds each multiply
		//value<fbits> rounded = result.template round_to<fbits>();
		convert_to_posit(result);
		return *this;
	}
	posit<nbits, es>& operator/=(const posit& rhs) {
		if (_trace_div) std::cout << "---------------------- DIV -------------------" << std::endl;
		// since we are encoding error conditions as -inf, we need to process -inf condition first
		if (rhs.isZero()) {
			setToInfinite();
			return *this;
			//throw divide_by_zero{};
		}
		
		if (isZero() || isInfinite()) {
			return *this;
		}

		if (rhs.isInfinite()) {
			setToZero();
			return *this;
		}

		value<fbits> v1, v2;
		v1 = convert_to_scientific_notation();
		posit<nbits, es> reciprocal = rhs.reciprocate();
		v2 = reciprocal.convert_to_scientific_notation();
		value<mbits> result;
		multiply(v1, v2, result);
		convert_to_posit(result); 
		return *this;
	}
	posit<nbits, es>& operator++() {
		increment_posit();
		return *this;
	}
	posit<nbits, es> operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit<nbits, es>& operator--() {
		decrement_posit();
		return *this;
	}
	posit<nbits, es> operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}

	posit<nbits, es> reciprocate() const {
		if (_trace_reciprocate) std::cout << "-------------------- RECIPROCATE ----------------" << std::endl;
		posit<nbits, es> p;
		// special case of inf
		if (isInfinite()) {
			p.setToZero();
			return p;
		}
		if (isZero()) {
			p.setToInfinite();
			return p;
		}
		// compute the reciprocal
		bool old_sign = _sign;
		std::bitset<nbits> raw_bits;
		if (isPowerOf2()) {
			raw_bits = twos_complement(_raw_bits);
			raw_bits.set(nbits-1, old_sign);
			p.set(raw_bits);
		}
		else {
			constexpr size_t operand_size = fhbits;
			std::bitset<operand_size> one;
			one.set(operand_size - 1, true);
			std::bitset<operand_size> frac;
			copy_into(_fraction.get(), 0, frac);
			frac.set(operand_size - 1, true);
			constexpr size_t reciprocal_size = 2 * fbits + 3;
			std::bitset<reciprocal_size> reciprocal;
			std::cout << "one    " << one << std::endl;
			std::cout << "frac   " << frac << std::endl;
			divide_with_fraction(one, frac, reciprocal);
			std::cout << "recip  " << reciprocal << std::endl;
			// radix point falls at operand size == reciprocal_size - operand_size - 1
			reciprocal <<= operand_size - 1;
			std::cout << "frac   " << reciprocal << std::endl;
			int new_scale = -scale();
			int msb = findMostSignificantBit(reciprocal);
			if (msb > 0) {
				int shift = reciprocal_size - msb;
				reciprocal <<= shift;
				new_scale -= (shift-1);
				std::cout << "result " << reciprocal << std::endl;
			}
			std::bitset<operand_size> tr;
			truncate(reciprocal, tr);
			std::cout << "tr     " << tr << std::endl;
			p.convert(_sign, new_scale, tr);
		}
		return p;
	}
	// SELECTORS
	bool isInfinite() const {
		return (_sign & _regime.isZero());
	}
	bool isZero() const {
		return (!_sign & _regime.isZero());
	}
	bool isOne() const { // pattern 010000....
		std::bitset<nbits> tmp(_raw_bits);
		tmp.set(nbits - 2, false);
		bool oneBitSet = tmp.none();
		return !_sign & oneBitSet;
	}
	bool isMinusOne() const { // pattern 110000...
		std::bitset<nbits> tmp(_raw_bits);
		tmp.set(nbits - 1, false);
		tmp.set(nbits - 2, false);
		bool oneBitSet = tmp.none();
		return _sign & oneBitSet;
	}
	bool isNegative() const {
		return _sign;
	}
	bool isPositive() const {
		return !_sign;
	}
	bool isPowerOf2() const {
		return _fraction.none();
	}

	int	   sign_value() const {
		return (_sign ? -1 : 1);
	}
	double regime_value() const {
		return _regime.value();
	}
	double exponent_value() const {
		return _exponent.value();
	}
	double fraction_value() const {
		return _fraction.value();
	}

	int				   regime_k() const {
		return _regime.regime_k();
	}
	bool               get_sign() const { return _sign;  }
	regime<nbits, es>  get_regime() const {
		return _regime;
	}
	exponent<nbits,es> get_exponent() const {
		return _exponent;
	}
	fraction<fbits>    get_fraction() const {
		return _fraction;
	}
	std::bitset<nbits> get() const {
		return _raw_bits;
	}
	std::bitset<nbits> get_decoded() const {
		std::bitset<rbits> r = _regime.get();
		unsigned int nrRegimeBits = _regime.nrBits();
		std::bitset<es> e = _exponent.get();
		unsigned int nrExponentBits = _exponent.nrBits();
		std::bitset<fbits> f = _fraction.get();
		unsigned int nrFractionBits = _fraction.nrBits();

		std::bitset<nbits> _Bits;
		_Bits.set(nbits - 1, _sign);
		int msb = nbits - 2;
		for (unsigned int i = 0; i < nrRegimeBits; i++) {
			_Bits.set(std::size_t(msb--), r[nbits - 2 - i]);
		}
		if (msb < 0) 
                    return _Bits;
		for (unsigned int i = 0; i < nrExponentBits && msb >= 0; i++) {
			_Bits.set(std::size_t(msb--), e[es - 1 - i]);
		}
		if (msb < 0) return _Bits;
		for (unsigned int i = 0; i < nrFractionBits && msb >= 0; i++) {
			_Bits.set(std::size_t(msb--), f[fbits - 1 - i]);
		}
		return _Bits;
	}
	std::string        get_quadrant() const {
		posit<nbits, es> pOne(1), pMinusOne(-1);
		if (_sign) {
			// west
			if (*this > pMinusOne) {
				return "SW";
			}
			else {
				return "NW";
			}
		}
		else {
			// east
			if (*this < pOne) {
				return "SE";
			}
			else {
				return "NE";
			}
		}
	}

	// MODIFIERS
	void setToZero() {
		_sign = false;
		_regime.setToZero();
		_exponent.reset();
		_fraction.reset();
		_raw_bits.reset();
	}
	void setToInfinite() {
		_sign = true;
		_regime.setToInfinite();
		_exponent.reset();
		_fraction.reset();
		_raw_bits.reset();
		_raw_bits.set(nbits - 1, true);
	}
	posit<nbits, es>&  set(const std::bitset<nbits>& raw_bits) {
		decode(raw_bits);
		return *this;
	}
	// Set the raw bits of the posit given a binary pattern
	posit<nbits,es>& set_raw_bits(uint64_t value) {
		setToZero();
		std::bitset<nbits> raw_bits;
		unsigned long mask = 1;
		for ( int i = 0; i < nbits; i++ ) {
			raw_bits.set(i,(value & mask));
			mask <<= 1;
		}
		// decode to cache the posit number interpretation
		decode(raw_bits);
		return *this;
	}
	int decode_regime(std::bitset<nbits>& raw_bits) {
		// let m be the number of identical bits in the regime
		int m = 0;   // regime runlength counter
		int k = 0;   // converted regime scale
		if (raw_bits[nbits - 2] == 1) {   // run length of 1's
			m = 1;   // if a run of 1's k = m - 1
			for (int i = nbits - 3; i >= 0; --i) {
				if (raw_bits[i] == 1) {
					m++;
				}
				else {
					break;
				}
			}
			k = m - 1;
		}
		else {
			m = 1;  // if a run of 0's k = -m
			for (int i = nbits - 3; i >= 0; --i) {
				if (raw_bits[i] == 0) {
					m++;
				}
				else {
					break;
				}
			}
			k = -m;
		}
		return k;
	}
	// decode takes the raw bits representing a posit coming from memory
	// and decodes the regime, the exponent, and the fraction.
	// This function has the functionality of the posit register-file load.
	void extract_fields(const std::bitset<nbits>& raw_bits) {
		std::bitset<nbits> tmp(raw_bits);
		if (_sign) tmp = twos_complement(tmp);
		unsigned int nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));

		// get the exponent bits
		// start of exponent is nbits - (sign_bit + regime_bits)
		int32_t msb = nbits - 1 - (1 + nrRegimeBits);
		unsigned int nrExponentBits = 0;
		if (es > 0) {
			std::bitset<es> _exp;
			if (msb >= 0 && es > 0) {
				nrExponentBits = (msb >= es - 1 ? es : msb + 1);
				for (unsigned int i = 0; i < nrExponentBits; i++) {
					_exp[es - 1 - i] = tmp[msb - i];
				}
			}
			_exponent.set(_exp, nrExponentBits);
		}

		// finally, set the fraction bits
		// we do this so that the fraction is right extended with 0;
		// The max fraction is <nbits - 3 - es>, but we are setting it to <nbits - 3> and right-extent
		// The msb bit of the fraction represents 2^-1, the next 2^-2, etc.
		// If the fraction is empty, we have a fraction of nbits-3 0 bits
		// If the fraction is one bit, we have still have fraction of nbits-3, with the msb representing 2^-1, and the rest are right extended 0's
		std::bitset<fbits> _frac;
		msb = msb - nrExponentBits;
		unsigned int nrFractionBits = (msb < 0 ? 0 : msb + 1);
		if (msb >= 0) {
			for (int i = msb; i >= 0; --i) {
				_frac[fbits - 1 - (msb - i)] = tmp[i];
			}
		}
		_fraction.set(_frac, nrFractionBits);
	}
	void decode(const std::bitset<nbits>& raw_bits) {
		_raw_bits = raw_bits;	// store the raw bits for reference
		// check special cases
		_sign     = raw_bits.test(nbits - 1);
		// check for special cases
		bool special = false;
		if (_sign) {
			std::bitset<nbits> tmp(raw_bits);
			tmp.reset(nbits - 1);
			if (tmp.none()) {			
				setToInfinite();  // special case = +-inf
			}
			else {
				extract_fields(raw_bits);
			}
		}
		else {
			if (raw_bits.none()) {  // special case = 0
				setToZero();
			}
			else {
				extract_fields(raw_bits);
			}
		}
		if (_trace_decode) std::cout << "raw bits: " << _raw_bits << " posit bits: " << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;

		// we are storing both the raw bit representation and the decoded form
		// so no need to transform back via 2's complement of regime/exponent/fraction
	}
	int64_t to_int64() const {
		if (isZero()) return 0;
		if (isInfinite()) throw "inf";
		// returning the integer representation of a posit only works for [1,inf) and is approximate
		int64_t value;
		int s = scale();
		if (s < 0) {
			value = (_fraction.get().to_ullong() >> -s);
		}
		else {
			value = (_fraction.get().to_ullong() << s);
		}	
		return value;
	}
	float to_float() const {
		return (float)to_double();
	}
	double to_double() const {
		if (isZero())
			return 0.0;
		if (isInfinite())
			return INFINITY;
		return sign_value() * regime_value() * exponent_value() * (1.0 + fraction_value());
	}
	
	// Maybe remove explicit, MTL compiles, but we have lots of double computation then
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

	// currently, size is tied to fbits size of posit config. Is there a need for a case that captures a user-defined sized fraction?
	value<fbits> convert_to_scientific_notation() const {
		value<fbits> v(_sign, scale(), get_fraction().get(), isZero());
		return v;
	}
	// collect the posit components into a bitset
	std::bitset<nbits> collect() {
		std::bitset<rbits> r = _regime.get();
		unsigned int nrRegimeBits = _regime.nrBits();
		std::bitset<es> e = _exponent.get();
		unsigned int nrExponentBits = _exponent.nrBits();
		std::bitset<fbits> f = _fraction.get();
		unsigned int nrFractionBits = _fraction.nrBits();
		std::bitset<nbits> raw_bits;
		// collect
		raw_bits.set(nbits - 1, _sign);
		int msb = nbits - 2;
		for (unsigned int i = 0; i < nrRegimeBits; i++) {
			raw_bits.set(msb--, r[nbits - 2 - i]);
		}
		if (msb >= 0) {
			for (unsigned int i = 0; i < nrExponentBits; i++) {
				raw_bits.set(msb--, e[es - 1 - i]);
			}
		}
		if (msb >= 0) {
			for (unsigned int i = 0; i < nrFractionBits; i++) {
				raw_bits.set(msb--, f[fbits - 1 - i]);
			}
		}
		return raw_bits;
	}
	// given a decoded posit, take its 2's complement
	void take_2s_complement() {
		// transform back through 2's complement
		std::bitset<rbits> r = _regime.get();
		unsigned int nrRegimeBits = _regime.nrBits();
		std::bitset<es> e = _exponent.get();
		unsigned int nrExponentBits = _exponent.nrBits();
		std::bitset<fbits> f = _fraction.get();
		unsigned int nrFractionBits = _fraction.nrBits();
		std::bitset<nbits> raw_bits;
		// collect
		raw_bits.set(nbits - 1, _sign);
		int msb = nbits - 2;
		for (unsigned int i = 0; i < nrRegimeBits; i++) {
			raw_bits.set(msb--, r[nbits - 2 - i]);
		}
		if (msb >= 0) {
			for (unsigned int i = 0; i < nrExponentBits; i++) {
				raw_bits.set(msb--, e[es - 1 - i]);
			}
		}
		if (msb >= 0) {
			for (unsigned int i = 0; i < nrFractionBits; i++) {
				raw_bits.set(msb--, f[fbits - 1 - i]);
			}
		}
		// transform
		raw_bits = twos_complement(raw_bits);
		// distribute
		std::bitset<nbits - 1> regime_bits;
		for (unsigned int i = 0; i < nrRegimeBits; i++) {
			regime_bits.set(nbits - 2 - i, raw_bits[nbits - 2 - i]);
		}
		_regime.set(regime_bits, nrRegimeBits);
		if (es > 0 && nrExponentBits > 0) {
			std::bitset<es> exponent_bits;
			for (unsigned int i = 0; i < nrExponentBits; i++) {
				exponent_bits.set(es - 1 - i, raw_bits[nbits - 2 - nrRegimeBits - i]);
			}
			_exponent.set(exponent_bits, nrExponentBits);
		}
		if (nrFractionBits > 0) {
			std::bitset<fbits> fraction_bits;   // was nbits - 2
			for (unsigned int i = 0; i < nrFractionBits; i++) {
				// fraction_bits.set(nbits - 3 - i, raw_bits[nbits - 2 - nrRegimeBits - nrExponentBits - i]);
				fraction_bits.set(fbits - 1 - i, raw_bits[nbits - 2 - nrRegimeBits - nrExponentBits - i]);
			}
			_fraction.set(fraction_bits, nrFractionBits);
		}
	}
	// scale returns the shifts to normalize the number =  regime + exponent shifts
	int scale() const {
		// how many shifts represent the regime?
		// regime = useed ^ k = 2 ^ (k*(2 ^ e))
		// scale = useed ^ k * 2^e 
		return _regime.scale() + _exponent.scale();
	}
	unsigned int exp() const {
		return _exponent.scale();
	}
	// special case check for projecting values between (0, minpos] to minpos and [maxpos, inf) to maxpos
	// Returns true if the scale is too small or too large for this posit config
	// DO NOT USE the k value for this, as the k value encodes the useed regions and thus is too coarse to make this decision
	// Using the scale directly is the simplest expression of the inward projection test.
	bool check_inward_projection_range(int scale) {
		// calculate the max k factor for this posit config
		int posit_size = nbits;
		int k = scale < 0 ?	-(posit_size - 2) : (posit_size - 2);
		return scale < 0 ? scale < k*(1<<es) : scale > k*(1<<es);
	}
	// project to the next 'larger' posit: this is 'pushing away' from zero, projecting to the next bigger scale
	void project_up() {
		bool carry = _fraction.increment();
		if (carry && es > 0)
			carry = _exponent.increment();
		if (carry) 
                    _regime.increment();
	}
	// step up to the next posit in a lexicographical order
	void increment_posit() {
		std::bitset<nbits> raw(_raw_bits);
		increment_bitset(raw);
		decode(raw);
	}
	// step down to the previous posit in a lexicographical order
	void decrement_posit() {
		std::bitset<nbits> raw(_raw_bits);
		decrement_bitset(raw);
		decode(raw);
	}
	
	// Generalized version
	template <size_t FBits>
	void convert_to_posit(const value<FBits>& v) {
        //convert(v.sign(), v.scale(), v.fraction(), FBits);
		convert(v.sign(), v.scale(), v.fraction());
    }

#if 0   // DEPRECATED
	// this routine will not allocate 0 or infinity due to the test on (0,minpos], and [maxpos,inf)
	// TODO: is that the right functionality? right now the special cases are deal with in the
	// assignment operators for integer/float/double. I don't like that distribution of knowledge.
	void convert_to_posit(value<fbits>& v) {
		convert_to_posit(v.sign(), v.scale(), v.fraction());
	}
	void convert_to_posit(bool _negative, int _scale, std::bitset<fbits> _frac) {
		setToZero();
		if (_trace_conversion) std::cout << "sign " << (_negative ? "-1 " : " 1 ") << "scale " << _scale << " fraction " << _frac << std::endl;

		// construct the posit
		_sign = _negative;	
		unsigned int nr_of_regime_bits = _regime.assign_regime_pattern(_scale >> es);
		bool geometric_round = _exponent.assign_exponent_bits(_scale, nr_of_regime_bits);
		unsigned int nr_of_exp_bits    = _exponent.nrBits();
		unsigned int remaining_bits    = nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0;
		bool round_up = _fraction.assign_fraction(remaining_bits, _frac);
		if (round_up) 
                    project_up();
		// store raw bit representation
		_raw_bits = _sign ? twos_complement(collect()) : collect();
		_raw_bits.set(nbits - 1, _sign);
		if (_trace_conversion) std::cout << "raw bits: "  << _raw_bits << " posit bits: "  << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;
	}


	/** Generalized conversion function (could replace convert_to_posit). \p _frac is fraction of arbitrary size with hidden bit at \p hpos.
         *  \p hpos == \p FBits means that the hidden bit is in front of \p _frac, i.e. \p _frac is a pure fraction without hidden bit.
         *  
         * 
         */
	template <size_t FBits>
	void convert(bool _negative, int _scale, std::bitset<FBits> _frac, int hpos) {
		if (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
        setToZero();
        if (_trace_conversion) std::cout << "sign " << (_negative ? "-1 " : " 1 ") << "scale " << std::setw(3) << _scale << " fraction " << _frac << std::endl;
                
        // construct the posit
		_sign = _negative;
		int k = calculate_unconstrained_k<nbits, es>(_scale);
		// interpolation rule checks
		if (check_inward_projection_range(_scale)) {    // regime dominated
			if (_trace_conversion) std::cout << "inward projection" << std::endl;
			// we are projecting to minpos/maxpos
			_regime.assign_regime_pattern(k);
			// store raw bit representation
			_raw_bits = _sign ? twos_complement(collect()) : collect();
			_raw_bits.set(nbits - 1, _sign);
			// we are done
			if (_trace_rounding) std::cout << "projection  rounding ";
		} 
		else {
			unsigned int nr_of_regime_bits = _regime.assign_regime_pattern(k);
			bool carry = false;
			switch (_exponent.assign_exponent_bits(_scale, k, nr_of_regime_bits)) {
			case GEOMETRIC_ROUND_UP:
#ifdef INCREMENT_POSIT_CARRY_CHAIN
				carry = _exponent.increment();
				if (carry)_regime.increment();
#endif // INCREMENT_POSIT_CARRY_CHAIN
				break;
			case NO_ADDITIONAL_ROUNDING:
				break;
			case ARITHMETIC_ROUNDING:
				unsigned int nr_of_exp_bits = _exponent.nrBits();
				unsigned int remaining_bits = nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0;
				bool round_up = _fraction.assign(remaining_bits, _frac, hpos);
				if (round_up) project_up();
			}
			// store raw bit representation
			_raw_bits = _sign ? twos_complement(collect()) : collect();
			_raw_bits.set(nbits - 1, _sign);
		}

        if (_trace_conversion) std::cout << "raw bits: "  << _raw_bits << " posit bits: "  << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;            
    }

#endif	

	template<size_t input_fbits>
	void convert(bool sign, int scale, std::bitset<input_fbits> input_fraction) {
		setToZero();
		if (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
		if (_trace_conversion) std::cout << "sign " << (sign ? "-1 " : " 1 ") << "scale " << std::setw(3) << scale << " fraction " << input_fraction << std::endl;

		// construct the posit
		_sign = sign;
		int k = calculate_unconstrained_k<nbits, es>(scale);
		// interpolation rule checks
		if (check_inward_projection_range(scale)) {    // regime dominated
			if (_trace_conversion) std::cout << "inward projection" << std::endl;
			// we are projecting to minpos/maxpos
			_regime.assign_regime_pattern(k);
			// store raw bit representation
			_raw_bits = _sign ? twos_complement(collect()) : collect();
			_raw_bits.set(nbits - 1, _sign);
			// we are done
			if (_trace_rounding) std::cout << "projection  rounding ";
		}
		else {
			const size_t pt_len = nbits + 3 + es;
			std::bitset<pt_len> pt_bits;
			std::bitset<pt_len> regime;
			std::bitset<pt_len> exponent;
			std::bitset<pt_len> fraction;
			std::bitset<pt_len> sticky_bit;

			bool s = sign;
			int e = scale;
			bool r = (e >= 0);

			unsigned run = (r ? 1 + (e >> es) : -(e >> es));
			regime.set(0, 1 ^ r);
			for (unsigned i = 1; i <= run; i++) regime.set(i, r);

			unsigned esval = e % (uint32_t(1) << es);
			exponent = convert_to_bitset<pt_len>(esval);
			unsigned nf = (unsigned)std::max<int>(0, (nbits + 1) - (2 + run + es));
			// TODO: what needs to be done if nf > fbits?
			//assert(nf <= input_fbits);
			// copy the most significant nf fraction bits into fraction
			unsigned lsb = nf <= input_fbits ? 0 : nf - input_fbits;
			for (unsigned i = lsb; i < nf; i++) fraction[i] = input_fraction[input_fbits - nf + i];

			bool sb = anyAfter(input_fraction, input_fbits - 1 - nf);

			// construct the untruncated posit
			// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
			regime <<= es + nf + 1;
			exponent <<= nf + 1;
			fraction <<= 1;
			sticky_bit.set(0, sb);

			pt_bits |= regime;
			pt_bits |= exponent;
			pt_bits |= fraction;
			pt_bits |= sticky_bit;

			unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
			bool blast = pt_bits.test(len - nbits);
			bool bafter = pt_bits.test(len - nbits - 1);
			bool bsticky = anyAfter(pt_bits, len - nbits - 1 - 1);

			bool rb = (blast & bafter) | (bafter & bsticky);

			pt_bits <<= pt_len - len;
			std::bitset<nbits> ptt;
			truncate(pt_bits, ptt);

			if (rb) increment_bitset(ptt);
			if (s) ptt = twos_complement(ptt);
			decode(ptt);
		}
	}

private:
	std::bitset<nbits>     _raw_bits;	// raw bit representation
	bool				   _sign;       // decoded posit representation
	regime<nbits, es>	   _regime;		// decoded posit representation
	exponent<nbits, es>    _exponent;	// decoded posit representation
	fraction<fbits> 	   _fraction;	// decoded posit representation

	// HELPER methods
	// multiply two values
	void multiply(const value<fbits>& v1, const value<fbits>& v2, value<mbits>& result) {
		static_assert(fhbits > 0, "posit configuration does not support multiplication");
		if (_trace_mul) std::cout << "v1  " << components(v1) << std::endl << "v2  " << components(v2) << std::endl;
		bool new_sign = v1.sign() ^ v2.sign();
		int new_scale = v1.scale() + v2.scale();
		std::bitset<mbits> result_fraction;

		if (nbits > 3) {
			// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
			std::bitset<fhbits> r1 = v1.get_fixed_point();
			std::bitset<fhbits> r2 = v2.get_fixed_point();
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
		else {   // posit<3,0> is pure sign and scale

		}
		if (_trace_mul) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;
		// TODO: how do you recognize the special case of zero?
		result.set(new_sign, new_scale, result_fraction, false);
	}

    // template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees>
	friend std::ostream& operator<< (std::ostream& ostr, const posit<nnbits, ees>& p);
	template<size_t nnbits, size_t ees>
	friend std::istream& operator>> (std::istream& istr, posit<nnbits, ees>& p);

	template<size_t nnbits, size_t ees>
	friend bool operator==(const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator!=(const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator< (const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator> (const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator<=(const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator>=(const posit<nnbits, ees>& lhs, const posit<nnbits, ees>& rhs);
};

////////////////// POSIT operators
template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const posit<nbits, es>& p) {
	if (p.isZero()) {
		ostr << double(0.0);
		return ostr;
	}
	else if (p.isInfinite()) {
		ostr << std::numeric_limits<double>::infinity();
		return ostr;
	}
	ostr << p.to_double();
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator>> (std::istream& istr, const posit<nbits, es>& p) {
	istr >> p._Bits;
	return istr;
}

template<size_t nbits, size_t es>
inline bool operator==(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return lhs._raw_bits == rhs._raw_bits; }
template<size_t nbits, size_t es>
inline bool operator!=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator< (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	if (lhs.isInfinite()) {
		return false;
	}
	if (rhs.isInfinite()) {
		return true;
	}
	return lessThan(lhs._raw_bits, rhs._raw_bits); 
}
template<size_t nbits, size_t es>
bool operator> (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return operator< (rhs, lhs); }
template<size_t nbits, size_t es>
inline bool operator<=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return operator< (lhs, rhs) || operator==(lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator>=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator< (lhs, rhs); }

// POSIT BINARY ARITHMETIC OPERATORS
template<size_t nbits, size_t es>
inline posit<nbits, es> operator+(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> sum = lhs;
	sum += rhs;
	return sum;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator-(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> diff = lhs;
	diff -= rhs;
	return diff;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator*(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> mul = lhs;
	mul *= rhs;
	return mul;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator/(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

/// Magnitude of a posit (equivalent to turning the sign bit off).
template<size_t nbits, size_t es> 
posit<nbits, es> abs(const posit<nbits, es>& p) {
    return posit<nbits, es>(false, p.get_regime(), p.get_exponent(), p.get_fraction());
}


	}  // namespace unum

}  // namespace sw