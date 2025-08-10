#pragma once
// takum_impl.hpp: definition of a arbitrary, fixed-size takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>

namespace sw {	namespace universal {
		
// Forward definitions
template<unsigned nbits, typename bt> class takum;

// convert a floating-point value to a specific takum configuration. Semantically, p = v, return reference to p
template<unsigned nbits, typename bt>
inline takum<nbits, bt>& convert(const triple<nbits, bt>& v, takum<nbits, bt>& p) {
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
template<unsigned _nbits, typename bt = uint8_t>
class takum {
	static_assert(_nbits > 4, "takum requires at least 5 bits");
public:
	typedef bt BlockType;

	static constexpr unsigned nbits = _nbits;

	static constexpr unsigned bitsInByte = 8ull;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr unsigned bitsInMSU = (1 + ((nbits - 1) % bitsInBlock));
	static constexpr bool     MSU_CONTAINS_REGIME = (bitsInMSU > 4);
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr unsigned MSU = nrBlocks - 1;
	static constexpr bt       MSU_MASK = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt       SIGN_BIT_MASK = bt(1ull << ((nbits - 1ull) % bitsInBlock));
	static constexpr bt       DIRECTION_BIT_MASK = bt(1ull << ((nbits - 2ull) % bitsInBlock));
	static constexpr unsigned regimeFieldShift = (bitsInMSU > 4 ? (bitsInMSU - 5) : 0);
	static constexpr bt       REGIME_FIELD_MASK = bt(0x7ll << regimeFieldShift);
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
	constexpr takum& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr takum& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr takum& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr takum& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr takum& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr takum& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	constexpr takum& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr takum& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr takum& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr takum& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
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
	takum& operator=(long double rhs)                     noexcept { return convert_ieee754(rhs); }
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
	constexpr void setzero()                       noexcept { _block.clear(); }
	constexpr void setnar()                        noexcept { _block.clear(); setbit(nbits - 1); }
	constexpr void setnan(bool sign = false)       noexcept { _block.clear(); (sign ? setbit(nbits - 1) : setbit(nbits - 1)); }
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
	constexpr bool iszero()    const noexcept { return _block.iszero(); }
	constexpr bool isneg()     const noexcept { return _block.sign(); }
	constexpr bool ispos()     const noexcept { return !_block.sign(); }
	constexpr bool isinf()     const noexcept { return false; }
	constexpr bool isnan()     const noexcept { return false; }
	constexpr bool isnar()     const noexcept {
		BlockBinary tmp(_block);
		if (tmp.test(nbits - 1)) {
			tmp.reset(nbits - 1);
			return tmp.iszero();
		}
		return false; 
	}
	constexpr bool sign()      const noexcept { return _block.sign(); }
	constexpr bool direct()    const noexcept { return _block.test(nbits - 2); }
	constexpr int  scale()     const noexcept { return -100; }
	constexpr unsigned regime()    const noexcept {
		unsigned r{ 0 };
		if constexpr (nrBlocks == 1) {
			bt msu = _block[MSU];
			r = static_cast<unsigned>((msu & REGIME_FIELD_MASK) >> regimeFieldShift);
		}
		else {
			if constexpr (MSU_CONTAINS_REGIME) {
				bt msu = _block[MSU];
				r = static_cast<unsigned>((msu & REGIME_FIELD_MASK) >> regimeFieldShift);
			}
			else {
				r = 0;
			}
		}
		return r; 
	}
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false; // fail silently as no-op
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	constexpr bt block(unsigned b) const noexcept {
		if (b < nrBlocks) return _block[b];
		return bt(0); // return 0 when block index out of bounds
	}
	constexpr uint8_t nibble(unsigned n) const noexcept {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			int nibbleIndexInWord = int(n % (bitsInBlock >> 2ull));
			bt mask = bt(0xF << (nibbleIndexInWord * 4));
			bt nibblebits = bt(mask & word);
			return uint8_t(nibblebits >> (nibbleIndexInWord * 4));
		}
		return false;
	}


	inline std::string get()   const noexcept { return std::string("tbd"); }


	void debugConstexprParameters() {
		std::cout << "constexpr parameters for " << type_tag(*this) << '\n';
		std::cout << "bitsInByte            " << bitsInByte << '\n';
		std::cout << "bitsInBlock           " << bitsInBlock << '\n';
		std::cout << "nrBlocks              " << nrBlocks << '\n';
		std::cout << "bitsInMSU             " << bitsInMSU << '\n';
		std::cout << "storageMask           " << to_binary(storageMask, bitsInBlock) << '\n';
		std::cout << "MSU                   " << MSU << '\n';
		std::cout << "MSU_MASK              " << to_binary(MSU_MASK, bitsInBlock) << '\n';
		std::cout << "MSB_UNIT              " << MSB_UNIT << '\n';
		std::cout << "MSU_CONTAINS_REGIME   " << (MSU_CONTAINS_REGIME ? "yes" : "no") << '\n';
		std::cout << "SIGN_BIT_MASK         " << to_binary(SIGN_BIT_MASK, bitsInBlock) << '\n';
		std::cout << "DIRECTION_BIT_MASK    " << to_binary(DIRECTION_BIT_MASK, bitsInBlock) << '\n';
		std::cout << "REGIME_FIELD_MASK     " << to_binary(REGIME_FIELD_MASK, bitsInBlock) << '\n';
		std::cout << "MSB_BIT_MASK          " << to_binary(MSB_BIT_MASK, bitsInBlock) << '\n';
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
		CONSTEXPRESSION takum& convert_signed(SignedInt rhs) noexcept {
			return convert_ieee754(double(rhs));
		}
		template<typename UnsignedInt>
		CONSTEXPRESSION takum& convert_unsigned(UnsignedInt rhs) noexcept {
			return convert_ieee754(double(rhs));
		}
		template<typename Real>
		CONSTEXPRESSION takum& convert_ieee754(Real rhs) noexcept {
			using std::log2;
			using std::floor;
			using sw::universal::scale;
			bool s{ false };
			uint64_t rawExponent{ 0 };
			uint64_t rawFraction{ 0 };
			uint64_t bits{ 0 };
			extractFields(rhs, s, rawExponent, rawFraction, bits);
			if (rawExponent == ieee754_parameter<Real>::eallset) { // nan and inf need to be remapped
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::snanmask) ||
					rawFraction == (ieee754_parameter<Real>::fmask & (ieee754_parameter<Real>::qnanmask | ieee754_parameter<Real>::snanmask))) {
					// 1.11111111.00000000.......00000001 signalling nan
					// 0.11111111.00000000000000000000001 signalling nan
					// MSVC
					// 1.11111111.10000000.......00000001 signalling nan
					// 0.11111111.10000000.......00000001 signalling nan
					setnar();
					return *this;
				}
				if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask)) {
					// 1.11111111.10000000.......00000000 quiet nan
					// 0.11111111.10000000.......00000000 quiet nan
					setnar();
					return *this;
				}
				if (rawFraction == 0ull) {
					// 1.11111111.0000000.......000000000 -inf
					// 0.11111111.0000000.......000000000 +inf
					(s ? maxneg() : maxpos());
					return *this;
				}
			}

