// integer_to_posit.hpp: conversion routines to take an integer<size> type into a posit<nbits,es> type
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/posit/posit>
#include <universal/integer/integer>

namespace sw {
namespace unum {

/*
  Why is the convert function not part of the Integer or Posit types?
  It would tightly couple the types, which we want to avoid.
  If we want to productize these convertions we would need a new
  layer in the module design that sits above the Universal types. TODO
 */

// convert a Posit to an Integer
template<typename Integer, typename Posit>
void convert_p2i(const Posit& p, Integer& v) {
	// get the scale of the posit value
	int scale = sw::unum::scale(p);
	if (scale < 0) {
		v = 0;
		return;
	}
	if (scale == 0) {
		v = 1;
	}
	else {
		// gather all the fraction bits
		// sw::unum::bitblock<p.fhbits> significant = sw::unum::significant<p.nbits, p.es, p.fbits>(p);
                sw::unum::bitblock<Posit::fhbits> significant = sw::unum::significant<Posit::nbits, Posit::es, Posit::fbits>(p);
		// the radix point is at fbits, to make an integer out of this
		// we shift that radix point fbits to the right.
		// that is equivalent to a scale of 2^fbits
		v.clear();
		int msb = (v.nbits < p.fbits + 1) ? v.nbits : p.fbits + 1;
		for (int i = msb-1; i >= 0; --i) {
			v.set(i, significant[i]);
		}
		int shift = scale - p.fbits;  // if scale > fbits we need to shift left
		v <<= shift;
		if (p.isneg()) {
			v.flip();
			v += 1;
		}
	}
}

// native type specializations
template<typename Posit>
void convert_p2i(const Posit& p, short& v) {
	v = short(p);
}
template<typename Posit>
void convert_p2i(const Posit& p, int& v) {
	v = int(p);
}
template<typename Posit>
void convert_p2i(const Posit& p, long& v) {
	v = long(p);
}
template<typename Posit>
void convert_p2i(const Posit& p, long long& v) {
	v = (long long)(p);
}
template<typename Posit>
void convert_p2i(const Posit& p, unsigned short& v) {
	v = (unsigned short)(p);
}
template<typename Posit>
void convert_p2i(const Posit& p, unsigned int& v) {
	v = (unsigned int)(p);
}
template<typename Posit>
void convert_p2i(const Posit& p, unsigned long& v) {
	v = (unsigned long)(p);
}
template<typename Posit>
void convert_p2i(const Posit& p, unsigned long long& v) {
	v = (unsigned long long)(p);
}

/////////////////////////////////////////////////////////////////////////
// convert an Integer to a Posit
template<typename Integer, typename Posit>
void convert_i2p(const Integer& w, Posit& p) {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t ibits = w.nbits;

	bool sign = w < 0;
	bool isZero = w == 0;
	bool isInf = false;
	bool isNan = false;
	long _scale = scale(w);
	Integer w2 = sign ? twos_complement(w) : w;
	int msb = findMsb(w2);
	bitblock<ibits> fraction_without_hidden_bit;
	int fbit = ibits - 1;
	for (int i = msb - 1; i >= 0; --i) {
		fraction_without_hidden_bit.set(fbit, w2.at(i));
		--fbit;
	}
	value<ibits> v;
	v.set(sign, _scale, fraction_without_hidden_bit, isZero, isInf, isNan);
	p = v;
}

// native type specializations
template<typename Posit>
void convert_i2p(short& v, Posit& p) {
	v = short(p);
}
template<typename Posit>
void convert_i2p(int& v, Posit& p) {
	v = int(p);
}
template<typename Posit>
void convert_i2p(long& v, Posit& p) {
	v = long(p);
}
template<typename Posit>
void convert_i2p(long long& v, Posit& p) {
	v = (long long)(p);
}
template<typename Posit>
void convert_i2p(unsigned short& v, Posit& p) {
	v = (unsigned short)(p);
}
template<typename Posit>
void convert_i2p(unsigned int& v, Posit& p) {
	v = (unsigned int)(p);
}
template<typename Posit>
void convert_i2p(unsigned long& v, Posit& p) {
	v = (unsigned long)(p);
}
template<typename Posit>
void convert_i2p(unsigned long long& v, Posit& p) {
	v = (unsigned long long)(p);
}

} // namespace unum
} // namespace sw
