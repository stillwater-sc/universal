#pragma once
// fixpnt_functions.hpp: information functions for fixed-point types and values
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/bitblock/bitblock.hpp>

namespace sw {
namespace unum {

// functions to provide details about
// the properties of a fixed-point configuration


// calculate exponential scale of maxpos
template<size_t nbits, size_t rbits>
int scale_maxpos_fixpnt() {
	assert(nbits >= rbits);
	return (nbits > rbits) ? (nbits - rbits - 1) : 1;
}

// calculate exponential scale of minpos
template<size_t nbits, size_t rbits>
int scale_minpos_fixpnt() {
	return -int(rbits);
}

// calculate the value of maximum positive number
template<size_t nbits, size_t rbits>
long double value_maxpos_fixpnt() {
	return (long double)((0x1 << scale_maxpos_fixpnt<nbits, rbits>()) - 1);
}

// calculate the value of maximum negative number
template<size_t nbits, size_t rbits>
long double value_maxneg_fixpnt() {
	return -(long double)(0x1 << scale_maxpos_fixpnt<nbits, rbits>());
}

// calculate the value of minimum positive number
template<size_t nbits, size_t rbits>
long double value_minpos_fixpnt() {
	return (long double)(std::pow(2.0l, scale_minpos_fixpnt<nbits, rbits>()));
}

// calculate the value of minimum positive number
template<size_t nbits, size_t rbits>
long double value_minneg_fixpnt() {
	return -(long double)(std::pow(2.0l, scale_minpos_fixpnt<nbits, rbits>()));
}

// generate the maxneg through maxpos value range of a fixed-point configuration
template<size_t nbits, size_t rbits>
void ReportFixedPointRanges(std::ostream& ostr = std::cout) {
	using namespace std;
	ostr << "fixpnt<" << nbits << ", " << rbits << "> : "
		<< value_maxneg_fixpnt<nbits, rbits>() << " "
		<< value_minneg_fixpnt<nbits, rbits>() << " "
		<< "0 "
		<< value_minpos_fixpnt<nbits, rbits>() << " "
		<< value_maxpos_fixpnt<nbits, rbits>()
		<< endl;
}

// forward reference
template<size_t nbits, size_t rbits> class fixpnt;

template<size_t nbits, size_t rbits>
inline int sign_value(const fixpnt<nbits, rbits>& p) {
	return (p < 0) ? -1 : 1;
}

template<size_t nbits, size_t rbits>
inline long double fraction_value(const fixpnt<nbits, rbits>& p) {
	return 0.0l;
}

// get the sign of the fixed-point
template<size_t nbits, size_t rbits>
inline bool sign(const fixpnt<nbits, rbits>& p) {
	return (p < 0) ? true : false;
}

// calculate the scale of the fixed-point value
template<size_t nbits, size_t rbits>
inline int scale(const fixpnt<nbits, rbits>& p) {
	int _scale;
	return _scale;
}

// get the fraction bits of a posit
template<size_t nbits, size_t rbits>
inline bitblock<rbits> extract_fraction(const fixpnt<nbits, rbits>& p) {
	bitblock<rbits> fraction;
	for (int i = 0; i < rbits; ++i) {
	fraction[i] = p[i];
	}
	return fraction;
}

} // namespace unum
} // namespace sw
