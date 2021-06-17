#pragma once
// cfloat.hpp: 'classic' float: definition of an arbitrary configuration linear floating-point representation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// compiler specific configuration for C++20 bit_cast
#include <universal/utility/bit_cast.hpp>
// supporting types and functions
#include <universal/native/ieee754.hpp>
#include <universal/native/subnormal.hpp>
#include <universal/native/bit_functions.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
// cfloat exception structure
#include <universal/number/cfloat/exceptions.hpp>
// composition types used by cfloat
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

#ifndef CFLOAT_THROW_ARITHMETIC_EXCEPTION
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#endif
#ifndef TRACE_CONVERSION
#define TRACE_CONVERSION 0
#endif

namespace sw::universal {

constexpr bool _trace_cfloat_add = false;  // TODO consolidate in a trace include file

/*
 * classic floats have denorms, but no gradual overflow, and 
 * project values outside of their dynamic range to +-inf
 * 
 * Behavior flags
 *   gradual underflow: use all fraction encodings when exponent is all 0's
 *   gradual overflow: use all fraction encodings when exponent is all 1's
 *   saturation to maxneg or maxpos when value is out of dynamic range
 */
// Forward definitions
template<size_t nbits, size_t es, typename bt, 
	bool hasSubnormals, bool hasSupernormals, bool isSaturating> class cfloat;
template<size_t nbits, size_t es, typename bt,
	bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> 
	abs(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&);

/// <summary>
/// decode an cfloat value into its constituent parts
/// </summary>
/// <typeparam name="bt"></typeparam>
/// <param name="v"></param>
/// <param name="s"></param>
/// <param name="e"></param>
/// <param name="f"></param>
template<size_t nbits, size_t es, size_t fbits, typename bt,
	bool hasSubnormals, bool hasSupernormals, bool isSaturating>
void decode(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v, bool& s, blockbinary<es, bt>& e, blockbinary<fbits, bt>& f) {
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
template<size_t nbits, size_t es, typename bt,
	bool hasSubnormals, bool hasSupernormals, bool isSaturating>
int scale(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
	return v.scale();
}

/// <summary>
/// parse a text string into a cfloat value
/// </summary>
/// <typeparam name="bt"></typeparam>
/// <param name="str"></param>
/// <returns></returns>
template<size_t nbits, size_t es, typename bt,
	bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> parse(const std::string& str) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{ 0 };
	if (str[0] == 'b') {
		size_t index = nbits;
		for (size_t i = 1; i < str.size(); ++i) {
			if (str[i] == '1') {
				a.setbit(--index, true);
			}
			else if (str[i] == '0') {
				a.setbit(--index, false);
			}
			else if (str[i] == '.' || str[i] == '\'') {
				// ignore annotation
			}
		}
	}
	else {
		std::cerr << "parse currently only parses binary string formats\n";
	}
	return a;
}

// convert a blocktriple to a cfloat
template<size_t srcbits, size_t nbits, size_t es, typename bt,
	bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline /*constexpr*/ void convert(const blocktriple<srcbits, bt>& src, 
	                              cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& tgt) {
//	std::cout << "convert: " << to_binary(src) << std::endl;
	// test special cases
	if (src.isnan()) {
		tgt.setnan(src.sign());
	}
	else	if (src.isinf()) {
		tgt.setinf(src.sign());
	}
	else 	if (src.iszero()) {
		tgt.setzero();
		tgt.setsign(src.sign()); // preserve sign
	}
	else {
		int64_t scale   = src.scale();
		int64_t expBias = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::EXP_BIAS;
		if (scale < cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::MIN_EXP_SUBNORMAL) {
			tgt.setzero();
			return;
		}
		if (scale > cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::MAX_EXP) {
			if (src.sign()) {
				tgt.maxneg();
			}
			else {
				tgt.maxpos();
			}
			return;
		}

		// tgt.clear();
		if constexpr (nbits < 65) {
			// we can use a uint64_t to construct the cfloat
			uint64_t raw = (src.sign() ? 1ull : 0ull);
			raw <<= es; // shift to make room for the exponent bits
			if (scale >= cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::MIN_EXP_SUBNORMAL && scale < cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::MIN_EXP_NORMAL) {
				// resulting cfloat will be a subnormal number: all exponent bits are 0
				raw <<= cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits;
				int rightShift = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::MIN_EXP_NORMAL - static_cast<int>(scale);
				uint64_t fracbits = (1ull << srcbits) | src.fraction_ull(); // add the hidden bit explicitely as it will shift into the msb of the denorm
				//uint64_t fracbits = src.fraction_ull();
				fracbits >>= rightShift + (srcbits - cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits);
				raw |= fracbits;
			}
			else {
				// resulting cfloat will be a normal number: construct the exponent
				raw |= scale + expBias;  // this is guaranteed to be an unsigned string of bits
				raw <<= cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits;
				uint64_t fracbits = src.fraction_ull();
				constexpr size_t shift = srcbits - cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits;

				//  ... lsb | guard  round sticky   round
				//       x     0       x     x       down
				//       0     1       0     0       down  round to even
				//       1     1       0     0        up   round to even
				//       x     1       0     1        up
				uint64_t mask = (1ull << shift);
				bool lsb = fracbits & mask;
				mask >>= 1;
				bool guard = fracbits & mask;
				mask >>= 1;
				bool round = fracbits & mask;
				mask = 0xFFFF'FFFF'FFFF'FFFF << (shift - 2);
				mask = ~mask;
//				std::cout << to_binary(fracbits) << std::endl;
//				std::cout << to_binary(mask) << std::endl;
				bool sticky = fracbits & mask;
				bool roundup = (guard && (lsb || (round || sticky)));
//				std::cout << (roundup ? "rounding up\n" : "rounding down\n");
				fracbits >>= shift;
				fracbits += (roundup ? 1ull : 0ull);
				raw |= fracbits;
			}
			tgt.setbits(raw);
		}
		else {
			// compose the segments
			tgt.setsign(src.sign());
			tgt.setexponent(src.scale());
			// this api doesn't work: tgt.setfraction(src.significant());
			std::cerr << "convert nbits > 64 TBD\n";
		}

	}
}


/// <summary>
/// An arbitrary configuration real number with gradual under/overflow and uncertainty bit
/// </summary>
/// <typeparam name="nbits">number of bits in the encoding</typeparam>
/// <typeparam name="es">number of exponent bits in the encoding</typeparam>
/// <typeparam name="bt">the type to use as storage class: one of [uint8_t|uint16_t|uint32_t]</typeparam>
template<size_t _nbits, size_t _es, typename bt = uint8_t,
	bool _hasSubnormals = true, bool _hasSupernormals = true, bool _isSaturating = false>
class cfloat {
public:
	static_assert(_nbits > _es + 1ull, "nbits is too small to accomodate the requested number of exponent bits");
	static_assert(_es < 2147483647ull, "my God that is a big number, are you trying to break the Interweb?");
	static_assert(_es > 0, "number of exponent bits must be bigger than 0 to be a classic floating point number");
	static constexpr size_t bitsInByte = 8ull;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static_assert(bitsInBlock <= 64, "storage unit for block arithmetic needs to be <= uint64_t"); // TODO: carry propagation on uint64_t requires assembly code
	static constexpr size_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64ull - bitsInBlock));
	static constexpr bt ALL_ONES = bt(~0); // block type specific all 1's value

	static constexpr size_t nbits = _nbits;
	static constexpr size_t es = _es;
	static constexpr size_t fbits  = nbits - 1ull - es;    // number of fraction bits excluding the hidden bit
	static constexpr size_t fhbits = nbits - es;           // number of fraction bits including the hidden bit
	static constexpr size_t abits = fhbits + 3ull;         // size of the addend
	static constexpr size_t mbits = 2ull * fhbits;         // size of the multiplier output
	static constexpr size_t divbits = 3ull * fhbits + 4ull;// size of the divider output

	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr size_t MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken
	static constexpr bt     MSU_MASK = (ALL_ONES >> (nrBlocks * bitsInBlock - nbits));
	static constexpr size_t bitsInMSU = bitsInBlock - (nrBlocks * bitsInBlock - nbits);
	static constexpr size_t fBlocks = 1ull + ((fbits - 1ull) / bitsInBlock); // nr of blocks with fraction bits
	static constexpr size_t FSU = fBlocks - 1ull;  // FSU = Fraction Significant Unit: the index of the block that contains the most significant fraction bits
	static constexpr bt     FSU_MASK = (ALL_ONES >> (fBlocks * bitsInBlock - fbits));
	static constexpr size_t bitsInFSU = bitsInBlock - (fBlocks * bitsInBlock - fbits);

	static constexpr bt SIGN_BIT_MASK = bt(bt(1ull) << ((nbits - 1ull) % bitsInBlock));
	static constexpr bt LSB_BIT_MASK = bt(1ull);
	static constexpr bool MSU_CAPTURES_E = (1ull + es) <= bitsInMSU;
	static constexpr size_t EXP_SHIFT = (MSU_CAPTURES_E ? (1 == nrBlocks ? (nbits - 1ull - es) : (bitsInMSU - 1ull -es)) : 0);
	static constexpr bt MSU_EXP_MASK = ((ALL_ONES << EXP_SHIFT) & ~SIGN_BIT_MASK) & MSU_MASK;
	static constexpr int EXP_BIAS = ((1l << (es - 1ull)) - 1l);
	static constexpr int MAX_EXP = (1l << es) - EXP_BIAS;
	static constexpr int MIN_EXP_NORMAL = 1 - EXP_BIAS;
	static constexpr int MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP
	static constexpr bt BLOCK_MASK = bt(-1);

	static constexpr bool hasSubnormals   = _hasSubnormals;
	static constexpr bool hasSupernormals = _hasSupernormals;
	static constexpr bool isSaturating    = _isSaturating;
	typedef bt BlockType;

	// constructors
	constexpr cfloat() noexcept : _block{ 0 } {};

	constexpr cfloat(const cfloat&) noexcept = default;
	constexpr cfloat(cfloat&&) noexcept = default;

	constexpr cfloat& operator=(const cfloat&) noexcept = default;
	constexpr cfloat& operator=(cfloat&&) noexcept = default;

	// decorated/converting constructors

	/// <summary>
	/// construct an cfloat from another, block type bt must be the same
	/// </summary>
	/// <param name="rhs"></param>
	template<size_t nnbits, size_t ees>
	cfloat(const cfloat<nnbits, ees, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
		// this->assign(rhs);
	}

	// specific value constructor
	constexpr cfloat(const SpecificValue code) : _block{ 0 } {
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
		}
	}

	/// <summary>
	/// construct an cfloat from a native type, specialized for size
	/// </summary>
	/// <param name="iv">initial value to construct</param>
	constexpr cfloat(signed char iv)        noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(short iv)              noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(int iv)                noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(long iv)               noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(long long iv)          noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(char iv)               noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(unsigned short iv)     noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(unsigned int iv)       noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(unsigned long iv)      noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(unsigned long long iv) noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(float iv)              noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(double iv)             noexcept : _block{ 0 } { *this = iv; }
	constexpr cfloat(long double iv)        noexcept : _block{ 0 } { *this = iv; }

	// assignment operators
	constexpr cfloat& operator=(signed char rhs) { return convert_signed_integer(rhs); }
	constexpr cfloat& operator=(short rhs)       { return convert_signed_integer(rhs); }
	constexpr cfloat& operator=(int rhs)         { return convert_signed_integer(rhs); }
	constexpr cfloat& operator=(long rhs)        { return convert_signed_integer(rhs); }
	constexpr cfloat& operator=(long long rhs)   { return convert_signed_integer(rhs); }

	constexpr cfloat& operator=(char rhs)               { return convert_unsigned_integer(rhs); }
	constexpr cfloat& operator=(unsigned short rhs)     { return convert_unsigned_integer(rhs); }
	constexpr cfloat& operator=(unsigned int rhs)       { return convert_unsigned_integer(rhs); }
	constexpr cfloat& operator=(unsigned long rhs)      { return convert_unsigned_integer(rhs); }
	constexpr cfloat& operator=(unsigned long long rhs) { return convert_unsigned_integer(rhs); }

	CONSTEXPRESSION cfloat& operator=(float rhs)        { return convert_ieee754(rhs); }
	CONSTEXPRESSION cfloat& operator=(double rhs)       { return convert_ieee754(rhs); }
	CONSTEXPRESSION cfloat& operator=(long double rhs)  { return convert_ieee754(rhs); }

	// arithmetic operators
	// prefix operator
	inline cfloat operator-() const {
		cfloat tmp(*this);
		tmp._block[MSU] ^= SIGN_BIT_MASK;
		return tmp;
	}

	cfloat& operator+=(const cfloat& rhs) {
		if constexpr (_trace_cfloat_add) std::cout << "---------------------- ADD -------------------" << std::endl;
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
			return *this;
		}
		if (rhs.isinf()) {
			*this = rhs;
			return *this;
		}
		if (iszero()) {
			*this = rhs;
			return *this;
		}
		if (rhs.iszero()) return *this;

		// arithmetic operation
		blocktriple<abits, bt> a, b, sum;

		// transform the inputs into (sign,scale,significant) 
		// triples of the correct width
		normalizeAddition(a); 
		rhs.normalizeAddition(b);
		sum.add(a, b);

		convert(sum, *this);

		return *this;
	}
	cfloat& operator+=(double rhs) {
		return *this += cfloat(rhs);
	}
	cfloat& operator-=(const cfloat& rhs) {
		return *this += -rhs;
	}
	cfloat& operator-=(double rhs) {
		return *this -= cfloat(rhs);
	}
	cfloat& operator*=(const cfloat& rhs) {
		return *this;
	}
	cfloat& operator*=(double rhs) {
		return *this *= cfloat(rhs);
	}
	cfloat& operator/=(const cfloat& rhs) {
		return *this;
	}
	cfloat& operator/=(double rhs) {
		return *this /= cfloat(rhs);
	}
	/// <summary>
	/// move to the next bit encoding modulo 2^nbits
	/// </summary>
	/// <typeparam name="bt"></typeparam>
	inline cfloat& operator++() {
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
	inline cfloat operator++(int) {
		cfloat tmp(*this);
		operator++();
		return tmp;
	}
	inline cfloat& operator--() {
		return *this;
	}
	inline cfloat operator--(int) {
		cfloat tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	
	/// <summary>
	/// clear the content of this cfloat to zero
	/// </summary>
	/// <returns>void</returns>
	inline constexpr void clear() noexcept {
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(0);
		}
	}
	/// <summary>
	/// set the number to +0
	/// </summary>
	/// <returns>void</returns>
	inline constexpr void setzero() noexcept { clear(); }
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
		else if constexpr (4 == nrBlocks) {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
		}
		else {
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			for (size_t i = 1; i < nrBlocks - 1; ++i) {
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
		else if constexpr (4 == nrBlocks) {
			_block[0] = BLOCK_MASK;
			_block[1] = BLOCK_MASK;
			_block[2] = BLOCK_MASK;
		}
		else {
			for (size_t i = 0; i < nrBlocks - 1; ++i) {
				_block[i] = BLOCK_MASK;
			}
		}
		_block[MSU] = NaNType == NAN_TYPE_SIGNALLING ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
	}
	inline constexpr void setsign(bool sign = true) {
		if (sign) {
			_block[MSU] |= SIGN_BIT_MASK;
		}
		else {
			_block[MSU] &= ~SIGN_BIT_MASK;
		}
	}
	inline constexpr bool setexponent(int scale) {
		if (scale < MIN_EXP_SUBNORMAL || scale > MAX_EXP) return false; // this scale cannot be represented
		if constexpr (nbits < 65) {
			// we can use a uint64_t to construct the cfloat
			//uint64_t raw{ 0 };
			if (scale >= MIN_EXP_SUBNORMAL && scale < MIN_EXP_NORMAL) {
				// we are a subnormal number: all exponent bits are 1
			}
			else {
				// we are a normal number
				//uint32_t exponent_bits = scale + EXP_BIAS;
			}
		}
		else {
			// TBD
			return false;
		}
		return true;
	}
	/// <summary>
	/// set the fraction bits given a significant in the form ???
	/// </summary>
	/// <param name="significant"></param>
	inline constexpr void setfraction(const blockbinary<fbits, bt>& fraction) {
		for (size_t i = 0; i < fbits; ++i) {
			setbit(i, fraction.test(i));
		}
	}
	inline constexpr void setfraction(uint64_t raw_bits) {
		// unoptimized as it is not meant to be an end-user API, it is a test API
		if constexpr (fbits < 65) {
			uint64_t mask{ 1ull };
			for (size_t i = 0; i < fbits; ++i) {
				setbit(i, (mask & raw_bits));
				mask <<= 1;
			}
		}
	}
	// specific number system values of interest
	inline constexpr cfloat& maxpos() noexcept {
		// maximum positive value has this bit pattern: 0-1...1-111...111, that is, sign = 0, e = 1.1, f = 111...101
		clear();
		flip();
		setbit(nbits - 1ull, false);
		setbit(1ull, false);
		return *this;
	}
	inline constexpr cfloat& minpos() noexcept {
		// minimum positive value has this bit pattern: 0-000-00...010, that is, sign = 0, e = 00, f = 00001, u = 0
		clear();
		setbit(0);
		return *this;
	}
	inline constexpr cfloat& zero() noexcept {
		// the zero value
		clear();
		return *this;
	}
	inline constexpr cfloat& minneg() noexcept {
		// minimum negative value has this bit pattern: 1-000-00...010, that is, sign = 1, e = 00, f = 00001, u = 0
		clear();
		setbit(nbits - 1ull);
		setbit(0);
		return *this;
	}
	inline constexpr cfloat& maxneg() noexcept {
		// maximum negative value has this bit pattern: 1-1...1-111...101, that is, sign = 1, e = 1.1, f = 111...101, u = 0
		clear();
		flip();
		setbit(1ull, false);
		return *this;
	}

	/// <summary>
	/// set a specific bit in the encoding to true or false. If bit index is out of bounds, no modification takes place.
	/// </summary>
	/// <param name="i">bit index to set</param>
	/// <param name="v">boolean value to set the bit to. Default is true.</param>
	/// <returns>void</returns>
	inline constexpr void setbit(size_t i, bool v = true) noexcept {
		if (i < nbits) {
			bt block = _block[i / bitsInBlock];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt((block & null) | mask);
			return;
		}
	}
	/// <summary>
	/// set the raw bits of the cfloat. This is a required API function for number systems in the Universal Numbers Library
	/// This enables verification test suites to inject specific test bit patterns using a common interface.
	//  This is a memcpy type operator, but the target number system may not have a linear memory layout and
	//  thus needs to steer the bits in potentially more complicated ways then memcpy.
	/// </summary>
	/// <param name="raw_bits">unsigned long long carrying bits that will be written verbatim to the cfloat</param>
	/// <returns>reference to the cfloat</returns>
	inline constexpr cfloat& setbits(uint64_t raw_bits) noexcept {
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
				for (size_t i = 0; i < nrBlocks; ++i) {
					_block[i] = raw_bits & storageMask;
					raw_bits >>= bitsInBlock;
				}
			}
			else {
				_block[0] = raw_bits & storageMask;
				for (size_t i = 1; i < nrBlocks; ++i) {
					_block[i] = 0;
				}
			}
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		return *this;
	}

	/// <summary>
	/// 1's complement of the encoding
	/// </summary>
	/// <returns>reference to this cfloat object</returns>
	inline constexpr cfloat& flip() noexcept { // in-place one's complement
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(~_block[i]);
		}
		_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
		return *this;
	}
	/// <summary>
	/// assign the value of the string representation of a scientific number to the cfloat
	/// </summary>
	/// <param name="stringRep">decimal scientific notation of a real number to be assigned</param>
	/// <returns>reference to this cfloat</returns>
	inline cfloat& assign(const std::string& stringRep) {
		std::cout << "assign TBD\n";
		return *this;
	}

	// selectors
	inline constexpr bool sign() const noexcept { return (_block[MSU] & SIGN_BIT_MASK) == SIGN_BIT_MASK; }
	inline constexpr int  scale() const noexcept {
		int e{ 0 };
		if constexpr (MSU_CAPTURES_E) {
			e = int((_block[MSU] & ~SIGN_BIT_MASK) >> EXP_SHIFT);
			if (e == 0) {
				// subnormal scale is determined by fraction
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				e = (2l - (1l << (es - 1ull))) - 1;
				for (size_t i = nbits - 2ull - es; i > 0; --i) {
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
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				e = (2l - (1l << (es - 1ull))) - 1;
				for (size_t i = nbits - 2ull - es; i > 0; --i) {
					if (test(i)) break;
					--e;
				}
			}
			else {
				e = unsigned(ebits) - EXP_BIAS;
			}
		}
		return e;
	}
	// tests
	inline constexpr bool isneg() const noexcept { return sign(); }
	inline constexpr bool ispos() const noexcept { return !sign(); }
	inline constexpr bool iszero() const noexcept {
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
			for (size_t i = 0; i < nrBlocks-1; ++i) if (_block[i] != 0) return false;
			return (_block[MSU] & ~SIGN_BIT_MASK) == 0;
		}
	}
	inline constexpr bool isone() const noexcept {
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
	inline constexpr bool isinf(int InfType = INF_TYPE_EITHER) const noexcept {
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
			for (size_t i = 1; i < nrBlocks - 1; ++i) {
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
	/// check if a value is a quiet or a signalling NaN
	/// quiet NaN      = 0-1111-11111-1: sign = 0, uncertainty = 1, es/fraction bits = 1
	/// signalling NaN = 1-1111-11111-1: sign = 1, uncertainty = 1, es/fraction bits = 1
	/// </summary>
	/// <param name="NaNType">default is 0, both types, 1 checks for Signalling NaN, -1 checks for Quiet NaN</param>
	/// <returns>true if the right kind of NaN, false otherwise</returns>
	inline constexpr bool isnan(int NaNType = NAN_TYPE_EITHER) const noexcept {
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
		else if constexpr (4 == nrBlocks) {
			isNaN = (_block[0] == BLOCK_MASK) && (_block[1] == BLOCK_MASK) && (_block[2] == BLOCK_MASK);
		}
		else {
			for (size_t i = 0; i < nrBlocks - 1; ++i) {
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
	inline constexpr bool isnormal() const noexcept {
		blockbinary<es, bt> e;
		exponent(e);
		return !e.iszero() && !isinf() && !isnan();
	}
	inline constexpr bool isdenorm() const noexcept {
		blockbinary<es, bt> e;
		exponent(e);
		return e.iszero();
	}
	inline constexpr bool test(size_t bitIndex) const noexcept {
		return at(bitIndex);
	}
	inline constexpr bool at(size_t bitIndex) const noexcept {
		if (bitIndex < nbits) {
			bt word = _block[bitIndex / bitsInBlock];
			bt mask = bt(1ull << (bitIndex % bitsInBlock));
			return (word & mask);
		}
		return false;
	}
	inline constexpr uint8_t nibble(size_t n) const noexcept {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			int nibbleIndexInWord = int(n % (bitsInBlock >> 2ull));
			bt mask = bt(0xF << (nibbleIndexInWord * 4));
			bt nibblebits = bt(mask & word);
			return uint8_t(nibblebits >> (nibbleIndexInWord * 4));
		}
		return false;
	}
	inline constexpr bt block(size_t b) const noexcept {
		if (b < nrBlocks) {
			return _block[b];
		}
		return 0;
	}

	// helper debug function
	void constexprClassParameters() const {
		std::cout << "-------------------------------------------------------------\n";
		std::cout << "type              : " << typeid(*this).name() << '\n';
		std::cout << "nbits             : " << nbits << '\n';
		std::cout << "es                : " << es << std::endl;
		std::cout << "hasSubnormals     : " << (hasSubnormals ? "true" : "false");
		std::cout << "hasSupernormals   : " << (hasSupernormals ? "true" : "false");
		std::cout << "isSaturating      : " << (isSaturating ? "true" : "false");
		std::cout << "ALL_ONES          : " << to_binary(ALL_ONES, 0, true) << '\n';
		std::cout << "BLOCK_MASK        : " << to_binary(BLOCK_MASK, 0, true) << '\n';
		std::cout << "nrBlocks          : " << nrBlocks << '\n';
		std::cout << "bits in MSU       : " << bitsInMSU << '\n';
		std::cout << "MSU               : " << MSU << '\n';
		std::cout << "MSU MASK          : " << to_binary(MSU_MASK, 0, true) << '\n';
		std::cout << "SIGN_BIT_MASK     : " << to_binary(SIGN_BIT_MASK, 0, true) << '\n';
		std::cout << "LSB_BIT_MASK      : " << to_binary(LSB_BIT_MASK, 0, true) << '\n';
		std::cout << "MSU CAPTURES E    : " << (MSU_CAPTURES_E ? "yes\n" : "no\n");
		std::cout << "EXP_SHIFT         : " << EXP_SHIFT << '\n';
		std::cout << "MSU EXP MASK      : " << to_binary(MSU_EXP_MASK, 0, true) << '\n';
		std::cout << "EXP_BIAS          : " << EXP_BIAS << '\n';
		std::cout << "MAX_EXP           : " << MAX_EXP << '\n';
		std::cout << "MIN_EXP_NORMAL    : " << MIN_EXP_NORMAL << '\n';
		std::cout << "MIN_EXP_SUBNORMAL : " << MIN_EXP_SUBNORMAL << '\n';
		std::cout << "fraction Blocks   : " << fBlocks << '\n';
		std::cout << "bits in FSU       : " << bitsInFSU << '\n';
		std::cout << "FSU               : " << FSU << '\n';
		std::cout << "FSU MASK          : " << to_binary(FSU_MASK, 0, true) << '\n';
	}
	// extract the sign field from the encoding
	inline constexpr void sign(bool& s) const {
		s = sign();
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
				for (size_t i = 0; i < es; ++i) { e.setbit(i, at(nbits - 1ull - es + i)); }
			}
		}
	}
	// extract the fraction field from the encoding
	inline constexpr void fraction(blockbinary<fbits, bt>& f) const {
		f.clear();
		if constexpr (0 == nrBlocks) return;
		else if constexpr (1 == nrBlocks) {
			bt fraction = bt(_block[MSU] & ~MSU_EXP_MASK);
			f.setbits(fraction);
		}
		else if constexpr (nrBlocks > 1) {
			for (size_t i = 0; i < fbits; ++i) { f.setbit(i, at(i)); } // TODO: TEST!
		}
	}
	inline constexpr uint64_t fraction_ull() const {
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
				raw = fbitMask & ((uint64_t(_block[2]) << (2*bitsInBlock)) | (uint64_t(_block[1]) << bitsInBlock) | uint64_t(_block[0]));
			}
			else if constexpr (4 == nrBlocks) {
				uint64_t fbitMask = 0xFFFF'FFFF'FFFF'FFFF >> (64 - fbits);
				raw = fbitMask & ((uint64_t(_block[3]) << (3 * bitsInBlock)) | (uint64_t(_block[2]) << (2 * bitsInBlock)) | (uint64_t(_block[1]) << bitsInBlock) | uint64_t(_block[0]));
			}
			else {
				uint64_t mask{ 1 };
				for (size_t i = 0; i < fbits; ++i) { 
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
	inline constexpr size_t significant(blockbinary<fhbits, bt>& s, bool isNormal = true) const {
		size_t shift = 0;
		if (iszero()) return 0;
		if constexpr (0 == nrBlocks) return 0;
		else if constexpr (1 == nrBlocks) {
			bt significant = bt(_block[MSU] & ~MSU_EXP_MASK & ~SIGN_BIT_MASK);
			if (isNormal) {
				significant |= (bt(0x1ul) << fbits);
			}
			else {
				size_t msb = findMostSignificantBit(significant);
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
				for (size_t i = 0; i < fbits; ++i) { s.setbit(i, at(i)); }
			}
			else {
				// Find the MSB of the subnormal: 
				size_t msb = 0;
				for (size_t i = 0; i < fbits; ++i) { // msb protected from not being assigned through iszero test at prelude of function
					msb = fbits - 1ull - i;
					if (test(msb)) break;
				}
				//      m-----lsb
				// h00001010101
				// 101010100000
				for (size_t i = 0; i <= msb; ++i) {
					s.setbit(fbits - msb + i, at(i));
				}
				shift = fhbits - msb;
			}
		}
		return shift;
	}
	// get the raw bits from the encoding
	inline constexpr void getbits(blockbinary<nbits, bt>& b) const {
		b.clear();
		for (size_t i = 0; i < nbits; ++i) { b.setbit(i, at(i)); }
	}
	// casts to native types
	long to_long() const { return long(to_native<double>()); }
	long long to_long_long() const { return (long long)(to_native<double>()); }
	// transform an cfloat to a native C++ floating-point. We are using the native
	// precision to compute, which means that all sub-values need to be representable 
	// by the native precision.
	// A more accurate appromation would require an adaptive precision algorithm
	// with a final rounding step.
	template<typename TargetFloat>
	TargetFloat to_native() const { 
		TargetFloat v{ 0 };
		if (iszero()) {
			if (sign()) { // the optimizer might destroy the sign
				return -TargetFloat(0);
			}
			else {
				return TargetFloat(0);
			}
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
			for (int i = static_cast<int>(nbits - 2ull - es); i >= 0; --i) {
				f += at(static_cast<size_t>(i)) ? fbit : TargetFloat(0);
				fbit *= TargetFloat(0.5);
			}
			blockbinary<es, bt> ebits;
			exponent(ebits);
			if (ebits.iszero()) {
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				TargetFloat exponentiation = TargetFloat(subnormal_exponent[es]); // precomputed values for 2^(2-2^(es-1))
				v = exponentiation * f;
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
			v = sign() ? -v : v;
		}
		return v;
	}

	// make conversions to native types explicit
	explicit operator int() const { return to_long_long(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator long double() const { return to_native<long double>(); }
	explicit operator double() const { return to_native<double>(); }
	explicit operator float() const { return to_native<float>(); }

#ifdef NEVER
	// normalize a non-special cfloat, that is, not a zero, inf, or nan, into a blocktriple
	template<size_t tgtSize>
	void generate_add_input(blocktriple<tgtSize, bt>& v) const {
		bool _sign = sign();
		int  _scale = scale();
		// fraction bits are the bottom fbits in the raw encoding
		// normal    encoding : 1.fffff
		// subnormal encoding : 0.fffff
	}
#endif

	// convert a cfloat to a blocktriple with the fraction format 01.ffffeeee
	// we are using the same block type so that we can use block copies to move bits around.
	// Since we tend to have at least two exponent bits, this will lead to
	// most cfloat<->blocktriple cases being efficient as the block types are aligned.
	// The relationship between the source cfloat and target blocktriple is not
	// arbitrary, enforce it: blocktriple fbits = cfloat (nbits - es - 1)
	constexpr void normalize(blocktriple<fbits, bt>& tgt) const {
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
					// brute force copy of blocks
					if constexpr (1 == fBlocks) {
						tgt.setblock(0, _block[0] & FSU_MASK);
					}
					else if constexpr (2 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1] & FSU_MASK);
					}
					else if constexpr (3 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1]);
						tgt.setblock(2, _block[2] & FSU_MASK);
					}
					else if constexpr (4 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1]);
						tgt.setblock(2, _block[2]);
						tgt.setblock(3, _block[3] & FSU_MASK);
					}
					else {
						for (size_t i = 0; i < FSU; ++i) {
							tgt.setblock(i, _block[i]);
						}
						tgt.setblock(FSU, _block[FSU] & FSU_MASK);
					}
				}
			}
			else { // it is a subnormal encoding in this target cfloat
				if constexpr (fbits < 64) {
					uint64_t raw = fraction_ull();
					int shift = MIN_EXP_NORMAL - scale;
					raw <<= shift;
					raw |= (1ull << fbits);
					tgt.setbits(raw);
				}
				else {
					// brute force copy of blocks
					if constexpr (1 == fBlocks) {
						tgt.setblock(0, _block[0] & FSU_MASK);
					}
					else if constexpr (2 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1] & FSU_MASK);
					}
					else if constexpr (3 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1]);
						tgt.setblock(2, _block[2] & FSU_MASK);
					}
					else if constexpr (4 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1]);
						tgt.setblock(2, _block[2]);
						tgt.setblock(3, _block[3] & FSU_MASK);
					}
					else {
						for (size_t i = 0; i < FSU; ++i) {
							tgt.setblock(i, _block[i]);
						}
						tgt.setblock(FSU, _block[FSU] & FSU_MASK);
					}
				}
			}
		}
	}

	// normalize a cfloat to a blocktriple used in add/sub
	constexpr void normalizeAddition(blocktriple<abits, bt>& tgt) const {
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
				if constexpr (abits < 64) { // max 63 bits of fraction to yield 64bit of raw significant bits
					uint64_t raw = fraction_ull();
					raw <<= (abits - fbits);
					raw |= (1ull << abits); // add the hidden bit
					tgt.setbits(raw);
				}
				else {
					// brute force copy of blocks
					if constexpr (1 == fBlocks) {
						tgt.setblock(0, _block[0] & FSU_MASK);
					}
					else if constexpr (2 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1] & FSU_MASK);
					}
					else if constexpr (3 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1]);
						tgt.setblock(2, _block[2] & FSU_MASK);
					}
					else if constexpr (4 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1]);
						tgt.setblock(2, _block[2]);
						tgt.setblock(3, _block[3] & FSU_MASK);
					}
					else {
						for (size_t i = 0; i < FSU; ++i) {
							tgt.setblock(i, _block[i]);
						}
						tgt.setblock(FSU, _block[FSU] & FSU_MASK);
					}
				}
			}
			else { // it is a subnormal encoding in this target cfloat
				if constexpr (abits < 64) {
					uint64_t raw = fraction_ull();
					raw <<= (abits - fbits);
					int shift = MIN_EXP_NORMAL - scale;
					raw <<= shift; // shift and do NOT add a hidden bit as MSB of subnormal is shifted in the hidden bit position
					tgt.setbits(raw);
				}
				else {
					// brute force copy of blocks
					if constexpr (1 == fBlocks) {
						tgt.setblock(0, _block[0] & FSU_MASK);
					}
					else if constexpr (2 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1] & FSU_MASK);
					}
					else if constexpr (3 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1]);
						tgt.setblock(2, _block[2] & FSU_MASK);
					}
					else if constexpr (4 == fBlocks) {
						tgt.setblock(0, _block[0]);
						tgt.setblock(1, _block[1]);
						tgt.setblock(2, _block[2]);
						tgt.setblock(3, _block[3] & FSU_MASK);
					}
					else {
						for (size_t i = 0; i < FSU; ++i) {
							tgt.setblock(i, _block[i]);
						}
						tgt.setblock(FSU, _block[FSU] & FSU_MASK);
					}
				}
			}
		}
	}


