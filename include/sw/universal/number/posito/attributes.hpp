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

// get the sign of the posito
template<unsigned nbits, unsigned es>
constexpr inline bool sign(const posito<nbits, es>& p) {
	return p.isneg();
}

// calculate the scale of a posit
template<unsigned nbits, unsigned es>
inline int scale(const posito<nbits, es>& p) {
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

}} // namespace sw::universal
