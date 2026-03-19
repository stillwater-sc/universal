#pragma once
// takum_impl.hpp: implementation of a linear takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Linear takum encoding (Hunhold, 2024, arXiv:2404.18603):
//
// Bit layout (of the magnitude after removing the sign):
//   [S:1][D:1][R:rbits][C:r bits][M:p bits]
//   where DR = (D << rbits) | R is a (1+rbits)-bit field,
//         r = dr_to_r(DR) gives the number of characteristic bits,
//         p = nbits - overhead - r gives the number of mantissa bits.
//
// Template parameters:
//   nbits  - total number of bits
//   rbits  - number of regime bits (default 3 per the takum spec)
//   bt     - storage block type (default uint8_t)
//
// Two's complement storage:
//   Zero = 0x00...0   NaR = 0x80...0
//   Negating the integer negates the represented value.
//   Comparison of the signed integer equals comparison of the real value.
//
// Linear takum value formula:
//   value = (-1)^sign * (1 + f) * 2^c

#include <cassert>
#include <limits>
#include <cmath>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>

namespace sw {	namespace universal {

// Forward definitions
template<unsigned nbits, unsigned rbits, typename bt> class takum;

// convert a floating-point value to a specific takum configuration
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt>& convert(const triple<nbits, bt>& v, takum<nbits, rbits, bt>& p) {
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

// template class representing a takum value with two's complement encoding
template<unsigned _nbits, unsigned _rbits = 3, typename bt = uint8_t>
class takum {
	static constexpr unsigned _overhead = 2 + _rbits;  // S:1 + D:1 + R:rbits
	static_assert(_nbits > _overhead, "takum requires more bits than the fixed overhead (S + D + R)");
	static_assert(_rbits > 0, "takum requires at least 1 regime bit");
	static_assert(_rbits <= 7, "takum regime field limited to 7 bits");
public:
	typedef bt BlockType;

	static constexpr unsigned nbits    = _nbits;
	static constexpr unsigned rbits    = _rbits;
	static constexpr unsigned overhead = _overhead;  // S + D + R field width

	// DR field properties
	static constexpr unsigned dr_bits       = 1 + rbits;           // D:1 + R:rbits
	static constexpr unsigned nr_dr_values  = 1u << dr_bits;       // number of DR combinations
	static constexpr unsigned max_r         = (1u << rbits) - 1;   // maximum characteristic bit count
	static constexpr unsigned r_mask        = max_r;               // mask for the R field
	static constexpr unsigned dr_field_mask = (1u << dr_bits) - 1; // mask for the DR field

	// Storage parameters
	static constexpr unsigned bitsInByte  = 8ull;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks    = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr unsigned bitsInMSU   = (1 + ((nbits - 1) % bitsInBlock));
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr unsigned MSU         = nrBlocks - 1;
	static constexpr bt       MSU_MASK    = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt       SIGN_BIT_MASK      = bt(1ull << ((nbits - 1ull) % bitsInBlock));
	static constexpr bt       DIRECTION_BIT_MASK  = bt(1ull << ((nbits - 2ull) % bitsInBlock));

	// Maximum characteristic bits available for this nbits
	static constexpr unsigned maxCharBits = (nbits > overhead) ? (nbits - overhead) : 0;

	// Constexpr functions replacing hardcoded lookup tables
	// Given DR index, return the number of characteristic bits (r)
	static constexpr unsigned dr_to_r(unsigned dr) noexcept {
		bool D = (dr >> rbits) & 1;
		unsigned R = dr & r_mask;
		return D ? R : (max_r - R);
	}
	// Given DR index, return the characteristic bias
	static constexpr int dr_to_c_bias(unsigned dr) noexcept {
		bool D = (dr >> rbits) & 1;
		unsigned r = dr_to_r(dr);
		// D=1: c_bias = 2^r - 1 (positive range start)
		// D=0: c_bias = -(2^(r+1)) + 1 (negative range start)
		return D ? (static_cast<int>((1ull << r) - 1))
		         : (1 - static_cast<int>(1ull << (r + 1)));
	}
	// Given a characteristic value c, find the DR index
	static unsigned find_dr(int c) noexcept {
		for (int dr = static_cast<int>(nr_dr_values) - 1; dr >= 0; --dr) {
			if (c >= dr_to_c_bias(static_cast<unsigned>(dr)))
				return static_cast<unsigned>(dr);
		}
		return 0;
	}
	// Maximum representable characteristic value
	static constexpr int max_characteristic() noexcept {
		// DR = nr_dr_values - 1 (D=1, R=max_r): c_bias + (2^max_r - 1)
		return dr_to_c_bias(nr_dr_values - 1) + static_cast<int>((1ull << max_r) - 1);
	}
	// Minimum representable characteristic value
	static constexpr int min_characteristic() noexcept {
		// DR = 0 (D=0, R=0): c_bias = -(2^(max_r+1)) + 1
		return dr_to_c_bias(0);
	}

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
	explicit operator float()     const noexcept { return to_ieee754<float>(); }
	explicit operator double()    const noexcept { return to_ieee754<double>(); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	takum(long double initial_value)                      noexcept : _block{} { *this = initial_value; }
	takum& operator=(long double rhs)                     noexcept { return convert_ieee754(rhs); }
	explicit operator long double()                 const noexcept { return to_ieee754<long double>(); }
#endif

	// arithmetic operators
	// prefix negation: two's complement negate
	takum operator-() const {
		if (iszero() || isnar()) return *this;
		takum result;
		uint64_t raw = raw_bits();
		uint64_t mask = nbits_mask();
		uint64_t negated = ((~raw) + 1ull) & mask;
		result.setbits(negated);
		return result;
	}

	// in-place arithmetic via double conversion
	takum& operator+=(const takum& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		double result = double(*this) + double(rhs);
		return convert_ieee754(result);
	}
	takum& operator+=(double rhs) { return *this += takum(rhs); }
	takum& operator-=(const takum& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		double result = double(*this) - double(rhs);
		return convert_ieee754(result);
	}
	takum& operator-=(double rhs) { return *this -= takum(rhs); }
	takum& operator*=(const takum& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		double result = double(*this) * double(rhs);
		return convert_ieee754(result);
	}
	takum& operator*=(double rhs) { return *this *= takum(rhs); }
	takum& operator/=(const takum& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (rhs.iszero()) {
#if TAKUM_THROW_ARITHMETIC_EXCEPTION
			throw takum_divide_by_zero();
#else
			setnar();
			return *this;
#endif
		}
		double result = double(*this) / double(rhs);
		return convert_ieee754(result);
	}
	takum& operator/=(double rhs) { return *this /= takum(rhs); }

	// prefix/postfix increment: advance to next/previous representable value
	takum& operator++() {
		if (isnar()) return *this;
		uint64_t raw = raw_bits();
		uint64_t mask = nbits_mask();
		if (raw == (mask >> 1)) return *this; // already maxpos
		raw = (raw + 1) & mask;
		setbits(raw);
		return *this;
	}
	takum operator++(int) {
		takum tmp(*this);
		operator++();
		return tmp;
	}
	takum& operator--() {
		if (isnar()) return *this;
		uint64_t raw = raw_bits();
		if (raw == ((1ull << (nbits - 1)) | 1ull)) return *this; // already maxneg
		raw = (raw - 1) & nbits_mask();
		setbits(raw);
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
	constexpr void setnan(bool sign = false)       noexcept { (void)sign; setnar(); }
	constexpr void setinf(bool sign)               noexcept { (sign ? maxneg() : maxpos()); }
	constexpr void setsign(bool s = true)          noexcept { setbit(nbits - 1, s); }
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned blockIndex = i / bitsInBlock;
		if (i < nbits) {
			bt block = _block[blockIndex];
			bt null = ~(1ull << (i % bitsInBlock));
			bt bit = bt(v ? 1 : 0);
			bt mask = bt(bit << (i % bitsInBlock));
			_block.setblock(blockIndex, bt((block & null) | mask));
		}
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
		_block.setblock(MSU, static_cast<bt>(_block[MSU] & MSU_MASK));
	}

	// Two's complement special values
	constexpr takum& maxpos() noexcept {
		clear(); flip(); setbit(nbits - 1, false);
		return *this;
	}
	constexpr takum& minpos() noexcept {
		clear(); setbit(0, true);
		return *this;
	}
	constexpr takum& zero() noexcept {
		clear();
		return *this;
	}
	constexpr takum& minneg() noexcept {
		clear(); flip();
		return *this;
	}
	constexpr takum& maxneg() noexcept {
		clear(); setbit(nbits - 1, true); setbit(0, true);
		return *this;
	}

	// selectors
	constexpr bool iszero()    const noexcept { return _block.iszero(); }
	constexpr bool isneg()     const noexcept { return _block.test(nbits - 1) && !isnar(); }
	constexpr bool ispos()     const noexcept { return !_block.test(nbits - 1) && !iszero(); }
	constexpr bool isinf()     const noexcept { return false; }
	constexpr bool isnan()     const noexcept { return isnar(); }
	constexpr bool isnar()     const noexcept {
		if (!_block.test(nbits - 1)) return false;
		for (unsigned i = 0; i < nrBlocks; ++i) {
			bt expected = (i == MSU) ? SIGN_BIT_MASK : bt(0);
			if (_block[i] != expected) return false;
		}
		return true;
	}
	constexpr bool sign()      const noexcept { return _block.test(nbits - 1); }

	// Extract fields from the magnitude representation
	constexpr bool direct() const noexcept {
		uint64_t mag = magnitude_bits();
		return static_cast<bool>((mag >> (nbits - 2)) & 1);
	}
	constexpr unsigned regime() const noexcept {
		uint64_t mag = magnitude_bits();
		return static_cast<unsigned>((mag >> (nbits - overhead)) & r_mask);
	}
	constexpr unsigned dr_field() const noexcept {
		uint64_t mag = magnitude_bits();
		return static_cast<unsigned>((mag >> (nbits - overhead)) & dr_field_mask);
	}
	int characteristic() const noexcept {
		if (iszero() || isnar()) return 0;
		unsigned dr = dr_field();
		unsigned r = dr_to_r(dr);
		unsigned avail = maxCharBits;
		unsigned p = (r < avail) ? (avail - r) : 0;
		unsigned c_stored = (r < avail) ? r : avail;
		uint64_t mag = magnitude_bits();
		uint64_t C_stored_bits = (c_stored > 0) ? ((mag >> p) & ((1ull << c_stored) - 1)) : 0;
		uint64_t C_bits = (r > avail) ? (C_stored_bits << (r - c_stored)) : C_stored_bits;
		return dr_to_c_bias(dr) + static_cast<int>(C_bits);
	}
	int scale() const noexcept {
		if (iszero() || isnar()) return 0;
		return characteristic();
	}
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false;
		bt word = _block[bitIndex / bitsInBlock];
		bt mask = bt(1ull << (bitIndex % bitsInBlock));
		return (word & mask);
	}
	constexpr bt block(unsigned b) const noexcept {
		if (b < nrBlocks) return _block[b];
		return bt(0);
	}
	constexpr uint8_t nibble(unsigned n) const noexcept {
		if (n < (1 + ((nbits - 1) >> 2))) {
			bt word = _block[(n * 4) / bitsInBlock];
			int nibbleIndexInWord = int(n % (bitsInBlock >> 2ull));
			bt mask = bt(0xF << (nibbleIndexInWord * 4));
			bt nibblebits = bt(mask & word);
			return uint8_t(nibblebits >> (nibbleIndexInWord * 4));
		}
		return 0;
	}

	inline std::string get() const noexcept { return std::string("tbd"); }

	void debugConstexprParameters() {
		std::cout << "constexpr parameters for " << type_tag(*this) << '\n';
		std::cout << "nbits                 " << nbits << '\n';
		std::cout << "rbits                 " << rbits << '\n';
		std::cout << "overhead              " << overhead << '\n';
		std::cout << "dr_bits               " << dr_bits << '\n';
		std::cout << "nr_dr_values          " << nr_dr_values << '\n';
		std::cout << "max_r                 " << max_r << '\n';
		std::cout << "maxCharBits           " << maxCharBits << '\n';
		std::cout << "min characteristic    " << min_characteristic() << '\n';
		std::cout << "max characteristic    " << max_characteristic() << '\n';
		std::cout << "bitsInBlock           " << bitsInBlock << '\n';
		std::cout << "nrBlocks              " << nrBlocks << '\n';
		std::cout << "MSU_MASK              " << to_binary(MSU_MASK, bitsInBlock) << '\n';
		std::cout << "SIGN_BIT_MASK         " << to_binary(SIGN_BIT_MASK, bitsInBlock) << '\n';
	}

	// Get the raw bit pattern as a uint64_t (works for nbits <= 64)
	constexpr uint64_t raw_bits() const noexcept {
		uint64_t raw = 0;
		for (unsigned i = 0; i < nrBlocks; ++i) {
			raw |= (static_cast<uint64_t>(_block[i]) << (i * bitsInBlock));
		}
		return raw;
	}

	// Two's complement magnitude (always non-negative)
	constexpr uint64_t magnitude_bits() const noexcept {
		uint64_t raw = raw_bits();
		if (raw & (1ull << (nbits - 1))) {
			raw = ((~raw) + 1) & nbits_mask();
		}
		return raw;
	}

protected:

	constexpr takum& flip() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block.setblock(i, bt(~_block[i]));
		}
		_block.setblock(MSU, bt(_block[MSU] & MSU_MASK));
		return *this;
	}

