#pragma once
// attributes.hpp: functions to query number system attributes
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// functions to provide details about properties of a posit configuration

using namespace sw::universal::internal;

// calculate exponential scale of useed
template<size_t es>
constexpr size_t useed_scale() {
	return (size_t(1) << es);
}

template<size_t es>
constexpr size_t useed() {
	return size_t(1) << (size_t(1) << es);
}

// calculate exponential scale of maxpos
template<size_t nbits, size_t es>
constexpr int maxpos_scale() {
	return (nbits - 2) * (1 << es);
}

// calculate exponential scale of minpos
template<size_t nbits, size_t es>
constexpr int minpos_scale() {
	return static_cast<int>(2 - int(nbits)) * (1 << es);
}

// calculate the constrained k value
template<size_t nbits, size_t es, typename bt>
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
template<size_t nbits, size_t es, typename bt>
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

template<size_t nbits, size_t es, typename bt>
constexpr inline int sign_value(const posit<nbits, es, bt>& p) {
	blockbinary<nbits, bt, BinaryNumberType::Signed> _bits = p.bits();
	return (_bits.test(nbits - 1) ? -1 : 1);
}

template<size_t nbits, size_t es, typename bt>
inline long double regime_value(const posit<nbits, es, bt>& p) {
	regime<nbits, es, bt>    _regime;
	blockbinary<nbits, bt, BinaryNumberType::Signed> tmp(p.bits());
	tmp = sign(p) ? twosComplement(tmp) : tmp;
	_regime.assign_regime_pattern(decode_regime(tmp));
	return _regime.value();
}

template<size_t nbits, size_t es, typename bt>
inline long double exponent_value(const posit<nbits, es, bt>& p) {
	regime<nbits, es, bt>    _regime;
	exponent<nbits, es, bt>  _exponent;
	blockbinary<nbits, bt, BinaryNumberType::Signed> tmp(p.bits());
	tmp = sign(p) ? twosComplement(tmp) : tmp;
	size_t nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp)); // get the regime bits
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);			 // get the exponent bits
	return _exponent.value();
}

template<size_t nbits, size_t es, typename bt>
inline long double fraction_value(const posit<nbits, es, bt>& p) {
	constexpr size_t fbits = nbits - 3 - es;
	bool		     		_sign;
	regime<nbits, es, bt>   _regime;
	exponent<nbits, es, bt> _exponent;
	fraction<fbits, bt>     _fraction;
	decode(p.bits(), _sign, _regime, _exponent, _fraction);
	return _fraction.value();
}

// get the sign of the posit
template<size_t nbits, size_t es, typename bt>
constexpr inline bool sign(const posit<nbits, es, bt>& p) {
	return p.isneg();
}

// calculate the scale of a posit
template<size_t nbits, size_t es, typename bt>
inline int scale(const posit<nbits, es, bt>& p) {
	regime<nbits, es, bt>    _regime;
	exponent<nbits, es, bt>  _exponent;
	blockbinary<nbits, bt> tmp(p.bits());
	tmp = sign(p) ? twos_complement(tmp) : tmp;
	int k = decode_regime(tmp);
	size_t nrRegimeBits = _regime.assign_regime_pattern(k);	// get the regime bits
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);							// get the exponent bits
	// return the scale
	return _regime.scale() + _exponent.scale();
}

// calculate the significant of a posit
template<size_t nbits, size_t es, typename bt, size_t fbits>
inline blockbinary<fbits+1, bt, BinaryNumberType::Unsigned> significant(const posit<nbits, es, bt>& p) {
	//constexpr size_t fbits = nbits - 3 - es;
	bool		     	 _sign;
	regime<nbits, es, bt>    _regime;
	exponent<nbits, es, bt>  _exponent;
	fraction<fbits, bt>      _fraction;
	decode(p.bits(), _sign, _regime, _exponent, _fraction);
	return _fraction.get_fixed_point();
}

// get the fraction bits of a posit
template<size_t nbits, size_t es, typename bt, size_t fbits>
inline blockbinary<fbits, bt> extract_fraction(const posit<nbits, es, bt>& p) {
	//constexpr size_t fbits = nbits - 3 - es;
	bool		     	 _sign;
	regime<nbits, es, bt>    _regime;
	exponent<nbits, es, bt>  _exponent;
	fraction<fbits, bt>      _fraction;
	decode(p.bits(), _sign, _regime, _exponent, _fraction);
	return _fraction.get();
}

// calculate the scale of the regime component of the posit
template<size_t nbits, size_t es, typename bt>
inline int regime_scale(const posit<nbits, es, bt>& p) {
	regime<nbits, es, bt>    _regime;
	blockbinary<nbits, bt> tmp(p.get());
	tmp = sign(p) ? twos_complement(tmp) : tmp;
	_regime.assign_regime_pattern(decode_regime(tmp));
	return _regime.scale();
}

// calculate the scale of the exponent component of the posit
template<size_t nbits, size_t es, typename bt>
inline int exponent_scale(const posit<nbits, es, bt>& p) {
	regime<nbits, es, bt>    _regime;
	exponent<nbits, es, bt>  _exponent;
	blockbinary<nbits, bt> tmp(p.get());
	tmp = sign(p) ? twos_complement(tmp) : tmp;
	size_t nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));
	_exponent.extract_exponent_bits(tmp, nrRegimeBits);
	return _exponent.scale();
}


//////////////////////////////////////////////////////////////////////////

// report dynamic range of a type, specialized for a posit
template<size_t nbits, size_t es>
std::string dynamic_range() {
	posit<nbits, es, std::uint32_t> p;
	return dynamic_range(p);
}

// report the dynamic range of the type associated with a value
template<size_t nbits, size_t es, typename bt>
std::string dynamic_range(const posit<nbits, es, bt>& p) {
	std::stringstream str;
	posit<nbits, es, bt> v(p);
	str << type_tag(v) << '\n';
	str << "useed scale  " << std::setw(4) << useed_scale<es>() << "     ";
	str << "minpos scale " << std::setw(10) << v.minpos().scale() << "     ";
	str << "maxpos scale " << std::setw(10) << v.maxpos().scale();
	return str.str();
}

// report the dynamic range of a posit
template<size_t nbits, size_t es, typename bt>
std::string posit_range() {
	std::stringstream str;
	posit<nbits, es, bt> p;
	str << type_tag(p) << '\n';
	str << "useed scale  " << std::setw(4) << useed_scale<nbits, es>() << "     ";
	str << "minpos scale " << std::setw(10) << minpos_scale<nbits, es>() << "     ";
	str << "maxpos scale " << std::setw(10) << maxpos_scale<nbits, es>() << "     ";
	str << "minimum " << std::setw(12) << std::numeric_limits<sw::universal::posit<nbits, es>>::min() << "     ";
	str << "maximum " << std::setw(12) << std::numeric_limits<sw::universal::posit<nbits, es>>::max();
	return str.str();
}

}} // namespace sw::universal
