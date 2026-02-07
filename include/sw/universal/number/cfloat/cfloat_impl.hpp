#pragma once
// cfloat_impl.hpp: implementation of an arbitrary configuration fixed-size 'classic' floating-point representation
// cfloat<> can emulate IEEE-754 floats and the new Deep Learning types, such as 
// IEEE-754 half-precision floats
// Google bfloat16
// NVIDIA TensorFloat 
// AMD FP16 and FP32
// Microsoft FP8 and FP9
// Tesla CFP8, CFP16
// 
// cfloat<> can also emulate more precise configurations, such as
// 80bit IEEE-754 extended precision floats
// true 128bit quad precision floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// compiler specific environment has been delegated to be handled by the
// number system include file <universal/number/cfloat/cfloat.hpp>
// 
// supporting types and functions
#include <limits>
#include <type_traits>
#include <universal/native/ieee754.hpp>
#include <universal/native/subnormal.hpp>
#include <universal/utility/find_msb.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
// arithmetic tracing options
#include <universal/number/algorithm/trace_constants.hpp>
// cfloat exception structure
#include <universal/number/cfloat/exceptions.hpp>
// composition types used by cfloat
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/support/decimal.hpp>

#ifndef CFLOAT_THROW_ARITHMETIC_EXCEPTION
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#endif

#ifndef TRACE_CONVERSION
#define TRACE_CONVERSION 0
#endif

namespace sw { namespace universal {

/*
 * classic floating-point cfloat offers denorms, gradual overflow, and 
 * saturation. The default configuration turns off denorms, supernorms,
 * and project values outside of their dynamic range to +-inf
 * 
 * Behavior flags
 *   subnormals  : gradual underflow: use all fraction encodings when exponent is all 0's
 *   supernormals: gradual overflow: use all fraction encodings when exponent is all 1's
 *   saturation to maxneg or maxpos when value is out of dynamic range
 */

/// <summary>
/// decode an cfloat value into its constituent parts
/// </summary>
/// <typeparam name="bt">block type</typeparam>
/// <param name="v">cfloat value to decode (input: const ref)</param>
/// <param name="s">sign (output: bool ref)</param>
/// <param name="e">exponent (output: blockbinary ref)</param>
/// <param name="f">fraction (output: blockbinary ref)</param>
template<unsigned nbits, unsigned es, unsigned fbitsPlus1, typename bt,
	bool hasSubnormals, bool hasSupernormals, bool isSaturating>
void decode(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v, bool& s, blockbinary<es, bt>& e, blockbinary<fbitsPlus1, bt>& f) {
	v.sign(s);
	v.exponent(e);
	v.fraction(f);
}

/// <summary>
/// return the binary scale of the given number
/// </summary>
/// <typeparam name="bt">Block type used for storage: derived through ADL</typeparam>
/// <param name="v">the cfloat number for which we seek to know the binary scale</param>
/// <returns>binary scale, i.e. 2^scale, of the value of the cfloat</returns>
template<unsigned nbits, unsigned es, typename bt,
	bool hasSubnormals, bool hasSupernormals, bool isSaturating>
int scale(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
	return v.scale();
}

/// <summary>
/// convert a blocktriple to a cfloat. blocktriples come out of the arithmetic
/// engine in the form ii.ff...ff and a scale. The conversion must take this
/// denormalized form into account during conversion.
/// 
/// The blocktriple must be in this form to round correctly, as all the bits
/// after an arithmetic operation must be taken into account.
/// 
/// Transformation:
///    ii.ff...ff  transform to    s.eee.fffff
/// All number systems that depend on blocktriple will need to have
/// the rounding decision answered, so that functionality can be
/// reused if we locate it inside blocktriple.
/// 
/// if (srcbits > fbits) // we need to round
///     if (ii.00..00 > 1) 
///         mask is at srcbits - fbits + 1
///     else 
///		    mask is at srcbits - fbits
/// }
/// else {               // no need to round
/// }
/// </summary>
/// <typeparam name="bt">type of the block used for cfloat storage</typeparam>
/// <param name="src">the blocktriple to be converted</param>
/// <param name="tgt">the resulting cfloat</param>
template<unsigned srcbits, BlockTripleOperator op, unsigned nbits, unsigned es, typename bt,
	bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline /*constexpr*/ void convert(const blocktriple<srcbits, op, bt>& src, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& tgt) {
	using btType = blocktriple<srcbits, op, bt>;
	using cfloatType = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	// test special cases
	if (src.isnan()) {
		tgt.setnan(src.sign() ? NAN_TYPE_SIGNALLING : NAN_TYPE_QUIET);
	}
	else	if (src.isinf()) {
		tgt.setinf(src.sign());
	}
	else 	if (src.iszero()) {
		tgt.setzero();
		tgt.setsign(src.sign()); // preserve sign
	}
	else {
		int significandScale = src.significandscale();
		int exponent = src.scale() + significandScale;
		// special case of underflow
		if constexpr (hasSubnormals) {
//			std::cout << "exponent = " << exponent << " bias = " << cfloatType::EXP_BIAS << " exp subnormal = " << cfloatType::MIN_EXP_SUBNORMAL << '\n';
			// why must exponent be less than (minExpSubnormal - 1) to be rounded to zero? 
			// because the half-way value that would round up to minpos is at exp = (minExpSubnormal - 1)
			if (exponent < cfloatType::MIN_EXP_SUBNORMAL) {
				tgt.setzero();
				if (exponent == (cfloatType::MIN_EXP_SUBNORMAL - 1)) {
					// -exponent because we are right shifting and exponent in this range is negative
					int adjustment = -(exponent + subnormal_reciprocal_shift[es]);
					std::pair<bool, unsigned> alignment = src.roundingDecision(adjustment);
					if (alignment.first) ++tgt; // we are minpos
				}
				tgt.setsign(src.sign());
				return;
			}
		}
		else {
			if (exponent + cfloatType::EXP_BIAS <= 0) {  // value is in the subnormal range, which maps to 0
				tgt.setzero();
				tgt.setsign(src.sign());
				return;
			}
		}
		// special case of overflow
		if constexpr (hasSupernormals) {
			if constexpr (isSaturating) {
				if (exponent > cfloatType::MAX_EXP) {
					if (src.sign()) tgt.maxneg(); else tgt.maxpos();
					return;
				}
			}
			else {
				if (exponent > cfloatType::MAX_EXP) {
					tgt.setinf(src.sign());
					return;
				}
			}
		}
		else {  // no supernormals will saturate at a different encoding: TODO can we hide it all in maxpos?
			if constexpr (isSaturating) {
				if (exponent > cfloatType::MAX_EXP) {
					if (src.sign()) tgt.maxneg(); else tgt.maxpos();
					return;
				}
			}
			else {
				if (exponent > cfloatType::MAX_EXP) {
					tgt.setinf(src.sign());
					return;
				}
			}
		}

		// our value needs to go through rounding to be correctly interpreted
		// 
		// tgt.clear();  // no need as all bits are going to be set by the code below

		// exponent construction
		int adjustment{ 0 };
		// construct exponent
		uint64_t biasedExponent = static_cast<uint64_t>(static_cast<long long>(exponent) + static_cast<long long>(cfloatType::EXP_BIAS)); // this is guaranteed to be positive if exponent in encoding range
//			std::cout << "exponent         " << to_binary(biasedExponent) << '\n';	
		if constexpr (hasSubnormals) {
			//if (exponent >= cfloatType::MIN_EXP_SUBNORMAL && exponent < cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::MIN_EXP_NORMAL) {
			if (exponent < cfloatType::MIN_EXP_NORMAL) {
				// the value is in the subnormal range of the cfloat
				biasedExponent = 0;
				// -exponent because we are right shifting and exponent in this range is negative
				adjustment = -(exponent + subnormal_reciprocal_shift[es]);
				// this is the right shift adjustment required for subnormal representation due 
				// to the scale of the input number, i.e. the exponent of 2^-adjustment
			}
			else {
				// the value is in the normal range of the cfloat
				biasedExponent = static_cast<uint64_t>(static_cast<long long>(exponent) + static_cast<long long>(cfloatType::EXP_BIAS)); // this is guaranteed to be positive
			}
		}
		else {
			if (exponent < cfloatType::MIN_EXP_NORMAL) biasedExponent = 1ull; // fixup biasedExponent if we are in the subnormal region
		}


		// get the rounding direction and the LSB right shift: 
		std::pair<bool, unsigned> alignment = src.roundingDecision(adjustment);
		unsigned rightShift = alignment.second;  // this is the shift to get the LSB of the src to the LSB of the tgt
		//std::cout << "rightShift       " << rightShift << '\n';

		if constexpr (btType::bfbits < 65) {
			bool roundup = alignment.first;
			//std::cout << "round-up?        " << (roundup ? "yes" : "no") << '\n';
			// we can use a uint64_t to construct the cfloat
			uint64_t raw = (src.sign() ? 1ull : 0ull); // process sign
			//std::cout << "raw bits (sign)  " << to_binary(raw) << '\n';
			// construct the fraction bits
			uint64_t fracbits = src.significand_ull(); // get all the bits, including the integer bits
			//std::cout << "fracbits         " << to_binary(fracbits) << '\n';
			fracbits >>= rightShift;
			//std::cout << "fracbits shifted " << to_binary(fracbits) << '\n';
			fracbits &= cfloatType::ALL_ONES_FR; // remove the hidden bit
			//std::cout << "fracbits masked  " << to_binary(fracbits) << '\n';
			if (roundup) ++fracbits;
			if (fracbits == (1ull << cfloatType::fbits)) { // check for overflow
				if (biasedExponent == cfloatType::ALL_ONES_ES) {
					fracbits = cfloatType::INF_ENCODING; // project to INF
				}
				else {
					++biasedExponent;
					fracbits = 0;
				}
			}

			raw <<= es; // shift sign to make room for the exponent bits
			raw |= biasedExponent;
			//std::cout << "raw bits (exp)   " << to_binary(raw) << '\n';
			raw <<= cfloatType::fbits; // make room for the fraction bits
			//std::cout << "raw bits (s+exp) " << to_binary(raw) << '\n';
			raw |= fracbits;
			//std::cout << "raw bits (final) " << to_binary(raw) << '\n';
			tgt.setbits(raw);
//			std::cout << "raw bits (all)   " << to_binary(raw) << '\n';
			if constexpr (isSaturating) {
				if (tgt.isnan()) {
					if (src.sign()) {
						tgt.maxneg();	// map back to maxneg
					}
					else {
						tgt.maxpos();	// map back to maxpos
					}
				}
			}
			else {
				// when you get too far, map it back to +-inf: 
				// TBD: this doesn't appear to be the right algorithm to catch all overflow patterns
				if (tgt.isnan()) tgt.setinf(src.sign());	// map back to +-inf
			}
		}
		else {
			// compose the segments
			auto fracbits = src.significand();  // why significand? cheesy optimization: we are going to overwrite the hidden bit position anyway when we write the exponent below, so no need to pay the overhead of generating the fraction here.
			//std::cout << "fraction      : " << to_binary(fracbits, true) << '\n';
			fracbits >>= static_cast<int>(rightShift);
			//std::cout << "aligned fbits : " << to_binary(fracbits, true) << '\n';

			// copy the blocks that contain fraction bits
			// significand blocks are organized like this:
			//   ADD        iii.ffffrrrrrrrrr          3 integer bits, f fraction bits, and 2*fhbits rounding bits
			//   MUL         ii.ffff'ffff              2 integer bits, 2*f fraction bits
			//   DIV         ii.ffff'ffff'ffff'rrrr    2 integer bits, 3*f fraction bits, and r rounding bits
			//std::cout << "fraction bits : " << to_binary(fracbits, true) << '\n';
			tgt.clear();
			//std::cout << "initial state : " << to_binary(tgt) << " : " << tgt << '\n';
			for (unsigned b = 0; b < btType::nrBlocks; ++b) {
				tgt.setblock(b, fracbits.block(b));
			}
			//std::cout << "fraction bits : " << to_binary(tgt, true) << '\n';
			tgt.setsign(src.sign());
			//std::cout << "adding sign   : " << to_binary(tgt) << '\n';
			if (!tgt.setexponent(exponent)) {
				std::cerr << "exponent value is out of range: " << exponent << '\n';
			}
			//std::cout << "add exponent  : " << to_binary(tgt) << '\n';
		}
	}
}


/// <summary>
/// An arbitrary, fixed-size floating-point number with configurable gradual under/overflow and saturation/non-saturation arithmetic.
/// Default configuration offers normal encoding and non-saturating arithmetic.
/// /// </summary>
/// <typeparam name="nbits">number of bits in the encoding</typeparam>
/// <typeparam name="es">number of exponent bits in the encoding</typeparam>
/// <typeparam name="bt">the type to use as storage class: one of [uint8_t|uint16_t|uint32_t]</typeparam>
/// <typeparam name="hasSubnormals">configure gradual underflow (==subnormals)</typeparam>
/// <typeparam name="hasSupernormals">configure gradual overflow (==supernormals)</typeparam>
/// <typeparam name="isSaturating">configure saturation arithmetic</typeparam>
template<unsigned _nbits, unsigned _es, typename bt = uint8_t,
	bool _hasSubnormals = false, bool _hasSupernormals = false, bool _isSaturating = false>
class cfloat {
public:
	static_assert(_nbits > _es + 1ull, "nbits is too small to accomodate the requested number of exponent bits");
	static_assert(_es < 21ull, "my God that is a big number, are you trying to break the Interweb?");
	static_assert(_es > 0, "number of exponent bits must be bigger than 0 to be a classic floating point number");
	// how do you assert on the condition that if es == 1 then subnormals and supernormals must be true?
	static constexpr bool     subsuper = (_hasSubnormals && _hasSupernormals);
	static constexpr bool     special = (subsuper ? true : (_es > 1));
	static_assert(special, "when es == 1, cfloat must have both subnormals and supernormals");
	static constexpr unsigned bitsInByte = 8u;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static_assert(bitsInBlock <= 64, "storage unit for block arithmetic needs to be <= uint64_t"); // TODO: carry propagation on uint64_t requires assembly code

	static constexpr unsigned nbits = _nbits;
	static constexpr unsigned es = _es;
	static constexpr unsigned fbits  = nbits - 1u - es;    // number of fraction bits excluding the hidden bit
	static constexpr unsigned fhbits = nbits - es;           // number of fraction bits including the hidden bit