protected:
	// HELPER methods

	template<typename Ty>
	constexpr cfloat& convert_unsigned_integer(const Ty& rhs) noexcept {
		clear();
		if (0 == rhs) return *this;
		uint64_t raw = static_cast<uint64_t>(rhs);
		int exponent = int(findMostSignificantBit(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr uint32_t sizeInBits = 8 * sizeof(Ty);
		uint32_t shift = sizeInBits - exponent - 1;
		raw <<= shift;
		raw = round<sizeInBits, uint64_t>(raw, exponent);
		return *this;
	}
	template<typename Ty>
	constexpr cfloat& convert_signed_integer(const Ty& rhs) noexcept {
		clear();
		if (0 == rhs) return *this;
		bool s = (rhs < 0);
		uint64_t raw = static_cast<uint64_t>(s ? -rhs : rhs);
		int exponent = int(findMostSignificantBit(raw)) - 1; // precondition that msb > 0 is satisfied by the zero test above
		constexpr uint32_t sizeInBits = 8 * sizeof(Ty);
		uint32_t shift = sizeInBits - exponent - 1;
		raw <<= shift;
		raw = round<sizeInBits, uint64_t>(raw, exponent);
#ifdef TODO
		// construct the target cfloat
		if constexpr (64 >= nbits - es - 1ull) {
			uint64_t bits = (s ? 1u : 0u);
			bits <<= es;
			bits |= exponent + EXP_BIAS;
			bits <<= nbits - 1ull - es;
			bits |= raw;
			bits &= 0xFFFF'FFFF;
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

public:
	template<typename Real>
	CONSTEXPRESSION cfloat& convert_ieee754(Real rhs) noexcept {
		// when we are a perfect match to single precision IEEE-754
		if constexpr (nbits == 32 && es == 8) {
			bool s{ false };
			uint64_t rawExponent{ 0 };
			uint64_t rawFraction{ 0 };
			// use native conversion
			extractFields(float(rhs), s, rawExponent, rawFraction);
			uint64_t raw{ s ? 1ull : 0ull };
			raw <<= 31;
			raw |= (rawExponent << fbits);
			raw |= rawFraction;
			setbits(raw);
			return *this;
		}
		// when we are a perfect match to double precision IEEE-754
		else if constexpr (nbits == 64 && es == 11) {
			bool s{ false };
			uint64_t rawExponent{ 0 };
			uint64_t rawFraction{ 0 };
			// use native conversion
			extractFields(double(rhs), s, rawExponent, rawFraction);
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
			extractFields(rhs, s, rawExponent, rawFraction);

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
					return *this;
				}
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask)) {
					// 1.11111111.10000000.......00000000 quiet nan
					// 0.11111111.10000000.......00000000 quiet nan
					setnan(NAN_TYPE_QUIET);
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

			// check special case of saturating to maxpos/maxneg if out of range
			if (exponent > MAX_EXP) {
				if (s) this->maxneg(); else this->maxpos(); // saturate to maxpos or maxneg
				return *this;
			}
			if (exponent < MIN_EXP_SUBNORMAL - 1) { // TODO: explain the MIN_EXP_SUBMORNAL - 1
				this->setbit(nbits - 1, s);
				return *this;
			}
			/////////////////  
			/// end of special case processing, move on to value projection and rounding

#if TRACE_CONVERSION
			std::cout << '\n';
			std::cout << "value             : " << rhs << '\n';
			std::cout << "segments          : " << to_binary(rhs) << '\n';
			std::cout << "sign     bit      : " << (s ? '1' : '0') << '\n';
			std::cout << "exponent bits     : " << to_binary(rawExponent, ieee754_parameter<Real>::ebits, true) << '\n';
			std::cout << "fraction bits     : " << to_binary(rawFraction, ieee754_parameter<Real>::fbits, true) << std::endl;
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

			if constexpr (ieee754_parameter<Real>::fbits > fbits) {
				// this is the common case for cfloats that are smaller than single and double precision IEEE-754
				constexpr int shiftRight = ieee754_parameter<Real>::fbits - fbits; // this is the bit shift to get the MSB of the src to the MSB of the tgt
				uint32_t biasedExponent{ 0 };
				int adjustment{ 0 };
				uint64_t mask;
				if (rawExponent != 0) {
					// the source real is a normal number, 
					if (exponent >= (MIN_EXP_SUBNORMAL - 1) && exponent < MIN_EXP_NORMAL) {
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
					if constexpr (shiftRight > 0) {		// if true we need to round
						// round-to-even logic
						//  ... lsb | guard  round sticky   round
						//       x     0       x     x       down
						//       0     1       0     0       down  round to even
						//       1     1       0     0        up   round to even
						//       x     1       0     1        up
						//       x     1       1     0        up
						//       x     1       1     1        up
						// collect lsb, guard, round, and sticky bits
						mask = (1ull << (shiftRight + adjustment)); // bit mask for the lsb bit
						bool lsb = (mask & rawFraction);
						mask >>= 1;
						bool guard = (mask & rawFraction);
						mask >>= 1;
						bool round = (mask & rawFraction);
						if constexpr (shiftRight > 1) {
							mask = (0xFFFF'FFFF'FFFF'FFFFull << (shiftRight - 2));
							mask = ~mask;
						}
						else {
							mask = 0;
						}
						bool sticky = (mask & rawFraction);
						rawFraction >>= shiftRight + adjustment;

						// execute rounding operation
						if (guard) {
							if (lsb && (!round && !sticky)) ++rawFraction; // round to even
							if (round || sticky) ++rawFraction;
							if (rawFraction == (1ul << fbits)) { // overflow
								++biasedExponent;
								rawFraction = 0;
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
					else { // all bits of the float go into this representation and need to be shifted up
						int shiftLeft = fbits - ieee754_parameter<Real>::fbits;
						rawFraction <<= shiftLeft;
					}
#if TRACE_CONVERSION
					std::cout << "biased exponent   : " << biasedExponent << " : 0x" << std::hex << biasedExponent << std::dec << '\n';
					std::cout << "shift             : " << shiftRight << '\n';
					std::cout << "adjustment shift  : " << adjustment << '\n';
					std::cout << "sticky bit mask   : " << to_binary(mask, 32, true) << '\n';
					std::cout << "fraction bits     : " << to_binary(rawFraction, 32, true) << '\n';
#endif
					// construct the target cfloat
					uint64_t bits = (s ? 1ull : 0ull);
					bits <<= es;
					bits |= biasedExponent;
					bits <<= fbits;
					bits |= rawFraction;
					setbits(bits);
				}
				else {
					// the source real is a subnormal number				
					mask = 0x00FF'FFFFu >> (fbits + exponent + subnormal_reciprocal_shift[es] + 1); // mask for sticky bit 

					// fraction processing: we have fbits+1 bits = 1 hidden + fbits explicit fraction bits 
					// f = 1.ffff  2^exponent * 2^fbits * 2^-(2-2^(es-1)) = 1.ff...ff >> (23 - (-exponent + fbits - (2 -2^(es-1))))
					// -exponent because we are right shifting and exponent in this range is negative
					adjustment = -(exponent + subnormal_reciprocal_shift[es]); // this is the right shift adjustment due to the scale of the input number, i.e. the exponent of 2^-adjustment

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
					// nbits = 40, es = 8, fbits = 31: rhs = float fbits = 23; shift left by (31 - 23) = 8

					if (rawExponent != 0) {
						// rhs is a normal encoding
						uint64_t bits{ s ? 1ull : 0ull };
						bits <<= es;
						bits |= biasedExponent;
						bits <<= fbits;
						rawFraction <<= upshift;
						bits |= rawFraction;
						setbits(bits);
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
					if (rawExponent != 0) {
						// nbits = 128, es = 15, fbits = 112: rhs = float: shift left by (112 - 23) = 89
						setbits(biasedExponent);
						shiftLeft(fbits);
						bt fractionBlock[nrBlocks]{ 0 };
						// copy fraction bits
						size_t blocksRequired = (8 * sizeof(rawFraction) + 1) / bitsInBlock;
						size_t maxBlockNr = (blocksRequired < nrBlocks ? blocksRequired : nrBlocks);
						uint64_t mask = static_cast<uint64_t>(ALL_ONES); // set up the block mask
						size_t shift = 0;
						for (size_t i = 0; i < maxBlockNr; ++i) {
							fractionBlock[i] = bt((mask & rawFraction) >> shift);
							mask <<= bitsInBlock;
							shift += bitsInBlock;
						}
						// shift fraction bits
						int bitsToShift = upshift;
						if (bitsToShift >= int(bitsInBlock)) {
							int blockShift = bitsToShift / bitsInBlock;
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
							bt mask = ALL_ONES << (bitsInBlock - bitsToShift);
							for (size_t i = MSU; i > 0; --i) {
								fractionBlock[i] <<= bitsToShift;
								// mix in the bits from the right
								bt bits = (mask & fractionBlock[i - 1]);
								fractionBlock[i] |= (bits >> (bitsInBlock - bitsToShift));
							}
							fractionBlock[0] <<= bitsToShift;
						}
						// OR the bits in
						for (size_t i = 0; i < MSU; ++i) {
							_block[i] |= fractionBlock[i];
						}
						// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
						_block[MSU] &= MSU_MASK;
						// finally, set the sign bit
						setsign(s);
					}
					else { // rhs is a subnormal
	//					std::cerr << "rhs is a subnormal : " << to_binary(rhs) << " : " << rhs << '\n';
					}
				}
			}

			// post-processing results to implement saturation
			if (this->isinf(INF_TYPE_POSITIVE) || this->isnan(NAN_TYPE_QUIET)) {
				clear();
				flip();
				setbit(nbits - 1ull, false);
				setbit(1ull, false);
			}
			else if (this->isinf(INF_TYPE_NEGATIVE) || this->isnan(NAN_TYPE_SIGNALLING)) {
				clear();
				flip();
				setbit(1ull, false);
			}
		}
		return *this;  // TODO: unreachable in some configurations
	}

protected:

	/// <summary>
	/// round a set of source bits to the present representation.
	/// srcbits is the number of bits of significant in the source representation
	/// </summary>
	/// <typeparam name="StorageType"></typeparam>
	/// <param name="raw"></param>
	/// <returns></returns>
	template<size_t srcbits, typename StorageType>
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
				if (raw == (1ull << nbits)) { // overflow
					++exponent;
					raw >>= 1u;
				}
			}
		}
		else {
			constexpr size_t shift = fhbits - srcbits;
			if constexpr (shift < (sizeof(StorageType))) {
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
		size_t blocksRequired = (8 * sizeof(v) + 1 ) / bitsInBlock;
		size_t maxBlockNr = (blocksRequired < nrBlocks ? blocksRequired : nrBlocks);
		bt b{ 0ul }; b = bt(~b);
		ArgumentBlockType mask = ArgumentBlockType(b);
		size_t shift = 0;
		for (size_t i = 0; i < maxBlockNr; ++i) {
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
		size_t blockShift = 0;
		if (bitsToShift >= long(bitsInBlock)) {
			blockShift = bitsToShift / bitsInBlock;
			if (MSU >= blockShift) {
				// shift by blocks
				for (size_t i = 0; i <= MSU - blockShift; ++i) {
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
					for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
						this->setbit(i);
					}
				}
				else {
					// clean up the blocks we have shifted clean
					bitsToShift += (long)(blockShift * bitsInBlock);
					for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
						this->setbit(i, false);
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
			for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
				this->setbit(i);
			}
		}
		else {
			// clean up the blocks we have shifted clean
			bitsToShift += (long)(blockShift * bitsInBlock);
			for (size_t i = nbits - bitsToShift; i < nbits; ++i) {
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

private:
	bt _block[nrBlocks];

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend std::ostream& operator<< (std::ostream& ostr, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& r);
	template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend std::istream& operator>> (std::istream& istr, cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& r);

	template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator==(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator!=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator< (const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator> (const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator<=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
	template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
	friend bool operator>=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs);
};

////////////////////// operators
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::ostream& operator<<(std::ostream& ostr, const cfloat<nbits,es,bt,hasSubnormals,hasSupernormals,isSaturating>& v) {
	// TODO: make it a native conversion
	ostr << double(v);
	return ostr;
}

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::istream& operator>>(std::istream& istr, const cfloat<nbits,es,bt,hasSubnormals,hasSupernormals,isSaturating>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator==(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) {
	for (size_t i = 0; i < lhs.nrBlocks; ++i) {
		if (lhs._block[i] != rhs._block[i]) {
			return false;
		}
	}
	return true;
}
template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator!=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator< (const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) { return (lhs - rhs).isneg(); }
template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator> (const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator<=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t nes, typename nbt, bool nsub, bool nsup, bool nsat>
inline bool operator>=(const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& lhs, const cfloat<nnbits,nes,nbt,nsub,nsup,nsat>& rhs) { return !operator< (lhs, rhs); }

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt> operator+(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt> operator-(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt> operator*(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline cfloat<nbits, es, bt> operator/(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& rhs) {
	cfloat<nbits, es, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

// convert to std::string
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::string to_string(const cfloat<nbits,es,bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
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

// transform cfloat to a binary representation
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::string to_binary(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << 'b';
	size_t index = nbits;
	s << (number.at(--index) ? '1' : '0') << '.';

	for (int i = int(es)-1; i >= 0; --i) {
		s << (number.at(--index) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}

	s << '.';

	constexpr int fbits = nbits - 1ull - es;
	for (int i = fbits-1; i >= 0; --i) {
		s << (number.at(--index) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}

	return s.str();
}

// transform a cfloat into a triple representation
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::string to_triple(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& number, bool nibbleMarker = true) {
	std::stringstream s;
	blocktriple<cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits, bt> triple;
	number.normalize(triple);
	s << to_triple(triple, nibbleMarker);
	return s.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> 
abs(const cfloat<nbits,es,bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
	return cfloat<nbits,es,bt, hasSubnormals, hasSupernormals, isSaturating>(false, v.scale(), v.fraction(), v.isZero());
}

///////////////////////////////////////////////////////////////////////
///   binary logic literal comparisons

// cfloat - literal float logic operators
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) == rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) != rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) < rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) > rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) <= rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, float rhs) {
	return float(lhs) >= rhs;
}
// cfloat - literal double logic operators
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) == rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) != rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) < rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) > rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) <= rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, double rhs) {
	return double(lhs) >= rhs;
}

// cfloat - literal long double logic operators
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) == rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) != rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) < rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) > rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) <= rhs;
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long double rhs) {
	return (long double)(lhs) >= rhs;
}

// cfloat - literal int logic operators
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator==(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator!=(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator<(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs), lhs);
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs)) || operator==(lhs, cfloat<nbits, es, bt>(rhs));
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, int rhs) {
	return !operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}

// cfloat - long long logic operators
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator==(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator==(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator!=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator!=(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator< (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator> (const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator<(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs), lhs);
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator<=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs)) || operator==(lhs, cfloat<nbits, es, bt>(rhs));
}
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline bool operator>=(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& lhs, long long rhs) {
	return !operator<(lhs, cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(rhs));
}

}  // namespace sw::universal
