#pragma once
// posit.hpp: definition of arbitrary posit number configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>
#include <iostream>

#include "../bitset/bitset_helpers.hpp"
#include "posit_regime_lookup.hpp"
#include "posit_helpers.hpp"

const uint8_t POSIT_ROUND_DOWN = 0;
const uint8_t POSIT_ROUND_TO_NEAREST = 1;
// set intermediate result reporting
const bool _trace_decode     = false;
const bool _trace_rounding   = false;
const bool _trace_conversion = false;
const bool _trace_add        = false;
const bool _trace_mult       = false;

template<size_t nbits, size_t es>
double useed() {
	return double(uint64_t(1) << (uint64_t(1) << es));
};

template<size_t nbits, size_t es>
class regime {
public:
	regime() {
		_Bits.reset();
		_k = 0;
		_RegimeBits = 0;
	}
	regime(const regime& r) {
		_Bits = r._Bits;
		_k = r._k;
		_RegimeBits = r._RegimeBits;
	}
	regime& operator= (const regime& r) {
		_Bits = r._Bits;
		_k = r._k;
		_RegimeBits = r._RegimeBits;
		return *this;
	}
	void reset() {
		_k = 0;
		_RegimeBits = 0;
		_Bits.reset();
	}
	unsigned int nrBits() const {
		return _RegimeBits;
	}
	int scale() const {
		return (int(_k) << es);
	}
	// return the k-value of the regime: useed ^ k
	int regime_k() const {
		return _k;
	}
	double value() const {
		double scale;
		int e2 = (1 << es) * _k;
		if (e2 < -63 || e2 > 63) {
			scale = pow(2.0, e2);
		}
		else {
			if (e2 >= 0) {
				scale = double((uint64_t(1) << e2));
			}
			else {
				scale = double(1.0) / double(uint64_t(1) << -e2);
			}
		}
		return scale;
	}
	bool isZero() const {
		return _Bits.none();
	}
	std::bitset<nbits - 1> get() const {
		return _Bits;
	}
	void set(const std::bitset<nbits - 1>& raw, unsigned int nrOfRegimeBits) {
		_Bits = raw;
		_RegimeBits = nrOfRegimeBits;
	}
	void setZero() {
		_Bits.reset();
		_RegimeBits = nbits - 1;
	}
	// construct the regime bit pattern given a number's scale and returning the number of regime bits
	unsigned int assign_regime_pattern(bool sign, int k) {
		_Bits.reset();
		if (k < 0) {
			_k = int8_t(-k < nbits-2 ? k : -(static_cast<int>(nbits) - 2)); // constrain regime to minpos
			k = -_k - 1;
			uint64_t regime = REGIME_BITS[k];
			uint64_t mask = REGIME_BITS[0];
			_RegimeBits = (k < nbits - 2 ? k + 2 : nbits - 1);
			for (unsigned int i = 0; i < _RegimeBits; i++) {
				_Bits[nbits - 2 - i] = !(regime & mask);
				mask >>= 1;
			}

		}
		else {
			_k = int8_t(k < nbits - 2 ? k : nbits - 2); // constrain regime to maxpos
			uint64_t regime = REGIME_BITS[k];
			uint64_t mask = REGIME_BITS[0];
			_RegimeBits = (k < nbits - 2 ? k + 2 : nbits - 1);
			for (unsigned int i = 0; i < _RegimeBits; i++) {
				_Bits[nbits - 2 - i] = regime & mask;
				mask >>= 1;
			}

		}
		return _RegimeBits;
	}
	bool increment() {
		if (_Bits.all()) return false; // rounding up/down as we are already at minpos/maxpos
		bool carry = increment_unsigned(_Bits,_RegimeBits);
		if (carry) {
			std::cout << "Regime needs to expand" << std::endl;
		}
		else {
			_k++;
		}
		return carry;
	}
private:
	std::bitset<nbits - 1>	_Bits;
	int8_t					_k;
	unsigned int			_RegimeBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees>
	friend std::ostream& operator<< (std::ostream& ostr, const regime<nnbits, ees>& r);
	template<size_t nnbits, size_t ees>
	friend std::istream& operator>> (std::istream& istr, regime<nnbits, ees>& r);

