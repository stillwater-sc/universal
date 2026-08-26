#pragma once
// posit_fields_io.hpp: the text layer for a posit's fields -- regime, exponent, fraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// operator<<, operator>> and to_string for positRegime, positExponent and
// positFraction (#1334). They used to sit in the field headers themselves, which
// posit_impl.hpp includes -- so posit's arithmetic core pulled <sstream>, <ostream>
// and <istream> for three debug printers, and posit_fraction.hpp reached the stream
// operators only through an unrelated <regex> include that happened to be in the
// graph. Removing that <regex> is what exposed this.
//
// The field types are internal to posit, so this header is separate from
// posit/iostream.hpp rather than folded into it: posit/iostream.hpp includes
// posit_impl.hpp, and posit_impl.hpp needs these operators when POSIT_TRACE_ENABLED
// is on -- folding them together would be circular.
//
// Self-contained: include it directly and it works.
#include <iostream>
#include <sstream>
#include <string>

#include <universal/number/posit/posit_regime.hpp>
#include <universal/number/posit/posit_exponent.hpp>
#include <universal/number/posit/posit_fraction.hpp>

namespace sw { namespace universal {

/////////////////  REGIME operators
template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const positRegime<nbits, es, bt>& r) {
	blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> bb = r.bits();
	unsigned nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (r._nrRegimeBits > nrOfRegimeBitsProcessed++) {
			ostr << (bb.test(unsigned(i)) ? '1' : '0');
		}
		else {
			ostr << '-';
		}
	}
	return ostr;
}

template<unsigned nbits, unsigned es, typename bt>
inline std::istream& operator>> (std::istream& istr, const positRegime<nbits, es, bt>& r) {
	istr >> r._block;
	return istr;
}

template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const positRegime<nbits, es, bt>& r, bool dashExtent = true, bool nibbleMarker = false) {
	std::stringstream s;
	blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> bb = r.bits();
	unsigned nrOfRegimeBitsProcessed = 0;
	for (unsigned i = 0; i < nbits - 1; ++i) {
		unsigned bitIndex = nbits - 2ul - i;
		if (r.nrBits() > nrOfRegimeBitsProcessed++) {
			s << (bb.test(bitIndex) ? '1' : '0');
			if (nibbleMarker && ((bitIndex % 4) == 0) && bitIndex != 0) s << '\'';
		}
		else {
			s << (dashExtent ? "-" : "");
		}	
	}
	return s.str();
}


/////////////////// EXPONENT operators
template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const positExponent<nbits, es, bt>& e) {
	if constexpr (es > 0) {
		unsigned nrOfExponentBitsProcessed = 0;
		for (unsigned i = 0; i < es; ++i) {
			unsigned bitIndex = es - 1ull - i;
			if (e._nrExpBits > nrOfExponentBitsProcessed++) {
				ostr << (e.test(bitIndex) ? "1" : "0");
			}
			else {
				ostr << "-";
			}
		}
	}
	else {
		ostr << "~"; // for proper alignment in tables
	}
	return ostr;
}

template<unsigned nbits, unsigned es, typename bt>
inline std::istream& operator>> (std::istream& istr, const positExponent<nbits, es, bt>& e) {
	istr >> e._Bits;
	return istr;
}

template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const positExponent<nbits, es, bt>& e, bool dashExtent = true, bool nibbleMarker = false) {
	using UnsignedExponent = blockbinary<es, bt, BinaryNumberType::Unsigned>;
	std::stringstream s;
	unsigned nrOfExponentBitsProcessed = 0;
	if constexpr (es > 0) {
		for (unsigned i = 0; i < es; ++i) {
			unsigned bitIndex = es - 1ull - i;
			bool emitted = false;
			if (e.nrBits() > nrOfExponentBitsProcessed++) {
				UnsignedExponent positExponentBits = e.bits();
				s << (positExponentBits.test(bitIndex) ? '1' : '0');
				emitted = true;
			}
			else if (dashExtent) {
				s << '-';
				emitted = true;
			}
			if (emitted && nibbleMarker && ((bitIndex % 4) == 0) && bitIndex != 0) s << '\'';
		}
	}
	else {
		s << '~'; // for proper alignment in tables
	}
	return s.str();
}


////////////////////// FRACTION operators
template<unsigned nfbits, typename bbt>
inline std::ostream& operator<<(std::ostream& ostr, const positFraction<nfbits, bbt>& f) {
	unsigned nrOfFractionBitsProcessed = 0;
	if constexpr (nfbits > 0) {
		int upperbound = int(nfbits) - 1;
		for (int i = upperbound; i >= 0; --i) {
			if (f._nrBits > ++nrOfFractionBitsProcessed) {
				ostr << (f._block.test(unsigned(i)) ? "1" : "0");
			}
			else {
				ostr << "-";
			}
		}
	}
	if (nrOfFractionBitsProcessed == 0) ostr << "~"; // for proper alignment in tables
	return ostr;
}

template<unsigned nfbits, typename bbt>
inline std::istream& operator>> (std::istream& istr, const positFraction<nfbits, bbt>& f) {
	istr >> f._block;
	return istr;
}

template<unsigned nfbits, typename bbt>
inline std::string to_string(const positFraction<nfbits, bbt>& f, bool dashExtent = true, bool nibbleMarker = false) {
	unsigned int nrOfFractionBitsProcessed = 0;
	std::stringstream s;
	if constexpr (nfbits > 0) {
		blockbinary<nfbits, bbt, BinaryNumberType::Unsigned> bb = f.bits();
		for (unsigned i = 0; i < nfbits; ++i) {
			unsigned bitIndex = nfbits - 1ull - i;
			bool emitted = false;
			if (f.nrBits() > nrOfFractionBitsProcessed++) {
				s << (bb.test(bitIndex) ? '1' : '0');
				emitted = true;
			}
			else if (dashExtent) {
				s << '-';
				emitted = true;
			}
			if (emitted && nibbleMarker && ((bitIndex % 4) == 0) && bitIndex != 0) s << '\'';
		}
	}
	if (nrOfFractionBitsProcessed == 0) s << '~'; // for proper alignment in tables
	return s.str();
}


}} // namespace sw::universal
