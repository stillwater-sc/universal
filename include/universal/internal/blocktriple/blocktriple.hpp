#pragma once
// blocktriple.hpp: definition of a (sign, scale, significant) representation of a generic floating-point value
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/native/subnormal.hpp>
#include <universal/native/bit_functions.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/trace_constants.hpp>


#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */

#define BIT_CAST_SUPPORT 0
#define CONSTEXPRESSION 

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */

#define BIT_CAST_SUPPORT 0
#define CONSTEXPRESSION 

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
#define BIT_CAST_SUPPORT 1
#define CONSTEXPRESSION constexpr
#include <bit>

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw::universal {

// Forward definitions
template<size_t nbits> class blocktriple;
template<size_t nbits> blocktriple<nbits> abs(const blocktriple<nbits>& v);

template<size_t nbits>
blocktriple<nbits>& convert(unsigned long long uint, blocktriple<nbits>& tgt) {
	return tgt;
}

/// <summary>
/// Generalized blocktriple representing a (sign, scale, significant) with unrounded arithmetic
/// </summary>
/// <typeparam name="nbits">number of fraction bits, including a leading 1 bit</typeparam>
template<size_t nbits>
class blocktriple {
public:
	using bt = uint32_t;
	using bits = blockbinary<nbits, bt>; // can we make it uint64_t?
	// storage unit for block arithmetic needs to be uin32_t: carry propagation on uint64_t requires assembly code

	static constexpr size_t bitsInByte = 8ull;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr size_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64ull - bitsInBlock));

	static constexpr size_t MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken


	static constexpr size_t fhbits = nbits;
	static constexpr size_t fbits = nbits - 1;
	static constexpr size_t abits = fhbits + 3ull;         // size of the addend
	static constexpr size_t mbits = 2ull * fhbits;         // size of the multiplier output
	static constexpr size_t divbits = 3ull * fhbits + 4ull;// size of the divider output
	static constexpr bt ALL_ONES = bt(~0);

	constexpr blocktriple(const blocktriple&) noexcept = default;
	constexpr blocktriple(blocktriple&&) noexcept = default;

	constexpr blocktriple& operator=(const blocktriple&) noexcept = default;
	constexpr blocktriple& operator=(blocktriple&&) noexcept = default;

	constexpr blocktriple() noexcept : 
		_nan{ false }, 	_inf{ false }, _zero{ true }, 
		_sign{ false }, _scale{ 0 }, _significant{ 0 } {}

	// decorated constructors
	constexpr blocktriple(signed char iv) noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(short iv)       noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(int iv)         noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(long iv)        noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(long long iv)   noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(char iv)               noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(unsigned short iv)     noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(unsigned int iv)       noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(unsigned long iv)      noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(unsigned long long iv) noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(float iv)       noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(double iv)      noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }
	constexpr blocktriple(long double iv) noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0u } { *this = iv; }

	// conversion operators
	constexpr blocktriple& operator=(signed char rhs) noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(short rhs)       noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(int rhs)         noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(long rhs)        noexcept { return convert_signed_integer(rhs); }
	constexpr blocktriple& operator=(long long rhs)   noexcept { return convert_signed_integer(rhs); }

	constexpr blocktriple& operator=(char rhs)               noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(unsigned short rhs)     noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(unsigned int rhs)       noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(unsigned long rhs)      noexcept { return convert_unsigned_integer(rhs); }
	constexpr blocktriple& operator=(unsigned long long rhs) noexcept { return convert_unsigned_integer(rhs); }

	constexpr blocktriple& operator=(float rhs) noexcept { // TODO: deal with subnormals and inf
		return convert_float(rhs);
	}
	constexpr blocktriple& operator=(double rhs) noexcept { // TODO: deal with subnormals and inf
		_nan = false;
		_inf = false;
		_zero = true;
		if (rhs == 0.0f) return *this;
#if BIT_CAST_SUPPORT
		_zero = false; 
		// TODO: check inf and NaN
		_inf = false; _nan = false;
		// normal
		uint64_t bc = std::bit_cast<uint64_t>(rhs);
		_sign = (0x8000'0000'0000'0000 & bc);
		_scale = int((0x7FF0'0000'0000'0000ull & bc) >> 52) - 1023;
		uint64_t raw = (1ull << 52) | (0x000F'FFFF'FFFF'FFFFull & bc);
		_significant.set_raw_bits(round<53, uint64_t>(raw));
#else
		_zero = true;
		_sign = false;
		_scale = 0;
		_significant.clear();
#endif // !BIT_CAST_SUPPORT
		return *this;
	}
	constexpr blocktriple& operator=(long double rhs) noexcept {
		return *this = double(rhs);
	};
	
	void add(blocktriple<nbits - 1>& a, blocktriple<nbits - 1>& b) {

	}
	void mul(blocktriple<nbits / 2>& a, blocktriple<nbits / 2>& b) {

	}
	/// <summary>
	/// round a set of source bits to the present representation.
	/// srcbits is the number of bits of significant in the source representation
	/// </summary>
	/// <typeparam name="StorageType"></typeparam>
	/// <param name="raw"></param>
	/// <returns></returns>
	template<size_t srcbits, typename StorageType>
	constexpr StorageType round(StorageType raw) noexcept {
		if constexpr (nbits < srcbits) {
			// round to even: lsb guard round sticky
			// collect guard, round, and sticky bits
			// this same logic will work for the case where
			// we only have a guard bit and no round and/or sticky bits
			// because the mask logic will make round and sticky both 0
			constexpr uint32_t upper = 8*sizeof(StorageType) + 2;
			constexpr uint32_t shift = srcbits - nbits - 1ull;
			StorageType mask = (StorageType{ 1ull } << shift);
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			if constexpr (shift > 1 && shift < upper) { // protect against a negative shift
				StorageType allones(StorageType(~0)); 
				mask = StorageType(allones << (shift - 2));
				mask = ~mask;
			}
			else {
				mask = 0;
			}
			bool sticky = (mask & raw);

			raw >>= (shift + 1);  // shift out the bits we are rounding away
			bool lsb = (raw & 0x1);
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
				if (raw == (1ull << nbits)) { // overflow
					++_scale;
					raw >>= 1;
				}
			}
		}
		else {
			constexpr size_t shift = nbits - srcbits;
			if constexpr (shift < sizeof(StorageType)) {
				raw <<= shift;
			}
			else {
#if !BIT_CAST_SUPPORT
				std::cerr << "round: shift " << shift << " is too large (>= " << sizeof(StorageType) << ")\n";
#endif
			}
		}
		StorageType significant = raw;
		return significant;
	}

	// modifiers
	constexpr void clear() noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		_sign = false;
		_scale = 0;
		_significant.clear();
	}
	constexpr void setzero(bool sign = false) noexcept {
		clear();
		_sign = sign;
	}
	constexpr void setnan(bool sign = true) noexcept {
		clear();
		_nan = true;
		_inf = false;
		_zero = false;
		_sign = sign;   // if true, signalling NaN, otherwise quiet
	}
	constexpr void setinf(bool sign = true) noexcept {
		clear();
		_inf = true;
		_zero = false;
		_sign = sign;
	}
	constexpr void set_raw_bits(uint64_t raw) noexcept {
		clear();
		_significant.set_raw_bits(raw);
	}
	// set a non-zero, non-inf, non-nan value
	constexpr void set(bool s, int scale, bits& significant) {
		_nan = false;
		_inf = false;
		_zero = false;
		_sign = s;
		_scale = scale;
		_significant = significant;
	}
	constexpr void setpos() noexcept { _sign = false; }

	// selectors
	inline constexpr bool isnan()       const noexcept { return _nan; }
	inline constexpr bool isinf()       const noexcept { return _inf; }
	inline constexpr bool iszero()      const noexcept { return _zero; }
	inline constexpr bool ispos()       const noexcept { return !_sign; }
	inline constexpr bool isneg()       const noexcept { return _sign; }
	inline constexpr bool sign()        const noexcept { return _sign; }
	inline constexpr int  scale()       const noexcept { return _scale; }
	inline constexpr bits significant() const noexcept { return _significant; }

	// fraction bit accessors
	inline constexpr bool at(size_t index)   const noexcept { return _significant.at(index); }
	inline constexpr bool test(size_t index) const noexcept { return _significant.at(index); }

	// conversion operators
	explicit operator float()       const noexcept { return to_float(); }
	explicit operator double()      const noexcept { return to_double(); }
	explicit operator long double() const noexcept { return to_long_double(); }

	template<size_t targetBits>
	blockbinary<targetBits, bt> alignSignificant(int alignmentShift) const {
		blockbinary<targetBits, bt> v;
		v.assignWithoutSignExtend(_significant);
		if (fhbits + static_cast<size_t>(alignmentShift) >= targetBits) {
			std::cerr << "alignmentShift " << alignmentShift << " is too large (>" << targetBits << ")\n";
			v.clear();
			return v;
		}
		return v <<= alignmentShift;
	}

#ifdef NEVER
	/// Normalized shift (e.g., for addition).
	template <size_t tgtSize>
	blockbinary<tgtSize, bt> nshift(int shift) const {
		blockbinary<tgtSize, bt> number;

		// Check range
		if (static_cast<int>(fbits) + shift >= static_cast<int>(tgtSize)) {
			std::cerr << "nshift: shift is too large\n";
			number.reset();
			return number;
		}

		int hpos = static_cast<int>(fbits) + shift;       // position of hidden bit
		if (hpos <= 0) {   // If hidden bit is LSB or beyond just set uncertainty bit and call it a day
			number[0] = true;
			return number;
		}
		number[size_t(hpos)] = true;           // hidden bit now safely set

											   // Copy fraction bits into certain part
		for (int npos = hpos - 1, fpos = int(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
			number[size_t(npos)] = _fraction[size_t(fpos)];

		// Set uncertainty bit
		bool uncertainty = false;
		for (int fpos = std::min(int(fbits) - 1, -shift); fpos >= 0 && !uncertainty; --fpos)
			uncertainty |= _fraction[size_t(fpos)];
		number[0] = uncertainty;
		return number;
	}
#endif

private:
	           // special cases to keep track of
	bool _nan; // most dominant state
	bool _inf; // second most dominant state
	bool _zero;// third most dominant special case

			   // the triple (sign, scale, significant)
	bool _sign;
	int  _scale;
	bits _significant;

	// helpers

	template<typename Ty>
	constexpr inline blocktriple& convert_unsigned_integer(const Ty& rhs) noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		if (0 == rhs) return *this;
		_zero = false;
		_sign = false;
		uint64_t raw = static_cast<uint64_t>(rhs);
		_scale = int(findMostSignificantBit(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr uint32_t sizeInBits = 8 * sizeof(Ty);
		uint32_t shift = sizeInBits - _scale - 1;
		raw <<= shift;
		_significant = round<sizeInBits, uint64_t>(raw);
		return *this;
	}
	template<typename Ty>
	constexpr inline blocktriple& convert_signed_integer(const Ty& rhs) noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		if (0 == rhs) return *this;
		_zero = false;
		_sign = (rhs < 0);
		uint64_t raw = static_cast<uint64_t>(_sign ? -rhs : rhs);
		_scale = int(findMostSignificantBit(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr uint32_t sizeInBits = 8 * sizeof(Ty);
		uint32_t shift = sizeInBits - _scale - 1;
		raw <<= shift;
		_significant = round<sizeInBits, uint64_t>(raw);
		return *this;
	}

	constexpr inline blocktriple& convert_float(const float& rhs) noexcept {
		clear();
#if BIT_CAST_SUPPORT
		// normal number
		uint32_t bc = std::bit_cast<uint32_t>(rhs);
		bool s = (0x8000'0000u & bc);
		uint32_t raw_exp = uint32_t((0x7F80'0000u & bc) >> 23u);
		uint32_t raw = (0x007F'FFFFu & bc);
#else // !BIT_CAST_SUPPORT
		float_decoder decoder;
		decoder.f = rhs;
		bool s = decoder.parts.sign ? true : false;
		uint32_t raw_exp = decoder.parts.exponent;
		uint32_t raw = decoder.parts.fraction;
#endif // !BIT_CAST_SUPPORT

		// special case handling
		if (raw_exp == 0xFFu) { // special cases
			if (raw == 1ul || raw == 0x0040'0001ul) {
				// 1.11111111.00000000000000000000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				// MSVC
				// 1.11111111.10000000000000000000001 signalling nan
				// 0.11111111.10000000000000000000001 signalling nan
				// NAN_TYPE_SIGNALLING;
				_nan = true;
				_inf = true; // this is the encoding of a signalling NaN
				return *this;
			}
			if (raw == 0x0040'0000ul) {
				// 1.11111111.10000000000000000000000 quiet nan
				// 0.11111111.10000000000000000000000 quiet nan
				// NAN_TYPE_QUIET);
				_nan = true;
				_inf = false; // this is the encoding of a quiet NaN
				return *this;
			}
			if (raw == 0ul) {
				// 1.11111111.00000000000000000000000 -inf
				// 0.11111111.00000000000000000000000 +inf
				_inf = true;
				_sign = s;  // + or - infinity
				return *this;
			}
		}
		if (rhs == 0.0) { // IEEE rule: this is valid for + and - 0.0
			_zero = true;
			_sign = s;
			return *this;
		}

		// this is not a special number
		// normal number consists of 23 fraction bits and one hidden bit, and no hidden bit for a subnormal
		int exponent = int(raw_exp) - 127;  // unbias the exponent

		constexpr size_t es = 8;
		constexpr int EXP_BIAS = 127;
		constexpr int MAX_EXP = 128;
		constexpr int MIN_EXP_NORMAL = 1 - EXP_BIAS;
		constexpr int MIN_EXP_SUBNORMAL = MIN_EXP_NORMAL - 23;
		// saturate to maxpos if out of range
		if (exponent > MAX_EXP) {
			// saturate to maxpos or maxneg
			return *this;
		}
		if (exponent < MIN_EXP_SUBNORMAL - 1) { // TODO: explain the MIN_EXP_SUBMORNAL - 1
			// set -0
			return *this;
		}
		// set the exponent
		uint32_t biasedExponent{ 0 };
		int shiftRight = 23 - static_cast<int>(fbits); // this is the bit shift to get the MSB of the src to the MSB of the tgt
		int adjustment{ 0 };
		uint32_t mask = 0x007F'FFFFu >> fbits; // mask for rounding
		if (exponent >= (MIN_EXP_SUBNORMAL - 1) && exponent < MIN_EXP_NORMAL) {
			// this number is a subnormal number in this representation
			// but it might be a normal number in IEEE single precision (float) representation
			if (exponent > -127) {
				// the source real is a normal number, so we must add the hidden bit to the fraction bits
				raw |= (1ull << 23);
				uint32_t subnormalShift = static_cast<uint32_t>(static_cast<int>(fbits) + exponent + subnormal_reciprocal_shift[es] + 1);
				mask = 0x00FF'FFFFu >> subnormalShift; // mask for rounding 
				// fraction processing: we have 24 bits = 1 hidden + 23 explicit fraction bits 
				// f = 1.ffff 2^exponent * 2^fbits * 2^-(2-2^(es-1)) = 1.ff...ff >> (23 - (-exponent + fbits - (2 -2^(es-1))))
				// -exponent because we are right shifting and exponent in this range is negative
				adjustment = -(exponent + subnormal_reciprocal_shift[es]); // this is the right shift adjustment required for subnormal representation due to the scale of the input number, i.e. the exponent of 2^-adjustment
				if (shiftRight > 0) {		// if true we need to round
					//  ... lsb | guard  round sticky   round
					//       x     0       x     x       down
					//       0     1       0     0       down  round to even
					//       1     1       0     0        up   round to even
					//       x     1       0     1        up
					//       x     1       1     0        up
					//       x     1       1     1        up
					// collect lsb, guard, round, and sticky bits

					// we need to project the rounding masks to the source bits to maximize information

					mask = (1ul << (23 - static_cast<int>(fbits) + adjustment)); // bit mask for the lsb bit
					bool lsb = (mask & raw);
					mask >>= 1;
					bool guard = (mask & raw);
					mask >>= 1;
					bool round = (mask & raw);
					if (shiftRight > 1) {
						mask = (0xFFFF'FFFFul << ((shiftRight - 2) + adjustment));
						mask = ~mask;
					}
					else {
						mask = 0;
					}

					bool sticky = (mask & raw);
					raw >>= shiftRight + adjustment;

					if (guard) {
						if (lsb && (!round && !sticky)) ++raw; // round to even
						if (round || sticky) ++raw;
						if (raw == (1ul << fbits)) { // overflow
							++biasedExponent;
							raw = 0;
						}
					}
				}
				else { // all bits of the float go into this representation and need to be shifted up
					std::cout << "conversion of IEEE float to more precise bfloats not implemented yet\n";
				}
			}
			else {
				// the source real is a subnormal number, and the target representation is a subnormal representation
				mask = 0x00FF'FFFFu >> (fbits + exponent + subnormal_reciprocal_shift[es] + 1); // mask for sticky bit 
				// fraction processing: we have 24 bits = 1 hidden + 23 explicit fraction bits 
				// f = 1.ffff 2^exponent * 2^fbits * 2^-(2-2^(es-1)) = 1.ff...ff >> (23 - (-exponent + fbits - (2 -2^(es-1))))
				// -exponent because we are right shifting and exponent in this range is negative
				adjustment = -(exponent + subnormal_reciprocal_shift[es]); // this is the right shift adjustment due to the scale of the input number, i.e. the exponent of 2^-adjustment
				if (shiftRight > 0) {		// if true we need to round
					std::cout << "conversion of subnormal IEEE float to subnormal bfloat not implemented yet\n";
				}
				else { // all bits of the float go into this representation and need to be shifted up
					std::cout << "conversion of subnormal IEEE float to more precise bfloats not implemented yet\n";
				}
			}
		}
		else {
			// the input is a normal, and the representation is a normal
			biasedExponent = static_cast<uint32_t>(exponent + EXP_BIAS); // reasonable to limit exponent to 32bits

			// fraction processing
			// float structure is: seee'eeee'efff'ffff'ffff'ffff'ffff'ffff, s = sign, e - exponent bit, f = fraction bit
			// target structure is for example bfloat<8,2>: seef'ffff
			// since both are normals, we can shift the incoming fraction to the target structure bits, and round
			// MSB of source = 23 - 1, MSB of target = fbits - 1: shift = MSB of src - MSB of tgt => 23 - fbits
			adjustment = 0;
			if (shiftRight > 0) {		// if true we need to round
				// round-to-even logic
				//  ... lsb | guard  round sticky   round
				//       x     0       x     x       down
				//       0     1       0     0       down  round to even
				//       1     1       0     0        up   round to even
				//       x     1       0     1        up
				//       x     1       1     0        up
				//       x     1       1     1        up
				// collect lsb, guard, round, and sticky bits
				mask = (1ul << (23 - static_cast<int>(fbits)));
				bool lsb = (mask & raw);
				mask >>= 1;
				bool guard = (mask & raw);
				mask >>= 1;
				bool round = (mask & raw);
				if (shiftRight > 1) {
					mask = (0xFFFF'FFFFul << (shiftRight - 2));
					mask = ~mask;
				}
				else {
					mask = 0;
				}

				bool sticky = (mask & raw);
				raw >>= shiftRight + adjustment;

				// execute rounding operation
				if (guard) {
					if (lsb && (!round && !sticky)) ++raw; // round to even
					if (round || sticky) ++raw;
					if (raw == (1ul << fbits)) { // overflow
						++biasedExponent;
						raw = 0;
					}
				}

		}
			else { // all bits of the double go into this representation and need to be shifted up
//				std::cout << "conversion of IEEE double to more precise bfloats not implemented yet\n";
			}
	}

		// construct the target bfloat
		uint32_t bits = (s ? 1u : 0u);
		bits <<= es;
		bits |= biasedExponent;
		bits <<= nbits - 1ull - es;
		bits |= raw;
		_significant.set_raw_bits(bits);

#ifdef LATER
		// implement saturation
		if (this->isinf(INF_TYPE_POSITIVE) || this->isnan(NAN_TYPE_QUIET)) {
			clear();
			flip();
			reset(nbits - 1ull);
			reset(1ull);
		}
		else if (this->isinf(INF_TYPE_NEGATIVE) || this->isnan(NAN_TYPE_SIGNALLING)) {
			clear();
			flip();
			reset(1ull);
		}
#endif
		return *this;
	}

	double      to_float() const {
		return float(to_double());
	}
	double      to_double() const {  // TODO: this needs a native, correctly rounded version
		if (_zero) return 0.0;
		double v = 1.0;
		double scale = 0.5;
		for (int i = int(nbits) - 2; i >= 0; i--) {
			if (_significant.test(size_t(i))) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		v *= std::pow(2.0, _scale);
		return (_sign ? -v : v);
	}
	long double to_long_double() const {
		return (long double)(to_double());
	}

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits>
	friend std::ostream& operator<< (std::ostream& ostr, const blocktriple<nnbits>& a);
	template<size_t nnbits>
	friend std::istream& operator>> (std::istream& istr, blocktriple<nnbits>& a);

	// declare as friends to avoid needing a marshalling function to get significant bits out
	template<size_t nnbits>
	friend std::string to_binary(const blocktriple<nnbits>&, bool);
	template<size_t nnbits>
	friend std::string to_triple(const blocktriple<nnbits>&, bool);

	// logic operators
	template<size_t nnbits>
	friend bool operator==(const blocktriple<nnbits>& lhs, const blocktriple<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator!=(const blocktriple<nnbits>& lhs, const blocktriple<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator< (const blocktriple<nnbits>& lhs, const blocktriple<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator> (const blocktriple<nnbits>& lhs, const blocktriple<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator<=(const blocktriple<nnbits>& lhs, const blocktriple<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator>=(const blocktriple<nnbits>& lhs, const blocktriple<nnbits>& rhs);
};

////////////////////// operators
template<size_t nbits>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<nbits>& a) {
	if (a._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << double(a);
	}
	return ostr;
}

template<size_t nbits>
inline std::istream& operator>> (std::istream& istr, const blocktriple<nbits>& a) {
	istr >> a._fraction;
	return istr;
}

template<size_t nbits>
inline bool operator==(const blocktriple<nbits>& lhs, const blocktriple<nbits>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._significant == rhs._significant && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }

template<size_t nbits>
inline bool operator!=(const blocktriple<nbits>& lhs, const blocktriple<nbits>& rhs) { return !operator==(lhs, rhs); }

template<size_t nbits>
inline bool operator< (const blocktriple<nbits>& lhs, const blocktriple<nbits>& rhs) {
	if (lhs._inf) {
		if (rhs._inf) return false; else return true; // everything is less than -infinity
	}
	else {
		if (rhs._inf) return false;
	}

	if (lhs._zero) {
		if (rhs._zero) return false; // they are both 0
		if (rhs._sign) return false; else return true;
	}
	if (rhs._zero) {
		if (lhs._sign) return true; else return false;
	}
	if (lhs._sign) {
		if (rhs._sign) {	// both operands are negative
			if (lhs._scale > rhs._scale) {
				return true;	// lhs is more negative
			}
			else {
				if (lhs._scale == rhs._scale) {
					// compare the fraction, which is an unsigned value
					if (lhs._significant == rhs._significant) return false; // they are the same value
					if (lhs._significant > rhs._significant) {
						return true; // lhs is more negative
					}
					else {
						return false; // lhs is less negative
					}
				}
				else {
					return false; // lhs is less negative
				}
			}
		}
		else {
			return true; // lhs is negative, rhs is positive
		}
	}
	else {
		if (rhs._sign) {
			return false; // lhs is positive, rhs is negative
		}
		else {
			if (lhs._scale > rhs._scale) {
				return false; // lhs is more positive
			}
			else {
				if (lhs._scale == rhs._scale) {
					// compare the fractions
					if (lhs._significant == rhs._significant) return false; // they are the same value
					if (lhs._significant > rhs._significant) {
						return false; // lhs is more positive than rhs
					}
					else {
						return true; // lhs is less positive than rhs
					}
				}
				else {
					return true; // lhs is less positive
				}
			}
		}
	}
}

template<size_t nbits>
inline bool operator> (const blocktriple<nbits>& lhs, const blocktriple<nbits>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits>
inline bool operator<=(const blocktriple<nbits>& lhs, const blocktriple<nbits>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits>
inline bool operator>=(const blocktriple<nbits>& lhs, const blocktriple<nbits>& rhs) { return !operator< (lhs, rhs); }


////////////////////////////////// string conversion functions //////////////////////////////

template<size_t nbits>
std::string to_binary(const sw::universal::blocktriple<nbits>& a, bool bNibbleMarker = true) {
	return to_triple(a, bNibbleMarker);
}

template<size_t nbits>
std::string to_triple(const blocktriple<nbits>& a, bool bNibbleMarker = true) {
	std::stringstream s;
	s << (a._sign ? "(-, " : "(+, ");
	s << a._scale << ", ";
	s << to_binary(a._significant, bNibbleMarker) << ')';
	return s.str();
}

template<size_t nbits>
blocktriple<nbits> abs(const blocktriple<nbits>& a) {
	blocktriple<nbits> absolute(a);
	absolute.setpos();
	return absolute;
}
// add two numbers with nbits significant bits, return the sumbits unrounded result value
template<size_t nbits, size_t sumbits>
void module_add(const blocktriple<nbits>& lhs, const blocktriple<nbits>& rhs, blocktriple<sumbits>& result) {
	using bt = uint32_t; // same as the storage of blocktriple
	int lhs_scale = lhs.scale();
	int rhs_scale = rhs.scale();
	int scale_of_result = std::max(lhs_scale, rhs_scale);

	// align the significants and add a leading 0 bit so that we can 
	// transform to a 2's complement encoding for negative numbers
	blockbinary<sumbits, bt> r1 = lhs.template alignSignificant<sumbits>(lhs_scale - scale_of_result + 3);
	blockbinary<sumbits, bt> r2 = rhs.template alignSignificant<sumbits>(rhs_scale - scale_of_result + 3);

	if (lhs.isneg()) r1 = twosComplement(r1);
	if (rhs.isneg()) r2 = twosComplement(r2);
	blockbinary<sumbits, bt> sum = r1 + r2;

	if constexpr (_trace_btriple_add) {
		std::cout << "r1  : " << to_binary(r1) << " : " << r1 << '\n';
		std::cout << "r2  : " << to_binary(r2) << " : " << r2 << '\n';
		std::cout << "sum : " << to_binary(sum) << " : " << sum << '\n';
	}
	if (sum.iszero()) {                         
		result.clear();
	}
	else {
		bool sign = false;
		if (sum.isneg()) {
			sum = twosComplement(sum);
			sign = true;
		}
		int shift = 0;
		// TODO: normalize subnormal if needed
		scale_of_result -= shift;
		result.set(sign, scale_of_result, sum);
	}
}

}  // namespace sw::universal
