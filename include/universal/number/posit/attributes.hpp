#pragma once
// attributes.hpp: functions to query number system attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// functions to provide details about properties of a posit configuration

using namespace sw::universal::internal;

// forward references
//template<unsigned nbits, unsigned es> class posit;
//template<unsigned nbits> int decode_regime(const bitblock<nbits>&);

template<unsigned es>
constexpr std::uint64_t useed() {
	return 1ull << (1ul << es);
}

// calculate exponential scale of useed
template<unsigned es>
constexpr int useed_scale() {
	return (1 << es);
}

// calculate exponential scale of maxpos
template<unsigned nbits, unsigned es>
constexpr int maxpos_scale() {
	return (nbits - 2) * (1 << es);
}

// calculate exponential scale of minpos
template<unsigned nbits, unsigned es>
constexpr int minpos_scale() {
	return static_cast<int>(2 - int(nbits)) * (1 << es);
}

// calculate the constrained k value
template<unsigned nbits, unsigned es>
constexpr int calculate_k(int scale) {
	// constrain the scale to range [minpos, maxpos]
	if (scale < 0) {
		scale = scale > minpos_scale<nbits, es>() ? scale : minpos_scale<nbits, es>();
	}
	else {
		scale = scale < maxpos_scale<nbits, es>() ? scale : maxpos_scale<nbits, es>();
	}
	// bad int k = scale < 0 ? -(-scale >> es) - 1 : (scale >> es);
	// the scale of a posit is  2 ^ scale = useed ^ k * 2 ^ exp
	// -> (scale >> es) = (k*2^es + exp) >> es
	// -> (scale >> es) = k + (exp >> es) -> k = (scale >> es)
	int k = scale < 0 ? -(-scale >> es) : (scale >> es);
	if (k == 0 && scale < 0) {
		// project back to south-east quadrant
		k = -1;
	}
	return k;
}

// calculate the unconstrained k value
template<unsigned nbits, unsigned es>
constexpr int calculate_unconstrained_k(int scale) {
	// the scale of a posit is  2 ^ scale = useed ^ k * 2 ^ exp
	// -> (scale >> es) = (k*2^es + exp) >> es
	// -> (scale >> es) = k + (exp >> es) 
	// -> k = (scale >> es)
	int k = scale < 0 ? -(-scale >> es) : (scale >> es);
	if (k == 0 && scale < 0) {
		// project back to south-east quadrant
		k = -1;
	}
	return k;
}

// double value representation of the useed value of a posit<nbits, es>
template<unsigned nbits, unsigned es>
constexpr double useed() {
	return std::pow(2.0, std::pow(2.0, es));
}

// calculate the value of useed
template<unsigned nbits, unsigned es>
constexpr double useed_value() {
	return double(uint64_t(1) << useed_scale<nbits, es>());
}

// generate the minpos bit pattern for the sign requested (true is negative half, false is positive half)
template<unsigned nbits, unsigned es>
constexpr bitblock<nbits> minpos_pattern(bool sign = false) {
	bitblock<nbits> _bits;
	_bits.reset();
	_bits.set(0, true);
	return (sign ? twos_complement(_bits) : _bits);
}

// generate the maxpos bit pattern for the sign requested (true is negative half, false is positive half)
template<unsigned nbits, unsigned es>
constexpr bitblock<nbits> maxpos_pattern(bool sign = false) {
	bitblock<nbits> _bits;
	_bits.reset();
	_bits.flip();
	_bits.set(nbits - 1, false);
	return (sign ? twos_complement(_bits) : _bits);
}

// maximum value of regime 0
template<unsigned nbits, unsigned es>
posit<nbits, es> maxprecision_max() {
	posit<nbits, es> a;
	// set regime 0
	a.clear();
	a.setbit(nbits - 2, true);
	// set all exponent and fraction bits to 1
	for (unsigned i = 0; i < nbits - 1 - 2; ++i) {
		a.setbit(i, true);
	}
	return a;
}

