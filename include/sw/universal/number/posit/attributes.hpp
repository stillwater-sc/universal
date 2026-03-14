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


template<unsigned es>
constexpr std::uint64_t useed() {
	return 1ull << (1u << es);
}

// useed_scale<es>() is defined in posit_scale_helpers.hpp

// double value representation of the useed value of a posit<nbits, es>
template<unsigned nbits, unsigned es>
constexpr double useed() {
	return std::pow(2.0, std::pow(2.0, es));
}

// calculate the value of useed
template<unsigned nbits, unsigned es>
constexpr double useed_value() {
	return double(uint64_t(1) << useed_scale<es>());
}

// generate the minpos bit pattern for the sign requested (true is negative half, false is positive half)
template<unsigned nbits, unsigned es, typename bt>
blockbinary<nbits, bt, BinaryNumberType::Signed> minpos_pattern(bool sign = false) {
	blockbinary<nbits, bt, BinaryNumberType::Signed> _bits;
	_bits.clear();
	_bits.setbit(0, true);
	return (sign ? twosComplement(_bits) : _bits);
}

// generate the maxpos bit pattern for the sign requested (true is negative half, false is positive half)
template<unsigned nbits, unsigned es, typename bt>
blockbinary<nbits, bt, BinaryNumberType::Signed> maxpos_pattern(bool sign = false) {
	blockbinary<nbits, bt, BinaryNumberType::Signed> _bits;
	_bits.clear();
	_bits.flip();
	_bits.setbit(nbits - 1, false);
	return (sign ? twosComplement(_bits) : _bits);
}

// maximum value of regime 0
template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> maxprecision_max() {
	posit<nbits, es, bt> a;
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
template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> maxprecision_min() {
	posit<nbits, es, bt> a;
	// set regime -1
	a.clear();
	a.setbit(nbits - 3, true);
	// set all exponent and fraction bits are already set to 0 with the reset()
	return a;
}

// maxpos_scale, minpos_scale, calculate_k, calculate_unconstrained_k
// are defined in posit_scale_helpers.hpp (included before posit_impl.hpp).
// The following 3-param overloads preserve backward compatibility for
// call sites that pass an unused <bt> template argument.
template<unsigned nbits, unsigned es, typename bt>
constexpr int calculate_k(int scale) {
	return calculate_k<nbits, es>(scale);
}
template<unsigned nbits, unsigned es, typename bt>
constexpr int calculate_unconstrained_k(int scale) {
	return calculate_unconstrained_k<nbits, es>(scale);
}

template<unsigned nbits, unsigned es, typename bt>
constexpr inline int sign_value(const posit<nbits, es, bt>& p) {
	blockbinary<nbits, bt, BinaryNumberType::Signed> _bits = p.bits();
	return (_bits.test(nbits - 1) ? -1 : 1);
}

template<unsigned nbits, unsigned es, typename bt>
inline long double regime_value(const posit<nbits, es, bt>& p) {
	positRegime<nbits, es, bt>    _regime;
	blockbinary<nbits, bt, BinaryNumberType::Signed> tmp(p.bits());
	tmp = sign(p) ? twosComplement(tmp) : tmp;
	_regime.assign_regime_pattern(decode_regime(tmp));
	return _regime.value();
}

template<unsigned nbits, unsigned es, typename bt>
inline long double exponent_value(const posit<nbits, es, bt>& p) {
	positRegime<nbits, es, bt>    _regime;
	positExponent<nbits, es, bt>  _exponent;
	blockbinary<nbits, bt, BinaryNumberType::Signed> tmp(p.bits());
	tmp = sign(p) ? twosComplement(tmp) : tmp;
	unsigned nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp)); // get the regime bits
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);			 // get the exponent bits
	return _exponent.value();
}

template<unsigned nbits, unsigned es, typename bt>
inline long double fraction_value(const posit<nbits, es, bt>& p) {
	constexpr unsigned fbits = nbits - 3 - es;
	bool		     		_sign;
	positRegime<nbits, es, bt>   _regime;
	positExponent<nbits, es, bt> _exponent;
	positFraction<fbits, bt>     _fraction;
	decode(p.bits(), _sign, _regime, _exponent, _fraction);
	return _fraction.value();
}

// get the sign of the posit
template<unsigned nbits, unsigned es, typename bt>
constexpr inline bool sign(const posit<nbits, es, bt>& p) {
	return p.isneg();
}

// calculate the scale of a posit
template<unsigned nbits, unsigned es, typename bt>
inline int scale(const posit<nbits, es, bt>& p) {
	positRegime<nbits, es, bt>    _regime;
	positExponent<nbits, es, bt>  _exponent;
	blockbinary<nbits, bt> tmp(p.bits());
	tmp = sign(p) ? twosComplement(tmp) : tmp;
	int k = decode_regime(tmp);
	unsigned nrRegimeBits = _regime.assign_regime_pattern(k);	// get the regime bits
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);							// get the exponent bits
	// return the scale
	return _regime.scale() + _exponent.scale();
}

// calculate the significant of a posit
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
inline blockbinary<fbits+1, bt, BinaryNumberType::Unsigned> significant(const posit<nbits, es, bt>& p) {
	//constexpr unsigned fbits = nbits - 3 - es;
	bool		     	 _sign;
	positRegime<nbits, es, bt>    _regime;
	positExponent<nbits, es, bt>  _exponent;
	positFraction<fbits, bt>      _fraction;
	decode(p.bits(), _sign, _regime, _exponent, _fraction);
	return _fraction.get_fixed_point();
}

// get the fraction bits of a posit
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
inline blockbinary<fbits, bt> extract_fraction(const posit<nbits, es, bt>& p) {
	//constexpr unsigned fbits = nbits - 3 - es;
	bool		     	 _sign;
	positRegime<nbits, es, bt>    _regime;
	positExponent<nbits, es, bt>  _exponent;
	positFraction<fbits, bt>      _fraction;
	decode(p.bits(), _sign, _regime, _exponent, _fraction);
	// _fraction.bits() returns Unsigned blockbinary; copy to Signed for compatibility
	blockbinary<fbits, bt> result{};
	auto ubits = _fraction.bits();
	for (unsigned i = 0; i < fbits; ++i) result.setbit(i, ubits.test(i));
	return result;
}

// calculate the scale of the regime component of the posit
template<unsigned nbits, unsigned es, typename bt>
inline int regime_scale(const posit<nbits, es, bt>& p) {
	positRegime<nbits, es, bt>    _regime;
	blockbinary<nbits, bt> tmp(p.bits());
	tmp = sign(p) ? twosComplement(tmp) : tmp;
	_regime.assign_regime_pattern(decode_regime(tmp));
	return _regime.scale();
}

// calculate the scale of the exponent component of the posit
template<unsigned nbits, unsigned es, typename bt>
inline int exponent_scale(const posit<nbits, es, bt>& p) {
	positRegime<nbits, es, bt>    _regime;
	positExponent<nbits, es, bt>  _exponent;
	blockbinary<nbits, bt> tmp(p.bits());
	tmp = sign(p) ? twosComplement(tmp) : tmp;
	unsigned nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);
	return _exponent.scale();
}


//////////////////////////////////////////////////////////////////////////

}} // namespace sw::universal