	CONSTEXPRESSION takum& assign(const std::string& str) noexcept {
		clear();
		return *this;
	}

	static constexpr uint64_t nbits_mask() noexcept {
		return (nbits < 64) ? ((1ull << nbits) - 1) : ~0ull;
	}

	//////////////////////////////////////////////////////
	/// conversion routines from native types

	template<typename SignedInt>
	CONSTEXPRESSION takum& convert_signed(SignedInt rhs) noexcept {
		return convert_ieee754(double(rhs));
	}
	template<typename UnsignedInt>
	CONSTEXPRESSION takum& convert_unsigned(UnsignedInt rhs) noexcept {
		return convert_ieee754(double(rhs));
	}

	/// Convert an IEEE-754 floating-point value to linear takum encoding
	template<typename Real>
	CONSTEXPRESSION takum& convert_ieee754(Real rhs) noexcept {
		static_assert(nbits <= 64, "takum > 64 bits not yet supported");

		if (rhs != rhs) { setnar(); return *this; }
		if (rhs == Real(0)) { setzero(); return *this; }
		if (rhs > std::numeric_limits<Real>::max()) { maxpos(); return *this; }
		if (rhs < std::numeric_limits<Real>::lowest()) { maxneg(); return *this; }

		bool s = (rhs < Real(0));
		double v = static_cast<double>(s ? -rhs : rhs);

		int h_raw;
		double frac = std::frexp(v, &h_raw);
		int h = h_raw - 1;
		double g = 2.0 * frac - 1.0;

		int c = h;
		double m_real = g;

		constexpr int c_max = max_characteristic();
		constexpr int c_min = min_characteristic();
		if (c > c_max) { s ? maxneg() : maxpos(); return *this; }
		if (c < c_min) { setzero(); return *this; }

		unsigned dr = find_dr(c);
		unsigned r = dr_to_r(dr);
		unsigned avail = maxCharBits;
		unsigned p = (r < avail) ? (avail - r) : 0;
		unsigned c_stored_bits = (r < avail) ? r : avail;

		unsigned C_bits_full = static_cast<unsigned>(c - dr_to_c_bias(dr));

		// When r > avail, only the MSBs of C are stored (truncated)
		unsigned C_stored;
		if (r <= avail) {
			C_stored = C_bits_full;
		}
		else {
			unsigned shift = r - c_stored_bits;
			C_stored = C_bits_full >> shift;
			unsigned remainder = C_bits_full & ((1u << shift) - 1);
			unsigned half = 1u << (shift - 1);
			if (remainder > half || (remainder == half && (C_stored & 1))) {
				C_stored++;
			}
			if (C_stored >= (1u << c_stored_bits)) {
				if (dr < nr_dr_values - 1) {
					dr++;
					r = dr_to_r(dr);
					p = (r < avail) ? (avail - r) : 0;
					c_stored_bits = (r < avail) ? r : avail;
					C_stored = 0;
					m_real = 0.0;
				}
				else {
					s ? maxneg() : maxpos();
					return *this;
				}
			}
		}

		uint64_t M_bits = 0;
		if (p > 0) {
			double scaled = m_real * static_cast<double>(1ull << p);
			M_bits = static_cast<uint64_t>(scaled);
			double remainder = scaled - static_cast<double>(M_bits);
			if (remainder > 0.5 || (remainder == 0.5 && (M_bits & 1))) {
				M_bits++;
			}
			if (M_bits >= (1ull << p)) {
				M_bits = 0;
				C_stored++;
				unsigned max_c_val = (1u << c_stored_bits);
				if (C_stored >= max_c_val) {
					if (dr < nr_dr_values - 1) {
						dr++;
						r = dr_to_r(dr);
						p = (r < avail) ? (avail - r) : 0;
						c_stored_bits = (r < avail) ? r : avail;
						C_stored = 0;
						M_bits = 0;
					}
					else {
						s ? maxneg() : maxpos();
						return *this;
					}
				}
			}
		}

		uint64_t magnitude = 0;
		magnitude |= (static_cast<uint64_t>(dr) << (nbits - overhead));
		if (c_stored_bits > 0) {
			magnitude |= (static_cast<uint64_t>(C_stored) << p);
		}
		if (p > 0) {
			magnitude |= M_bits;
		}

		uint64_t raw;
		if (s) {
			raw = ((~magnitude) + 1ull) & nbits_mask();
		}
		else {
			raw = magnitude;
		}

		setbits(raw);
		return *this;
	}