	static constexpr uint64_t  storageMask = (0xFFFFFFFFFFFFFFFFull >> (64ull - bitsInBlock));
	static constexpr bt       BLOCK_MASK = bt(~0);
	static constexpr bt       ALL_ONES = bt(~0); // block type specific all 1's value
	static constexpr uint32_t ALL_ONES_ES = (0xFFFF'FFFFul >> (32 - es));
	static constexpr uint64_t topfbits = fbits % 64;
	static constexpr uint64_t FR_SHIFT = (topfbits > 0 ? (64 - topfbits) : 0);
	static constexpr uint64_t ALL_ONES_FR = (topfbits > 0 ? (0xFFFF'FFFF'FFFF'FFFFull >> FR_SHIFT) : 0ull); // special case for nbits <= 64
	static constexpr uint64_t INF_ENCODING = (ALL_ONES_FR & ~1ull);

	static constexpr unsigned nrBlocks = 1u + ((nbits - 1ull) / bitsInBlock);
	static constexpr unsigned MSU = nrBlocks - 1u; // MSU == Most Significant Unit, as MSB is already taken
	static constexpr bt       MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr unsigned bitsInMSU = bitsInBlock - (nrBlocks * bitsInBlock - nbits);
	static constexpr unsigned fBlocks = 1ull + ((fbits - 1ull) / bitsInBlock); // nr of blocks with fraction bits
	static constexpr unsigned FSU = fBlocks - 1u;  // FSU = Fraction Significant Unit: the index of the block that contains the most significant fraction bits
	static constexpr bt       FSU_MASK = (ALL_ONES >> (fBlocks * bitsInBlock - fbits));
	static constexpr unsigned bitsInFSU = bitsInBlock - (fBlocks * bitsInBlock - fbits);

	static constexpr bt       SIGN_BIT_MASK = bt(bt(1ull) << ((nbits - 1ull) % bitsInBlock));
	static constexpr bt       LSB_BIT_MASK = bt(1ull);
	static constexpr bool     MSU_CAPTURES_EXP = (1ull + es) <= bitsInMSU;
	static constexpr unsigned EXP_SHIFT = (MSU_CAPTURES_EXP ? (1 == nrBlocks ? (nbits - 1ull - es) : (bitsInMSU - 1ull - es)) : 0);
	static constexpr bt       MSU_EXP_MASK = ((ALL_ONES << EXP_SHIFT) & ~SIGN_BIT_MASK) & MSU_MASK;
	static constexpr int      EXP_BIAS = ((1 << (es - 1u)) - 1l);
	static constexpr int      MAX_EXP = (es == 1) ? 1 : ((1 << es) - EXP_BIAS - 1);
	static constexpr int      MIN_EXP_NORMAL = 1 - EXP_BIAS;
	static constexpr int      MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP

	static constexpr bool     hasSubnormals   = _hasSubnormals;
	static constexpr bool     hasSupernormals = _hasSupernormals;
	static constexpr bool     isSaturating    = _isSaturating;
	typedef bt BlockType;

	// constructors
	cfloat() = default;
	cfloat(const cfloat&) = default;
	cfloat& operator=(const cfloat&) = default;

	// construct a cfloat from another
	template<unsigned nnbits, unsigned ees, typename bbt, bool ssub, bool ssup, bool ssat>
	cfloat(const cfloat<nnbits, ees, bbt, ssub, ssup, ssat>& rhs) noexcept : _block{} {
		if (rhs.isnan()) {
			setnan(rhs.sign() ? NAN_TYPE_SIGNALLING : NAN_TYPE_QUIET);
		}
		else if (rhs.isinf()) {
			setinf(rhs.sign());
		}
		else if (rhs.iszero()) {
			setzero();
		}
		else {
			// TODO: cfloat from another cfloat: marshall through a proper blocktriple
			/*
			if constexpr (std::is_same_v<bt, bbt>) {
				blocktriple<fbits, BlockTripleOperator::REP, bt> value;
				value.setnormal();
				value.setsign(rhs.sign());
				value.setscale(rhs.scale());
				//constexpr unsigned rhsFbits = nnbits - 1ul - ees;
				//blockbinary<rhsFbits, bbt, BinaryNumberType::Signed> fraction;
				//rhs.fraction<rhsFbits>(fraction);
				//std::cout << "fraction : " << to_binary(fraction) << '\n';
				//value.setfraction(fraction);
				convert(value, *this);
			}
			else {
				static_assert(nnbits < 64, "converting constructor marshalls values through native double precision, and rhs has more bits");
				*this = double(rhs); 
			}
			*/
			*this = double(rhs);
		}
	}

	// converting constructors
	constexpr cfloat(const std::string& stringRep) : _block{} { assign(stringRep); }
	// specific value constructor
	constexpr cfloat(const SpecificValue code) noexcept : _block{} {
		switch (code) {
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
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::infpos:
			setinf(false);
			break;
		case SpecificValue::infneg:
			setinf(true);
			break;
		case SpecificValue::nar: // approximation as cfloats don't have a NaR
		case SpecificValue::qnan:
			setnan(NAN_TYPE_QUIET);
			break;
		case SpecificValue::snan:
			setnan(NAN_TYPE_SIGNALLING);
			break;
		}
	}

	constexpr cfloat(signed char iv)                    noexcept : _block{} { *this = iv; }
	constexpr cfloat(short iv)                          noexcept : _block{} { *this = iv; }
	constexpr cfloat(int iv)                            noexcept : _block{} { *this = iv; }
	constexpr cfloat(long iv)                           noexcept : _block{} { *this = iv; }
	constexpr cfloat(long long iv)                      noexcept : _block{} { *this = iv; }
	constexpr cfloat(char iv)                           noexcept : _block{} { *this = iv; }
	constexpr cfloat(unsigned short iv)                 noexcept : _block{} { *this = iv; }
	constexpr cfloat(unsigned int iv)                   noexcept : _block{} { *this = iv; }
	constexpr cfloat(unsigned long iv)                  noexcept : _block{} { *this = iv; }
	constexpr cfloat(unsigned long long iv)             noexcept : _block{} { *this = iv; }
	CONSTEXPRESSION cfloat(float iv)                    noexcept : _block{} { *this = iv; }
	CONSTEXPRESSION cfloat(double iv)                   noexcept : _block{} { *this = iv; }

	// assignment operators
	constexpr cfloat& operator=(signed char rhs)        noexcept { return convert_signed_integer(rhs); }
	constexpr cfloat& operator=(short rhs)              noexcept { return convert_signed_integer(rhs); }
	constexpr cfloat& operator=(int rhs)                noexcept { return convert_signed_integer(rhs); }
	constexpr cfloat& operator=(long rhs)               noexcept { return convert_signed_integer(rhs); }
	constexpr cfloat& operator=(long long rhs)          noexcept { return convert_signed_integer(rhs); }

	constexpr cfloat& operator=(char rhs)               noexcept { return convert_unsigned_integer(rhs); }
	constexpr cfloat& operator=(unsigned short rhs)     noexcept { return convert_unsigned_integer(rhs); }
	constexpr cfloat& operator=(unsigned int rhs)       noexcept { return convert_unsigned_integer(rhs); }
	constexpr cfloat& operator=(unsigned long rhs)      noexcept { return convert_unsigned_integer(rhs); }
	constexpr cfloat& operator=(unsigned long long rhs) noexcept { return convert_unsigned_integer(rhs); }

	BIT_CAST_CONSTEXPR cfloat& operator=(float rhs)     noexcept { return convert_ieee754(rhs); }
	BIT_CAST_CONSTEXPR cfloat& operator=(double rhs)    noexcept { return convert_ieee754(rhs); }

	// make conversions to native types explicit
	explicit operator int()                       const noexcept { return to_int(); }
	explicit operator long()                      const noexcept { return to_long(); }
	explicit operator long long()                 const noexcept { return to_long_long(); }
	explicit operator float()                     const noexcept { return to_native<float>(); }
	explicit operator double()                    const noexcept { return to_native<double>(); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	explicit operator long double()               const noexcept { return to_native<long double>(); }
	BIT_CAST_CONSTEXPR cfloat(long double iv)           noexcept : _block{} { *this = iv; }
	BIT_CAST_CONSTEXPR cfloat& operator=(long double rhs)  noexcept { return convert_ieee754(rhs); }
#endif

	// arithmetic operators
	// prefix operator
	inline cfloat operator-() const {
		cfloat tmp(*this);
		tmp._block[MSU] ^= SIGN_BIT_MASK;
		return tmp;
	}

	cfloat& operator+=(const cfloat& rhs) CFLOAT_EXCEPT {
		if constexpr (_trace_add) std::cout << "---------------------- ADD -------------------" << std::endl;
		// special case handling of the inputs
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
		if (isnan(NAN_TYPE_SIGNALLING) || rhs.isnan(NAN_TYPE_SIGNALLING)) {
			throw cfloat_operand_is_nan{};
		}
#else
		if (isnan(NAN_TYPE_SIGNALLING) || rhs.isnan(NAN_TYPE_SIGNALLING)) {
			setnan(NAN_TYPE_SIGNALLING);
			return *this;
		}
		if (isnan(NAN_TYPE_QUIET) || rhs.isnan(NAN_TYPE_QUIET)) {
			setnan(NAN_TYPE_QUIET);
			return *this;
		}
#endif
		// normal + inf  = inf
		// normal + -inf = -inf
		// inf + normal = inf
		// inf + inf    = inf
		// inf + -inf    = ?
		// -inf + normal = -inf
		// -inf + -inf   = -inf
		// -inf + inf    = ?
		if (isinf()) {
			if (rhs.isinf()) {
				if (sign() != rhs.sign()) {
					setnan(NAN_TYPE_SIGNALLING);
				}
				return *this;
			}
			else {
				return *this;
			}
		}
		else {
			if (rhs.isinf()) {
				*this = rhs;
				return *this;
			}
		}

		if (iszero()) {
			*this = rhs;
			return *this;
		}
		if (rhs.iszero()) return *this;

		// arithmetic operation
		blocktriple<fbits, BlockTripleOperator::ADD, bt> a, b, sum;

		// transform the inputs into (sign,scale,significant) 
		normalizeAddition(a); 
		rhs.normalizeAddition(b);
		sum.add(a, b);

		convert(sum, *this);

		return *this;
	}
	cfloat& operator+=(double rhs) CFLOAT_EXCEPT {
		return *this += cfloat(rhs);
	}
	cfloat& operator-=(const cfloat& rhs) CFLOAT_EXCEPT {
		if constexpr (_trace_sub) std::cout << "---------------------- SUB -------------------" << std::endl;
		if (rhs.isnan()) 
			return *this += rhs;
		else 
			return *this += -rhs;
	}
	cfloat& operator-=(double rhs) CFLOAT_EXCEPT {
		return *this -= cfloat(rhs);
	}
	cfloat& operator*=(const cfloat& rhs) CFLOAT_EXCEPT {
		if constexpr (_trace_mul) std::cout << "---------------------- MUL -------------------\n";
		// special case handling of the inputs
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
		if (isnan(NAN_TYPE_SIGNALLING) || rhs.isnan(NAN_TYPE_SIGNALLING)) {
			throw cfloat_operand_is_nan{};
		}
#else
		if (isnan(NAN_TYPE_SIGNALLING) || rhs.isnan(NAN_TYPE_SIGNALLING)) {
			setnan(NAN_TYPE_SIGNALLING);
			return *this;
		}
		if (isnan(NAN_TYPE_QUIET) || rhs.isnan(NAN_TYPE_QUIET)) {
			setnan(NAN_TYPE_QUIET);
			return *this;
		}
#endif
		//  inf * inf = inf
		//  inf * -inf = -inf
		// -inf * inf = -inf
		// -inf * -inf = inf
		//	0 * inf = -nan(ind)
		//	inf * 0 = -nan(ind)
		bool resultSign = sign() != rhs.sign();
		if (isinf()) {
			if (rhs.iszero()) {
				setnan(NAN_TYPE_QUIET);
			}
			else {
				setsign(resultSign);
			}
			return *this;
		}
		if (rhs.isinf()) {
			if (iszero()) {
				setnan(NAN_TYPE_QUIET);
			}
			else {
				setinf(resultSign);
			}
			return *this;
		}

		if (iszero() || rhs.iszero()) {			
			setzero();
			setsign(resultSign); // deal with negative 0
			return *this;
		}

		// arithmetic operation
		blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;

		// transform the inputs into (sign,scale,significant) 
		// triples of the correct width
		normalizeMultiplication(a);
		rhs.normalizeMultiplication(b);
		product.mul(a, b);
		convert(product, *this);

		if constexpr (_trace_mul) std::cout << to_binary(a) << " : " << a << " *\n" << to_binary(b) << " : " << b << " =\n" << to_binary(product) << " : " << product << '\n';

		return *this;
	}
	cfloat& operator*=(double rhs) CFLOAT_EXCEPT {
		return *this *= cfloat(rhs);
	}
	cfloat& operator/=(const cfloat& rhs) CFLOAT_EXCEPT {
		if constexpr (_trace_div) std::cout << "---------------------- DIV -------------------" << std::endl;

		// special case handling of the inputs
		// qnan / qnan = qnan
		// qnan / snan = qnan
		// snan / qnan = snan
		// snan / snan = snan
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) throw cfloat_divide_by_zero();
		if (rhs.isnan()) throw cfloat_divide_by_nan();
		if (isnan()) throw cfloat_operand_is_nan();
#else
		if (isnan(NAN_TYPE_SIGNALLING) || rhs.isnan(NAN_TYPE_SIGNALLING)) {
			setnan(NAN_TYPE_SIGNALLING);
			return *this;
		}
		if (isnan(NAN_TYPE_QUIET) || rhs.isnan(NAN_TYPE_QUIET)) {
			setnan(NAN_TYPE_QUIET);
			return *this;
		}
		if (rhs.iszero()) {
			if (iszero()) {
				// zero divide by zero yields quiet NaN (in MSVC it is labeled -nan(ind) for indeterminate)
				setnan(NAN_TYPE_QUIET);
			}
			else {
				// non-zero divide by zero yields INF
				bool resultSign = sign() != rhs.sign();
				setinf(resultSign);
			}
			return *this;
		}
#endif
		//  inf /  inf = -nan(ind)
		//  inf / -inf = -nan(ind)
		// -inf /  inf = -nan(ind)
		// -inf / -inf = -nan(ind)
		//	1.0 /  inf = 0
		bool resultSign = sign() != rhs.sign();
		if (isinf()) {
			if (rhs.isinf()) {
				// inf divide by inf yields quiet NaN (in MSVC it is labeled -nan(ind) for indeterminate)
				setnan(NAN_TYPE_QUIET);
				return *this;
			}
			else {
				// we stay an infinite but may change sign
				setsign(resultSign);
				return *this;
			}
		}
		else {
			if (rhs.isinf()) {
				setzero();
				setsign(resultSign);
				return *this;
			}
		}

		if (iszero()) {
			setzero();
			setsign(resultSign); // deal with negative 0
			return *this;
		}

		// arithmetic operation
		using BlockTriple = blocktriple<fbits, BlockTripleOperator::DIV, bt>;
		BlockTriple a, b, quotient;

		// transform the inputs into (sign,scale,significant) 
		// triples of the correct width
		normalizeDivision(a);
		rhs.normalizeDivision(b);
		quotient.div(a, b);
		quotient.setradix(BlockTriple::radix);
		convert(quotient, *this);

		if constexpr (_trace_div) std::cout << to_binary(a) << " : " << a << " /\n" << to_binary(b) << " : " << b << " =\n" << to_binary(quotient) << " : " << quotient << '\n';

		return *this;
	}
	cfloat& operator/=(double rhs) CFLOAT_EXCEPT {
		return *this /= cfloat(rhs);
	}
	cfloat& reciprocal() CFLOAT_EXCEPT {
		cfloat c = 1.0 / *this;
		return *this = c;
	}
	/// <summary>
	/// move to the next bit encoding modulo 2^nbits
	/// </summary>
	/// <typeparam name="bt"></typeparam>
	cfloat& operator++() {
		if constexpr (0 == nrBlocks) {
			return *this;
		}
		else if constexpr (1 == nrBlocks) {
			if (sign()) {
				if (_block[MSU] == (SIGN_BIT_MASK | 1ul)) { // pattern: 1.00.001 = minneg
					_block[MSU] = 0; // pattern: 0.00.000 = +0 
				}
				else {
					--_block[MSU];
				}
				if constexpr (!hasSubnormals) {
					if (isdenormal()) {
						// special case, we need to jump past all the subnormal value encodings which puts us on 0
						_block[MSU] = 0; // pattern: 0.00.000 = +0
					}
				}
			}
			else {
				if constexpr (!hasSubnormals) {
					if (_block[MSU] == 0) {
						// special case, we need to jump past all the subnormal value encodings minus 1
						setfraction(0xFFFF'FFFF'FFFF'FFFFull);
					}
				}
				if ((_block[MSU] & (MSU_MASK >> 1)) == (MSU_MASK >> 1)) { // pattern: 0.11.111 = nan
					_block[MSU] |= SIGN_BIT_MASK; // pattern: 1.11.111 = snan : wrap to the other side of the encoding
				}
				else {
					++_block[MSU];
				}
			}
		}
		else {
			if (sign()) {
				// special case: pattern: 1.00.001 = minneg transitions to pattern: 0.00.000 = +0 
				if (isminnegencoding()) {
					setzero();
				}
				else {
					//  1111 0000
					//  1110 1111
					bool borrow = true;
					for (unsigned i = 0; i < MSU; ++i) {
						if (borrow) {
							if ((_block[i] & storageMask) == 0) { // block will underflow
								--_block[i];
								borrow = true;
							}
							else {
								--_block[i];
								borrow = false;
							}
						}
					}
					if (borrow) {
						--_block[MSU];
					}
					if constexpr (!hasSubnormals) {
						if (isdenormal()) {
							// special case, we need to jump past all the subnormal value encodings which puts us on 0
							setzero(); // pattern: 0.00.000 = +0
						}
					}
				}
			}
			else {
				// special case: pattern: 0.11.111 = nan transitions to pattern: 1.11.111 = snan 
				if (isnanencoding()) {
					setnan(NAN_TYPE_SIGNALLING);
				}
				else {
					if constexpr (!hasSubnormals) {
						if (iszero()) {
							// special case, we need to jump past all the subnormal value encodings minus 1 so that the increment code below ends up on normal minpos
							setfraction(0xFFFF'FFFF'FFFF'FFFFull);
						}
					}
					bool carry = true;
					for (unsigned i = 0; i < MSU; ++i) {
						if (carry) {
							if ((_block[i] & storageMask) == storageMask) { // block will overflow
								_block[i] = 0;
								carry = true;
							}
							else {
								++_block[i];
								carry = false;
							}
						}
					}
					if (carry) {
						++_block[MSU];
					}
				}
			}
		}
		return *this;
	}
	cfloat operator++(int) {
		cfloat tmp(*this);
		operator++();
		return tmp;
	}
	cfloat& operator--() {
		if constexpr (0 == nrBlocks) {
			return *this;
		}
		else if constexpr (1 == nrBlocks) {
			if (sign()) {
				++_block[MSU];
			}
			else {
				// positive range
				if (_block[MSU] == 0) { // pattern: 0.00.000 = 0
					if constexpr (hasSubnormals) {
						_block[MSU] |= SIGN_BIT_MASK | bt(1u); // pattern: 1.00.001 = minneg 
					}
					else {
						// special case, we need to jump past all the subnormal value encodings
						setfraction(0xFFFF'FFFF'FFFF'FFFFull); // set to 0.00.11...11
						++_block[MSU]; // increment into 0.01.0000
						_block[MSU] |= SIGN_BIT_MASK; // set to 1.01.0000
					}
				}
				else {
					--_block[MSU];
				}
				if constexpr (!hasSubnormals) {
					if (isdenormal()) {
						// special case, we need to jump past all the subnormal value encodings which puts us on 0
						_block[MSU] = 0; // pattern: 0.00.000 = +0
					}
				}
			}
		}
		else {
			if (sign()) {
				bool carry = true;
				for (unsigned i = 0; i < MSU; ++i) {
					if (carry) {
						if ((_block[i] & storageMask) == storageMask) { // block will overflow
							_block[i] = 0;
							carry = true;
						}
						else {
							++_block[i];
							carry = false;
						}
					}
				}
				if (carry) {
					++_block[MSU];
				}
			}
			else {
				// special case: pattern: 0.00.000 = +0 transitions to pattern: 1.00.001 = minneg
				if (iszeroencoding()) {
					if constexpr (hasSubnormals) {
						setsign(true);
						setbit(0, true);
					}
					else {
						// special case, we need to jump past all the subnormal value encodings 1.01.0000 = minneg normal
						setexponent(1 - EXP_BIAS);
						setsign(true);
					}
				}
				else {
					bool borrow = true;
					for (unsigned i = 0; i < MSU; ++i) {
						if (borrow) {
							if ((_block[i] & storageMask) == 0) { // block will underflow
								--_block[i];
								borrow = true;
							}
							else {
								--_block[i];
								borrow = false;
							}
						}
					}
					if (borrow) {
						--_block[MSU];
					}
					if constexpr (!hasSubnormals) {
						if (isdenormal()) {
							// special case, we need to jump past all the subnormal value encodings which puts us on 0
							setzero(); // pattern: 0.00.000 = +0
						}
					}
				}
			}
		}
		return *this;
	}
	cfloat operator--(int) {
		cfloat tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers	
	constexpr void clear() noexcept {
		if constexpr (0 == nrBlocks) {
			return;
		}
		else if constexpr (1 == nrBlocks) {
			_block[0] = bt(0);
		}
		else if constexpr (2 == nrBlocks) {
			_block[0] = bt(0);
			_block[1] = bt(0);
		}
		else if constexpr (3 == nrBlocks) {
			_block[0] = bt(0);
			_block[1] = bt(0);
			_block[2] = bt(0);
		}
		else if constexpr (4 == nrBlocks) {
			_block[0] = bt(0);
			_block[1] = bt(0);
			_block[2] = bt(0);
			_block[3] = bt(0);
		}
		else if constexpr (5 == nrBlocks) {
			_block[0] = bt(0);
			_block[1] = bt(0);
			_block[2] = bt(0);
			_block[3] = bt(0);
			_block[4] = bt(0);
		}
		else if constexpr (6 == nrBlocks) {
			_block[0] = bt(0);
			_block[1] = bt(0);
			_block[2] = bt(0);
			_block[3] = bt(0);
			_block[4] = bt(0);
			_block[5] = bt(0);
		}
		else if constexpr (7 == nrBlocks) {
			_block[0] = bt(0);
			_block[1] = bt(0);
			_block[2] = bt(0);
			_block[3] = bt(0);
			_block[4] = bt(0);
			_block[5] = bt(0);
			_block[6] = bt(0);
		}
		else if constexpr (8 == nrBlocks) {
			_block[0] = bt(0);
			_block[1] = bt(0);
			_block[2] = bt(0);
			_block[3] = bt(0);
			_block[4] = bt(0);
			_block[5] = bt(0);
			_block[6] = bt(0);
			_block[7] = bt(0);
		}
		else {
			for (unsigned i = 0; i < nrBlocks; ++i) {
				_block[i] = bt(0);
			}
		}
	}
	constexpr void setzero() noexcept { clear(); }
	constexpr void setinf(bool sign = true) noexcept {
		// the Inf encoding is the pattern 0b0'11...11'11...10 for a +inf, and 0b1'11...11'11...110 for a -inf
		if constexpr (0 == nrBlocks) {
			return;
		}
		else if constexpr (1 == nrBlocks) {
			_block[MSU] = sign ? bt(MSU_MASK ^ LSB_BIT_MASK) : bt(~SIGN_BIT_MASK & (MSU_MASK ^ LSB_BIT_MASK));
		}
		else if constexpr (2 == nrBlocks) {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}
		else if constexpr (3 == nrBlocks) {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[1] = BLOCK_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}
		else if constexpr (4 == nrBlocks) {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}
		else if constexpr (5 == nrBlocks) {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[3] = BLOCK_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}
		else if constexpr (6 == nrBlocks) {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[3] = BLOCK_MASK;
			_block[4] = BLOCK_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}
		else if constexpr (7 == nrBlocks) {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[3] = BLOCK_MASK;
			_block[4] = BLOCK_MASK;
			_block[5] = BLOCK_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}
		else if constexpr (8 == nrBlocks) {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[3] = BLOCK_MASK;
			_block[4] = BLOCK_MASK;
			_block[5] = BLOCK_MASK;
			_block[6] = BLOCK_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}
		else {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			for (unsigned i = 1; i < nrBlocks - 1; ++i) {
				_block[i] = BLOCK_MASK;
			}
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}	
	}
	constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING) noexcept {
		// the NaN encoding is the pattern 0b0'11...11'11...11 for a quiet Nan, and 0b1'11...11'11...111 for a signalling NaN
		if constexpr (0 == nrBlocks) {
			return;
		}
		else if constexpr (1 == nrBlocks) {
			// fall through
		}
		else if constexpr (2 == nrBlocks) {
			_block[0] = BLOCK_MASK;
		}
		else if constexpr (3 == nrBlocks) {
			_block[0] = BLOCK_MASK;
			_block[1] = BLOCK_MASK;
		}
		else if constexpr (4 == nrBlocks) {
			_block[0] = BLOCK_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
		}
		else if constexpr (5 == nrBlocks) {
			_block[0] = BLOCK_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[3] = BLOCK_MASK;
		}
		else if constexpr (6 == nrBlocks) {
			_block[0] = BLOCK_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[3] = BLOCK_MASK;
			_block[4] = BLOCK_MASK;
		}
		else if constexpr (7 == nrBlocks) {
			_block[0] = BLOCK_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[3] = BLOCK_MASK;
			_block[4] = BLOCK_MASK;
			_block[5] = BLOCK_MASK;
		}
		else if constexpr (8 == nrBlocks) {
			_block[0] = BLOCK_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[3] = BLOCK_MASK;
			_block[4] = BLOCK_MASK;
			_block[5] = BLOCK_MASK;
			_block[6] = BLOCK_MASK;
		}
		else {
			for (unsigned i = 0; i < nrBlocks - 1; ++i) {
				_block[i] = BLOCK_MASK;
			}
		}
		_block[MSU] = NaNType == NAN_TYPE_SIGNALLING ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
	}
	constexpr void setsign(bool sign = true) {
		if (sign) {
			_block[MSU] |= SIGN_BIT_MASK;
		}
		else {
			_block[MSU] &= ~SIGN_BIT_MASK;
		}
	}
	constexpr bool setexponent(int scale) {
		if (scale < MIN_EXP_SUBNORMAL || scale > MAX_EXP) return false; // this scale cannot be represented
		if constexpr (nbits < 65) {
			uint32_t exponentBits = static_cast<uint32_t>(scale + EXP_BIAS);
			if (scale >= MIN_EXP_SUBNORMAL && scale < MIN_EXP_NORMAL) {
				// we are a subnormal number: all exponent bits are 0
				exponentBits = 0;
			}
			// TODO: optimize
			uint32_t mask = (1ul << (es - 1));
			for (unsigned i = nbits - 2; i > nbits - 2 - es; --i) {
				setbit(i, (mask & exponentBits));
				mask >>= 1;
			}
		}
		else {
			uint32_t exponentBits = static_cast<uint32_t>(scale + EXP_BIAS);
			uint32_t mask = (1ul << (es - 1));
			for (unsigned i = nbits - 2; i > nbits - 2 - es; --i) {
				setbit(i, (mask & exponentBits));
				mask >>= 1;
			}
		}
		return true;
	}
	constexpr void setfraction(uint64_t raw_bits) {
		// unoptimized as it is not meant to be an end-user API, it is a test API
		if constexpr (fbits < 65) {
			uint64_t mask{ 1ull };
			for (unsigned i = 0; i < fbits; ++i) {
				setbit(i, (mask & raw_bits));
				mask <<= 1;
			}
		}
	}
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned blockIndex = i / bitsInBlock;
		if (blockIndex < nrBlocks) {
			bt block = _block[blockIndex];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[blockIndex] = bt((block & null) | mask);
		}
	}
	constexpr cfloat& setbits(uint64_t raw_bits) noexcept {
		if constexpr (0 == nrBlocks) {
			return *this;
		}
		else if constexpr (1 == nrBlocks) {
			_block[0] = raw_bits & storageMask;
		}
		else if constexpr (2 == nrBlocks) {
			if constexpr (bitsInBlock < 64) {
				_block[0] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[1] = raw_bits & storageMask;
			}
			else {
				_block[0] = raw_bits & storageMask;
				_block[1] = 0;
			}
		}
		else if constexpr (3 == nrBlocks) {
			if constexpr (bitsInBlock < 64) {
				_block[0] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[1] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[2] = raw_bits & storageMask;
			}
			else {
				_block[0] = raw_bits & storageMask;
				_block[1] = 0;
				_block[2] = 0;
			}
		}
		else if constexpr (4 == nrBlocks) {
			if constexpr (bitsInBlock < 64) {
				_block[0] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[1] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[2] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock;
				_block[3] = raw_bits & storageMask;
			}
			else {
				_block[0] = raw_bits & storageMask;
				_block[1] = 0;
				_block[2] = 0;
				_block[3] = 0;
			}
		}
		else {
			if constexpr (bitsInBlock < 64) {
				for (unsigned i = 0; i < nrBlocks; ++i) {
					_block[i] = raw_bits & storageMask;
					raw_bits >>= bitsInBlock;
				}
			}
			else {
				_block[0] = raw_bits & storageMask;
				for (unsigned i = 1; i < nrBlocks; ++i) {
					_block[i] = 0;
				}
			}
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		return *this;
	}
	constexpr void setblock(unsigned b, bt data) noexcept {
		if (b < nrBlocks) _block[b] = data;
	}
	
	// create specific number system values of interest
	constexpr cfloat& maxpos() noexcept {
		if constexpr (isSaturating) {
			// in a saturating encoding with supernormals we are removing the Inf encoding pattern 0b0'11...11'11...10 for a +inf, 
			// and 0b1'11...11'11...110 for a -inf and using it as a value
			if constexpr (hasSupernormals) {
				// maximum positive value has this bit pattern: 0-1...1-111...110, that is, sign = 0, e = 11..11, f = 111...110
				clear();
				flip();
				setbit(nbits - 1ull, false); // sign = 0
				setbit(0ull, false); // bit0 = 0
			}
			else {
				// maximum positive value has this bit pattern: 0-11...10-111...111, that is, sign = 0, e = 11..10, f = 111...111
				clear();
				flip();
				setbit(fbits, false); // set least significant exponent bit to 0
				setbit(nbits - 1ull, false); // set sign to 0
			}
		}
		else {
			// the Inf encoding is the pattern 0b0'11...11'11...10 for a +inf, and 0b1'11...11'11...110 for a -inf
			// the maxpos is the encoding before that
			if constexpr (hasSupernormals) {
				// maximum positive value has this bit pattern: 0-1...1-111...101, that is, sign = 0, e = 11..11, f = 111...101
				clear();
				flip();
				setbit(nbits - 1ull, false); // sign = 0
				setbit(1ull, false); // bit1 = 0
			}
			else {
				// maximum positive value has this bit pattern: 0-1...0-111...111, that is, sign = 0, e = 11..10, f = 111...111
				clear();
				flip();
				setbit(fbits, false); // set least significant exponent bit to 0
				setbit(nbits - 1ull, false); // set sign to 0
			}
		}
		return *this;
	}
	constexpr cfloat& minpos() noexcept {
		// minpos encoding is not impacted by saturating encodings, which only affects maxpos and inf
		if constexpr (hasSubnormals) {
			// minimum positive value has this bit pattern: 0-000-00...01, that is, sign = 0, e = 000, f = 00001
			clear();
			setbit(0);
		}
		else {
			// minimum positive value has this bit pattern: 0-001-00...0, that is, sign = 0, e = 001, f = 0000
			clear();
			setbit(fbits);
		}
		return *this;
	}
	constexpr cfloat& zero() noexcept {
		// the zero value
		clear();
		return *this;
	}
	constexpr cfloat& minneg() noexcept {
		// minneg encoding is not impacted by saturating encodings, which only affects maxpos and inf
		if constexpr (hasSubnormals) {
			// minimum negative value has this bit pattern: 1-000-00...01, that is, sign = 1, e = 00, f = 00001
			clear();
			setbit(nbits - 1ull);
			setbit(0);
		}
		else {
			// minimum negative value has this bit pattern: 1-001-00...0, that is, sign = 1, e = 001, f = 0000
			clear();
			setbit(fbits);
			setbit(nbits - 1ull);
		}
		return *this;
	}
	constexpr cfloat& maxneg() noexcept {
		if constexpr (isSaturating) {
			// in a saturating encoding with supernormals we are removing the Inf encoding pattern 0b0'11...11'11...10 for a +inf, 
			// and 0b1'11...11'11...110 for a -inf and using it as a value
			if constexpr (hasSupernormals) {
				// maximum negative value has this bit pattern: 1-1...1-111...110, that is, sign = 1, e = 1..1, f = 111...110
				clear();
				flip();
				setbit(0ull, false);
			}
			else {
				// maximum negative value has this bit pattern: 1-1...0-111...111, that is, sign = 1, e = 11..10, f = 111...111
				clear();
				flip();
				setbit(fbits, false);
			}
		}
		else {
			if constexpr (hasSupernormals) {
				// maximum negative value has this bit pattern: 1-1...1-111...101, that is, sign = 1, e = 1..1, f = 111...101
				clear();
				flip();
				setbit(1ull, false);
			}
			else {
				// maximum negative value has this bit pattern: 1-1...0-111...111, that is, sign = 1, e = 11..10, f = 111...111
				clear();
				flip();
				setbit(fbits, false);
			}
		}
		return *this;
	}


	/// <summary>
	/// assign the value of the string representation to the cfloat
	/// </summary>
	/// <param name="stringRep">decimal scientific notation of a real number to be assigned</param>
	/// <returns>reference to this cfloat</returns>
	/// Clang doesn't support constexpr yet on string manipulations, so we need to make it conditional
	CONSTEXPRESSION cfloat& assign(const std::string& str) noexcept {
		clear();
		unsigned nrChars = static_cast<unsigned>(str.size());
		unsigned nrBits = 0;
		unsigned nrDots = 0;
		std::string bits;
		if (nrChars > 2) {
			if (str[0] == '0' && str[1] == 'b') {
				for (unsigned i = 2; i < nrChars; ++i) {
					char c = str[i];
					switch (c) {
					case '0':
					case '1':
						++nrBits;
						bits += c;
						break;
					case '.':
						++nrDots;
						bits += c;
						break;
					case '\'':
						// consume this delimiting character
						break;
					default:
						std::cerr << "string contained a non-standard character: " << c << '\n';
						return *this;
					}
				}
			}
			else {
				std::cerr << "string must start with 0b: instead input pattern was " << str << '\n';
				return *this;
			}
		}
		else {
			std::cerr << "string is too short\n";
			return *this;
		}

		if (nrBits != nbits) {
			std::cerr << "number of bits in the string is " << nrBits << " and needs to be " << nbits << '\n';
			return *this;
		}
		if (nrDots != 2) {
			std::cerr << "number of segment delimiters in string is " << nrDots << " and needs to be 2 for a cfloat<>\n";
			return *this;
		}

		// assign the bits
		int field{ 0 };  // three fields: sign, exponent, mantissa: fields are separated by a '.'
		int nrExponentBits{ -1 };
		unsigned bit = nrBits;
		for (unsigned i = 0; i < bits.size(); ++i) {
			char c = bits[i];
			if (c == '.') {
				++field;
				if (field == 2) { // just finished parsing exponent field: we can now check the number of exponent bits
					if (nrExponentBits != es) {
						std::cerr << "provided binary string representation does not contain " << es << " exponent bits. Found " << nrExponentBits << ". Reset to 0\n";
						clear();
						return *this;
					}
				}
			}
			else {
				setbit(--bit, c == '1');
			}
			if (field == 1) { // exponent field
				++nrExponentBits;
			}
		}
		if (field != 2) {
			std::cerr << "provided binary string did not contain three fields separated by '.': Reset to 0\n";
			clear();
			return *this;
		}
		return *this;
	}

	// selectors
	constexpr bool sign() const noexcept { return (_block[MSU] & SIGN_BIT_MASK) == SIGN_BIT_MASK; }
	constexpr int  scale() const noexcept {
		int e{ 0 };
		if constexpr (MSU_CAPTURES_EXP) {
			e = static_cast<int>((_block[MSU] & ~SIGN_BIT_MASK) >> EXP_SHIFT);
			if (e == 0) {
				// subnormal scale is determined by fraction
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				e = (2l - (1l << (es - 1ull))) - 1;
				if constexpr (nbits > 2 + es) {
					for (unsigned i = nbits - 2ull - es; i > 0; --i) {
						if (test(i)) break;
						--e;
					}
				}
			}
			else {
				e -= EXP_BIAS;
			}
		}
		else {
			blockbinary<es, bt> ebits{};
			exponent(ebits);
			if (ebits.iszero()) {
				// subnormal scale is determined by fraction
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				e = (2l - (1l << (es - 1ull))) - 1;
				if constexpr (nbits > 2 + es) {
					for (unsigned i = nbits - 2ull - es; i > 0; --i) {
						if (test(i)) break;
						--e;
					}
				}
			}
			else {
				e = static_cast<int>(unsigned(ebits) - EXP_BIAS);
			}
		}
		return e;
	}
	constexpr bool isneg() const noexcept {
		if (isnan()) return false;
		return sign(); 
	}
	constexpr bool ispos() const noexcept { 
		if (isnan()) return false;
		return !sign(); 
	}
	constexpr bool iszero() const noexcept {
		// NOTE: this is a very specific design that makes the decsion that
		// for subnormal encodings found in a configuration that doesn't
		// support them, we assume that these values map to 0.
		if constexpr (hasSubnormals) {
			return iszeroencoding();
		}
		else { // all subnormals round to 0
			blockbinary<es, bt> ebits{};
			exponent(ebits);
			if (ebits.iszero()) return true; else return false;
		}
	}
	constexpr bool isone() const noexcept {
		// unbiased exponent = scale = 0, fraction = 0
		int s = scale();
		if (s == 0) {
			blockbinary<fbits, bt> f{};
			fraction(f);
			return f.iszero();
		}
		return false;
	}
	constexpr bool isinf(int InfType = INF_TYPE_EITHER) const noexcept {
		// the bit pattern encoding of Inf is independent of gradual overflow (supernormal) configuration
		bool isNegInf = false;
		bool isPosInf = false;
		if constexpr (0 == nrBlocks) {
			return false;
		}
		else if constexpr (1 == nrBlocks) {
			isNegInf = (_block[MSU] & MSU_MASK) == (MSU_MASK ^ LSB_BIT_MASK);
			isPosInf = (_block[MSU] & MSU_MASK) == ((MSU_MASK ^ SIGN_BIT_MASK) ^ LSB_BIT_MASK);
		}
		else if constexpr (2 == nrBlocks) {
			bool isInf = (_block[0] == (BLOCK_MASK ^ LSB_BIT_MASK));
			isNegInf = isInf && ((_block[MSU] & MSU_MASK) == MSU_MASK);
			isPosInf = isInf && (_block[MSU] & MSU_MASK) == (MSU_MASK ^ SIGN_BIT_MASK);
		}
		else if constexpr (3 == nrBlocks) {
			bool isInf = (_block[0] == (BLOCK_MASK ^ LSB_BIT_MASK)) && (_block[1] == BLOCK_MASK);
			isNegInf = isInf && ((_block[MSU] & MSU_MASK) == MSU_MASK);
			isPosInf = isInf && (_block[MSU] & MSU_MASK) == (MSU_MASK ^ SIGN_BIT_MASK);
		}
		else if constexpr (4 == nrBlocks) {
			bool isInf = (_block[0] == (BLOCK_MASK ^ LSB_BIT_MASK)) && (_block[1] == BLOCK_MASK) && (_block[2] == BLOCK_MASK);
			isNegInf = isInf && ((_block[MSU] & MSU_MASK) == MSU_MASK);
			isPosInf = isInf && (_block[MSU] & MSU_MASK) == (MSU_MASK ^ SIGN_BIT_MASK);
		}
		else {
			bool isInf = (_block[0] == (BLOCK_MASK ^ LSB_BIT_MASK));
			for (unsigned i = 1; i < nrBlocks - 1; ++i) {
				if (_block[i] != BLOCK_MASK) {
					isInf = false;
					break;
				}
			}
			isNegInf = isInf && ((_block[MSU] & MSU_MASK) == MSU_MASK);
			isPosInf = isInf && (_block[MSU] & MSU_MASK) == (MSU_MASK ^ SIGN_BIT_MASK);
		}

		return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
			(InfType == INF_TYPE_NEGATIVE ? isNegInf :
				(InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
	}
	constexpr bool isnan(int NaNType = NAN_TYPE_EITHER) const noexcept {
		if constexpr (hasSupernormals) {
			return isnanencoding(NaNType);
		}
		else {
			if (issupernormal()) {
				// all these supernormal encodings are NANs, except for the encoding representing INF
				bool isNaN = isinf() ? false : true;
				bool isNegNaN = isNaN && sign();
				bool isPosNaN = isNaN && !sign();
				return (NaNType == NAN_TYPE_EITHER ? (isNaN) :
					(NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
						(NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
			}
			else {
				return false;
			}
		}
	}
	// iszeroencoding returns true if it finds a pure -0 or +0 pattern and returns false otherwise
	constexpr bool iszeroencoding() const noexcept {
		if constexpr (0 == nrBlocks) {
			return true;
		}
		else if constexpr (1 == nrBlocks) {
			return (_block[MSU] & ~SIGN_BIT_MASK) == 0;
		}
		else if constexpr (2 == nrBlocks) {
			return (_block[0] == 0) && (_block[MSU] & ~SIGN_BIT_MASK) == 0;
		}
		else if constexpr (3 == nrBlocks) {
			return (_block[0] == 0) && _block[1] == 0 && (_block[MSU] & ~SIGN_BIT_MASK) == 0;
		}
		else if constexpr (4 == nrBlocks) {
			return (_block[0] == 0) && _block[1] == 0 && _block[2] == 0 && (_block[MSU] & ~SIGN_BIT_MASK) == 0;
		}
		else {
			for (unsigned i = 0; i < nrBlocks - 1; ++i) if (_block[i] != 0) return false;
			return (_block[MSU] & ~SIGN_BIT_MASK) == 0;
		}
	}
	// isminnegencoding returns true if it find the pattern 1.00.00001 and returns false otherwise
	constexpr bool isminnegencoding() const noexcept {
		if constexpr (0 == nrBlocks) {
			return false;
		}
		else if constexpr (1 == nrBlocks) {
			return (_block[MSU] & (SIGN_BIT_MASK | 1ul));
		}
		else if constexpr (2 == nrBlocks) {
			return ((_block[0] == 1ul) && (_block[1] == SIGN_BIT_MASK));
		}
		else if constexpr (3 == nrBlocks) {
			return ((_block[0] == 1ul) && (_block[1] == 0) && (_block[2] == SIGN_BIT_MASK));
		}
		else if constexpr (4 == nrBlocks) {
			return ((_block[0] == 1ul) && (_block[1] == 0) && (_block[2] == 0) && (_block[3] == SIGN_BIT_MASK));
		}
		else {
			if (_block[0] != 1ul) return false;
			for (unsigned i = 1; i < nrBlocks - 2; ++i) if (_block[i] != 0) return false;
			return (_block[MSU] == SIGN_BIT_MASK);
		}
	}
	constexpr bool isnanencoding(int NaNType = NAN_TYPE_EITHER) const noexcept {
		// the bit encoding of NaN is independent of the gradual overflow configuration
		bool isNaN = true;
		bool isNegNaN = false;
		bool isPosNaN = false;

		if constexpr (0 == nrBlocks) {
			return false;
		}
		else if constexpr (1 == nrBlocks) {
		}
		else if constexpr (2 == nrBlocks) {
			isNaN = (_block[0] == BLOCK_MASK);
		}
		else if constexpr (3 == nrBlocks) {
			isNaN = (_block[0] == BLOCK_MASK) && (_block[1] == BLOCK_MASK);
		}
		else if constexpr (4 == nrBlocks) {
			isNaN = (_block[0] == BLOCK_MASK) && (_block[1] == BLOCK_MASK) && (_block[2] == BLOCK_MASK);
		}
		else {
			for (unsigned i = 0; i < nrBlocks - 1; ++i) {
				if (_block[i] != BLOCK_MASK) {
					isNaN = false;
					break;
				}
			}
		}
		isNegNaN = isNaN && ((_block[MSU] & MSU_MASK) == MSU_MASK);
		isPosNaN = isNaN && (_block[MSU] & MSU_MASK) == (MSU_MASK ^ SIGN_BIT_MASK);

		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
			(NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
				(NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}
	// isnormal returns true if 0 or exponent bits are not all zero or one, false otherwise
	constexpr bool isnormal() const noexcept {
		if (iszeroencoding()) return true; // filter out the one special case
		blockbinary<es, bt> e{};
		exponent(e);
		return !e.iszero() && !e.all();
	}
	// isdenormal returns true if exponent bits are all zero, false otherwise
	constexpr bool isdenormal() const noexcept {
		if (iszeroencoding()) return false; // filter out the one special case
		blockbinary<es, bt> e{};
		exponent(e);
		return e.iszero(); 
	}
	// issupernormal returns true if exponent bits are all one, false otherwise
	constexpr bool issupernormal() const noexcept {
		blockbinary<es, bt> e{};
		exponent(e);
		return e.all();
	}
	// isinteger is TBD
	constexpr bool isinteger() const noexcept { return false; } // return (floor(*this) == *this) ? true : false; }
	
	template<typename NativeReal>
	constexpr bool inrange(NativeReal v) const noexcept {
		// the valid range for this cfloat includes the interval between 
		// maxpos and the value that would round down to maxpos
		bool bIsInRange = true;		
		if (v > 0) {
			cfloat c(SpecificValue::maxpos);
			cfloat<nbits + 1, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> d{};
			d = NativeReal(c);
			++d;
			if (v >= NativeReal(d)) bIsInRange = false;
		}
		else {
			cfloat c(SpecificValue::maxneg);
			cfloat<nbits + 1, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> d{};
			d = NativeReal(c);
			--d;
			if (v <= NativeReal(d)) bIsInRange = false;
		}

		return bIsInRange;
	}
	constexpr bool test(unsigned bitIndex) const noexcept {
		return at(bitIndex);
	}
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex < nbits) {
			bt word = _block[bitIndex / bitsInBlock];
			bt mask = bt(1ull << (bitIndex % bitsInBlock));
			return (word & mask);
		}
		return false;
	}
	constexpr uint8_t nibble(unsigned n) const noexcept {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			int nibbleIndexInWord = int(n % (bitsInBlock >> 2ull));
			bt mask = bt(0xF << (nibbleIndexInWord * 4));
			bt nibblebits = bt(mask & word);
			return uint8_t(nibblebits >> (nibbleIndexInWord * 4));
		}
		return 0;
	}
	constexpr bt block(unsigned b) const noexcept {
		if (b < nrBlocks) {
			return _block[b];
		}
		return 0;
	}

	constexpr void sign(bool& s) const {
		s = sign();
	}
	constexpr void exponent(blockbinary<es, bt>& e) const {
		e.clear();
		if constexpr (0 == nrBlocks) return;
		else if constexpr (1 == nrBlocks) {
			bt ebits = bt(_block[MSU] & ~SIGN_BIT_MASK);
			e.setbits(uint64_t(ebits >> EXP_SHIFT));
		}
		else if constexpr (nrBlocks > 1) {
			if (MSU_CAPTURES_EXP) {
				bt ebits = bt(_block[MSU] & ~SIGN_BIT_MASK);
				e.setbits(uint64_t(ebits >> ((nbits - 1ull - es) % bitsInBlock)));
			}
			else {
				for (unsigned i = 0; i < es; ++i) { e.setbit(i, at(nbits - 1ull - es + i)); }
			}
		}
	}
	template<unsigned targetFractionBits>
	constexpr blockbinary<targetFractionBits, bt>& fraction(blockbinary<targetFractionBits, bt>& f) const {
		static_assert(targetFractionBits >= fbits, "target blockbinary is too small and can't receive all fraction bits");
		f.clear();
		if constexpr (0 == nrBlocks) return f;
		else if constexpr (1 == nrBlocks) {
			bt fraction = bt(_block[MSU] & ~MSU_EXP_MASK);
			f.setbits(fraction);
		}
		else if constexpr (nrBlocks > 1) {
			for (unsigned i = 0; i < fbits; ++i) { f.setbit(i, at(i)); }
		}
		return f;
	}
	constexpr uint64_t fraction_ull() const {
		uint64_t raw{ 0 };
		if constexpr (nbits - es - 1ull < 65ull) { // no-op if precondition doesn't hold
			if constexpr (1 == nrBlocks) {
				uint64_t fbitMask = 0xFFFF'FFFF'FFFF'FFFF >> (64 - fbits);
				raw = fbitMask & uint64_t(_block[0]);
			}
			else if constexpr (2 == nrBlocks) {
				uint64_t fbitMask = 0xFFFF'FFFF'FFFF'FFFF >> (64 - fbits);
				raw = fbitMask & ((uint64_t(_block[1]) << bitsInBlock) | uint64_t(_block[0]));
			}
			else if constexpr (3 == nrBlocks) {
				uint64_t fbitMask = 0xFFFF'FFFF'FFFF'FFFF >> (64 - fbits);
				raw = fbitMask & ((uint64_t(_block[2]) << (2 * bitsInBlock)) | (uint64_t(_block[1]) << bitsInBlock) | uint64_t(_block[0]));
			}
			else if constexpr (4 == nrBlocks) {
				uint64_t fbitMask = 0xFFFF'FFFF'FFFF'FFFF >> (64 - fbits);
				raw = fbitMask & ((uint64_t(_block[3]) << (3 * bitsInBlock)) | (uint64_t(_block[2]) << (2 * bitsInBlock)) | (uint64_t(_block[1]) << bitsInBlock) | uint64_t(_block[0]));
			}
			else {
				uint64_t mask{ 1 };
				for (unsigned i = 0; i < fbits; ++i) { 
					if (test(i)) {
						raw |= mask;
					}
					mask <<= 1;
				}
			}
		}
		return raw;
	}
	// construct the significant from the encoding, returns normalization offset
	constexpr unsigned significant(blockbinary<fhbits, bt>& s, bool isNormal = true) const {
		unsigned shift = 0;
		if (iszero()) return 0;
		if constexpr (0 == nrBlocks) return 0;
		else if constexpr (1 == nrBlocks) {
			bt significant = bt(_block[MSU] & ~MSU_EXP_MASK & ~SIGN_BIT_MASK);
			if (isNormal) {
				significant |= (bt(0x1ul) << fbits);
			}
			else {
				unsigned msb = find_msb(significant);
//				std::cout << "msb : " << msb << " : fhbits : " << fhbits << " : " << to_binary(significant, true) << std::endl;
				shift = fhbits - msb;
				significant <<= shift;
			}
			s.setbits(significant);
		}
		else if constexpr (nrBlocks > 1) {
			s.clear();
			// TODO: design and implement a block-oriented algorithm, this sequential algorithm is super slow
			if (isNormal) {
				s.setbit(fbits);
				for (unsigned i = 0; i < fbits; ++i) { s.setbit(i, at(i)); }
			}
			else {
				// Find the MSB of the subnormal: 
				unsigned msb = 0;
				for (unsigned i = 0; i < fbits; ++i) { // msb protected from not being assigned through iszero test at prelude of function
					msb = fbits - 1ull - i;
					if (test(msb)) break;
				}
				//      m-----lsb
				// h00001010101
				// 101010100000
				for (unsigned i = 0; i <= msb; ++i) {
					s.setbit(fbits - msb + i, at(i));
				}
				shift = fhbits - msb;
			}
		}
		return shift;
	}
	template<unsigned targetbits>
	constexpr void bits(blockbinary<targetbits, bt>& b) const {
		unsigned upperbound = (nbits > targetbits ? targetbits : nbits);
		b.clear();
		for (unsigned i = 0; i < upperbound; ++i) { b.setbit(i, at(i)); }
	}

	// casts to native types
	int to_int() const {
		if (isnan()) return 0;
		if (isinf()) return sign() ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
		return int(to_native<float>());
	}
	long to_long() const {
		if (isnan()) return 0;
		if (isinf()) return sign() ? std::numeric_limits<long>::min() : std::numeric_limits<long>::max();
		return long(to_native<double>());
	}
	long long to_long_long() const {
		if (isnan()) return 0;
		if (isinf()) return sign() ? std::numeric_limits<long long>::min() : std::numeric_limits<long long>::max();
		return (long long)(to_native<double>());
	}

	// transform an cfloat to a native C++ floating-point. We are using the native
	// precision to compute, which means that all sub-values need to be representable 
	// by the native precision.
	// A more accurate approximation would require an adaptive precision algorithm
	// with a final rounding step.
	template<typename TargetFloat>
	TargetFloat to_native() const { 
		TargetFloat v{ 0.0 };
		if (iszero()) {
			// the optimizer might destroy the sign
			return sign() ? -TargetFloat(0) : TargetFloat(0);
		}
		else if (isnan()) {
			v = sign() ? std::numeric_limits<TargetFloat>::signaling_NaN() : std::numeric_limits<TargetFloat>::quiet_NaN();
		}
		else if (isinf()) {
			v = sign() ? -std::numeric_limits<TargetFloat>::infinity() : std::numeric_limits<TargetFloat>::infinity();
		}
		else { // TODO: this approach has catastrophic cancellation when nbits is large and native target float is too small
			TargetFloat f{ 0 };
			TargetFloat fbit{ 0.5 };
			for (int i = static_cast<int>(nbits - 2ull - es); i >= 0; --i) {
				f += at(static_cast<unsigned>(i)) ? fbit : TargetFloat(0);
				fbit *= TargetFloat(0.5);
			}
			blockbinary<es, bt> ebits;
			exponent(ebits);
			if constexpr (hasSubnormals) {
				if (ebits.iszero()) {
					// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
					TargetFloat exponentiation = TargetFloat(subnormal_exponent[es]); // precomputed values for 2^(2-2^(es-1))
					v = exponentiation * f;  // f is already f/2^fbits
					return sign() ? -v : v;
				}
			}
			else {
				if (ebits.iszero()) { // underflow to 0
					// compiler fast float optimization might destroy the sign
					return sign() ? -TargetFloat(0) : TargetFloat(0);
				}
			}
			if constexpr (hasSupernormals) {
				// regular: (-1)^s * 2^(e+1-2^(es-1)) * (1 + f/2^fbits))
				int exponent = static_cast<int>(unsigned(ebits) - EXP_BIAS);
				if (-64 < exponent && exponent < 64) {
					TargetFloat exponentiation = (exponent >= 0 ? TargetFloat(1ull << exponent) : (1.0f / TargetFloat(1ull << -exponent)));
					v = exponentiation * (TargetFloat(1.0) + f);
				}
				else {
					double exponentiation = ipow(exponent);
					v = TargetFloat(exponentiation * (1.0 + f));
				}
			}
			else {
				if (ebits.all()) {
					// supernormals are mapped to quiet NaNs
					v = std::numeric_limits<TargetFloat>::quiet_NaN();
					return v;
				}
				else {
					// regular: (-1)^s * 2^(e+1-2^(es-1)) * (1 + f/2^fbits))
					int exponent = static_cast<int>(unsigned(ebits) - EXP_BIAS);
					if (-64 < exponent && exponent < 64) {
						TargetFloat exponentiation = (exponent >= 0 ? TargetFloat(1ull << exponent) : (1.0f / TargetFloat(1ull << -exponent)));
						v = exponentiation * (TargetFloat(1.0) + f);
					}
					else {
						double exponentiation = ipow(exponent);
						v = TargetFloat(exponentiation * (1.0 + f));
					}
				}
			}
			v = sign() ? -v : v;
		}
		return v;
	}

	// convert a cfloat to a blocktriple with the fraction format 1.ffff
	// we are using the same block type so that we can use block copies to move bits around.
	// Since we tend to have at least two exponent bits, this will lead to
	// most cfloat<->blocktriple cases being efficient as the block types are aligned.
	// The relationship between the source cfloat and target blocktriple is not
	// arbitrary, enforce it by setting the blocktriple fbits to the cfloat's (nbits - es - 1)
	constexpr void normalize(blocktriple<fbits, BlockTripleOperator::REP, bt>& tgt) const {
		// test special cases
		if (isnan()) {
			tgt.setnan();
		}
		else if (isinf()) {
			tgt.setinf();
		}
		else if (iszero()) {
			tgt.setzero();
		}
		else {
			tgt.setnormal(); // a blocktriple is always normalized
			int scale = this->scale();
			tgt.setsign(sign());
			tgt.setscale(scale);
			// set significant
			// we are going to unify to the format 01.ffffeeee
			// where 'f' is a fraction bit, and 'e' is an extension bit
			// so that normalize can be used to generate blocktriples for add/sub/mul/div/sqrt
			if (isnormal()) {
				if constexpr (fbits < 64) { // max 63 bits of fraction to yield 64bit of raw significant bits
					uint64_t raw = fraction_ull();
					raw |= (1ull << fbits);
					tgt.setbits(raw);
				}
				else {
					blockcopy(tgt);
					tgt.setbit(fbits);
				}
			}
			else { // it is a subnormal encoding in this target cfloat
				int shift = MIN_EXP_NORMAL - scale;
				if constexpr (fbits < 64) {
					uint64_t raw = fraction_ull();
					raw <<= shift;
					raw |= (1ull << fbits);
					tgt.setbits(raw);
				}
				else {
					blockcopy(tgt);
					tgt <<= shift;
					tgt.setbit(fbits);
				}
			}
		}
	}

	// normalize a cfloat to a blocktriple used in add/sub, which has the form 00h.fffff
	// that is 3 + fbits, the 3 extra bits are required to be able to use 2's complement 
	// and capture the largest value of an addition/subtraction.
	// TODO: currently abits = 2*fhbits as the worst case input argument size to
	// capture the smallest normal value in aligned form. There is a faster/smaller
	// implementation where the input is constrainted to just the round, guard, and sticky bits.
	constexpr void normalizeAddition(blocktriple<fbits, BlockTripleOperator::ADD, bt>& tgt) const {
		using BlockTripleConfiguration = blocktriple<fbits, BlockTripleOperator::ADD, bt>;
		// test special cases
		if (isnan()) {
			tgt.setnan();
		}
		else if (isinf()) {
			tgt.setinf();
		}
		else if (iszero()) {
			tgt.setzero();
		}
		else {
			tgt.setnormal(); // a blocktriple is always normalized
			int scale = this->scale();
			tgt.setsign(sign());
			tgt.setscale(scale);
			// set significant
			// we are going to unify to the format 001.ffffeeee
			// where 'f' is a fraction bit, and 'e' is an extension bit
			// so that normalize can be used to generate blocktriples for add/sub/mul/div/sqrt
			if (isnormal()) {
				if constexpr (fbits < 64 && BlockTripleConfiguration::rbits < (64 - fbits)) {
					uint64_t raw = fraction_ull();
					raw |= (1ull << fbits); // add the hidden bit
					//std::cout << "normalize      : " << *this << '\n';
					//std::cout << "significant    : " << to_binary(raw, fbits + 2) << '\n';
					raw <<= BlockTripleConfiguration::rbits;  // rounding bits required for correct rounding
					//std::cout << "rounding shift : " << to_binary(raw, fbits + 2 + BlockTripleConfiguration::rbits) << '\n';
					tgt.setbits(raw);
				}
				else {
					blockcopy(tgt);
					tgt.setradix();
					tgt.setbit(fbits); // add the hidden bit
					tgt.bitShift(BlockTripleConfiguration::rbits);  // rounding bits required for correct rounding
				}
			}
			else {
				if (isdenormal()) { // it is a subnormal encoding in this target cfloat
					if constexpr (hasSubnormals) {
						if constexpr (BlockTripleConfiguration::rbits < (64 - fbits)) {
							uint64_t raw = fraction_ull();
							int shift = MIN_EXP_NORMAL - scale;
							raw <<= shift; // shift but do NOT add a hidden bit as the MSB of the subnormal is shifted in the hidden bit position
							raw <<= BlockTripleConfiguration::rbits;  // rounding bits required for correct rounding
							tgt.setbits(raw);
						}
						else {
							blockcopy(tgt);
							tgt.setradix();
							int shift = MIN_EXP_NORMAL - scale;
							tgt.bitShift(shift + BlockTripleConfiguration::rbits);  // rounding bits required for correct rounding
						}
					}
					else {  // this cfloat has no subnormals
						tgt.setzero(tgt.sign()); // preserve the sign
					}
				}
				else {
					// by design, a cfloat is either normal, subnormal, or supernormal, so this else clause is by deduction covering a supernormal
					if constexpr (hasSupernormals) {
						if constexpr (fbits < 64 && BlockTripleConfiguration::rbits < (64 - fbits)) {
							uint64_t raw = fraction_ull();
							raw |= (1ull << fbits); // add the hidden bit
							raw <<= BlockTripleConfiguration::rbits;  // rounding bits required for correct rounding
							tgt.setbits(raw);
						}
						else {
							blockcopy(tgt);
							tgt.setradix();
							tgt.setbit(fbits); // add the hidden bit
							tgt.bitShift(BlockTripleConfiguration::rbits);  // rounding bits required for correct rounding
						}
					}
					else {  // this cfloat has no supernormals and thus this represents a nan, signalling or quiet determined by the sign
						tgt.setnan(tgt.sign());
					}			
				}
			}
		}
		// tgt.setradix(radix);
	}

	// Normalize a cfloat to a blocktriple used in mul, which has the form 0'00001.fffff
	// that is 2*fbits, plus 1 overflow bit, and the radix set at <fbits>.
	// The result radix will go to 2*fbits after multiplication.
	constexpr void normalizeMultiplication(blocktriple<fbits, BlockTripleOperator::MUL, bt>& tgt) const {
		// test special cases
		if (isnan()) {
			tgt.setnan();
		}
		else if (isinf()) {
			tgt.setinf();
		}
		else if (iszero()) {
			tgt.setzero();
		}
		else {
			tgt.setnormal(); // a blocktriple is always normalized
			int scale = this->scale();
			tgt.setsign(sign());
			tgt.setscale(scale);

			// set significant
			// we are going to unify to the format 01.ffffeeee
			// where 'f' is a fraction bit, and 'e' is an extension bit
			// so that normalize can be used to generate blocktriples for add/sub/mul/div/sqrt
			if (isnormal() || issupernormal()) {
				if constexpr (fbits < 64) { // max 63 bits of fraction to yield 64bit of raw significant bits
					uint64_t raw = fraction_ull();
					raw |= (1ull << fbits);
					tgt.setbits(raw);
				}
				else {
					blockcopy(tgt);
					tgt.setradix();
					tgt.setbit(fbits); // add the hidden bit
				}
			}
			else { 
				// it is a subnormal encoding in this target cfloat
				if constexpr (hasSubnormals) {
					if constexpr (fbits < 64) {
						uint64_t raw = fraction_ull();
						int shift = MIN_EXP_NORMAL - scale;
						raw <<= shift;
						raw |= (1ull << fbits);
						tgt.setbits(raw);
					}
					else {
						blockcopy(tgt);
						int shift = MIN_EXP_NORMAL - scale;
						tgt.bitShift(shift);
						tgt.setbit(fbits);
					}
				}
				else { // this cfloat has no subnormals
					tgt.setzero(tgt.sign()); // preserve the sign
				}
			}
		}
		tgt.setradix(fbits); // override the radix with the input scale for accurate value printing
	}

	// normalize a cfloat to a blocktriple used in div, which has the form 0'00000'00001.fffff
	// that is 3*fbits, plus 1 overflow bit, and the radix set at <fbits>.
	// the result radix will go to 2*fbits after multiplication.
	// TODO: needs implementation
	constexpr void normalizeDivision(blocktriple<fbits, BlockTripleOperator::DIV, bt>& tgt) const {
		constexpr unsigned divshift = blocktriple<fbits, BlockTripleOperator::DIV, bt>::divshift;
		// test special cases
		if (isnan()) {
			tgt.setnan();
		}
		else if (isinf()) {
			tgt.setinf();
		}
		else if (iszero()) {
			tgt.setzero();
		}
		else {
			tgt.setnormal(); // a blocktriple is always normalized
			int scale = this->scale();
			tgt.setsign(sign());
			tgt.setscale(scale);
			// set significant
			// we are going to unify to the format 01.ffffeeee
			// where 'f' is a fraction bit, and 'e' is an extension bit
			// so that normalize can be used to generate blocktriples for add/sub/mul/div/sqrt
			if (isnormal() || issupernormal()) {
				if constexpr (fbits < 64 && divshift < (64 - fbits)) {
					uint64_t raw = fraction_ull();
					raw |= (1ull << fbits);
					raw <<= divshift; // shift the input value to the output radix
					tgt.setbits(raw);
				}
				else {
					// brute force copy of blocks
					blockcopy(tgt);
					tgt.setbit(fbits);
					tgt.bitShift(divshift); // shift the input value to the output radix
				}
			}
			else { // it is a subnormal encoding in this target cfloat
				if constexpr (fbits < 64 && divshift < (64 - fbits)) {
					uint64_t raw = fraction_ull();
					int shift = MIN_EXP_NORMAL - scale;
					raw <<= shift;
					raw |= (1ull << fbits);
					raw <<= divshift; // shift the input value to the output radix
					tgt.setbits(raw);
				}
				else {
					blockcopy(tgt);
					int shift = MIN_EXP_NORMAL - scale;
					tgt.bitShift(shift);
					tgt.setbit(fbits);
					tgt.bitShift(divshift); // shift the input value to the output radix
				}
			}
		}
		tgt.setradix(blocktriple<fbits, BlockTripleOperator::DIV, bt>::radix);
	}

	// helper debug function
	void constexprClassParameters() const noexcept {
		std::cout << "-------------------------------------------------------------\n";
		std::cout << "type              : " << typeid(*this).name() << '\n';
		std::cout << "nbits             : " << nbits << '\n';
		std::cout << "es                : " << es << std::endl;
		std::cout << "hasSubnormals     : " << (hasSubnormals ? "true" : "false") << '\n';
		std::cout << "hasSupernormals   : " << (hasSupernormals ? "true" : "false") << '\n';
		std::cout << "isSaturating      : " << (isSaturating ? "true" : "false") << '\n';
		std::cout << "ALL_ONES          : " << to_binary(ALL_ONES, 0, true) << '\n';
		std::cout << "BLOCK_MASK        : " << to_binary(BLOCK_MASK, 0, true) << '\n';
		std::cout << "nrBlocks          : " << nrBlocks << '\n';
		std::cout << "bits in MSU       : " << bitsInMSU << '\n';
		std::cout << "MSU               : " << MSU << '\n';
		std::cout << "MSU MASK          : " << to_binary(MSU_MASK, 0, true) << '\n';
		std::cout << "SIGN_BIT_MASK     : " << to_binary(SIGN_BIT_MASK, 0, true) << '\n';
		std::cout << "LSB_BIT_MASK      : " << to_binary(LSB_BIT_MASK, 0, true) << '\n';
		std::cout << "MSU CAPTURES_EXP  : " << (MSU_CAPTURES_EXP ? "yes\n" : "no\n");
		std::cout << "EXP_SHIFT         : " << EXP_SHIFT << '\n';
		std::cout << "MSU EXP MASK      : " << to_binary(MSU_EXP_MASK, 0, true) << '\n';
		std::cout << "ALL_ONE_MASK_ES   : " << to_binary(ALL_ONES_ES) << '\n';
		std::cout << "EXP_BIAS          : " << EXP_BIAS << '\n';
		std::cout << "MAX_EXP           : " << MAX_EXP << '\n';
		std::cout << "MIN_EXP_NORMAL    : " << MIN_EXP_NORMAL << '\n';
		std::cout << "MIN_EXP_SUBNORMAL : " << MIN_EXP_SUBNORMAL << '\n';
		std::cout << "fraction Blocks   : " << fBlocks << '\n';
		std::cout << "bits in FSU       : " << bitsInFSU << '\n';
		std::cout << "FSU               : " << FSU << '\n';
		std::cout << "FSU MASK          : " << to_binary(FSU_MASK, 0, true) << '\n';
		std::cout << "topfbits          : " << topfbits << '\n';
		std::cout << "ALL_ONE_MASK_FR   : " << to_binary(ALL_ONES_FR) << '\n';
	}
	void showLimbs() const {
		for (unsigned b = 0; b < nrBlocks; ++b) {
			std::cout << to_binary(_block[nrBlocks - b - 1], sizeof(bt) * 8) << ' ';
		}
		std::cout << '\n';
	}

protected:
	// HELPER methods

	/// <summary>
	/// 1's complement of the encoding used to set up specific encoding patterns.
	/// This is not an arithmetic operator that makes sense for floating-point numbers.
	/// </summary>
	/// <returns>reference to this cfloat object</returns>
	constexpr cfloat& flip() noexcept { // in-place one's complement
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}

	/// <summary>
	/// shift left is a bit level encoding helper for fast limb-based conversions between different cfloats
	/// </summary>
	/// <param name="bitsToShift"></param>
	void shiftLeft(unsigned bitsToShift) {
		if (bitsToShift == 0) return;
		if (bitsToShift > nbits) {
			setzero();
		}
		if (bitsToShift >= bitsInBlock) {
			int blockShift = static_cast<int>(bitsToShift / bitsInBlock);
			for (int i = static_cast<int>(MSU); i >= blockShift; --i) {
				_block[i] = _block[i - blockShift];
			}
			for (int i = blockShift - 1; i >= 0; --i) {
				_block[i] = bt(0);
			}
			// adjust the shift
			bitsToShift -= blockShift * bitsInBlock;
			if (bitsToShift == 0) return;
		}
		if constexpr (MSU > 0) {
			// construct the mask for the upper bits in the block that need to move to the higher word
			bt mask = 0xFFFFFFFFFFFFFFFF << (bitsInBlock - bitsToShift);
			for (unsigned i = MSU; i > 0; --i) {
				_block[i] <<= bitsToShift;
				// mix in the bits from the right
				bt bits = bt(mask & _block[i - 1]);
				_block[i] |= (bits >> (bitsInBlock - bitsToShift));
			}
		}
		_block[0] <<= bitsToShift;
	}

	// convert an unsigned integer into a cfloat
	// TODO: this method does not protect against being called with a signed integer
	template<typename Ty>
	constexpr cfloat& convert_unsigned_integer(const Ty& rhs) noexcept {
		clear();
		if (0 == rhs) return *this;

		uint64_t raw = static_cast<uint64_t>(rhs);
		int msb = static_cast<int>(find_msb(raw)) - 1; // msb > 0 due to zero test above 
		int exponent = msb;
		// remove the MSB as it represents the hidden bit in the cfloat representation
		uint64_t hmask = ~(1ull << msb);
		raw &= hmask;

		constexpr uint32_t sizeInBits = 8 * sizeof(Ty);
		uint32_t shift = sizeInBits - exponent - 1;
		raw <<= shift;
		raw = round<sizeInBits, uint64_t>(raw, exponent);

		// construct the target cfloat
		if constexpr (fbits < (64 - es)) {
			uint64_t biasedExponent = static_cast<uint64_t>(static_cast<long long>(exponent) + static_cast<long long>(EXP_BIAS));
			uint64_t bits = 0;
			bits <<= es;
			bits |= biasedExponent;
			bits <<= fbits;
			bits |= raw;
			setbits(bits);
		}
		else {
			setsign(false);
			setexponent(exponent);
			setfraction(raw);
		}
		return *this;
	}
	// convert a signed integer into a cfloat
	// TODO: this method does not protect against being called with a signed integer
	template<typename Ty>
	constexpr cfloat& convert_signed_integer(const Ty& rhs) noexcept {
		clear();
		if (0 == rhs) return *this;
		bool s = (rhs < 0);
		using UnsignedTy = std::make_unsigned_t<Ty>;
		UnsignedTy urhs = static_cast<UnsignedTy>(rhs);
		uint64_t raw = static_cast<uint64_t>(s ? (UnsignedTy(0) - urhs) : urhs);

		int msb = static_cast<int>(find_msb(raw)) - 1; // msb > 0 due to zero test above 
		int exponent = msb;
		// remove the MSB as it represents the hidden bit in the cfloat representation
		uint64_t hmask = ~(1ull << msb);
		raw &= hmask;

		// shift the msb to the msb of the fraction
		constexpr uint32_t sizeInBits = 8 * sizeof(Ty);
		uint32_t shift = sizeInBits - exponent - 1;
		raw <<= shift;
		raw = round<sizeInBits, uint64_t>(raw, exponent);

		// construct the target cfloat
		if constexpr (fbits < (64 - es)) {
			uint64_t biasedExponent = static_cast<uint64_t>(static_cast<long long>(exponent) + static_cast<long long>(EXP_BIAS));
			uint64_t bits = (s ? 1ull : 0ull);
			bits <<= es;
			bits |= biasedExponent;
			bits <<= fbits;
			bits |= raw;
			setbits(bits);
		}
		else {
			setsign(s);
			setexponent(exponent);
			setfraction(raw);
		}
		return *this;
	}

public:
	template<typename Real>
	CONSTEXPRESSION cfloat& convert_ieee754(Real rhs) noexcept {
		if constexpr (nbits == 32 && es == 8 && sizeof(Real) == 4) {
			// we CANNOT use the native conversion to float as cfloats have supernormals
			// which IEEE-754 does not have and thus a native conversion would destroy
			// only if the Real type is a float can we use the direct conversion

			// when our cfloat is a perfect match to single precision IEEE-754
			bool s{ false };
			uint64_t rawExponent{ 0 };
			uint64_t rawFraction{ 0 };
			uint64_t bits{ 0 };
			extractFields(rhs, s, rawExponent, rawFraction, bits);
			if (rawExponent == ieee754_parameter<Real>::eallset) { // nan and inf need to be remapped
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::snanmask) ||
					rawFraction == (ieee754_parameter<Real>::fmask & (ieee754_parameter<Real>::qnanmask | ieee754_parameter<Real>::snanmask))) {
					// 1.11111111.00000000.......00000001 signalling nan
					// 0.11111111.00000000000000000000001 signalling nan
					// MSVC
					// 1.11111111.10000000.......00000001 signalling nan
					// 0.11111111.10000000.......00000001 signalling nan
					setnan(NAN_TYPE_SIGNALLING);
					//setsign(s);  a cfloat encodes a signalling nan with sign = 1, and a quiet nan with sign = 0
					return *this;
				}
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask)) {
					// 1.11111111.10000000.......00000000 quiet nan
					// 0.11111111.10000000.......00000000 quiet nan
					setnan(NAN_TYPE_QUIET);
					//setsign(s);  a cfloat encodes a signalling nan with sign = 1, and a quiet nan with sign = 0
					return *this;
				}
				if (rawFraction == 0ull) {
					// 1.11111111.0000000.......000000000 -inf
					// 0.11111111.0000000.......000000000 +inf
					setinf(s);
					return *this;
				}
			}
			uint64_t raw{ s ? 1ull : 0ull };
			raw <<= 31;
			raw |= (rawExponent << fbits);
			raw |= rawFraction;
			setbits(raw);
			return *this;
		}
		else if constexpr (nbits == 64 && es == 11 && sizeof(Real) == 8) {
			// when our cfloat is a perfect match to double precision IEEE-754
			bool s{ false };
			uint64_t rawExponent{ 0 };
			uint64_t rawFraction{ 0 };
			uint64_t bits{ 0 };
			extractFields(rhs, s, rawExponent, rawFraction, bits);
			if (rawExponent == ieee754_parameter<Real>::eallset) { // nan and inf need to be remapped
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::snanmask) ||
					rawFraction == (ieee754_parameter<Real>::fmask & (ieee754_parameter<Real>::qnanmask | ieee754_parameter<Real>::snanmask))) {
					// 1.11111111.00000000.......00000001 signalling nan
					// 0.11111111.00000000000000000000001 signalling nan
					// MSVC
					// 1.11111111.10000000.......00000001 signalling nan
					// 0.11111111.10000000.......00000001 signalling nan
					setnan(NAN_TYPE_SIGNALLING);
					//setsign(s);  a cfloat encodes a signalling nan with sign = 1, and a quiet nan with sign = 0
					return *this;
				}
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask)) {
					// 1.11111111.10000000.......00000000 quiet nan
					// 0.11111111.10000000.......00000000 quiet nan
					setnan(NAN_TYPE_QUIET);
					//setsign(s);  a cfloat encodes a signalling nan with sign = 1, and a quiet nan with sign = 0
					return *this;
				}
				if (rawFraction == 0ull) {
					// 1.11111111.0000000.......000000000 -inf
					// 0.11111111.0000000.......000000000 +inf
					setinf(s);
					return *this;
				}
			}
			// normal and subnormal handling
			uint64_t raw{ s ? 1ull : 0ull };
			raw <<= 63;
			raw |= (rawExponent << fbits);
			raw |= rawFraction;
			setbits(raw);
			return *this;
		}
		else {
			clear();
			// extract raw IEEE-754 bits
			bool s{ false };
			uint64_t rawExponent{ 0 };
			uint64_t rawFraction{ 0 };
			uint64_t bits{ 0 };
			extractFields(rhs, s, rawExponent, rawFraction, bits);
			// special case handling
			if (rawExponent == ieee754_parameter<Real>::eallset) { // nan and inf
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::snanmask) ||
					rawFraction == (ieee754_parameter<Real>::fmask & (ieee754_parameter<Real>::qnanmask | ieee754_parameter<Real>::snanmask))) {
					// 1.11111111.00000000.......00000001 signalling nan
					// 0.11111111.00000000000000000000001 signalling nan
					// MSVC
					// 1.11111111.10000000.......00000001 signalling nan
					// 0.11111111.10000000.......00000001 signalling nan
					setnan(NAN_TYPE_SIGNALLING);
					//setsign(s);  a cfloat encodes a signalling nan with sign = 1, and a quiet nan with sign = 0
					return *this;
				}
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask)) {
					// 1.11111111.10000000.......00000000 quiet nan
					// 0.11111111.10000000.......00000000 quiet nan
					setnan(NAN_TYPE_QUIET);
					//setsign(s);  a cfloat encodes a signalling nan with sign = 1, and a quiet nan with sign = 0
					return *this;
				}
				if (rawFraction == 0ull) {
					// 1.11111111.0000000.......000000000 -inf
					// 0.11111111.0000000.......000000000 +inf
					setinf(s);
					return *this;
				}
			}
			if (rhs == 0.0) { // IEEE rule: this is valid for + and - 0.0
				setbit(nbits - 1ull, s);
				return *this;
			}
	
