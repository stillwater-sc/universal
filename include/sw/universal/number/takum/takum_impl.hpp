#pragma once
// TAKUM_THROW_ARITHMETIC_EXCEPTION: throw specific exceptions on arithmetic errors, left to the
// application to enable. The default lives here rather than in the takum.hpp umbrella,
// so that including core.hpp alone defines it as well (#1436).
#if !defined(TAKUM_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 0
#endif

#include <cstdint>       // the fixed-width integer types
#include <type_traits>   // std::is_same_v
#include <iosfwd>     // std::ostream/std::istream in the friend declarations (#1334)
#include <universal/utility/icf_array_bounds.hpp>
#include <string>
// takum_impl.hpp: implementation of a linear takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// LINEAR takum encoding (Hunhold, 2024, arXiv:2404.18603, Definition 8, Sec 4.7;
// restated as Definition 2 of arXiv:2408.10594, the hardware codec paper).
//
//   The takum specification defines two variants that share an identical bit
//   layout and differ only in the value map:
//     - logarithmic takum, base sqrt(e): 2404.18603 Def. 2 / 2408.10594 Def. 1
//     - linear takum,      base 2:       2404.18603 Def. 8 / 2408.10594 Def. 2
//   This type implements the LINEAR variant.  Note that 2404.18603 Sec 4.7
//   designates the *logarithmic* variant as the standard and requires
//   implementations to state which one they provide -- hence the emphasis here.
//   The naming follows the libtakum reference implementation, which uses the
//   bare name for the linear variant and takum_log for the logarithmic one.
//   See docs/takum-design.md.
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

#include <universal/native/ieee754_core.hpp>   // the bit-manipulation half (#1334)
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/abstract/triple.hpp>
#include <math/constexpr_math/exp2.hpp>