// minimum value of regime -1
template<unsigned nbits, unsigned es>
posit<nbits, es> maxprecision_min() {
	posit<nbits, es> a;
	// set regime -1
	a.clear();
	a.setbit(nbits - 3, true);
	// set all exponent and fraction bits are already set to 0 with the reset()
	return a;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////               field value extractions               ///////////////
///////////////////////////////////////////////////////////////////////////////////

template<unsigned nbits, unsigned es>
constexpr inline int sign_value(const posit<nbits, es>& p) {
	bitblock<nbits> _bits = p.get();
	return (_bits[nbits - 1] ? -1 : 1);
}

template<unsigned nbits, unsigned es>
inline long double regime_value(const posit<nbits, es>& p) {
	positRegime<nbits, es>    _regime;
	bitblock<nbits> tmp(p.get());
	tmp = sign(p) ? twos_complement(tmp) : tmp;
	_regime.assign_regime_pattern(decode_regime(tmp));
	return _regime.value();
}

template<unsigned nbits, unsigned es>
inline long double exponent_value(const posit<nbits, es>& p) {
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	bitblock<nbits> tmp(p.get());
	tmp = sign(p) ? twos_complement(tmp) : tmp;
	unsigned nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp)); // get the regime bits
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);			 // get the exponent bits
	return _exponent.value();
}

template<unsigned nbits, unsigned es>
inline long double fraction_value(const posit<nbits, es>& p) {
	constexpr unsigned fbits = nbits - 3 - es;
	bool		     	 _sign;
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	positFraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);
	return _fraction.value();
}

// get the sign of the posit
template<unsigned nbits, unsigned es>
constexpr inline bool sign(const posit<nbits, es>& p) {
	return p.isneg();
}

// calculate the scale of a posit
template<unsigned nbits, unsigned es>
inline int scale(const posit<nbits, es>& p) {
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	internal::bitblock<nbits> tmp(p.get());
	tmp = sign(p) ? internal::twos_complement(tmp) : tmp;
	int k = decode_regime(tmp);
	unsigned nrRegimeBits = _regime.assign_regime_pattern(k);	// get the regime bits
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);							// get the exponent bits
	// return the scale
	return _regime.scale() + _exponent.scale();
}

// get the significant of a posit
template<unsigned nbits, unsigned es, unsigned fbits>
inline bitblock<fbits + 1> extract_significant(const posit<nbits, es>& p) {
	//constexpr unsigned fbits = nbits - 3 - es;
	bool		     	 _sign;
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	positFraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);
	return _fraction.get_fixed_point();
}

// get the fraction bits of a posit
template<unsigned nbits, unsigned es, unsigned fbits>
inline bitblock<fbits> extract_fraction(const posit<nbits, es>& p) {
	//constexpr unsigned fbits = nbits - 3 - es;
	bool		     	 _sign;
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	positFraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);
	return _fraction.get();
}

// calculate the scale of the regime component of the posit
template<unsigned nbits, unsigned es>
inline int regime_scale(const posit<nbits, es>& p) {
	positRegime<nbits, es>    _regime;
	bitblock<nbits> tmp(p.get());
	tmp = sign(p) ? twos_complement(tmp) : tmp;
	_regime.assign_regime_pattern(decode_regime(tmp));
	return _regime.scale();
}

// calculate the scale of the exponent component of the posit
template<unsigned nbits, unsigned es>
inline int exponent_scale(const posit<nbits, es>& p) {
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	bitblock<nbits> tmp(p.get());
	tmp = sign(p) ? twos_complement(tmp) : tmp;
	unsigned nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);
	return _exponent.scale();
}

// obtain the decoded posit bits
template<unsigned nbits, unsigned es>
inline bitblock<nbits> decoded(const posit<nbits, es>& p) {
	constexpr unsigned rbits = nbits - 1;
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	bool		     	 _sign;
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	positFraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);

	bitblock<rbits> r		= _regime.get();
	unsigned nrRegimeBits	= _regime.nrBits();
	bitblock<es>	e		= _exponent.get();
	unsigned nrExponentBits	= _exponent.nrBits();
	bitblock<fbits> f		= _fraction.get();
	unsigned nrFractionBits	= _fraction.nrBits();

	bitblock<nbits> _Bits;
	_Bits.set(nbits - 1, _sign);
	int msb = nbits - 2u;
	for (unsigned i = 0; i < nrRegimeBits; i++) {
		_Bits.set(msb--, r[nbits - 2 - i]);
	}
	if (msb < 0) 
				return _Bits;
	for (unsigned i = 0; i < nrExponentBits && msb >= 0; i++) {
		_Bits.set(msb--, e[es - 1 - i]);
	}
	if (msb < 0) return _Bits;
	for (unsigned i = 0; i < nrFractionBits && msb >= 0; i++) {
		_Bits.set(msb--, f[fbits - 1 - i]);
	}
	return _Bits;
}

}} // namespace sw::universal