			// normal number consists of fbits fraction bits and one hidden bit
			// subnormal number has no hidden bit
			int exponent = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;  // unbias the exponent

			// check special case of 
			//  1- saturating to maxpos/maxneg, or 
			//  2- projecting to +-inf 
			// if the value is out of range.
			// 
			// One problem here is that at the rounding cusps of maxpos <-> inf <-> nan
			// you need to go through the rounding logic to know which encoding you end up
			// with. 
			// For each specific cfloat configuration, you can work out these rounding cusps
			// but they need to go through the value transformation to map them back to native
			// IEEE-754. That is a complex computation to do as a static constexpr as you need
			// to construct the value, then evaluate it, and store it.
			// 
			// The algorithm used here is to check for the obvious out of range values by
			// comparing their scale to the max scale this cfloat encoding can represent.
			// For the rounding cusps, we go through the rounding logic, and then clean up
			// after rounding using the observation that no conversion from a value can ever
			// yield the NaN encoding.
			//
			// The rounding logic will correctly sort between maxpos and inf, and we clean
			// up any NaN encodings by projecting back to the configuration's saturation rule.
			//
			// We could improve on this by creating the database of rounding cusps and
			// referencing them with a direct value comparison with the input. That would be
			// the most performant implementation.
			if (exponent > MAX_EXP) {
				if constexpr (isSaturating) {
					if (s) this->maxneg(); else this->maxpos(); // saturate to maxpos or maxneg
				}
				else {
					setinf(s);
				}
				return *this;
			}
			if constexpr (hasSubnormals) {
				if (exponent < MIN_EXP_SUBNORMAL - 1) { 
					// map to +-0 any values that have a scale less than (MIN_EXP_SUBMORNAL - 1)
					this->setbit(nbits - 1, s);
					return *this;
				}
			}
			else {
				if (exponent < MIN_EXP_NORMAL - 1) {
					// map to +-0 any values that have a scale less than (MIN_EXP_MORNAL - 1)
					this->setbit(nbits - 1, s);
					return *this;
				}
			}