	//////////////////////////////////////////////////////
	/// conversion routines to native types

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

	/// Decode a linear takum to an IEEE-754 floating-point value
	template<typename TargetFloat>
	TargetFloat to_ieee754() const noexcept {
		if (iszero()) return TargetFloat(0);
		if (isnar()) return std::numeric_limits<TargetFloat>::quiet_NaN();

		static_assert(nbits <= 64, "takum > 64 bits not yet supported");

		bool s = sign();
		uint64_t mag = magnitude_bits();

		unsigned dr = static_cast<unsigned>((mag >> (nbits - overhead)) & dr_field_mask);
		unsigned r = dr_to_r(dr);
		unsigned avail = maxCharBits;
		unsigned p = (r < avail) ? (avail - r) : 0;
		unsigned c_stored = (r < avail) ? r : avail;

		uint64_t C_stored_bits = 0;
		if (c_stored > 0) {
			C_stored_bits = (mag >> p) & ((1ull << c_stored) - 1);
		}
		uint64_t C_bits = (r > avail) ? (C_stored_bits << (r - c_stored)) : C_stored_bits;
		int c = dr_to_c_bias(dr) + static_cast<int>(C_bits);

		TargetFloat f = TargetFloat(0);
		if (p > 0) {
			uint64_t M_bits = mag & ((1ull << p) - 1);
			f = static_cast<TargetFloat>(M_bits) / static_cast<TargetFloat>(1ull << p);
		}

		TargetFloat value = (TargetFloat(1) + f) * std::exp2(static_cast<TargetFloat>(c));
		if (s) value = -value;

		return value;
	}

private:
	BlockBinary _block;

	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const takum<nnbits, nrbits, nbt>& r);
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, takum<nnbits, nrbits, nbt>& r);

	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend bool operator==(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs);
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend bool operator!=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs);
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend bool operator< (const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs);
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend bool operator> (const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs);
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend bool operator<=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs);
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend bool operator>=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs);
};

