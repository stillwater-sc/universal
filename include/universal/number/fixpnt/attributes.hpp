#pragma once
// attributes.hpp: information functions for fixed-point type and value attributes
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath> // for std:pow()

namespace sw { namespace universal {

// functions to provide details about
// the properties of a fixed-point configuration
// in terms of native types.
// Since many fixed-point configurations cannot be represented by native types,
// these are all convenience functions.
// They should not be used for the core algorithms.

// calculate exponential scale of maxpos
template<size_t nbits, size_t rbits>
int scale_maxpos_fixpnt() {
	assert(nbits >= rbits);
	return (nbits > rbits) ? (nbits - rbits - 1) : 0;
}

// calculate exponential scale of minpos
template<size_t nbits, size_t rbits>
int scale_minpos_fixpnt() {
	assert(nbits >= rbits);
	return -int(rbits);
}

// generate the maxneg through maxpos value range of a fixed-point configuration
// the type of arithmetic, Modulo or Saturating, does not affect the range
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
std::string fixpnt_range(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
	using FixedPoint = fixpnt<nbits, rbits, arithmetic, bt>;
	std::stringstream s;
	FixedPoint fp;
	s << std::setw(40) << type_tag(v) << " : [ "
		<< fp.maxneg() << " ... "
		<< fp.minneg() << " "
		<< "0 "
		<< fp.minpos() << " ... "
		<< fp.maxpos() << " ]";
	return s.str();
}

// free function to get the sign of a fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline bool sign(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
	return v.sign();
}

// free function to get the fraction of a fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline long double fraction_value(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
	fixpnt<nbits, rbits, arithmetic, bt> fp(v);
	// null the integer part
	for (size_t i = rbits; i < nbits; ++i) {
		fp.reset(i);
	}
	return (long double)(fp);
}



// clang <complex> implementation is calling these functions so we need implementations for fixpnt

//template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
//inline bool isnan(const fixpnt<nbits, rbits, arithmetic, bt>& p) { return false; }

//template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
//inline bool isinf(const fixpnt<nbits, rbits, arithmetic, bt>& p) { return false; }

// copysign returns a value with the magnitude of a, and the sign of b
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
inline fixpnt<nbits, rbits, arithmetic, bt> copysign(const fixpnt<nbits, rbits, arithmetic, bt>& a, const fixpnt<nbits, rbits, arithmetic, bt>& b) { 
    fixpnt<nbits, rbits, arithmetic, bt> c(a);
    if (a.sign() == b.sign()) return c;
    return -c; 
}

}} // namespace sw::universal
