#pragma once
// blocktriple.hpp: definition of a (sign, scale, significant) representation of a real value
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <iostream>
#include <iomanip>
#include <limits>

#include <universal/native/ieee754.hpp>
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
template<size_t nbits, typename bt> class blocktriple;
template<size_t nbits, typename bt> blocktriple<nbits,bt> abs(const blocktriple<nbits,bt>& v);


template<size_t nbits, typename bt>
blocktriple<nbits, bt>& convert(unsigned long long uint, blocktriple<nbits, bt>& tgt) {
	return tgt;
}

/// <summary>
/// Generalized blocktriple representing a (sign, scale, significant) with unrounded arithmetic
/// </summary>
/// <typeparam name="bt"></typeparam>
template<size_t nbits, typename bt = uint32_t>
class blocktriple {
public:
	static constexpr size_t fbits = nbits - 1;
	using bits = blockbinary<nbits, bt>;

	constexpr blocktriple(const blocktriple&) noexcept = default;
	constexpr blocktriple(blocktriple&&) noexcept = default;

	constexpr blocktriple& operator=(const blocktriple&) noexcept = default;
	constexpr blocktriple& operator=(blocktriple&&) noexcept = default;

	constexpr blocktriple() noexcept : 
		_nan{ false }, 	_inf{ false }, _zero{ true }, 
		_sign{ false }, _scale{ 0 }, _significant{ 0 } {}