			/////////////////  
			/// end of special case processing, move on to value sampling and rounding

#if TRACE_CONVERSION
			std::cout << '\n';
			std::cout << "value             : " << rhs << '\n';
			std::cout << "segments          : " << to_binary(rhs) << '\n';
			std::cout << "sign     bit      : " << (s ? '1' : '0') << '\n';
			std::cout << "exponent bits     : " << to_binary(rawExponent, ieee754_parameter<Real>::ebits, true) << '\n';
			std::cout << "fraction bits     : " << to_binary(rawFraction, ieee754_parameter<Real>::fbits, true) << '\n';
			std::cout << "exponent value    : " << exponent << '\n';
#endif

			// do the following scenarios have different rounding bits?
			// input is normal, cfloat is normal           <-- rounding can happen with native ieee-754 bits
			// input is normal, cfloat is subnormal
			// input is subnormal, cfloat is normal
			// input is subnormal, cfloat is subnormal

			// The first condition is the relationship between the number 
			// of fraction bits from the source and the number of fraction bits 
			// in the target cfloat: these are constexpressions and guard the shifts
			// input fbits >= cfloat fbits                 <-- need to round
			// input fbits < cfloat fbits                  <-- no need to round

			// quick check if we are truncating to 0 for all subnormal values
			if constexpr (!hasSubnormals) {
				if (exponent < MIN_EXP_NORMAL) {
					setsign(s); // rest of the bits, exponent and fraction, are already set correctly
					return *this;
				}
			}
			if constexpr (fbits < ieee754_parameter<Real>::fbits) {
				// this is the common case for cfloats that are smaller in size compared to single and double precision IEEE-754
				constexpr int rightShift = ieee754_parameter<Real>::fbits - fbits; // this is the bit shift to get the MSB of the src to the MSB of the tgt
				uint32_t biasedExponent{ 0 };
				int adjustment{ 0 }; // right shift adjustment for subnormal representation
				uint64_t mask;
				if (rawExponent != 0) {
					// the source real is a normal number, 
//					if (exponent >= (MIN_EXP_SUBNORMAL - 1) && exponent < MIN_EXP_NORMAL) {
					if (exponent < MIN_EXP_NORMAL) {
//						exponent = (exponent < MIN_EXP_SUBNORMAL ? MIN_EXP_SUBNORMAL : exponent); // clip to the smallest subnormal exponent, otherwise the adjustment is off
						// the value is a subnormal number in this representation: biasedExponent = 0
						// add the hidden bit to the fraction bits so the denormalization has the correct MSB
						rawFraction |= ieee754_parameter<Real>::hmask;

						// fraction processing: we have 1 hidden + 23 explicit fraction bits 
						// f = 1.ffff 2^exponent * 2^fbits * 2^-(2-2^(es-1)) = 1.ff...ff >> (23 - (-exponent + fbits - (2 -2^(es-1))))
						// -exponent because we are right shifting and exponent in this range is negative
						adjustment = -(exponent + subnormal_reciprocal_shift[es]);
						// this is the right shift adjustment required for subnormal representation due 
						// to the scale of the input number, i.e. the exponent of 2^-adjustment
					}
					else {
						// the value is a normal number in this representation: common case
						biasedExponent = static_cast<uint32_t>(exponent + EXP_BIAS); // project the exponent into the target 
						// fraction processing
						// float structure is: seee'eeee'efff'ffff'ffff'ffff'ffff'ffff, s = sign, e - exponent bit, f = fraction bit
						// target structure is for example cfloat<8,2>: seef'ffff
						// since both are normals, we can shift the incoming fraction to the target structure bits, and round
						// MSB of source = 23 - 1, MSB of target = fbits - 1: shift = MSB of src - MSB of tgt => 23 - fbits
						adjustment = 0;
					}
					if constexpr (rightShift > 0) {
						// if true we need to round
						// round-to-even logic
						//  ... lsb | guard  round sticky   round
						//       x     0       x     x       down
						//       0     1       0     0       down  round to even
						//       1     1       0     0        up   round to even
						//       x     1       0     1        up
						//       x     1       1     0        up
						//       x     1       1     1        up
						// collect lsb, guard, round, and sticky bits


#if TRACE_CONVERSION
						std::cout << "fraction bits     : " << to_binary(rawFraction, ieee754_parameter<Real>::nbits, true) << '\n';
						std::cout << "lsb mask bits     : " << to_binary(mask, ieee754_parameter<Real>::nbits, true) << '\n';
#endif
						mask = (1ull << (rightShift + adjustment)); // bit mask for the lsb bit
						bool lsb = (mask & rawFraction);
						mask >>= 1;
						bool guard = (mask & rawFraction);
						mask >>= 1;
						bool round = (mask & rawFraction);
						if ((rightShift + adjustment) > 1) {
							mask = (0xFFFF'FFFF'FFFF'FFFFull << (rightShift + adjustment - 2));
							mask = ~mask;
						}
						else {
							mask = 0;
						}
#if TRACE_CONVERSION
						std::cout << "right shift       : " << rightShift << '\n';
						std::cout << "adjustment        : " << adjustment << '\n';
						std::cout << "shift to LSB      : " << (rightShift + adjustment) << '\n';
						std::cout << "fraction bits     : " << to_binary(rawFraction, ieee754_parameter<Real>::nbits, true) << '\n';
						std::cout << "sticky mask bits  : " << to_binary(mask, ieee754_parameter<Real>::nbits, true) << '\n';
#endif
						bool sticky = (mask & rawFraction);
						rawFraction >>= (static_cast<int64_t>(rightShift) + static_cast<int64_t>(adjustment));

						// execute rounding operation
						if (guard) {
							if (lsb && (!round && !sticky)) ++rawFraction; // round to even
							if (round || sticky) ++rawFraction;
							if (rawFraction == (1ull << fbits)) { // overflow
								if (biasedExponent == ALL_ONES_ES) { // overflow to INF == .111..01
									rawFraction = INF_ENCODING;
								}
								else {
									++biasedExponent;
									rawFraction = 0;
								}
							}
						}
#if TRACE_CONVERSION
						std::cout << "lsb               : " << (lsb ? "1\n" : "0\n");
						std::cout << "guard             : " << (guard ? "1\n" : "0\n");
						std::cout << "round             : " << (round ? "1\n" : "0\n");
						std::cout << "sticky            : " << (sticky ? "1\n" : "0\n");
						std::cout << "rounding decision : " << (lsb && (!round && !sticky) ? "round to even\n" : "-\n");
						std::cout << "rounding direction: " << (round || sticky ? "round up\n" : "round down\n");
#endif
					}
					else { // all bits of the float go into this representation and need to be shifted up, no rounding necessary
						int shiftLeft = fbits - ieee754_parameter<Real>::fbits;
						rawFraction <<= shiftLeft;
					}
#if TRACE_CONVERSION
					std::cout << "biased exponent   : " << biasedExponent << " : 0x" << std::hex << biasedExponent << std::dec << '\n';
					std::cout << "right shift       : " << rightShift << '\n';
					std::cout << "adjustment shift  : " << adjustment << '\n';
					std::cout << "sticky bit mask   : " << to_binary(mask, 32, true) << '\n';
					std::cout << "fraction bits     : " << to_binary(rawFraction, 32, true) << '\n';
#endif
					// construct the target cfloat
					bits = (s ? 1ull : 0ull);
					bits <<= es;
					bits |= biasedExponent;
					bits <<= fbits;
					bits |= rawFraction;
#if TRACE_CONVERSION
					std::cout << "sign bit          : " << (s ? '1' : '0') << '\n';
					std::cout << "biased exponent   : " << biasedExponent << " : 0x" << std::hex << biasedExponent << std::dec << '\n';
					std::cout << "fraction bits     : " << to_binary(rawFraction, 32, true) << '\n';
					std::cout << "cfloat bits       : " << to_binary(bits, nbits, true) << '\n';
#endif
					setbits(bits);
				}
				else {
					// the source real is a subnormal number				
					mask = 0x00FF'FFFFu >> (fbits + exponent + subnormal_reciprocal_shift[es] + 1); // mask for sticky bit 

					// fraction processing: we have fbits+1 bits = 1 hidden + fbits explicit fraction bits 
					// f = 1.ffff  2^exponent * 2^fbits * 2^-(2-2^(es-1)) = 1.ff...ff >> (23 - (-exponent + fbits - (2 -2^(es-1))))
					// -exponent because we are right shifting and exponent in this range is negative
					adjustment = -(exponent + subnormal_reciprocal_shift[es]); // this is the right shift adjustment due to the scale of the input number, i.e. the exponent of 2^-adjustment
#if TRACE_CONVERSION					
					std::cout << "source is subnormal: TBD\n";
					std::cout << "shift to LSB    " << (rightShift + adjustment) << '\n';
					std::cout << "adjustment      " << adjustment << '\n';
					std::cout << "exponent        " << exponent << '\n';
					std::cout << "subnormal shift " << subnormal_reciprocal_shift[es] << '\n';
#endif
					if (exponent >= (MIN_EXP_SUBNORMAL - 1) && exponent < MIN_EXP_NORMAL) {
						// the value is a subnormal number in this representation
					}
					else {
						// the value is a normal number in this representation
					}
				}
			}
			else {
				// no need to round, but we need to shift left to deliver the bits
				// cfloat<40,  8> = float
				// cfloat<48,  9> = float
				// cfloat<56, 10> = float
				// cfloat<64, 11> = float
				// cfloat<64, 10> = double 
				// can we go from an input subnormal to a cfloat normal? 
				// yes, for example a cfloat<64,11> assigned to a subnormal float

				// map exponent into target cfloat encoding
				uint64_t biasedExponent = static_cast<uint64_t>(static_cast<int64_t>(exponent) + EXP_BIAS);
				constexpr int upshift = fbits - ieee754_parameter<Real>::fbits;
				// output processing
				if constexpr (nbits < 65) {
					// we can compose the bits in a native 64-bit unsigned integer
					// common case: normal to normal
					// reference example: nbits = 40, es = 8, fbits = 31: rhs = float fbits = 23; shift left by (31 - 23) = 8

					if (rawExponent != 0) {
						// rhs is a normal encoding
						uint64_t raw{ s ? 1ull : 0ull };
						raw <<= es;
						raw |= biasedExponent;
						raw <<= fbits;
						rawFraction <<= upshift;
						raw |= rawFraction;
						setbits(raw);
					}
					else {
						// rhs is a subnormal
	//					std::cerr << "rhs is a subnormal : " << to_binary(rhs) << " : " << rhs << '\n';
						// we need to calculate the effective scale to see 
						// if this value becomes a normal, or maps to a subnormal encoding
						// in this target format
					}
				}
				else {
					// we need to write and shift bits into place
					// use cases are cfloats like cfloat<80, 11, bt>
					// even though the bits that come in are single or double precision
					// we need to write the fields and then shifting them in place
					// 
					// common case: normal to normal
					if constexpr (bitsInBlock < 64) {
						if (rawExponent != 0) {
							// reference example: nbits = 128, es = 15, fbits = 112: rhs = float: shift left by (112 - 23) = 89
							setbits(biasedExponent);
							shiftLeft(fbits);
							bt fractionBlock[nrBlocks]{ 0 };
							// copy fraction bits
							unsigned blocksRequired = (8 * sizeof(rawFraction) + 1) / bitsInBlock;
							unsigned maxBlockNr = (blocksRequired < nrBlocks ? blocksRequired : nrBlocks);
							uint64_t mask = static_cast<uint64_t>(ALL_ONES); // set up the block mask
							unsigned shift = 0;
							for (unsigned i = 0; i < maxBlockNr; ++i) {
								fractionBlock[i] = bt((mask & rawFraction) >> shift);
								mask <<= bitsInBlock;
								shift += bitsInBlock;
							}
							// shift fraction bits
							int bitsToShift = upshift;
							if (bitsToShift >= static_cast<int>(bitsInBlock)) {
								int blockShift = static_cast<int>(bitsToShift / bitsInBlock);
								for (int i = MSU; i >= blockShift; --i) {
									fractionBlock[i] = fractionBlock[i - blockShift];
								}
								for (int i = blockShift - 1; i >= 0; --i) {
									fractionBlock[i] = bt(0);
								}
								// adjust the shift
								bitsToShift -= blockShift * bitsInBlock;
							}
							if (bitsToShift > 0) {
								// construct the mask for the upper bits in the block that need to move to the higher word
								bt bitsToMoveMask = bt(ALL_ONES << (bitsInBlock - bitsToShift));
								for (unsigned i = MSU; i > 0; --i) {
									fractionBlock[i] <<= bitsToShift;
									// mix in the bits from the right
									bt fracbits = static_cast<bt>(bitsToMoveMask & fractionBlock[i - 1]); // operator & yields an int
									fractionBlock[i] |= (fracbits >> (bitsInBlock - bitsToShift));
								}
								fractionBlock[0] <<= bitsToShift;
							}
							// OR the bits in
							for (unsigned i = 0; i <= MSU; ++i) {
								_block[i] |= fractionBlock[i];
							}
							// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
							_block[MSU] &= MSU_MASK;
							// finally, set the sign bit
							setsign(s);
						}
						else {
							// rhs is a subnormal
		//					std::cerr << "rhs is a subnormal : " << to_binary(rhs) << " : " << rhs << '\n';
						}
					}
					else {
						// BlockType is incorrect
					}
				}
			}
			// post-processing results to implement saturation and projection after rounding logic
			if constexpr (isSaturating) {
				if (isinf(INF_TYPE_POSITIVE) || isnan(NAN_TYPE_QUIET)) {
					maxpos();
				}
				else if (isinf(INF_TYPE_NEGATIVE) || isnan(NAN_TYPE_SIGNALLING)) {
					maxneg();
				}
			}
			else {
				if (isnan(NAN_TYPE_QUIET)) {
					setinf(false);
				}
				else if (isnan(NAN_TYPE_SIGNALLING)) {
					setinf(true);
				}
			}
			return *this;  // TODO: unreachable in some configurations	
		}
	}

