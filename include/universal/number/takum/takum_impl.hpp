#pragma once
// takum_impl.hpp: definition of a arbitrary, fixed-size takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>

namespace sw {	namespace universal {
		
// Forward definitions
template<unsigned nbits, unsigned es, typename bt> class takum;

// convert a floating-point value to a specific takum configuration. Semantically, p = v, return reference to p
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt>& convert(const triple<nbits, bt>& v, takum<nbits, es, bt>& p) {
	if (v.iszero()) {
		p.setzero();
		return p;
	}
	if (v.isnan() || v.isinf()) {
		p.setnan();
		return p;
	}
	return p;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<unsigned _nbits, unsigned _es, typename bt = uint8_t>
class takum {
public:
	typedef bt BlockType;

	static constexpr unsigned nbits = _nbits;
	static constexpr unsigned es = _es;

	static constexpr unsigned bitsInByte = 8ull;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr unsigned MSU = nrBlocks - 1;
	static constexpr bt       MSU_MASK = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt       SIGN_BIT_MASK = bt(1ull << ((nbits - 1ull) % bitsInBlock));
	static constexpr unsigned MSB_UNIT = (1ull + ((nbits - 2) / bitsInBlock)) - 1ull;
	static constexpr bt       MSB_BIT_MASK = bt(1ull << ((nbits - 2ull) % bitsInBlock));

	using BlockBinary = blockbinary<nbits, bt, BinaryNumberType::Unsigned>;

	/// trivial constructor
	takum() = default;

	takum(const takum&) = default;
	takum(takum&&) = default;

	takum& operator=(const takum&) = default;
	takum& operator=(takum&&) = default;

	// specific value constructor
	constexpr takum(const SpecificValue code) noexcept
		: _block{} {
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
		case SpecificValue::infneg:
		case SpecificValue::nar:
		case SpecificValue::qnan:
		case SpecificValue::snan:
			setnar();
			break;
		}
	}

	constexpr takum(signed char initial_value)         noexcept : _block{} { *this = initial_value; }
	constexpr takum(short initial_value)               noexcept : _block{} { *this = initial_value; }
	constexpr takum(int initial_value)                 noexcept : _block{} { *this = initial_value; }
	constexpr takum(long long initial_value)           noexcept : _block{} { *this = initial_value; }
	constexpr takum(unsigned long long initial_value)  noexcept : _block{} { *this = initial_value; }
	constexpr takum(float initial_value)               noexcept : _block{} { *this = initial_value; }
	constexpr takum(double initial_value)              noexcept : _block{} { *this = initial_value; }

	// assignment operators
	constexpr takum& operator=(signed char rhs)        noexcept { return *this = (long long)(rhs); }
	constexpr takum& operator=(short rhs)              noexcept { return *this = (long long)(rhs); }
	constexpr takum& operator=(int rhs)                noexcept { return *this = (long long)(rhs); }
	constexpr takum& operator=(long long rhs)          noexcept { return *this; }
	constexpr takum& operator=(unsigned long long rhs) noexcept { return *this; }
	CONSTEXPRESSION takum& operator=(float rhs)        noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION takum& operator=(double rhs)       noexcept { return convert_ieee754(rhs); }

	explicit constexpr operator int()       const noexcept { return to_signed<int>(); }
	explicit constexpr operator long()      const noexcept { return to_signed<long>(); }
	explicit constexpr operator long long() const noexcept { return to_signed<long long>(); }
	explicit constexpr operator float()     const noexcept { return to_ieee754<float>(); }
	explicit constexpr operator double()    const noexcept { return to_ieee754<double>(); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	takum(long double initial_value)                      noexcept { *this = initial_value; }
	CONSTEXPRESSION takum& operator=(long double rhs)     noexcept { return convert_ieee754(rhs); }
	explicit constexpr operator long double()       const noexcept { return to_ieee754<long double>(); }
#endif

	// arithmetic operators
	// prefix operator
	takum operator-() const {				
		return *this;
	}

	// in-place arithmetic assignment operators
	takum& operator+=(const takum& rhs) { return *this; }
	takum& operator+=(double rhs) { return *this += takum(rhs); }
	takum& operator-=(const takum& rhs) { return *this; }
	takum& operator-=(double rhs) { return *this -= takum(rhs); }
	takum& operator*=(const takum& rhs) { return *this; }
	takum& operator*=(double rhs) { return *this *= takum(rhs); }
	takum& operator/=(const takum& rhs) { return *this; }
	takum& operator/=(double rhs) { return *this /= takum(rhs); }

	// prefix/postfix operators
	takum& operator++() {
		return *this;
	}
	takum operator++(int) {
		takum tmp(*this);
		operator++();
		return tmp;
	}
	takum& operator--() {
		return *this;
	}
	takum operator--(int) {
		takum tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear()                         noexcept { _block.clear(); }
	constexpr void setzero()                       noexcept { _block.clear(); setbit(nbits - 2, true); }
	constexpr void setnar(bool sign = false)       noexcept { _block.clear(); setbit(nbits - 1); }
	constexpr void setnan(bool sign = false)       noexcept { _block.clear(); setbit(nbits - 1); setbit(nbits - 2); }
	constexpr void setinf(bool sign)               noexcept { (sign ? maxneg() : maxpos()); } // TODO: is that what we want?
	constexpr void setsign(bool s = true)          noexcept { setbit(nbits - 1, s); }
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned blockIndex = i / bitsInBlock;
		if (i < nbits) {
			bt block = _block[blockIndex];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			//_block[i / bitsInBlock] = bt((block & null) | mask);
			_block.setblock(blockIndex, bt((block & null) | mask));
		}
		// nop if i is out of range
	}
	constexpr void setbits(uint64_t value) noexcept {
		if constexpr (1 == nrBlocks) {
			_block.setblock(0, value & storageMask);
		}
		else if constexpr (1 < nrBlocks) {
			for (unsigned i = 0; i < nrBlocks; ++i) {
				_block.setblock(i, value & storageMask);
				value >>= bitsInBlock;
			}
		}
		_block.setblock(MSU, static_cast<bt>(_block[MSU] & MSU_MASK)); // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	}

	// create specific number system values of interest
	constexpr takum& maxpos() noexcept {
		// maximum positive value has this bit pattern: 0-01..1-111...111, that is, sign = 0, integer = 01..11, fraction = 11..11
		clear();
		flip();
		setbit(nbits - 1ull, false); // sign = 0
		setbit(nbits - 2ull, false); // msb  = 0
		return *this;
	}
	constexpr takum& minpos() noexcept {
		// minimum positive value has this bit pattern: 0-100-00...01, that is, sign = 0, integer = 10..00, fraction = 00..01
		clear();
		setbit(nbits - 2, true);    // msb  = 1
		setbit(0, true);            // lsb  = 1
		return *this;
	}
	constexpr takum& zero() noexcept {
		// the zero value has this bit pattern: 0-100..00-00..000, sign = 0, msb = 1, rest 0
		clear();
		setbit(nbits - 2, true);    // msb = 1
		return *this;
	}
	constexpr takum& minneg() noexcept {
		// minimum negative value has this bit pattern: 1-100-00...01, that is, sign = 1, integer = 10..00, fraction = 00..01
		clear();
		setbit(nbits - 1ull, true); // sign = 1
		setbit(nbits - 2, true);    // msb  = 1
		setbit(0, true);            // lsb  = 1
		return *this;
	}
	constexpr takum& maxneg() noexcept {
		// maximum negative value has this bit pattern: 1-01..1-11..11, that is, sign = 1, integer = 01..1, fraction = 11..11
		clear();
		flip();
		setbit(nbits - 2ull, false); // msb  = 0
		return *this;
	}

	// selectors
	constexpr bool iszero() const noexcept { return _block.iszero(); }
	constexpr bool isneg()  const noexcept { return _block.sign(); }
	constexpr bool ispos()  const noexcept { return !_block.sign(); }
	constexpr bool isinf()  const noexcept { return false; }
	constexpr bool isnan()  const noexcept { return false; }
	constexpr bool isnar()  const noexcept { return false; }
	constexpr bool sign()   const noexcept { return _block.sign(); }
	constexpr int scale()   const noexcept { return false; }
	inline std::string get() const noexcept { return std::string("tbd"); }


	long double to_long_double() const {
		return 0.0l;
	}
	double to_double() const {
		return 0.0;
	}
	float to_float() const {
		return 0.0f;
	}

protected:

		/// <summary>
		/// 1's complement of the encoding. Used internally to create specific bit patterns
		/// </summary>
		/// <returns>reference to this cfloat object</returns>
		constexpr takum& flip() noexcept { // in-place one's complement
			for (unsigned i = 0; i < nrBlocks; ++i) {
				//_block[i] = bt(~_block[i]);
				_block.setblock(i, bt(~_block[i]));
			}
			//_block[MSU] &= MSU_MASK; // assert precondition of properly nulled leading non-bits
			_block.setblock(MSU, bt(_block[MSU] & MSU_MASK));
			return *this;
		}

		/// <summary>
		/// assign the value of the string representation to the cfloat
		/// </summary>
		/// <param name="stringRep">decimal scientific notation of a real number to be assigned</param>
		/// <returns>reference to this cfloat</returns>
		/// Clang doesn't support constexpr yet on string manipulations, so we need to make it conditional
		CONSTEXPRESSION takum& assign(const std::string& str) noexcept {
			clear();
			return *this;
		}

		//////////////////////////////////////////////////////
		/// convertion routines from native types

		template<typename SignedInt>
		CONSTEXPRESSION takum& convert_signed(SignedInt v) noexcept {
			return convert_ieee754(double(v));
		}
		template<typename UnsignedInt>
		CONSTEXPRESSION takum& convert_unsigned(UnsignedInt v) noexcept {
			return convert_ieee754(double(v));
		}
		template<typename Real>
		CONSTEXPRESSION takum& convert_ieee754(Real v) noexcept {


			return *this;
		}

		//////////////////////////////////////////////////////
		/// convertion routines to native types

		template<typename SignedInt>
		typename std::enable_if< std::is_integral<SignedInt>::value&& std::is_signed<SignedInt>::value, SignedInt>::type
			to_signed() const {
			return SignedInt(to_ieee754<double>());
		}
		template<typename UnsignedInt>
		typename std::enable_if< std::is_integral<UnsignedInt>::value&& std::is_unsigned<UnsignedInt>::value, UnsignedInt>::type
			to_unsigned() const {
			return UnsignedInt(to_ieee754<double>());
		}
		template<typename TargetFloat>
		CONSTEXPRESSION TargetFloat to_ieee754() const noexcept {   // TODO: don't use bit math, use proper limb math to speed this up
			TargetFloat value{ 0 };
			bool negative = sign();
			return (negative ? -value : value);
		}

private:
	BlockBinary _block;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const takum<nnbits, nes, nbt>& r);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend std::istream& operator>> (std::istream& istr, takum<nnbits, nes, nbt>& r);

	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator==(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator!=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator< (const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator> (const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator<=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
	template<unsigned nnbits, unsigned nes, typename nbt>
	friend bool operator>=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs);
};

// return the Unit in the Last Position
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> ulp(const takum<nbits, es, bt>& a) {
	takum<nbits, es, bt> b(a);
	return ++b - a;
}

template<unsigned nbits, unsigned es, typename bt>
std::string to_binary(const takum<nbits, es, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	s << (number.sign() ? "1." : "0.");

	return s.str();
}

////////////////////// operators
template<unsigned nnbits, unsigned nes, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const takum<nnbits, nes, nbt>& v) {

	return ostr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline std::istream& operator>>(std::istream& istr, const takum<nnbits, nes, nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator==(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return false; }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator!=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator< (const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return false; }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator> (const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator<=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nnbits, unsigned nes, typename nbt>
inline bool operator>=(const takum<nnbits, nes, nbt>& lhs, const takum<nnbits, nes, nbt>& rhs) { return !operator< (lhs, rhs); }

// takum - takum binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> operator+(const takum<nbits, es, bt>& lhs, const takum<nbits, es, bt>& rhs) {
	takum<nbits, es, bt> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> operator-(const takum<nbits, es, bt>& lhs, const takum<nbits, es, bt>& rhs) {
	takum<nbits, es, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> operator*(const takum<nbits, es, bt>& lhs, const takum<nbits, es, bt>& rhs) {
	takum<nbits, es, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es, typename bt>
inline takum<nbits, es, bt> operator/(const takum<nbits, es, bt>& lhs, const takum<nbits, es, bt>& rhs) {
	takum<nbits, es, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<unsigned nbits, unsigned es, typename bt>
inline std::string components(const takum<nbits, es, bt>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << " zero b" << std::setw(nbits) << v.fraction();
		return s.str();
	}
	else if (v.isinf()) {
		s << " infinite b" << std::setw(nbits) << v.fraction();
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
	return s.str();
}

/*
/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned nbits, unsigned es, typename bt>
constexpr takum<nbits, es, bt> abs(const takum<nbits, es, bt>& v) {
	return takum<nbits>();
}
*/

}}  // namespace sw::universal