	constexpr blocktriple(signed char iv) noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(short iv)       noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(int iv)         noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(long iv)        noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(long long iv)   noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(char iv)               noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(unsigned short iv)     noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(unsigned int iv)       noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(unsigned long iv)      noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(unsigned long long iv) noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(float iv)       noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(double iv)      noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }
	constexpr blocktriple(long double iv) noexcept :
		_nan{ false }, _inf{ false }, _zero{ true },
		_sign{ false }, _scale{ 0 }, _significant{ 0 } { *this = iv; }

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

	template<typename Ty>
	constexpr blocktriple& convert_unsigned_integer(const Ty& rhs) noexcept {
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
	constexpr blocktriple& convert_signed_integer(const Ty& rhs) noexcept {
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
	constexpr blocktriple& operator=(float rhs) noexcept { // TODO: deal with subnormals and inf
		_nan = false;
		_inf = false;
		_zero = true;
		if (rhs == 0.0f) return *this;
#if BIT_CAST_SUPPORT
		_zero = false; 
		// TODO: check inf and NaN
		_inf = false; _nan = false;
		// normal number
		uint32_t bc = std::bit_cast<uint32_t>(rhs);
		_sign = (0x8000'0000 & bc);
		_scale = int((0x7F80'0000 & bc) >> 23) - 127;
		uint32_t raw = (1ul << 23) | (0x007F'FFFF & bc);
		_significant = round<24, uint32_t>(raw);
#else
		_zero = true;
		_sign = false;
		_scale = 0;
		_significant.clear();
#endif // !BIT_CAST_SUPPORT
		return *this;
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
		_significant = round<53, uint64_t>(raw);
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
	
	/// <summary>
	/// round a set of source bits to the present representation.
	/// srcbits is the number of bits of significant in the source representation
	/// </summary>
	/// <typeparam name="StorageType"></typeparam>
	/// <param name="raw"></param>
	/// <returns></returns>
	template<size_t srcbits, typename StorageType>
	constexpr bt round(StorageType raw) noexcept {
		if constexpr (nbits < srcbits) {
			 // round to even: lsb guard round sticky
			// collect guard, round, and sticky bits
			// this same logic will work for the case where
			// we only have a guard bit and no round and sticky bits
			// because the mask logic will make round and sticky both 0
			constexpr uint32_t shift = srcbits - nbits - 1;
			StorageType mask = (StorageType(1ull) << shift);
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			if constexpr (shift > 1) { // protect against a negative shift
				mask = StorageType(StorageType(~0) << (shift - 2));
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
			raw <<= shift;
		}
		bt significant = bt(raw);
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

	// selectors
	inline constexpr bool isnan()       const noexcept { return _nan; }
	inline constexpr bool isinf()       const noexcept { return _inf; }
	inline constexpr bool iszero()      const noexcept { return _zero; }
	inline constexpr bool ispos()       const noexcept { return !_sign; }
	inline constexpr bool isneg()       const noexcept { return _sign; }
	inline constexpr bool sign()        const noexcept { return _sign; }
	inline constexpr int  scale()       const noexcept { return _scale; }
	inline constexpr bits significant() const noexcept { return _significant; }

	// conversion operators
	explicit operator float()       const noexcept { return to_float(); }
	explicit operator double()      const noexcept { return to_double(); }
	explicit operator long double() const noexcept { return to_long_double(); }

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
	template<size_t sbits, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const blocktriple<sbits, bbt>& a);
	template<size_t sbits, typename bbt>
	friend std::istream& operator>> (std::istream& istr, blocktriple<sbits, bbt>& a);

	// declare as friends to avoid needing a marshalling function to get significant bits out
	template<size_t sbits, typename bbt>
	friend std::string to_binary(const blocktriple<sbits, bbt>&, bool);
	template<size_t sbits, typename bbt>
	friend std::string to_triple(const blocktriple<sbits, bbt>&, bool);

	// logic operators
	template<size_t ssbits, typename bbt>
	friend bool operator==(const blocktriple<ssbits, bbt>& lhs, const blocktriple<ssbits, bbt>& rhs);
	template<size_t ssbits, typename bbt>
	friend bool operator!=(const blocktriple<ssbits, bbt>& lhs, const blocktriple<ssbits, bbt>& rhs);
	template<size_t ssbits, typename bbt>
	friend bool operator< (const blocktriple<ssbits, bbt>& lhs, const blocktriple<ssbits, bbt>& rhs);
	template<size_t ssbits, typename bbt>
	friend bool operator> (const blocktriple<ssbits, bbt>& lhs, const blocktriple<ssbits, bbt>& rhs);
	template<size_t ssbits, typename bbt>
	friend bool operator<=(const blocktriple<ssbits, bbt>& lhs, const blocktriple<ssbits, bbt>& rhs);
	template<size_t ssbits, typename bbt>
	friend bool operator>=(const blocktriple<ssbits, bbt>& lhs, const blocktriple<ssbits, bbt>& rhs);
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

template<size_t sbits, typename bt>
inline bool operator==(const blocktriple<sbits, bt>& lhs, const blocktriple<sbits, bt>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._significant == rhs._significant && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }

template<size_t sbits, typename bt>
inline bool operator!=(const blocktriple<sbits, bt>& lhs, const blocktriple<sbits, bt>& rhs) { return !operator==(lhs, rhs); }

template<size_t sbits, typename bt>
inline bool operator< (const blocktriple<sbits, bt>& lhs, const blocktriple<sbits, bt>& rhs) {
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
	return false;
}

template<size_t sbits, typename bt>
inline bool operator> (const blocktriple<sbits, bt>& lhs, const blocktriple<sbits, bt>& rhs) { return  operator< (rhs, lhs); }
template<size_t sbits, typename bt>
inline bool operator<=(const blocktriple<sbits, bt>& lhs, const blocktriple<sbits, bt>& rhs) { return !operator> (lhs, rhs); }
template<size_t sbits, typename bt>
inline bool operator>=(const blocktriple<sbits, bt>& lhs, const blocktriple<sbits, bt>& rhs) { return !operator< (lhs, rhs); }


template<size_t nbits, typename bt>
std::string to_binary(const sw::universal::blocktriple<nbits, bt>& a, bool bNibbleMarker = true) {
	std::stringstream s;
	s << (a._sign ? "(-, " : "(+, ");
	s << a._scale << ", "; 
	s << to_binary(a._significant, bNibbleMarker) << ')';
	return s.str();
}

template<size_t nbits, typename bt>
std::string to_triple(const blocktriple<nbits, bt>& a, bool bNibbleMarker = true) {
	std::stringstream s;
	s << (a._sign ? "(-, " : "(+, ") << a._scale << ", " << to_binary(a._significant, bNibbleMarker) << ')';
	return s.str();
}

// add two values with fbits fraction bits, round them to abits, and return the abits+1 result value
template<size_t fhbits, size_t abits, typename bt>
void module_add(const blocktriple<fhbits, bt>& lhs, const blocktriple<fhbits,bt>& rhs, blocktriple<abits + 1, bt>& result) {
	// with sign/magnitude adders it is customary to organize the computation 
	// along the four quadrants of sign combinations
	//  + + = +
	//  + - =   lhs > rhs ? + : -
	//  - + =   lhs > rhs ? - : +
	//  - - = 
	// to simplify the result processing assign the biggest 
	// absolute value to R1, then the sign of the result will be sign of the value in R1.

	int lhs_scale = lhs.scale();
	int rhs_scale = rhs.scale();
	int scale_of_result = std::max(lhs_scale, rhs_scale);

#ifdef LATER
	// align the fractions
	blockbinary<abits> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	blockbinary<abits> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = rhs.sign();
	bool signs_are_different = r1_sign != r2_sign;

	if (signs_are_different && abs(lhs) < abs(rhs)) {
		std::swap(r1, r2);
		std::swap(r1_sign, r2_sign);
	}

//	if (signs_are_different) r2 = twos_complement(r2);

//	if (_trace_btriple_add) {
//		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
//		if (signs_are_different) {
//			std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2 orig  " << twos_complement(r2) << std::endl;
//		}
//		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
//	}

	blockbinary<abits + 1> sum;
	bool carry = false; // add_unsigned(r1, r2, sum);

//	if (_trace_btriple_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

	int shift = 0;
	if (carry) {
		if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
			shift = -1;
		}
		else {
			// the carry && signs!= implies ||result|| < ||r1||, must find MSB (in the complement)
			for (int i = int(abits) - 1; i >= 0 && !sum[size_t(i)]; --i) {
				++shift;
			}
		}
	}
	assert(shift >= -1);

	if (shift >= long(abits)) {            // we have actual 0                            
		sum.reset();
		result.set(false, 0, sum, true, false, false);
		return;
	}

	scale_of_result -= shift;
	const int hpos = int(abits) - 1 - shift;         // position of the hidden bit 
	sum <<= abits - hpos + 1;
	if (_trace_btriple_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
#endif
}

}  // namespace sw::universal
