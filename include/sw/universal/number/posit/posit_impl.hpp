#pragma once
// posit_impl.hpp: implementation of fixed-size arbitrary configuration generalized posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>
#include <regex>
#include <type_traits>

#if POSIT_THROW_ARITHMETIC_EXCEPTION
// propagate this behavior down to constituent classes
#ifndef BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION
#define BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION 1
#endif
#endif

// calling environment should define behavioral flags
// typically set in the library aggregation include file <posit>
// but can be set by individual programs when including posit.hpp
// For example:
// - define to non-zero if you want to enable arithmetic and logic literals
// #define POSIT_ENABLE_LITERALS 1
// - define to non-zero if you want to throw exceptions on arithmetic errors
// #define POSIT_THROW_ARITHMETIC_EXCEPTION 1

#include <universal/utility/find_msb.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/internal/value/value.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
// intermediate value tracing
#include <universal/number/algorithm/trace_constants.hpp>
// posit environment
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/number/posit/positFraction.hpp>
#include <universal/number/posit/positExponent.hpp>
#include <universal/number/posit/positRegime.hpp>
#include <universal/number/posit/attributes.hpp>

namespace sw { namespace universal {

// inject internal namespace
using namespace sw::universal::internal;

// specialized configuration constants
constexpr unsigned NBITS_IS_2   =   2;
constexpr unsigned NBITS_IS_3   =   3;
constexpr unsigned NBITS_IS_4   =   4;
constexpr unsigned NBITS_IS_5   =   5;
constexpr unsigned NBITS_IS_6   =   6;
constexpr unsigned NBITS_IS_7   =   7;
constexpr unsigned NBITS_IS_8   =   8;
constexpr unsigned NBITS_IS_10  =  10;
constexpr unsigned NBITS_IS_12  =  12;
constexpr unsigned NBITS_IS_14  =  14;
constexpr unsigned NBITS_IS_16  =  16;
constexpr unsigned NBITS_IS_20  =  20;
constexpr unsigned NBITS_IS_24  =  24;
constexpr unsigned NBITS_IS_28  =  28;
constexpr unsigned NBITS_IS_32  =  32;
constexpr unsigned NBITS_IS_40  =  40;
constexpr unsigned NBITS_IS_48  =  48;
constexpr unsigned NBITS_IS_56  =  56;
constexpr unsigned NBITS_IS_64  =  64;
constexpr unsigned NBITS_IS_80  =  80;
constexpr unsigned NBITS_IS_96  =  96;
constexpr unsigned NBITS_IS_128 = 128;
constexpr unsigned NBITS_IS_256 = 256;
constexpr unsigned ES_IS_0 = 0;
constexpr unsigned ES_IS_1 = 1;
constexpr unsigned ES_IS_2 = 2;
constexpr unsigned ES_IS_3 = 3;
constexpr unsigned ES_IS_4 = 4;
constexpr unsigned ES_IS_5 = 5;

// Not A Real is the posit encoding for INFINITY and arithmetic errors that can propagate
// The symbol NAR can be used to initialize a posit, i.e., posit<nbits,es>(NAR), or posit<nbits,es> p = NAR
#define NAR INFINITY

///////////////////////////////////////////////////////////////////////////////////////////////
// key posit algorithms

// special case check for projecting values between (0, minpos] to minpos and [maxpos, inf) to maxpos
// Returns true if the scale is too small or too large for this posit config
// DO NOT USE the k value for this, as the k value encodes the useed regions
// and thus is too coarse to make this decision.
// Using the scale directly is the simplest expression of the inward projection test.
template<unsigned nbits, unsigned es>
bool check_inward_projection_range(int scale) {
	// calculate the min/max k factor for this posit config
	int posit_size = nbits;
	int k = scale < 0 ? -(posit_size - 2) : (posit_size - 2);
	return scale < 0 ? scale < k*(1 << es) : scale > k*(1 << es);
}

// decode_regime measures the run-length of the regime and returns the k value associated with that run-length
// how many shifts represent the regime?
// regime = useed ^ k = (2 ^ (2 ^ es)) ^ k = 2 ^ (k*(2 ^ es))
// scale  = useed ^ k * 2^e = k*(2 ^ es) + e 
template<unsigned nbits, typename bt>
int decode_regime(const blockbinary<nbits, bt, BinaryNumberType::Signed>& raw_bits) {
	// let m be the number of identical bits in the regime
	int m = 0;   // regime runlength counter
	int k = 0;   // converted regime scale
	if (raw_bits.test(nbits - 2)) {   // run length of 1's
		m = 1;   // if a run of 1's k = m - 1
		int start = (nbits == 2 ? nbits - 2 : nbits - 3);
		for (int i = start; i >= 0; --i) {
			if (raw_bits.test(unsigned(i))) {
				m++;
			}
			else {
				break;
			}
		}
		k = m - 1;
	}
	else {
		m = 1;  // if a run of 0's k = -m
		int start = (nbits == 2 ? nbits - 2 : nbits - 3);
		for (int i = start; i >= 0; --i) {
			if (raw_bits.test(unsigned(i))) {
				break;
			}
			else {
				m++;
			}
		}
		k = -m;
	}
	return k;
}

// extract_fields takes a raw posit encoding and extracts the sign, regime, exponent, and fraction components
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
void extract_fields(const blockbinary<nbits, bt, BinaryNumberType::Signed>& raw_bits, bool& _sign, positRegime<nbits, es, bt>& _regime, positExponent<nbits, es, bt>& _exponent, positFraction<fbits, bt>& _fraction) {
	using TwosComplementNumber = blockbinary<nbits, bt, BinaryNumberType::Signed>;
	// check special case
	if (raw_bits.iszero()) {
		_sign = false;
		_regime.setzero();
		_exponent.setzero();
		_fraction.setzero();
		return;
	}
	TwosComplementNumber tmp(raw_bits);
	_sign = raw_bits.test(nbits - 1);
	if (_sign) tmp = twosComplement(tmp);
	unsigned nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));

	// get the exponent bits
	// start of exponent is nbits-1 - (sign_bit + regime_bits)
	int msb = static_cast<int>(nbits - 1ul - (1ul + nrRegimeBits));
	unsigned nrExponentBits = (msb >= static_cast<int>(es - 1ull)) ? es : static_cast<unsigned>(msb + 1ll);
	if constexpr (es > 0) {
		_exponent.reset();
		if (msb >= 0) {
			for (unsigned i = 0; i < nrExponentBits; ++i) {
				_exponent.setbit(es - 1ul - i, tmp.at(static_cast<unsigned>(msb) - i));
			}
		}
		_exponent.setNrBits(nrExponentBits);
	}

	// finally, set the fraction bits
	// we do this so that the fraction is right extended with 0;
	// The max fraction is <nbits - 3 - es>, but we are setting it to <nbits - 3> and right-extent
	// The msb bit of the fraction represents 2^-1, the next 2^-2, etc.
	// If the fraction is empty, we have a fraction of nbits-3 0 bits
	// If the fraction is one bit, we have still have fraction of nbits-3, with the msb representing 2^-1, and the rest are right extended 0's
	blockbinary<fbits, bt, BinaryNumberType::Unsigned> _frac{ 0 };
	msb = msb - int(nrExponentBits);
	unsigned nrFractionBits = (msb < 0 ? 0ull : static_cast<unsigned>(msb) + 1ull);
	if (msb >= 0) {
		//std::cout <<  "  : " << to_binary(_frac) << '\n';
		unsigned msfbit = static_cast<unsigned>(msb);
		for (unsigned i = 0; i <= msfbit; ++i) {
			_frac.setbit(fbits - 1ull - (msfbit - i), tmp.at(i));
		}
//		for (int i = msb; i >= 0; --i) {
//			_frac[fbits - 1ull - (static_cast<unsigned>(msb) - static_cast<unsigned>(i))] = tmp[static_cast<unsigned>(i)];
//		}
	}
	_fraction.set(_frac, nrFractionBits);
}