	// post-processing results to implement saturation and projection after rounding logic
	// arithmetic bit operations can't produce NaN encodings, so we need to re-interpret
	// these encodings and 'project' them to the proper values.
	void constexpr post_process() noexcept {
		if constexpr (isSaturating) {
			if (isinf(INF_TYPE_POSITIVE) || isnan(NAN_TYPE_QUIET)) {
				maxpos();
			}
			else if (isinf(INF_TYPE_NEGATIVE) || isnan(NAN_TYPE_SIGNALLING)) {
				maxneg();
			}
		}
		else {
			if (isnan(NAN_TYPE_QUIET)) {
				setinf(false);
			}
			else if (isnan(NAN_TYPE_SIGNALLING)) {
				setinf(true);
			}
		}
	}

protected:

	/// <summary>
	/// round a set of source bits to the present representation.
	/// srcbits is the number of bits of significant in the source representation
	/// </summary>
	/// <typeparam name="StorageType"></typeparam>
	/// <param name="raw"></param>
	/// <returns></returns>
	template<unsigned srcbits, typename StorageType>
	constexpr uint64_t round(StorageType raw, int& exponent) noexcept {
		if constexpr (fhbits < srcbits) {
			// round to even: lsb guard round sticky
		    // collect guard, round, and sticky bits
		    // this same logic will work for the case where
		    // we only have a guard bit and no round and sticky bits
		    // because the mask logic will make round and sticky both 0
			constexpr uint32_t shift = srcbits - fhbits - 1ull;
			StorageType mask = (StorageType(1ull) << shift);
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			if constexpr (shift > 1u) { // protect against a negative shift
				StorageType allones(StorageType(~0));
				mask = StorageType(allones << (shift - 2));
				mask = ~mask;
			}
			else {
				mask = 0;
			}
			bool sticky = (mask & raw);

			raw >>= (shift + 1);  // shift out the bits we are rounding away
			bool lsb = (raw & 0x1u);
			//  ... lsb | guard  round sticky   round
			//       x     0       x     x       down
			//       0     1       0     0       down  round to even
			//       1     1       0     0        up   round to even
			//       x     1       0     1        up
			//       x     1       1     0        up
			//       x     1       1     1        up
			if (guard) {
				if (lsb && (!round && !sticky)) ++raw; // round to even
				if (round || sticky) ++raw;
				if (raw == (1ull << fbits)) { // overflow
					++exponent;
					raw >>= 1u;
				}
			}
		}
		else {
			constexpr unsigned shift = fhbits - srcbits;
			if constexpr (shift < (sizeof(StorageType) * 8)) {
				raw <<= shift;
			}
			else {
				std::cerr << "round: shift " << shift << " >= " << sizeof(StorageType) << std::endl;
				raw = 0;
			}
		}
		uint64_t significant = raw;
		return significant;
	}

