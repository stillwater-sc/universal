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
#include <universal/internal/blockfraction/blockfraction.hpp>
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

// TODO: does this collide with the definitions in bfloat/areal?
#ifndef BIT_CAST_SUPPORT
#define BIT_CAST_SUPPORT 1
#define CONSTEXPRESSION constexpr
#include <bit>
#else
#ifndef CONSTEXPRESSION
#define CONSTEXPRESSION
#endif
#endif

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw::universal {

// Forward definitions
template<size_t nbits, typename bt> class blocktriple;
template<size_t nbits, typename bt> blocktriple<nbits, bt> abs(const blocktriple<nbits, bt>& v);

template<size_t nbits, typename bt>
blocktriple<nbits, bt>& convert(unsigned long long uint, blocktriple<nbits, bt>& tgt) {
	return tgt;
}

/// <summary>
/// Generalized blocktriple representing a (sign, scale, significant) with unrounded arithmetic
/// </summary>
/// <typeparam name="nbits">number of fraction bits in the significant</typeparam>
template<size_t _nbits, typename bt = uint32_t> 
class blocktriple {
public:
	static constexpr size_t nbits = _nbits;
	static constexpr size_t fbits = nbits;  // a convenience and consistency alias
	static constexpr size_t bfbits = nbits + 2; // bf = 0h.ffff <- nbits of fraction bits plus two bits before radix point
	typedef bt BlockType;
	// to maximize performance, can we make the default blocktype a uint64_t?
	// storage unit for block arithmetic needs to be uin32_t until we can figure out 
	// how to manage carry propagation on uint64_t using assembly code
	using Frac = sw::universal::blockfraction<bfbits, bt>;

	static constexpr size_t bitsInByte = 8ull;
	static constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr size_t nrBlocks = 1ull + ((nbits - 1ull) / bitsInBlock);
	static constexpr size_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64ull - bitsInBlock));

	static constexpr size_t MSU = nrBlocks - 1ull; // MSU == Most Significant Unit, as MSB is already taken

	static constexpr size_t abits = fbits + 3ull;          // size of the addend
	static constexpr size_t mbits = 2ull * fbits;          // size of the multiplier output
	static constexpr size_t divbits = 3ull * fbits + 4ull; // size of the divider output
	static constexpr bt ALL_ONES = bt(~0);

	constexpr blocktriple(const blocktriple&) noexcept = default;
	constexpr blocktriple(blocktriple&&) noexcept = default;

	constexpr blocktriple& operator=(const blocktriple&) noexcept = default;
	constexpr blocktriple& operator=(blocktriple&&) noexcept = default;

	constexpr blocktriple() noexcept : 
		_nan{ false }, 	_inf{ false }, _zero{ true }, 
		_sign{ false }, _scale{ 0 } {} // _significant has default constructor

	// decorated constructors
	constexpr blocktriple(signed char iv) noexcept { *this = iv; }
	constexpr blocktriple(short iv)       noexcept { *this = iv; }
	constexpr blocktriple(int iv)         noexcept { *this = iv; }
	constexpr blocktriple(long iv)        noexcept { *this = iv; }
	constexpr blocktriple(long long iv)   noexcept { *this = iv; }
	constexpr blocktriple(char iv)               noexcept { *this = iv; }
	constexpr blocktriple(unsigned short iv)     noexcept { *this = iv; }
	constexpr blocktriple(unsigned int iv)       noexcept { *this = iv; }
	constexpr blocktriple(unsigned long iv)      noexcept { *this = iv; }
	constexpr blocktriple(unsigned long long iv) noexcept  { *this = iv; }
	constexpr blocktriple(float iv)       noexcept { *this = iv; }
	constexpr blocktriple(double iv)      noexcept { *this = iv; }
	constexpr blocktriple(long double iv) noexcept { *this = iv; }

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

	constexpr blocktriple& operator=(float rhs)              noexcept { return convert_float(rhs); }
	constexpr blocktriple& operator=(double rhs)             noexcept { return convert_double(rhs); }
	constexpr blocktriple& operator=(long double rhs) noexcept {
		return *this = double(rhs);
	};
	
	// align the blocktriple
	inline constexpr void align(int rightShift) noexcept {
		_scale += rightShift;
		_significant >>= rightShift;
	}

	// apply a 2's complement recoding of the fraction bits
	inline constexpr void twosComplement() noexcept {
		_significant.twosComplement();
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
	constexpr void setpos() noexcept { _sign = false; }
	constexpr void setnormal() noexcept {
		_nan = false;
		_inf = false;
		_zero = false;
	}
	constexpr void setsign(bool s) noexcept { _sign = s; }
	constexpr void setscale(int scale) noexcept { _scale = scale; }
	constexpr void setbit(size_t index, bool v = true) noexcept { _significant.setbit(index, v); }
	constexpr void setbits(uint64_t raw) noexcept {
		// do not clear the nan/inf/zero booleans: caller must manage
		_significant.setbits(raw);
	}

	// selectors
	inline constexpr bool isnan()       const noexcept { return _nan; }
	inline constexpr bool isinf()       const noexcept { return _inf; }
	inline constexpr bool iszero()      const noexcept { return _zero; }
	inline constexpr bool ispos()       const noexcept { return !_sign; }
	inline constexpr bool isneg()       const noexcept { return _sign; }
	inline constexpr bool sign()        const noexcept { return _sign; }
	inline constexpr int  scale()       const noexcept { return _scale; }
	inline constexpr Frac significant() const noexcept { return _significant; }

	// fraction bit accessors
	inline constexpr bool at(size_t index)   const noexcept { return _significant.at(index); }
	inline constexpr bool test(size_t index) const noexcept { return _significant.at(index); }

	// conversion operators
	explicit operator float()       const noexcept { return to_float(); }
	explicit operator double()      const noexcept { return to_double(); }
	explicit operator long double() const noexcept { return to_long_double(); }

	// ALU operators
	/// <summary>
	/// add two real numbers with abits fraction bits yielding an nbits unrounded sum
	/// To avoid fraction bit copies, the input requirements are pushed to the
	/// calling environment to prepare the correct storage
	/// </summary>
	/// <param name="lhs">ephemeral blocktriple<abits> that may get modified</param>
	/// <param name="rhs">ephemeral blocktriple<abits> that may get modified</param>
	/// <param name="result">unrounded sum</param>
	void add(blocktriple<nbits-1, bt>& lhs, blocktriple<nbits-1, bt>& rhs) {
		int lhs_scale = lhs.scale();
		int rhs_scale = rhs.scale();
		int scale_of_result = std::max(lhs_scale, rhs_scale);

		// avoid copy by directly manipulating the fraction bits of the arguments
		int expDiff = lhs_scale - rhs_scale;
		if (expDiff < 0) {
			lhs.align(-expDiff);
		}
		else if (expDiff > 0) {
			rhs.align(expDiff);
		}
		if (lhs.isneg()) lhs._significant.twosComplement();
		if (rhs.isneg()) rhs._significant.twosComplement();

		_significant.uradd(lhs._significant, rhs._significant);

		if constexpr (_trace_btriple_add) {
			std::cout << typeid(*this).name() << '\n';
			std::cout << "lhs : " << to_binary(lhs) << " : " << lhs << '\n';
			std::cout << "rhs : " << to_binary(rhs) << " : " << rhs << '\n';
			std::cout << typeid(_significant).name() << '\n';
			std::cout << "sum : " << to_binary(*this) << " : " << *this << '\n';
		}
		if (_significant.iszero()) {
			clear();
		}
		else {
			if (isneg()) {
				_significant.twosComplement();
				_sign = true;
			}
			_scale = scale_of_result;
			if (_significant.checkCarry()) {
				_scale += 1;
				// no need to shift as the default behavior has all the bits
				// already at the right place for this case
			}
			else {
				// need to normalize
				_significant <<= 1;
			}
		}
	}

private:
	// special cases to keep track of
	bool _nan; // most dominant state
	bool _inf; // second most dominant state
	bool _zero;// third most dominant special case

	// the triple (sign, scale, significant)
	bool _sign;
	int  _scale;

public:
	Frac _significant;

	// helpers

private:
	/// <summary>
/// round a set of source bits to the present representation.
/// srcbits is the number of bits of significant in the source representation
/// round<> is intended only for rounding raw IEEE-754 bits
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
			constexpr uint32_t upper = 8 * sizeof(StorageType) + 2;
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
//		_significant = round<sizeInBits, uint64_t>(raw);
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
//		_significant = round<sizeInBits, uint64_t>(raw);
		return *this;
	}

	CONSTEXPRESSION inline blocktriple& convert_float(float rhs) noexcept {
#if BIT_CAST_SUPPORT
		// normal number
		uint32_t bc = std::bit_cast<uint32_t>(rhs);
		bool s = (0x8000'0000ul & bc);
		uint32_t raw_exp = static_cast<uint32_t>((0x7F80'0000ul & bc) >> 23);
		uint32_t raw = (1ul << 23) | (0x007F'FFFFul & bc);
#else // !BIT_CAST_SUPPORT
		float_decoder decoder;
		decoder.f = rhs;
		bool s = decoder.parts.sign ? true : false;
		uint32_t raw_exp = decoder.parts.exponent;
		uint32_t raw = (1ul << 23) | decoder.parts.fraction;
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
				_nan = false;
				_inf = true;
				_sign = s;  // + or - infinity
				return *this;
			}
		}
		if (rhs == 0.0f) { // IEEE rule: this is valid for + and - 0.0
			_nan = false;
			_inf = false;
			_zero = true;
			_sign = s;
			return *this;
		}
		// normal number, not zero
		_nan = false;
		_inf = false;
		_zero = false;
		_sign = s;
		_scale = static_cast<int>(raw_exp) - 127;
		raw <<= 1;
		uint32_t rounded_bits = round<24, uint32_t>(raw);
		_significant.setbits(rounded_bits);
		return *this;
	}
	CONSTEXPRESSION inline blocktriple& convert_double(double rhs) noexcept { // TODO: deal with subnormals and inf
#if BIT_CAST_SUPPORT
		uint64_t bc = std::bit_cast<uint64_t>(rhs);
		bool s = (0x8000'0000'0000'0000ull & bc);
		uint32_t raw_exp = static_cast<uint32_t>((0x7FF0'0000'0000'0000ull & bc) >> 52);
		uint64_t raw = (1ull << 52) | (0x000F'FFFF'FFFF'FFFFull & bc);
#else
		double_decoder decoder;
		decoder.d = rhs;
		bool s = decoder.parts.sign ? true : false;
		uint64_t raw_exp = decoder.parts.exponent;
		uint64_t raw = (1ull << 52) | decoder.parts.fraction;
#endif // !BIT_CAST_SUPPORT

		// special case handling
		if (raw_exp == 0x7FFu) { // special cases
			if (raw == 1ul || raw == 0x0040'0001ul) {
				// 1.111'1111'1111.0000000000...0000000001 signalling nan
				// 0.111'1111'1111.0000000000...0000000001 signalling nan
				// MSVC
				// 1.111'1111'1111.1000000000...0000000001 signalling nan
				// 0.111'1111'1111.1000000000...0000000001 signalling nan
				// NAN_TYPE_SIGNALLING;
				_nan = true;
				_inf = true; // this is the encoding of a signalling NaN
				return *this;
	}
			if (raw == 0x0008'0000'0000'0000ull) {
				// 1.111'1111'1111.1000000000...0000000000 quiet nan
				// 0.111'1111'1111.1000000000...0000000000 quiet nan
				// NAN_TYPE_QUIET);
				_nan = true;
				_inf = false; // this is the encoding of a quiet NaN
				return *this;
			}
			if (raw == 0ul) {
				// 1.11111111.00000000000000000000000 -inf
				// 0.11111111.00000000000000000000000 +inf
				_nan = false;
				_inf = true;
				_sign = s;  // + or - infinity
				return *this;
			}
		}
		if (rhs == 0.0f) { // IEEE rule: this is valid for + and - 0.0
			_nan = false;
			_inf = false;
			_zero = true;
			_sign = s;
			return *this;
		}
		// normal number
		_nan = false;
		_inf = false;
		_zero = false;
		_sign = s;
		_scale = static_cast<int>(raw_exp) - 1023;
		raw <<= 1;
		uint64_t rounded_bits = round<53, uint64_t>(raw); // round manipulates _scale if needed
		_significant.setbits(rounded_bits);
		return *this;
	}

	double      to_float() const {
		return float(to_double());
	}
	double      to_double() const {  // TODO: this needs a native, correctly rounded version
		if (_zero) return 0.0;
		// significant is in the form: 0h.ffff
		double v{ 0.0 };
		if (_significant.test(bfbits - 1)) v = 2.0;
		if (_significant.test(bfbits - 2)) v += 1.0;
		double scale = 0.5;
		for (int i = static_cast<int>(bfbits - 3); i >= 0; i--) {
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
	template<size_t nnbits, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const blocktriple<nnbits, bbt>& a);
	template<size_t nnbits, typename bbt>
	friend std::istream& operator>> (std::istream& istr, blocktriple<nnbits, bbt>& a);

	// declare as friends to avoid needing a marshalling function to get significant bits out
	template<size_t nnbits, typename bbt>
	friend std::string to_binary(const blocktriple<nnbits, bbt>&, bool);
	template<size_t nnbits, typename bbt>
	friend std::string to_triple(const blocktriple<nnbits, bbt>&, bool);

	// logic operators
	template<size_t nnbits, typename bbt>
	friend bool operator==(const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator!=(const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator< (const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator> (const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator<=(const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
	template<size_t nnbits, typename bbt>
	friend bool operator>=(const blocktriple<nnbits, bbt>& lhs, const blocktriple<nnbits, bbt>& rhs);
};

////////////////////// operators
template<size_t nbits, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<nbits, bt>& a) {
	if (a._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << double(a);
	}
	return ostr;
}

template<size_t nbits, typename bt>
inline std::istream& operator>> (std::istream& istr, const blocktriple<nbits, bt>& a) {
	istr >> a._fraction;
	return istr;
}

template<size_t nbits, typename bt>
inline bool operator==(const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._significant == rhs._significant && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }

template<size_t nbits, typename bt>
inline bool operator!=(const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return !operator==(lhs, rhs); }

template<size_t nbits, typename bt>
inline bool operator< (const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) {
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

template<size_t nbits, typename bt>
inline bool operator> (const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, typename bt>
inline bool operator<=(const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, typename bt>
inline bool operator>=(const blocktriple<nbits, bt>& lhs, const blocktriple<nbits, bt>& rhs) { return !operator< (lhs, rhs); }


////////////////////////////////// string conversion functions //////////////////////////////

template<size_t nbits, typename bt>
std::string to_binary(const sw::universal::blocktriple<nbits, bt>& a, bool bNibbleMarker = true) {
	return to_triple(a, bNibbleMarker);
}

template<size_t nbits, typename bt>
std::string to_triple(const blocktriple<nbits, bt>& a, bool bNibbleMarker = true) {
	std::stringstream s;
	s << (a._sign ? "(-, " : "(+, ");
	s << a._scale << ", ";
	s << to_binary(a._significant, bNibbleMarker) << ')';
	return s.str();
}

template<size_t nbits, typename bt>
blocktriple<nbits> abs(const blocktriple<nbits, bt>& a) {
	blocktriple<nbits> absolute(a);
	absolute.setpos();
	return absolute;
}

}  // namespace sw::universal