// decode takes the raw bits representing a posit coming from memory
// and decodes the sign, regime, the exponent, and the fraction.
// This function has the functionality of the posit register-file load.
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
void decode(const blockbinary<nbits, bt, BinaryNumberType::Signed>& raw_bits, bool& _sign, positRegime<nbits, es, bt>& _regime, positExponent<nbits, es, bt>& _exponent, positFraction<fbits, bt>& _fraction) {
	//_block = raw_bits;	// store the raw bits for reference
	// check special cases
	_sign = raw_bits.test(nbits - 1);
	if (_sign) {
		blockbinary<nbits, bt> tmp(raw_bits);
		tmp.reset(nbits - 1);
		if (tmp.none()) {
			// setnar();   special case = NaR (Not a Real)
			_sign = true;
			_regime.setinf();
			_exponent.reset();
		}
		else {
			extract_fields(raw_bits, _sign, _regime, _exponent, _fraction);
		}
	}
	else {
		if (raw_bits.none()) {
			// setzero();  special case = 0
			_sign = false;
			_regime.setzero();  // <-- all the 0's end up in the regime
			_exponent.reset();
			_fraction.reset();
		}
		else {
			extract_fields(raw_bits, _sign, _regime, _exponent, _fraction);
		}
	}
	//if (_trace_decode) std::cout << "raw bits: " << raw_bits << " posit bits: " << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;
	if (_trace_decode) std::cout << "raw bits: " << raw_bits << " posit bits: " << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << std::endl;
}