	template<typename ArgumentBlockType>
	constexpr void copyBits(ArgumentBlockType v) {
		unsigned blocksRequired = (8 * sizeof(v) + 1 ) / bitsInBlock;
		unsigned maxBlockNr = (blocksRequired < nrBlocks ? blocksRequired : nrBlocks);
		bt b{ 0ul }; b = bt(~b);
		ArgumentBlockType mask = ArgumentBlockType(b);
		unsigned shift = 0;
		for (unsigned i = 0; i < maxBlockNr; ++i) {
			_block[i] = bt((mask & v) >> shift);
			mask <<= bitsInBlock;
			shift += bitsInBlock;
		}
	}
	void shiftLeft(int leftShift) {
		if (leftShift == 0) return;
		if (leftShift < 0) return shiftRight(-leftShift);
		if (leftShift > long(nbits)) leftShift = nbits; // clip to max
		if (leftShift >= long(bitsInBlock)) {
			int blockShift = leftShift / static_cast<int>(bitsInBlock);
			for (signed i = signed(MSU); i >= blockShift; --i) {
				_block[i] = _block[i - blockShift];
			}
			for (signed i = blockShift - 1; i >= 0; --i) {
				_block[i] = bt(0);
			}
			// adjust the shift
			leftShift -= (long)(blockShift * bitsInBlock);
			if (leftShift == 0) return;
		}
		// construct the mask for the upper bits in the block that need to move to the higher word
//		bt mask = static_cast<bt>(0xFFFFFFFFFFFFFFFFull << (bitsInBlock - leftShift));
		bt mask = ALL_ONES;
		mask <<= (bitsInBlock - leftShift);
		for (unsigned i = MSU; i > 0; --i) {
			_block[i] <<= leftShift;
			// mix in the bits from the right
			bt bits = static_cast<bt>(mask & _block[i - 1]);
			_block[i] |= (bits >> (bitsInBlock - leftShift));
		}
		_block[0] <<= leftShift;
	}
	void shiftRight(int rightShift) {
		if (rightShift == 0) return;
		if (rightShift < 0) return shiftLeft(-rightShift);
		if (rightShift >= long(nbits)) {
			setzero();
			return;
		}
		bool signext = sign();
		unsigned blockShift = 0;
		if (rightShift >= long(bitsInBlock)) {
			blockShift = rightShift / bitsInBlock;
			if (MSU >= blockShift) {
				// shift by blocks
				for (unsigned i = 0; i <= MSU - blockShift; ++i) {
					_block[i] = _block[i + blockShift];
				}
			}
			// adjust the shift
			rightShift -= (long)(blockShift * bitsInBlock);
			if (rightShift == 0) {
				// fix up the leading zeros if we have a negative number
				if (signext) {
					// rightShift is guaranteed to be less than nbits
					rightShift += (long)(blockShift * bitsInBlock);
					for (unsigned i = nbits - rightShift; i < nbits; ++i) {
						this->setbit(i);
					}
				}
				else {
					// clean up the blocks we have shifted clean
					rightShift += (long)(blockShift * bitsInBlock);
					for (unsigned i = nbits - rightShift; i < nbits; ++i) {
						this->setbit(i, false);
					}
				}
				return;  // shift was aligned to block boundary, no per-bit shift needed
			}
		}

		bt mask = ALL_ONES;
		mask >>= (bitsInBlock - rightShift); // this is a mask for the lower bits in the block that need to move to the lower word
		for (unsigned i = 0; i < MSU; ++i) {  // TODO: can this be improved? we should not have to work on the upper blocks in case we block shifted
			_block[i] >>= rightShift;
			// mix in the bits from the left
			bt bits = static_cast<bt>(mask & _block[i + 1]); // & operator returns an int
			_block[i] |= (bits << (bitsInBlock - rightShift));
		}
		_block[MSU] >>= rightShift;

		// fix up the leading zeros if we have a negative number
		if (signext) {
			// bitsToShift is guaranteed to be less than nbits
			rightShift += (long)(blockShift * bitsInBlock);
			for (unsigned i = nbits - rightShift; i < nbits; ++i) {
				this->setbit(i);
			}
		}
		else {
			// clean up the blocks we have shifted clean
			rightShift += (long)(blockShift * bitsInBlock);
			for (unsigned i = nbits - rightShift; i < nbits; ++i) {
				this->setbit(i, false);
			}
		}

		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
	}