#include <universal/internal/bit_manipulation.hpp>
#include <universal/number/takum/takum_codec.hpp>
#include <universal/number/takum/takum_wide_arithmetic.hpp>

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
public:
	typedef bt BlockType;

	// The field codec shared with the logarithmic takum.  It owns the bit layout,
	// the DR/characteristic/mantissa geometry, and the rounded encode path; this
	// class supplies only the linear value map.  Its static_asserts validate
	// nbits / rbits on instantiation.
	using Codec = takum_codec<_nbits, _rbits>;

	static constexpr unsigned nbits    = _nbits;
	static constexpr unsigned rbits    = _rbits;
	static constexpr unsigned overhead = Codec::overhead;  // S + D + R field width

	// DR field properties (aliases of the codec's geometry)
	static constexpr unsigned dr_bits       = Codec::dr_bits;
	static constexpr unsigned nr_dr_values  = Codec::nr_dr_values;
	static constexpr unsigned max_r         = Codec::max_r;
	static constexpr unsigned r_mask        = Codec::r_mask;
	static constexpr unsigned dr_field_mask = Codec::dr_field_mask;

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
	static constexpr unsigned maxCharBits = Codec::maxCharBits;

	// Can a double evaluate the arithmetic with a single effective rounding?  A takum
	// significand is s = 1 + p bits and p reaches maxCharBits.  Narrow configurations
	// evaluate + - * / and fma in a double and convert back, which rounds TWICE: once
	// to 53 bits, once to the takum.  That is harmless only when 53 >= 2s + 2
	// (Figueroa, "When is double rounding innocuous?", 1995), i.e. s <= 25, i.e.
	// nbits <= 26 + rbits -- 29 for the specified rbits = 3.  Above it the first
	// rounding can move an exact result onto a takum midpoint and the second then
	// breaks the tie the wrong way: takum<32,3> 0x44000001 * 0x40000001 came back as
	// 0x44000002, where the correctly rounded product is 0x44000003 (#1616).
	//
	// So everything wider -- including takum<64,3>, whose 60-bit significand does not
	// even fit a double (#1300) -- evaluates exactly in integers
	// (takum_wide_arithmetic.hpp) and rounds once, in the codec.  Narrower
	// configurations keep the double path: their operands are exact doubles and the
	// double rounding provably lands where a single rounding would.
	static constexpr bool wide_significand = 2u * (maxCharBits + 1u) + 2u > 53u;

	// Does the range fit where the double path needs it?  The double path is only
	// sound while every operand, and every product, quotient and sum of two of them,
	// is a normal double.  A characteristic within +/-510 keeps all of those inside
	// 2^+/-1022.  Every rbits <= 3 configuration qualifies (|c| <= 255) and decides
	// this at compile time; rbits = 4 and 5 reach 2^16 and 2^32, where double(x) is
	// infinity or zero and takum<24,4> returned NaR for 2^2000 * 1 (#1626).
	static constexpr int64_t double_safe_characteristic = 510;
	static constexpr bool range_fits_double =
		(Codec::max_characteristic() <= double_safe_characteristic) &&
		(Codec::min_characteristic() >= -double_safe_characteristic);

	// Is this value inside the range the double path handles?  Zero is.
	constexpr bool in_double_safe_range() const noexcept {
		if (iszero()) return true;
		const int64_t c = Codec::characteristic_of(magnitude_bits());
		return c >= -double_safe_characteristic && c <= double_safe_characteristic;
	}

	// Which path an operation takes.  wide_significand: always exact, a double cannot
	// round the result once.  Otherwise the double path, unless the format reaches
	// past double's range and this operand pair actually does -- decided per call,
	// so values that a double holds keep its speed (the integer path costs 4-20x on
	// a narrow rbits = 4 layout) and only far ones pay for exactness.
	constexpr bool takes_exact_path(const takum& rhs) const noexcept {
		if constexpr (wide_significand) return true;
		else if constexpr (range_fits_double) return false;
		else return !(in_double_safe_range() && rhs.in_double_safe_range());
	}

	// Codec geometry, re-exported so that the public surface of takum<> is
	// unchanged by the codec extraction (manipulators, numeric_limits and the
	// api/constexpr.cpp static_asserts all reach for these).
	static constexpr unsigned dr_to_r(unsigned dr)        noexcept { return Codec::dr_to_r(dr); }
	static constexpr int64_t  dr_to_c_bias(unsigned dr)   noexcept { return Codec::dr_to_c_bias(dr); }
	static constexpr unsigned find_dr(int64_t c)          noexcept { return Codec::find_dr(c); }
	static constexpr int64_t  max_characteristic()        noexcept { return Codec::max_characteristic(); }
	static constexpr int64_t  min_characteristic()        noexcept { return Codec::min_characteristic(); }

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

	// Every type with an exact-match assignment operator below needs a matching
	// constructor.  Without one, construction from that type is ambiguous -- it is a
	// Conversion to each of int, long long, unsigned long long, float, double and
	// long double alike, with nothing to break the tie -- while assignment keeps
	// working, which makes the gap easy to miss.  char and unsigned short are the
	// exceptions: they promote to int, and a Promotion outranks every Conversion.
	constexpr takum(signed char initial_value)         noexcept : _block{} { *this = initial_value; }
	constexpr takum(short initial_value)               noexcept : _block{} { *this = initial_value; }
	constexpr takum(int initial_value)                 noexcept : _block{} { *this = initial_value; }
	constexpr takum(long initial_value)                noexcept : _block{} { *this = initial_value; }
	constexpr takum(long long initial_value)           noexcept : _block{} { *this = initial_value; }
	constexpr takum(unsigned int initial_value)        noexcept : _block{} { *this = initial_value; }
	constexpr takum(unsigned long initial_value)       noexcept : _block{} { *this = initial_value; }
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

	explicit CONSTEXPRESSION operator int()       const noexcept { return to_signed<int>(); }
	explicit CONSTEXPRESSION operator long()      const noexcept { return to_signed<long>(); }
	explicit CONSTEXPRESSION operator long long() const noexcept { return to_signed<long long>(); }
	explicit CONSTEXPRESSION operator float()     const noexcept { return to_ieee754<float>(); }
	explicit CONSTEXPRESSION operator double()    const noexcept { return to_ieee754<double>(); }

	// guard long double support to enable ARM and RISC-V embedded environments