#ifdef TBD
// needed to avoid double rounding situations during arithmetic: TODO: does that mean the condensed version below should be removed?
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
inline blockbinary<nbits, bt, BinaryNumberType::Signed>& convert_to_bb(bool _sign, int _scale, const blockbinary<fbits, bt>& fraction_in, blockbinary<nbits, bt, BinaryNumberType::Signed>& ptt) {
	if (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
	if (_trace_conversion) std::cout << "sign " << (_sign ? "-1 " : " 1 ") << "scale " << std::setw(3) << _scale << " fraction " << fraction_in << std::endl;

	ptt.reset(); // ptt will yield the final bits of the posit
	// construct the posit
	// interpolation rule checks
	if (check_inward_projection_range<nbits, es>(_scale)) {    // regime dominated
		if (_trace_conversion) std::cout << "inward projection" << std::endl;
		// we are projecting to minpos/maxpos
		int k = calculate_unconstrained_k<nbits, es>(_scale);
		ptt = k < 0 ? minpos_pattern<nbits, es>(_sign) : maxpos_pattern<nbits, es>(_sign);
		// we are done
		if (_trace_rounding) std::cout << "projection  rounding ";
	}
	else {
		const unsigned pt_len = nbits + 3 + es;
		blockbinary<pt_len, bt> pt_bits;
		blockbinary<pt_len, bt> regime;
		blockbinary<pt_len, bt> exponent;
		blockbinary<pt_len, bt> fraction;
		blockbinary<pt_len, bt> sticky_bit;

		bool s = _sign;
		int e = _scale;
		bool r = (e >= 0);

		unsigned run = static_cast<unsigned>(r ? 1ll + (e >> es) : -(e >> es));
		regime.set(0, 1 ^ r);  // TODO: this expression can be improved 1 ^ r is the same as !r as r is a boolean, no need for a bitwise operator
		for (unsigned i = 1; i <= run; i++) regime.set(i, r);

		unsigned esval = e % (unsigned(1) << static_cast<int>(es));
		exponent = convert_to_bitblock<pt_len>(esval);
		unsigned nf = unsigned(std::max<int>(0, (static_cast<int>(nbits) + 1) - (2 + int(run) + static_cast<int>(es))));
		// TODO: what needs to be done if nf > fbits?
		//assert(nf <= input_fbits);
		// copy the most significant nf fraction bits into fraction
		unsigned lsb = nf <= fbits ? 0 : nf - fbits;
		for (unsigned i = lsb; i < nf; i++) fraction[i] = fraction_in[fbits - nf + i];

		bool sb = anyAfter(fraction_in, static_cast<int>(fbits) - 1 - int(nf));

		// construct the untruncated posit
		// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
		regime <<= es + nf + 1;
		exponent <<= nf + 1;
		fraction <<= 1;
		sticky_bit.set(0, sb);

		pt_bits |= regime;
		pt_bits |= exponent;
		pt_bits |= fraction;
		pt_bits |= sticky_bit;

		unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
		bool blast = pt_bits.test(len - nbits);
		bool bafter = pt_bits.test(len - nbits - 1);
		bool bsticky = anyAfter(pt_bits, int(len) - static_cast<int>(nbits) - 1 - 1);

		bool rb = (blast & bafter) | (bafter & bsticky);

		pt_bits <<= pt_len - len;
		truncate(pt_bits, ptt);
		if (rb) increment_bitset(ptt);
		if (s) ptt = twos_complement(ptt);
	}
	return ptt;
}
#endif

// needed to avoid double rounding situations during arithmetic: TODO: does that mean the condensed version below should be removed?
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
inline posit<nbits, es, bt>& convert_(bool _sign, int _scale, const blocksignificand<fbits, bt>& fraction_in, posit<nbits, es, bt>& p) {
	if constexpr (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
	if constexpr (_trace_conversion) std::cout << "sign " << (_sign ? "-1 " : " 1 ") << "scale " << std::setw(3) << _scale << " fraction " << fraction_in << std::endl;

	p.clear();
	// construct the posit
	// interpolation rule checks
	if (check_inward_projection_range<nbits, es>(_scale)) {    // regime dominated
		if (_trace_conversion) std::cout << "inward projection" << std::endl;
		// we are projecting to minpos/maxpos or minneg/maxneg
		int k = calculate_unconstrained_k<nbits, es, bt>(_scale);
		k < 0 ? (_sign ? p.minneg() : p.minpos()) : (_sign ? p.maxneg() : p.maxpos());
		// we are done
		if (_trace_rounding) std::cout << "projection  rounding ";
	}
	else {
		constexpr unsigned pt_len = nbits + 3 + es;
		using BlockBinary = blockbinary<pt_len, bt, BinaryNumberType::Signed>;
		BlockBinary pt_bits{ 0 };
		BlockBinary regime{ 0 };
		BlockBinary exponent{0};
		BlockBinary fraction{0};
		BlockBinary sticky_bit{0};

		bool s = _sign;
		int e  = _scale;
		bool r = (e >= 0);  // positive or negative regime to create runs such as: 11..110 or 00..001

		unsigned run = unsigned(r ? 1 + (e >> es) : -(e >> es));
		regime.setbit(0, 1 ^ r);
		for (unsigned i = 1; i <= run; ++i) regime.setbit(i, r);

		exponent = e % (1ull << es);
		int nbits_plus_one = static_cast<int>(nbits) + 1;
		int sign_regime_es = static_cast<int>(2ull + run + es);
		unsigned nrFbits = static_cast<unsigned>(std::max<int>(0, (nbits_plus_one - sign_regime_es)));

		// when nrFbits > fbits then we need to round
		// copy the most significant nf fraction bits into fraction
		unsigned lsb = (nrFbits <= fbits ? 0 : nrFbits - fbits);
		for (unsigned i = lsb; i < nrFbits; ++i) fraction.setbit(i, fraction_in.test(fbits - nrFbits + i));

		bool sb = (nrFbits < fbits) ? fraction_in.anyAfter(fbits - nrFbits) : false;

		// construct the untruncated posit
		// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
		regime <<= es + nrFbits + 1u;
		exponent <<= nrFbits + 1u;
		fraction <<= 1u;
		sticky_bit.setbit(0, sb);

		pt_bits |= regime;
		pt_bits |= exponent;
		pt_bits |= fraction;
		pt_bits |= sticky_bit;

		unsigned len = 1 + std::max<unsigned>((nbits + 1ull), (2u + run + es));
		bool blast = pt_bits.test(len - nbits);
		bool bafter = pt_bits.test(len - nbits - 1ull);
		bool bsticky = pt_bits.anyAfter(len - nbits - 1ull);

		bool rb = (blast & bafter) | (bafter & bsticky);

		blockbinary<nbits, bt> ptt{0};
		pt_bits <<= pt_len - len;
		truncate(pt_bits, ptt);
		if (rb) ++ptt;
		if (s) ptt = ptt.twosComplement();
		p.setbits(ptt);
	}
	return p;
}

// convert a floating point value to a specific posit configuration. Semantically, p = v, return reference to p
template<unsigned nbits, unsigned es, typename bt, unsigned fbits, BlockTripleOperator op>
inline posit<nbits, es, bt>& convert(const blocktriple<fbits, op, bt>& v, posit<nbits, es, bt>& p) {
	if constexpr (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << '\n';
	if constexpr (_trace_conversion) std::cout << to_triple(v) << " : " << v << '\n';

	if (v.iszero()) {
		p.setzero();
		return p;
	}
	if (v.isnan() || v.isinf()) {
		p.setnar();
		return p;
	}
	// The blocktriple's significant may have overflowed into the integer bits
	// (especially after ADD). The real scale = v.scale() + v.significandscale().
	// Extract the fraction bits starting below the MSB position.
	using BlockTriple = blocktriple<fbits, op, bt>;
	int sigScale = v.significandscale(); // how many bits the MSB is above radix
	int realScale = v.scale() + sigScale;
	// The MSB (hidden bit) is at position radix + sigScale.
	// Fraction bits start at (radix + sigScale - 1) downward.
	// We extract enough bits for convert_ rounding.
	// nrFbits can be up to nbits-1-es (when regime is minimal), so we need
	// at least nbits-1-es + 3 (guard/round/sticky) = nbits+2-es bits.
	// Using nbits+4 is safe for all configurations and still fits within the blocktriple.
	constexpr unsigned extractBits = nbits + 4;
	blocksignificand<extractBits, bt> frac;
	int msbPos = static_cast<int>(BlockTriple::radix) + sigScale; // position of hidden bit
	for (unsigned i = 0; i < extractBits; ++i) {
		int srcPos = msbPos - 1 - static_cast<int>(i); // start from bit below MSB
		if (srcPos >= 0 && srcPos < static_cast<int>(BlockTriple::bfbits)) {
			frac.setbit(extractBits - 1 - i, v.at(static_cast<unsigned>(srcPos)));
		}
	}
	return convert_<nbits, es, bt, extractBits>(v.sign(), realScale, frac, p);
}

// Bridge: convert internal::value<fbits> to posit (needed by quire output path)
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
inline posit<nbits, es, bt>& convert(const internal::value<fbits>& v, posit<nbits, es, bt>& p) {
	if (v.iszero()) { p.setzero(); return p; }
	if (v.isinf() || v.isnan()) { p.setnar(); return p; }
	// Copy bitblock fraction to blocksignificand
	blocksignificand<fbits, bt> sig;
	sig.clear();
	bitblock<fbits> frac = v.fraction();
	for (unsigned i = 0; i < fbits; ++i) sig.setbit(i, frac[i]);
	return convert_<nbits, es, bt, fbits>(v.sign(), v.scale(), sig, p);
}

// Bridge: extract internal::value<fbits> from a posit (needed by quire input path)
template<unsigned nbits, unsigned es, typename bt>
internal::value<nbits - 3 - es> posit_to_value(const posit<nbits, es, bt>& p) {
	constexpr unsigned pf = nbits - 3 - es;
	internal::value<pf> v;
	if (p.iszero()) return v;
	if (p.isnar()) { v.setinf(); return v; }
	// Extract fraction as blockbinary, convert to bitblock
	blockbinary<pf, bt> frac_bb = extract_fraction<nbits, es, bt, pf>(p);
	bitblock<pf> frac_bits;
	for (unsigned i = 0; i < pf; ++i) frac_bits[i] = frac_bb.test(i);
	v.set(sign(p), scale(p), frac_bits, false, false);
	return v;
}

// Bridge: normalize a posit to an internal::value<tgt_fbits> (needed by quire_add)
template<unsigned nbits, unsigned es, typename bt, unsigned tgt_fbits>
void posit_normalize_to(const posit<nbits, es, bt>& p, internal::value<tgt_fbits>& v) {
	constexpr unsigned pf = nbits - 3 - es;
	blockbinary<pf, bt> frac_bb = extract_fraction<nbits, es, bt, pf>(p);
	bitblock<tgt_fbits> _fr;
	int tgt, src;
	for (tgt = int(tgt_fbits) - 1, src = int(pf) - 1; tgt >= 0 && src >= 0; tgt--, src--) _fr[tgt] = frac_bb.test(static_cast<unsigned>(src));
	v.set(sign(p), scale(p), _fr, p.iszero(), p.isnar());
}

// quadrant returns a two character string indicating the quadrant of the projective reals the posit resides: from 0, SE, NE, NaR, NW, SW
template<unsigned nbits, unsigned es, typename bt>
std::string quadrant(const posit<nbits, es, bt>& p) {
	posit<nbits, es, bt> pOne(1), pMinusOne(-1);
	if (sign(p)) {
		// west
		if (p > pMinusOne) {
			return "SW";
		}
		else {
			return "NW";
		}
	}
	else {
		// east
		if (p < pOne) {
			return "SE";
		}
		else {
			return "NE";
		}
	}
}

// Construct posit from its components
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
posit<nbits, es, bt>& construct(bool s, const positRegime<nbits, es, bt>& r, const positExponent<nbits, es, bt>& e, const positFraction<fbits, bt>& f, posit<nbits, es, bt>& p) {
	// generate raw bit representation
	blockbinary<nbits, bt> _block = s ? twos_complement(collect(s, r, e, f)) : collect(s, r, e, f);
	_block.set(nbits - 1, s);
	p.set(_block);
	return p;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class posit represents posit numbers of arbitrary configuration and their basic arithmetic operations (add/sub, mul/div)
template<unsigned _nbits, unsigned _es, typename bt = std::uint8_t>
class posit {

	static_assert(_es < 10ull, "my God that is a big number, are you trying to break the Interweb?");

public:
	static constexpr unsigned   nbits   = _nbits;
	static constexpr unsigned   es      = _es;
	static constexpr unsigned   sbits   = 1;                          // number of sign bits:     specified
	static constexpr unsigned   rbits   = nbits - sbits;              // maximum number of regime bits:   derived
	static constexpr unsigned   ebits   = es;                         // maximum number of exponent bits: specified
	static constexpr unsigned   fbits   = (es + 2 >= nbits ? 0 : nbits - 3 - es); // maximum number of fraction bits: derived
	static constexpr unsigned   fhbits  = fbits + 1;                  // maximum number of fraction + one hidden bit

	static constexpr unsigned   abits   = fhbits + 3;                 // size of the addend
	static constexpr unsigned   mbits   = 2 * fhbits;                 // size of the multiplier output
	static constexpr unsigned   divbits = 3 * fhbits + 4;             // size of the divider output

	static constexpr unsigned   bitsInByte = 8ull;
	static constexpr unsigned   bitsInBlock = sizeof(bt) * bitsInByte;
//	static_assert(bitsInBlock <= 64, "storage unit for block arithmetic needs to be <= uint64_t"); // TODO: carry propagation on uint64_t requires assembly code, but allow single block uint64_t

	static constexpr unsigned   storageMask = (0xFFFFFFFFFFFFFFFFull >> (64ull - bitsInBlock));
	static constexpr bt       BLOCK_MASK = bt(~0);
	static constexpr bt       ALL_ONES = bt(~0); // block type specific all 1's value
	static constexpr uint32_t ALL_ONES_ES = (0xFFFFul >> (16 - es));

	static constexpr unsigned   nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr unsigned   MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken
	static constexpr bt       MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr unsigned   bitsInMSU = bitsInBlock - (nrBlocks * bitsInBlock - nbits);

	static constexpr bt       SIGN_BIT_MASK = bt(bt(1ull) << ((nbits - 1ull) % bitsInBlock));
	static constexpr bt       LSB_BIT_MASK = bt(1ull);

	typedef bt BlockType;

	/// trivial constructor
	posit() = default;
	
	constexpr posit(const posit&) = default;
	constexpr posit(posit&&) = default;
	
	posit& operator=(const posit&) = default;
	posit& operator=(posit&&) = default;

	/// Construct posit from another posit with the same alignment
	template<unsigned nnbits, unsigned ees, typename bbt>
	posit(const posit<nnbits, ees, bbt>& a) {
		*this = double(a);
	}

	// specific value constructor
	constexpr posit(const SpecificValue code) noexcept
		: _block{}  {
		switch (code) {
		case SpecificValue::infpos:
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::infneg:
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::snan:
		case SpecificValue::qnan:
		case SpecificValue::nar:
			setnar();
			break;
		}
	}

	// initializers for native types, allow for implicit conversion (Peter)
	constexpr posit(signed char initial_value)        noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(short initial_value)              noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(int initial_value)                noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(long initial_value)               noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(long long initial_value)          noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(char initial_value)               noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(unsigned short initial_value)     noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(unsigned int initial_value)       noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(unsigned long initial_value)      noexcept : _block{ 0 } { *this = initial_value; }
	constexpr posit(unsigned long long initial_value) noexcept : _block{ 0 } { *this = initial_value; }
	CONSTEXPRESSION posit(float initial_value)        noexcept : _block{ 0 } { *this = initial_value; }
	CONSTEXPRESSION posit(double initial_value)       noexcept : _block{ 0 } { *this = initial_value; }

	// assignment operators for native types
	// Route integer assignments through convert_ieee754 via double cast.
	// The blocktriple path has a known issue where round() shifts the hidden
	// bit below the radix position, causing convert(blocktriple, posit) to
	// misread it as a fraction bit, producing off-by-one rounding errors.
	constexpr posit& operator=(signed char rhs)        noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(short rhs)              noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(int rhs)                noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(long rhs)               noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(long long rhs)          noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(char rhs)               noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(unsigned short rhs)     noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(unsigned int rhs)       noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(unsigned long rhs)      noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	constexpr posit& operator=(unsigned long long rhs) noexcept { return convert_ieee754(static_cast<double>(rhs)); }
	CONSTEXPRESSION posit& operator=(float rhs) noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION posit& operator=(double rhs) noexcept { return convert_ieee754(rhs); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	CONSTEXPRESSION posit(long double initial_value)  noexcept : _block{ 0 } { *this = initial_value; }
	CONSTEXPRESSION posit& operator=(long double rhs) noexcept { return convert_ieee754(rhs); }
	// TODO: we need this regardless as the design marshalls values through long double
	// explicit operator long double() const noexcept { return to_native<long double>(); }
#endif

#ifdef ADAPTER_POSIT_AND_INTEGER
	// convenience assignment operator
	template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
	posit& operator=(const integer<nbits, BlockType, NumberType>& rhs) {
		convert_i2p(rhs, *this);
		return *this;
	}
#endif

	// assignment for value type
	template<unsigned vbits>
	posit& operator=(const blocksignificand<vbits, bt>& rhs) {
		clear();
		convert(rhs, *this);
		return *this;
	}
	
	// negation operator
	posit operator-() const {
		if (iszero()) {
			return *this;
		}
		if (isnar()) {
			return *this;
		}
		posit<nbits, es, bt> negated;  // TODO: artificial initialization to pass -Wmaybe-uninitialized
		negated.setbits(twosComplement(_block));
		return negated;
	}
	// prefix increment operator
	posit& operator++() {
		++_block;
		return *this;
	}
	// postfix increment operator
	posit operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	// prefix decrement operator
	posit& operator--() {
		--_block;
		return *this;
	}
	// postfix decrement operator
	posit operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}

	// we model a hw pipeline with register assignments, functional block, and conversion
	posit& operator+=(const posit& rhs) {
		if (_trace_add) std::cout << "---------------------- ADD -------------------" << std::endl;
		// special case handling of the inputs
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		if (isnar() || rhs.isnar()) {
			throw posit_operand_is_nar{};
		}
#else
		if (isnar() || rhs.isnar()) {
			setnar();
			return *this;
		}
#endif
		if (iszero()) {
			*this = rhs;
			return *this;
		}
		if (rhs.iszero()) return *this;

		// arithmetic operation
		blocktriple<fbits, BlockTripleOperator::ADD, bt> a, b, sum;

		// transform the inputs into (sign,scale,fraction) triples
		normalizeAddition(a);
		rhs.normalizeAddition(b);
		//module_add<fbits,abits>(a, b, sum);		// add the two inputs
		sum.add(a, b);

		// special case handling of the result
		if (sum.iszero()) {
			setzero();
		}
		else if (sum.isinf()) {
			setnar();
		}
		else {
			convert(sum, *this);
		}
		return *this;                
	}
	posit& operator+=(double rhs) {
		return *this += posit<nbits, es>(rhs);
	}
	posit& operator-=(const posit& rhs) {
		return *this += (-rhs);
	}
	posit& operator-=(double rhs) {
		return *this -= posit<nbits, es>(rhs);
	}
	posit& operator*=(const posit& rhs) {
		static_assert(fhbits > 0, "posit configuration does not support multiplication");
		if (_trace_mul) std::cout << "---------------------- MUL -------------------" << std::endl;
		// special case handling of the inputs
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		if (isnar() || rhs.isnar()) {
			throw posit_operand_is_nar{};
		}
#else
		if (isnar() || rhs.isnar()) {
			setnar();
			return *this;
		}
#endif
		if (iszero() || rhs.iszero()) {
			setzero();
			return *this;
		}

		// arithmetic operation via blocktriple
		blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;

		// transform the inputs into (sign,scale,fraction) triples
		normalizeMultiplication(a);
		rhs.normalizeMultiplication(b);
		product.mul(a, b);

		// special case handling on the output
		if (product.iszero()) {
			setzero();
		}
		else if (product.isinf()) {
			setnar();
		}
		else {
			convert(product, *this);
		}
		return *this;
	}
	posit& operator*=(double rhs) {
		return *this *= posit<nbits, es>(rhs);
	}
	posit& operator/=(const posit& rhs) {
		if (_trace_div) std::cout << "---------------------- DIV -------------------" << std::endl;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) {
			throw posit_divide_by_zero{};    // not throwing is a quiet signalling NaR
		}
		if (rhs.isnar()) {
			throw posit_divide_by_nar{};
		}
		if (isnar()) {
			throw posit_numerator_is_nar{};
		}
		if (iszero() || isnar()) {
			return *this;
		}
#else
		// not throwing is a quiet signalling NaR
		if (rhs.iszero()) {
			setnar();
			return *this;
		}
		if (rhs.isnar()) {
			setnar();
			return *this;
		}
		if (iszero() || isnar()) {
			return *this;
		}
#endif
		// arithmetic operation via blocktriple
		blocktriple<fbits, BlockTripleOperator::DIV, bt> a, b, ratio;

		// transform the inputs into (sign,scale,fraction) triples
		normalizeDivision(a);
		rhs.normalizeDivision(b);
		ratio.div(a, b);

		// special case handling on the output
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		if (ratio.iszero()) {
			throw posit_division_result_is_zero{};
		}
		else if (ratio.isinf()) {
			throw posit_division_result_is_infinite{};
		}
		else {
			convert(ratio, *this);
		}
#else
		if (ratio.iszero()) {
			setzero();  // this shouldn't happen as we should project back onto minpos
		}
		else if (ratio.isinf()) {
			setnar();  // this shouldn't happen as we should project back onto maxpos
		}
		else {
			convert(ratio, *this);
		}
#endif

		return *this;
	}
	posit& operator/=(double rhs) {
		return *this /= posit<nbits, es, bt>(rhs);
	}
	
	posit reciprocal() const noexcept {
		if (_trace_reciprocal) std::cout << "-------------------- RECIPROCAL ----------------" << std::endl;
		posit<nbits, es, bt> p;
		// special case of NaR (Not a Real)
		if (isnar()) {
			p.setnar();
			return p;
		}
		if (iszero()) {
			p.setnar();
			return p;
		}
		// compute the reciprocal
		bool old_sign = _block[nbits-1];
		blockbinary<nbits, bt> raw_bits;
		if (ispowerof2()) {
			raw_bits = twos_complement(_block);
			raw_bits.set(nbits-1, old_sign);
			p.setbits(raw_bits);
		}
		else {
			bool s{ false };
			positRegime<nbits, es, bt> r;
			positExponent<nbits, es, bt> e;
			positFraction<fbits, bt> f;
			decode(_block, s, r, e, f);

			constexpr unsigned operand_size = fhbits;
			blockbinary<operand_size, bt> one;
			one.set(operand_size - 1, true);
			blockbinary<operand_size, bt> frac;
			copy_into(f.bits(), 0, frac);
			frac.set(operand_size - 1, true);
			constexpr unsigned reciprocal_size = 3 * fbits + 4;
			blockbinary<reciprocal_size> reciprocal;
			divide_with_fraction(one, frac, reciprocal);
			if (_trace_reciprocal) {
				std::cout << "one    " << one << std::endl;
				std::cout << "frac   " << frac << std::endl;
				std::cout << "recip  " << reciprocal << std::endl;
			}

			// radix point falls at operand size == reciprocal_size - operand_size - 1
			reciprocal <<= operand_size - 1;
			if (_trace_reciprocal) std::cout << "frac   " << reciprocal << std::endl;
			int new_scale = -scale(*this);
			int msb = findMostSignificantBit(reciprocal);
			if (msb > 0) {
				int shift = static_cast<int>(reciprocal_size - static_cast<unsigned>(msb));
				reciprocal <<= static_cast<unsigned>(shift);
				new_scale -= (shift-1);
				if (_trace_reciprocal) std::cout << "result " << reciprocal << std::endl;
			}
			//std::bitset<operand_size> tr;
			//truncate(reciprocal, tr);
			//std::cout << "tr     " << tr << std::endl;

			// the following is failing for some reason
			// value<reciprocal_size> v(old_sign, new_scale, reciprocal);
			// convert(v, p);
			// instead the following works
			convert_<nbits, es, bt, reciprocal_size>(old_sign, new_scale, reciprocal, p);
		}
		return p;
	}
	// absolute value is simply the 2's complement when negative
	posit abs() const noexcept {
		posit p(*this);
		if (isneg()) p.setbits(twosComplement(_block));
		return p;
	}

	// make conversions to native types explicit
	explicit operator int()         const noexcept { return static_cast<int>(to_native<float>()); }
	explicit operator long()        const noexcept { return static_cast<long>(to_native<double>()); }
	explicit operator long long()   const noexcept { return static_cast<long long>(to_native<long double>()); }
	explicit operator float()       const noexcept { return to_native<float>(); }
	explicit operator double()      const noexcept { return to_native<double>(); }
	explicit operator long double() const noexcept { return to_native<long double>(); }

	// Selectors
	constexpr bool sign() const noexcept { return _block.test(nbits - 1); }
	int scale() const noexcept { 
		blockbinary<nbits, bt> tmp(bits());
		tmp = sign() ? twosComplement(tmp) : tmp;
		positRegime<nbits, es, bt> r;
		int k = decode_regime(tmp);
		unsigned nrRegimeBits = r.assign_regime_pattern(k);
		positExponent<nbits, es, bt> e;
		e.extract_exponent_bits(tmp, nrRegimeBits);
		return r.scale() + e.scale();
	}
	constexpr bool isnar() const noexcept {
		if (!_block.test(nbits - 1)) return false;
		blockbinary<nbits, bt> tmp(_block);			
		tmp.setbit(nbits - 1, false);
		return tmp.none();
	}
	constexpr bool iszero() const noexcept { return _block.none() ? true : false; }
	constexpr bool isone() const noexcept { // pattern 010000....
		blockbinary<nbits, bt> tmp(_block);
		tmp.setbit(nbits - 2, false);
		return _block.test(nbits - 2) && tmp.none();
	}
	constexpr bool isminusone() const noexcept { // pattern 110000...
		blockbinary<nbits, bt> tmp(_block);
		tmp.setbit(nbits - 1, false);
		tmp.setbit(nbits - 2, false);
		return _block.test(nbits - 1) && _block.test(nbits - 2) && tmp.none();
	}
	constexpr bool isneg() const noexcept { return _block.test(nbits - 1); }
	constexpr bool ispos() const noexcept { return !_block.test(nbits - 1); }
	constexpr bool ispowerof2() const noexcept {
		bool s{ false };
		positRegime<nbits, es, bt> r;
		positExponent<nbits, es, bt> e;
		positFraction<fbits, bt> f;
		decode(_block, s, r, e, f);
		return f.none();
	}
	constexpr bool isinteger() const noexcept { return true; } // TODO: return (floor(*this) == *this) ? true : false; }

	blockbinary<nbits, bt, BinaryNumberType::Signed> bits() const noexcept { return _block; }
	unsigned long long encoding() const noexcept { return _block.to_ullong(); }

	// Modifiers
	constexpr void clear() noexcept { _block.clear(); }
	constexpr void flip() noexcept { _block.flip(); }
	constexpr void setzero() noexcept { _block.clear(); }
	constexpr void setnar() noexcept {
		_block.clear();
		_block.setbit(nbits - 1);
	}
	constexpr posit& maxpos() noexcept {
		_block.clear();
		_block.setbit(nbits - 1);
		_block.flip();
		return *this;
	}
	constexpr posit& minpos() noexcept {
		_block.clear();
		_block.setbit(0);
		return *this;
	}
	constexpr posit& zero() noexcept {
		_block.clear();
		return *this;
	}
	constexpr posit& minneg() noexcept {
		_block.clear();
		_block.flip();
		return *this;
	}
	constexpr posit& maxneg() noexcept {
		_block.clear();
		_block.setbit(nbits - 1);
		_block.setbit(0);
		return *this;
	}

	// set the posit bits explicitely
	constexpr posit<nbits, es, bt>& setbits(const blockbinary<nbits, bt, BinaryNumberType::Signed>& block) {
		_block = block;
		return *this;
	}
	// compatibility alias for old posit API
	constexpr posit<nbits, es, bt>& setBitblock(const blockbinary<nbits, bt, BinaryNumberType::Signed>& block) {
		return setbits(block);
	}
	// Set the raw bits of the posit given an unsigned value starting from the lsb. Handy for enumerating a posit state space
	constexpr posit<nbits, es, bt>& setbits(uint64_t value) {
		_block.setbits(value);
		return *this;
	}

	// currently, size is tied to fbits size of posit config. Is there a need for a case that captures a user-defined sized fraction?
	template<BlockTripleOperator op = BlockTripleOperator::REP>
	blocktriple<fbits, op, bt> to_value() const noexcept {
		using namespace sw::universal::internal;
		blocktriple<fbits, op, bt> v;
		if (iszero()) { v.setzero(); return v; }
		if (isnar())  { v.setnan();  return v; }
		bool		     		_sign{ false };
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(_block, _sign, _regime, _exponent, _fraction);
		v.setnormal();
		v.setsign(_sign);
		v.setscale(_regime.scale() + _exponent.scale());
		// copy fraction bits into blocktriple significant with hidden bit
		v.setbit(fbits);  // hidden bit
		auto frac = _fraction.bits();
		for (unsigned i = 0; i < fbits; ++i) {
			v.setbit(i, frac.at(i));
		}
		return v;
	}

	constexpr void normalizeAddition(blocktriple<fbits, BlockTripleOperator::ADD, bt>& tgt) const {
		using BlockTripleConfiguration = blocktriple<fbits, BlockTripleOperator::ADD, bt>;
		// test special cases
		if (isnar()) {
			tgt.setnan();
		}
		else if (iszero()) {
			tgt.setzero();
		}
		else {
			tgt.setnormal(); // a blocktriple is always normalized
			// decode the posit fields
			bool		     		s{ false };
			positRegime<nbits, es, bt>   r;
			positExponent<nbits, es, bt> e;
			positFraction<fbits, bt>     f;
			decode(_block, s, r, e, f);
			tgt.setsign(s);
			tgt.setscale(r.scale() + e.scale());
			// extract fraction bits into the blocktriple significant with hidden bit
			if constexpr (fbits < 64 && BlockTripleConfiguration::rbits < (64 - fbits)) {
				uint64_t raw = f.bits().to_ull();
				raw |= (1ull << fbits); // add the hidden bit
				raw <<= BlockTripleConfiguration::rbits;  // rounding bits required for correct rounding
				tgt.setbits(raw);
			}
			else {
				// copy fraction bits into blocktriple block by block
				auto frac = f.bits();
				constexpr unsigned fracBlocks = blockbinary<fbits, bt, BinaryNumberType::Unsigned>::nrBlocks;
				for (unsigned i = 0; i < fracBlocks; ++i) {
					tgt.setblock(i, frac[i]);
				}
				tgt.setradix();
				tgt.setbit(fbits); // add the hidden bit
				tgt.bitShift(BlockTripleConfiguration::rbits);  // rounding bits required for correct rounding
			}
		}
	}


	constexpr void normalizeMultiplication(blocktriple<fbits, BlockTripleOperator::MUL, bt>& tgt) const {
		// test special cases
		if (isnar()) {
			tgt.setnan();
		}
		else if (iszero()) {
			tgt.setzero();
		}
		else {
			tgt.setnormal();
			// decode the posit fields
			bool		     		s{ false };
			positRegime<nbits, es, bt>   r;
			positExponent<nbits, es, bt> e;
			positFraction<fbits, bt>     f;
			decode(_block, s, r, e, f);
			tgt.setsign(s);
			tgt.setscale(r.scale() + e.scale());
			// extract fraction bits into the blocktriple significant with hidden bit
			// no rounding shift needed for MUL — blocktriple::mul handles radix placement
			if constexpr (fbits < 64) {
				uint64_t raw = f.bits().to_ull();
				raw |= (1ull << fbits); // add the hidden bit
				tgt.setbits(raw);
			}
			else {
				auto frac = f.bits();
				constexpr unsigned fracBlocks = blockbinary<fbits, bt, BinaryNumberType::Unsigned>::nrBlocks;
				for (unsigned i = 0; i < fracBlocks; ++i) {
					tgt.setblock(i, frac[i]);
				}
				tgt.setbit(fbits); // add the hidden bit
			}
		}
	}

	constexpr void normalizeDivision(blocktriple<fbits, BlockTripleOperator::DIV, bt>& tgt) const {
		using BlockTripleConfiguration = blocktriple<fbits, BlockTripleOperator::DIV, bt>;
		constexpr unsigned divshift = BlockTripleConfiguration::divshift;
		// test special cases
		if (isnar()) {
			tgt.setnan();
		}
		else if (iszero()) {
			tgt.setzero();
		}
		else {
			tgt.setnormal();
			// decode the posit fields
			bool		     		s{ false };
			positRegime<nbits, es, bt>   r;
			positExponent<nbits, es, bt> e;
			positFraction<fbits, bt>     f;
			decode(_block, s, r, e, f);
			tgt.setsign(s);
			tgt.setscale(r.scale() + e.scale());
			// extract fraction bits with hidden bit, shifted left by divshift for alignment
			if constexpr (fbits < 64 && divshift < (64 - fbits)) {
				uint64_t raw = f.bits().to_ull();
				raw |= (1ull << fbits); // add the hidden bit
				raw <<= divshift;       // alignment shift for division
				tgt.setbits(raw);
			}
			else {
				auto frac = f.bits();
				constexpr unsigned fracBlocks = blockbinary<fbits, bt, BinaryNumberType::Unsigned>::nrBlocks;
				for (unsigned i = 0; i < fracBlocks; ++i) {
					tgt.setblock(i, frac[i]);
				}
				tgt.setbit(fbits); // add the hidden bit
				tgt.bitShift(divshift);  // alignment shift for division
			}
		}
	}

	// helper debug function
	void constexprClassParameters() const noexcept {
		std::cout << "-------------------------------------------------------------\n";
		std::cout << "type              : " << type_tag(*this) << '\n';
		std::cout << "nbits             : " << nbits << '\n';
		std::cout << "es                : " << es << std::endl;
		std::cout << "ALL_ONES          : " << to_binary(ALL_ONES, 0, true) << '\n';
		std::cout << "BLOCK_MASK        : " << to_binary(BLOCK_MASK, 0, true) << '\n';
		std::cout << "nrBlocks          : " << nrBlocks << '\n';
		std::cout << "bits in MSU       : " << bitsInMSU << '\n';
		std::cout << "MSU               : " << MSU << '\n';
		std::cout << "MSU MASK          : " << to_binary(MSU_MASK, 0, true) << '\n';
		std::cout << "SIGN_BIT_MASK     : " << to_binary(SIGN_BIT_MASK, 0, true) << '\n';
		std::cout << "LSB_BIT_MASK      : " << to_binary(LSB_BIT_MASK, 0, true) << '\n';
	}
	void showLimbs() const {
		for (unsigned b = 0; b < nrBlocks; ++b) {
			std::cout << to_binary(_block[nrBlocks - b - 1], sizeof(bt) * 8) << ' ';
		}
		std::cout << '\n';
	}

private:
	blockbinary<nbits, bt, BinaryNumberType::Signed> _block;

	// HELPER methods

	template<typename Real>
	Real to_native() const {
		if (iszero())  return 0.0l;
		if (isnar())   return std::numeric_limits<Real>::quiet_NaN();;
		bool		     		_sign{ false };
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(_block, _sign, _regime, _exponent, _fraction);
		Real s = (_sign ? -1.0l : 1.0l);
		Real r = _regime.value();
		Real e = _exponent.value();
		Real f = (1.0l + _fraction.value());
		return s * r * e * f;
	}
	
	template <typename Real>
	CONSTEXPRESSION posit<nbits, es, bt>& convert_ieee754(const Real& rhs) noexcept {
		// Direct IEEE-754 to posit conversion using frexp to extract scale and fraction.
		// This avoids blocktriple::round() which has an off-by-one for same-precision sources.
		if (rhs == Real(0)) { setzero(); return *this; }
		if (std::isnan(rhs) || std::isinf(rhs)) { setnar(); return *this; }

		bool s = (rhs < Real(0));
		int scale;
		Real frac = std::frexp(s ? -rhs : rhs, &scale);
		// frexp: |rhs| = frac * 2^scale where 0.5 <= frac < 1.0
		// convert to 1.xxx form: scale -= 1, frac *= 2 => 1.0 <= frac < 2.0
		--scale;
		frac *= 2;
		frac -= 1;  // remove hidden bit => 0.0 <= frac < 1.0

		// Extract enough fraction bits for convert_ rounding.
		// We must preserve all IEEE significand bits so that the sticky bit in
		// convert_() can detect perturbations deep in the fraction (e.g. 1e-6
		// eps at ~bit 20).  Using only nbits+4 loses this information and causes
		// midpoint ties where there should be none.
		// std::numeric_limits<Real>::digits includes the hidden bit (24 for float,
		// 53 for double), which is already removed above, so digits-1 fraction
		// bits suffice.  We take the max with nbits+4 for tiny-posit safety.
		constexpr unsigned ieeeBits = std::numeric_limits<Real>::digits;  // 24 float, 53 double
		constexpr unsigned extractBits = (ieeeBits > nbits + 4) ? ieeeBits : nbits + 4;
		blocksignificand<extractBits, bt> fracBits;
		for (unsigned i = 0; i < extractBits; ++i) {
			frac *= 2;
			if (frac >= Real(1)) {
				fracBits.setbit(extractBits - 1 - i, true);
				frac -= Real(1);
			}
		}

		return convert_<nbits, es, bt, extractBits>(s, scale, fracBits, *this);
	}

	// friend functions
	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const posit<nnbits, ees, bbt>& p);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend std::istream& operator>> (std::istream& istr, posit<nnbits, ees, bbt>& p);

	// posit - posit logic functions
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);

#if POSIT_ENABLE_LITERALS
	// posit - literal logic functions

	// posit - signed char
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, signed char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, signed char rhs);

	// posit - char
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, char rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, char rhs);

	// posit - short
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, short rhs);

	// posit - unsigned short
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, unsigned short rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, unsigned short rhs);

	// posit - int
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, int rhs);

	// posit - unsigned int
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, unsigned int rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, unsigned int rhs);

	// posit - long
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, long rhs);

	// posit - unsigned long long
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, unsigned long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, unsigned long rhs);

	// posit - long long
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, long long rhs);

	// posit - unsigned long long
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, unsigned long long rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, unsigned long long rhs);

	// posit - float
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, float rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, float rhs);

	// posit - double
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, double rhs);

