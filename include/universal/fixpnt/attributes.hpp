#pragma once
// attributes.hpp: information functions for fixed-point type and value attributes
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath> // for std:pow()
#include <universal/bitblock/bitblock.hpp>

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
	return -int(rbits);
}

/*  these should not be here
  it is incorrect to calculate these independent of the fixpnt sampling
// calculate the value of maximum positive number
template<size_t nbits, size_t rbits>
long double value_maxpos_fixpnt() {
	// 2's complement maxpos value = 2^(nbits-1) - 1
	// fixed-point's shift is 2^rbits
	// maxpos = 2's compl maxpos / shift
	long double numerator = (long double)((1ul << (nbits-1)) - 1);
	long double denominator = (long double)(1ul << (rbits));
	return numerator / denominator;
}

// calculate the value of maximum negative number
template<size_t nbits, size_t rbits>
long double value_maxneg_fixpnt() {
	// 2's complement maxneg value = 2^(nbits-1)
	// fixed-point's shift is 2^rbits
	// maxneg = 2's compl maxneg / shift
	long double numerator = (long double)(1ul << (nbits-1));
	long double denominator = (long double)(1ul << (rbits));
	return -numerator / denominator;
}

// calculate the value of minimum positive number
template<size_t nbits, size_t rbits>
long double value_minpos_fixpnt() {
	long double denominator = (long double)(1ul << (rbits));
	return 1.0l / denominator;
}

// calculate the value of minimum positive number
template<size_t nbits, size_t rbits>
long double value_minneg_fixpnt() {
	long double denominator = (long double)(1ul << (rbits));
	return -1.0l / denominator;
}
*/

// generate the maxneg through maxpos value range of a fixed-point configuration
// the type of arithmetic, Modulo or Saturating, does not affect the range
template<size_t nbits, size_t rbits>
void ReportFixedPointRanges(std::ostream& ostr = std::cout) {
	using namespace std;
	using FixedPoint = fixpnt<nbits, rbits, sw::universal::Saturating, uint32_t>;
	FixedPoint fp;
	ostr << "fixpnt<" << nbits << ", " << rbits << "> : "
		<< maxneg(fp) << " "
		<< minneg(fp) << " "
		<< "0 "
		<< minpos(fp) << " "
		<< maxpos(fp)
		<< endl;
}

template<size_t nbits, size_t rbits, bool arithmetic>
inline int sign_value(const fixpnt<nbits, rbits, arithmetic>& p) {
	return (p < 0) ? -1 : 1;
}

template<size_t nbits, size_t rbits, bool arithmetic>
inline long double fraction_value(const fixpnt<nbits, rbits, arithmetic>& p) {
	return 0.0l;
}

// get the sign of the fixed-point
template<size_t nbits, size_t rbits, bool arithmetic>
inline bool sign(const fixpnt<nbits, rbits, arithmetic>& p) {
	return (p < 0) ? true : false;
}

// get the fraction bits of a posit
template<size_t nbits, size_t rbits, bool arithmetic>
inline sw::unum::bitblock<rbits> extract_fraction(const fixpnt<nbits, rbits, arithmetic>& p) {
	sw::unum::bitblock<rbits> fraction;
	for (int i = 0; i < rbits; ++i) {
	fraction[i] = p[i];
	}
	return fraction;
}

// clang <complex> implementation is calling these functions so we need implementations for fixpnt

template<size_t nbits, size_t rbits, bool arithmetic>
inline bool isnan(const fixpnt<nbits, rbits, arithmetic>& p) { return false; }

template<size_t nbits, size_t rbits, bool arithmetic>
inline bool isinf(const fixpnt<nbits, rbits, arithmetic>& p) { return false; }

// copysign returns a value with the magnitude of a, and the sign of b
template<size_t nbits, size_t rbits, bool arithmetic>
inline fixpnt<nbits, rbits, arithmetic> copysign(const fixpnt<nbits, rbits, arithmetic>& a, const fixpnt<nbits, rbits, arithmetic>& b) { 
    fixpnt<nbits, rbits, arithmetic> c(a);
    if (a.sign() == b.sign()) return c;
    return -c; 
}

}} // namespace sw::universal