#if LONG_DOUBLE_SUPPORT
	CONSTEXPRESSION takum(long double initial_value)                      noexcept : _block{} { *this = initial_value; }
	// Narrow to double first.  convert_ieee754() reads the source's IEEE 754
	// fields, and a long double is not one format: x87 80-bit on x86, IEEE
	// binary128 on ARM64, and plain double on MSVC, with different biases and,
	// for x87, an explicit rather than implicit integer bit.  Rather than decode
	// three layouts, take the one conversion the platform already provides.  The
	// cost is the bits beyond double's 53; the alternative was a wrong answer.
	CONSTEXPRESSION takum& operator=(long double rhs)                     noexcept {
		return convert_ieee754(static_cast<double>(rhs));
	}
	explicit CONSTEXPRESSION operator long double()                 const noexcept { return to_ieee754<long double>(); }
#endif

	// arithmetic operators
	// prefix negation: two's complement negate
	constexpr takum operator-() const noexcept {
		if (iszero() || isnar()) return *this;
		takum result;
		uint64_t raw = raw_bits();
		uint64_t mask = nbits_mask();
		uint64_t negated = ((~raw) + 1ull) & mask;
		result.setbits(negated);
		return result;
	}

	// in-place arithmetic.  Narrow configurations evaluate in a double, where both
	// operands are exact and the conversion back is the single rounding that
	// decides the result; the rest (see takes_exact_path) evaluate exactly in
	// integers instead.  CONSTEXPRESSION because the underlying convert_ieee754 /
	// to_ieee754 path becomes constexpr only when sw::bit_cast is constexpr
	// (BIT_CAST_IS_CONSTEXPR=true) and the constexpr_math::exp2 helper is
	// constant-evaluable; the integer path is constexpr throughout.
	CONSTEXPRESSION takum& operator+=(const takum& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (takes_exact_path(rhs)) {
			return wide_sum(rhs, false);
		}
		else {
			double result = double(*this) + double(rhs);
			return convert_ieee754(result);
		}
	}
	CONSTEXPRESSION takum& operator+=(double rhs) { return *this += takum(rhs); }
	CONSTEXPRESSION takum& operator-=(const takum& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (takes_exact_path(rhs)) {
			return wide_sum(rhs, true);
		}
		else {
			double result = double(*this) - double(rhs);
			return convert_ieee754(result);
		}
	}
	CONSTEXPRESSION takum& operator-=(double rhs) { return *this -= takum(rhs); }
	CONSTEXPRESSION takum& operator*=(const takum& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (takes_exact_path(rhs)) {
			if (iszero() || rhs.iszero()) { setzero(); return *this; }
			return assign_wide(takum_wide::multiply(to_wide_operand(), rhs.to_wide_operand()));
		}
		else {
			double result = double(*this) * double(rhs);
			return convert_ieee754(result);
		}
	}
	CONSTEXPRESSION takum& operator*=(double rhs) { return *this *= takum(rhs); }
	CONSTEXPRESSION takum& operator/=(const takum& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (rhs.iszero()) {
#if TAKUM_THROW_ARITHMETIC_EXCEPTION
			if (!std::is_constant_evaluated()) throw takum_divide_by_zero();
			setnar();
			return *this;
#else
			setnar();
			return *this;
#endif
		}
		if (takes_exact_path(rhs)) {
			if (iszero()) { setzero(); return *this; }
			return assign_wide(takum_wide::divide(to_wide_operand(), rhs.to_wide_operand()));
		}
		else {
			double result = double(*this) / double(rhs);
			return convert_ieee754(result);
		}
	}
	CONSTEXPRESSION takum& operator/=(double rhs) { return *this /= takum(rhs); }

	// Exact-arithmetic surface, public so that fma() -- which is a free function
	// and cannot reach a private member -- shares this decode and this rounding
	// tail rather than reimplementing either.  Both are well defined at every
	// width: the operators reach them when takes_exact_path() says so, but
	// fma() takes them at every width, since no width makes std::fma's double
	// rounding safe (#1616).

	// Decode to  value = (-1)^sign * S * 2^e.  Pre: neither zero nor NaR.
	constexpr takum_wide::operand to_wide_operand() const noexcept {
		return takum_wide::decode_operand<Codec>(sign(), magnitude_bits());
	}

	// Round a fully evaluated exact result into this takum.  A zero V is the
	// genuine zero the codec cannot express; saturation follows conversion from a
	// native float and lands on maxpos / maxneg / minpos / minneg.
	CONSTEXPRESSION takum& assign_wide(const takum_wide::wide_value& r) noexcept {
		if (takum_wide::iszero(r.V)) { setzero(); return *this; }
		auto enc = takum_wide::encode<Codec>(r);
		if (enc.overflowed()) { if (r.sign) maxneg(); else maxpos(); return *this; }
		if (enc.underflowed()) { if (r.sign) minneg(); else minpos(); return *this; }
		// The codec never sets the sign bit (I4); two's-complement negate here.
		setbits(r.sign ? (((~enc.magnitude) + 1ull) & nbits_mask()) : enc.magnitude);
		return *this;
	}

	// prefix/postfix increment: advance to next/previous representable value
	constexpr takum& operator++() noexcept {
		if (isnar()) return *this;
		uint64_t raw = raw_bits();
		uint64_t mask = nbits_mask();
		if (raw == (mask >> 1)) return *this; // already maxpos
		raw = (raw + 1) & mask;
		setbits(raw);
		return *this;
	}
	constexpr takum operator++(int) noexcept {
		takum tmp(*this);
		operator++();
		return tmp;
	}
	constexpr takum& operator--() noexcept {
		if (isnar()) return *this;
		uint64_t raw = raw_bits();
		if (raw == ((1ull << (nbits - 1)) | 1ull)) return *this; // already maxneg
		raw = (raw - 1) & nbits_mask();
		setbits(raw);
		return *this;
	}
	constexpr takum operator--(int) noexcept {
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
			bt null = bit_clear_mask<bt>(i, bitsInBlock);
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
	constexpr int64_t characteristic() const noexcept {
		if (iszero() || isnar()) return 0;
		return Codec::characteristic_of(magnitude_bits());
	}
	constexpr int64_t scale() const noexcept {
		if (iszero() || isnar()) return 0;
		return characteristic();
	}
	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false;
		// in bounds: bitIndex < nbits => index <= nrBlocks-1. The pragma silences a
		// GCC -fipa-icf false positive; see utility/icf_array_bounds.hpp.
		UNIVERSAL_ICF_ARRAY_BOUNDS_PUSH
		bt word = _block[bitIndex / bitsInBlock];
		UNIVERSAL_ICF_ARRAY_BOUNDS_POP
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

	// DECLARED here, DEFINED in takum/debug.hpp, so the arithmetic core needs no
	// <iostream> (#1334). Include debug.hpp to call it.
	void debugConstexprParameters();

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

	static constexpr uint64_t nbits_mask() noexcept { return Codec::nbits_mask(); }

	// Exact addition / subtraction for wide configurations.  The zero cases are
	// peeled off here because takum_wide::sum() decodes a significand and a zero
	// has none; they are also the cases issue #1300 opened on -- x + 0 came back
	// quantized to a double rather than as x.
	CONSTEXPRESSION takum& wide_sum(const takum& rhs, bool subtract) noexcept {
		if (rhs.iszero()) return *this;
		if (iszero()) { *this = (subtract ? -rhs : rhs); return *this; }
		return assign_wide(takum_wide::sum(takum_wide::widen(to_wide_operand()),
		                                   takum_wide::widen(rhs.to_wide_operand()),
		                                   subtract));
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

	/// Convert an IEEE-754 floating-point value to linear takum encoding.
	/// Constexpr-promoted by replacing std::frexp with bit-extraction:
	/// for an IEEE 754 normal v = (1 + rawFrac/2^fbits) * 2^(rawExp - bias),
	/// the takum decomposition is c = rawExp - bias and m_real = rawFrac/2^fbits
	/// directly, with no math-library calls.  Subnormals are normalized via a
	/// bounded leading-zero shift on rawFrac.  Long double routes through
	/// double at constant evaluation (LONG_DOUBLE_DOWNCAST pattern); finite
	/// long-double inputs out of double range are extremely rare in compile-
	/// time literals.
	template<typename Real>
	CONSTEXPRESSION takum& convert_ieee754(Real rhs) noexcept {
		static_assert(nbits <= 64, "takum > 64 bits not yet supported");

		if (rhs != rhs) { setnar(); return *this; }
		// Takum has no infinity (numeric_limits<>::has_infinity == false); both
		// +inf and -inf map to NaR.  Detect via direct equality so this stays
		// constexpr-clean (std::isinf is not constexpr until C++23).
		if constexpr (std::numeric_limits<Real>::has_infinity) {
			if (rhs == std::numeric_limits<Real>::infinity()) { setnar(); return *this; }
			if (rhs == -std::numeric_limits<Real>::infinity()) { setnar(); return *this; }
		}
		if (rhs == Real(0)) { setzero(); return *this; }
		if (rhs > std::numeric_limits<Real>::max()) { maxpos(); return *this; }
		if (rhs < std::numeric_limits<Real>::lowest()) { maxneg(); return *this; }

		bool s = (rhs < Real(0));
		Real abs_v = s ? -rhs : rhs;

		// Extract IEEE 754 fields directly so we avoid std::frexp at constant
		// evaluation.  extractFields is BIT_CAST_CONSTEXPR.
		bool ignored_sign = false;
		uint64_t rawExp = 0;
		uint64_t rawFrac = 0;
		uint64_t bits = 0;
		extractFields(abs_v, ignored_sign, rawExp, rawFrac, bits);

		// The source format's bias / fbits.  Real is float or double here and
		// never long double: operator=(long double) narrows first, so the fields
		// extracted above always carry that format's own encoding.
		//
		// This used to force double's parameters whenever Real was long double,
		// on the assumption that extractFields routes long double through double.
		// It does so only when LONG_DOUBLE_DOWNCAST is defined.  Everywhere else
		// -- Linux and macOS x86-64 with gcc or clang -- it returns genuine x87
		// 80-bit fields, whose exponent is biased by 16383 rather than 1023, and
		// reading those with double's bias put every finite value past
		// max_characteristic(): takum<32,3>{3.0L} came out as maxpos.
		static_assert(!std::is_same_v<Real, long double>,
		              "convert_ieee754 requires a format whose extractFields encoding matches "
		              "ieee754_parameter<Real>; narrow long double to double before calling");
		using Param = ieee754_parameter<Real>;
		constexpr int src_bias  = Param::bias;
		constexpr int src_fbits = Param::fbits;

		// Use int64_t for the unbiased exponent / characteristic so that
		// max_characteristic() / min_characteristic() comparisons stay correct
		// for _rbits up to 5 (where the takum-format c range exceeds int).
		// The IEEE 754 source itself produces |c| <= 1075 (double), well
		// inside int range, so the widening is purely defensive.
		int64_t c = 0;
		double m_real = 0.0;
		if (rawExp != 0u) {
			// normal: value = (1 + rawFrac/2^fbits) * 2^(rawExp - bias)
			c = static_cast<int64_t>(rawExp) - src_bias;
			m_real = static_cast<double>(rawFrac) / static_cast<double>(1ull << src_fbits);
		}
		else {
			// subnormal: rawFrac > 0 by construction (we returned for v == 0).
			// Normalize by left-shifting rawFrac until the implicit-bit slot is
			// set; each shift decrements the represented exponent by 1.
			int64_t implicit_exp = static_cast<int64_t>(1) - src_bias; // base subnormal exponent
			uint64_t f = rawFrac;
			while ((f & (1ull << src_fbits)) == 0u) {
				f <<= 1;
				--implicit_exp;
			}
			f &= (1ull << src_fbits) - 1u; // strip implicit bit
			c = implicit_exp;
			m_real = static_cast<double>(f) / static_cast<double>(1ull << src_fbits);
		}

		// Everything from here is format-independent: the codec owns the field
		// geometry, the round-to-nearest-even of C and M, the carry propagation
		// into the next DR, and the saturation decision.  For the LINEAR takum
		// the (c, m) pair handed over is exactly the IEEE 754 exponent and
		// fraction; a logarithmic takum would instead pass c = floor(l),
		// m = l - c and reach the identical code.
		auto enc = Codec::encode_rounded(c, m_real);
		if (enc.overflowed()) {
			if (s) maxneg(); else maxpos();
			return *this;
		}
		if (enc.underflowed()) { if (s) minneg(); else minpos(); return *this; }

		// The codec never sets the sign bit (I4); two's-complement negate here.
		uint64_t raw = s ? (((~enc.magnitude) + 1ull) & nbits_mask()) : enc.magnitude;
		setbits(raw);
		return *this;
	}

	//////////////////////////////////////////////////////
	/// conversion routines to native types

	template<typename SignedInt>
	CONSTEXPRESSION typename std::enable_if< std::is_integral<SignedInt>::value&& std::is_signed<SignedInt>::value, SignedInt>::type
		to_signed() const noexcept {
		return SignedInt(to_ieee754<double>());
	}
	template<typename UnsignedInt>
	CONSTEXPRESSION typename std::enable_if< std::is_integral<UnsignedInt>::value&& std::is_unsigned<UnsignedInt>::value, UnsignedInt>::type
		to_unsigned() const noexcept {
		return UnsignedInt(to_ieee754<double>());
	}

	/// Decode a linear takum to an IEEE-754 floating-point value, correctly rounded.
	///
	/// |value| = S * 2^e with the integer significand S = 2^p + M, so the conversion
	/// is one integer rounding of S to the precision the target holds AT THIS
	/// MAGNITUDE -- digits for a normal result, fewer for a subnormal one -- followed
	/// by an exact scaling.  The earlier form, (1 + M/2^p) * float(2^c), rounded up
	/// to three times: M/2^p to the target, 1 + f again, and the product once more
	/// when it was subnormal.  takum<32,3> -> float was wrong for 2.9% of encodings,
	/// every takum value in (2^-150, 2^-149) became 0.0f instead of 2^-149, and
	/// takum<64,3> -> double was double-rounded the same way (#1622).
	///
	/// Constexpr-promoted: powers of two come from constexpr_math::detail::pow2,
	/// which sets the exponent field directly -- exact, O(1), and unlike exp2 it
	/// does not evaluate a Taylor series for an integer argument.
	template<typename TargetFloat>
	CONSTEXPRESSION TargetFloat to_ieee754() const noexcept {
		if (iszero()) return TargetFloat(0);
		if (isnar()) return std::numeric_limits<TargetFloat>::quiet_NaN();

		static_assert(nbits <= 64, "takum > 64 bits not yet supported");
		using Limits = std::numeric_limits<TargetFloat>;
		constexpr int64_t digits = Limits::digits;
		constexpr int64_t emin   = Limits::min_exponent - 1;   // 2^emin is the smallest normal
		constexpr int64_t emax   = Limits::max_exponent - 1;   // 2^emax is the largest binade

		const bool s = sign();
		const auto d = Codec::decode(magnitude_bits());
		if (d.c > emax) return s ? -Limits::infinity() : Limits::infinity();

		uint64_t S = (1ull << d.p) | d.M_bits;
		int64_t  e = d.c - static_cast<int64_t>(d.p);

		// Round S to the bits the target holds at 2^c, to nearest even.  A carry that
		// reaches 2^keep is still exact, except in the top binade, where it is the
		// correctly rounded overflow to infinity -- returned here, because narrowing
		// a double above FLT_MAX is an implementation-defined choice between FLT_MAX
		// and infinity, and only infinity is correctly rounded.
		const int64_t keep = (d.c >= emin) ? digits : digits - (emin - d.c);
		const int64_t drop = static_cast<int64_t>(d.p) + 1 - keep;
		if (drop > 0) {
			if (drop >= 64) {
				S = 0;                                         // below half the smallest subnormal
			}
			else {
				const uint64_t rem  = S & ((1ull << drop) - 1ull);
				const uint64_t half = 1ull << (drop - 1);
				S >>= drop;
				if (rem > half || (rem == half && (S & 1ull))) ++S;
				e += drop;
				// keep < p + 1 <= 62 on this branch, so the shift is in range
				if (d.c == emax && (S >> keep) != 0ull) return s ? -Limits::infinity() : Limits::infinity();
			}
		}
		if (S == 0) return s ? -TargetFloat(0) : TargetFloat(0);

		// S now fits the target exactly, so only the scaling remains, and it is exact:
		// every step moves monotonically toward the final value, which the rounding
		// above made representable.  float scales in double, whose range covers every
		// step; the result converts to float without rounding.
		using Work = std::conditional_t<(sizeof(TargetFloat) < sizeof(double)), double, TargetFloat>;
		Work value = static_cast<Work>(S);
		using sw::math::constexpr_math::detail::pow2;
		while (e >  960) { value *= static_cast<Work>(pow2( 960)); e -= 960; }
		while (e < -960) { value *= static_cast<Work>(pow2(-960)); e += 960; }
		value *= static_cast<Work>(pow2(static_cast<int>(e)));
		const TargetFloat result = static_cast<TargetFloat>(value);
		return s ? -result : result;
	}

private:
	BlockBinary _block;

	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const takum<nnbits, nrbits, nbt>& r);
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend std::istream& operator>> (std::istream& istr, takum<nnbits, nrbits, nbt>& r);

	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend constexpr bool operator==(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept;
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend constexpr bool operator!=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept;
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend constexpr bool operator< (const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept;
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend constexpr bool operator> (const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept;
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend constexpr bool operator<=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept;
	template<unsigned nnbits, unsigned nrbits, typename nbt>
	friend constexpr bool operator>=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept;
};

// return the Unit in the Last Position
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum<nbits, rbits, bt> ulp(const takum<nbits, rbits, bt>& a) {
	takum<nbits, rbits, bt> b(a);
	return ++b - a;
}


// native semantic representation: radix-2, delegates to to_binary


template<unsigned nnbits, unsigned nrbits, typename nbt>
inline constexpr bool operator==(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return false;
	return (lhs._block == rhs._block);
}
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline constexpr bool operator!=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return true;
	return !(lhs._block == rhs._block);
}

template<unsigned nnbits, unsigned nrbits, typename nbt>
inline constexpr bool operator< (const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept {
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
inline constexpr bool operator> (const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept { return  operator< (rhs, lhs); }
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline constexpr bool operator<=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return false;
	return !operator> (lhs, rhs);
}
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline constexpr bool operator>=(const takum<nnbits, nrbits, nbt>& lhs, const takum<nnbits, nrbits, nbt>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return false;
	return !operator< (lhs, rhs);
}

// Binary arithmetic operators
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum<nbits, rbits, bt> operator+(const takum<nbits, rbits, bt>& lhs, const takum<nbits, rbits, bt>& rhs) {
	takum<nbits, rbits, bt> sum(lhs); sum += rhs; return sum;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum<nbits, rbits, bt> operator-(const takum<nbits, rbits, bt>& lhs, const takum<nbits, rbits, bt>& rhs) {
	takum<nbits, rbits, bt> diff(lhs); diff -= rhs; return diff;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum<nbits, rbits, bt> operator*(const takum<nbits, rbits, bt>& lhs, const takum<nbits, rbits, bt>& rhs) {
	takum<nbits, rbits, bt> mul(lhs); mul *= rhs; return mul;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum<nbits, rbits, bt> operator/(const takum<nbits, rbits, bt>& lhs, const takum<nbits, rbits, bt>& rhs) {
	takum<nbits, rbits, bt> ratio(lhs); ratio /= rhs; return ratio;
}

template<unsigned nbits, unsigned rbits, typename bt>
constexpr takum<nbits, rbits, bt> abs(const takum<nbits, rbits, bt>& v) noexcept {
	if (v.isneg()) return -v;
	return v;
}

// sqrt (satisfies the forward declaration in takum_fwd.hpp).  It follows the
// arithmetic operators: narrow configurations evaluate in a double, where the
// double rounding is innocuous for the same reason it is for + - * / (Figueroa:
// 53 >= 2s + 2 covers sqrt as well), and everything wider takes an exact integer
// root.  std::sqrt above that gate was not correctly rounded: takum<32,3>
// sqrt(0x40000003) gave 0x40000002, and at 52 bits 1.6% of results were off (#1622).
// A value outside the range a double holds takes the integer root too (#1626).
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> sqrt(const takum<nbits, rbits, bt>& v) {
	using Takum = takum<nbits, rbits, bt>;
	if (v.takes_exact_path(v)) {
		Takum result;
		if (v.isnar() || v.sign()) { result.setnar(); return result; }   // sqrt of a negative is NaR
		if (v.iszero()) return v;
		return result.assign_wide(takum_wide::sqrt(v.to_wide_operand()));
	}
	else {
		return Takum(std::sqrt(double(v)));
	}
}

}}  // namespace sw::universal
