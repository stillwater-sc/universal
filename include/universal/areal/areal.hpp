#pragma once
// areal.hpp: definition of an arbitrary configuration linear floating-point representation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/blockbin/blockbinary.hpp>
#include <universal/areal/exceptions.hpp>

// compiler specific operators
#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
//#pragma warning(disable : 4310)  // cast truncates constant value

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

#define THROW_ARITHMETIC_EXCEPTION 0

namespace sw::universal {
		
	static constexpr double oneOver2p6 = 0.015625;
	static constexpr double oneOver2p14 = 0.00006103515625;
	static constexpr double oneOver2p30 = 1.0 / 1073741824.0;
	static constexpr double oneOver2p50 = 1.0 / 1125899906842624.0;
	static constexpr double oneOver2p62 = 1.0 / 4611686018427387904.0;
	static constexpr double oneOver2p126 = oneOver2p62 * oneOver2p62 * 0.25;
	static constexpr double oneOver2p254 = oneOver2p126 * oneOver2p126 * 0.25;
	static constexpr double oneOver2p510 = oneOver2p254 * oneOver2p254 * 0.25;
	static constexpr double oneOver2p1022 = oneOver2p510 * oneOver2p510 * 0.25;

// precomputed values for subnormal exponents as a function of es
// es > 11 requires a long double representation, which MSVC does not provide.
	static constexpr double subnormal_exponent[] = {
		0,                    // es = 0 : not a valid value
		2.0,                  // es = 1 : 2^(2 - 2^(es-1)) = 2^1
		1.0,                  // es = 2 : 2^(2 - 2^(es-1)) = 2^0
		0.25,                 // es = 3 : 2^(2 - 2^(es-1)) = 2^-2
		oneOver2p6,           // es = 4 : 2^(2 - 2^(es-1)) = 2^-6
		oneOver2p14,          // es = 5 : 2^(2 - 2^(es-1)) = 2^-14
		oneOver2p30,          // es = 6 : 2^(2 - 2^(es-1)) = 2^-30
		oneOver2p62,          // es = 7 : 2^(2 - 2^(es-1)) = 2^-62
		oneOver2p126,         // es = 8 : 2^(2 - 2^(es-1)) = 2^-126
		oneOver2p254,         // es = 9 : 2^(2 - 2^(es-1)) = 2^-254
		oneOver2p510,         // es = 10 : 2^(2 - 2^(es-1)) = 2^-510
		oneOver2p1022         // es = 11 : 2^(2 - 2^(es-1)) = 2^-1022
	};

// Forward definitions
template<size_t nbits, size_t es, typename bt> class areal;
template<size_t nbits, size_t es, typename bt> areal<nbits,es,bt> abs(const areal<nbits,es,bt>&);
template<typename bt> inline std::string to_binary(const bt&, bool);

static constexpr int NAN_TYPE_SIGNALLING = -1;   // a Signalling NaN
static constexpr int NAN_TYPE_EITHER     = 0;    // any NaN
static constexpr int NAN_TYPE_QUIET      = 1;    // a Quiet NaN

static constexpr int INF_TYPE_NEGATIVE   = -1;   // -inf
static constexpr int INF_TYPE_EITHER     = 0;    // any inf
static constexpr int INF_TYPE_POSITIVE   = 1;    // +inf

constexpr bool AREAL_NIBBLE_MARKER = true;


/// <summary>
/// decode an areal value into its constituent parts
/// </summary>
/// <typeparam name="bt"></typeparam>
/// <param name="v"></param>
/// <param name="s"></param>
/// <param name="e"></param>
/// <param name="f"></param>
/// <param name="ubit"></param>
template<size_t nbits, size_t es, size_t fbits, typename bt>
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
template<size_t nbits, size_t es, typename bt>
int scale(const areal<nbits, es, bt>& v) {
	return v.scale();
}

/////////////////////////////////////////////////////////////////////////////////
/// free functions that can set an areal to extreme values in its state space
/// organized in descending order.

// fill an areal object with maximum positive value
template<size_t nbits, size_t es, typename bt>
areal<nbits, es, bt>& maxpos(areal<nbits, es, bt>& amaxpos) {

	return amaxpos;
}
// fill an areal object with mininum positive value
template<size_t nbits, size_t es, typename bt>
areal<nbits, es, bt>& minpos(areal<nbits, es, bt>& aminpos) {

	return aminpos;
}
// fill an areal object with zero
template<size_t nbits, size_t es, typename bt>
areal<nbits, es, bt>& zero(areal<nbits, es, bt>& tobezero) {
	tobezero.clear();
	return tobezero;
}
// fill an areal object with smallest negative value
template<size_t nbits, size_t es, typename bt>
areal<nbits, es, bt>& minneg(areal<nbits, es, bt>& aminneg) {

	return aminneg;
}
// fill an areal object with largest negative value
template<size_t nbits, size_t es, typename bt>
areal<nbits, es, bt>& maxneg(areal<nbits, es, bt>& amaxneg) {

	return amaxneg;
}

/// <summary>
/// An arbitrary configuration real number with gradual under/overflow and uncertainty bit
/// </summary>
/// <typeparam name="nbits">number of bits in the encoding</typeparam>
/// <typeparam name="es">number of exponent bits in the encoding</typeparam>
/// <typeparam name="bt">the type to use as storage class: one of [uint8_t|uint16_t|uint32_t]</typeparam>
template<size_t _nbits, size_t _es, typename bt = uint8_t>
class areal {
public:
	static_assert(_nbits > _es + 2ull, "nbits is too small to accomodate requested exponent bits");
	static_assert(_es < 2147483647ull, "that is too big a number, are you trying to break the Interweb?");
	static_assert(_es > 0, "number of exponent bits must be bigger than 0");
	static constexpr size_t bitsInByte = 8ull;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static_assert(bitsInBlock <= 64, "storage unit for block arithmetic needs to be <= uint32_t");