			if (rhs == 0.0) {
				setzero();
				return *this;
			}

			// convert input to double
			double v{ rhs };
			uint64_t S = s ? 1 : 0;
			// bool d{ false };
			uint64_t D{ 0 }, R{ 0 };
			uint64_t r{ 0 };
			long a{ 0 }; //, b{ 0 };
			long h = scale(v);
			double fs = log2(1.0 + fraction(v));
			double l = h + fs;
			long amb = static_cast<int>(floor((s ? -1 : 1) * l));
			if (amb >= 0) {
				D = 1;
				r = static_cast<int>(floor(log2(amb + 1)));
				R = r;
				a = amb;
			}
			else {
				D = 0;
				r = static_cast<int>(floor(log2(-amb)));
				R = 7 - r;
				a = amb + 3 * (1ul << r) - 2;
			}
			uint64_t A = a - (1ul << r) + 1ul;
			double f = (s ? -1 : 1) * l - amb;
			int m = nbits - 5 - r;
			uint64_t F = static_cast<uint64_t>((1ull << m) * f);

			// compose the takum
			static_assert(nbits < 64, "requested takum straddles multiple uint64_ts");
			uint64_t raw{ 0 };
			raw |= (S << (nbits - 1));
			raw |= (D << (nbits - 2));
			raw |= (R << (nbits - 5));
			raw |= (A << (nbits - 5 - r));
			raw |= (F);
			setbits(raw);

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
		CONSTEXPRESSION TargetFloat to_ieee754() const noexcept {
			TargetFloat value{ 0 };
			if (iszero()) return value;
			if (isnar()) return std::numeric_limits<TargetFloat>::quiet_NaN();

			bool negative, direction;
			unsigned regime, r;
			if constexpr (nrBlocks == 1) {
				bt msu = _block[MSU];
				negative = (msu & SIGN_BIT_MASK);
				direction = (msu & DIRECTION_BIT_MASK);
				//std::cout << "bitsInMSU     : " << bitsInMSU << '\n';
				regime = static_cast<unsigned>((msu & REGIME_FIELD_MASK) >> (bitsInMSU - 5));
				r = direction ? regime :  7 - regime;
				unsigned m = (r > nbits - 5) ? 0 : nbits - 5 - r;
				// construct the exponent field mask
				//std::cout << "regime        : " << int(regime) << '\n';
				//std::cout << "r             : " << r << '\n';
				//std::cout << "m             : " << m << '\n';
				bt exponentFieldMask = static_cast<bt>((r > 0 ? (0xFFFF'FFFF'FFFF'FFFFull >> (64 - r)) : 0));
				//std::cout << to_binary(exponentFieldMask) << '\n';
				exponentFieldMask <<= m;
				//std::cout << to_binary(exponentFieldMask) << '\n';
				bt A = static_cast<bt>((msu & exponentFieldMask) >> m);
				//std::cout << "A             : " << int(A) << '\n';
				TargetFloat a = static_cast<TargetFloat>((1ull << r) - 1ull + A);
				//std::cout << "a             : " << a << '\n';
				TargetFloat b = static_cast<TargetFloat>(direction ? 0 : (3*(1ull << r) - 2ull));
				//std::cout << "b             : " << b << '\n';
				TargetFloat s = (negative ? 1.0f : 0.0f);
				TargetFloat e = (1.0f - 2.0f * s) * (a - b + s);
				//std::cout << "e             : " << e << '\n';

				bt fractionFieldMask = static_cast<bt>(0xFFFF'FFFF'FFFF'FFFFull >> (64 - m));
				//std::cout << to_binary(fractionFieldMask) << '\n';
				bt fraction = static_cast<bt>(msu & fractionFieldMask);
				//std::cout << to_binary(fraction) << '\n';
				TargetFloat f = 0.0f;
				TargetFloat bitValue = 0.5f;
				bt bitMask = static_cast<bt>(1ull << (m - 1));
				for (unsigned i = 0; i < m; ++i) {
					f += (fraction & bitMask) ? bitValue : 0.0f;
					bitMask >>= 1;
					bitValue *= 0.5f;
				}
				//std::cout << "f             : " << f << '\n';
				value = ((1 - 3 * s) + f) * std::exp2(e);
			}
			else {
				if constexpr (MSU_CONTAINS_REGIME) {
					bt msu = _block[MSU];
					negative = (msu & SIGN_BIT_MASK);
					direction = (msu & DIRECTION_BIT_MASK);
					regime = static_cast<uint8_t>((msu & REGIME_FIELD_MASK) >> (bitsInMSU - 5));
				}
				else {
					negative = sign();
					direction = direct();
					regime = 0;
				}
			}

			return value;
		}

private:
	BlockBinary _block;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const takum<nnbits, nbt>& r);
	template<unsigned nnbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, takum<nnbits, nbt>& r);

	template<unsigned nnbits, typename nbt>
	friend bool operator==(const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator!=(const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator< (const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator> (const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator<=(const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs);
	template<unsigned nnbits, typename nbt>
	friend bool operator>=(const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs);
};

// return the Unit in the Last Position
template<unsigned nbits, typename bt>
inline takum<nbits, bt> ulp(const takum<nbits, bt>& a) {
	takum<nbits, bt> b(a);
	return ++b - a;
}

template<unsigned nbits, typename bt>
std::string to_binary(const takum<nbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	bool D = number.direct();
	s << "0b";
	s << (number.sign() ? "1." : "0.");
	s << (number.direct() ? "1." : "0.");
	int bit = static_cast<int>(nbits) - 3;
	for (int i = 0; (i < 3) && (bit >= 0); ++i) {
		s << (number.at(static_cast<unsigned>(bit--)) ? '1' : '0');
	}
	s << '.';
	unsigned regime = number.regime();
	int r = static_cast<int>(D ? regime : 7 - regime);
	// exponent field
	for (int i = r - 1; i >= 0 && bit >= 0; --i) {
		s << (number.at(static_cast<unsigned>(bit--)) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}
	// fraction field
	s << '.';
	while (bit >= 0) {
		s << (number.at(static_cast<unsigned>(bit)) ? '1' : '0');
		if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
		--bit;

	}
	return s.str();
}

////////////////////// operators
template<unsigned nnbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const takum<nnbits, nbt>& v) {
	ostr << double(v);
	return ostr;
}

template<unsigned nnbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, const takum<nnbits, nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nnbits, typename nbt>
inline bool operator==(const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs) {
	return (lhs._block == rhs._block);
}
template<unsigned nnbits, typename nbt>
inline bool operator!=(const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, typename nbt>
inline bool operator< (const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs) {
	std::cerr << "operator<() TBD\n";
	return false; 
}
template<unsigned nnbits, typename nbt>
inline bool operator> (const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, typename nbt>
inline bool operator<=(const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nnbits, typename nbt>
inline bool operator>=(const takum<nnbits, nbt>& lhs, const takum<nnbits, nbt>& rhs) { return !operator< (lhs, rhs); }

// takum - takum binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, typename bt>
inline takum<nbits, bt> operator+(const takum<nbits, bt>& lhs, const takum<nbits, bt>& rhs) {
	takum<nbits, bt> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, typename bt>
inline takum<nbits, bt> operator-(const takum<nbits, bt>& lhs, const takum<nbits, bt>& rhs) {
	takum<nbits, bt> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, typename bt>
inline takum<nbits, bt> operator*(const takum<nbits, bt>& lhs, const takum<nbits, bt>& rhs) {
	takum<nbits, bt> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, typename bt>
inline takum<nbits, bt> operator/(const takum<nbits, bt>& lhs, const takum<nbits, bt>& rhs) {
	takum<nbits, bt> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<unsigned nbits, typename bt>
inline std::string components(const takum<nbits, bt>& v) {
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
template<unsigned nbits, typename bt>
constexpr takum<nbits, bt> abs(const takum<nbits, bt>& v) {
	return takum<nbits>();
}
*/

}}  // namespace sw::universal