#if LONG_DOUBLE_SUPPORT
	// posit - long double
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(const posit<nnbits, ees, bbt>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(const posit<nnbits, ees, bbt>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (const posit<nnbits, ees, bbt>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (const posit<nnbits, ees, bbt>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(const posit<nnbits, ees, bbt>& lhs, long double rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(const posit<nnbits, ees, bbt>& lhs, long double rhs);
#endif

	// literal - posit logic functions

	// signed char - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(signed char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(signed char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (signed char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (signed char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(signed char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(signed char lhs, const posit<nnbits, ees, bbt>& rhs);

	// char - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(char lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(char lhs, const posit<nnbits, ees, bbt>& rhs);

	// short - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(short lhs, const posit<nnbits, ees, bbt>& rhs);

	// unsigned short - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(unsigned short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(unsigned short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (unsigned short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (unsigned short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(unsigned short lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(unsigned short lhs, const posit<nnbits, ees, bbt>& rhs);

	// int - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(int lhs, const posit<nnbits, ees, bbt>& rhs);

	// unsigned int - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(unsigned int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(unsigned int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (unsigned int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (unsigned int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(unsigned int lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(unsigned int lhs, const posit<nnbits, ees, bbt>& rhs);

	// long - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(long lhs, const posit<nnbits, ees, bbt>& rhs);

	// unsigned long - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(unsigned long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(unsigned long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (unsigned long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (unsigned long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(unsigned long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(unsigned long lhs, const posit<nnbits, ees, bbt>& rhs);

	// long long - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(long long lhs, const posit<nnbits, ees, bbt>& rhs);

	// unsigned long long - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(unsigned long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(unsigned long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (unsigned long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (unsigned long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(unsigned long long lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(unsigned long long lhs, const posit<nnbits, ees, bbt>& rhs);

	// float - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(float lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(float lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (float lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (float lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(float lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(float lhs, const posit<nnbits, ees, bbt>& rhs);

	// double - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(double lhs, const posit<nnbits, ees, bbt>& rhs);

#if LONG_DOUBLE_SUPPORT
	// long double - posit
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator==(long double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator!=(long double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator< (long double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator> (long double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator<=(long double lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend bool operator>=(long double lhs, const posit<nnbits, ees, bbt>& rhs);
#endif

#endif // POSIT_ENABLE_LITERALS

};

////////////////// convenience/shim functions

template<unsigned nbits, unsigned es, typename bt>
inline bool isnar(const posit<nbits, es, bt>& p) {
	return p.isnar();
}
template<unsigned nbits, unsigned es, typename bt>
inline bool iszero(const posit<nbits, es, bt>& p) {
	return p.iszero();
}
template<unsigned nbits, unsigned es, typename bt>
inline bool ispos(const posit<nbits, es, bt>& p) {
	return p.ispos();
}
template<unsigned nbits, unsigned es, typename bt>
inline bool isneg(const posit<nbits, es, bt>& p) {
	return p.isneg();
}
template<unsigned nbits, unsigned es, typename bt>
inline bool isone(const posit<nbits, es, bt>& p) {
	return p.isone();
}		
template<unsigned nbits, unsigned es, typename bt>
inline bool isminusone(const posit<nbits, es, bt>& p) {
	return p.isminusone();
}
template<unsigned nbits, unsigned es, typename bt>
inline bool ispowerof2(const posit<nbits, es, bt>& p) {
	return p.ispowerof2();
}

////////////////// POSIT operators

// stream operators

// generate a posit format ASCII format nbits.esxNN...NNp
template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const posit<nbits, es, bt>& p) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the posit into a string
	std::stringstream ss;
#if POSIT_ERROR_FREE_IO_FORMAT
	ss << nbits << '.' << es << 'x' << to_hex(p.bits()) << 'p';
#else
	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
//	ss << std::showpos << std::setw(width) << std::setprecision(prec) << (long double)p;
	// TODO: how do you react to fmtflags being set, such as hexfloat or showpos?
	// it appears that the fmtflags are opaque and not a user-visible feature
	ss << std::setw(width) << std::setprecision(prec);
	ss << to_string(p, prec);  // TODO: we need a true native serialization function
#endif
	return ostr << ss.str();
}

// read an ASCII float or posit format: nbits.esxNN...NNp, for example: 32.2x80000000p
template<unsigned nbits, unsigned es, typename bt>
inline std::istream& operator>> (std::istream& istr, posit<nbits, es, bt>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

// generate a posit format ASCII format nbits.esxNN...NNp
template<unsigned nbits, unsigned es, typename bt>
inline std::string hex_format(const posit<nbits, es, bt>& p) {
	// we need to transform the posit into a string
	std::stringstream ss;
	ss << nbits << '.' << es << 'x' << to_hex(p.bits()) << 'p';
	return ss.str();
}

template<typename Float>
inline std::string hex_format(Float f) {
	std::stringstream ss;
	ss << std::hexfloat << std::setprecision(std::numeric_limits<Float>::digits10) << f;
	return ss.str();
}

// convert a posit value to a string using "nar" as designation of NaR
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const posit<nbits, es, bt>& p, std::streamsize precision = 17) {
	if (p.isnar()) {
		return std::string("nar");
	}
	std::stringstream ss;
	ss << std::setprecision(precision) << (long double)p;
	return ss.str();
}

// binary representation of a posit with delimiters: i.e. 0.10.00.000000 => sign.regime.exp.fraction
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_binary(const posit<nbits, es, bt>& number, bool nibbleMarker = false) {
	
	constexpr unsigned fbits = (es + 2ull >= nbits ? 0ull : nbits - 3ull - es);             // maximum number of fraction bits: derived
	bool negative{ false };
	positRegime<nbits, es, bt> r;
	positExponent<nbits, es, bt> e;
	positFraction<fbits, bt> f;
	auto raw = number.bits();
	extract_fields(raw, negative, r, e, f);

	std::stringstream s;
	s << (negative ? "0b1." : "0b0.");
	s << to_string(r, false, nibbleMarker) << "."
	  << to_string(e, false, nibbleMarker) << "."
	  << to_string(f, false, nibbleMarker);

	return s.str();
}

template<unsigned nbits, unsigned es, typename bt>
inline std::string to_triple(const posit<nbits, es, bt>& number, bool nibbleMarker = false) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);             // maximum number of fraction bits: derived
	bool s{ false };
	positRegime<nbits, es, bt> r;
	positExponent<nbits, es, bt> e;
	positFraction<fbits, bt> f;
	blockbinary<nbits, bt> raw = number.bits();
	std::stringstream ss;
	extract_fields(raw, s, r, e, f);

	ss << (s ? "(-, " : "(+, ");
	ss << scale(number) 
	   << ", "
	   << to_string(f, false, nibbleMarker)
	   << ')';

	return ss.str();
}

// numerical helpers

template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> ulp(const posit<nbits, es, bt>& a) {
	posit<nbits, es, bt> b(a);
	return ++b - a;
}

// binary exponent representation: i.e. 1.0101010e2^-37
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_base2_scientific(const posit<nbits, es, bt>& number) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);             // maximum number of fraction bits: derived
	bool s{ false };
	scale(number);
	positRegime<nbits, es, bt> r;
	positExponent<nbits, es, bt> e;
	positFraction<fbits, bt> f;
	blockbinary<nbits, bt> raw = number.bits();
	std::stringstream ss;
	extract_fields(raw, s, r, e, f);
	ss << (s ? "-" : "+") << "1." << to_string(f, true) << "e2^" << std::showpos << r.scale() + e.scale();
	return ss.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// posit - posit binary logic operators

template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return lhs._block == rhs._block;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(lhs._block, rhs._block);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return !operator< (lhs, rhs);
}

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator+(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> sum = lhs;
	return sum += rhs;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator-(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> diff = lhs;
	return diff -= rhs;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator*(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> mul = lhs;
	return mul *= rhs;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator/(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> ratio(lhs);
	return ratio /= rhs;
}

#if POSIT_ENABLE_LITERALS

// posit - signed char logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es>& lhs, signed char rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es>& lhs, signed char rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es>& lhs, signed char rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es>& lhs, signed char rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es>& lhs, signed char rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es>& lhs, signed char rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// signed char - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(signed char lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(signed char lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (signed char lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (signed char lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(signed char lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(signed char lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - char logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es>& lhs, char rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es>& lhs, char rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es>& lhs, char rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es>& lhs, char rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es>& lhs, char rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es>& lhs, char rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// char - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(char lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(char lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (char lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (char lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(char lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(char lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - short logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es>& lhs, short rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es>& lhs, short rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es>& lhs, short rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es>& lhs, short rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es>& lhs, short rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es>& lhs, short rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// short - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(short lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(short lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (short lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (short lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(short lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(short lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - unsigned short logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es>& lhs, unsigned short rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es>& lhs, unsigned short rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es>& lhs, unsigned short rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es>& lhs, unsigned short rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es>& lhs, unsigned short rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es>& lhs, unsigned short rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// unsigned short - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(unsigned short lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(unsigned short lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (unsigned short lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (unsigned short lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(unsigned short lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(unsigned short lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - int logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, int rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, int rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, int rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, int rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, int rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, int rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// int - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(int lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(int lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (int lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (int lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(int lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(int lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - unsigned int logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, unsigned int rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, unsigned int rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, unsigned int rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, unsigned int rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, unsigned int rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, unsigned int rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// unsigned int - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(unsigned int lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(unsigned int lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (unsigned int lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (unsigned int lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(unsigned int lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(unsigned int lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - long logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, long rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, long rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, long rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, long rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, long rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, long rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// long - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(long lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(long lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (long lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (long lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(long lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(long lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - unsigned long logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, unsigned long rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, unsigned long rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, unsigned long rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, unsigned long rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, unsigned long rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, unsigned long rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// unsigned long - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(unsigned long lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(unsigned long lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (unsigned long lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (unsigned long lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(unsigned long lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(unsigned long lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - unsigned long long logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, unsigned long long rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, unsigned long long rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, unsigned long long rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, unsigned long long rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, unsigned long long rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, unsigned long long rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// unsigned long long - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(unsigned long long lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(unsigned long long lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (unsigned long long lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (unsigned long long lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(unsigned long long lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(unsigned long long lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - long long logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, long long rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, long long rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, long long rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, long long rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, long long rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, long long rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// long long - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(long long lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(long long lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (long long lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (long long lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(long long lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(long long lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - float logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, float rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, float rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, float rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, float rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, float rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, float rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// float  - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(float lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(float lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (float lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (float lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(float lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(float lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

// posit - double logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, double rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, double rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, double rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, double rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, double rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, double rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// double  - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(double lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(double lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (double lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (double lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(double lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(double lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}

#if LONG_DOUBLE_SUPPORT
// posit - long double logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(const posit<nbits, es>& lhs, long double rhs) {
	return lhs == posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator!=(const posit<nbits, es>& lhs, long double rhs) {
	return !operator==(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator< (const posit<nbits, es>& lhs, long double rhs) {
	return lhs < posit<nbits, es>(rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator> (const posit<nbits, es>& lhs, long double rhs) {
	return operator< (posit<nbits, es>(rhs), lhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(const posit<nbits, es>& lhs, long double rhs) {
	return !operator>(lhs, posit<nbits, es>(rhs));
}
template<unsigned nbits, unsigned es>
inline bool operator>=(const posit<nbits, es>& lhs, long double rhs) {
	return !operator<(lhs, posit<nbits, es>(rhs));
}

// long double  - posit logic operators
template<unsigned nbits, unsigned es>
inline bool operator==(long double lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) == rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator!=(long double lhs, const posit<nbits, es>& rhs) {
	return !operator==(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator< (long double lhs, const posit<nbits, es>& rhs) {
	return posit<nbits, es>(lhs) < rhs;
}
template<unsigned nbits, unsigned es>
inline bool operator> (long double lhs, const posit<nbits, es>& rhs) {
	return operator< (posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator<=(long double lhs, const posit<nbits, es>& rhs) {
	return !operator>(posit<nbits, es>(lhs), rhs);
}
template<unsigned nbits, unsigned es>
inline bool operator>=(long double lhs, const posit<nbits, es>& rhs) {
	return !operator<(posit<nbits, es>(lhs), rhs);
}
#endif

// BINARY ADDITION
template<unsigned nbits, unsigned es>
inline posit<nbits, es> operator+(const posit<nbits, es>& lhs, double rhs) {
	posit<nbits, es> sum = lhs;
	sum += posit<nbits, es>(rhs);
	return sum;
}

// TODO: need to find a place in traits
// non-posit: native integer and floating point types
template <typename T>
constexpr bool is_intrinsic_numerical = std::is_integral<T>::value || std::is_floating_point<T>::value;

template <typename T, typename U = void>
using enable_intrinsic_numerical = std::enable_if_t<is_intrinsic_numerical<T>, U>;

// More generic alternative to avoid ambiguities with intrinsic +
template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posit<nbits, es> operator+(const posit<nbits, es>& lhs, Value rhs) {
	posit<nbits, es> sum = lhs;
	sum += posit<nbits, es>(rhs);
	return sum;
}

template<unsigned nbits, unsigned es>
inline posit<nbits, es> operator+(double lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> sum(lhs);
	sum += rhs;
	return sum;
}

// BINARY SUBTRACTION
template<unsigned nbits, unsigned es>
inline posit<nbits, es> operator-(double lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> diff(lhs);
	diff -= rhs;
	return diff;
}

// More generic alternative to avoid ambiguities with intrinsic +
template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posit<nbits, es> operator-(const posit<nbits, es>& lhs, Value rhs) {
	posit<nbits, es> diff = lhs;
	diff -= posit<nbits, es>(rhs);
	return diff;
}

template<unsigned nbits, unsigned es>
inline posit<nbits, es> operator-(const posit<nbits, es>& lhs, double rhs) {
	posit<nbits, es> diff(lhs);
	diff -= posit<nbits, es>(rhs);
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es>
inline posit<nbits, es> operator*(double lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> mul(lhs);
	mul *= rhs;
	return mul;
}

template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posit<nbits, es> operator*(Value lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> mul(lhs);
	mul *= rhs;
	return mul;
}
    
template<unsigned nbits, unsigned es>
inline posit<nbits, es> operator*(const posit<nbits, es>& lhs, double rhs) {
	posit<nbits, es> mul(lhs);
	mul *= posit<nbits, es>(rhs);
	return mul;
}

// BINARY DIVISION
template<unsigned nbits, unsigned es>
inline posit<nbits, es> operator/(double lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posit<nbits, es> operator/(Value lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es>
inline posit<nbits, es> operator/(const posit<nbits, es>& lhs, double rhs) {
	posit<nbits, es> ratio(lhs);
	ratio /= posit<nbits, es>(rhs);
	return ratio;
}

template<unsigned nbits, unsigned es, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posit<nbits, es> operator/(const posit<nbits, es>& lhs, Value rhs) {
	posit<nbits, es> ratio(lhs);
	ratio /= posit<nbits, es>(rhs);
	return ratio;
}

#endif // POSIT_ENABLE_LITERALS

// Magnitude of a posit (expensive as we are creating a new posit).
template<unsigned nbits, unsigned es> 
posit<nbits, es> abs(const posit<nbits, es>& p) {
	return p.abs();
}
template<unsigned nbits, unsigned es>
posit<nbits, es> fabs(const posit<nbits, es>& v) {
	posit<nbits, es> p(v);
	return p.abs();
}

// Atomic fused operators

// FMA: fused multiply-add:  a*b + c
template<unsigned nbits, unsigned es>
internal::value<1 + 2 * (nbits - es)> fma(const posit<nbits, es>& a, const posit<nbits, es>& b, const posit<nbits, es>& c) {
	constexpr unsigned fbits = nbits - 3 - es;
	constexpr unsigned fhbits = fbits + 1;      // size of fraction + hidden bit
	constexpr unsigned mbits = 2 * fhbits;      // size of the multiplier output
	constexpr unsigned abits = mbits + 4;       // size of the addend

	internal::value<mbits> product;
	internal::value<abits + 1> sum;
	internal::value<fbits> va, vb, ctmp;

	// special case handling of input arguments
	if (a.isnar() || b.isnar() || c.isnar()) {
		sum.setnan();
		return sum;
	}

	if (a.iszero() || b.iszero()) {  // product will only become non-zero if neither a and b are zero
		if (c.iszero()) {
			sum.setzero();
		}
		else {
			ctmp.set(sign(c), scale(c), extract_fraction<nbits, es, fbits>(c), c.iszero(), c.isnar());
			sum.template right_extend<fbits, abits + 1>(ctmp); // right-extend the c input argument and assign to sum
		}
	}
	else { // else clause guarantees that the product is non-zero	
		// first, the multiply: transform the inputs into (sign,scale,fraction) triples
		va.set(sign(a), scale(a), extract_fraction<nbits, es, fbits>(a), a.iszero(), a.isnar());;
		vb.set(sign(b), scale(b), extract_fraction<nbits, es, fbits>(b), b.iszero(), b.isnar());;

		module_multiply(va, vb, product);    // multiply the two inputs

		// second, the add : at this point we are guaranteed that product is non-zero and non-nar
		if (c.iszero()) {				
			sum.template right_extend<mbits, abits + 1>(product);   // right-extend the product and assign to sum
		}
		else {
			ctmp.set(sign(c), scale(c), extract_fraction<nbits, es, fbits>(c), c.iszero(), c.isnar());
			internal::value<mbits> vc;
			vc.template right_extend<fbits, mbits>(ctmp); // right-extend the c argument and assign to adder input
//			module_add<mbits, abits>(product, vc, sum);
		}
	}

	return sum;
}

// FAM: fused add-multiply: (a + b) * c
template<unsigned nbits, unsigned es>
internal::value<2 * (nbits - 2 - es)> fam(const posit<nbits, es>& a, const posit<nbits, es>& b, const posit<nbits, es>& c) {
	constexpr unsigned fbits = nbits - 3 - es;
	constexpr unsigned abits = fbits + 4;       // size of the addend
	constexpr unsigned fhbits = fbits + 1;      // size of fraction + hidden bit
	constexpr unsigned mbits = 2 * fhbits;      // size of the multiplier output

	internal::value<fbits> va, vb;
	internal::value<abits+1> sum, vc;
	internal::value<mbits> product;

	// special case
	if (c.iszero()) return product;

	// first the add
	if (!a.iszero() || !b.iszero()) {
		// transform the inputs into (sign,scale,fraction) triples
		va.set(sign(a), scale(a), extract_fraction<nbits, es, fbits>(a), a.iszero(), a.isnar());;
		vb.set(sign(b), scale(b), extract_fraction<nbits, es, fbits>(b), b.iszero(), b.isnar());;

		module_add(va, vb, sum);    // multiply the two inputs
		if (sum.iszero()) return product;  // product is still zero
	}
	// second, the multiply		
	vc.set(c.get_size(), scale(c), extract_fraction<nbits, es, fbits>(c), c.iszero(), c.isnar());
	module_multiply(sum, vc, product);
	return product;
}

// FMMA: fused multiply-multiply-add: (a * b) +/- (c * d)
template<unsigned nbits, unsigned es>
internal::value<nbits> fmma(const posit<nbits, es>& a, const posit<nbits, es>& b, const posit<nbits, es>& c, const posit<nbits, es>& d, bool opIsAdd = true)
{
	// todo: implement
	internal::value<nbits> result;
	return result;
}

// Standard posit short-hand types
/*
TODO: how do we use the same names as the posit C-types?
right now, because we pull in the C++ as run-time to the C functions this causes a redefinition error
using posit8_t   = posit<8, 0>;
using posit16_t  = posit<16, 1>;
using posit32_t  = posit<32, 2>;
using posit64_t  = posit<64, 3>;
using posit128_t = posit<128, 4>;
using posit256_t = posit<256, 5>;
*/


}} // namespace sw::universal


