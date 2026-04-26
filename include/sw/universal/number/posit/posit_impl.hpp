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
#include <universal/number/shared/specific_value_encoding.hpp>
// intermediate value tracing
#include <universal/number/algorithm/trace_constants.hpp>
// posit environment
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/number/posit/posit_fraction.hpp>
#include <universal/number/posit/posit_exponent.hpp>
#include <universal/number/posit/posit_regime.hpp>

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
template<unsigned nbits, unsigned es, typename bt>
constexpr bool check_inward_projection_range(int scale) {
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
constexpr int decode_regime(const blockbinary<nbits, bt, BinaryNumberType::Signed>& raw_bits) {
	// let m be the number of identical bits in the regime
	int m = 0;   // regime runlength counter
	int k = 0;   // converted regime scale
	// decode_regime expects the posit payload in magnitude order:
	// sign bit removed, and negative numbers already converted from two's complement
	// back to the canonical positive encoding. Once in that form, the first payload run
	// completely determines k: run of 1s => k = m - 1, run of 0s => k = -m.
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
constexpr void extract_fields(const blockbinary<nbits, bt, BinaryNumberType::Signed>& raw_bits, bool& _sign, positRegime<nbits, es, bt>& _regime, positExponent<nbits, es, bt>& _exponent, positFraction<fbits, bt>& _fraction) {
	using TwosComplementNumber = blockbinary<nbits, bt, BinaryNumberType::Signed>;
	// check special case: zero
	if (raw_bits.iszero()) {
		_sign = false;
		_regime.setzero();
		_exponent.setzero();
		_fraction.setzero();
		return;
	}
	// check special case: NaR (sign bit set, all other bits zero)
	_sign = raw_bits.test(nbits - 1);
	if (_sign) {
		TwosComplementNumber tmp(raw_bits);
		tmp.reset(nbits - 1);
		if (tmp.none()) {
			_regime.setinf();
			_exponent.setzero();
			_fraction.setzero();
			return;
		}
	}
	TwosComplementNumber tmp(raw_bits);
	// Negative posits are stored in two's complement, but regime/exponent/fraction are defined on the
	// unsigned magnitude pattern. Convert once up front so the remainder of the decoder can treat both
	// signs identically and reason only about the canonical posit field layout.
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
		// Fraction bits are copied into a fixed-width right-extended buffer so later code can evaluate
		// the hidden bit and rounding path without caring how many payload bits were actually available
		// after the regime consumed space in this particular posit.
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
constexpr void decode(const blockbinary<nbits, bt, BinaryNumberType::Signed>& raw_bits, bool& _sign, positRegime<nbits, es, bt>& _regime, positExponent<nbits, es, bt>& _exponent, positFraction<fbits, bt>& _fraction) {
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
	if constexpr (_trace_decode) std::cout << "raw bits: " << raw_bits << " posit bits: " << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << std::endl;
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
		exponent.setbits(esval);
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
constexpr inline posit<nbits, es, bt>& convert_(bool _sign, int _scale, const blocksignificand<fbits, bt>& fraction_in, posit<nbits, es, bt>& p) {
	if constexpr (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
	if constexpr (_trace_conversion) std::cout << "sign " << (_sign ? "-1 " : " 1 ") << "scale " << std::setw(3) << _scale << " fraction " << fraction_in << std::endl;

	p.clear();
	// construct the posit
	// interpolation rule checks
	if (check_inward_projection_range<nbits, es, bt>(_scale)) {    // regime dominated
		if constexpr (_trace_conversion) std::cout << "inward projection" << std::endl;
		// we are projecting to minpos/maxpos or minneg/maxneg
		int k = calculate_unconstrained_k<nbits, es>(_scale);
		k < 0 ? (_sign ? p.minneg() : p.minpos()) : (_sign ? p.maxneg() : p.maxpos());
		// we are done
		if constexpr (_trace_rounding) std::cout << "projection  rounding ";
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
constexpr inline posit<nbits, es, bt>& convert(const blocktriple<fbits, op, bt>& v, posit<nbits, es, bt>& p) {
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
	// Capture sticky information from blocktriple bits below the extracted range.
	// Without this, division (and other ops with wide significands) can lose
	// rounding-critical bits, causing systematic -1 ULP errors.
	int lowestExtracted = msbPos - static_cast<int>(extractBits);
	if (lowestExtracted > 0) {
		if (v.any(static_cast<unsigned>(lowestExtracted))) {
			frac.setbit(0, true); // fold remaining bits into sticky position
		}
	}
	return convert_<nbits, es, bt, extractBits>(v.sign(), realScale, frac, p);
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
	// Integer assignments route through convert_signed_integer / convert_unsigned_integer,
	// which are constexpr for nbits <= 64. The previous double-cast route through
	// convert_ieee754 was not actually constexpr because std::frexp is not constexpr.
	constexpr posit& operator=(signed char rhs)        noexcept { return convert_signed_integer(rhs); }
	constexpr posit& operator=(short rhs)              noexcept { return convert_signed_integer(rhs); }
	constexpr posit& operator=(int rhs)                noexcept { return convert_signed_integer(rhs); }
	constexpr posit& operator=(long rhs)               noexcept { return convert_signed_integer(rhs); }
	constexpr posit& operator=(long long rhs)          noexcept { return convert_signed_integer(rhs); }
	constexpr posit& operator=(char rhs)               noexcept {
		// Plain char is implementation-defined as either signed or unsigned;
		// dispatch to the matching conversion so negative values on signed-char
		// platforms (the common case) sign-extend correctly.
		if constexpr (std::is_signed_v<char>) {
			return convert_signed_integer(rhs);
		}
		else {
			return convert_unsigned_integer(static_cast<unsigned char>(rhs));
		}
	}
	constexpr posit& operator=(unsigned short rhs)     noexcept { return convert_unsigned_integer(rhs); }
	constexpr posit& operator=(unsigned int rhs)       noexcept { return convert_unsigned_integer(rhs); }
	constexpr posit& operator=(unsigned long rhs)      noexcept { return convert_unsigned_integer(rhs); }
	constexpr posit& operator=(unsigned long long rhs) noexcept { return convert_unsigned_integer(rhs); }
	CONSTEXPRESSION posit& operator=(float rhs) noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION posit& operator=(double rhs) noexcept { return convert_ieee754(rhs); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	CONSTEXPRESSION posit(long double initial_value)  noexcept : _block{ 0 } { *this = initial_value; }
	CONSTEXPRESSION posit& operator=(long double rhs) noexcept { return convert_ieee754(rhs); }
	// TODO: we need this regardless as the design marshalls values through long double
	// explicit operator long double() const noexcept { return to_native<long double>(); }
#else
	// On MSVC, long double and double are distinct types for overload resolution
	// even though they have identical representation.  Without these overloads
	// assigning a long double is ambiguous among float/double/integer overloads.
	CONSTEXPRESSION posit(long double initial_value)  noexcept : _block{ 0 } { *this = static_cast<double>(initial_value); }
	CONSTEXPRESSION posit& operator=(long double rhs) noexcept { return convert_ieee754(static_cast<double>(rhs)); }
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
	constexpr posit operator-() const {
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
	constexpr posit& operator++() {
		++_block;
		return *this;
	}
	// postfix increment operator
	constexpr posit operator++(int) {
		posit tmp(*this);
		operator++();
		return tmp;
	}
	// prefix decrement operator
	constexpr posit& operator--() {
		--_block;
		return *this;
	}
	// postfix decrement operator
	constexpr posit operator--(int) {
		posit tmp(*this);
		operator--();
		return tmp;
	}

	// we model a hw pipeline with register assignments, functional block, and conversion
	constexpr posit& operator+=(const posit& rhs) {
		if constexpr (_trace_add) std::cout << "---------------------- ADD -------------------" << std::endl;
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
	constexpr posit& operator+=(double rhs) {
		return *this += posit<nbits, es, bt>(rhs);
	}
	constexpr posit& operator-=(const posit& rhs) {
		return *this += (-rhs);
	}
	constexpr posit& operator-=(double rhs) {
		return *this -= posit<nbits, es, bt>(rhs);
	}
	constexpr posit& operator*=(const posit& rhs) {
		static_assert(fhbits > 0, "posit configuration does not support multiplication");
		if constexpr (_trace_mul) std::cout << "---------------------- MUL -------------------" << std::endl;
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
	constexpr posit& operator*=(double rhs) {
		return *this *= posit<nbits, es, bt>(rhs);
	}
	constexpr posit& operator/=(const posit& rhs) {
		if constexpr (_trace_div) std::cout << "---------------------- DIV -------------------" << std::endl;
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
	constexpr posit& operator/=(double rhs) {
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
		bool old_sign = _block.test(nbits-1);
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
	constexpr posit<nbits, es, bt>& setbit(unsigned bitIndex, bool value = true) noexcept {
		_block.setbit(bitIndex, value);
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

	// normalize: decompose posit value into a blocktriple<fbits, REP> for quire accumulation
	template<typename TargetBlockType = bt>
	constexpr void normalize(blocktriple<fbits, BlockTripleOperator::REP, TargetBlockType>& tgt) const noexcept {
		using namespace sw::universal::internal;
		if (iszero()) { tgt.setzero(); return; }
		if (isnar())  { tgt.setnan();  return; }
		bool               _sign{ false };
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(_block, _sign, _regime, _exponent, _fraction);
		tgt.setnormal();
		tgt.setsign(_sign);
		// REP normalization collapses the tapered posit encoding into a canonical 1.ffff blocktriple.
		// The variable-length regime and optional exponent are fully absorbed into scale; the significand
		// is rebuilt with an explicit hidden bit so quire/blocktriple consumers no longer need posit-specific logic.
		tgt.setscale(_regime.scale() + _exponent.scale());
		tgt.setbit(fbits);  // hidden bit
		auto frac = _fraction.bits();
		for (unsigned i = 0; i < fbits; ++i) {
			tgt.setbit(i, frac.at(i));
		}
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
			// no rounding shift needed for MUL -- blocktriple::mul handles radix placement
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
				tgt.setradix();
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
	
	// Encode a positive value (sign=0) into the posit bit pattern as a uint64_t.
	// Inputs:
	//   scale: binary scale of the value (value = 1.fraction * 2^scale). Can be negative.
	//   source_fraction: bits below the hidden 1, with bit (num_fbits-1) being the MSB.
	//   num_fbits: number of valid fraction bits in source_fraction (0 if no fraction).
	//   saturated: out parameter, true if we projected to maxpos/minpos.
	// All work is on uint64_t, so this is constexpr-clean for nbits <= 64.
	static constexpr uint64_t encode_positive_with_scale_and_fraction(
		int scale, uint64_t source_fraction, unsigned num_fbits, bool& saturated) noexcept {
		saturated = false;

		if (check_inward_projection_range<nbits, es, bt>(scale)) {
			saturated = true;
			constexpr unsigned tw = nbits - 1u;
			if (scale > 0) {
				// maxpos: 0 sign + (nbits-1) ones
				return (tw >= 64u) ? ~0ull : ((1ull << tw) - 1ull);
			}
			else {
				// minpos: 0 sign + 0...01
				return 1ull;
			}
		}

		constexpr unsigned target_width = nbits - 1u; // payload width (excludes sign)

		// Compute regime k and exponent field with Euclidean division so that
		// 0 <= e_int < 2^es regardless of the sign of scale.
		constexpr int es_pow = static_cast<int>(1u << es);
		int k = scale / es_pow;
		int e_int = scale % es_pow;
		if (e_int < 0) {
			e_int += es_pow;
			k -= 1;
		}
		uint64_t exp_field = static_cast<uint64_t>(e_int);

		// Encoding layout in target_width payload bits (MSB-first):
		//   k >= 0 (positive regime):  [k+1 ones][0 terminator][es exp bits][fraction]
		//   k <  0 (negative regime):  [-k zeros][1 terminator][es exp bits][fraction]
		// If the regime alone fills target_width, the terminator and lower fields are absent.

		uint64_t encoded = 0;
		unsigned regime_total = (k >= 0) ? static_cast<unsigned>(k + 2) : static_cast<unsigned>(-k + 1);
		unsigned regime_to_place = (regime_total <= target_width) ? regime_total : target_width;

		if (k >= 0) {
			// Place the ones; terminator zero is implicit. If we hit the boundary,
			// only the ones get placed (no terminator).
			unsigned ones_to_place = (regime_to_place < regime_total)
				? regime_to_place
				: (regime_to_place - 1u);
			if (ones_to_place > 0u) {
				encoded |= ((1ull << ones_to_place) - 1ull) << (target_width - ones_to_place);
			}
		}
		else {
			// Zeros are already there. Place the terminator '1' at position
			// (target_width - regime_total), only if the terminator fits.
			if (regime_to_place == regime_total) {
				encoded |= 1ull << (target_width - regime_total);
			}
			// If the regime overflows the boundary, the encoded payload stays all zeros.
		}

		unsigned bits_remaining = target_width - regime_to_place;

		// Place exponent field (top es bits, MSB-aligned in available space)
		unsigned exp_bits_to_place = (es <= bits_remaining) ? static_cast<unsigned>(es) : bits_remaining;
		if (exp_bits_to_place > 0u) {
			uint64_t exp_top = (es > exp_bits_to_place) ? (exp_field >> (es - exp_bits_to_place)) : exp_field;
			encoded |= exp_top << (bits_remaining - exp_bits_to_place);
		}
		bits_remaining -= exp_bits_to_place;

		// Place fraction (top num_fbits bits of source_fraction, MSB-aligned)
		unsigned frac_to_embed = (num_fbits <= bits_remaining) ? num_fbits : bits_remaining;
		if (frac_to_embed > 0u) {
			uint64_t frac_value = (num_fbits > frac_to_embed)
				? (source_fraction >> (num_fbits - frac_to_embed))
				: source_fraction;
			uint64_t frac_mask = (frac_to_embed >= 64u) ? ~0ull : ((1ull << frac_to_embed) - 1ull);
			frac_value &= frac_mask;
			encoded |= frac_value << (bits_remaining - frac_to_embed);
		}

		// Round-to-nearest-even on discarded bits.
		// Discarded sequence (MSB-first): low (es - exp_bits_to_place) bits of exp_field,
		// then low (num_fbits - frac_to_embed) bits of source_fraction.
		bool round_bit = false;
		bool sticky_bit = false;
		unsigned discarded_exp = static_cast<unsigned>(es) - exp_bits_to_place;
		unsigned discarded_frac = num_fbits - frac_to_embed;

		if (discarded_exp > 0u) {
			round_bit = ((exp_field >> (discarded_exp - 1u)) & 1ull) != 0ull;
			if (discarded_exp > 1u) {
				uint64_t low_exp_mask = (1ull << (discarded_exp - 1u)) - 1ull;
				if ((exp_field & low_exp_mask) != 0ull) sticky_bit = true;
			}
			if (num_fbits > 0u) {
				uint64_t fmask = (num_fbits >= 64u) ? ~0ull : ((1ull << num_fbits) - 1ull);
				if ((source_fraction & fmask) != 0ull) sticky_bit = true;
			}
		}
		else if (discarded_frac > 0u) {
			round_bit = ((source_fraction >> (discarded_frac - 1u)) & 1ull) != 0ull;
			if (discarded_frac > 1u) {
				uint64_t low_frac_mask = (1ull << (discarded_frac - 1u)) - 1ull;
				if ((source_fraction & low_frac_mask) != 0ull) sticky_bit = true;
			}
		}

		bool last_bit = (encoded & 1ull) != 0ull;
		if (round_bit && (sticky_bit || last_bit)) {
			++encoded;
			// Rounding could have overflowed past target_width into the sign bit.
			if ((encoded >> target_width) != 0ull) {
				saturated = true;
				return (target_width >= 64u) ? ~0ull : ((1ull << target_width) - 1ull);
			}
		}
		return encoded;
	}

	// Thin wrapper for positive integer inputs (scale = num_fbits).
	static constexpr uint64_t encode_positive_uint64(uint64_t v, bool& saturated) noexcept {
		saturated = false;
		if (v == 0ull) return 0ull;
		int scale = static_cast<int>(find_msb(v)) - 1;
		uint64_t fraction = (scale == 0) ? 0ull
			: ((scale >= 64) ? v : (v & ((1ull << scale) - 1ull)));
		return encode_positive_with_scale_and_fraction(
			scale, fraction, static_cast<unsigned>(scale), saturated);
	}

	// Constexpr integer-to-posit conversion for nbits <= 64.
	// Builds the posit encoding directly via uint64_t shift/mask, bypassing convert_<>()
	// (which is not constexpr because it relies on non-constexpr blockbinary operators).
	// For nbits > 64, the existing IEEE-754 path is used at runtime.
	template<typename Ty>
	constexpr posit<nbits, es, bt>& convert_unsigned_integer(Ty rhs) noexcept {
		clear();
		if (rhs == Ty(0)) return *this;

		if constexpr (nbits > 64) {
			return convert_ieee754(static_cast<double>(rhs));
		}
		else {
			bool saturated = false;
			uint64_t encoded = encode_positive_uint64(static_cast<uint64_t>(rhs), saturated);
			setbits(encoded);
			return *this;
		}
	}

	template<typename Ty>
	constexpr posit<nbits, es, bt>& convert_signed_integer(Ty rhs) noexcept {
		clear();
		if (rhs == Ty(0)) return *this;

		if constexpr (nbits > 64) {
			return convert_ieee754(static_cast<double>(rhs));
		}
		else {
			bool s = (rhs < Ty(0));
			using UnsignedTy = std::make_unsigned_t<Ty>;
			UnsignedTy abs_v = s
				? static_cast<UnsignedTy>(UnsignedTy(0) - static_cast<UnsignedTy>(rhs))
				: static_cast<UnsignedTy>(rhs);

			bool saturated = false;
			uint64_t encoded = encode_positive_uint64(static_cast<uint64_t>(abs_v), saturated);
			if (s) {
				// Posit negation = two's complement on the full nbits encoding.
				// Done in uint64_t to avoid the non-constexpr blockbinary::operator+= chain.
				if constexpr (nbits >= 64) {
					// Special case for nbits>=64 to avoid undefined behavior of shifting by 64.
					 encoded = ~encoded + 1ull;  // mask is ~0, so AND is no-op
				} else {
					constexpr uint64_t nbits_mask = (uint64_t(1) << nbits) - 1ull;
					encoded                       = (~encoded + 1ull) & nbits_mask;
				}
			}
			setbits(encoded);
			return *this;
		}
	}

	template <typename Real>
	BIT_CAST_CONSTEXPR posit<nbits, es, bt>& convert_ieee754(const Real& rhs) noexcept {
		// Direct IEEE-754 to posit conversion via bit-cast field extraction.
		// extractFields uses __builtin_bit_cast (constexpr on gcc/clang/MSVC),
		// avoiding std::frexp / std::isnan / std::isinf which are not constexpr
		// before C++26.
		bool s = false;
		uint64_t rawExp = 0;
		uint64_t rawFrac = 0;
		uint64_t bits = 0;
		extractFields(rhs, s, rawExp, rawFrac, bits);

		constexpr unsigned ieee_fbits = ieee754_parameter<Real>::fbits;
		constexpr int ieee_bias = ieee754_parameter<Real>::bias;

		// NaN / Inf: rawExp is all-ones. Posit has only one exception (NaR), so both map to it.
		if (rawExp == ieee754_parameter<Real>::eallset) {
			setnar();
			return *this;
		}
		// Zero: rawExp == 0 && rawFrac == 0 (both +0 and -0 map to posit zero).
		if (rawExp == 0 && rawFrac == 0) {
			setzero();
			return *this;
		}

		int scale;
		uint64_t source_fraction;
		if (rawExp == 0) {
			// Subnormal: value = rawFrac * 2^(1 - bias - fbits).
			// Normalize to 1.frac' * 2^scale by finding the leading 1 in rawFrac.
			// If find_msb returns p (1-indexed), the leading 1 is at bit (p-1):
			//   value = (1 + low/2^(p-1)) * 2^(p - bias - fbits)
			// so scale = p - bias - fbits, and the new fraction is the bits below
			// the leading 1, shifted up to occupy the top of an fbits-wide field.
			unsigned msb_one_indexed = find_msb(rawFrac);
			scale = static_cast<int>(msb_one_indexed) - ieee_bias - static_cast<int>(ieee_fbits);
			unsigned shift = ieee_fbits + 1u - msb_one_indexed;
			source_fraction = (rawFrac << shift) & ieee754_parameter<Real>::fmask;
		}
		else {
			// Normal: scale = unbiased exponent, fraction = rawFrac (already MSB-aligned at fbits-1).
			scale = static_cast<int>(rawExp) - ieee_bias;
			source_fraction = rawFrac;
		}

		if constexpr (nbits <= 64) {
			// Direct uint64_t encoding path (the same encoder used by integer construction).
			bool saturated = false;
			uint64_t encoded = encode_positive_with_scale_and_fraction(
				scale, source_fraction, ieee_fbits, saturated);
			if (s) {
				// Posit negation = two's complement on the full nbits encoding.
				if constexpr (nbits == 64u) {
					encoded = ~encoded + 1ull;  // mask would be ~0 -> AND is a no-op
				} else {
					constexpr uint64_t nbits_mask = (uint64_t(1) << nbits) - 1ull;
					encoded                       = (~encoded + 1ull) & nbits_mask;
				}

			}
			setbits(encoded);
			return *this;
		}
		else {
			// nbits > 64: route through convert_<>() with a blocksignificand.
			// convert_<>() is constexpr (PR 716) and handles wide blockbinary arithmetic.
			constexpr unsigned ieeeBits = ieee_fbits + 1u; // hidden bit + fraction bits
			constexpr unsigned extractBits = (ieeeBits > nbits + 4u) ? ieeeBits : (nbits + 4u);
			blocksignificand<extractBits, bt> fracBits;
			// Place source_fraction's top ieee_fbits bits at the top of fracBits.
			for (unsigned i = 0; i < ieee_fbits; ++i) {
				if ((source_fraction >> (ieee_fbits - 1u - i)) & 1ull) {
					fracBits.setbit(extractBits - 1u - i, true);
				}
			}
			return convert_<nbits, es, bt, extractBits>(s, scale, fracBits, *this);
		}
	}

	// friend functions
	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const posit<nnbits, ees, bbt>& p);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend std::istream& operator>> (std::istream& istr, posit<nnbits, ees, bbt>& p);

	// posit - posit logic functions (constexpr to match implementations)
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend constexpr bool operator==(const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend constexpr bool operator!=(const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend constexpr bool operator< (const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend constexpr bool operator> (const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend constexpr bool operator<=(const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);
	template<unsigned nnbits, unsigned ees, typename bbt>
	friend constexpr bool operator>=(const posit<nnbits, ees, bbt>& lhs, const posit<nnbits, ees, bbt>& rhs);

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
#if POSIT_ERROR_FREE_IO_FORMAT
	std::stringstream ss;
	ss << nbits << '.' << es << 'x' << to_hex(p.bits()) << 'p';
	return ostr << ss.str();
#else
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	char fillChar = ostr.fill();
	bool bShowpos    = fmt & std::ios_base::showpos;
	bool bUppercase  = fmt & std::ios_base::uppercase;
	bool bFixed      = fmt & std::ios_base::fixed;
	bool bScientific = fmt & std::ios_base::scientific;
	bool bInternal   = fmt & std::ios_base::internal;
	bool bLeft       = fmt & std::ios_base::left;

	if (p.isnar()) {
		std::string s = bUppercase ? "NAR" : "nar";
		if (width > 0 && s.length() < static_cast<size_t>(width)) {
			size_t pad = static_cast<size_t>(width) - s.length();
			if (bLeft) { s.append(pad, fillChar); }
			else { s.insert(static_cast<std::string::size_type>(0), pad, fillChar); }
		}
		return ostr << s;
	}

	constexpr unsigned pfbits = posit<nbits, es, bt>::fbits;
	if constexpr (pfbits == 0) {
		// degenerate posit with no fraction bits: format via double
		std::ostringstream oss;
		oss.precision(prec);
		if (bFixed) oss << std::fixed;
		if (bScientific) oss << std::scientific;
		if (bUppercase) oss << std::uppercase;
		if (bShowpos) oss << std::showpos;
		oss << static_cast<double>(p);
		std::string s = oss.str();
		if (width > 0 && s.length() < static_cast<size_t>(width)) {
			size_t pad = static_cast<size_t>(width) - s.length();
			if (bInternal) {
				bool hasSign = !s.empty() && (s[0] == '-' || s[0] == '+');
				s.insert(hasSign ? 1u : 0u, pad, fillChar);
			} else if (bLeft) { s.append(pad, fillChar); }
			else { s.insert(0u, pad, fillChar); }
		}
		return ostr << s;
	} else {
		auto v = p.template to_value<BlockTripleOperator::REP>();
		return ostr << v.to_string(prec, width, bFixed, bScientific,
		                            bInternal, bLeft, bShowpos, bUppercase, fillChar);
	}
#endif
}

// parse a posit from a string in either posit hex format (nbits.esxHEXVALUEp)
// or a decimal floating-point representation
template<unsigned nbits, unsigned es, typename bt>
bool parse(const std::string& txt, posit<nbits, es, bt>& p) {
	// check if the txt is of the native posit form: nbits.esXhexvalue
	std::regex posit_regex(R"(^[0-9]+\.[0-9]+[xX][0-9A-Fa-f]+p?$)");
	if (std::regex_match(txt, posit_regex)) {
		// found a posit representation: parse nbits.esxHEXVALUEp
		std::string nbitsStr, esStr, bitStr;
		auto it = txt.begin();
		for (; it != txt.end(); ++it) {
			if (*it == '.') break;
			nbitsStr.append(1, *it);
		}
		for (++it; it != txt.end(); ++it) {
			if (*it == 'x' || *it == 'X') break;
			esStr.append(1, *it);
		}
		for (++it; it != txt.end(); ++it) {
			if (*it == 'p') break;
			bitStr.append(1, *it);
		}
		unsigned nbits_in = 0;
		unsigned es_in = 0;
		{
			std::istringstream ss(nbitsStr);
			ss >> nbits_in;
			if (ss.fail()) return false;
		}
		{
			std::istringstream ss(esStr);
			ss >> es_in;
			if (ss.fail()) return false;
		}
		// native posit form must match target configuration
		if (nbits_in != nbits || es_in != es) return false;
		uint64_t raw = 0;
		std::istringstream ss(bitStr);
		ss >> std::hex >> raw;
		if (ss.fail()) return false;
		ss >> std::ws;
		if (!ss.eof()) return false;
		p.setbits(raw);
		return true;
	}
	else {
		// assume it is a float/double/long double representation
		std::istringstream ss(txt);
		double d;
		ss >> d;
		if (ss.fail()) return false;
		ss >> std::ws;
		if (!ss.eof()) return false;
		p = d;
		return true;
	}
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
	if (p.isnar()) return std::string("nar");
	constexpr unsigned pfbits = posit<nbits, es, bt>::fbits;
	if constexpr (pfbits == 0) {
		std::ostringstream oss;
		oss << std::setprecision(precision) << static_cast<double>(p);
		return oss.str();
	} else {
		auto v = p.template to_value<BlockTripleOperator::REP>();
		return v.to_string(precision, 0, false, true, false, false, false, false, ' ');
	}
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

// native semantic representation: radix-2, delegates to to_binary
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_native(const posit<nbits, es, bt>& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
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

	if (number.iszero()) {
		ss << "(+, 0, ~)";
	}
	else if (number.isnar()) {
		ss << "(nar)";
	}
	else {
		ss << (s ? "(-, " : "(+, ");
		ss << r.scale() + e.scale()
		   << ", "
		   << to_string(f, false, nibbleMarker)
		   << ')';
	}

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
constexpr inline bool operator==(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return lhs._block == rhs._block;
}
template<unsigned nbits, unsigned es, typename bt>
constexpr inline bool operator!=(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nbits, unsigned es, typename bt>
constexpr inline bool operator< (const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(lhs._block, rhs._block);
}
template<unsigned nbits, unsigned es, typename bt>
constexpr inline bool operator> (const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nbits, unsigned es, typename bt>
constexpr inline bool operator<=(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nbits, unsigned es, typename bt>
constexpr inline bool operator>=(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	return !operator< (lhs, rhs);
}

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es, typename bt>
constexpr inline posit<nbits, es, bt> operator+(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> sum = lhs;
	return sum += rhs;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es, typename bt>
constexpr inline posit<nbits, es, bt> operator-(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> diff = lhs;
	return diff -= rhs;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es, typename bt>
constexpr inline posit<nbits, es, bt> operator*(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> mul = lhs;
	return mul *= rhs;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es, typename bt>
constexpr inline posit<nbits, es, bt> operator/(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> ratio(lhs);
	return ratio /= rhs;
}

#if POSIT_ENABLE_LITERALS

// posit - signed char logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, signed char rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, signed char rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, signed char rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, signed char rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, signed char rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, signed char rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// signed char - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(signed char lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(signed char lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(signed char lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(signed char lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(signed char lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(signed char lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - char logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, char rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, char rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, char rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, char rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, char rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, char rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// char - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(char lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(char lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(char lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(char lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(char lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(char lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - short logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, short rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, short rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, short rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, short rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, short rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, short rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// short - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(short lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(short lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (short lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (short lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(short lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(short lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - unsigned short logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, unsigned short rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, unsigned short rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, unsigned short rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, unsigned short rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, unsigned short rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, unsigned short rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// unsigned short - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(unsigned short lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(unsigned short lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(unsigned short lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(unsigned short lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(unsigned short lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(unsigned short lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - int logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, int rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, int rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, int rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, int rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, int rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, int rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// int - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(int lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(int lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(int lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(int lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(int lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(int lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - unsigned int logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, unsigned int rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, unsigned int rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, unsigned int rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, unsigned int rhs) {
	return operator<(posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, unsigned int rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, unsigned int rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// unsigned int - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(unsigned int lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(unsigned int lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(unsigned int lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(unsigned int lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(unsigned int lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(unsigned int lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - long logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, long rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, long rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, long rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, long rhs) {
	return operator<(posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, long rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, long rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// long - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(long lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(long lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(long lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - unsigned long logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, unsigned long rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, unsigned long rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, unsigned long rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, unsigned long rhs) {
	return operator<(posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, unsigned long rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, unsigned long rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// unsigned long - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(unsigned long lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(unsigned long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(unsigned long lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(unsigned long lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(unsigned long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(unsigned long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - unsigned long long logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, unsigned long long rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, unsigned long long rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es, bt>& lhs, unsigned long long rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es, bt>& lhs, unsigned long long rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, unsigned long long rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, unsigned long long rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// unsigned long long - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(unsigned long long lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(unsigned long long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (unsigned long long lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (unsigned long long lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(unsigned long long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(unsigned long long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - long long logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, long long rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, long long rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es, bt>& lhs, long long rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es, bt>& lhs, long long rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, long long rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, long long rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// long long - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(long long lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(long long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (long long lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (long long lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(long long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(long long lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - float logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, float rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, float rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es, bt>& lhs, float rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es, bt>& lhs, float rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, float rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, float rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// float  - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(float lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(float lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (float lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (float lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(float lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(float lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

// posit - double logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, double rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, double rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const posit<nbits, es, bt>& lhs, double rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const posit<nbits, es, bt>& lhs, double rhs) {
	return operator< (posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, double rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, double rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// double  - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(double lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(double lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (double lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (double lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(double lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(double lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}

#if LONG_DOUBLE_SUPPORT
// posit - long double logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const posit<nbits, es, bt>& lhs, long double rhs) {
	return lhs == posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const posit<nbits, es, bt>& lhs, long double rhs) {
	return !operator==(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(const posit<nbits, es, bt>& lhs, long double rhs) {
	return lhs < posit<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(const posit<nbits, es, bt>& lhs, long double rhs) {
	return operator<(posit<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const posit<nbits, es, bt>& lhs, long double rhs) {
	return !operator>(lhs, posit<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const posit<nbits, es, bt>& lhs, long double rhs) {
	return !operator<(lhs, posit<nbits, es, bt>(rhs));
}

// long double  - posit logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(long double lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(long double lhs, const posit<nbits, es, bt>& rhs) {
	return !operator==(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<(long double lhs, const posit<nbits, es, bt>& rhs) {
	return posit<nbits, es, bt>(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>(long double lhs, const posit<nbits, es, bt>& rhs) {
	return operator<(rhs, posit<nbits, es, bt>(lhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(long double lhs, const posit<nbits, es, bt>& rhs) {
	return !operator>(posit<nbits, es, bt>(lhs), rhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(long double lhs, const posit<nbits, es, bt>& rhs) {
	return !operator<(posit<nbits, es, bt>(lhs), rhs);
}
#endif

// BINARY ADDITION
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator+(const posit<nbits, es, bt>& lhs, double rhs) {
	posit<nbits, es, bt> sum = lhs;
	sum += posit<nbits, es, bt>(rhs);
	return sum;
}

// TODO: need to find a place in traits
// non-posit: native integer and floating point types
template <typename T>
constexpr bool is_intrinsic_numerical = std::is_integral<T>::value || std::is_floating_point<T>::value;

template <typename T, typename U = void>
using enable_intrinsic_numerical = std::enable_if_t<is_intrinsic_numerical<T>, U>;

// More generic alternative to avoid ambiguities with intrinsic +
template<unsigned nbits, unsigned es, typename bt, typename Value, typename = enable_intrinsic_numerical<Value> >
inline posit<nbits, es, bt> operator+(const posit<nbits, es, bt>& lhs, Value rhs) {
	posit<nbits, es, bt> sum = lhs;
	sum += posit<nbits, es, bt>(rhs);
	return sum;
}

template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator+(double lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> sum(lhs);
	sum += rhs;
	return sum;
}

// BINARY SUBTRACTION
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator-(double lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> diff(lhs);
	diff -= rhs;
	return diff;
}

// More generic alternative to avoid ambiguities with intrinsic +
template<unsigned nbits, unsigned es, typename bt, typename Value, typename = enable_intrinsic_numerical<Value>>
inline posit<nbits, es, bt> operator-(const posit<nbits, es, bt>& lhs, Value rhs) {
	posit<nbits, es, bt> diff = lhs;
	diff -= posit<nbits, es, bt>(rhs);
	return diff;
}

template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator-(const posit<nbits, es, bt>& lhs, double rhs) {
	posit<nbits, es, bt> diff(lhs);
	diff -= posit<nbits, es, bt>(rhs);
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator*(double lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> mul(lhs);
	mul *= rhs;
	return mul;
}

template<unsigned nbits, unsigned es, typename bt, typename Value, typename = enable_intrinsic_numerical<Value>>
inline posit<nbits, es, bt> operator*(Value lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
    
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator*(const posit<nbits, es, bt>& lhs, double rhs) {
	posit<nbits, es, bt> mul(lhs);
	mul *= posit<nbits, es, bt>(rhs);
	return mul;
}

// BINARY DIVISION
template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator/(double lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, typename Value, typename = enable_intrinsic_numerical<Value>>
inline posit<nbits, es, bt> operator/(Value lhs, const posit<nbits, es, bt>& rhs) {
	posit<nbits, es, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt>
inline posit<nbits, es, bt> operator/(const posit<nbits, es, bt>& lhs, double rhs) {
	posit<nbits, es, bt> ratio(lhs);
	ratio /= posit<nbits, es, bt>(rhs);
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, typename Value, typename = enable_intrinsic_numerical<Value>>
inline posit<nbits, es, bt> operator/(const posit<nbits, es, bt>& lhs, Value rhs) {
	posit<nbits, es, bt> ratio(lhs);
	ratio /= posit<nbits, es, bt>(rhs);
	return ratio;
}

#endif // POSIT_ENABLE_LITERALS

// Magnitude of a posit (expensive as we are creating a new posit).
template<unsigned nbits, unsigned es, typename bt> 
posit<nbits, es, bt> abs(const posit<nbits, es, bt>& p) {
	return p.abs();
}
template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fabs(const posit<nbits, es, bt>& v) {
	posit<nbits, es, bt> p(v);
	return p.abs();
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

