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

template<unsigned es>
constexpr unsigned useed_scale() {
	return (1u << es);
}

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
template<unsigned nbits, unsigned es, typename bt = uint8_t>
blockbinary<nbits, bt, BinaryNumberType::Signed> minpos_pattern(bool sign = false) {
	blockbinary<nbits, bt, BinaryNumberType::Signed> _bits;
	_bits.clear();
	_bits.setbit(0, true);
	return (sign ? twosComplement(_bits) : _bits);
}

// generate the maxpos bit pattern for the sign requested (true is negative half, false is positive half)
template<unsigned nbits, unsigned es, typename bt = uint8_t>
blockbinary<nbits, bt, BinaryNumberType::Signed> maxpos_pattern(bool sign = false) {
	blockbinary<nbits, bt, BinaryNumberType::Signed> _bits;
	_bits.clear();
	_bits.flip();
	_bits.setbit(nbits - 1, false);
	return (sign ? twosComplement(_bits) : _bits);
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
template<unsigned nbits, unsigned es, typename bt>
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
template<unsigned nbits, unsigned es, typename bt>
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