	template<size_t nnbits, size_t ees>
	friend bool operator==(const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator!=(const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator< (const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator> (const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator<=(const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator>=(const regime<nnbits, ees>& lhs, const regime<nnbits, ees>& rhs);
};

template<size_t nbits, size_t es>
class exponent {
public:
	exponent() {
		reset();
	}
	exponent(const exponent& e) {
		_Bits = e._Bits;
		_NrOfBits = e._NrOfBits;
	}
	exponent& operator=(const exponent& e) {
		_Bits = e._Bits;
		_NrOfBits = e._NrOfBits;
		return *this;
	}
	void reset() {
		_NrOfBits = 0;
		_Bits.reset();
	}
	unsigned int nrBits() const {
		return _NrOfBits;
	}
	int scale() const {
		return _Bits.to_ulong();
	}
	double value() const {
		return double(uint64_t(1) << scale());
	}
	std::bitset<es> get() const {
		return _Bits;
	}
	void set(const std::bitset<es>& raw, int nrOfExponentBits) {
		_Bits = raw;
		_NrOfBits = nrOfExponentBits;
	}
	// calculate the exponent given a number's scale and the number of regime bits, returning the number of exponent bits assigned
	unsigned int assign_exponent_bits(unsigned int msb, unsigned int nr_of_regime_bits) {
		_Bits.reset();
		_NrOfBits = (nbits - 1 - nr_of_regime_bits > es ? es : nbits - 1 - nr_of_regime_bits);
		if (_NrOfBits > 0) {
			unsigned int exponent = (es > 0 ? msb % (1 << es) : 0);
			uint64_t mask = (uint64_t(1) << es) >> 1;  // (es - 1) can be negative, causing a compilation warning
			for (unsigned int i = 0; i < _NrOfBits; i++) {
				_Bits[es - 1 - i] = exponent & mask;
				mask >>= 1;
			}
		}
		return _NrOfBits;
	}
	bool increment() {
		return increment_unsigned(_Bits, _NrOfBits);
	}
private:
	std::bitset<es> _Bits;
	unsigned int	_NrOfBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees>
	friend std::ostream& operator<< (std::ostream& ostr, const exponent<nnbits, ees>& e);
	template<size_t nnbits, size_t ees>
	friend std::istream& operator>> (std::istream& istr, exponent<nnbits, ees>& e);

	template<size_t nnbits, size_t ees>
	friend bool operator==(const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator!=(const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator< (const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator> (const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator<=(const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator>=(const exponent<nnbits, ees>& lhs, const exponent<nnbits, ees>& rhs);
};

// fraction is spec'ed with the size of the posit it belongs to.
// However, the size of the fraction segment is nbits-3, but we maintain an extra guard bit, so the size of the actual fraction we manage is nbits-2
template<size_t fbits>
class fraction {
public:
	fraction() {
		_Bits.reset();
	}
	fraction(const fraction& f) {
		_Bits = f._Bits;
		_NrOfBits = f._NrOfBits;
	}
	fraction& operator=(const fraction& f) {
		_Bits = f._Bits;
		_NrOfBits = f._NrOfBits;
		return *this;
	}
	void reset() {
		_NrOfBits = 0;
		_Bits.reset();
	}
	unsigned int nrBits() const {
		return _NrOfBits;
	}
	double value() const {
		// TODO: this fails when fbits > 64 and we cannot represent the fraction by a 64bit unsigned integer
		return double(_Bits.to_ullong()) / double(uint64_t(1) << (fbits));
	}
	std::bitset<fbits> get() const {
		return _Bits;
	}
	void set(const std::bitset<fbits>& raw, int nrOfFractionBits) {
		_Bits = raw;
		_NrOfBits = (fbits > nrOfFractionBits ? fbits : nrOfFractionBits);
	}
	// copy the remaining bits into the fraction
	bool assign_fraction(unsigned int remaining_bits, std::bitset<fbits>& _fraction) {
		bool round_up = false;
		if (remaining_bits > 0 && fbits > 0) {
			_NrOfBits = 0;
			for (unsigned int i = 0; i < remaining_bits; i++) {
				_Bits[fbits - 1 - i] = _fraction[fbits - 1 - i];
				_NrOfBits++;
			}
			round_up = _fraction[fbits - 1 - remaining_bits];
		}
		else {
			round_up = _fraction[fbits - 1];
			_NrOfBits = 0;
		}
		return round_up;
	}
	// normalize the fraction and return its fraction in the argument
	void normalize(std::bitset<fbits+1>& number) const {
		number.set(fbits, true); // set hidden bit
		for (int i = 0; i < fbits; i++) {
			number.set(i, _Bits[i]);
		}
	}
	/*   h is hidden bit
	*   h.bbbb_bbbb_bbbb_b...      fraction
	*   0.000h_bbbb_bbbb_bbbb_b... number
	*  >-.----<                    shift of 4
	*/
	void denormalize(int shift, std::bitset<fbits+1>& number) const {
		number.reset();
		if (fbits == 0) return;
		if (shift < 0) shift = -shift;
		if (shift <= static_cast<int>(fbits)) {
			number.set(static_cast<int>(fbits) - shift); // set hidden bit
			for (int i = static_cast<int>(fbits) - 1 - shift; i >= 0; i--) {
				number.set(i, _Bits[i + shift]);
			}
		}
	}
	bool increment() {
		return increment_unsigned(_Bits, _NrOfBits);
	}
private:
	// maximum size fraction is <nbits - one sign bit - minimum two regime bits>
	// but we maintain 1 guard bit for rounding decisions
	std::bitset<fbits> _Bits;
	unsigned int       _NrOfBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nfbits>
	friend std::ostream& operator<< (std::ostream& ostr, const fraction<nfbits>& f);
	template<size_t nfbits>
	friend std::istream& operator>> (std::istream& istr, fraction<nfbits>& f);

	template<size_t nfbits>
	friend bool operator==(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator!=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator< (const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator> (const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator<=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator>=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
};

/*
 class posit represents arbitrary configuration posits and their basic arithmetic operations (add/sub, mul/div)
 */
template<size_t nbits, size_t es> class posit {
public:
	posit<nbits, es>() {
		reset();
		validate();
	}
	posit<nbits, es>(int64_t initial_value) {
		*this = initial_value;
	}
	posit<nbits, es>(const posit& p) {
		*this = p;
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
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		bool _sign = (0x8000000000000000 & rhs);  // 1 is negative, 0 is positive
		if (_sign) {
			// process negative number: process 2's complement of the input
			unsigned int _scale = findMostSignificantBit(-rhs) - 1;
			uint64_t _fraction_without_hidden_bit = (-rhs << (64 - _scale));
			std::bitset<nbits-2> _fraction = copy_integer_fraction<nbits-2>(_fraction_without_hidden_bit);
			convert_to_posit(_sign, _scale, _fraction);
			take_2s_complement();
		}
		else {
			// process positive number
			if (rhs != 0) {
				unsigned int _scale = findMostSignificantBit(rhs) - 1;
				uint64_t _fraction_without_hidden_bit = (rhs << (64 - _scale));
				std::bitset<nbits-2> _fraction = copy_integer_fraction<nbits-2>(_fraction_without_hidden_bit);
				convert_to_posit(_sign, _scale, _fraction);
			}
		}
		return *this;
	}
	posit<nbits, es>& operator=(uint64_t rhs) {
		reset();
		if (rhs != 0) {
			unsigned int _scale = findMostSignificantBit(rhs) - 1;
			uint64_t _fraction_without_hidden_bit = (rhs << (64 - _scale));
			std::bitset<nbits-2> _fraction = copy_integer_fraction<nbits-2>(_fraction_without_hidden_bit);
			convert_to_posit(false, _scale, _fraction);
		}
		decode();
		return *this;
	}
	posit<nbits, es>& operator=(const float rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_sign = false;
			_regime.setZero();
			break;
		case FP_INFINITE:
			_sign = true;
			_regime.setZero();
			_raw_bits.set(nbits - 1, true);
			break;
		case FP_NAN:
			std::cerr << "float is NAN: returning 0" << std::endl;
			break;
		case FP_SUBNORMAL:
			std::cerr << "TODO: subnormal number: returning 0" << std::endl;
			break;
		case FP_NORMAL:
			{
				if (rhs == 0.0) break;  // 0 is a special case
				bool _negative = extract_sign(rhs);
				int _scale = extract_exponent(rhs) - 1;
				uint32_t _23b_fraction_without_hidden_bit = extract_fraction(rhs);
				std::bitset<nbits-2> _fraction = extract_float_fraction<nbits-2>(_23b_fraction_without_hidden_bit);
				if (_trace_conversion) std::cout << "float " << rhs << " sign " << _negative << " scale " << _scale << " 23b fraction 0x" << std::hex << _23b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
				convert_to_posit(_negative, _scale, _fraction);
			}
			break;
		}
		return *this;
	}
	posit<nbits, es>& operator=(const double rhs) {
		reset();
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_sign = false;
			_regime.setZero();
			break;
		case FP_INFINITE:
			_sign = true;
			_regime.setZero();
			_raw_bits.set(nbits - 1, true);
			break;
		case FP_NAN:
			std::cerr << "float is NAN" << std::endl;
			break;
		case FP_SUBNORMAL:
			std::cerr << "TODO: subnormal number" << std::endl;
			break;
		case FP_NORMAL:
			{
				if (rhs == 0.0) break;  // 0 is a special case
				bool _negative = extract_sign(rhs);
				int _scale = extract_exponent(rhs) - 1;
				uint64_t _52b_fraction_without_hidden_bit = extract_fraction(rhs);
				std::bitset<nbits-2> _fraction = extract_double_fraction<nbits-2>(_52b_fraction_without_hidden_bit);
				if (_trace_conversion) std::cout << "double " << rhs << "sign " << _negative << " scale " << _scale << " 52b fraction 0x" << std::hex << _52b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
				convert_to_posit(_negative, _scale, _fraction);
			}
			break;
		}
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
		}
		else if (rhs.isZero()) {
			return *this;
		}
		if (isInfinite()) {
			return *this;
		}
		else if (rhs.isInfinite()) {
			*this = rhs;
			return *this;
		}

		// align the fractions, and produce right extended fractions in r1 and r2 with hidden bits explicit
		std::bitset<nbits-1> r1, r2, sum; // fraction is at most nbits-3 bits, but we need to incorporate one sticky bit for rounding decisions, and a leading slot for the hidden bit
		std::bitset<nbits - 2> result_fraction; // fraction part of the sum
		
		// with sign/magnitude adders it is customary to organize the computation 
		// along the four quadrants of sign combinations
		//  + + = +
		//  + - =   lhs > rhs ? + : -
		//  - + =   lhs > rhs ? - : +
		//  - - = -
		// to simplify the result processing
		bool r1_sign, r2_sign;	

		// we need to order the operands in terms of scale, 
		// with the largest scale taking the r1 slot
		// and the smaller operand aligned to the larger in r2.
		int lhs_scale = scale();
		int rhs_scale = rhs.scale();
		int scale_of_result;
		// we need to determine the biggest operand
		bool rhs_bigger = (abs(to_double()) < abs(rhs.to_double()));		//    TODO: need to do this in native posit integer arithmetic
		int diff = lhs_scale - rhs_scale;
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

		if (_trace_add) {
			std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1  " << r1 << " diff " << diff << std::endl;
			std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2  " << r2 << std::endl;
		}
		
		if (r1_sign != r2_sign) r2 = twos_complement(r2);
		bool carry = add_unsigned<nbits-1>(r1, r2, sum);

		if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum " << sum << std::endl;
		if (carry) {
			if (r1_sign == r2_sign) {
				// the carry implies that we have a bigger number than r1
				scale_of_result++;   
			}
			else {
				// the carry implies that we have a smaller number than r1
				unsigned int msb = nbits;
				for (int i = nbits - 2; i >= 0; i--) {
					if (sum.test(i)) {
						msb = i;
						break;
					}
				}
				if (msb == nbits) {
					// we have actual 0
					reset();
					return *this;
				}
				else {
					// and extract the fraction, leaving the hidden bit behind
					for (int i = 0; i < nbits - 2; i++) {
						result_fraction[i] = sum[i];
					}
				}				
			}
		}
		else {
			// no carry implies that the scale remains the same
			// and that the first fraction bits came after the nbits-3 slot
			for (int i = 0; i < nbits - 2; i++) {
				result_fraction[i] = sum[i];
			}
		}
		
		if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum " << sum << " fraction " << result_fraction << std::endl;
		convert_to_posit(r1_sign, scale_of_result, result_fraction);
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
		*this = *this + posit<nbits, es>(1);
		return *this;
	}
	posit<nbits, es> operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	posit<nbits, es>& operator--() {
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
	fraction<nbits-2> get_fraction() const {
		return _fraction;
	}
	std::bitset<nbits> get() const {
		return _raw_bits;
	}
	std::bitset<nbits> get_decoded() const {
		std::bitset<nbits-1> r = _regime.get();
		unsigned int nrRegimeBits = _regime.nrBits();
		std::bitset<es> e = _exponent.get();
		unsigned int nrExponentBits = _exponent.nrBits();
		std::bitset<nbits-2> f = _fraction.get();
		unsigned int nrFractionBits = _fraction.nrBits();

		std::bitset<nbits> _Bits;
		_Bits.set(nbits - 1, _sign);
		int msb = nbits - 2;
		for (unsigned int i = 0; i < nrRegimeBits; i++) {
			_Bits.set(msb--, r[nbits - 2 - i]);
		}
		if (msb < 0) return _Bits;
		for (unsigned int i = 0; i < nrExponentBits; i++) {
			_Bits.set(msb--, e[es - 1 - i]);
		}
		if (msb < 0) return _Bits;
		for (unsigned int i = 0; i < nrFractionBits; i++) {
			_Bits.set(msb--, f[nbits - 3 - i]);
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
				_regime.setZero();
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
			std::bitset<nbits-2> _frac;
			msb = msb - nrExponentBits;
			unsigned int nrFractionBits = (msb < 0 ? 0 : msb + 1);
			if (msb >= 0) {
				for (int i = msb; i >= 0; --i) {
					_frac[nbits - 3 - (msb - i)] = tmp[i];
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
		std::bitset<nbits - 1> r = _regime.get();
		unsigned int nrRegimeBits = _regime.nrBits();
		std::bitset<es> e = _exponent.get();
		unsigned int nrExponentBits = _exponent.nrBits();
		std::bitset<nbits-2> f = _fraction.get();
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
				raw_bits.set(msb--, f[nbits - 3 - i]);
			}
		}
		return raw_bits;
	}
	// given a decoded posit, take its 2's complement
	void take_2s_complement() {
		// transform back through 2's complement
		std::bitset<nbits - 1> r = _regime.get();
		unsigned int nrRegimeBits = _regime.nrBits();
		std::bitset<es> e = _exponent.get();
		unsigned int nrExponentBits = _exponent.nrBits();
		std::bitset<nbits-2> f = _fraction.get();
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
				raw_bits.set(msb--, f[nbits - 3 - i]);
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
			std::bitset<nbits-2> fraction_bits;
			for (unsigned int i = 0; i < nrFractionBits; i++) {
				fraction_bits.set(nbits - 3 - i, raw_bits[nbits - 2 - nrRegimeBits - nrExponentBits - i]);
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

	// this routine will not allocate 0 or infinity due to the test on (0,minpos], and [maxpos,inf)
	// TODO: is that the right functionality? right now the special cases are deal with in the
	// assignment operators for integer/float/double. I don't like that distribution of knowledge.
	void convert_to_posit(bool _negative, int _scale, std::bitset<nbits-2>& _frac) {
		reset();
		if (_trace_conversion) std::cout << "sign " << (_negative ? "-1 " : " 1 ") << "scale " << _scale << " fraction " << _frac << std::endl;

		// construct the posit
		_sign = _negative;	
		unsigned int nr_of_regime_bits = _regime.assign_regime_pattern(_sign, (_scale >> es));
		unsigned int nr_of_exp_bits    = _exponent.assign_exponent_bits(_scale, nr_of_regime_bits);
		unsigned int remaining_bits    = (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
		bool round_up = _fraction.assign_fraction(remaining_bits, _frac);
		if (round_up) {
			bool carry = _fraction.increment();
			if (carry && es > 0) {
				carry = _exponent.increment();
			}
			if (carry) _regime.increment();
		}
		// store raw bit representation
		_raw_bits = (_sign ? twos_complement(collect()) : collect());
		_raw_bits.set(nbits - 1, _sign);
		if (_trace_conversion) std::cout << "raw bits: "  << _raw_bits << " posit bits: "  << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;
	}

private:
	std::bitset<nbits>     _raw_bits;	// raw bit representation
	bool				   _sign;       // decoded posit representation
	regime<nbits, es>	   _regime;		// decoded posit representation
	exponent<nbits, es>    _exponent;	// decoded posit representation
	fraction<nbits-2>	   _fraction;	// decoded posit representation

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