	// calculate the integer power 2 ^ b using exponentiation by squaring
	double ipow(int exponent) const {
		bool negative = (exponent < 0);
		exponent = negative ? -exponent : exponent;
		double result(1.0);
		double base = 2.0;
		for (;;) {
			if (exponent % 2) result *= base;
			exponent >>= 1;
			if (exponent == 0) break;
			base *= base;
		}
		return (negative ? (1.0 / result) : result);
	}

	template<BlockTripleOperator btop>
	constexpr void blockcopy(blocktriple<fbits, btop, bt>& tgt) const {
		// brute force copy of blocks
		if constexpr (1 == fBlocks) {
			tgt.setblock(0, static_cast<bt>(_block[0] & FSU_MASK));
		}
		else if constexpr (2 == fBlocks) {
			tgt.setblock(0, _block[0]);
			tgt.setblock(1, static_cast<bt>(_block[1] & FSU_MASK));
		}
		else if constexpr (3 == fBlocks) {
			tgt.setblock(0, _block[0]);
			tgt.setblock(1, _block[1]);
			tgt.setblock(2, static_cast<bt>(_block[2] & FSU_MASK));
		}
		else if constexpr (4 == fBlocks) {
			tgt.setblock(0, _block[0]);
			tgt.setblock(1, _block[1]);
			tgt.setblock(2, _block[2]);
			tgt.setblock(3, static_cast<bt>(_block[3] & FSU_MASK));
		}
		else if constexpr (5 == fBlocks) {
			tgt.setblock(0, _block[0]);
			tgt.setblock(1, _block[1]);
			tgt.setblock(2, _block[2]);
			tgt.setblock(3, _block[3]);
			tgt.setblock(4, static_cast<bt>(_block[4] & FSU_MASK));
		}
		else if constexpr (6 == fBlocks) {
			tgt.setblock(0, _block[0]);
			tgt.setblock(1, _block[1]);
			tgt.setblock(2, _block[2]);
			tgt.setblock(3, _block[3]);
			tgt.setblock(4, _block[4]);
			tgt.setblock(5, static_cast<bt>(_block[5] & FSU_MASK));
		}
		else if constexpr (7 == fBlocks) {
			tgt.setblock(0, _block[0]);
			tgt.setblock(1, _block[1]);
			tgt.setblock(2, _block[2]);
			tgt.setblock(3, _block[3]);
			tgt.setblock(4, _block[4]);
			tgt.setblock(5, _block[5]);
			tgt.setblock(6, static_cast<bt>(_block[6] & FSU_MASK));
		}
		else if constexpr (8 == fBlocks) {
			tgt.setblock(0, _block[0]);
			tgt.setblock(1, _block[1]);
			tgt.setblock(2, _block[2]);
			tgt.setblock(3, _block[3]);
			tgt.setblock(4, _block[4]);
			tgt.setblock(5, _block[5]);
			tgt.setblock(6, _block[6]);
			tgt.setblock(7, static_cast<bt>(_block[7] & FSU_MASK));
		}
		else {
			for (unsigned i = 0; i < FSU; ++i) {
				tgt.setblock(i, _block[i]);
			}
			tgt.setblock(FSU, static_cast<bt>(_block[FSU] & FSU_MASK));
		}
	}

private:
	bt _block[nrBlocks];

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend std::ostream& operator<< (std::ostream& ostr, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& r);
	template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend std::istream& operator>> (std::istream& istr, cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& r);

