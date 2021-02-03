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
		_significant = round_to<sizeInBits, uint64_t>(raw);
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
		_significant = round_to<sizeInBits, uint64_t>(raw);
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
		_significant = round_to<24, uint32_t>(raw);
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
		_significant = round_to<53, uint64_t>(raw);
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
	/// round to a target number of bits. tgtbits is the number of significant bits to round to.
	/// </summary>
	/// <typeparam name="StorageType"></typeparam>
	/// <param name="raw"></param>
	/// <returns></returns>
	template<size_t nrsrcbits, typename StorageType>
	constexpr bt round_to(StorageType raw) noexcept {
		if constexpr (nbits < nrsrcbits) {
			 // round to even: lsb guard round sticky
			// collect guard, round, and sticky bits
			// this same logic will work for the case where
			// we only have a guard bit and no round and sticky bits
			// because the mask logic will make round and sticky both 0
			constexpr uint32_t shift = nrsrcbits - nbits - 1;
			StorageType mask = (StorageType(1ull) << shift);
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			if constexpr (shift > 1) { // protect against a negative shift
				mask = StorageType(-1ll << (shift - 2));
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
			constexpr size_t shift = nbits - nrsrcbits;
			raw <<= shift;
		}
		bt significant = bt(raw);
		return significant;
	}

	// modifiers
	constexpr void reset() noexcept {
		_nan = false;
		_inf = false;
		_zero = true;
		_sign = false;
		_scale = 0;
		_significant.clear();
	}

	constexpr void set_raw_bits(uint64_t raw) noexcept {
		reset();
		_significant.set_raw_bits(raw);
	}
	// conversion operators
	explicit operator float() const { return to_float(); }
	explicit operator double() const { return to_double(); }
	explicit operator long double() const { return to_long_double(); }

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

#ifdef LATER
/// <summary>
/// Compute class to provide a linear floating-point representation and arithmetic. 
/// The representation is (sign, scale, significant), i.e. normalized binary floating point numbers.
/// All special cases, inf, NaN, zero are managed as booleans for fast checking.
/// 
/// Parameters are the number of fraction bits to maintain, and the blocktype to organize them by.
/// The exponent is an implicit signed integer.
/// </summary>
/// <typeparam name="bt">block type to use: default is uint32_t</typeparam>
template<size_t significantbits, typename bt = uint32_t>
class blocktriple {
public:
	static constexpr size_t nbits = significantbits;
	static constexpr size_t fbits = significantbits - 1;

	constexpr blocktriple() : _sign(false), _scale(0), _significant(0), _inf(false), _zero(true), _nan(false) {}
	constexpr blocktriple(bool sign, int scale, const blockbinary<significantbits, bt>& significant_bits, bool zero = true, bool inf = false)
		: _sign(sign), _scale(scale), _signficant(significant_bits), _inf(inf), _zero(zero), _nan(false) {}

	blocktriple(const blocktriple&) noexcept = default;
	blocktriple(blocktriple&&) noexcept = default;

	constexpr blocktriple(const signed char initial_value)        { *this = initial_value; }
	constexpr blocktriple(const short initial_value)              { *this = initial_value; }
	constexpr blocktriple(const int initial_value) noexcept       { *this = initial_value; }
	constexpr blocktriple(const long initial_value)               { *this = initial_value; }
	constexpr blocktriple(const long long initial_value)          { *this = initial_value; }
	constexpr blocktriple(const char initial_value)               { *this = initial_value; }
	constexpr blocktriple(const unsigned short initial_value)     { *this = initial_value; }
	constexpr blocktriple(const unsigned int initial_value)       { *this = initial_value; }
	constexpr blocktriple(const unsigned long initial_value)      { *this = initial_value; }
	constexpr blocktriple(const unsigned long long initial_value) { *this = initial_value; }
	constexpr blocktriple(const float initial_value)              { *this = initial_value; }
	constexpr blocktriple(const double initial_value)             { *this = initial_value; }
	constexpr blocktriple(const long double initial_value)        { *this = initial_value; }

	blocktriple& operator=(const blocktriple&) noexcept = default;
	blocktriple& operator=(blocktriple&&) noexcept = default;

	constexpr blocktriple& operator=(const signed char rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	constexpr blocktriple& operator=(const short rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	constexpr blocktriple& operator=(const int rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	constexpr blocktriple& operator=(const long rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	constexpr blocktriple& operator=(const long long rhs) {
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
		if (rhs == 0) {
			setzero();
			return *this;
		}
		reset();
		_sign = (0x8000000000000000 & rhs);  // 1 is negative, 0 is positive
		if (_sign) {
			// process negative number: process 2's complement of the input
			_scale = findMostSignificantBit(-rhs) - 1;
			uint64_t _fraction_without_hidden_bit = _scale == 0 ? 0 : (-rhs << (64 - _scale));
			_fraction.set_raw_bits(_fraction_without_hidden_bit);
			//take_2s_complement();
			_nrOfBits = fbits;
			if (_trace_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		}
		else {
			// process positive number
			_scale = findMostSignificantBit(rhs) - 1;
			uint64_t _fraction_without_hidden_bit = _scale == 0 ? 0 : (rhs << (64 - _scale));
			_fraction.set_raw_bits(_fraction_without_hidden_bit);
			_nrOfBits = fbits;
			if (_trace_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		}
		return *this;
	}
	constexpr blocktriple& operator=(const char rhs) {
		*this = (unsigned long long)(rhs);
		return *this;
	}
	constexpr blocktriple& operator=(const unsigned short rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	constexpr blocktriple& operator=(const unsigned int rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	constexpr blocktriple& operator=(const unsigned long rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	constexpr blocktriple& operator=(const unsigned long long rhs) {
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
		if (rhs == 0) {
			setzero();
		}
		else {
			reset();
			_scale = findMostSignificantBit(rhs) - 1;
			uint64_t _fraction_without_hidden_bit = _scale == 0 ? 0ull : (rhs << (64 - _scale));
			_fraction = _fraction_without_hidden_bit;
			_nrOfBits = fbits;
		}
		if (_trace_conversion) std::cout << "uint64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
		return *this;
	}
	*/
	constexpr blocktriple& operator=(const float rhs) {
		reset();
		/*
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_nrOfBits = fbits;
			_zero = true;
			break;
		case FP_INFINITE:
			_inf  = true;
			_sign = true;
			break;
		case FP_NAN:
			_nan = true;
			_sign = true;
			break;
		case FP_SUBNORMAL:
		case FP_NORMAL:
			{
			    float _fr{ 0.0f };
				unsigned int _23b_fraction_without_hidden_bit{ 0u };
				int _exponent{ 0 };
				extract_fp_components(rhs, _sign, _exponent, _fr, _23b_fraction_without_hidden_bit);
				_scale = _exponent - 1;
				_fraction = _23b_fraction_without_hidden_bit;
				_nrOfBits = fbits;
				if (_trace_conversion) std::cout << "float " << rhs << " sign " << _sign << " scale " << _scale << " 23b fraction 0x" << std::hex << _23b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
			}
			break;
		}
		*/
		return *this;
	}
	constexpr blocktriple& operator=(const double rhs) {
		reset();
		/*
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_nrOfBits = fbits;
			_zero = true;
			break;
		case FP_INFINITE:
			_inf = true;
			_sign = true;
			break;
		case FP_NAN:
			_nan = true;
			_sign = true;
			break;
		case FP_SUBNORMAL:
		case FP_NORMAL:
			{
				double _fr{ 0.0 };
				unsigned long long _52b_fraction_without_hidden_bit{ 0ull };
				int _exponent{ 0 };
				extract_fp_components(rhs, _sign, _exponent, _fr, _52b_fraction_without_hidden_bit);
				_scale = _exponent - 1;
				_fraction = 0; // extract_52b_fraction<fbits, bt>(_52b_fraction_without_hidden_bit);
				_nrOfBits = fbits;
				if (_trace_conversion) std::cout << "double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _52b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
			}
			break;
		}
		*/
		return *this;
	}
	constexpr blocktriple& operator=(const long double rhs) {
		reset();
		/*
		if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_nrOfBits = fbits;
			_zero = true;
			break;
		case FP_INFINITE:
			_inf = true;
			_sign = true;
			break;
		case FP_NAN:
			_nan = true;
			_sign = true;
			break;
		case FP_SUBNORMAL:
		case FP_NORMAL:
			{
				long double _fr{ 0.0l };
				unsigned long long _63b_fraction_without_hidden_bit{ 0ull };
				int _exponent{ 0 };
				extract_fp_components(rhs, _sign, _exponent, _fr, _63b_fraction_without_hidden_bit);
				_scale = _exponent - 1;
				// how to interpret the fraction bits: TODO: this should be a static compile-time code block
				if (sizeof(long double) == 8) {
					// we are just a double and thus only have 52bits of fraction
					_fraction = 0; //  extract_52b_fraction<fbits, bt>(_63b_fraction_without_hidden_bit);
					if (_trace_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

				}
				else if (sizeof(long double) == 16) {
					// how to differentiate between 80bit and 128bit formats?
					_fraction = 0; //  extract_63b_fraction<fbits, bt>(_63b_fraction_without_hidden_bit);
					if (_trace_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 63b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

				}
				_nrOfBits = fbits;
			}
			break;
		}
		*/
		return *this;
	}

	// conversion operators
	explicit operator float() const { return to_float(); }
	explicit operator double() const { return to_double(); }
	explicit operator long double() const { return to_long_double(); }

	// operators
	blocktriple operator-() const {				
		return blocktriple<fbits>(!_sign, _scale, _fraction, _zero, _inf);
	}

	// modifiers
	constexpr void reset() noexcept {
		_sign  = false;
		_scale = 0;
		_inf = false;
		_zero = false;
		_nan = false;
		_significant.clear();
	}
	constexpr void set(bool sign, int scale, blockbinary<fbits> fraction_bits, bool zero, bool inf, bool nan = false) noexcept {
		_sign     = sign;
		_scale    = scale;
		_zero     = zero;
		_inf      = inf;
		_nan      = nan;
	}
	constexpr void setzero() noexcept {
		_zero     = true;
		_sign     = false;
		_inf      = false;
		_nan      = false;
		_scale    = 0;
		_significant.clear();
	}
	constexpr void setinf() noexcept {      // this maps to NaR on the posit side, and that has a sign = 1
		_inf      = true;
		_sign     = true;
		_zero     = false;
		_nan      = false;
		_scale    = 0;
		_significant.reset();
	}
	constexpr void setnan() noexcept {		// this will also map to NaR
		_nan      = true;
		_sign     = true;
		_zero     = false;
		_inf      = false;
		_scale    = 0;
		_significant.reset();
	}
	constexpr void setscale(int e) { _scale = e; }
	inline void set_raw_bits(uint64_t v) { _fraction.set_raw_bits(v); }
	
	// selectors
	inline bool isneg() const { return _sign; }
	inline bool ispos() const { return !_sign; }
	inline bool iszero() const { return _zero; }
	inline bool isinf() const { return _inf; }
	inline bool isnan() const { return _nan; }
	inline bool sign() const { return _sign; }
	inline int scale() const { return _scale; }
	blockbinary<fbits, bt> fraction() const { return _fraction; }

	/// Normalized shift (e.g., for addition).
	template <size_t Size>
	blockbinary<Size, bt> nshift(long shift) const {
		blockbinary<Size> number;

#if BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTIONS
		// Check range
		if (long(fbits) + shift >= long(Size))
			throw shift_too_large{};
#else
		// Check range
		if (long(fbits) + shift >= long(Size)) {
			std::cerr << "nshift: shift is too large\n";
			number.reset();
			return number;
		}
#endif // BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTIONS

		const long hpos = fbits + shift;       // position of hidden bit
												  
		if (hpos <= 0) {   // If hidden bit is LSB or beyond just set uncertainty bit and call it a day
			number[0] = true;
			return number;
		}
		number[hpos] = true;                   // hidden bit now safely set

											   // Copy fraction bits into certain part
		for (long npos = hpos - 1, fpos = long(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
			number[npos] = _fraction[fpos];

		// Set uncertainty bit
		bool uncertainty = false;
		for (long fpos = std::min(long(fbits) - 1, -shift); fpos >= 0 && !uncertainty; --fpos)
			uncertainty |= _fraction[fpos];
		number[0] = uncertainty;
		return number;
	}

	// get the fraction value including the implicit hidden bit (this is at an exponent level 1 smaller)
	template<typename Ty = double>
	Ty get_implicit_fraction_value() const {
		if (_zero) return (long double)0.0;
		Ty v = 1.0;
		Ty scale = 0.5;
		for (int i = int(fbits) - 1; i >= 0; i--) {
			if (_fraction.test(i)) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		return v;
	}
	int sign_value() const { return (_sign ? -1 : 1); }
	double scale_value() const {
		if (_zero) return (long double)(0.0);
		return std::pow((long double)2.0, (long double)_scale);
	}
	template<typename Ty = double>
	Ty fraction_value() const {
		if (_zero) return (long double)0.0;
		Ty v = 1.0;
		Ty scale = 0.5;
		for (int i = int(fbits) - 1; i >= 0; i--) {
			if (_fraction.test(i)) v += scale;
			scale *= 0.5;
			if (scale == 0.0) break;
		}
		return v;
	}
	long double to_long_double() const {
		return sign_value() * scale_value() * fraction_value<long double>();
	}
	double      to_double() const {
		return sign_value() * scale_value() * fraction_value<double>();
	}
	float       to_float() const {
		return float(sign_value() * scale_value() * fraction_value<float>());
	}

	// TODO: this does not implement a 'real' right extend. tgtbits need to be shorter than fbits
	template<size_t srcbits, size_t tgtbits>
	void right_extend(const blocktriple<srcbits,bt>& src) {
		_sign = src.sign();
		_scale = src.scale();
		_nrOfBits = tgtbits;
		_inf = src.isinf();
		_zero = src.iszero();
		_nan = src.isnan();
		blockbinary<srcbits, bt> src_fraction = src.fraction();
		if (!_inf && !_zero && !_nan) {
			for (int s = srcbits - 1, t = tgtbits - 1; s >= 0 && t >= 0; --s, --t)
				_fraction[t] = src_fraction[s];
		}
	}
	template<size_t tgt_fbits>
	blocktriple<tgt_fbits, bt> round_to() {
		blockbinary<tgt_fbits, bt> rounded_fraction;
		if (tgt_fbits == 0) {
			bool round_up = false;
			if (fbits >= 2) {
				bool blast = _fraction[int(fbits) - 1];
				bool sb = anyAfter(_fraction, int(fbits) - 2);
				if (blast && sb) round_up = true;
			}
			else if (fbits == 1) {
				round_up = _fraction[0];
			}
			return blocktriple<tgt_fbits, bt>(_sign, (round_up ? _scale + 1 : _scale), rounded_fraction, _zero, _inf);
		}
		else {
			if (!_zero || !_inf) {
				if (tgt_fbits < fbits) {
					int rb = int(tgt_fbits) - 1;
					int lb = int(fbits) - int(tgt_fbits) - 1;
					for (int i = int(fbits) - 1; i > lb; i--, rb--) {
						rounded_fraction[rb] = _fraction[i];
					}
					bool blast = _fraction[lb];
					bool sb = false;
					if (lb > 0) sb = anyAfter(_fraction, lb-1);
					if (blast || sb) rounded_fraction[0] = true;
				}
				else {
					int rb = int(tgt_fbits) - 1;
					for (int i = int(fbits) - 1; i >= 0; i--, rb--) {
						rounded_fraction[rb] = _fraction[i];
					}
				}
			}
		}
		return blocktriple<tgt_fbits, bt>(_sign, _scale, rounded_fraction, _zero, _inf);
	}

private:
	bool                    _sign;
	int                     _scale;
	bool                    _inf;
	bool                    _zero;
	bool                    _nan;
	blockbinary<significantbits, bt>  _significant;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t ffbits, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const blocktriple<ffbits, bbt>& r);
	template<size_t ffbits, typename bbt>
	friend std::istream& operator>> (std::istream& istr, blocktriple<ffbits, bbt>& r);

	// logic operators
	template<size_t ffbits, typename bbt>
	friend bool operator==(const blocktriple<ffbits, bbt>& lhs, const blocktriple<ffbits, bbt>& rhs);
	template<size_t ffbits, typename bbt>
	friend bool operator!=(const blocktriple<ffbits, bbt>& lhs, const blocktriple<ffbits, bbt>& rhs);
	template<size_t ffbits, typename bbt>
	friend bool operator< (const blocktriple<ffbits, bbt>& lhs, const blocktriple<ffbits, bbt>& rhs);
	template<size_t ffbits, typename bbt>
	friend bool operator> (const blocktriple<ffbits, bbt>& lhs, const blocktriple<ffbits, bbt>& rhs);
	template<size_t ffbits, typename bbt>
	friend bool operator<=(const blocktriple<ffbits, bbt>& lhs, const blocktriple<ffbits, bbt>& rhs);
	template<size_t ffbits, typename bbt>
	friend bool operator>=(const blocktriple<ffbits, bbt>& lhs, const blocktriple<ffbits, bbt>& rhs);
};

////////////////////// operators
template<size_t fbits, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<fbits, bt>& v) {
	if (v._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << (long double)v;
	}
	return ostr;
}

template<size_t fbits, typename bt>
inline std::istream& operator>> (std::istream& istr, const blocktriple<fbits, bt>& v) {
	istr >> v._fraction;
	return istr;
}

template<size_t fbits, typename bt>
std::string to_binary(const sw::universal::blocktriple<fbits, bt>& a, bool bNibbleMarker = true) {
	std::stringstream ss;
	return ss.str();
}

template<size_t fbits, typename bt>
inline blocktriple<fbits, bt> operator/(const blocktriple<fbits, bt>& lhs, const blocktriple<fbits, bt>& rhs) {
	return lhs;
}

template<size_t fbits, typename bt>
inline bool operator==(const blocktriple<fbits, bt>& lhs, const blocktriple<fbits, bt>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._nrOfBits == rhs._nrOfBits && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }

template<size_t fbits, typename bt>
inline bool operator!=(const blocktriple<fbits, bt>& lhs, const blocktriple<fbits, bt>& rhs) { return !operator==(lhs, rhs); }

template<size_t fbits, typename bt>
inline bool operator< (const blocktriple<fbits, bt>& lhs, const blocktriple<fbits, bt>& rhs) {
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
					if (lhs._fraction == rhs._fraction) return false; // they are the same value
					if (lhs._fraction > rhs._fraction) {
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
					if (lhs._fraction == rhs._fraction) return false; // they are the same value
					if (lhs._fraction > rhs._fraction) {
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

template<size_t fbits, typename bt>
inline bool operator> (const blocktriple<fbits, bt>& lhs, const blocktriple<fbits, bt>& rhs) { return  operator< (rhs, lhs); }
template<size_t fbits, typename bt>
inline bool operator<=(const blocktriple<fbits, bt>& lhs, const blocktriple<fbits, bt>& rhs) { return !operator> (lhs, rhs); }
template<size_t fbits, typename bt>
inline bool operator>=(const blocktriple<fbits, bt>& lhs, const blocktriple<fbits, bt>& rhs) { return !operator< (lhs, rhs); }

template<size_t fbits, typename bt>
inline std::string components(const blocktriple<fbits, bt>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << "(+,0," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	else if (v.isinf()) {
		s << "(inf," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ')';
	return s.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<size_t fbits, typename bt>
blocktriple<fbits, bt> abs(const blocktriple<fbits, bt>& v) {
	return blocktriple<fbits, bt>(false, v.scale(), v.fraction(), v.iszero());
}

// add two values with fbits fraction bits, round them to abits, and return the abits+1 result value
template<size_t fbits, size_t abits, typename bt>
void module_add(const blocktriple<fbits,bt>& lhs, const blocktriple<fbits,bt>& rhs, blocktriple<abits + 1,bt>& result) {
	// with sign/magnitude adders it is customary to organize the computation 
	// along the four quadrants of sign combinations
	//  + + = +
	//  + - =   lhs > rhs ? + : -
	//  - + =   lhs > rhs ? - : +
	//  - - = 
	// to simplify the result processing assign the biggest 
	// absolute value to R1, then the sign of the result will be sign of the value in R1.

	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

	// align the fractions
	blockbinary<abits,bt> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	blockbinary<abits,bt> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = rhs.sign();
	bool signs_are_different = r1_sign != r2_sign;

	if (signs_are_different && sw::universal::abs(lhs) < sw::universal::abs(rhs)) {
		std::swap(r1, r2);
		std::swap(r1_sign, r2_sign);
	}

	if (signs_are_different) r2 = twos_complement(r2);

	if (_trace_add) {
		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
		if (signs_are_different) {
			std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2 orig  " << twos_complement(r2) << std::endl;
		}
		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
	}

	blockbinary<abits + 1,bt> sum;
	const bool carry = add_unsigned(r1, r2, sum);

	if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

	long shift = 0;
	if (carry) {
		if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
			shift = -1;
		} 
		else {
			// the carry && signs!= implies ||result|| < ||r1||, must find MSB (in the complement)
			for (int i = abits - 1; i >= 0 && !sum[i]; i--) {
				shift++;
			}
		}
	}
	assert(shift >= -1);

	if (shift >= long(abits)) { // we have actual 0                            
		sum.reset();
		result.set(false, 0, sum, true, false, false);
		return;
	}

	scale_of_result -= shift;
	const int hpos = abits - 1 - shift;         // position of the hidden bit 
	sum <<= abits - hpos + 1;
	if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
}

// subtract module: use ADDER
template<size_t fbits, size_t abits, typename bt>
void module_subtract(const blocktriple<fbits,bt>& lhs, const blocktriple<fbits,bt>& rhs, blocktriple<abits + 1,bt>& result) {
	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

	// align the fractions
	blockbinary<abits,bt> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
	blockbinary<abits,bt> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
	bool r1_sign = lhs.sign(), r2_sign = !rhs.sign();
	bool signs_are_different = r1_sign != r2_sign;

	if (sw::universal::abs(lhs) < sw::universal::abs(rhs)) {
		std::swap(r1, r2);
		std::swap(r1_sign, r2_sign);
	}

	if (signs_are_different) r2 = twos_complement(r2);

	if (_trace_sub) {
		std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
		std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
	}

	blockbinary<abits + 1,bt> sum;
	const bool carry = add_unsigned(r1, r2, sum);

	if (_trace_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

	long shift = 0;
	if (carry) {
		if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
			shift = -1;
		}
		else {
			// the carry && signs!= implies r2 is complement, result < r1, must find hidden bit (in the complement)
			for (int i = abits - 1; i >= 0 && !sum[i]; i--) {
				shift++;
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
	const int hpos = abits - 1 - shift;         // position of the hidden bit 
	sum <<= abits - hpos + 1;
	if (_trace_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
	result.set(r1_sign, scale_of_result, sum, false, false, false);
}

// multiply module
template<size_t fbits, size_t mbits, typename bt>
void module_multiply(const blocktriple<fbits,bt>& lhs, const blocktriple<fbits,bt>& rhs, blocktriple<mbits,bt>& result) {
	static constexpr size_t fhbits = fbits + 1;  // fraction + hidden bit
	if (_trace_mul) std::cout << "lhs  " << components(lhs) << std::endl << "rhs  " << components(rhs) << std::endl;

	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	if (lhs.iszero() || rhs.iszero()) {
		result.setzero();
		return;
	}

	bool new_sign = lhs.sign() ^ rhs.sign();
	int new_scale = lhs.scale() + rhs.scale();
	blockbinary<mbits,bt> result_fraction;

	if (fbits > 0) {
		// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
		blockbinary<fhbits,bt> r1 = lhs.get_fixed_point();
		blockbinary<fhbits,bt> r2 = rhs.get_fixed_point();
		multiply_unsigned(r1, r2, result_fraction);

		if (_trace_mul) std::cout << "r1  " << r1 << std::endl << "r2  " << r2 << std::endl << "res " << result_fraction << std::endl;
		// check if the radix point needs to shift
		int shift = 2;
		if (result_fraction.test(mbits - 1)) {
			shift = 1;
			if (_trace_mul) std::cout << " shift " << shift << std::endl;
			new_scale += 1;
		}
		result_fraction <<= shift;    // shift hidden bit out	
	}
	else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
		// multiply the hidden bits together, i.e. 1*1: we know the answer a priori
	}
	if (_trace_mul) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

	result.set(new_sign, new_scale, result_fraction, false, false, false);
}

// divide module
template<size_t fbits, size_t divbits, typename bt>
void module_divide(const blocktriple<fbits,bt>& lhs, const blocktriple<fbits,bt>& rhs, blocktriple<divbits,bt>& result) {
	static constexpr size_t fhbits = fbits + 1;  // fraction + hidden bit
	if (_trace_div) std::cout << "lhs  " << components(lhs) << std::endl << "rhs  " << components(rhs) << std::endl;

	if (lhs.isinf() || rhs.isinf()) {
		result.setinf();
		return;
	}
	if (lhs.iszero() || rhs.iszero()) {
		result.setzero();
		return;
	}

	bool new_sign = lhs.sign() ^ rhs.sign();
	int new_scale = lhs.scale() - rhs.scale();
	blockbinary<divbits,bt> result_fraction;

	if (fbits > 0) {
		// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
		blockbinary<fhbits,bt> r1 = lhs.get_fixed_point();
		blockbinary<fhbits,bt> r2 = rhs.get_fixed_point();
		divide_with_fraction(r1, r2, result_fraction);
		if (_trace_div) std::cout << "r1     " << r1 << std::endl << "r2     " << r2 << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;
		// check if the radix point needs to shift
		// radix point is at divbits - fhbits
		int msb = divbits - fhbits;
		int shift = fhbits;
		if (!result_fraction.test(msb)) {
			msb--; shift++;
			while (!result_fraction.test(msb)) { // search for the first 1
				msb--; shift++;
			}
		}
		result_fraction <<= shift;    // shift hidden bit out
		new_scale -= (shift - fhbits);
		if (_trace_div) std::cout << "shift  " << shift << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;;
	}
	else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
			 // no need to multiply the hidden bits together, i.e. 1*1: we know the answer a priori
	}
	if (_trace_div) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

	result.set(new_sign, new_scale, result_fraction, false, false, false);
}

#endif // LATER

}  // namespace sw::universal
