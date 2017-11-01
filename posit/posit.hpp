#pragma once
// posit.hpp: definition of arbitrary posit number configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>
#include <cassert>
#include <iostream>

#include "../bitset/bitset_helpers.hpp"
#include "../bitset/bitset_arithmetic.hpp"
#include "posit_regime_lookup.hpp"
#include "posit_helpers.hpp"
#include "trace_constants.hpp"
#include "value.hpp"
#include "fraction.hpp"
#include "exponent.hpp"
#include "regime.hpp"

const uint8_t POSIT_ROUND_TO_NEAREST = 1;


// double value representation of the useed value of a posit<nbits, es>
template<size_t nbits, size_t es>
double useed() 
{
	return double(uint64_t(1) << (uint64_t(1) << es));
}

/*
 class posit represents arbitrary configuration posits and their basic arithmetic operations (add/sub, mul/div)
 */
template<size_t nbits, size_t es> 
class posit {
public:
	static constexpr size_t rbits = nbits - 1;
	static constexpr size_t ebits = es;
	static constexpr size_t mnbits = 3 + es;                   // Min # of non-fraction bits: 1sign, 2+regime, es
// 	static constexpr size_t fbits = nbits - 3;
	static constexpr size_t fbits = mnbits > nbits ? 0 : nbits - mnbits; // avoid negative 

