#pragma once
// areal_impl.hpp: implementation of an arbitrary configuration fixed-size floating-point representation with an uncertainty bit to represent a faithful floating-point system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/native/subnormal.hpp>
#include <universal/utility/find_msb.hpp>
#include <universal/native/integers.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/areal/exceptions.hpp>

#ifndef THROW_ARITHMETIC_EXCEPTION
#define THROW_ARITHMETIC_EXCEPTION 0
#endif
#ifndef TRACE_CONVERSION
#define TRACE_CONVERSION 0
#endif

namespace sw { namespace universal {
		
	constexpr bool AREAL_NIBBLE_MARKER = true;

// Forward definitions
template<unsigned nbits, unsigned es, typename bt> class areal;
template<unsigned nbits, unsigned es, typename bt> areal<nbits,es,bt> abs(const areal<nbits,es,bt>&);

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
	static constexpr int MAX_EXP = (1l << es) - EXP_BIAS;
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
		clear();
		if (0 == rhs) return *this;
		uint64_t raw = static_cast<uint64_t>(rhs);
		int exponent = int(find_msb(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr uint32_t sizeInBits = 8 * sizeof(Ty);
		uint32_t shift = sizeInBits - exponent - 1;
		raw <<= shift;
		raw = round<sizeInBits, uint64_t>(raw, exponent);
		return *this;
	}
	template<typename Ty>
	constexpr areal& convert_signed_integer(const Ty& rhs) noexcept {
		clear();
		if (0 == rhs) return *this;
		bool s = (rhs < 0);
		uint64_t raw = static_cast<uint64_t>(s ? -rhs : rhs);
		int exponent = int(find_msb(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr uint32_t sizeInBits = 8 * sizeof(Ty);
		uint32_t shift = sizeInBits - exponent - 1;
		raw <<= shift;
		raw = round<sizeInBits, uint64_t>(raw, exponent);
#ifdef LATER
		bool ubit = true;
		// construct the target areal
		if constexpr (64 >= nbits - es - 1ull) {
			uint64_t bits = (s ? 1u : 0u);
			bits <<= es;
			bits |= exponent + EXP_BIAS;
			bits <<= nbits - 1ull - es;
			bits |= raw;
			bits &= 0xFFFF'FFFE;
			bits |= (ubit ? 0x1 : 0x0);
			if constexpr (1 == nrBlocks) {
				_block[MSU] = bt(bits);
			}
			else {
				copyBits(bits);
			}
		}
		else {
			std::cerr << "TBD\n";
		}
#endif
		return *this;
	}


	CONSTEXPRESSION areal& operator=(float rhs) {
		clear();
#if BIT_CAST_IS_CONSTEXPR
		// normal number
		uint32_t bc      = std::bit_cast<uint32_t>(rhs);
		bool s           = (0x8000'0000u & bc);
		uint32_t raw_exp = uint32_t((0x7F80'0000u & bc) >> 23u);
		uint32_t raw     = (0x007F'FFFFu & bc);
#else // !BIT_CAST_IS_CONSTEXPR
		float_decoder decoder;
		decoder.f        = rhs;
		bool s           = decoder.parts.sign ? true : false;
		uint32_t raw_exp = decoder.parts.exponent;
		uint32_t raw     = decoder.parts.fraction;
#endif // !BIT_CAST_IS_CONSTEXPR

		// special case handling
		if (raw_exp == 0xFFu) { // special cases
			if (raw == 1ul) {
				// 1.11111111.00000000000000000000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				setnan(NAN_TYPE_SIGNALLING);
				return *this;
			}
			if (raw == 0x0040'0000ul) {
				// 1.11111111.10000000000000000000000 quiet nan
				// 0.11111111.10000000000000000000000 quiet nan
				setnan(NAN_TYPE_QUIET);
				return *this;
			}
			if (raw == 0ul) {
				// 1.11111111.00000000000000000000000 -inf
				// 0.11111111.00000000000000000000000 +inf
				setinf(s);
				return *this;
			}
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
		uint32_t mask = 0x007F'FFFF >> fbits; // mask for sticky bit 
		if (exponent >= MIN_EXP_SUBNORMAL && exponent < MIN_EXP_NORMAL) {
			// this number is a subnormal number in this representation
			// trick though is that it might be a normal number in IEEE single precision representation
			if (exponent > -127) {
				// the source real is a normal number, so we must add the hidden bit to the fraction bits
				raw |= (1ull << 23);
				mask = 0x00FF'FFFFu >> (fbits + exponent + subnormal_reciprocal_shift[es] + 1); // mask for sticky bit 
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
					// ubit = false; already set to false
					std::cout << "conversion of IEEE float to more precise areals not implemented yet\n";
				}
			}
			else {
				// the source real is a subnormal number, and the target representation is a subnormal representation
				mask = 0x00FF'FFFFu >> (fbits + exponent + subnormal_reciprocal_shift[es] + 1); // mask for sticky bit 
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
					// ubit = false; already set to false
					std::cout << "conversion of subnormal IEEE float to more precise areals not implemented yet\n";
				}
			}
		}
		else {
			// this number is a normal/supernormal number in this representation, we can leave the hidden bit hidden
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
			else { // all bits of the double go into this representation and need to be shifted up
				// ubit = false; already set to false
				std::cout << "conversion of IEEE double to more precise areals not implemented yet\n";
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
		uint32_t bits = (s ? 1u : 0u);
		bits <<= es;
		bits |= biasedExponent;
		bits <<= nbits - 1ull - es;
		bits |= raw;
		bits &= 0xFFFF'FFFEu;
		bits |= (ubit ? 0x1u : 0x0u);
		if constexpr (1 == nrBlocks) {
			_block[MSU] = bt(bits);
		}
		else {
			copyBits(bits);
		}
		return *this;
	}
	CONSTEXPRESSION areal& operator=(double rhs) {
		clear();
#if BIT_CAST_IS_CONSTEXPR
		// normal number
		uint64_t bc      = std::bit_cast<uint64_t>(rhs);
		bool s           = (0x8000'0000'0000'0000ull & bc);
		uint32_t raw_exp = static_cast<uint32_t>((0x7FF0'0000'0000'0000ull & bc) >> 52);
		uint64_t raw     = (0x000F'FFFF'FFFF'FFFFull & bc);
#else // !BIT_CAST_IS_CONSTEXPR
		double_decoder decoder;
		decoder.d        = rhs;
		bool s           = decoder.parts.sign ? true : false;
		uint32_t raw_exp = static_cast<uint32_t>(decoder.parts.exponent);
		uint64_t raw     = decoder.parts.fraction;
#endif // BIT_CAST_IS_CONSTEXPR
		if (raw_exp == 0x7FFul) { // special cases
			if (raw == 1ull) {
				// 1.11111111111.0000000000000000000000000000000000000000000000000001 signalling nan
				// 0.11111111111.0000000000000000000000000000000000000000000000000001 signalling nan
				setnan(NAN_TYPE_SIGNALLING);
				return *this;
			}
			if (raw == 0x0008'0000'0000'0000ull) {
				// 1.11111111111.1000000000000000000000000000000000000000000000000000 quiet nan
				// 0.11111111111.1000000000000000000000000000000000000000000000000000 quiet nan
				setnan(NAN_TYPE_QUIET);
				return *this;
			}
			if (raw == 0ull) {
				// 1.11111111111.0000000000000000000000000000000000000000000000000000 -inf
				// 0.11111111111.0000000000000000000000000000000000000000000000000000 +inf
				setinf(s);
				return *this;
			}
		}
		if (rhs == 0.0) { // IEEE rule: this is valid for + and - 0.0
			set(nbits - 1ull, s);
			return *this;
		}
		// this is not a special number
		int exponent = int(raw_exp) - 1023;  // unbias the exponent
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
		// set the exponent
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
			// this number is a subnormal number in this representation
			// but it might be a normal number in IEEE double precision representation
			// which will require a reinterpretation of the bits as the hidden bit becomes explicit in a subnormal representation
			if (exponent > -1022) {
				mask = 0x001F'FFFF'FFFF'FFFFull >> (fbits + exponent + subnormal_reciprocal_shift[es] + 1); // mask for sticky bit 
				// the source real is a normal number, so we must add the hidden bit to the fraction bits
				raw |= (1ull << 52);
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
					// ubit = false; already set to false
					std::cout << "conversion of IEEE double to more precise areals not implemented yet\n";
				}
			}
			else {
				// this is a subnormal double
				std::cout << "conversion of subnormal IEEE doubles not implemented yet\n";
			}
		}
		else {
			// this number is a normal/supernormal number in this representation, we can leave the hidden bit hidden
			biasedExponent = static_cast<uint64_t>(exponent + EXP_BIAS); // reasonable to limit exponent to 32bits

			// fraction processing
			mask = 0x000F'FFFF'FFFF'FFFF >> fbits; // mask for sticky bit 
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
				// ubit = false; already set to false
				std::cout << "conversion of IEEE double to more precise areals not implemented yet\n";
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
		uint64_t bits = (s ? 1ull : 0ull);
		bits <<= es;
		bits |= biasedExponent;
		bits <<= nbits - 1ull - es;
		bits |= raw;
		bits &= 0xFFFF'FFFF'FFFF'FFFE;
		bits |= (ubit ? 0x1 : 0x0);
		if constexpr (nrBlocks == 1) {
			_block[MSU] = bt(bits);
		}
		else {
			copyBits(bits);
		}
		return *this;
	}
	CONSTEXPRESSION areal& operator=(long double rhs) {
		return *this = double(rhs);
	}

	// arithmetic operators
	// prefix operator
	inline areal operator-() const {
		areal tmp(*this);
		tmp._block[MSU] ^= SIGN_BIT_MASK;
		return tmp;
	}

	areal& operator+=(const areal& rhs) {
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
	areal& operator+=(double rhs) {
		return *this += areal(rhs);
	}
	areal& operator-=(const areal& rhs) {
		// subtraction is addition with negated rhs
		// but we need to handle NaN specially
		if (rhs.isnan()) {
			return *this += rhs;
		}
		return *this += -rhs;
	}
	areal& operator-=(double rhs) {
		return *this -= areal<nbits, es>(rhs);
	}
	areal& operator*=(const areal& rhs) {
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
	areal& operator*=(double rhs) {
		return *this *= areal<nbits, es>(rhs);
	}
	areal& operator/=(const areal& rhs) {
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
				setinf(resultSign); // x/0 = ±inf
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
	areal& operator/=(double rhs) {
		return *this /= areal<nbits, es>(rhs);
	}
	/// <summary>
	/// move to the next bit encoding modulo 2^nbits
	/// </summary>
	/// <typeparam name="bt"></typeparam>
	inline areal& operator++() {
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
	inline areal operator++(int) {
		areal tmp(*this);
		operator++();
		return tmp;
	}
	inline areal& operator--() {

		return *this;
	}
	inline areal operator--(int) {
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
		if (i < nbits) {
			unsigned blockIndex = i /bitsInBlock;
			bt block = _block[blockIndex];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[blockIndex] = bt((block & null) | mask);
			return;
		}
	}
	/// <summary>
	/// reset a specific bit in the encoding to false. If bit index is out of bounds, no modification takes place.
	/// </summary>
	/// <param name="i">bit index to reset</param>
	/// <returns>void</returns>
	inline constexpr void reset(unsigned i) noexcept {
		if (i < nbits) {
			bt block = _block[i / bitsInBlock];
			bt mask = ~(1ull << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt(block & mask);
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
	/// assign the value of the string representation of a scientific number to the areal
	/// </summary>
	/// <param name="stringRep">decimal scientific notation of a real number to be assigned</param>
	/// <returns>reference to this areal</returns>
	inline areal& assign(const std::string& stringRep) {
		std::cout << "assign TBD\n";
		return *this;
	}

	// selectors
	inline constexpr bool sign() const noexcept { return (_block[MSU] & SIGN_BIT_MASK) == SIGN_BIT_MASK; }
	inline constexpr bool ubit() const noexcept { return (_block[0] & LSB_BIT_MASK) != 0; }
	inline constexpr int scale() const {
		int e{ 0 };
		if constexpr (MSU_CAPTURES_E) {
			e = int((_block[MSU] & ~SIGN_BIT_MASK) >> EXP_SHIFT);
			if (e == 0) {
				// subnormal scale is determined by fraction
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				e = (2l - (1l << (es - 1ull))) - 1;
				for (unsigned i = nbits - 2ull - es; i > 0; --i) {
					if (test(i)) break;
					--e;
				}
			}
			else {
				e -= EXP_BIAS;
			}
		}
		else {
			blockbinary<es, bt> ebits;
			exponent(ebits);
			if (ebits.iszero()) {
				// subnormal scale is determined by fraction
				e = -1;
				for (unsigned i = nbits - 2ull - es; i > 0; --i) {
					if (test(i)) break;
					--e;
				}
			}
			else {
				e = int(ebits) - EXP_BIAS;
			}
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
			bt word = _block[bitIndex / bitsInBlock];
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
	void constexprClassParameters() const {
		std::cout << "nbits             : " << nbits << '\n';
		std::cout << "es                : " << es << std::endl;
		std::cout << "ALLONES           : " << to_binary(ALLONES, bitsInBlock, true) << '\n';
		std::cout << "BLOCK_MASK        : " << to_binary(BLOCK_MASK, bitsInBlock, true) << '\n';
		std::cout << "nrBlocks          : " << nrBlocks << '\n';
		std::cout << "bits in MSU       : " << bitsInMSU << '\n';
		std::cout << "MSU               : " << MSU << '\n';
		std::cout << "MSU MASK          : " << to_binary(MSU_MASK, bitsInBlock, true) << '\n';
		std::cout << "SIGN_BIT_MASK     : " << to_binary(SIGN_BIT_MASK, bitsInBlock, true) << '\n';
		std::cout << "LSB_BIT_MASK      : " << to_binary(LSB_BIT_MASK, bitsInBlock, true) << '\n';
		std::cout << "MSU CAPTURES E    : " << (MSU_CAPTURES_E ? "yes\n" : "no\n");
		std::cout << "EXP_SHIFT         : " << EXP_SHIFT << '\n';
		std::cout << "MSU EXP MASK      : " << to_binary(MSU_EXP_MASK, bitsInBlock, true) << '\n';
		std::cout << "EXP_BIAS          : " << EXP_BIAS << '\n';
		std::cout << "MAX_EXP           : " << MAX_EXP << '\n';
		std::cout << "MIN_EXP_NORMAL    : " << MIN_EXP_NORMAL << '\n';
		std::cout << "MIN_EXP_SUBNORMAL : " << MIN_EXP_SUBNORMAL << '\n';
	}

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
			if (ebits.iszero()) {
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				TargetFloat exponentiation = static_cast<TargetFloat>(subnormal_exponent[es]); // precomputed values for 2^(2-2^(es-1))
				v = exponentiation * f;
			}
			else {
				// regular: (-1)^s * 2^(e+1-2^(es-1)) * (1 + f/2^fbits))
				int exponent = unsigned(ebits) + 1ll - (1ll << (es - 1ull));
				if (exponent < 64) {
					TargetFloat exponentiation = (exponent >= 0 ? TargetFloat(1ull << exponent) : (1.0f / TargetFloat(1ull << -exponent)));
					v = exponentiation * (TargetFloat(1) + f);
				}
				else {
					double exponentiation = ipow(exponent);
					v = static_cast<TargetFloat>(exponentiation * (1.0 + f));
				}
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
					for (unsigned i = 0; i < fbits; ++i) {
						tgt.setbit(divshift + fbits - 1 - i, at(1 + fbits - 1 - i));
					}
				}
			}
		}
	}

protected:
	// HELPER methods

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
		bt mask = 0xFFFFFFFFFFFFFFFF << (bitsInBlock - bitsToShift);
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

private:
	bt _block[nrBlocks];

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const areal<nnbits,nes,nbt>& r);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend std::istream& operator>> (std::istream& istr, areal<nnbits,nes,nbt>& r);

	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator==(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator!=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator< (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator> (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator<=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator>=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
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
inline void convert(const blocktriple<srcbits, op, bt>& src, areal<nbits, es, bt>& tgt, bool inputUncertain = false) {
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

		// get the rounding decision
		std::pair<bool, unsigned> alignment = src.roundingDecision(adjustment);
		bool roundup = alignment.first;
		unsigned rightShift = alignment.second;

		// check if rounding occurred (any bits shifted out that were non-zero)
		if (rightShift > 0) {
			// check if there are any non-zero bits that will be shifted out
			uint64_t significandBits = src.significand_ull();
			if (rightShift < 64) {
				uint64_t shiftedOutMask = (1ull << rightShift) - 1;
				roundingOccurred = (significandBits & shiftedOutMask) != 0;
			}
			else {
				roundingOccurred = (significandBits != 0);
			}
		}

		// construct the result
		uint64_t fracbits = src.significand_ull();
		fracbits >>= rightShift;

		// mask to fraction bits only (remove hidden bit)
		constexpr uint64_t fractionMask = (fbits < 64) ? ((1ull << fbits) - 1) : 0xFFFFFFFFFFFFFFFFull;
		fracbits &= fractionMask;

		if (roundup) {
			++fracbits;
			if (fracbits == (1ull << fbits)) { // overflow of fraction
				if (biasedExponent == ((1ull << es) - 1)) {
					// overflow to maxpos/maxneg
					if (src.sign()) tgt.maxneg(); else tgt.maxpos();
					tgt.set(0, true); // overflow
					return;
				}
				else {
					++biasedExponent;
					fracbits = 0;
				}
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
}

////////////////////// operators
template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const areal<nbits,es,bt>& v) {
	// TODO: make it a native conversion
	double d = double(v);
	bool ubit = v.at(0);
	if (ubit) {
		if (v.isnan()) {
			ostr << '[' << d << ']';
		}
		else {
			areal<nbits, es, bt> next(v);
			++next;
			double dnext = double(next);
			ostr << '(' << d << ", " << dnext << ')';
		}
	}
	else { // exact value
		ostr << '[' << d << ']';
	}
	return ostr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline std::istream& operator>>(std::istream& istr, const areal<nnbits,nes,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator==(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { 
	for (unsigned i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) {
			return false;
		}
	}
	return true;
}
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator!=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator< (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return (lhs - rhs).isneg(); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator> (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator<=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator>=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator< (lhs, rhs); }

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es, typename bt>
inline areal<nbits, es, bt> operator+(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es, typename bt>
inline areal<nbits, es, bt> operator-(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es, typename bt>
inline areal<nbits, es, bt> operator*(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es, typename bt>
inline areal<nbits, es, bt> operator/(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

// convert to std::string
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const areal<nbits,es,bt>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << " zero b";
		return s.str();
	}
	else if (v.isinf()) {
		s << " infinite b";
		return s.str();
	}
//	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
	return s.str();
}

// transform areal to a binary representation
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_binary(const areal<nbits, es, bt>& number, bool nibbleMarker = false) {
	std::stringstream ss;
	ss << 'b';
	unsigned index = nbits;
	for (unsigned i = 0; i < nbits; ++i) {
		ss << (number.at(--index) ? '1' : '0');
		if (index > 0 && (index % 4) == 0 && nibbleMarker) ss << '\'';
	}
	return ss.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nbits, unsigned es, typename bt>
areal<nbits,es> abs(const areal<nbits,es,bt>& v) {
	return areal<nbits,es>(false, v.scale(), v.fraction(), v.isZero());
}


///////////////////////////////////////////////////////////////////////
///   binary logic literal comparisons

// posit - long logic operators
template<unsigned nbits, unsigned es, typename bt>
inline bool operator==(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator==(lhs, areal<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator!=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator!=(lhs, areal<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator< (const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<(lhs, areal<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator> (const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<(areal<nbits, es, bt>(rhs), lhs);
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator<=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<(lhs, areal<nbits, es, bt>(rhs)) || operator==(lhs, areal<nbits, es, bt>(rhs));
}
template<unsigned nbits, unsigned es, typename bt>
inline bool operator>=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return !operator<(lhs, areal<nbits, es, bt>(rhs));
}

}} // namespace sw::universal