	static constexpr size_t nbits = _nbits;
	static constexpr size_t es = _es;
	using BlockType = bt;

	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr size_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64ull - bitsInBlock));

	static constexpr size_t MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken
	static constexpr bt MSU_MASK = (bt(-1) >> (nrBlocks * bitsInBlock - nbits));
	static constexpr size_t bitsInMSU = bitsInBlock - (nrBlocks * bitsInBlock - nbits);
	static constexpr bt SIGN_BIT_MASK = bt(bt(1ull) << ((nbits - 1ull) % bitsInBlock));
	static constexpr bt LSB_BIT_MASK = bt(1ull);
	static constexpr bool MSU_CAPTURES_E = (nbits - 1ull - es) < bitsInMSU;
	static constexpr size_t EXP_SHIFT = (MSU_CAPTURES_E ? (nbits - 1ull - es) : 0);
	static constexpr bt MSU_EXP_MASK = ((bt(-1) << EXP_SHIFT) & ~SIGN_BIT_MASK) & MSU_MASK;
	static constexpr int EXP_BIAS = ((1l << (es - 1ull)) - 1l);
	static constexpr bt BLOCK_MASK = bt(-1);

	static constexpr size_t fbits  = nbits - 2ull - es;    // number of fraction bits excluding the hidden bit
	static constexpr size_t fhbits = fbits + 1ull;         // number of fraction bits including the hidden bit
	static constexpr size_t abits = fhbits + 3ull;         // size of the addend
	static constexpr size_t mbits = 2ull * fhbits;         // size of the multiplier output
	static constexpr size_t divbits = 3ull * fhbits + 4ull;// size of the divider output

	// constructors
	areal() noexcept : _block{ 0 } {};

	// decorated/converting constructors

	/// <summary>
	/// construct an areal from another, block type bt must be the same
	/// </summary>
	/// <param name="rhs"></param>
	template<size_t nnbits, size_t ees>
	areal(const areal<nnbits, ees, bt>& rhs) {
		// this->assign(rhs);
	}

	/// <summary>
	/// construct an areal from a native type, specialized for size
	/// </summary>
	/// <param name="initial_value"></param>
	areal(signed char initial_value)        { *this = initial_value; }
	areal(short initial_value)              { *this = initial_value; }
	areal(int initial_value)                { *this = initial_value; }
	areal(long long initial_value)          { *this = initial_value; }
	areal(unsigned long long initial_value) { *this = initial_value; }
	areal(float initial_value)              { *this = initial_value; }
	areal(double initial_value)             { *this = initial_value; }
	areal(long double initial_value)        { *this = initial_value; }
	areal(const areal& rhs)                 { *this = rhs; }

	// assignment operators
	areal& operator=(signed char rhs) {
		return *this = (long long)(rhs);
	}
	areal& operator=(short rhs) {
		return *this = (long long)(rhs);
	}
	areal& operator=(int rhs) {
		return *this = (long long)(rhs);
	}
	areal& operator=(long long rhs) {
		return *this;
	}
	areal& operator=(unsigned long long rhs) {
		return *this;
	}
	areal& operator=(float rhs) {

		return *this;
	}
	areal& operator=(double rhs) {

		return *this;
	}
	areal& operator=(long double rhs) {

		return *this;
	}

	// arithmetic operators
	// prefix operator
	inline areal operator-() const {
		areal tmp(*this);
		tmp._block[MSU] ^= SIGN_BIT_MASK;
		return tmp;
	}

	areal& operator+=(const areal& rhs) {
		return *this;
	}
	areal& operator+=(double rhs) {
		return *this += areal(rhs);
	}
	areal& operator-=(const areal& rhs) {

		return *this;
	}
	areal& operator-=(double rhs) {
		return *this -= areal<nbits, es>(rhs);
	}
	areal& operator*=(const areal& rhs) {

		return *this;
	}
	areal& operator*=(double rhs) {
		return *this *= areal<nbits, es>(rhs);
	}
	areal& operator/=(const areal& rhs) {

		return *this;
	}
	areal& operator/=(double rhs) {
		return *this /= areal<nbits, es>(rhs);
	}
	inline areal& operator++() {
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
		switch (nrBlocks) {
		case 0:
			return;
		case 1:
			_block[MSU] = sign ? bt(MSU_MASK ^ LSB_BIT_MASK) : bt(~SIGN_BIT_MASK & (MSU_MASK ^ LSB_BIT_MASK));
			break;
		case 2:
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
			break;
		case 3:
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			_block[1] = BLOCK_MASK;
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
			break;
		default:
			_block[0] = BLOCK_MASK ^ LSB_BIT_MASK;
			for (size_t i = 1; i < nrBlocks - 1; ++i) {
				_block[i] = BLOCK_MASK;
			}
			_block[MSU] = sign ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
			break;
		}
		
	}
	/// <summary>
	/// set the number to a quiet NaN (+nan) or a signalling NaN (-nan, default)
	/// </summary>
	/// <param name="sign">boolean to make it + or - infinity, default is -inf</param>
	/// <returns>void</returns> 
	inline constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING) noexcept {
		switch (nrBlocks) {
		case 0:
			return;
		case 1:
			break;
		case 2:
			_block[0] = BLOCK_MASK;
			break;
		case 3:
			_block[0] = BLOCK_MASK;
			_block[1] = BLOCK_MASK;
			break;
		default:
			for (size_t i = 0; i < nrBlocks - 1; ++i) {
				_block[i] = BLOCK_MASK;
			}
			break;
		}
		_block[MSU] = NaNType == NAN_TYPE_SIGNALLING ? MSU_MASK : bt(~SIGN_BIT_MASK & MSU_MASK);
	}
	/// <summary>
	/// set the raw bits of the areal. This is a required function in the Universal number systems
	/// that enables verification test suites to inject specific bit patterns using a common interface.
	/// </summary>
	/// <param name="raw_bits">unsigned long long carrying bits that will be written verbatim to the areal</param>
	/// <returns>reference to the areal</returns>
	inline areal& set_raw_bits(uint64_t raw_bits) noexcept {
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = raw_bits & storageMask;
			raw_bits >>= bitsInBlock; // shift can be the same size as type as it is protected by loop constraints
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
		return *this;
	}
	inline areal& assign(const std::string& stringRep) {
		std::cout << "assign TBD\n";
		return *this;
	}

	// selectors
	inline constexpr bool sign() const { return (_block[MSU] & SIGN_BIT_MASK) == SIGN_BIT_MASK; }
	inline int scale() const {
		int e{ 0 };
		// make if constexpr
		if (MSU_CAPTURES_E) {
			e = int((_block[MSU] & ~SIGN_BIT_MASK) >> EXP_SHIFT);
			if (e == 0) {
				// subnormal scale is determined by fraction
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				;
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
				e = -1;
				for (size_t i = nbits - 2ull - es; i > 0; --i) {
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
	inline bool iszero() const { // TODO: need to deal with -0 as well
		switch (nrBlocks) {
		case 0:
			return true;
		case 1:
			return (_block[MSU] & ~SIGN_BIT_MASK) == 0 ? true : false;
		case 2:
			return (_block[0] == 0) && (_block[MSU] & ~SIGN_BIT_MASK) == 0 ? true : false;
			break;
		case 3:
			return (_block[0] == 0) && _block[1] == 0 && (_block[MSU] & ~SIGN_BIT_MASK) == 0 ? true : false;
			break;
		default:
			for (size_t i = 0; i < nrBlocks-1; ++i) if (_block[i] != 0) return false;
			return (_block[MSU] & ~SIGN_BIT_MASK) == 0 ? true : false;
		}
	}
	/// <summary>
	/// check if value is infinite, -inf, or +inf. 
	/// +inf = 0-1111-11111-0: sign = 0, uncertainty = 0, es/fraction bits = 1
	/// -inf = 1-1111-11111-0: sign = 1, uncertainty = 0, es/fraction bits = 1
	/// </summary>
	/// <param name="InfType">default is 0, both types, -1 checks for -inf, 1 checks for +inf</param>
	/// <returns>true if +-inf, false otherwise</returns>
	inline bool isinf(int InfType = INF_TYPE_EITHER) const {
		bool isInf = false;
		bool isNegInf = false;
		bool isPosInf = false;
		switch (nrBlocks) {
		case 0:
			return false;
		case 1:
			isNegInf = (_block[MSU] & MSU_MASK) == (MSU_MASK ^ LSB_BIT_MASK);
			isPosInf = (_block[MSU] & MSU_MASK) == ((MSU_MASK ^ SIGN_BIT_MASK) ^ LSB_BIT_MASK);
			return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
					(InfType == INF_TYPE_NEGATIVE ?	isNegInf :
					  (InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
		case 2:
			isInf = (_block[0] == (BLOCK_MASK ^ LSB_BIT_MASK));
			break;
		case 3:
			isInf = (_block[0] == (BLOCK_MASK ^ LSB_BIT_MASK)) && 
				    (_block[1] == BLOCK_MASK);
			break;
		default:
			isInf = (_block[0] == (BLOCK_MASK ^ LSB_BIT_MASK));
			for (size_t i = 1; i < nrBlocks - 1; ++i) {
				if (_block[i] != BLOCK_MASK) {
					isInf = false;
					break;
				}
			}
			break;
		}
		isNegInf = isInf && ((_block[MSU] & MSU_MASK) == MSU_MASK);
		isPosInf = isInf && (_block[MSU] & MSU_MASK) == (MSU_MASK ^ SIGN_BIT_MASK);
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
	inline bool isnan(int NaNType = NAN_TYPE_EITHER) const {
		bool isNaN = true;
		switch (nrBlocks) {
		case 0:
			return false;
		case 1:
			break;
		case 2:
			isNaN = (_block[0] == BLOCK_MASK);
			break;
		case 3:
			isNaN = (_block[0] == BLOCK_MASK) && (_block[1] == BLOCK_MASK);
			break;
		default:
			for (size_t i = 0; i < nrBlocks - 1; ++i) {
				if (_block[i] != BLOCK_MASK) {
					isNaN = false;
					break;
				}
			}
			break;
		}
		bool isNegNaN = isNaN && ((_block[MSU] & MSU_MASK) == MSU_MASK);
		bool isPosNaN = isNaN && (_block[MSU] & MSU_MASK) == (MSU_MASK ^ SIGN_BIT_MASK);
		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) : 
			     (NaNType == NAN_TYPE_SIGNALLING ? isNegNaN : 
				   (NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}

	inline constexpr bool test(size_t bitIndex) const {
		return at(bitIndex);
	}
	inline constexpr bool at(size_t bitIndex) const {
		if (bitIndex < nbits) {
			bt word = _block[bitIndex / bitsInBlock];
			bt mask = bt(1ull << (bitIndex % bitsInBlock));
			return (word & mask);
		}
		throw "bit index out of bounds";
	}
	inline constexpr uint8_t nibble(size_t n) const {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			int nibbleIndexInWord = n % (bitsInBlock >> 2);
			bt mask = 0xF << (nibbleIndexInWord * 4);
			bt nibblebits = mask & word;
			return (nibblebits >> (nibbleIndexInWord * 4));
		}
		throw "nibble index out of bounds";
	}
	inline constexpr bt block(size_t b) const {
		if (b < nrBlocks) {
			return _block[b];
		}
		throw "block index out of bounds";
	}

	void debug() const {
		std::cout << "nbits         : " << nbits << std::endl;
		std::cout << "es            : " << es << std::endl;
		std::cout << "BLOCK_MASK    : " << to_binary<bt>(BLOCK_MASK, true) << std::endl;
		std::cout << "nrBlocks      : " << nrBlocks << std::endl;
		std::cout << "bits in MSU   : " << bitsInMSU << std::endl;
		std::cout << "MSU           : " << MSU << std::endl;
		std::cout << "MSU MASK      : " << to_binary<bt>(MSU_MASK, true) << std::endl;
		std::cout << "SIGN_BIT_MASK : " << to_binary<bt>(SIGN_BIT_MASK, true) << std::endl;
		std::cout << "LSB_BIT_MASK  : " << to_binary<bt>(LSB_BIT_MASK, true) << std::endl;
		std::cout << "MSU CAPTURES E: " << (MSU_CAPTURES_E ? "yes\n" : "no\n");
		std::cout << "EXP_SHIFT     : " << EXP_SHIFT << std::endl;
		std::cout << "MSU EXP MASK  : " << to_binary<bt>(MSU_EXP_MASK, true) << std::endl;
		std::cout << "EXP_BIAS      : " << EXP_BIAS << std::endl;
	}

	// extract the exponent field from the encoding
	inline constexpr void exponent(blockbinary<es, bt>& e) const {
		e.clear();
		switch (nrBlocks) {
		case 0:
			return;
		case 1:
		{
			bt ebits = bt(_block[MSU] & ~SIGN_BIT_MASK);
			e.set_raw_bits(uint64_t(ebits >> EXP_SHIFT));
		}
		break;
		default:
		{
			if (MSU_CAPTURES_E) {
				bt ebits = bt(_block[MSU] & ~SIGN_BIT_MASK);
				e.set_raw_bits(uint64_t(ebits >> EXP_SHIFT));
			}
			else {
				for (size_t i = 0; i < es; ++i) { e.set(i, at(nbits - 1ull - es + i)); }
			}
		}
		break;
		}
	}
	// extract the fraction field from the encoding
	inline constexpr void fraction(blockbinary<fbits, bt>& f) const {
		f.clear();
		switch (nrBlocks) {
		case 0:
			return;
		case 1:
		{
			bt fbits = bt(_block[MSU] & ~MSU_EXP_MASK);
			f.set_raw_bits(fbits >> bt(1ull));
		}
		break;
		default:
		{
			for (size_t i = 0; i < fbits; ++i) { f.set(i, at(nbits - 1ull - es - fbits + i)); }
		}
		break;
		}

	}
	
	// casts to native types
	long long to_long_long() const { return (long long)(to_double()); }
	long double to_long_double() const { return to_double(); }
	// transform value to a native C++ double. We are using doubles to compute,
	// which means that all sub-values need to be representable by doubles.
	// A more accurate appromation would require an adaptive precision algorithm
	// with a final rounding step.
	double to_double() const {
		double v{ 0.0 };
		if (iszero()) return v;
		if (isnan()) {
			v = sign() ? std::numeric_limits<double>::signaling_NaN() : std::numeric_limits<double>::quiet_NaN();
		}
		else if (isinf()) {
			v = sign() ? -INFINITY : INFINITY;
		}
		else {
			double f{ 0.0 };
			double fbit{ 0.5 };
			for (size_t i = nbits - 2ull - es; i > 0; --i) {
				f += at(i) ? fbit : 0.0;
				fbit *= 0.5;
			}
			blockbinary<es, bt> ebits;
			exponent(ebits);
			if (ebits.iszero()) {
				// subnormals: (-1)^s * 2^(2-2^(es-1)) * (f/2^fbits))
				double exponent = subnormal_exponent[es]; // precomputed values for 2^(2-2^(es-1))
				v = exponent * f;
			}
			else {
				// regular: (-1)^s * 2^(e+1-2^(es-1)) * (1 + f/2^fbits))
				double exponent = double(1ll << (unsigned(ebits) + 1ll - (1ll << (es - 1ull))));
				v = exponent * (1 + f);
			}
			v = sign() ? -v : v;
		}
		return v;
	}
	float to_float() const { return float(to_double());	}

	// make conversions to native types explicit
	explicit operator int() const { return to_long_long(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

protected:
	// HELPER methods
	// none

private:
	bt _block[nrBlocks];

	//////////////////////////////////////////////////////////////////////////////
	// friend functions

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t nes, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const areal<nnbits,nes,nbt>& r);
	template<size_t nnbits, size_t nes, typename nbt>
	friend std::istream& operator>> (std::istream& istr, areal<nnbits,nes,nbt>& r);

	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator==(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator!=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator< (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator> (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator<=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
	template<size_t nnbits, size_t nes, typename nbt>
	friend bool operator>=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs);
};

////////////////////// operators
template<size_t nnbits, size_t nes, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const areal<nnbits,nes,nbt>& v) {
	// TODO: make it a native conversion
	double d = double(v);
	ostr << d;
//	bool ubit = v.at(0);
//	ostr << (ubit ? "..." : "=  ");
	return ostr;
}

template<size_t nnbits, size_t nes, typename nbt>
inline std::istream& operator>>(std::istream& istr, const areal<nnbits,nes,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t nnbits, size_t nes, typename nbt>
inline bool operator==(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return false; }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator!=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator< (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return false; }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator> (const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator<=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t nes, typename nbt>
inline bool operator>=(const areal<nnbits,nes,nbt>& lhs, const areal<nnbits,nes,nbt>& rhs) { return !operator< (lhs, rhs); }

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t es, typename bt>
inline areal<nbits, es, bt> operator+(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t es, typename bt>
inline areal<nbits, es, bt> operator-(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t es, typename bt>
inline areal<nbits, es, bt> operator*(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t es, typename bt>
inline areal<nbits, es, bt> operator/(const areal<nbits, es, bt>& lhs, const areal<nbits, es, bt>& rhs) {
	areal<nbits, es, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

// convert to std::string
template<size_t nbits, size_t es, typename bt>
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
template<size_t nbits, size_t es, typename bt>
inline std::string to_binary(const areal<nbits, es, bt>& number, bool nibbleMarker = false) {
	std::stringstream ss;
	ss << 'b';
	size_t index = nbits;
	for (size_t i = 0; i < nbits; ++i) {
		ss << (number.at(--index) ? '1' : '0');
		if (index > 0 && (index % 4) == 0 && nibbleMarker) ss << '\'';
	}
	return ss.str();
}

// helper to report on BlockType blocks
template<typename bt>
inline std::string to_binary(const bt& number, bool nibbleMarker) {
	std::stringstream ss;
	ss << 'b';
	constexpr size_t nbits = sizeof(bt) * 8;
	bt mask = bt(bt(1ull) << (nbits - 1ull));
	size_t index = nbits;
	for (size_t i = 0; i < nbits; ++i) {
		ss << (number & mask ? '1' : '0');
		--index;
		if (index > 0 && (index % 4) == 0 && nibbleMarker) ss << '\'';
		mask >>= 1ul;
	}
	return ss.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<size_t nbits, size_t es, typename bt>
areal<nbits,es> abs(const areal<nbits,es,bt>& v) {
	return areal<nbits,es>(false, v.scale(), v.fraction(), v.isZero());
}


///////////////////////////////////////////////////////////////////////
///   binary logic literal comparisons

// posit - long logic operators
template<size_t nbits, size_t es, typename bt>
inline bool operator==(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator==(lhs, areal<nbits, es, bt>(rhs));
}
template<size_t nbits, size_t es, typename bt>
inline bool operator!=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator!=(lhs, areal<nbits, es, bt>(rhs));
}
template<size_t nbits, size_t es, typename bt>
inline bool operator< (const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<(lhs, areal<nbits, es, bt>(rhs));
}
template<size_t nbits, size_t es, typename bt>
inline bool operator> (const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<(areal<nbits, es, bt>(rhs), lhs);
}
template<size_t nbits, size_t es, typename bt>
inline bool operator<=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return operator<(lhs, areal<nbits, es, bt>(rhs)) || operator==(lhs, areal<nbits, es, bt>(rhs));
}
template<size_t nbits, size_t es, typename bt>
inline bool operator>=(const areal<nbits, es, bt>& lhs, long long rhs) {
	return !operator<(lhs, areal<nbits, es, bt>(rhs));
}

}  // namespace sw::universal