	posit<nbits, es>() {
		reset();
		validate();
	}
	posit<nbits, es>(int64_t initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(uint64_t initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(float initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(double initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(const posit& rhs) {
		*this = rhs;
	}
	posit<nbits, es>& operator=(int8_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(int16_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(int32_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	posit<nbits, es>& operator=(int64_t rhs) {
		reset();
		value<fbits> v(rhs);
		if (v.isZero()) {
			_sign = false;
			_regime.setZero();
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
		reset();
		value<fbits> v(rhs);
		if (v.isZero()) {
			_sign = false;
			_regime.setZero();
			return *this;
		}
		convert_to_posit(v);
		return *this;
	}
	posit<nbits, es>& operator=(float rhs) {
		reset();
		value<fbits> v(rhs);
		if (v.isZero()) {
			_sign = false;
			_regime.setZero();
			return *this;
		}
		if (v.isInfinite()) {
			_sign = true;
			_regime.setInfinite();
			_raw_bits.set(nbits - 1, true);
			return *this;
		}
		convert_to_posit(v);

		return *this;
	}
	posit<nbits, es>& operator=(double rhs) {
		reset();
		value<fbits> v(rhs);
		if (v.isZero()) {
			_sign = false;
			_regime.setZero();
			return *this;
		}
		if (v.isInfinite()) {
			_sign = true;
			_regime.setInfinite();
			_raw_bits.set(nbits - 1, true);
			return *this;
		}
		convert_to_posit(v);

		return *this;
	}
	posit<nbits, es>& operator=(const posit& rhs) {
		_raw_bits = rhs._raw_bits;
		_sign     = rhs._sign;
		_regime   = rhs._regime;
		_exponent = rhs._exponent;
		_fraction = rhs._fraction;
		return *this;
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
	posit<nbits, es>& operator+=(const posit& rhs) {
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

		constexpr size_t adder_size = fbits + 3;  // add a stick bit and two guard bits
		constexpr size_t result_size = adder_size + 1;
		// align the fractions, and produce right extended fractions in r1 and r2 with hidden bits explicit
		std::bitset<adder_size> r1, r2; // fraction is at most nbits-3 bits, but we need to incorporate one sticky bit and two guard bits for rounding decisions, and a leading slot for the hidden bit
		std::bitset<result_size> sum, result_fraction; // fraction part of the sum
		
		// with sign/magnitude adders it is customary to organize the computation 
		// along the four quadrants of sign combinations
		//  + + = +
		//  + - =   lhs > rhs ? + : -
		//  - + =   lhs > rhs ? - : +
		//  - - = -
		// to simplify the result processing
		bool r1_sign, r2_sign;	

		int lhs_scale = scale(), rhs_scale = rhs.scale(), scale_of_result= std::max(lhs_scale, rhs_scale);
		// we need to determine the biggest operand

		// Wouldn't it suffice to compare the scales?
		bool rhs_bigger = std::abs(to_double()) < std::abs(rhs.to_double());		//    TODO: need to do this in native posit integer arithmetic
		int diff = lhs_scale - rhs_scale;                         // To be removed
		
		// we need to order the operands in terms of scale, 
		// with the largest scale taking the r1 slot
		// and the smaller operand aligned to the larger in r2.
#if 0
		if (rhs_bigger) {
			rhs._fraction.normalize(r1);	  // <-- rhs is bigger operand
			_fraction.denormalize(diff, r2);  // denormalize the smaller operand
			scale_of_result = rhs_scale;
			r1_sign = rhs._sign;
			r2_sign = _sign;
		}
		else {
			_fraction.normalize(r1);			  // <-- lhs bigger operand
			rhs._fraction.denormalize(diff, r2);  // denormalize the smaller operand
			scale_of_result = lhs_scale;
			r1_sign = _sign;
			r2_sign = rhs._sign;
		}
#else
                r1 = _fraction.template nshift<adder_size>(lhs_scale - scale_of_result + 2);
                r2 = rhs._fraction.template nshift<adder_size>(rhs_scale - scale_of_result + 2);
                r1_sign = _sign;
                r2_sign = rhs._sign;
                
                if (rhs_bigger) {
                    std::swap(r1, r2);
                    std::swap(r1_sign, r2_sign);
                } 
#endif

		if (_trace_add) {
			std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1  " << r1 << " diff " << diff << std::endl;
			std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2  " << r2 << std::endl;
		}
		
		if (r1_sign != r2_sign) r2 = twos_complement(r2);
		bool carry = add_unsigned<adder_size>(r1, r2, sum);

		if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum " << sum << std::endl;
                
                // std::bitset<fbits> rounded_fraction;
                int shift = 0;
                if (carry) {
                    if (r1_sign == r2_sign)   // the carry implies that we have a bigger number than r1
                        shift = -1;
                    else 
                        // the carry implies that we added a complement and have a smaller number than r1                        
                        // find the hidden bit (in the complement)
                        for (int i = adder_size - 1; i >= 0 && !sum[i]; i--)
                            shift++;
                }
                
                if (shift >= adder_size) { // we have actual 0                            
                    reset();
                    return *this;
                }
                
                scale_of_result -= shift;
                std::bitset<fbits> rounded_fraction;
                
                assert(shift >= -1);
                // With larger value just round without shift
                if (shift == -1) {
                    result_fraction[result_size-1] = false; // carry is new hidden bit
                    rounded_fraction = round<fbits>(result_fraction, 3);
                } else {                
                    result_fraction = sum << shift;
                    result_fraction[result_size-1] = false;     // get rid of a possible complement bit
                    result_fraction[result_size-2] = false;     // turn off hidden bit
                    rounded_fraction = round<fbits>(result_fraction, 2);
                } 
#if 0      
		if (carry) {
			if (r1_sign == r2_sign) {
				// the carry implies that we have a bigger number than r1
				scale_of_result++; 
				// and that the first fraction bits came after a hidden bit at the carry position in the adder result register
				result_fraction = sum >> 1;                             // no rounding yet
// 				for (int i = 0; i < result_size; i++) {
// 					result_fraction[i] = sum[i+1];
// 				}
			}
			else {
				// the carry implies that we have a smaller number than r1
				// find the hidden bit 
				int shift = 0; // shift in addition to removal of hidden bit     
				for (int i = adder_size - 1; i >= 0 && !sum.test(i); i--) // until hidden bit at i
                                    shift++;
				if (shift < adder_size) {
					// adjust the scale
					scale_of_result -= shift;
					// and extract the fraction, leaving the hidden bit behind
					for (int i = result_size - 1; i >= shift ; i--) {
						result_fraction[i] = sum[i - shift];  // fract_size is already 1 smaller than adder_size so we get the implied hidden bit removal automatically
					}
				}
				else {
					// we have actual 0
					reset();
					return *this;
				}				
			}
		}
		else {
			// no carry implies that the scale remains the same
			// and that the first fraction bits came after a hidden bit at nbits-2 position in the adder result register
			for (int i = 0; i < nbits - 2; i++) {
				result_fraction[i] = sum[i];
			}
		}
		std::bitset<fbits> truncated_fraction;
		truncate<result_size, fbits>(result_fraction, truncated_fraction);	
		
		
		if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum " << sum << " fraction " << result_fraction << "truncated " << truncated_fraction << std::endl;

		convert_to_posit(r1_sign, scale_of_result, truncated_fraction);
#endif		
		convert_to_posit(r1_sign, scale_of_result, rounded_fraction);
		return *this;
	}
	posit<nbits, es>& operator-=(const posit& rhs) {
		if (isInfinite() && rhs.isInfinite()) {
			reset();  // IN FP this operation is a NAN, but will return a 0
		}
		else {
		*this += -rhs;
		}
		return *this;
	}
	posit<nbits, es>& operator*=(const posit& rhs) {
		return *this;
	}
	posit<nbits, es>& operator/=(const posit& rhs) {
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

	// SELECTORS
	bool isInfinite() const {
		return (_sign & _regime.isZero());
	}
	bool isZero() const {
		return (!_sign & _regime.isZero());
	}
	bool isNegative() const {
		return _sign;
	}
	bool isPositive() const {
		return !_sign;
	}
	double useed_value() const {
		return double(uint64_t(1) << useed_scale());
	}
	double maxpos_value() {
		return pow(double(useed_value()), double(nbits-2));
	}
	double minpos_value() {
		return pow(double(useed_value()), double(static_cast<int>(2-nbits)));
	}
	int useed_scale() const {
		return (uint32_t(1) << es);
	}
	int maxpos_scale() {
		return (nbits - 2) * (1 << es);
	}
	int minpos_scale() {
		return static_cast<int>(2 - nbits) * (1 << es);
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
	void validate() {
		if (nbits < es + 3) {
			throw "Requested es is too large for nbits";
		}
	}

	// MODIFIERS
	void reset() {
		_raw_bits.reset();
		_sign = false;
		_regime.reset();
		_exponent.reset();
		_fraction.reset();
	}
	posit<nbits, es>&  set(const std::bitset<nbits>& raw_bits) {
		decode(raw_bits);
		return *this;
	}
	// Set the raw bits of the posit given a binary pattern
	posit<nbits,es>& set_raw_bits(uint64_t value) {
		reset();
		std::bitset<nbits> raw_bits;
		unsigned long mask = 1;
		for ( int i = 0; i < nbits; i++ ) {
			raw_bits.set(i,(value & mask));
			mask = mask << 1;
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
	void decode(const std::bitset<nbits>& raw_bits) {
		_raw_bits = raw_bits;	// store the raw bits for reference
		// check special cases
		_sign     = raw_bits.test(nbits - 1);
		// check for special cases
		bool special = false;
		std::bitset<nbits> tmp(raw_bits);
		if (_sign) {
			tmp.reset(nbits - 1);
			if (tmp.none()) {
				// special case = +-inf
				_sign = true;
				_regime.setInfinite();
				_exponent.reset();
				_fraction.reset();
				special = true;
			}
			tmp.set(nbits - 1);
		}
		else {
			// special case = 0
			if (tmp.none()) {  // special case = 0
				// that is reset state
				_sign = false;
				_regime.setZero();
				_exponent.reset();
				_fraction.reset();
				special = true;
			}
		}
		if (!special) {
			if (_sign) tmp = twos_complement(tmp);
			unsigned int nrRegimeBits = _regime.assign_regime_pattern(_sign, decode_regime(tmp));

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
	double to_double() const {
		if (isZero())
			return 0.0;
		if (isInfinite())
			return INFINITY;
		return sign_value() * regime_value() * exponent_value() * (1.0 + fraction_value());
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
	// project to the next 'larger' posit: this is 'pushing away' from zero, projecting to the next bigger scale
	void project_up() {
		bool carry = _fraction.increment();
		if (carry && es > 0) {
			carry = _exponent.increment();
		}
		if (carry) _regime.increment();
		// store raw bit representation
		_raw_bits = (_sign ? twos_complement(collect()) : collect());
		_raw_bits.set(nbits - 1, _sign);
	}
	// step up to the next posit in a lexicographical order
	void increment_posit() {
		std::bitset<nbits> raw(_raw_bits);
		increment_twos_complement(raw);
		decode(raw);
	}
	// step down to the previous posit in a lexicographical order
	void decrement_posit() {
		std::bitset<nbits> raw(_raw_bits);
		decrement_twos_complement(raw);
		decode(raw);
	}
	// this routine will not allocate 0 or infinity due to the test on (0,minpos], and [maxpos,inf)
	// TODO: is that the right functionality? right now the special cases are deal with in the
	// assignment operators for integer/float/double. I don't like that distribution of knowledge.
	void convert_to_posit(value<fbits>& v) {
		convert_to_posit(v.sign(), v.scale(), v.fraction());
	}	
	void convert_to_posit(bool _negative, int _scale, std::bitset<fbits> _frac) {
		reset();
		if (_trace_conversion) std::cout << "sign " << (_negative ? "-1 " : " 1 ") << "scale " << _scale << " fraction " << _frac << std::endl;

		// construct the posit
		_sign = _negative;	
		unsigned int nr_of_regime_bits = _regime.assign_regime_pattern(_sign, (_scale >> es));
		unsigned int nr_of_exp_bits    = _exponent.assign_exponent_bits(_scale, nr_of_regime_bits);
		unsigned int remaining_bits    = (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
		bool round_up = _fraction.assign_fraction(remaining_bits, _frac);
		if (round_up) project_up();
		if (_trace_conversion) std::cout << "raw bits: "  << _raw_bits << " posit bits: "  << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;
	}

private:
	std::bitset<nbits>     _raw_bits;	// raw bit representation
	bool				   _sign;       // decoded posit representation
	regime<nbits, es>	   _regime;		// decoded posit representation
	exponent<nbits, es>    _exponent;	// decoded posit representation
	fraction<fbits> 	   _fraction;	// decoded posit representation

	// HELPER methods
	void align_numbers(int lhs_scale, const std::bitset<nbits>& lhs, int rhs_scale, const std::bitset<nbits>& rhs, int& scale, std::bitset<nbits>& r1, std::bitset<nbits>& r2) {

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
