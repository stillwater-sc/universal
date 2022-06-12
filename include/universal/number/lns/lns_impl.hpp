#pragma once
// lns_impl.hpp: implementation of an arbitrary logarithmic number system configuration
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>

namespace sw { namespace universal {
		
// Forward definitions
template<size_t nbits, size_t rbits, typename bt> class lns;
template<size_t nbits, size_t rbits, typename bt> lns<nbits, rbits, bt> abs(const lns<nbits, rbits, bt>& v);

// convert a floating-point value to a specific lns configuration. Semantically, p = v, return reference to p
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt>& convert(const triple<nbits, bt>& v, lns<nbits, rbits, bt>& p) {
	if (v.iszero()) {
		return p.setnan();
	}
	if (v.isnan() || v.isinf()) {
		return p.setnan();
	}
	return p;
}

template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt>& minpos(lns<nbits, rbits, bt>& lminpos) {
	return lminpos;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& maxpos(lns<nbits, rbits, bt>& lmaxpos) {
	return lmaxpos;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& minneg(lns<nbits, rbits, bt>& lminneg) {
	return lminneg;
}
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt>& maxneg(lns<nbits, rbits, bt>& lmaxneg) {
	return lmaxneg;
}

// template class representing a value in scientific notation, using a template size for the number of fraction bits
template<size_t _nbits, size_t _rbits, typename bt = uint8_t>
class lns {
public:
	static constexpr size_t   nbits = _nbits;
	static constexpr size_t   rbits = _rbits;
	typedef bt BlockType;
	static constexpr double   scaling = double(1ull << rbits);
	static constexpr size_t   bitsInByte = 8ull;
	static constexpr size_t   bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr size_t   nrBlocks = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr size_t   MSU = nrBlocks - 1;
	static constexpr bt       MSU_MASK = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt       SIGN_BIT_MASK = bt(bt(1) << ((nbits - 1ull) % bitsInBlock));
	static constexpr bt       MSB_BIT_MASK = bt(bt(1) << ((nbits - 2ull) % bitsInBlock));
	static constexpr bt       MSB_UNIT = (1 + ((nbits - 2) / bitsInBlock)) - 1;

	/// tivial constructor
	lns() noexcept = default;

	constexpr lns(signed char initial_value)        noexcept { *this = initial_value; }
	constexpr lns(short initial_value)              noexcept { *this = initial_value; }
	constexpr lns(int initial_value)                noexcept { *this = initial_value; }
	constexpr lns(long long initial_value)          noexcept { *this = initial_value; }
	constexpr lns(unsigned long long initial_value) noexcept { *this = initial_value; }
	constexpr lns(float initial_value)              noexcept { *this = initial_value; }
	constexpr lns(double initial_value)             noexcept { *this = initial_value; }

	// assignment operators
	constexpr lns& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr lns& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	CONSTEXPRESSION lns& operator=(float rhs)        noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION lns& operator=(double rhs)       noexcept { return convert_ieee754(rhs); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	lns(long double initial_value)                        noexcept { *this = initial_value; }
	CONSTEXPRESSION lns& operator=(long double rhs)       noexcept { return convert_ieee754(rhs); }
	explicit operator long double()                 const noexcept { return to_long_double(); }
#endif

	// arithmetic operators
	// prefix operator
	lns operator-() const {				
		return *this;
	}

	// in-place arithmetic assignment operators
	lns& operator+=(const lns& rhs) { return *this; }
	lns& operator+=(double rhs) { return *this += lns(rhs); }
	lns& operator-=(const lns& rhs) { return *this; }
	lns& operator-=(double rhs) { return *this -= lns(rhs); }
	lns& operator*=(const lns& rhs) {
		if constexpr (nrBlocks == 1) {
			bool lhsSign = sign();
			bool rhsSign = rhs.sign();
			_block[0] = static_cast<bt>((~SIGN_BIT_MASK & _block[0]) + (~SIGN_BIT_MASK & rhs.block(0)));
			setbit(nbits - 1, lhsSign ^ rhsSign);
			// null any leading bits that fall outside of nbits
			_block[MSU] = static_cast<bt>(MSU_MASK & _block[MSU]);
		}
		else {
			bool lhsSign = sign();
			bool rhsSign = rhs.sign();
			lns<nbits, rbits, BlockType> sum, r(rhs);
			r.setsign(false);
			BlockType* pA = _block;
			BlockType const* pB = r._block;
			BlockType* pC = sum._block;
			BlockType* pEnd = pC + nrBlocks;
			std::uint64_t carry = 0;
			while (pC != pEnd) {
				carry += static_cast<std::uint64_t>(*pA) + static_cast<std::uint64_t>(*pB);
				*pC = static_cast<bt>(carry);
				carry >>= bitsInBlock;
				++pA; ++pB; ++pC;
			}
			setbit(nbits - 1, lhsSign ^ rhsSign);
			// enforce precondition for fast comparison by properly nulling bits that are outside of nbits
			BlockType* pLast = pEnd - 1;
			*pLast = static_cast<bt>(MSU_MASK & *pLast);
			*this = sum;
		}
		return *this;
	}
	lns& operator*=(double rhs) { return *this *= lns(rhs); }
	lns& operator/=(const lns& rhs) {
		return *this;
	}
	lns& operator/=(double rhs) { return *this /= lns(rhs); }

	// prefix/postfix operators
	lns& operator++() {
		return *this;
	}
	lns operator++(int) {
		lns tmp(*this);
		operator++();
		return tmp;
	}
	lns& operator--() {
		return *this;
	}
	lns operator--(int) {
		lns tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear() noexcept { 
		for (size_t i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(0ull);
		}
	}
	constexpr void setzero()                       noexcept { clear(); }
	constexpr void setsign(bool s = true)          noexcept { setbit(nbits - 1, s); }
	constexpr void setbit(size_t i, bool v = true) noexcept {
		if (i < nbits) {
			bt block = _block[i / bitsInBlock];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block[i / bitsInBlock] = bt((block & null) | mask);
		}
		// nop if i is out of range
	}
	constexpr void setbits(uint64_t value) noexcept {
		if constexpr (1 == nrBlocks) {
			_block[0] = value & storageMask;
		}
		else if constexpr (1 < nrBlocks) {
			for (size_t i = 0; i < nrBlocks; ++i) {
				_block[i] = value & storageMask;
				value >>= bitsInBlock;
			}
		}
		_block[MSU] &= MSU_MASK; // enforce precondition for fast comparison by properly nulling bits that are outside of nbits
	}
	
	// selectors
	constexpr bool iszero() const noexcept { // special encoding
		if constexpr (nrBlocks == 1) {
			return ((_block[MSB_UNIT] & MSB_BIT_MASK) > 0);  // +- 0 is possible
		}
		else {
			for (size_t i = 0; i < MSB_UNIT - 1; ++i) {
				if (_block[i] != 0) return false;
			}
			return ((_block[MSB_UNIT] & MSB_BIT_MASK) > 0);
		}
	}
	constexpr bool isneg()  const noexcept { return sign(); }
	constexpr bool ispos()  const noexcept { return !sign(); }
	constexpr bool isinf()  const noexcept { return false; }
	constexpr bool isnan()  const noexcept { // special encoding
		return false; 
	}
	constexpr bool sign()   const noexcept { 
		return (SIGN_BIT_MASK & _block[MSU]) != 0;
	}
	constexpr int  scale()  const noexcept { return false; }

	constexpr bool at(size_t bitIndex) const noexcept {
		if (bitIndex >= nbits) return false; // fail silently as no-op
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	inline constexpr bt block(size_t b) const noexcept {
		if (b < nrBlocks) return _block[b];
		return bt(0); // return 0 when block index out of bounds
	}

	long double to_long_double() const noexcept {
		return 0.0l;
	}
	double to_double() const noexcept {
		return 0.0;
	}
	float to_float() const noexcept {
		return 0.0f;
	}

	explicit operator double() const noexcept { return to_double(); }
	explicit operator float() const noexcept { return to_float(); }

protected:

	template<typename SignedInt>
	constexpr lns& convert_signed(SignedInt v) {
		clear();
		return *this;
	}
	template<typename UnsignedInt>
	constexpr lns& convert_unsigned(UnsignedInt v) {
		clear();
		return *this;
	}
	template<typename Real>
	CONSTEXPRESSION lns& convert_ieee754(Real v) {
		clear();
		return *this;
	}

private:
	BlockType  _block[nrBlocks];

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const lns<nnbits, rrbits, nbt>& r);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, lns<nnbits, rrbits, nbt>& r);

	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator==(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator< (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator> (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
	template<size_t nnbits, size_t rrbits, typename nbt>
	friend bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs);
};

////////////////////// operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const lns<nnbits, rrbits, nbt>& v) {
	ostr << v.to_double();
	return ostr;
}

template<size_t nnbits, size_t rrbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, const lns<nnbits, rrbits, nbt>& v) {
	istr >> v._fraction;
	return istr;
}

// lns - logic operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator==(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) {
	using LNS = lns<nnbits, rrbits, nbt>;
	if constexpr (LNS::nrBlocks == 1) {
		return lhs._block[0] == rhs._block[0];
	}
	if constexpr (LNS::nrBlocks == 2) {
		return (lhs._block[0] == rhs._block[0]) && 
			   (lhs._block[1] == rhs._block[1]);
	}
	if constexpr (LNS::nrBlocks == 3) {
		return (lhs._block[0] == rhs._block[0]) &&
			   (lhs._block[1] == rhs._block[1]) &&
			   (lhs._block[2] == rhs._block[2]);
	}
	if constexpr (LNS::nrBlocks == 4) {
		return (lhs._block[0] == rhs._block[0]) &&
			   (lhs._block[1] == rhs._block[1]) &&
			   (lhs._block[2] == rhs._block[2]) &&
			   (lhs._block[3] == rhs._block[3]);
	}
	else {
		for (size_t i = 0; i < LNS::nrBlocks; ++i) {
			if (lhs.block(i) != rhs.block(i)) return false;
		}
		return true;
	}
}
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator< (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) {
	using LNS = lns<nnbits, rrbits, nbt>;
	bool lhsSign = lhs.at(nnbits - 1);
	bool rhsSign = rhs.at(nnbits - 1);
	if (lhsSign) {
		if (rhsSign) {
			LNS l(lhs);
			l.setbit(nnbits - 1, false);
			LNS r(rhs);
			r.setbit(nnbits - 1, false);
		}
		else {
			return true;
		}
	}
	else {
		if (rhsSign) {
			return false;
		}
		else {

		}
	}
	return false; 
}
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator> (const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, const lns<nnbits, rrbits, nbt>& rhs) { return !operator< (lhs, rhs); }

// lns - literal logic operators
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator==(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return lhs == lns<nnbits, rrbits, nbt>(rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator!=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator==(lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator< (const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return lhs < lns<nnbits, rrbits, nbt>(rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator> (const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return  operator< (rhs, lhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator<=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator> (lhs, rhs); }
template<size_t nnbits, size_t rrbits, typename nbt>
inline bool operator>=(const lns<nnbits, rrbits, nbt>& lhs, double rhs) { return !operator< (lhs, rhs); }

// lns - lns binary arithmetic operators
// BINARY ADDITION
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator+(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator-(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator*(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<size_t nbits, size_t rbits, typename bt>
inline lns<nbits, rbits, bt> operator/(const lns<nbits, rbits, bt>& lhs, const lns<nbits, rbits, bt>& rhs) {
	lns<nbits, rbits, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

template<size_t nbits, size_t rbits, typename bt>
inline std::string to_binary(const lns<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	s << (number.sign() ? "1." : "0.");
	for (int i = static_cast<int>(nbits) - 2; i >= rbits; --i) {
		s << (number.at(static_cast<size_t>(i)) ? '1' : '0');
		if ((i - rbits) > 0 && ((i - rbits) % 4) == 0 && nibbleMarker) s << '\'';
	}
	if constexpr (rbits > 0) {
		s << '.';
		for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
			s << (number.at(static_cast<size_t>(i)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	return s.str();
}

template<size_t nbits, size_t rbits, typename bt>
inline std::string components(const lns<nbits, rbits, bt>& v) {
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

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> abs(const lns<nbits, rbits, bt>& v) {
	return lns<nbits, rbits, bt>();
}


}} // namespace sw::universal
