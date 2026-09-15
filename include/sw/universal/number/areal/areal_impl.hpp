#pragma once

// Behavioural switches default HERE, beside the code they govern, rather than in the
// areal.hpp umbrella: including core.hpp directly would otherwise leave them undefined,
// #if would evaluate them as 0, and the switch would silently flip on that path -- the
// trap #1390 hit with POSIT_ENABLE_LITERALS (#1334, #1436).
#if !defined(AREAL_ENABLE_LITERALS)
#define AREAL_ENABLE_LITERALS 1
#endif
#if !defined(AREAL_THROW_ARITHMETIC_EXCEPTION)
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

#include <cstddef>    // std::size_t in parse()
#include <cstdint>    // std::int64_t, std::uint64_t in parse() and the encodings
#include <cstdio>     // no longer used here (assign()'s printf stub is gone, #1454); kept for
                      // code that has reached <cstdio> through this header
#include <string_view> // parse()
#include <iosfwd>     // std::ostream/std::istream in the friend declarations. UNCONDITIONAL:
                      // the declarations exist whether or not tracing is on, so this must not
                      // sit inside the TRACE_CONVERSION guard below.
#include <universal/utility/icf_array_bounds.hpp>
#include <string>
// areal_impl.hpp: implementation of an arbitrary configuration fixed-size floating-point representation with an uncertainty bit to represent a faithful floating-point system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <cmath>      // std::ldexp in to_native()
#include <limits>

#include <universal/native/ieee754_core.hpp>   // the stream-free half of <universal/native/ieee754.hpp>
#include <universal/native/subnormal.hpp>
#include <universal/utility/find_msb.hpp>
// The text headers the trace blocks need, and ONLY they need: every to_binary() call in
// this header sits inside `#if TRACE_CONVERSION`. native/integers.hpp supplies
// to_binary(Integer) and carries <sstream>; native/manipulators.hpp supplies
// to_binary(float)/to_binary(double), which the core's ieee754_core.hpp deliberately
// omits. Leaving them unconditional kept three I/O-family headers in the graph of every
// areal translation unit (#1334). debug.hpp includes integers.hpp in its own right.
// TRACE_CONVERSION defaults to 0 here, before its first use; the default used to
// come after this #if, which then tested it undefined (-Wundef, #1436)
#ifndef TRACE_CONVERSION
#define TRACE_CONVERSION 0
#endif
#if TRACE_CONVERSION
#include <universal/native/integers.hpp>
#include <universal/native/ieee754.hpp>   // to_binary(float)/to_binary(double): the text
                                          // half of ieee754, which ieee754_core.hpp omits
#endif
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/utility/decimal_to_binary.hpp>   // parse(): exact decimal text to binary (#1454)
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/areal/exceptions.hpp>

#ifndef THROW_ARITHMETIC_EXCEPTION
#define THROW_ARITHMETIC_EXCEPTION 0
#endif
// TRACE_CONVERSION: the trace statements below print, so they need <iostream>. They are
// compiled only when tracing is actually switched on, and the include now moves inside the
// guard with them -- an unconditional <iostream> here put four stream headers into the
// graph of every translation unit that touched an areal (#1334, the same move cfloat's
// trace includes got in #1417/#1418).
#if TRACE_CONVERSION
#include <iostream>
#endif

#include <universal/internal/bit_manipulation.hpp>

namespace sw { namespace universal {
		
	constexpr bool AREAL_NIBBLE_MARKER = true;

// Forward definitions
template<unsigned nbits, unsigned es, typename bt> class areal;
// text to areal (#1454): defined after the class, used by assign() and operator>>
template<unsigned nbits, unsigned es, typename bt> bool parse(const std::string& txt, areal<nbits, es, bt>& v);
template<unsigned nbits, unsigned es, typename bt> areal<nbits,es,bt> abs(const areal<nbits,es,bt>&);
// fused multiply-add: a*b + c, faithfully rounded with the uncertainty bit (see math_functions.hpp)
template<unsigned nbits, unsigned es, typename bt>
areal<nbits,es,bt> fma(const areal<nbits,es,bt>&, const areal<nbits,es,bt>&, const areal<nbits,es,bt>&);

/// <summary>
/// decode an areal value into its constituent parts
/// </summary>
/// <typeparam name="bt"></typeparam>
/// <param name="v"></param>
/// <param name="s"></param>
/// <param name="e"></param>
/// <param name="f"></param>
/// <param name="ubit"></param>
template<unsigned nbits, unsigned es, unsigned fbits, typename bt>
void decode(const areal<nbits, es, bt>& v, bool& s, blockbinary<es, bt>& e, blockbinary<fbits, bt>& f, bool& ubit) {
	s = v.at(nbits - 1ull);
	ubit = v.at(0);
	v.exponent(e);
	v.fraction(f);
}

/// <summary>
/// return the binary scale of the given number
/// </summary>
/// <typeparam name="bt">Block type used for storage: derived through ADL</typeparam>
/// <param name="v">the areal number for which we seek to know the binary scale</param>
/// <returns>binary scale, i.e. 2^scale, of the value of the areal</returns>
template<unsigned nbits, unsigned es, typename bt>
int scale(const areal<nbits, es, bt>& v) {
	return v.scale();
}

/// <summary>
/// An arbitrary configuration real number with gradual under/overflow and uncertainty bit
/// </summary>
/// <typeparam name="nbits">number of bits in the encoding</typeparam>
/// <typeparam name="es">number of exponent bits in the encoding</typeparam>
/// <typeparam name="bt">the type to use as storage class: one of [uint8_t|uint16_t|uint32_t]</typeparam>
template<unsigned _nbits, unsigned _es, typename bt = uint8_t>
class areal {
public:
	static_assert(_nbits > _es + 2ull, "nbits is too small to accomodate the requested number of exponent bits");
	static_assert(_es < 2147483647ull, "my God that is a big number, are you trying to break the Interweb?");
	static_assert(_es > 0, "number of exponent bits must be bigger than 0 to be a floating point number");
	static constexpr unsigned bitsInByte = 8ull;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static_assert(bitsInBlock <= 64, "storage unit for block arithmetic needs to be <= uint64_t"); // TODO: carry propagation on uint64_t requires assembly code

	static constexpr unsigned nbits = _nbits;
	static constexpr unsigned es = _es;
	static constexpr unsigned fbits  = nbits - 2ull - es;    // number of fraction bits excluding the hidden bit
	static constexpr unsigned fhbits = fbits + 1ull;         // number of fraction bits including the hidden bit
	static constexpr unsigned abits = fhbits + 3ull;         // size of the addend
	static constexpr unsigned mbits = 2ull * fhbits;         // size of the multiplier output
	static constexpr unsigned divbits = 3ull * fhbits + 4ull;// size of the divider output

	static constexpr unsigned nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static_assert(nrBlocks == 1 || bitsInBlock <= 32,
		"multi-block arithmetic requires BlockType of 32 bits or less for portable carry propagation");
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64ull - bitsInBlock));