// return the Unit in the Last Position
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt> ulp(const takum<nbits, rbits, bt>& a) {
	takum<nbits, rbits, bt> b(a);
	return ++b - a;
}

template<unsigned nbits, unsigned rbits, typename bt>
std::string to_binary(const takum<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	using T = takum<nbits, rbits, bt>;
	std::stringstream s;
	bool negative = number.sign();
	uint64_t mag = number.magnitude_bits();

	s << "0b";
	s << (negative ? "1." : "0.");

	// Direction bit from magnitude
	bool D = static_cast<bool>((mag >> (nbits - 2)) & 1);
	s << (D ? "1." : "0.");

	// Regime field from magnitude (rbits bits)
	unsigned regime = static_cast<unsigned>((mag >> (nbits - T::overhead)) & T::r_mask);
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
		s << ((regime >> i) & 1 ? '1' : '0');
	}
	s << '.';

	// Characteristic and mantissa bits
	unsigned dr = (D ? (1u << rbits) : 0) + regime;
	unsigned r = T::dr_to_r(dr);
	unsigned avail = T::maxCharBits;
	unsigned p = (r < avail) ? (avail - r) : 0;
	unsigned c_stored = (r < avail) ? r : avail;
	int bit = static_cast<int>(nbits) - static_cast<int>(T::overhead) - 1;

	for (unsigned i = 0; i < c_stored && bit >= 0; ++i) {
		s << ((mag >> bit) & 1 ? '1' : '0');
		--bit;
		if (i < c_stored - 1 && ((c_stored - 1 - i) % 4) == 0 && nibbleMarker) s << '\'';
	}
	s << '.';
	for (unsigned i = 0; i < p && bit >= 0; ++i) {
		s << ((mag >> bit) & 1 ? '1' : '0');
		if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
		--bit;
	}

	return s.str();
}