	template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator==(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator!=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator< (const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator> (const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator<=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator>=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
};

///////////////////////////// IOSTREAM operators ///////////////////////////////////////////////

// convert cfloat to decimal fixpnt string, i.e. "-1234.5678"
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::string to_decimal_fixpnt_string(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& value, long long precision) {
	constexpr unsigned fbits = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits;
	constexpr unsigned bias = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::EXP_BIAS;
	std::stringstream str;
	if (value.iszero()) {
		str << '0';
		return str.str();
	}
	if (value.sign()) str << '-';

	// construct the discretization levels of the fraction part
	support::decimal range, discretizationLevels, step;
	// create the decimal range we are discretizing
	range.setdigit(1);
	range.shiftLeft(fbits); // the decimal range of the fraction
	discretizationLevels.powerOf2(fbits); // calculate the discretization levels of this range
	step = div(range, discretizationLevels);
	// now construct the value of this range by adding the fraction samples
	support::decimal partial, multiplier;
	partial.setzero();  // if you just want the fraction
	multiplier.setdigit(1);
	// convert the fraction part
	for (unsigned i = 0; i < fbits; ++i) {
		if (value.at(i)) {
			support::add(partial, multiplier);
		}
		support::add(multiplier, multiplier);
	}
	if (value.isdenormal()) {
		support::mul(partial, step);
		support::decimal scale;
		scale.powerOf2(bias - 1ull);
		partial = support::div(partial, scale);
	} 
	else {
		support::add(partial, multiplier); // add the hidden bit
		support::mul(partial, step);
		support::decimal scale;
		int exponent = value.scale();
		if (exponent < 0) {
			scale.powerOf2(static_cast<unsigned>(-exponent));
			partial = support::div(partial, scale);
		}
		else {
			scale.powerOf2(static_cast<unsigned>(exponent));
			support::mul(partial, scale);
		}
	}

	// the radix is at fbits
	// The partial represents the parts in the range, so we can deduce
	// the number of leading zeros by comparing to the length of range
	int nrLeadingZeros = static_cast<int>(range.size()) - static_cast<int>(partial.size()) - 1;
	if (nrLeadingZeros >= 0) str << "0.";
	for (int i = 0; i < nrLeadingZeros; ++i) str << '0';
	int digitsWritten = (nrLeadingZeros > 0) ? nrLeadingZeros : 0;
	int position = static_cast<int>(partial.size()) - 1;
	for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
		str << (int)*rit;
		++digitsWritten;
		if (position == fbits) str << '.';
		--position;
	}
	if (digitsWritten < precision) { // deal with trailing 0s
		for (unsigned i = static_cast<unsigned>(digitsWritten); i < fbits; ++i) {
			str << '0';
		}
	}

	return str.str();
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::string to_string(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& value, long long precision) {
	constexpr unsigned fbits = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits;
	std::stringstream str;
	if (value.iszero()) {
		str << '0';
		return str.str();
	}
	if (value.sign()) str << '-';

	// denormalize the number to gain access to the most sigificant digits
	// 1.ffff^e
	// scale is e
	// lsbScale is e - fbits
	// shift to get lsb to position 2^0 = (e - fbits)
	std::int64_t scale = value.scale();
//	std::int64_t shift = scale + fbits; // we want the lsb at 2^0
	std::int64_t lsbScale = scale - fbits;  // scale of the lsb
	support::decimal partial, multiplier;
	partial.setzero();

	multiplier.powerOf2(lsbScale);

	// convert the fraction bits 
	for (unsigned i = 0; i < fbits; ++i) {
		if (value.at(i)) {
			support::add(partial, multiplier);
		}
		support::add(multiplier, multiplier);
	}
	if (!value.isdenormal()) {
		support::add(partial, multiplier); // add the hidden bit
	}
	str << partial;
	return str.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////
/// stream operators

// ostream output generates an ASCII format for the floating-point argument
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::ostream& operator<<(std::ostream& ostr, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
	std::streamsize precision = ostr.precision();
	std::streamsize width = ostr.width();

	std::ios_base::fmtflags ff = ostr.flags();
	// extract the format flags that change the representation
	bool scientific = (ff & std::ios_base::scientific) == std::ios_base::scientific;
	bool fixed      = !scientific && (ff & std::ios_base::fixed);

	std::string representation;
	if (fixed) {
		representation = to_decimal_fixpnt_string(v, precision);
	}
	else {
		std::stringstream ss;
		ss << std::setprecision(precision) << double(v);  // TODO: make this native
		representation = ss.str();
//		representation = to_string(v, precision);
	}

	// implement setw and left/right operators
	std::streamsize repWidth = static_cast<std::streamsize>(representation.size());
	if (width > repWidth) {
		std::streamsize diff = width - static_cast<std::streamsize>(representation.size());
		char fill = ostr.fill();
		if ((ff & std::ios_base::left) == std::ios_base::left) {
			representation.append(static_cast<unsigned>(diff), fill);
		}
		else {
			representation.insert(0ull, static_cast<unsigned>(diff), fill);
		}
	}

	return ostr << representation;
}

// istream input: currently marshalling through native double
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::istream& operator>>(std::istream& istr, cfloat<nbits,es,bt,hasSubnormals,hasSupernormals,isSaturating>& v) {
	double d(0.0);
	istr >> d;
	v = d;
	return istr;
}

// encoding helpers

// return the Unit in the Last Position
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ulp(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& a) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> b(a);
	return ++b - a;
}

// transform cfloat to a binary representation
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::string to_binary(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	unsigned index = nbits;
	s << (number.at(--index) ? '1' : '0') << '.';

	for (int i = int(es) - 1; i >= 0; --i) {
		s << (number.at(--index) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}

	s << '.';

	constexpr int fbits = nbits - 1ull - es;
	for (int i = fbits - 1; i >= 0; --i) {
		s << (number.at(--index) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}

	return s.str();
}

// transform a cfloat into a triple representation
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::string to_triple(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& number, bool nibbleMarker = true) {
	std::stringstream s;
	blocktriple<cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits, BlockTripleOperator::REP, bt> triple;
	number.normalize(triple);
	s << to_triple(triple, nibbleMarker);
	return s.str();
}

// Magnitude of a cfloat (equivalent to turning the sign bit off).
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
abs(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a(v);
	a.setsign(false);
	return a;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
fabs(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> v) {
	return abs(v);
}

////////////////////// debug helpers

// convenience method to gain access to the values of the constexpr variables that govern the cfloat behavior
template<unsigned nbits, unsigned es, typename bt = uint8_t, bool hasSubnormals = false, bool hasSupernormals = false, bool isSaturating = false>
void ReportCfloatClassParameters() {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a;
	a.constexprClassParameters();
}

//////////////////////////////////////////////////////
/// cfloat - cfloat binary logic operators

template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator==(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	for (unsigned i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) {
			return false;
		}
	}
	return true;
}
template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator!=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator< (const cfloat<nnbits, nes, nbt, nsub, nsup, nsat>& lhs, const cfloat<nnbits, nes, nbt, nsub, nsup, nsat>& rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	// need this as arithmetic difference is defined as snan(indeterminate)
	if (lhs.isinf(INF_TYPE_NEGATIVE) && rhs.isinf(INF_TYPE_NEGATIVE)) return false;
	if (lhs.isinf(INF_TYPE_POSITIVE) && rhs.isinf(INF_TYPE_POSITIVE)) return false;
	if constexpr (nsub) {
		cfloat<nnbits, nes, nbt, nsub, nsup, nsat> diff = (lhs - rhs);
		return (!diff.iszero() && diff.sign()) ? true : false;  // got to guard against -0
	}
	else {
		if (lhs.iszero() && rhs.iszero()) return false;  // we need to 'collapse' all zero encodings
		if (lhs.sign() && !rhs.sign()) return true;
		if (!lhs.sign() && rhs.sign()) return false;
		bool positive = lhs.ispos();
		if (positive) {
			if (lhs.scale() < rhs.scale()) return true;
			if (lhs.scale() > rhs.scale()) return false;
		}
		else {
			if (lhs.scale() > rhs.scale()) return true;
			if (lhs.scale() < rhs.scale()) return false;
		}
		// sign and scale are the same
		if (lhs.scale() == rhs.scale()) {
			// compare fractions: we do not have subnormals, so we can ignore the hidden bit
			blockbinary<nnbits - 1ull - nes, nbt> l, r;
			lhs.fraction(l);
			rhs.fraction(r);
			blockbinary<nnbits - nes, nbt> ll, rr; // fbits + 1 so we can 0 extend to honor 2's complement encoding of blockbinary
			ll.assignWithoutSignExtend(l);
			rr.assignWithoutSignExtend(r);
			return (positive ? (ll < rr) : (ll > rr));
		}
		return false;
	}
}
template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator> (const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) { 
	if (lhs.isnan() || rhs.isnan()) return false;
	// need this as arithmetic difference is defined as snan(indeterminate)
	if (lhs.isinf(INF_TYPE_NEGATIVE) && rhs.isinf(INF_TYPE_NEGATIVE)) return false;
	if (lhs.isinf(INF_TYPE_POSITIVE) && rhs.isinf(INF_TYPE_POSITIVE)) return false;
	return  operator< (rhs, lhs); 
}
template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator<=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) { 
	if (lhs.isnan() || rhs.isnan()) return false;
	return !operator> (lhs, rhs); 
}
template<unsigned nnbits, unsigned nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator>=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	return !operator< (lhs, rhs); 
}

//////////////////////////////////////////////////////
/// cfloat - cfloat binary arithmetic operators

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

/// binary cfloat - literal arithmetic operators

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(float lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(float lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(float lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(float lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(double lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(double lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(double lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(double lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(int lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(int lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(int lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(int lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(unsigned int lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(unsigned int lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(unsigned int lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(unsigned int lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(long long lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(long long lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(long long lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(long long lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(unsigned long long lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(unsigned long long lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(unsigned long long lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(unsigned long long lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

///  binary cfloat - literal arithmetic operators

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat sum(lhs);
	sum += Cfloat(rhs);
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat mul(lhs);
	mul *= Cfloat(rhs);
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat ratio(lhs);
	ratio /= Cfloat(rhs);
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat sum(lhs);
	sum += Cfloat(rhs);
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat mul(lhs);
	mul *= Cfloat(rhs);
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat ratio(lhs);
	ratio /= Cfloat(rhs);
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat sum(lhs);
	sum += Cfloat(rhs);
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat mul(lhs);
	mul *= Cfloat(rhs);
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat ratio(lhs);
	ratio /= Cfloat(rhs);
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, unsigned int rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat sum(lhs);
	sum += Cfloat(rhs);
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, unsigned int rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, unsigned int rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat mul(lhs);
	mul *= Cfloat(rhs);
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, unsigned int rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat ratio(lhs);
	ratio /= Cfloat(rhs);
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat sum(lhs);
	sum += Cfloat(rhs);
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat mul(lhs);
	mul *= Cfloat(rhs);
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat ratio(lhs);
	ratio /= Cfloat(rhs);
	return ratio;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator+(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, unsigned long long rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat sum(lhs);
	sum += Cfloat(rhs);
	return sum;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator-(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, unsigned long long rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator*(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, unsigned long long rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat mul(lhs);
	mul *= Cfloat(rhs);
	return mul;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> operator/(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, unsigned long long rhs) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat ratio(lhs);
	ratio /= Cfloat(rhs);
	return ratio;
}

///////////////////////////////////////////////////////////////////////
///   binary logic literal comparisons

// cfloat - literal float logic operators
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) != rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) > rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) <= rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) >= rhs;
}
// cfloat - literal double logic operators
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) != rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) > rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) <= rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) >= rhs;
}

#if LONG_DOUBLE_SUPPORT
// cfloat - literal long double logic operators
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) == rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) != rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) < rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) > rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) <= rhs;
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) >= rhs;
}
#endif

// cfloat - literal int logic operators
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator==(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator!=(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator<(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs)) || operator==(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return !operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}

// cfloat - long long logic operators
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator==(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator!=(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator<(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs)) || operator==(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return !operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}

// standard library functions for floating point

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> frexp(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& x, int* exp) {
	*exp = x.scale();
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> fraction(x);
	fraction.setexponent(0);
	return fraction;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ldexp(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& x, int exp) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> result(x);
	int xexp = x.scale();
	result.setexponent(xexp + exp);  // TODO: this does not work for subnormals
	return result;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> 
fma(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> x,
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> y,
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> z) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> fused{ 0 };
	constexpr unsigned FBITS = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits;
	constexpr unsigned EXTRA_FBITS = FBITS+2;
	constexpr unsigned EXTENDED_PRECISION = nbits + EXTRA_FBITS;
	// the C++ fma spec indicates that the x*y+z is evaluated in 'infinite' precision
	// with only a single rounding event. The minimum finite precision that would behave like this
	// is the precision where the product x*y does not need to be rounded, which will
	// need at least 2*(fbits+1) mantissa bits to capture all bits that can be
	// generated by the product.
	cfloat<EXTENDED_PRECISION, es, bt, hasSubnormals, hasSupernormals, isSaturating> preciseX(x), preciseY(y), preciseZ(z);
//	ReportValue(preciseX, "extended precision x");
//	ReportValue(preciseY, "extended precision y");
//	ReportValue(preciseZ, "extended precision z");
	cfloat<EXTENDED_PRECISION, es, bt, hasSubnormals, hasSupernormals, isSaturating> product = preciseX * preciseY;
//	ReportValue(product, "extended precision p");
	fused = product + preciseZ;
	return fused;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&
minpos(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& c) {
	return c.minpos();
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&
maxpos(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& c) {
	return c.maxpos();
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&
minneg(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& c) {
	return c.minneg();
}
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&
maxneg(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& c) {
	return c.maxneg();
}

}} // namespace sw::universal