	static constexpr unsigned MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken
	static constexpr bt ALLONES = bt(~0);
	static constexpr bt MSU_MASK = (ALLONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr unsigned bitsInMSU = bitsInBlock - (nrBlocks * bitsInBlock - nbits);
	static constexpr bt SIGN_BIT_MASK = bt(bt(1ull) << ((nbits - 1ull) % bitsInBlock));
	static constexpr bt LSB_BIT_MASK = bt(1ull);
	static constexpr bool MSU_CAPTURES_E = (1ull + es) <= bitsInMSU;
	static constexpr unsigned EXP_SHIFT = (MSU_CAPTURES_E ? (1 == nrBlocks ? (nbits - 1ull - es) : (bitsInMSU - 1ull - es)) : 0);
	static constexpr bt MSU_EXP_MASK = ((ALLONES << EXP_SHIFT) & ~SIGN_BIT_MASK) & MSU_MASK;
	static constexpr int EXP_BIAS = ((1l << (es - 1ull)) - 1l);
	// the largest finite scale: the all-ones exponent field, 2^es - 1, less the bias. It used to be
	// one more, which let a value of scale MAX_EXP + 1 through the overflow checks, where its biased
	// exponent then ran into the sign bit (#1503)
	static constexpr int MAX_EXP = (1l << es) - 1l - EXP_BIAS;
	static constexpr int MIN_EXP_NORMAL = 1 - EXP_BIAS;
	static constexpr int MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP
	static constexpr bt BLOCK_MASK = bt(-1);

	using BlockType = bt;

	// constructors
	constexpr areal() noexcept : _block{ 0 } {};

	constexpr areal(const areal&) noexcept = default;
	constexpr areal(areal&&) noexcept = default;

	constexpr areal& operator=(const areal&) noexcept = default;
	constexpr areal& operator=(areal&&) noexcept = default;

	// decorated/converting constructors

	/// <summary>
	/// construct an areal from another, block type bt must be the same
	/// </summary>
	/// <param name="rhs"></param>
	template<unsigned nnbits, unsigned ees>
	areal(const areal<nnbits, ees, bt>& rhs) {
		// this->assign(rhs);
	}

	// specific value constructor
	constexpr areal(const SpecificValue code) : _block{ 0 } {
		switch (code) {
		case SpecificValue::maxpos:
		case SpecificValue::infpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::maxneg:
		case SpecificValue::infneg:
			maxneg();
			break;
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:
			setnan();
			break;
		default:
			zero();
			break;
		}
	}

	/// <summary>
	/// construct an areal from a native type, specialized for size
	/// </summary>
	/// <param name="iv">initial value to construct</param>
	constexpr areal(signed char iv)        noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(short iv)              noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(int iv)                noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(long iv)               noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(long long iv)          noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(char iv)               noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(unsigned short iv)     noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(unsigned int iv)       noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(unsigned long iv)      noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(unsigned long long iv) noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(float iv)              noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(double iv)             noexcept : _block{ 0 } { *this = iv; }
	constexpr areal(long double iv)        noexcept : _block{ 0 } { *this = iv; }

	// assignment operators
	constexpr areal& operator=(signed char rhs) { return convert_signed_integer(rhs); }
	constexpr areal& operator=(short rhs)       { return convert_signed_integer(rhs); }
	constexpr areal& operator=(int rhs)         { return convert_signed_integer(rhs); }
	constexpr areal& operator=(long rhs)        { return convert_signed_integer(rhs); }
	constexpr areal& operator=(long long rhs)   { return convert_signed_integer(rhs); }

	constexpr areal& operator=(char rhs)               { return convert_unsigned_integer(rhs); }
	constexpr areal& operator=(unsigned short rhs)     { return convert_unsigned_integer(rhs); }
	constexpr areal& operator=(unsigned int rhs)       { return convert_unsigned_integer(rhs); }
	constexpr areal& operator=(unsigned long rhs)      { return convert_unsigned_integer(rhs); }
	constexpr areal& operator=(unsigned long long rhs) { return convert_unsigned_integer(rhs); }

	template<typename Ty>
	constexpr areal& convert_unsigned_integer(const Ty& rhs) noexcept {
		if constexpr (fbits < 53) {
			// For small fraction fields, double can exactly represent all
			// integers distinguishable at this precision; delegate safely
			return operator=(static_cast<double>(rhs));
		}
		else {
			// Native conversion: avoids silent precision loss through
			// double for integers > 2^53
			clear();
			if (rhs == 0) return *this;

			uint64_t raw = static_cast<uint64_t>(rhs);
			unsigned msb = find_msb(raw);    // 1-indexed; LSB=1
			int exponent = static_cast<int>(msb) - 1;

			if (exponent > MAX_EXP) {
				maxpos();
				this->set(0); // ubit: value is in (maxpos, inf)
				return *this;
			}

			// Remove hidden bit; fracBits significant fraction bits remain
			raw &= ~(1ull << (msb - 1));
			int fracBits = exponent; // number of fraction bits in the source

			// Determine if low-order bits are truncated
			bool ubit = false;
			if (fracBits > static_cast<int>(fbits)) {
				int lostBits = fracBits - static_cast<int>(fbits);
				ubit = (raw & ((1ull << lostBits) - 1)) != 0;
			}

			uint64_t biasedExponent = static_cast<uint64_t>(exponent + EXP_BIAS);

			if constexpr (nbits <= 64) {
				// Shift raw to fill (fbits+1) positions for assembly;
				// bit 0 will be replaced by ubit via the mask below
				if (fracBits <= static_cast<int>(fbits)) {
					raw <<= (static_cast<int>(fbits) + 1 - fracBits);
				}
				else {
					int shiftRight = fracBits - static_cast<int>(fbits) - 1;
					if (shiftRight > 0) raw >>= shiftRight;
				}
				uint64_t bits = 0ull; // sign = 0 for unsigned
				bits <<= es;
				bits |= biasedExponent;
				bits <<= nbits - 1ull - es;
				bits |= raw;
				bits &= 0xFFFF'FFFF'FFFF'FFFEull;
				bits |= (ubit ? 0x1ull : 0x0ull);
				if constexpr (nrBlocks == 1) {
					_block[MSU] = bt(bits);
				}
				else {
					copyBits(bits);
				}
			}
			else {
				// Large areal (> 64 bits): place bits individually
				int startBit = (fracBits > static_cast<int>(fbits))
				             ? (fracBits - static_cast<int>(fbits)) : 0;
				for (int i = startBit; i < fracBits; ++i) {
					if (raw & (1ull << i)) {
						int pos = static_cast<int>(fbits) - (fracBits - 1) + i;
						if (pos >= 1) set(static_cast<unsigned>(pos));
					}
				}
				for (unsigned i = 0; i < es; ++i) {
					if (biasedExponent & (1ull << i)) {
						set(fbits + 1 + i);
					}
				}
				// unsigned: no sign bit
				if (ubit) set(0);
			}
			saturate_reserved();
			return *this;
		}
	}
	template<typename Ty>
	constexpr areal& convert_signed_integer(const Ty& rhs) noexcept {
		if constexpr (fbits < 53) {
			// For small fraction fields, double can exactly represent all
			// integers distinguishable at this precision; delegate safely
			return operator=(static_cast<double>(rhs));
		}
		else {
			// Native conversion: avoids silent precision loss through
			// double for integers > 2^53
			clear();
			if (rhs == 0) return *this;

			bool sign = (rhs < 0);
			// Compute magnitude safely (handles most-negative value without overflow)
			uint64_t magnitude = sign ? (0ull - static_cast<uint64_t>(rhs))
			                          : static_cast<uint64_t>(rhs);

			unsigned msb = find_msb(magnitude); // 1-indexed; LSB=1
			int exponent = static_cast<int>(msb) - 1;

			if (exponent > MAX_EXP) {
				if (sign) maxneg(); else maxpos();
				this->set(0); // ubit: value is in (maxpos, inf) or (maxneg, -inf)
				return *this;
			}

			// Remove hidden bit; fracBits significant fraction bits remain
			magnitude &= ~(1ull << (msb - 1));
			int fracBits = exponent;

			// Determine if low-order bits are truncated
			bool ubit = false;
			if (fracBits > static_cast<int>(fbits)) {
				int lostBits = fracBits - static_cast<int>(fbits);
				ubit = (magnitude & ((1ull << lostBits) - 1)) != 0;
			}

			uint64_t biasedExponent = static_cast<uint64_t>(exponent + EXP_BIAS);

			if constexpr (nbits <= 64) {
				// Shift magnitude to fill (fbits+1) positions for assembly;
				// bit 0 will be replaced by ubit via the mask below
				if (fracBits <= static_cast<int>(fbits)) {
					magnitude <<= (static_cast<int>(fbits) + 1 - fracBits);
				}
				else {
					int shiftRight = fracBits - static_cast<int>(fbits) - 1;
					if (shiftRight > 0) magnitude >>= shiftRight;
				}
				uint64_t bits = (sign ? 1ull : 0ull);
				bits <<= es;
				bits |= biasedExponent;
				bits <<= nbits - 1ull - es;
				bits |= magnitude;
				bits &= 0xFFFF'FFFF'FFFF'FFFEull;
				bits |= (ubit ? 0x1ull : 0x0ull);
				if constexpr (nrBlocks == 1) {
					_block[MSU] = bt(bits);
				}
				else {
					copyBits(bits);
				}
			}
			else {
				// Large areal (> 64 bits): place bits individually
				int startBit = (fracBits > static_cast<int>(fbits))
				             ? (fracBits - static_cast<int>(fbits)) : 0;
				for (int i = startBit; i < fracBits; ++i) {
					if (magnitude & (1ull << i)) {
						int pos = static_cast<int>(fbits) - (fracBits - 1) + i;
						if (pos >= 1) set(static_cast<unsigned>(pos));
					}
				}
				for (unsigned i = 0; i < es; ++i) {
					if (biasedExponent & (1ull << i)) {
						set(fbits + 1 + i);
					}
				}
				if (sign) set(nbits - 1);
				if (ubit) set(0);
			}
			saturate_reserved();
			return *this;
		}
	}


	CONSTEXPRESSION areal& operator=(float rhs) {
		clear();
		// extract IEEE-754 fields using the unified extractFields abstraction
		bool s{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t rawBits{ 0 };
		extractFields(rhs, s, rawExponent, rawFraction, rawBits);
		uint32_t raw_exp = static_cast<uint32_t>(rawExponent);
		uint32_t raw     = static_cast<uint32_t>(rawFraction);

		// special case handling
		if (raw_exp == 0xFFu) { // special cases
			// IEEE-754: exponent all ones is infinity with a zero fraction, and a NaN with ANY other
			// fraction, whatever the payload; the quiet bit, the fraction's MSB, tells the two NaNs
			// apart. This used to match two payloads only, so every other NaN fell through to the
			// numeric path and came out as (maxpos, inf), inf, or a finite value (the #1303 defect)
			if (raw == 0ul) {
				// 1.11111111.00000000000000000000000 -inf
				// 0.11111111.00000000000000000000000 +inf
				setinf(s);
				return *this;
			}
			setnan((raw & 0x0040'0000ul) ? NAN_TYPE_QUIET : NAN_TYPE_SIGNALLING);
			return *this;
		}
		if (rhs == 0.0) { // IEEE rule: this is valid for + and - 0.0
			set(nbits - 1ull, s);
			return *this;
		}
		
		// this is not a special number
		int exponent = int(raw_exp) - 127;  // unbias the exponent

#if TRACE_CONVERSION
		std::cout << '\n';
		std::cout << "value           : " << rhs << '\n';
		std::cout << "segments        : " << to_binary(rhs) << '\n';
		std::cout << "sign     bit    : " << (s ? '1' : '0') << '\n';
		std::cout << "exponent value  : " << exponent << '\n';
		std::cout << "fraction bits   : " << to_binary(raw, true) << std::endl;
#endif
		// saturate to minpos/maxpos with uncertainty bit set to 1
		if (exponent > MAX_EXP) {
			if (s) maxneg(); else maxpos(); // saturate the maxpos or maxneg
			this->set(0);
			return *this;
		}
		if (exponent < MIN_EXP_SUBNORMAL) {
			if (s) this->set(nbits - 1); // set -0
			this->set(0); // and set the uncertainty bit to reflect (0,minpos) or (-0,minneg)
			return *this;
		}
		// set the exponent
		uint32_t biasedExponent{ 0 };
		int shiftRight = 23 - static_cast<int>(fbits) - 1; // this is the bit shift to get the MSB of the src to the MSB of the tgt
		int adjustment{ 0 };
		// we have 23 fraction bits and one hidden bit for a normal number, and no hidden bit for a subnormal
		// simpler rounding as compared to IEEE as uncertainty bit captures any non-zero bit past the LSB
		// ...  lsb | sticky      ubit
		//       x      0          0
		//       x  |   1          1
		bool ubit = false;
		// mask for sticky bit - guard against shift overflow for large fbits
		uint32_t mask = (fbits >= 23) ? 0u : (0x007F'FFFFu >> fbits); 
		if (exponent >= MIN_EXP_SUBNORMAL && exponent < MIN_EXP_NORMAL) {
			// this number is a subnormal number in this representation
			// trick though is that it might be a normal number in IEEE single precision representation
			if (exponent > -127) {
				// the source real is a normal number, so we must add the hidden bit to the fraction bits
				raw |= (1ull << 23);
				// mask for sticky bit - guard against shift overflow
				int shiftAmount = static_cast<int>(fbits) + exponent + subnormal_reciprocal_shift[es] + 1;
				mask = (shiftAmount >= 24) ? 0u : (0x00FF'FFFFu >> shiftAmount); 
#if TRACE_CONVERSION
				std::cout << "fraction bits   : " << to_binary(raw, true) << std::endl;
#endif
				// fraction processing: we have 24 bits = 1 hidden + 23 explicit fraction bits 
				// f = 1.ffff 2^exponent * 2^fbits * 2^-(2-2^(es-1)) = 1.ff...ff >> (23 - (-exponent + fbits - (2 -2^(es-1))))
				// -exponent because we are right shifting and exponent in this range is negative
				adjustment = -(exponent + subnormal_reciprocal_shift[es]); // this is the right shift adjustment due to the scale of the input number, i.e. the exponent of 2^-adjustment
				if (shiftRight > 0) {		// do we need to round?
					ubit = (mask & raw) != 0;
					raw >>= shiftRight + adjustment;
				}
				else { // all bits of the float go into this representation and need to be shifted up
					// target has more precision than source, shift left to align
					// ubit = false; already set to false (exact representation)
					int shiftLeft = (-shiftRight) - adjustment;
					if (shiftLeft > 0) raw <<= shiftLeft;
					else if (shiftLeft < 0) raw >>= (-shiftLeft);
				}
			}
			else {
				// the source real is a subnormal number, and the target representation is a subnormal representation
				// mask for sticky bit - guard against shift overflow
				int shiftAmount2 = static_cast<int>(fbits) + exponent + subnormal_reciprocal_shift[es] + 1;
				mask = (shiftAmount2 >= 24) ? 0u : (0x00FF'FFFFu >> shiftAmount2);
#if TRACE_CONVERSION
				std::cout << "fraction bits   : " << to_binary(raw, true) << std::endl;
#endif
				// fraction processing: we have 24 bits = 1 hidden + 23 explicit fraction bits
				// f = 1.ffff 2^exponent * 2^fbits * 2^-(2-2^(es-1)) = 1.ff...ff >> (23 - (-exponent + fbits - (2 -2^(es-1))))
				// -exponent because we are right shifting and exponent in this range is negative
				adjustment = -(exponent + subnormal_reciprocal_shift[es]); // this is the right shift adjustment due to the scale of the input number, i.e. the exponent of 2^-adjustment
				if (shiftRight > 0) {		// do we need to round?
					ubit = (mask & raw) != 0;
					raw >>= shiftRight + adjustment;
				}
				else { // all bits of the float go into this representation and need to be shifted up
					// target has more precision than source, shift left to align
					// ubit = false; already set to false (exact representation)
					int shiftLeft = (-shiftRight) - adjustment;
					if (shiftLeft > 0) raw <<= shiftLeft;
					else if (shiftLeft < 0) raw >>= (-shiftLeft);
				}
			}
		}
		else {
			// this number is a normal/max-exponent value number in this representation, we can leave the hidden bit hidden
			biasedExponent = static_cast<uint32_t>(exponent + EXP_BIAS); // reasonable to limit exponent to 32bits

			// fraction processing
			if (shiftRight > 0) {		// do we need to round?
				// we have 23 fraction bits and one hidden bit for a normal number, and no hidden bit for a subnormal
				// simpler rounding as uncertainty bit captures any non-zero bit past the LSB
				// ...  lsb | sticky      ubit
				//       x      0          0
				//       x  |   1          1
				ubit = (mask & raw) != 0;
				raw >>= shiftRight;
			}
			else { // all bits of the float go into this representation and need to be shifted up
				// target has more precision than source, shift left to align
				// ubit = false; already set to false (exact representation)
				raw <<= (-shiftRight);
			}
		}
#if TRACE_CONVERSION
		std::cout << "biased exponent : " << biasedExponent << " : 0x" << std::hex << biasedExponent << std::dec << '\n';
		std::cout << "shift           : " << shiftRight << '\n';
		std::cout << "adjustment shift: " << adjustment << '\n';
		std::cout << "sticky bit mask : " << to_binary(mask, true) << '\n';
		std::cout << "uncertainty bit : " << (ubit ? "1\n" : "0\n");
		std::cout << "fraction bits   : " << to_binary(raw, true) << '\n';
#endif
		// construct the target areal
		if constexpr (nbits <= 32) {
			// fast path for small areals that fit in 32 bits
			uint32_t bits = (s ? 1u : 0u);
			bits <<= es;
			bits |= biasedExponent;
			bits <<= nbits - 1u - es;
			bits |= raw;
			bits &= 0xFFFF'FFFEu;
			bits |= (ubit ? 0x1u : 0x0u);
			if constexpr (1 == nrBlocks) {
				_block[MSU] = bt(bits);
			}
			else {
				copyBits(bits);
			}
		}
		else {
			// large areal: set bits individually to avoid shift overflow
			clear();
			// set fraction bits (raw is already aligned, max 23 bits from float)
			for (unsigned i = 0; i < 23 && i < fbits; ++i) {
				if (raw & (1u << i)) {
					set(i + 1); // +1 for ubit position
				}
			}
			// set exponent bits
			for (unsigned i = 0; i < es; ++i) {
				if (biasedExponent & (1u << i)) {
					set(fbits + 1 + i); // +1 for ubit
				}
			}
			// set sign bit
			if (s) set(nbits - 1);
			// set ubit
			if (ubit) set(0);
		}
		saturate_reserved();
		return *this;
	}
	CONSTEXPRESSION areal& operator=(double rhs) {
		clear();
		// extract IEEE-754 fields using the unified extractFields abstraction
		bool s{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t rawBits{ 0 };
		extractFields(rhs, s, rawExponent, rawFraction, rawBits);
		uint32_t raw_exp = static_cast<uint32_t>(rawExponent);
		uint64_t raw     = rawFraction;

		if (raw_exp == 0x7FFul) { // special cases
			// infinity with a zero fraction, a NaN with any other; see operator=(float)
			if (raw == 0ull) {
				// 1.11111111111.0000000000000000000000000000000000000000000000000000 -inf
				// 0.11111111111.0000000000000000000000000000000000000000000000000000 +inf
				setinf(s);
				return *this;
			}
			setnan((raw & 0x0008'0000'0000'0000ull) ? NAN_TYPE_QUIET : NAN_TYPE_SIGNALLING);
			return *this;
		}
		if (rhs == 0.0) { // IEEE rule: this is valid for + and - 0.0
			set(nbits - 1ull, s);
			return *this;
		}
		// Handle subnormal doubles specially: they need normalization first
		// Subnormal double: value = 0.fraction * 2^-1022
		// We must find the effective exponent based on the leading 1 bit position
		int exponent;
		if (raw_exp == 0 && raw != 0) {
			// This is a subnormal double - find the leading 1 bit to get effective exponent
			// raw is the 52-bit fraction; find position of MSB (0-51)
			int msbPosition = 51;
			uint64_t testBit = 1ull << 51;
			while ((raw & testBit) == 0 && msbPosition >= 0) {
				testBit >>= 1;
				--msbPosition;
			}
			// Effective exponent: value = 2^msbPosition * 2^-52 * 2^-1022 = 2^(msbPosition - 1074)
			// In normalized form: 1.0 * 2^effectiveExp where effectiveExp = msbPosition - 1074
			exponent = msbPosition - 1074;
			// Normalize the fraction: shift left so the leading 1 moves to the hidden bit position (52)
			// The shift amount is (52 - msbPosition) to move the MSB from position p to position 52
			int normalizeShift = 52 - msbPosition;
			raw <<= normalizeShift;
			// Now raw has the leading 1 at position 52 (the hidden bit position)
			// The remaining fraction bits are in positions 0-51
#if TRACE_CONVERSION
			std::cout << "subnormal double: msbPosition=" << msbPosition << " effectiveExp=" << exponent << '\n';
			std::cout << "normalized fraction: " << to_binary(raw, true) << '\n';
#endif
		}
		else {
			// Normal double or zero
			exponent = int(raw_exp) - 1023;  // unbias the exponent
		}
#if TRACE_CONVERSION
		std::cout << '\n';
		std::cout << "value           : " << rhs << '\n';
		std::cout << "segments        : " << to_binary(rhs) << '\n';
		std::cout << "sign   bits     : " << (s ? '1' : '0') << '\n';
		std::cout << "exponent value  : " << exponent << '\n';
		std::cout << "fraction bits   : " << to_binary(raw, true) << std::endl;
#endif
		// saturate to minpos/maxpos with uncertainty bit set to 1
		if (exponent > MAX_EXP) {
			if (s) maxneg(); else maxpos(); // saturate the maxpos or maxneg
			this->set(0); // and set the uncertainty bit to reflect it is (maxpos, inf) or (maxneg, -inf)
			return *this;
		}
		if (exponent < MIN_EXP_SUBNORMAL) {
			if (s) this->set(nbits - 1); // set -0
			this->set(0); // and set the uncertainty bit to reflect (0,minpos) or (-0,minneg)
			return *this;
		}
		// Now we have a normalized representation (exponent, raw with hidden bit at position 52)
		// This works for both normal and subnormal source doubles
		uint64_t biasedExponent{ 0 };
		int shiftRight = 52 - static_cast<int>(fbits) - 1; // this is the bit shift to get the MSB of the src to the MSB of the tgt
		int adjustment{ 0 };
		// we have 52 fraction bits and one hidden bit for a normal number, and no hidden bit for a subnormal
		// simpler rounding as compared to IEEE as uncertainty bit captures any non-zero bit past the LSB
		// ...  lsb | sticky      ubit
		//       x      0          0
		//       x  |   1          1
		bool ubit = false;
		uint64_t mask;
		if (exponent >= MIN_EXP_SUBNORMAL && exponent < MIN_EXP_NORMAL) {
			// this number is a subnormal number in this target areal representation
			// For subnormal doubles, raw already has the hidden bit from normalization above
			// For normal doubles, we need to add the hidden bit now
			if (raw_exp != 0) {
				raw |= (1ull << 52);  // add hidden bit for normal source doubles
			}
			// mask for sticky bit - guard against shift overflow
			int subnormalShift = static_cast<int>(fbits) + exponent + subnormal_reciprocal_shift[es] + 1;
			mask = (subnormalShift >= 53) ? 0ull : (0x001F'FFFF'FFFF'FFFFull >> subnormalShift);
#if TRACE_CONVERSION
			std::cout << "mask     bits   : " << to_binary(mask, true) << std::endl;
			std::cout << "fraction bits   : " << to_binary(raw, true) << std::endl;
#endif
			// fraction processing: we have 53 bits = 1 hidden + 52 explicit fraction bits
			// f = 1.ffff 2^exponent * 2^fbits * 2^-(2-2^(es-1)) = 1.ff...ff >> (52 - (-exponent + fbits - (2 -2^(es-1))))
			// -exponent because we are right shifting and exponent in this range is negative
			adjustment = -(exponent + subnormal_reciprocal_shift[es]);
#if TRACE_CONVERSION
			std::cout << "exponent        : " << exponent << std::endl;
			std::cout << "bias shift      : " << subnormal_reciprocal_shift[es] << std::endl;
			std::cout << "adjustment      : " << adjustment << std::endl;
#endif
			if (shiftRight > 0) {		// do we need to round?
				ubit = (mask & raw) != 0;
				raw >>= (static_cast<std::int64_t>(shiftRight) + adjustment);
			}
			else { // all bits of the double go into this representation and need to be shifted up
				// target has more precision than source, shift left to align
				// ubit = false; already set to false (exact representation)
				int64_t shiftLeft = static_cast<int64_t>(-shiftRight) - adjustment;
				if (shiftLeft > 0) raw <<= shiftLeft;
				else if (shiftLeft < 0) raw >>= (-shiftLeft);
			}
		}
		else {
			// this number is a normal/max-exponent value number in this representation, we can leave the hidden bit hidden
			biasedExponent = static_cast<uint64_t>(exponent + EXP_BIAS); // reasonable to limit exponent to 32bits

			// fraction processing
			// mask for sticky bit - guard against shift overflow for large fbits
			if constexpr (fbits >= 52) {
				mask = 0ull;
			}
			else {
				mask = 0x000F'FFFF'FFFF'FFFFull >> fbits;
			}
			if (shiftRight > 0) {		// do we need to round?
				// we have 52 fraction bits and one hidden bit for a normal number, and no hidden bit for a subnormal
				// simpler rounding as uncertainty bit captures any non-zero bit past the LSB
				// ...  lsb | sticky      ubit
				//       x      0          0
				//       x  |   1          1
				ubit = (mask & raw) != 0;
				raw >>= shiftRight;
			}
			else { // all bits of the double go into this representation and need to be shifted up
				// target has more precision than source, shift left to align
				// ubit = false; already set to false (exact representation)
				// Guard against shift overflow: double fraction MSB is at position 51,
				// so shifting by more than 12 would overflow 64-bit raw
				int shiftAmount = -shiftRight;
				if (shiftAmount <= 12) {
					raw <<= shiftAmount;
				}
				// else: for large types, raw stays as-is; we'll place bits directly in the else branch below
			}
		}
#if TRACE_CONVERSION
		std::cout << "biased exponent : " << biasedExponent << " : " << std::hex << biasedExponent << std::dec << '\n';
		std::cout << "shift           : " << shiftRight << '\n';
		std::cout << "sticky bit mask : " << to_binary(mask, true) << '\n';
		std::cout << "uncertainty bit : " << (ubit ? "1\n" : "0\n");
		std::cout << "fraction bits   : " << to_binary(raw, true) << '\n';
#endif
		// construct the target areal
		if constexpr (nbits <= 64) {
			// fast path for areals that fit in 64 bits
			uint64_t bits = (s ? 1ull : 0ull);
			bits <<= es;
			bits |= biasedExponent;
			bits <<= nbits - 1ull - es;
			bits |= raw;
			bits &= 0xFFFF'FFFF'FFFF'FFFEull;
			bits |= (ubit ? 0x1ull : 0x0ull);
			if constexpr (nrBlocks == 1) {
				_block[MSU] = bt(bits);
			}
			else {
				copyBits(bits);
			}
		}
		else {
			// large areal (> 64 bits): set bits individually to avoid shift overflow
			clear();
			// For large types, raw contains the 52 double fraction bits (unshifted).
			// These need to be placed at the TOP of the areal fraction field.
			// Double bit i (0=LSB, 51=MSB) goes to areal position (fbits - 51 + i).
			// This aligns the double's MSB with the areal fraction MSB.
			for (unsigned i = 0; i < 52 && i < fbits; ++i) {
				if (raw & (1ull << i)) {
					// Position in areal: fbits - 51 + i
					// For fbits >= 52, this correctly places bits at the top
					unsigned pos = (fbits >= 52) ? (fbits - 51 + i) : (i + 1);
					set(pos);
				}
			}
			// set exponent bits
			for (unsigned i = 0; i < es; ++i) {
				if (biasedExponent & (1ull << i)) {
					set(fbits + 1 + i); // +1 for ubit
				}
			}
			// set sign bit
			if (s) set(nbits - 1);
			// set ubit
			if (ubit) set(0);
		}
		saturate_reserved();
		return *this;
	}
	CONSTEXPRESSION areal& operator=(long double rhs) {
		// Narrowing to double rounds: a finite value past DBL_MAX became inf, one below double's range
		// became an exact zero, and bits below double's precision were gone before truncation could
		// see them, so 1 + 2^-60 read as exactly 1 (#1503). The exact path is taken when long double
		// has more precision or range than double; keyed on the format, not LONG_DOUBLE_SUPPORT,
		// which is 1 where long double is double. Zero, inf and nan narrow exactly.
		using ld = std::numeric_limits<long double>;
		using d  = std::numeric_limits<double>;
		if constexpr (ld::digits > d::digits || ld::max_exponent > d::max_exponent
		              || ld::min_exponent < d::min_exponent) {
			constexpr long double ldmax = std::numeric_limits<long double>::max();
			if (rhs != rhs || rhs == 0.0l || rhs > ldmax || rhs < -ldmax)
				return *this = double(rhs);
			return assign_extended(rhs);
		}
		else {
			return *this = double(rhs);  // long double has double's format: narrowing is exact
		}
	}

	// arithmetic operators
	// prefix operator: negate by flipping the sign bit (uniform for all
	// values, INCLUDING NaN). For finite values this is straightforward
	// arithmetic negation. For NaN, the sign bit doubles as the NaN-kind
	// discriminator (sNaN has sign bit set, qNaN has sign bit clear; see
	// setnan at line 1112), so unary -sNaN intentionally yields qNaN and
	// vice versa. This is project-specific areal behavior and is locked
	// in by static/range/areal/api/special_cases.cpp::TestNaN -- do NOT
	// "fix" this to preserve NaN kind without first updating that test.
	constexpr areal operator-() const noexcept {
		areal tmp(*this);
		tmp._block[MSU] ^= SIGN_BIT_MASK;
		return tmp;
	}

	constexpr areal& operator+=(const areal& rhs) {
		// special case handling of NaN
		if (isnan() || rhs.isnan()) {
			setnan();
			return *this;
		}
		// inf + (-inf) = NaN
		// inf + finite = inf
		// finite + inf = inf
		if (isinf()) {
			if (rhs.isinf()) {
				if (sign() != rhs.sign()) {
					setnan(); // inf + (-inf) = NaN
				}
				// else: inf + inf = inf (no change)
			}
			return *this;
		}
		if (rhs.isinf()) {
			*this = rhs;
			return *this;
		}
		// zero cases
		if (iszero()) {
			if (rhs.iszero()) {
				// two exact zeros sum to -0 only when both are -0, as IEEE-754 has it in every
				// rounding mode but toward negative: +0 + -0 and +0 - +0 are +0 (#1507)
				setsign(sign() && rhs.sign());
				return *this;
			}
			*this = rhs;
			return *this;
		}
		if (rhs.iszero()) {
			return *this;
		}

		// arithmetic operation using blocktriple
		bool inputUncertain = ubit() || rhs.ubit();

		blocktriple<fbits, BlockTripleOperator::ADD, bt> a, b, sum;
		normalizeAddition(a);
		rhs.normalizeAddition(b);
		sum.add(a, b);

		convert(sum, *this, inputUncertain);

		return *this;
	}
	constexpr areal& operator+=(double rhs) {
		return *this += areal(rhs);
	}
	constexpr areal& operator-=(const areal& rhs) {
		// subtraction is addition with negated rhs
		// but we need to handle NaN specially
		if (rhs.isnan()) {
			return *this += rhs;
		}
		return *this += -rhs;
	}
	constexpr areal& operator-=(double rhs) {
		return *this -= areal(rhs);  // injected class name preserves bt
	}
	constexpr areal& operator*=(const areal& rhs) {
		// special case handling of NaN
		if (isnan() || rhs.isnan()) {
			setnan();
			return *this;
		}

		bool resultSign = sign() != rhs.sign();

		// inf * 0 = NaN
		// inf * finite = inf
		// 0 * inf = NaN
		if (isinf()) {
			if (rhs.iszero()) {
				setnan(); // inf * 0 = NaN
			}
			else {
				setsign(resultSign);
			}
			return *this;
		}
		if (rhs.isinf()) {
			if (iszero()) {
				setnan(); // 0 * inf = NaN
			}
			else {
				setinf(resultSign);
			}
			return *this;
		}

		// zero cases
		if (iszero() || rhs.iszero()) {
			setzero();
			setsign(resultSign);
			return *this;
		}

		// arithmetic operation using blocktriple
		bool inputUncertain = ubit() || rhs.ubit();

		blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;
		normalizeMultiplication(a);
		rhs.normalizeMultiplication(b);
		product.mul(a, b);

		convert(product, *this, inputUncertain);

		return *this;
	}
	constexpr areal& operator*=(double rhs) {
		return *this *= areal(rhs);  // injected class name preserves bt
	}
	constexpr areal& operator/=(const areal& rhs) {
		// special case handling of NaN
		if (isnan() || rhs.isnan()) {
			setnan();
			return *this;
		}

		bool resultSign = sign() != rhs.sign();

		// handle division by zero
		if (rhs.iszero()) {
			if (iszero()) {
				setnan(); // 0/0 = NaN
			}
			else {
				setinf(resultSign); // x/0 = +/-inf
			}
			return *this;
		}

		// inf / inf = NaN
		// inf / finite = inf
		// finite / inf = 0 with ubit
		if (isinf()) {
			if (rhs.isinf()) {
				setnan(); // inf / inf = NaN
			}
			else {
				setsign(resultSign);
			}
			return *this;
		}
		if (rhs.isinf()) {
			// finite / inf = 0 with ubit (true value is in (0, minpos))
			setzero();
			setsign(resultSign);
			set(0, true); // set ubit to indicate uncertainty
			return *this;
		}

		// zero / finite = zero
		if (iszero()) {
			setzero();
			setsign(resultSign);
			return *this;
		}

		// arithmetic operation using blocktriple
		bool inputUncertain = ubit() || rhs.ubit();

		blocktriple<fbits, BlockTripleOperator::DIV, bt> a, b, quotient;
		normalizeDivision(a);
		rhs.normalizeDivision(b);
		quotient.div(a, b);
		quotient.setradix(blocktriple<fbits, BlockTripleOperator::DIV, bt>::radix);

		convert(quotient, *this, inputUncertain);

		return *this;
	}
	constexpr areal& operator/=(double rhs) {
		return *this /= areal(rhs);  // injected class name preserves bt
	}
	/// <summary>
	/// move to the next bit encoding modulo 2^nbits
	/// </summary>
	/// <typeparam name="bt"></typeparam>
	constexpr areal& operator++() {
		if constexpr (0 == nrBlocks) {
			return *this;
		}
		else if constexpr (1 == nrBlocks) {
			// special cases are: 011...111 and 111...111
			if ((_block[MSU] & MSU_MASK) == MSU_MASK) { // == all bits are set
				_block[MSU] = 0;
			}
			else {
				++_block[MSU];
			}
		}
		else {
			bool carry = true;
			for (unsigned i = 0; i < MSU; ++i) {
				if ((_block[i] & storageMask) == storageMask) { // block will overflow
					++_block[i];
				}
				else {
					++_block[i];
					carry = false;
					break;
				}
			}
			if (carry) {
				// encoding behaves like a 2's complement modulo wise
				if ((_block[MSU] & MSU_MASK) == MSU_MASK) {
					_block[MSU] = 0;
				}
				else {
					++_block[MSU]; // a carry will flip the sign
				}
			}
		}
		return *this;
	}
	constexpr areal operator++(int) {
		areal tmp(*this);
		operator++();
		return tmp;
	}
	/// <summary>
	/// move to the previous bit encoding modulo 2^nbits (the symmetric
	/// inverse of operator++). 000...000 wraps to 111...111.
	/// </summary>
	constexpr areal& operator--() {
		if constexpr (0 == nrBlocks) {
			return *this;
		}
		else if constexpr (1 == nrBlocks) {
			// special case: 000...000 wraps to 111...111
			if ((_block[MSU] & MSU_MASK) == 0) {
				_block[MSU] = MSU_MASK;
			}
			else {
				--_block[MSU];
			}
		}
		else {
			bool borrow = true;
			for (unsigned i = 0; i < MSU; ++i) {
				if ((_block[i] & storageMask) == 0) { // block will underflow
					_block[i] = static_cast<bt>(storageMask);
				}
				else {
					--_block[i];
					borrow = false;
					break;
				}
			}
			if (borrow) {
				// encoding behaves like a 2's complement modulo wise
				if ((_block[MSU] & MSU_MASK) == 0) {
					_block[MSU] = MSU_MASK;
				}
				else {
					--_block[MSU];
				}
			}
		}
		return *this;
	}
	constexpr areal operator--(int) {
		areal tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	
	/// <summary>
	/// clear the content of this areal to zero
	/// </summary>
	/// <returns>void</returns>
	inline constexpr void clear() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(0);
		}
	}
	/// <summary>
	/// set the number to +0
	/// </summary>
	/// <returns>void</returns>
	inline constexpr void setzero() noexcept { clear(); }
	/// <summary>
	/// set the sign bit of the areal
	/// </summary>
	/// <param name="sign">true for negative, false for positive</param>
	/// <returns>void</returns>
	inline constexpr void setsign(bool sign = true) noexcept {
		if (sign) {
			_block[MSU] |= SIGN_BIT_MASK;
		}
		else {
			_block[MSU] &= ~SIGN_BIT_MASK;
		}
	}
	/// <summary>
	/// set the number to +inf
	/// </summary>
	/// <param name="sign">boolean to make it + or - infinity, default is -inf</param>
	/// <returns>void</returns> 
	inline constexpr void setinf(bool sign = true) noexcept {
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
		else {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			for (unsigned i = 1; i < nrBlocks - 1; ++i) {
				_block[i] = BLOCK_MASK;
			}
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}	
	}
	/// <summary>
	/// set the number to a quiet NaN (+nan) or a signalling NaN (-nan, default)
	/// </summary>
	/// <param name="sign">boolean to make it + or - infinity, default is -inf</param>
	/// <returns>void</returns> 
	inline constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING) noexcept {
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
		else {
			for (unsigned i = 0; i < nrBlocks - 1; ++i) {
				_block[i] = BLOCK_MASK;
			}
		}
		_block[MSU] = NaNType == NAN_TYPE_SIGNALLING ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
	}

	// fill an areal object with maximum positive value
	inline constexpr areal<nbits, es, bt>& maxpos() noexcept {
		// maximum positive value has this bit pattern: 0-1...1-111...100, that is, sign = 0, e = 1.1, f = 111...110, u = 0
		clear();
		flip();
		reset(nbits - 1ull);
		reset(0ull);
		reset(1ull);
		return *this;
	}
	// fill an areal object with mininum positive value
	inline constexpr areal<nbits, es, bt>& minpos() noexcept {
		// minimum positive value has this bit pattern: 0-000-00...010, that is, sign = 0, e = 00, f = 00001, u = 0
		clear();
		set(1);
		return *this;
	}
	// fill an areal object with the zero encoding: 0-0...0-00...000-0
	inline constexpr areal<nbits, es, bt>& zero() noexcept {
		clear();
		return *this;
	}
	// fill an areal object with smallest negative value
	inline constexpr areal<nbits, es, bt>& minneg() noexcept {
		// minimum negative value has this bit pattern: 1-000-00...010, that is, sign = 1, e = 00, f = 00001, u = 0
		clear();
		set(nbits - 1ull);
		set(1);
		return *this;
	}
	// fill an areal object with largest negative value
	inline constexpr areal<nbits, es, bt>& maxneg() noexcept {
		// maximum negative value has this bit pattern: 1-1...1-111...110, that is, sign = 1, e = 1.1, f = 111...110, u = 0
		clear();
		flip();
		reset(0ull);
		reset(1ull);
		return *this;
	}
	// Conversions of a finite value call this after they assemble the encoding. In the top binade,
	// every value past maxpos + ulp truncates onto the all-ones fraction, which encodes inf (ubit
	// clear) or nan (ubit set). A finite value there lies in (maxpos, inf), where larger values
	// already saturate, so it saturates the same way (#1503).
	inline constexpr void saturate_reserved() noexcept {
		if (isinf() || isnan()) {
			if (sign())
				maxneg();
			else
				maxpos();
			set(0);  // (maxpos, inf) or (maxneg, -inf)
		}
	}

	/// <summary>
	/// set the raw bits of the areal. This is a required API function for number systems in the Universal Numbers Library
	/// This enables verification test suites to inject specific test bit patterns using a common interface.
	//  This is a memcpy type operator, but the target number system may not have a linear memory layout and
	//  thus needs to steer the bits in potentially more complicated ways then memcpy.
	/// </summary>
	/// <param name="raw_bits">unsigned long long carrying bits that will be written verbatim to the areal</param>
	/// <returns>reference to the areal</returns>
	inline constexpr areal& setbits(uint64_t raw_bits) noexcept {
		if constexpr (0 == nrBlocks) {
			return *this;
		}
		else if constexpr (1 == nrBlocks) {
			_block[0] = raw_bits & storageMask;
		}
		else {
			for (unsigned i = 0; i < nrBlocks; ++i) {
				_block[i] = raw_bits & storageMask;
				raw_bits >>= bitsInBlock; // shift can be the same size as type as it is protected by loop constraints
			}
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		return *this;
	}
	/// <summary>
	/// set a specific bit in the encoding to true or false. If bit index is out of bounds, no modification takes place.
	/// </summary>
	/// <param name="i">bit index to set</param>
	/// <param name="v">boolean value to set the bit to. Default is true.</param>
	/// <returns>void</returns>
	inline constexpr void set(unsigned i, bool v = true) noexcept {
		if (i >= nbits) return;
		unsigned blockIndex = i / bitsInBlock;
		if (blockIndex < nrBlocks) {
			// GCC -O3 false positive: see blockbinary.hpp setbit() comment
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#if __GNUC__ >= 12
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
#endif
			bt block = _block[blockIndex];
			bt null = bit_clear_mask<bt>(i, bitsInBlock);
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[blockIndex] = bt((block & null) | mask);
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
		}
	}
	/// <summary>
	/// reset a specific bit in the encoding to false. If bit index is out of bounds, no modification takes place.
	/// </summary>
	/// <param name="i">bit index to reset</param>
	/// <returns>void</returns>
	inline constexpr void reset(unsigned i) noexcept {
		if (i < nbits) {
			// in bounds: i < nbits => index <= nrBlocks-1. The pragma silences a GCC
			// -fipa-icf false positive, as in at(); see utility/icf_array_bounds.hpp. The folded
			// write draws -Wstringop-overflow as well as -Warray-bounds.
			UNIVERSAL_ICF_ARRAY_BOUNDS_PUSH
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
			bt block = _block[i / bitsInBlock];
			bt mask = ~(1ull << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt(block & mask);
			UNIVERSAL_ICF_ARRAY_BOUNDS_POP
			return;
		}
	}
	/// <summary>
	/// 1's complement of the encoding
	/// </summary>
	/// <returns>reference to this areal object</returns>
	inline constexpr areal& flip() noexcept { // in-place one's complement
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	/// <summary>
	/// assign the value of a text representation to the areal: a decimal, the exact form
	/// [d], or the uncertain form (d, dnext) that operator<< writes. See parse().
	/// </summary>
	/// <param name="stringRep">the text to assign</param>
	/// <returns>reference to this areal; zero if the text does not parse, as dfloat and bfloat16 do</returns>
	inline areal& assign(const std::string& stringRep) {
		if (!parse(stringRep, *this)) clear();
		return *this;
	}

	// selectors
	inline constexpr bool sign() const noexcept { return (_block[MSU] & SIGN_BIT_MASK) == SIGN_BIT_MASK; }
	inline constexpr bool ubit() const noexcept { return (_block[0] & LSB_BIT_MASK) != 0; }
	inline constexpr int scale() const {
		// the biased exponent field, read as the unsigned number it is
		int e{ 0 };
		if constexpr (MSU_CAPTURES_E) {
			e = int((_block[MSU] & ~SIGN_BIT_MASK) >> EXP_SHIFT);
		}
		else {
			// the field straddles a limb boundary. blockbinary<es, bt> is Signed, and its int()
			// sign-extends, which read every code with the top bit set as code - 2^es (#1506)
			blockbinary<es, bt> ebits;
			exponent(ebits);
			e = static_cast<int>(unsigned(ebits));
		}
		if (e == 0) {
			// subnormal scale is determined by fraction
			// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits)), so the leading fraction bit is
			// at MIN_EXP_NORMAL - 1. The straddling branch used to count down from -1 (#1506)
			e = (2l - (1l << (es - 1ull))) - 1;
			for (unsigned i = nbits - 2ull - es; i > 0; --i) {
				if (test(i)) break;
				--e;
			}
		}
		else {
			e -= EXP_BIAS;
		}
		return e;
	}
	inline constexpr bool isneg() const { return sign(); }
	inline constexpr bool ispos() const { return !sign(); }
	inline constexpr bool iszero() const {
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
		else {
			for (unsigned i = 0; i < nrBlocks-1; ++i) if (_block[i] != 0) return false;
			return (_block[MSU] & ~SIGN_BIT_MASK) == 0;
		}
	}
	inline constexpr bool isone() const {
		// unbiased exponent = scale = 0, fraction = 0
		int s = scale();
		if (s == 0) {
			blockbinary<fbits, bt> f;
			fraction(f);
			return f.iszero();
		}
		return false;
	}
	/// <summary>
	/// check if value is infinite, -inf, or +inf. 
	/// +inf = 0-1111-11111-0: sign = 0, uncertainty = 0, es/fraction bits = 1
	/// -inf = 1-1111-11111-0: sign = 1, uncertainty = 0, es/fraction bits = 1
	/// </summary>
	/// <param name="InfType">default is 0, both types, -1 checks for -inf, 1 checks for +inf</param>
	/// <returns>true if +-inf, false otherwise</returns>
	inline constexpr bool isinf(int InfType = INF_TYPE_EITHER) const {
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
	/// <summary>
	/// check if the value is a normal number (exponent is not all 0s or all 1s)
	/// Note: named with underscore suffix to avoid conflict with std::isnormal
	/// </summary>
	/// <returns>true if normal, false otherwise</returns>
	inline constexpr bool isnormal_() const noexcept {
		if (iszero() || isnan() || isinf()) return false;
		blockbinary<es, bt> ebits;
		exponent(ebits);
		return !ebits.iszero(); // subnormal has all-zero exponent
	}
	/// <summary>
	/// check if the value is a subnormal number (exponent is all 0s but value is not zero)
	/// Note: named with underscore suffix to avoid conflict with std::issubnormal
	/// </summary>
	/// <returns>true if subnormal, false otherwise</returns>
	inline constexpr bool issubnormal_() const noexcept {
		if (iszero() || isnan() || isinf()) return false;
		blockbinary<es, bt> ebits;
		exponent(ebits);
		return ebits.iszero(); // subnormal has all-zero exponent
	}
	/// <summary>
	/// check if a value is a quiet or a signalling NaN
	/// quiet NaN      = 0-1111-11111-1: sign = 0, uncertainty = 1, es/fraction bits = 1
	/// signalling NaN = 1-1111-11111-1: sign = 1, uncertainty = 1, es/fraction bits = 1
	/// </summary>
	/// <param name="NaNType">default is 0, both types, 1 checks for Signalling NaN, -1 checks for Quiet NaN</param>
	/// <returns>true if the right kind of NaN, false otherwise</returns>
	inline constexpr bool isnan(int NaNType = NAN_TYPE_EITHER) const {
		bool isNaN = true;
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
		else {
			for (unsigned i = 0; i < nrBlocks - 1; ++i) {
				if (_block[i] != BLOCK_MASK) {
					isNaN = false;
					break;
				}
			}
		}
		bool isNegNaN = isNaN && ((_block[MSU] & MSU_MASK) == MSU_MASK);
		bool isPosNaN = isNaN && (_block[MSU] & MSU_MASK) == (MSU_MASK ^ SIGN_BIT_MASK);
		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) : 
			     (NaNType == NAN_TYPE_SIGNALLING ? isNegNaN : 
				   (NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}

	inline constexpr bool test(unsigned bitIndex) const noexcept {
		return at(bitIndex);
	}
	inline constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex < nbits) {
			// in bounds: bitIndex < nbits => index <= nrBlocks-1. The pragma silences a
			// GCC -fipa-icf false positive; see utility/icf_array_bounds.hpp.
			UNIVERSAL_ICF_ARRAY_BOUNDS_PUSH
			bt word = _block[bitIndex / bitsInBlock];
			UNIVERSAL_ICF_ARRAY_BOUNDS_POP
			bt mask = bt(1ull << (bitIndex % bitsInBlock));
			return (word & mask);
		}
		return false;
	}
	inline constexpr uint8_t nibble(unsigned n) const noexcept {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			int nibbleIndexInWord = int(n % (bitsInBlock >> 2ull));
			bt mask = bt(0xF << (nibbleIndexInWord * 4));
			bt nibblebits = bt(mask & word);
			return uint8_t(nibblebits >> (nibbleIndexInWord * 4));
		}
		return false;
	}
	inline constexpr bt block(unsigned b) const noexcept {
		if (b < nrBlocks) {
			return _block[b];
		}
		return 0;
	}

	// helper debug function, can remove/deprecate
	// defined out-of-line in number/areal/debug.hpp so this header needs no <iostream>
	// (#1334). Include that header to call it -- forgetting to is a link error naming the
	// missing function, the same shape blocktriple's constexprClassParameters() got in
	// #1388.
	void constexprClassParameters() const;

	// extract the exponent field from the encoding
	inline constexpr void exponent(blockbinary<es, bt>& e) const {
		e.clear();
		if constexpr (0 == nrBlocks) return;
		else if constexpr (1 == nrBlocks) {
			bt ebits = bt(_block[MSU] & ~SIGN_BIT_MASK);
			e.setbits(uint64_t(ebits >> EXP_SHIFT));
		}
		else if constexpr (nrBlocks > 1) {
			if (MSU_CAPTURES_E) {
				bt ebits = bt(_block[MSU] & ~SIGN_BIT_MASK);
				e.setbits(uint64_t(ebits >> ((nbits - 1ull - es) % bitsInBlock)));
			}
			else {
				for (unsigned i = 0; i < es; ++i) { e.setbit(i, at(nbits - 1ull - es + i)); }
			}
		}
	}
	// extract the fraction field from the encoding
	inline constexpr void fraction(blockbinary<fbits, bt>& f) const {
		f.clear();
		if constexpr (0 == nrBlocks) return;
		else if constexpr (1 == nrBlocks) {
			bt fraction = bt(_block[MSU] & ~MSU_EXP_MASK);
			f.setbits(bt(fraction >> bt(1ull)));
		}
		else if constexpr (nrBlocks > 1) {
			for (unsigned i = 0; i < fbits; ++i) { f.setbit(i, at(nbits - 1ull - es - fbits + i)); }
		}
	}
	// extract the fraction bits as a uint64_t (for normalization)
	// Note: areal encoding is [sign | exponent | fraction | ubit]
	// fraction bits are at positions [1, fbits] (bit 0 is the ubit)
	constexpr uint64_t fraction_ull() const noexcept {
		uint64_t raw{ 0 };
		if constexpr (fbits < 65ull) { // no-op if precondition doesn't hold
			if constexpr (1 == nrBlocks) {
				// mask out the ubit (bit 0) and shift right by 1
				uint64_t fbitMask = (0xFFFFFFFFFFFFFFFFull >> (64 - fbits)) << 1;
				raw = (fbitMask & uint64_t(_block[0])) >> 1;
			}
			else if constexpr (2 == nrBlocks) {
				uint64_t combined = (uint64_t(_block[1]) << bitsInBlock) | uint64_t(_block[0]);
				uint64_t fbitMask = (0xFFFFFFFFFFFFFFFFull >> (64 - fbits)) << 1;
				raw = (fbitMask & combined) >> 1;
			}
			else if constexpr (3 == nrBlocks) {
				uint64_t combined = (uint64_t(_block[2]) << (2 * bitsInBlock)) | (uint64_t(_block[1]) << bitsInBlock) | uint64_t(_block[0]);
				uint64_t fbitMask = (0xFFFFFFFFFFFFFFFFull >> (64 - fbits)) << 1;
				raw = (fbitMask & combined) >> 1;
			}
			else {
				// general case: extract bit by bit
				uint64_t mask{ 1 };
				for (unsigned i = 0; i < fbits; ++i) {
					if (test(i + 1)) { // fraction bits start at bit 1 (bit 0 is ubit)
						raw |= mask;
					}
					mask <<= 1;
				}
			}
		}
		return raw;
	}
	
	// casts to native types
	long to_long() const { return long(to_native<double>()); }
	long long to_long_long() const { return (long long)(to_native<double>()); }
	// transform an areal to a native C++ floating-point. We are using the native
	// precision to compute, which means that all sub-values need to be representable 
	// by the native precision.
	// A more accurate appromation would require an adaptive precision algorithm
	// with a final rounding step.
	template<typename TargetFloat>
	TargetFloat to_native() const { 
		TargetFloat v = TargetFloat(0);
		if (iszero()) {
			if (sign()) // the optimizer might destroy the sign
				return -TargetFloat(0);
			else
				return TargetFloat(0);
		}
		else if (isnan()) {
			v = sign() ? std::numeric_limits<TargetFloat>::signaling_NaN() : std::numeric_limits<TargetFloat>::quiet_NaN();
		}
		else if (isinf()) {
			v = sign() ? -INFINITY : INFINITY;
		}
		else { // TODO: this approach has catastrophic cancellation when nbits is large and native target float is small
			TargetFloat f{ 0 };
			TargetFloat fbit{ 0.5 };
			for (unsigned i = nbits - 2ull - es; i > 0; --i) {
				f += at(i) ? fbit : TargetFloat(0);
				fbit *= TargetFloat(0.5);
			}
			blockbinary<es, bt> ebits;
			exponent(ebits);
			// The power of two is applied with std::ldexp in TargetFloat itself (#1509). It used to be
			// built from 1ull << e, and as a double outside (-64, 64): clang 18 miscompiles
			// 1 / (long double)(1ull << n) for a runtime n (0.75 read back as -nan, the #937 cfloat
			// fault), a double cannot hold a long double's exponent range, and the subnormal scale
			// table in native/subnormal.hpp is 0 for es >= 12.
			if (ebits.iszero()) {
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				v = std::ldexp(f, MIN_EXP_NORMAL);
			}
			else {
				// regular: (-1)^s * 2^(e+1-2^(es-1)) * (1 + f/2^fbits))
				int exponent = unsigned(ebits) + 1ll - (1ll << (es - 1ull));
				v = std::ldexp(TargetFloat(1) + f, exponent);
			}
			v = sign() ? -v : v;
		}
		return v;
	}

	// make conversions to native types explicit
	explicit operator int()         const noexcept { return to_long_long(); }
	explicit operator long long()   const noexcept { return to_long_long(); }
	explicit operator long double() const noexcept { return to_native<long double>(); }
	explicit operator double()      const noexcept { return to_native<double>(); }
	explicit operator float()       const noexcept { return to_native<float>(); }

	// normalize areal to a blocktriple for addition
	// blocktriple for ADD has the form: iii.fffrrrrr (3 integer bits, f fraction bits, r rounding bits)
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
			tgt.setnormal();
			int scl = scale();
			tgt.setsign(sign());
			tgt.setscale(scl);
			// set significand: we need format 001.ffffeeee
			if (isnormal_()) {
				if constexpr (fbits < 64 && BlockTripleConfiguration::rbits < (64 - fbits)) {
					uint64_t raw = fraction_ull();
					raw |= (1ull << fbits); // add the hidden bit
					raw <<= BlockTripleConfiguration::rbits;  // rounding bits required for correct rounding
					tgt.setbits(raw);
				}
				else {
					// For larger configurations, build bit by bit
					tgt.clear();
					tgt.setnormal();
					tgt.setsign(sign());
					tgt.setscale(scl);
					tgt.setradix(); // set radix point for proper significand interpretation
					tgt.setbit(static_cast<unsigned>(BlockTripleConfiguration::radix)); // set hidden bit
					for (unsigned i = 0; i < fbits; ++i) {
						tgt.setbit(static_cast<unsigned>(BlockTripleConfiguration::radix) - 1 - i, at(1 + fbits - 1 - i));
					}
				}
			}
			else {
				// subnormal: shift fraction and don't add hidden bit
				if constexpr (fbits < 64 && BlockTripleConfiguration::rbits < (64 - fbits)) {
					uint64_t raw = fraction_ull();
					int shift = MIN_EXP_NORMAL - scl;
					raw <<= shift;
					raw <<= BlockTripleConfiguration::rbits;
					tgt.setbits(raw);
				}
				else {
					tgt.clear();
					tgt.setnormal();
					tgt.setsign(sign());
					tgt.setscale(scl);
					tgt.setradix(); // set radix point for proper significand interpretation
					for (unsigned i = 0; i < fbits; ++i) {
						tgt.setbit(static_cast<unsigned>(BlockTripleConfiguration::radix) - 1 - i, at(1 + fbits - 1 - i));
					}
				}
			}
		}
	}

	// normalize areal to a blocktriple for multiplication
	// blocktriple for MUL has the form: ii.ffffffff (2 integer bits, 2*f fraction bits)
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
			tgt.setnormal();
			int scl = scale();
			tgt.setsign(sign());
			tgt.setscale(scl);
			// set significand: format 01.ffffeeee
			if (isnormal_()) {
				if constexpr (fbits < 64) {
					uint64_t raw = fraction_ull();
					raw |= (1ull << fbits); // add hidden bit
					tgt.setbits(raw);
				}
				else {
					tgt.clear();
					tgt.setnormal();
					tgt.setsign(sign());
					tgt.setscale(scl);
					tgt.setradix(fbits); // set radix point for proper significand interpretation
					tgt.setbit(fbits); // hidden bit
					for (unsigned i = 0; i < fbits; ++i) {
						tgt.setbit(fbits - 1 - i, at(1 + fbits - 1 - i));
					}
				}
			}
			else {
				// subnormal
				if constexpr (fbits < 64) {
					uint64_t raw = fraction_ull();
					int shift = MIN_EXP_NORMAL - scl;
					raw <<= shift;
					raw |= (1ull << fbits);
					tgt.setbits(raw);
				}
				else {
					tgt.clear();
					tgt.setnormal();
					tgt.setsign(sign());
					tgt.setscale(scl);
					tgt.setradix(fbits); // set radix point for proper significand interpretation
					for (unsigned i = 0; i < fbits; ++i) {
						tgt.setbit(fbits - 1 - i, at(1 + fbits - 1 - i));
					}
				}
			}
		}
		tgt.setradix(fbits);
	}

	// normalize areal to a blocktriple for division
	// blocktriple for DIV has the form: ii.fffffffff'ffff'rrrr (2 integer bits, 3*f fraction bits, r rounding bits)
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
			tgt.setnormal();
			int scl = scale();
			tgt.setsign(sign());
			tgt.setscale(scl);
			// set significand
			if (isnormal_()) {
				if constexpr (fbits < 64 && divshift < (64 - fbits)) {
					uint64_t raw = fraction_ull();
					raw |= (1ull << fbits); // add hidden bit
					raw <<= divshift; // shift to output radix
					tgt.setbits(raw);
				}
				else {
					tgt.clear();
					tgt.setnormal();
					tgt.setsign(sign());
					tgt.setscale(scl);
					tgt.setradix(); // set radix point for proper significand interpretation
					tgt.setbit(fbits + divshift); // hidden bit at correct position
					for (unsigned i = 0; i < fbits; ++i) {
						tgt.setbit(fbits + divshift - 1 - i, at(1 + fbits - 1 - i));
					}
				}
			}
			else {
				// subnormal
				if constexpr (fbits < 64 && divshift < (64 - fbits)) {
					uint64_t raw = fraction_ull();
					int shift = MIN_EXP_NORMAL - scl;
					raw <<= shift;
					raw |= (1ull << fbits);
					raw <<= divshift;
					tgt.setbits(raw);
				}
				else {
					tgt.clear();
					tgt.setnormal();
					tgt.setsign(sign());
					tgt.setscale(scl);
					tgt.setradix(); // set radix point for proper significand interpretation
					for (unsigned i = 0; i < fbits; ++i) {
						tgt.setbit(divshift + fbits - 1 - i, at(1 + fbits - 1 - i));
					}
				}
			}
		}
	}

protected:
	// HELPER methods

	// a finite, non-zero long double, truncated toward zero with the ubit set when that loses
	// anything. Scaling by a power of two is exact, so |rhs| is normalized into [1, 2) with the
	// factors 2^(2^i) and no frexp, which keeps this constexpr; the top 64 bits of the significand
	// then go into a blocktriple the way parse() packs one, and any bits below them into the ubit.
	CONSTEXPRESSION areal& assign_extended(long double rhs) {
		using BT = blocktriple<fbits, BlockTripleOperator::MUL, bt>;
		constexpr int radix_pos = static_cast<int>(BT::radix);
		constexpr int ldmaxexp  = std::numeric_limits<long double>::max_exponent;
		constexpr int levels    = (ldmaxexp > 8192 ? 14 : ldmaxexp > 512 ? 10 : 7);  // 2^(2^(levels-1)) is finite
		long double pow2[levels]{}, inv2[levels]{};  // 2^(2^i) and 2^-(2^i)
		pow2[0] = 2.0l;
		inv2[0] = 0.5l;
		for (int i = 1; i < levels; ++i) {
			pow2[i] = pow2[i - 1] * pow2[i - 1];
			inv2[i] = inv2[i - 1] * inv2[i - 1];
		}
		const bool negative = rhs < 0.0l;
		long double m = negative ? -rhs : rhs;
		int scale = 0;
		for (int i = levels - 1; i >= 0; --i) {
			while (m >= pow2[i]) {
				m *= inv2[i];
				scale += (1 << i);
			}
			while (m < inv2[i]) {
				m *= pow2[i];
				scale -= (1 << i);
			}
		}
		if (m < 1.0l) {  // a value below 1 ends in [0.5, 1); now m is in [1, 2)
			m *= 2.0l;
			scale -= 1;
		}
		const long double sig = m * 9223372036854775808.0l;  // 2^63: [2^63, 2^64)
		const uint64_t top = static_cast<uint64_t>(sig);
		bool uncertain = (sig != static_cast<long double>(top));  // a significand wider than 64 bits
		BT t;
		t.setnormal();
		t.setsign(negative);
		t.setscale(scale);
		for (int k = 0; k < 64; ++k) {
			const bool bit = (top >> (63 - k)) & 1u;
			if (radix_pos - k >= 0) {
				if (bit)
					t.setbit(static_cast<unsigned>(radix_pos - k), true);
			}
			else if (bit) {
				uncertain = true;
			}
		}
		clear();
		convert(t, *this, uncertain);
		return *this;
	}

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
			if constexpr (shift < sizeof(raw)*8) {
				raw <<= shift;
			}
			else {
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
	void shiftLeft(int bitsToShift) {
		if (bitsToShift == 0) return;
		if (bitsToShift < 0) return shiftRight(-bitsToShift);
		if (bitsToShift > long(nbits)) bitsToShift = nbits; // clip to max
		if (bitsToShift >= long(bitsInBlock)) {
			int blockShift = bitsToShift / bitsInBlock;
			for (signed i = signed(MSU); i >= blockShift; --i) {
				_block[i] = _block[i - blockShift];
			}
			for (signed i = blockShift - 1; i >= 0; --i) {
				_block[i] = bt(0);
			}
			// adjust the shift
			bitsToShift -= (long)(blockShift * bitsInBlock);
			if (bitsToShift == 0) return;
		}
		// construct the mask for the upper bits in the block that need to move to the higher word
		bt mask = bit_high_mask<bt>(bitsToShift, bitsInBlock);
		for (unsigned i = MSU; i > 0; --i) {
			_block[i] <<= bitsToShift;
			// mix in the bits from the right
			bt bits = (mask & _block[i - 1]);
			_block[i] |= (bits >> (bitsInBlock - bitsToShift));
		}
		_block[0] <<= bitsToShift;
	}

	void shiftRight(int bitsToShift) {
		if (bitsToShift == 0) return;
		if (bitsToShift < 0) return shiftLeft(-bitsToShift);
		if (bitsToShift >= long(nbits)) {
			setzero();
			return;
		}
		bool signext = sign();
		unsigned blockShift = 0;
		if (bitsToShift >= long(bitsInBlock)) {
			blockShift = bitsToShift / bitsInBlock;
			if (MSU >= blockShift) {
				// shift by blocks
				for (unsigned i = 0; i <= MSU - blockShift; ++i) {
					_block[i] = _block[i + blockShift];
				}
			}
			// adjust the shift
			bitsToShift -= (long)(blockShift * bitsInBlock);
			if (bitsToShift == 0) {
				// fix up the leading zeros if we have a negative number
				if (signext) {
					// bitsToShift is guaranteed to be less than nbits
					bitsToShift += (long)(blockShift * bitsInBlock);
					for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
						this->set(i);
					}
				}
				else {
					// clean up the blocks we have shifted clean
					bitsToShift += (long)(blockShift * bitsInBlock);
					for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
						this->reset(i);
					}
				}
			}
		}
		//bt mask = 0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock);  // is that shift necessary?
		bt mask = bt(0xFFFFFFFFFFFFFFFFull);
		mask >>= (bitsInBlock - bitsToShift); // this is a mask for the lower bits in the block that need to move to the lower word
		for (unsigned i = 0; i < MSU; ++i) {  // TODO: can this be improved? we should not have to work on the upper blocks in case we block shifted
			_block[i] >>= bitsToShift;
			// mix in the bits from the left
			bt bits = (mask & _block[i + 1]);
			_block[i] |= (bits << (bitsInBlock - bitsToShift));
		}
		_block[MSU] >>= bitsToShift;

		// fix up the leading zeros if we have a negative number
		if (signext) {
			// bitsToShift is guaranteed to be less than nbits
			bitsToShift += (long)(blockShift * bitsInBlock);
			for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
				this->set(i);
			}
		}
		else {
			// clean up the blocks we have shifted clean
			bitsToShift += (long)(blockShift * bitsInBlock);
			for (unsigned i = nbits - bitsToShift; i < nbits; ++i) {
				this->reset(i);
			}
		}

		// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		_block[MSU] &= MSU_MASK;
	}

private:
	bt _block[nrBlocks];

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const areal<nnbits,nes,nbt>& r);
	// operator>> is not a friend: it goes through parse() and the public interface (#1454)

	template<unsigned nnbits, unsigned nes, typename nbt>
	friend constexpr bool operator==(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend constexpr bool operator!=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend constexpr bool operator< (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend constexpr bool operator> (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend constexpr bool operator<=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend constexpr bool operator>=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
};

/// <summary>
/// convert a blocktriple to an areal with ubit propagation
/// The ubit is set if: inputUncertain || rounding occurred
/// </summary>
/// <typeparam name="srcbits">number of fraction bits in the blocktriple</typeparam>
/// <typeparam name="op">blocktriple operator type</typeparam>
/// <typeparam name="nbits">total bits in the areal</typeparam>
/// <typeparam name="es">exponent bits in the areal</typeparam>
/// <typeparam name="bt">block type</typeparam>
/// <param name="src">the blocktriple to convert from</param>
/// <param name="tgt">the areal to convert to</param>
/// <param name="inputUncertain">whether any input was uncertain (ubit was set)</param>
template<unsigned srcbits, BlockTripleOperator op, unsigned nbits, unsigned es, typename bt>
constexpr void convert(const blocktriple<srcbits, op, bt>& src, areal<nbits, es, bt>& tgt, bool inputUncertain = false) {
	using ArealType = areal<nbits, es, bt>;
	// test special cases
	if (src.isnan()) {
		tgt.setnan(src.sign() ? NAN_TYPE_SIGNALLING : NAN_TYPE_QUIET);
	}
	else if (src.isinf()) {
		tgt.setinf(src.sign());
	}
	else if (src.iszero()) {
		tgt.setzero();
		tgt.setsign(src.sign()); // preserve sign
		if (inputUncertain) tgt.set(0, true); // propagate uncertainty
	}
	else {
		int significandScale = src.significandscale();
		int exponent = src.scale() + significandScale;

		// check for underflow
		if (exponent < ArealType::MIN_EXP_SUBNORMAL) {
			tgt.setzero();
			tgt.setsign(src.sign());
			// underflow means true value is in (0, minpos), so set ubit
			tgt.set(0, true);
			return;
		}

		// check for overflow
		if (exponent > ArealType::MAX_EXP) {
			// saturate to maxpos/maxneg with ubit set
			if (src.sign()) tgt.maxneg(); else tgt.maxpos();
			tgt.set(0, true); // overflow means true value is in (maxpos, inf)
			return;
		}

		// normal conversion with rounding
		constexpr unsigned fbits = nbits - 2 - es; // fraction bits in areal

		// determine if we're in subnormal range
		uint64_t biasedExponent{ 0 };
		int adjustment{ 0 };
		bool roundingOccurred = false;

		if (exponent < ArealType::MIN_EXP_NORMAL) {
			// subnormal result
			biasedExponent = 0;
			adjustment = -(exponent + subnormal_reciprocal_shift[es]);
		}
		else {
			// normal result
			biasedExponent = static_cast<uint64_t>(static_cast<long long>(exponent) + static_cast<long long>(ArealType::EXP_BIAS));
		}

		// areal uses truncation (floor) with ubit to indicate uncertainty
		// This is different from IEEE rounding - we do NOT round up
		// Instead, we truncate and set ubit=1 if any precision is lost

		// The hidden bit in the blocktriple is at position srcRadix + significandScale.
		// significandScale accounts for carry/normalization shifts during arithmetic.
		constexpr int srcRadix = blocktriple<srcbits, op, bt>::radix;
		int hiddenBitPos = srcRadix + significandScale;

		// For subnormals, adjustment shifts the extraction window to include the hidden bit.
		// For normals (adjustment=0): extract fraction from (hiddenBitPos-1) down
		// For subnormals (adjustment>0): shift up to capture hidden bit in fraction

		// check if any bits will be lost (shifted out)
		int firstDiscardPos = hiddenBitPos - static_cast<int>(fbits) - 1 + adjustment;
		for (int i = firstDiscardPos; i >= 0; --i) {
			if (src.at(static_cast<unsigned>(i))) {
				roundingOccurred = true;
				break;
			}
		}

		// construct the result by extracting fraction bits and assembling the encoding
		if constexpr (nbits <= 64) {
			// fast path for areals that fit in 64 bits
			uint64_t fracbits = 0;
			for (unsigned i = 0; i < fbits; ++i) {
				int bitPos = hiddenBitPos - 1 - static_cast<int>(i) + adjustment;
				if (bitPos >= 0 && src.at(static_cast<unsigned>(bitPos))) {
					fracbits |= (1ull << (fbits - 1 - i));
				}
			}

			// assemble the areal encoding: [sign | exponent | fraction | ubit]
			// areal bit layout (LSB to MSB): ubit(1) | fraction(fbits) | exponent(es) | sign(1)
			uint64_t raw = (src.sign() ? 1ull : 0ull); // sign
			raw <<= es;
			raw |= biasedExponent;
			raw <<= fbits;
			raw |= fracbits;
			raw <<= 1; // make room for ubit

			// set ubit based on input uncertainty or rounding
			if (inputUncertain || roundingOccurred) {
				raw |= 1ull;
			}

			tgt.setbits(raw);
		}
		else {
			// large areal (> 64 bits): set bits individually to avoid shift overflow
			tgt.clear();

			// set fraction bits directly
			for (unsigned i = 0; i < fbits; ++i) {
				int bitPos = hiddenBitPos - 1 - static_cast<int>(i) + adjustment;
				if (bitPos >= 0 && src.at(static_cast<unsigned>(bitPos))) {
					// fraction bit i maps to target position (fbits - 1 - i) + 1 (for ubit)
					tgt.set(fbits - i); // position in areal: ubit at 0, fraction starts at 1
				}
			}

			// set exponent bits
			for (unsigned i = 0; i < es; ++i) {
				if (biasedExponent & (1ull << i)) {
					tgt.set(fbits + 1 + i); // +1 for ubit
				}
			}

			// set sign bit
			if (src.sign()) tgt.set(nbits - 1);

			// set ubit based on input uncertainty or rounding
			if (inputUncertain || roundingOccurred) {
				tgt.set(0);
			}
		}
		tgt.saturate_reserved();  // a finite result past maxpos + ulp (#1503)
	}
}

////////////////////// text to areal (#1454)
//
// parse() accepts the three forms an areal is written in:
//   d          a decimal (or inf, nan): its value truncated toward zero, with the ubit set
//              when that loses anything -- the same faithful rule as conversion from double,
//              but from the exact decimal, so digits beyond double precision still count
//   [d]        an exact value, as operator<< writes one: the exact areal nearest to d.
//              operator<< prints only a few digits, so d need not be exact itself
//   (d, dnext) an uncertain value, as operator<< writes one: the open interval between two
//              adjacent exact values, the nearest ones to d and dnext. Either order is
//              accepted; endpoints that are not adjacent are an error
// Inf and nan are accepted in any of the common spellings, bare or bracketed. A nan parses
// as a quiet nan: the text does not say which kind it was.
namespace areal_parse {

inline bool is_space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

inline std::string_view trim(std::string_view s) {
	while (!s.empty() && is_space(s.front())) s.remove_prefix(1);
	while (!s.empty() && is_space(s.back())) s.remove_suffix(1);
	return s;
}

// 1 for nan, 2 for inf, 0 otherwise; negative is set from a leading '-'
inline int special_value(std::string_view s, bool& negative) {
	negative = false;
	if (!s.empty() && (s.front() == '+' || s.front() == '-')) {
		negative = (s.front() == '-');
		s.remove_prefix(1);
	}
	std::string t;
	for (char c : s) t.push_back(static_cast<char>((c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c));
	if (t == "nan" || t == "qnan" || t == "snan") return 1;
	if (t == "inf" || t == "infinity") return 2;
	return 0;
}

// the exact binary value d, truncated toward zero, with the ubit set when that loses anything.
// It is packed like cfloat's parse(): blocktriple<fbits, MUL> leaves fbits bits below the
// fraction, and convert() truncates and sets the ubit for anything below the fraction,
// including the converter's own guard and sticky bits.
template<unsigned BigBits, unsigned nbits, unsigned es, typename bt>
bool from_binary(const ::sw::universal::decimal_to_binary::basic_result<BigBits>& d, areal<nbits, es, bt>& v) {
	using BT = blocktriple<areal<nbits, es, bt>::fbits, BlockTripleOperator::MUL, bt>;
	constexpr unsigned radix_pos = static_cast<unsigned>(BT::radix);
	if (!d.valid) return false;
	if (d.is_zero) {
		v.setzero();
		v.setsign(d.negative);
		return true;
	}
	// clamp absurd decimal exponents; convert() saturates or underflows well inside this range
	constexpr std::int64_t scale_limit = std::int64_t(1) << 30;
	std::int64_t scale = d.binary_scale;
	if (scale > scale_limit) scale = scale_limit;
	if (scale < -scale_limit) scale = -scale_limit;
	BT t;
	t.setnormal();
	t.setsign(d.negative);
	t.setscale(static_cast<int>(scale));
	for (unsigned i = 0; i <= radix_pos; ++i) {
		if (d.mantissa.at(i)) t.setbit(i, true);
	}
	convert(t, v, d.guard_bit || d.sticky_bit);
	return true;
}

// the decimal s truncated toward zero, with the ubit set when that loses anything
template<unsigned nbits, unsigned es, typename bt>
bool faithful(std::string_view s, areal<nbits, es, bt>& v) {
	bool negative{ false };
	switch (special_value(s, negative)) {
	case 1: v.setnan(NAN_TYPE_QUIET); return true;
	case 2: v.setinf(negative); return true;
	default: break;
	}
	// The exact binary value. The converter's working integer has a fixed width, and a decimal
	// that needs more loses bits silently (#1504), so the width comes from the converter's own estimate:
	// 2048 bits (which dispatches down itself), 8192 and 32768 bits for long exact expansions and
	// the exponent range of the widest areals, and a rejection beyond that rather than a wrong value.
	using BT = blocktriple<areal<nbits, es, bt>::fbits, BlockTripleOperator::MUL, bt>;
	constexpr unsigned target_bits = static_cast<unsigned>(BT::radix) + 1u;
	const auto scan = ::sw::universal::string_parse::scan_decimal_float(s);
	if (!scan.valid) return false;
	// A value far outside the type needs no exact conversion, only its order of magnitude: the
	// first nonzero digit puts it in [10^p, 10^(p+1)). Saturate or underflow directly, so a short
	// text such as 1e5000 does not ask the converter for thousands of working bits.
	std::int64_t p{ 0 };
	bool nonzero{ false };
	for (std::size_t i = 0; i < scan.int_part.size() && !nonzero; ++i) {
		if (scan.int_part[i] != '0') { nonzero = true; p = static_cast<std::int64_t>(scan.int_part.size() - 1 - i); }
	}
	for (std::size_t i = 0; i < scan.frac_part.size() && !nonzero; ++i) {
		if (scan.frac_part[i] != '0') { nonzero = true; p = -static_cast<std::int64_t>(i + 1); }
	}
	if (nonzero) {
		p += scan.exp10;
		using A = areal<nbits, es, bt>;
		// log2(10) is 3.3219...: 3321/1000 and 3322/1000 bound it, and the margin of 4 absorbs the rest
		if ((p * 3321) / 1000 > static_cast<std::int64_t>(A::MAX_EXP) + 4) {
			if (scan.negative) v.maxneg(); else v.maxpos();
			v.set(0, true);  // (maxpos, inf) or (maxneg, -inf)
			return true;
		}
		if (((p + 1) * 3322) / 1000 < static_cast<std::int64_t>(A::MIN_EXP_SUBNORMAL) - 4) {
			v.setzero();
			v.setsign(scan.negative);
			v.set(0, true);  // (0, minpos) or (-0, minneg)
			return true;
		}
	}
	const std::uint64_t need = ::sw::universal::decimal_to_binary::detail::required_working_bits(scan, target_bits);
	if (need <= 2048u) return from_binary(::sw::universal::decimal_to_binary::convert<2048u>(scan, target_bits), v);
	if (need <= 8192u) return from_binary(::sw::universal::decimal_to_binary::convert<8192u>(scan, target_bits), v);
	if (need <= 32768u) return from_binary(::sw::universal::decimal_to_binary::convert<32768u>(scan, target_bits), v);
	return false;  // a decimal this long cannot be converted exactly here
}

// the exact areal nearest to the decimal s; ties go to the even encoding, and a finite value
// beyond maxpos stays finite
template<unsigned nbits, unsigned es, typename bt>
bool nearest_exact(std::string_view s, areal<nbits, es, bt>& v) {
	areal<nbits, es, bt> t;
	if (!faithful(s, t)) return false;
	if (!t.ubit() || t.isnan() || t.isinf()) {
		v = t;
		return true;
	}
	// s lies strictly between the exact values lo and hi
	areal<nbits, es, bt> lo(t), hi(t);
	lo.set(0, false);
	++hi;
	if (hi.isinf() || hi.isnan()) {  // beyond maxpos: the nearest finite exact value
		v = lo;
		return true;
	}
	// one more fraction bit, same exponent range: its last fraction bit says whether s is at
	// or past the midpoint of lo and hi, and its ubit whether s is past it
	areal<nbits + 1, es, bt> w;
	faithful(s, w);
	const bool atOrPastMidpoint = w.at(1), pastMidpoint = w.ubit();
	if (!atOrPastMidpoint) v = lo;
	else if (pastMidpoint) v = hi;
	else v = lo.at(1) ? hi : lo;  // a tie: the encoding whose last fraction bit is 0
	return true;
}

// lo is exact and hi the next exact value after it
template<unsigned nbits, unsigned es, typename bt>
bool adjacent(const areal<nbits, es, bt>& lo, const areal<nbits, es, bt>& hi) {
	if (lo.ubit() || lo.isnan() || lo.isinf() || hi.isnan()) return false;
	areal<nbits, es, bt> next(lo);
	next.set(0, true);
	++next;
	return next == hi;
}

} // namespace areal_parse

template<unsigned nbits, unsigned es, typename bt>
bool parse(const std::string& txt, areal<nbits, es, bt>& v) {
	std::string_view s = areal_parse::trim(txt);
	if (s.empty()) return false;
	if (s.front() == '[') {
		if (s.back() != ']') return false;
		return areal_parse::nearest_exact(areal_parse::trim(s.substr(1, s.size() - 2)), v);
	}
	if (s.front() == '(') {
		if (s.back() != ')') return false;
		const std::string_view inner = s.substr(1, s.size() - 2);
		const auto comma = inner.find(',');
		if (comma == std::string_view::npos || inner.find(',', comma + 1) != std::string_view::npos) return false;
		areal<nbits, es, bt> a, b;
		if (!areal_parse::nearest_exact(areal_parse::trim(inner.substr(0, comma)), a)) return false;
		if (!areal_parse::nearest_exact(areal_parse::trim(inner.substr(comma + 1)), b)) return false;
		// operator<< writes the endpoint nearer zero first; a mathematical interval of negative
		// values reads the other way round
		if (areal_parse::adjacent(a, b)) {
			v = a;
		}
		else if (areal_parse::adjacent(b, a)) {
			v = b;
		}
		else {
			return false;
		}
		v.set(0, true);
		return true;
	}
	return areal_parse::faithful(s, v);
}

////////////////////// operators
// operator<< and operator>> moved to iostream.hpp in #1334; operator<< stays a friend of the
// class (declared above), which is what lets this header get by with <iosfwd>.

// areal-specific equality: bit-pattern equality, intentionally diverging
// from IEEE-754 in two cases:
//   - NaN == NaN returns true when the encodings match (areal models both
//     quiet and signalling NaN via setnan(NAN_TYPE_QUIET)/setnan(NAN_TYPE_SIGNALLING)
//     and the corresponding isnan() variants; bit-pattern equality lets
//     regression suites distinguish qNaN from sNaN). Two NaNs with different
//     encodings (e.g., qNaN vs sNaN) compare unequal.
//   - +0 != -0 (different bit patterns)
// The ordering operators (<, <=, >, >=) below use IEEE-style semantics
// (NaN ordering returns false). This mixed convention is intentional and
// is locked in by the regression suite at static/range/areal/logic/logic.cpp.
template<unsigned nnbits, unsigned nes, typename nbt>
constexpr bool operator==(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) {
	for (unsigned i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) {
			return false;
		}
	}
	return true;
}
template<unsigned nnbits, unsigned nes, typename nbt>
constexpr bool operator!=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator==(lhs, rhs); }

// Sign-magnitude comparison: directly compares the bit encoding rather than
// going through arithmetic subtraction. This is constexpr-clean (the prior
// (lhs - rhs).isneg() approach would require arithmetic to be constexpr in
// every path, including the special-value handling inside operator-=, which
// is fragile). Semantics:
//   - NaN comparisons return false (IEEE-754)
//   - +0 == -0 (so neither is less than the other)
//   - Different signs: negative is less than positive
//   - Same sign: compare magnitude bits (everything except sign bit).
//     For positives: larger magnitude bits == larger value.
//     For negatives: larger magnitude bits == smaller value (more negative).
template<unsigned nnbits, unsigned nes, typename nbt>
constexpr bool operator< (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	bool lhsNeg = lhs.sign();
	bool rhsNeg = rhs.sign();
	if (lhsNeg != rhsNeg) {
		// +0 == -0 in IEEE-754; neither is strictly less than the other.
		if (lhs.iszero() && rhs.iszero()) return false;
		return lhsNeg;
	}
	// Same sign: compare magnitude bits (sign masked off in the MSU).
	using ArealType = areal<nnbits, nes, nbt>;
	constexpr nbt MAGNITUDE_MSU_MASK =
		static_cast<nbt>(ArealType::MSU_MASK & static_cast<nbt>(~ArealType::SIGN_BIT_MASK));
	for (int i = static_cast<int>(ArealType::MSU); i >= 0; --i) {
		nbt l = lhs._block[i];
		nbt r = rhs._block[i];
		if (i == static_cast<int>(ArealType::MSU)) {
			l = static_cast<nbt>(l & MAGNITUDE_MSU_MASK);
			r = static_cast<nbt>(r & MAGNITUDE_MSU_MASK);
		}
		if (l != r) {
			// Positives: smaller magnitude bits => smaller value.
			// Negatives: smaller magnitude bits => larger value (closer to zero).
			return lhsNeg ? (l > r) : (l < r);
		}
	}
	return false; // equal magnitudes
}
template<unsigned nnbits, unsigned nes, typename nbt>
constexpr bool operator> (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return  operator< (rhs, lhs); }
// IEEE-754: any comparison involving NaN is false (including <= and >=);
// otherwise <= is the negation of > and >= is the negation of <.
template<unsigned nnbits, unsigned nes, typename nbt>
constexpr bool operator<=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	return !operator> (lhs, rhs);
}
template<unsigned nnbits, unsigned nes, typename nbt>
constexpr bool operator>=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) {
	if (lhs.isnan() || rhs.isnan()) return false;
	return !operator< (lhs, rhs);
}

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es, typename bt>
constexpr areal<nbits, es, bt> operator+(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es, typename bt>
constexpr areal<nbits, es, bt> operator-(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es, typename bt>
constexpr areal<nbits, es, bt> operator*(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es, typename bt>
constexpr areal<nbits, es, bt> operator/(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

// to_string() and to_binary() moved to manipulators.hpp in #1334: both format through a
// std::stringstream, which is what keeps them out of the core.

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nbits, unsigned es, typename bt>
areal<nbits,es> abs(const areal<nbits,es,bt>& v) {
	return areal<nbits,es>(false, v.scale(), v.fraction(), v.isZero());
}


///////////////////////////////////////////////////////////////////////
///   binary logic literal comparisons

// areal - long long logic operators. All forward to the primary areal/areal
// operators (which are constexpr) and are themselves constexpr so they can
// participate in constant evaluation (e.g., static_assert(a == 0LL)).
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator==(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator==(lhs, areal<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator!=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator!=(lhs, areal<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator< (const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<(lhs, areal<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator> (const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<(areal<nbits, es, bt>(rhs), lhs);
}
// Forward scalar relational overloads to the primary areal/areal operators
// rather than rebuilding the relations from operator< / operator==. The
// rebuild approach was incorrect once operator== became bit-pattern equality
// (so +0 != -0) and operator<= / >= acquired their own NaN-returns-false
// guard: e.g., "lhs <= rhs" must NOT degrade to "lhs < rhs || lhs == rhs"
// because for lhs=-0 and rhs=+0 that gives "false || false" while the
// areal/areal operator<= correctly returns true.
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator<=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<=(lhs, areal<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator>=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator>=(lhs, areal<nbits, es, bt>(rhs));
}

}} // namespace sw::universal