////////////////////// operators
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const takum<nnbits, nrbits, nbt>& v) {
	ostr << double(v);
	return ostr;
}

template<unsigned nnbits, unsigned nrbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, takum<nnbits, nrbits, nbt>& v) {
	double d;
	istr >> d;
	v = d;
	return istr;
}

template<unsigned nnbits, unsigned nrbits, typename nbt>
inline bool operator==(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) {
	if (lhs.isnar() || rhs.isnar()) return false;
	return (lhs._block == rhs._block);
}
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline bool operator!=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) {
	if (lhs.isnar() || rhs.isnar()) return true;
	return !(lhs._block == rhs._block);
}

template<unsigned nnbits, unsigned nrbits, typename nbt>
inline bool operator< (const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) {
	if (lhs.isnar() || rhs.isnar()) return false;
	uint64_t l = lhs.raw_bits();
	uint64_t r = rhs.raw_bits();
	int64_t ls, rs;
	// Sign-extend to 64 bits; guard against nnbits == 64 (where 1ull << 64 is UB)
	uint64_t sign_ext = (nnbits < 64) ? ~((1ull << nnbits) - 1) : 0ull;
	if (l & (1ull << (nnbits - 1))) {
		ls = static_cast<int64_t>(l | sign_ext);
	}
	else {
		ls = static_cast<int64_t>(l);
	}
	if (r & (1ull << (nnbits - 1))) {
		rs = static_cast<int64_t>(r | sign_ext);
	}
	else {
		rs = static_cast<int64_t>(r);
	}
	return ls < rs;
}
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline bool operator> (const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline bool operator<=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) {
	if (lhs.isnar() || rhs.isnar()) return false;
	return !operator> (lhs, rhs);
}
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline bool operator>=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) {
	if (lhs.isnar() || rhs.isnar()) return false;
	return !operator< (lhs, rhs);
}

// Binary arithmetic operators
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt> operator+(const takum<nbits, rbits, bt>& lhs, const takum<nbits, rbits, bt>& rhs) {
	takum<nbits, rbits, bt> sum(lhs); sum += rhs; return sum;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt> operator-(const takum<nbits, rbits, bt>& lhs, const takum<nbits, rbits, bt>& rhs) {
	takum<nbits, rbits, bt> diff(lhs); diff -= rhs; return diff;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt> operator*(const takum<nbits, rbits, bt>& lhs, const takum<nbits, rbits, bt>& rhs) {
	takum<nbits, rbits, bt> mul(lhs); mul *= rhs; return mul;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt> operator/(const takum<nbits, rbits, bt>& lhs, const takum<nbits, rbits, bt>& rhs) {
	takum<nbits, rbits, bt> ratio(lhs); ratio /= rhs; return ratio;
}

template<unsigned nbits, unsigned rbits, typename bt>
constexpr takum<nbits, rbits, bt> abs(const takum<nbits, rbits, bt>& v) {
	if (v.isneg()) return -v;
	return v;
}

}}  // namespace sw::universal
