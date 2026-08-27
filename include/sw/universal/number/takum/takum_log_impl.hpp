#pragma once
// takum_log_impl.hpp: implementation of the logarithmic takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// LOGARITHMIC takum encoding (Hunhold, 2024, arXiv:2404.18603, Definition 2;
// restated as Definition 1 of arXiv:2408.10594, the hardware codec paper).
//
//   The takum specification defines two variants that share an identical bit
//   layout and differ only in the value map:
//     - logarithmic takum, base sqrt(e): 2404.18603 Def. 2 / 2408.10594 Def. 1
//     - linear takum,      base 2:       2404.18603 Def. 8 / 2408.10594 Def. 2
//   This type implements the LOGARITHMIC variant, which 2404.18603 Sec 4.7
//   designates as the standard.  The naming follows the libtakum reference
//   implementation, which uses takum_log for this variant and the bare name for
//   the linear one.  See docs/takum-design.md.
//
// The bit layout, the (S, D, R, C, M) decode, the two's-complement ordering and
// the zero / NaR encodings are all owned by takum_codec<nbits, rbits>, shared
// verbatim with takum<>.  This class supplies only the value map:
//
//   m := 2^-p uint(M)  in [0,1)              (the mantissa)
//   l := (-1)^S (c + m)  in (-255, 255)      (the logarithmic value)
//   value := (-1)^S sqrt(e)^l
//
// Equivalently, for the positive representative reached by two's-complement
// negation, |value| = sqrt(e)^(c + m) = e^((c + m) / 2).
//
// Properties that distinguish this from the linear takum (arXiv:2404.18603):
//   - Proposition 7: incrementing the bit string yields the RECIPROCAL, so
//     negation and inversion are both single integer operations.  The linear
//     takum has no such bitwise inversion.
//   - Proposition 11: machine precision lambda(p) = sqrt(e)^(2^-p-1) - 1, which
//     is bounded by (2/3) of the linear takum's epsilon(p).
//
// PRECISION LIMITATION: like takum<>, conversion currently routes through
// double.  The logarithmic value l needs about p + 9 bits to be represented
// exactly, so for p > 44 or so (nbits beyond ~53) the double cannot carry the
// full mantissa and conversion loses low bits.  The reference implementation
// has the same constraint and uses long double for takum_log64.  Tracked with
// the native-arithmetic work in #1297.

#include <cstdint>       // the fixed-width integer types
#include <type_traits>   // std::enable_if
#include <cassert>
#include <iosfwd>     // std::ostream/std::istream in the friend declarations (#1334)
#include <universal/utility/icf_array_bounds.hpp>
#include <limits>
#include <cmath>

#include <universal/native/ieee754_core.hpp>   // the bit-manipulation half (#1334)
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <math/constexpr_math/exp.hpp>
#include <math/constexpr_math/log.hpp>

#include <universal/internal/bit_manipulation.hpp>
#include <universal/number/takum/takum_codec.hpp>
#include <universal/number/takum/takum_log_arithmetic.hpp>

namespace sw {	namespace universal {

// Forward definitions
template<unsigned nbits, unsigned rbits, typename bt> class takum_log;

// template class representing a logarithmic takum with two's complement encoding
template<unsigned _nbits, unsigned _rbits = 3, typename bt = uint8_t>
class takum_log {
public:
	typedef bt BlockType;

	// The field codec shared with the linear takum.  It owns the bit layout, the
	// DR/characteristic/mantissa geometry and the rounded encode path; this class
	// supplies only the logarithmic value map.
	using Codec = takum_codec<_nbits, _rbits>;

	static constexpr unsigned nbits    = _nbits;
	static constexpr unsigned rbits    = _rbits;
	static constexpr unsigned overhead = Codec::overhead;

	static constexpr unsigned dr_bits       = Codec::dr_bits;
	static constexpr unsigned nr_dr_values  = Codec::nr_dr_values;
	static constexpr unsigned max_r         = Codec::max_r;
	static constexpr unsigned r_mask        = Codec::r_mask;
	static constexpr unsigned dr_field_mask = Codec::dr_field_mask;
	static constexpr unsigned maxCharBits   = Codec::maxCharBits;

	// Does the format outrun a double?  l carries p fraction bits with p reaching
	// maxCharBits, so this is true exactly when nbits > 54 + rbits -- 57 for the
	// specified rbits = 3, which puts takum_log<64,3> and its 59 fraction bits on
	// the wrong side of a double's 53.  Those configurations evaluate addition and
	// subtraction in extended precision (takum_log_arithmetic.hpp); everything
	// narrower keeps the double path, where the conversion error stays well below
	// the format's own quantum.  Issue #1300.
	static constexpr bool wide_significand = (maxCharBits + 1u) > 53u;

	// The base of the value map.  std::numeric_limits<>::radix is a
	// static constexpr int and cannot express sqrt(e), so the value base is
	// exposed here instead; numeric_limits keeps the integer representation
	// radix.  See docs/takum-design.md.
	static constexpr double value_base   = 1.6487212707001281468;  // sqrt(e)
	static constexpr double log2_of_base = 0.7213475204444817;     // log2(sqrt(e)) == 1/(2 ln 2)

	// Codec geometry, re-exported so takum_log<> and takum<> present the same surface
	static constexpr unsigned dr_to_r(unsigned dr)      noexcept { return Codec::dr_to_r(dr); }
	static constexpr int64_t  dr_to_c_bias(unsigned dr) noexcept { return Codec::dr_to_c_bias(dr); }
	static constexpr unsigned find_dr(int64_t c)        noexcept { return Codec::find_dr(c); }
	static constexpr int64_t  max_characteristic()      noexcept { return Codec::max_characteristic(); }
	static constexpr int64_t  min_characteristic()      noexcept { return Codec::min_characteristic(); }

	static constexpr unsigned bitsInByte  = 8ull;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks    = (1 + ((nbits - 1) / bitsInBlock));
	static constexpr unsigned bitsInMSU   = (1 + ((nbits - 1) % bitsInBlock));
	static constexpr uint64_t storageMask = (0xFFFFFFFFFFFFFFFFull >> (64 - bitsInBlock));
	static constexpr unsigned MSU         = nrBlocks - 1;
	static constexpr bt       MSU_MASK    = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));
	static constexpr bt       SIGN_BIT_MASK = bt(1ull << ((nbits - 1ull) % bitsInBlock));

	using BlockBinary = blockbinary<nbits, bt, BinaryNumberType::Unsigned>;

	takum_log() = default;
	takum_log(const takum_log&) = default;
	takum_log(takum_log&&) = default;
	takum_log& operator=(const takum_log&) = default;
	takum_log& operator=(takum_log&&) = default;

	constexpr takum_log(const SpecificValue code) noexcept : _block{} {
		switch (code) {
		case SpecificValue::maxpos: maxpos(); break;
		case SpecificValue::minpos: minpos(); break;
		case SpecificValue::zero:
		default:                    zero();   break;
		case SpecificValue::minneg: minneg(); break;
		case SpecificValue::maxneg: maxneg(); break;
		case SpecificValue::infpos:
		case SpecificValue::infneg:
		case SpecificValue::nar:
		case SpecificValue::qnan:
		case SpecificValue::snan:   setnar();  break;
		}
	}

	// The constructor set mirrors the assignment set below; see takum_impl.hpp for
	// why a missing overload makes construction ambiguous while assignment still works.
	constexpr takum_log(signed char initial_value)        noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(short initial_value)              noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(int initial_value)                noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(long initial_value)               noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(long long initial_value)          noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(unsigned int initial_value)       noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(unsigned long initial_value)      noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(unsigned long long initial_value) noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(float initial_value)              noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(double initial_value)             noexcept : _block{} { *this = initial_value; }

	constexpr takum_log& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr takum_log& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr takum_log& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr takum_log& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr takum_log& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr takum_log& operator=(char rhs)               noexcept { return convert_unsigned(rhs); }
	constexpr takum_log& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr takum_log& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr takum_log& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr takum_log& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	CONSTEXPRESSION takum_log& operator=(float rhs)        noexcept { return convert_ieee754(rhs); }
	CONSTEXPRESSION takum_log& operator=(double rhs)       noexcept { return convert_ieee754(rhs); }

	explicit CONSTEXPRESSION operator int()       const noexcept { return to_signed<int>(); }
	explicit CONSTEXPRESSION operator long()      const noexcept { return to_signed<long>(); }
	explicit CONSTEXPRESSION operator long long() const noexcept { return to_signed<long long>(); }
	explicit CONSTEXPRESSION operator float()     const noexcept { return to_ieee754<float>(); }
	explicit CONSTEXPRESSION operator double()    const noexcept { return to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	CONSTEXPRESSION takum_log(long double initial_value)   noexcept : _block{} { *this = initial_value; }
	CONSTEXPRESSION takum_log& operator=(long double rhs)  noexcept { return convert_ieee754(rhs); }
	explicit CONSTEXPRESSION operator long double()  const noexcept { return to_ieee754<long double>(); }
#endif

	// prefix negation: two's complement negate (Proposition 6, shared with takum<>)
	constexpr takum_log operator-() const noexcept {
		if (iszero() || isnar()) return *this;
		takum_log result;
		result.setbits(((~raw_bits()) + 1ull) & nbits_mask());
		return result;
	}

	// Exact reciprocal (Proposition 7), unique to the logarithmic variant.
	//
	// value = (-1)^S sqrt(e)^l with l = (-1)^S (c + m), so 1/value keeps the sign
	// and negates l.  Negating l with S fixed means negating (c + m), which is the
	// two's complement of the D:R:C:M field -- the sign bit is left alone.  That
	// makes negation (two's complement of the whole word, Prop. 6) and inversion
	// (two's complement of everything but the sign) symmetric bit operations.
	//
	// The linear takum has no such inversion; see docs/takum-design.md.
	constexpr takum_log reciprocal() const noexcept {
		if (isnar()) return *this;
		if (iszero()) { takum_log r; r.setnar(); return r; }
		constexpr uint64_t field_mask = (1ull << (nbits - 1)) - 1ull;
		uint64_t raw   = raw_bits();
		uint64_t sign  = raw & (1ull << (nbits - 1));
		uint64_t field = raw & field_mask;
		takum_log result;
		result.setbits(sign | (((~field) + 1ull) & field_mask));
		return result;
	}

	// Addition and subtraction have no logarithmic shortcut -- unlike the multiply
	// and divide below, which are exact integer work on l -- so they must leave the
	// logarithmic domain and come back, at the cost of a transcendental or two.
	//
	// Narrow configurations do that through a double, which is fine there: the
	// operands' l carries p fraction bits, and while double(x) is never the exact
	// value of a takum_log (e^(l/2) is transcendental), the conversion error sits
	// far below the format's own quantum whenever p + 1 stays under 53.
	//
	// Wide ones do not have that luxury.  takum_log<64,3> carries 59 fraction bits
	// of l against a double's 53, so both operands were quantized before the
	// addition: measured against an 80-digit reference, 99.22% of 64-bit sums came
	// back incorrectly rounded, against 0.00% at 16 and 32 bits.  Those evaluate
	// the identity in an extended-precision local double-double instead, factoring
	// out the larger operand so the transcendentals only ever see a bounded
	// argument -- see takum_log_arithmetic.hpp.  Issue #1300.
	CONSTEXPRESSION takum_log& operator+=(const takum_log& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if constexpr (wide_significand) {
			return wide_sum(rhs, false);
		}
		else {
			return convert_ieee754(double(*this) + double(rhs));
		}
	}
	CONSTEXPRESSION takum_log& operator+=(double rhs) { return *this += takum_log(rhs); }
	CONSTEXPRESSION takum_log& operator-=(const takum_log& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if constexpr (wide_significand) {
			return wide_sum(rhs, true);
		}
		else {
			return convert_ieee754(double(*this) - double(rhs));
		}
	}
	CONSTEXPRESSION takum_log& operator-=(double rhs) { return *this -= takum_log(rhs); }
	// Multiplication and division are EXACT in the logarithmic domain, so neither
	// goes near a double.
	//
	// |x*y| = sqrt(e)^(lx + ly) and |x/y| = sqrt(e)^(lx - ly), and l is held as an
	// integer characteristic plus a p-bit fraction, so combining them is exact
	// integer arithmetic.  encode_fraction() then performs the single rounding the
	// result actually needs.
	//
	// The double route these replaced was not merely slower: it rounded both
	// operands to a double first, and takum_log<64,3> carries 59 fraction bits of
	// l against a double's 53.  Measured against a 113-bit reference, 99% of
	// wide-configuration products came back incorrectly rounded (issue #1300).
	// The narrow configurations were already correct and stay so; this path is
	// exact at every width, and needs no extended-precision type to be so.
	CONSTEXPRESSION takum_log& operator*=(const takum_log& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (iszero() || rhs.iszero()) { setzero(); return *this; }
		return combine_logarithmic(rhs, false, sign() != rhs.sign());
	}
	CONSTEXPRESSION takum_log& operator*=(double rhs) { return *this *= takum_log(rhs); }
	CONSTEXPRESSION takum_log& operator/=(const takum_log& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (rhs.iszero()) {
#if TAKUM_THROW_ARITHMETIC_EXCEPTION
			if (!std::is_constant_evaluated()) throw takum_divide_by_zero();
#endif
			setnar();
			return *this;
		}
		if (iszero()) { setzero(); return *this; }
		return combine_logarithmic(rhs, true, sign() != rhs.sign());
	}
	CONSTEXPRESSION takum_log& operator/=(double rhs) { return *this /= takum_log(rhs); }

	constexpr takum_log& operator++() noexcept {
		if (isnar()) return *this;
		uint64_t raw = raw_bits(), mask = nbits_mask();
		if (raw == (mask >> 1)) return *this;   // already maxpos
		setbits((raw + 1ull) & mask);
		return *this;
	}
	constexpr takum_log operator++(int) noexcept { takum_log tmp(*this); operator++(); return tmp; }
	constexpr takum_log& operator--() noexcept {
		if (isnar()) return *this;
		uint64_t raw = raw_bits();
		if (raw == ((1ull << (nbits - 1)) | 1ull)) return *this;  // already maxneg
		setbits((raw - 1ull) & nbits_mask());
		return *this;
	}
	constexpr takum_log operator--(int) noexcept { takum_log tmp(*this); operator--(); return tmp; }

	// modifiers
	constexpr void clear()                           noexcept { _block.clear(); }
	constexpr void setzero()                         noexcept { _block.clear(); }
	constexpr void setnar()                          noexcept { _block.clear(); setbit(nbits - 1); }
	constexpr void setnan(bool sign = false)         noexcept { (void)sign; setnar(); }
	constexpr void setinf(bool sign)                 noexcept { (sign ? maxneg() : maxpos()); }
	constexpr void setsign(bool s = true)            noexcept { setbit(nbits - 1, s); }
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		unsigned blockIndex = i / bitsInBlock;
		if (i < nbits) {
			bt block = _block[blockIndex];
			bt null  = bit_clear_mask<bt>(i, bitsInBlock);
			bt bit   = bt(v ? 1 : 0);
			bt mask  = bt(bit << (i % bitsInBlock));
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

	constexpr takum_log& maxpos() noexcept { clear(); flip(); setbit(nbits - 1, false); return *this; }
	constexpr takum_log& minpos() noexcept { clear(); setbit(0, true); return *this; }
	constexpr takum_log& zero()   noexcept { clear(); return *this; }
	constexpr takum_log& minneg() noexcept { clear(); flip(); return *this; }
	constexpr takum_log& maxneg() noexcept { clear(); setbit(nbits - 1, true); setbit(0, true); return *this; }

	// selectors
	constexpr bool iszero() const noexcept { return _block.iszero(); }
	constexpr bool isneg()  const noexcept { return _block.test(nbits - 1) && !isnar(); }
	constexpr bool ispos()  const noexcept { return !_block.test(nbits - 1) && !iszero(); }
	constexpr bool isinf()  const noexcept { return false; }
	constexpr bool isnan()  const noexcept { return isnar(); }
	constexpr bool isnar()  const noexcept {
		if (!_block.test(nbits - 1)) return false;
		for (unsigned i = 0; i < nrBlocks; ++i) {
			bt expected = (i == MSU) ? SIGN_BIT_MASK : bt(0);
			if (_block[i] != expected) return false;
		}
		return true;
	}
	constexpr bool sign() const noexcept { return _block.test(nbits - 1); }

	constexpr bool direct() const noexcept {
		return static_cast<bool>((magnitude_bits() >> (nbits - 2)) & 1);
	}
	constexpr unsigned regime() const noexcept {
		return static_cast<unsigned>((magnitude_bits() >> (nbits - overhead)) & r_mask);
	}
	constexpr unsigned dr_field() const noexcept {
		return static_cast<unsigned>((magnitude_bits() >> (nbits - overhead)) & dr_field_mask);
	}
	// The takum characteristic c: the integer part of the logarithmic value of
	// the magnitude, in units of the value base sqrt(e) -- NOT a power of two.
	constexpr int64_t characteristic() const noexcept {
		if (iszero() || isnar()) return 0;
		return Codec::characteristic_of(magnitude_bits());
	}
	// The logarithmic value l of the magnitude: |value| == sqrt(e)^l.
	CONSTEXPRESSION double logarithmic_value() const noexcept {
		if (iszero() || isnar()) return 0.0;
		auto d = Codec::decode(magnitude_bits());
		return static_cast<double>(d.c) + d.fraction();
	}
	// scale(): the integer power-of-two exponent, per the library-wide convention
	// that scale() is base 2 for every number system.  The characteristic is in
	// units of sqrt(e), so it is converted here rather than returned directly.
	//
	// Returns int64_t, as takum<>::scale() does.  An int cannot hold the result: at
	// rbits = 5 the characteristic reaches ~2^32, so takum_log<16,5> maxpos scales to
	// ~3.1e9 and the narrowing conversion is undefined behaviour -- UBSan reported
	// maxpos as INT_MIN and minpos as INT_MAX, an inverted range.  Widening the return
	// type removes the conversion rather than clamping it, and manipulators.hpp's
	// components() reaches this for every takum_log configuration.
	//
	// Accuracy: sqrt(e)^l is a power of two only for l = 2k ln2, which is irrational
	// for k != 0, so no encoding sits exactly on a boundary and the floor below is
	// always well defined.  It is computed in double, though, which at |l| ~ 255
	// leaves ~1e-13 of absolute slack; an encoding closer than that to a power of two
	// can floor to the neighbouring exponent.  Deciding those cases needs the exact
	// (c, M_bits) pair carried at more than double precision -- a follow-up if a
	// caller ever needs it.  Note that std::ilogb(double(*this)) is NOT the fix: it
	// agrees with this on every uniformly sampled encoding, is no better on the
	// boundary set (both are coin flips there), and returns INT_MAX for the ~34% of
	// wide-rbits encodings whose magnitude overflows a double.
	CONSTEXPRESSION int64_t scale() const noexcept {
		if (iszero() || isnar()) return 0;
		double  log2_magnitude = logarithmic_value() * log2_of_base;
		int64_t s = static_cast<int64_t>(log2_magnitude);
		if (log2_magnitude < 0.0 && static_cast<double>(s) != log2_magnitude) --s;  // floor
		return s;
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
	constexpr bt block(unsigned b) const noexcept { return (b < nrBlocks) ? _block[b] : bt(0); }
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

	constexpr uint64_t raw_bits() const noexcept {
		uint64_t raw = 0;
		for (unsigned i = 0; i < nrBlocks; ++i) raw |= (static_cast<uint64_t>(_block[i]) << (i * bitsInBlock));
		return raw;
	}
	constexpr uint64_t magnitude_bits() const noexcept {
		uint64_t raw = raw_bits();
		if (raw & (1ull << (nbits - 1))) raw = ((~raw) + 1) & nbits_mask();
		return raw;
	}

protected:
	// Combine two logarithmic values exactly: l = lx + ly for a product, lx - ly
	// for a quotient.  Both are exact binary fractions over 2^p, so aligning them
	// to a common denominator and adding is exact integer arithmetic -- no double,
	// no extended-precision type, and one rounding at the end.
	//
	// Pre: neither operand is zero or NaR; the caller has handled those and knows
	//      the sign of the result.
	CONSTEXPRESSION takum_log& combine_logarithmic(const takum_log& rhs, bool subtract,
	                                               bool negative) noexcept {
		auto a = Codec::decode(magnitude_bits());
		auto b = Codec::decode(rhs.magnitude_bits());

		// Align the two fractions on the wider denominator.  Widening is exact, and
		// p never exceeds 59, so the shifted numerators stay well inside uint64_t.
		const unsigned q = (a.p > b.p) ? a.p : b.p;
		const uint64_t Na = a.M_bits << (q - a.p);
		const uint64_t Nb = b.M_bits << (q - b.p);
		const uint64_t den = (q > 0) ? (1ull << q) : 1ull;

		int64_t  c = subtract ? (a.c - b.c) : (a.c + b.c);
		uint64_t N = 0;
		if (subtract) {
			if (Na >= Nb) {
				N = Na - Nb;
			}
			else {
				// borrow one whole unit from the characteristic
				N = den - (Nb - Na);
				--c;
			}
		}
		else {
			N = Na + Nb;                       // at most 2^60, no overflow
			if (q > 0 && N >= den) { N -= den; ++c; }
		}

		auto enc = Codec::encode_fraction(c, N, q);
		if (enc.overflowed())  { if (negative) maxneg(); else maxpos(); return *this; }
		if (enc.underflowed()) { setzero(); return *this; }
		// the codec never sets the sign bit (I4); apply it here
		setbits(negative ? (((~enc.magnitude) + 1ull) & nbits_mask()) : enc.magnitude);
		return *this;
	}

	constexpr takum_log& flip() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) _block.setblock(i, bt(~_block[i]));
		_block.setblock(MSU, bt(_block[MSU] & MSU_MASK));
		return *this;
	}
	static constexpr uint64_t nbits_mask() noexcept { return Codec::nbits_mask(); }

	// Extended-precision addition / subtraction for wide configurations.  The zero
	// cases are peeled off first: takum_log_arith::combine() decodes an l and a
	// zero has none, and x +/- 0 must return x unchanged rather than route through
	// a conversion that would quantize it -- the symptom #1300 opened on.
	takum_log& wide_sum(const takum_log& rhs, bool subtract) {
		if (rhs.iszero()) return *this;
		if (iszero()) { *this = (subtract ? -rhs : rhs); return *this; }

		const auto da = Codec::decode(magnitude_bits());
		const auto db = Codec::decode(rhs.magnitude_bits());
		const auto la = takum_log_arith::exact_l(da.c, da.M_bits, da.p);
		const auto lb = takum_log_arith::exact_l(db.c, db.M_bits, db.p);

		const auto r = takum_log_arith::combine(la, sign(), lb, rhs.sign() != subtract);
		if (r.zero)     { setzero(); return *this; }
		if (r.take_big) {
			// the smaller operand could not move the result
			const bool aBig = !takum_log_arith::less(la, lb);
			if (!aBig) *this = (subtract ? -rhs : rhs);
			return *this;
		}

		const auto s = takum_log_arith::to_integer_fraction(r.l);
		const auto enc = Codec::encode_fraction(s.c, s.N, takum_log_arith::qbits);
		if (enc.overflowed())  { if (r.sign) maxneg(); else maxpos(); return *this; }
		if (enc.underflowed()) { setzero(); return *this; }
		setbits(r.sign ? (((~enc.magnitude) + 1ull) & nbits_mask()) : enc.magnitude);
		return *this;
	}

	template<typename SignedInt>
	CONSTEXPRESSION takum_log& convert_signed(SignedInt rhs) noexcept { return convert_ieee754(double(rhs)); }
	template<typename UnsignedInt>
	CONSTEXPRESSION takum_log& convert_unsigned(UnsignedInt rhs) noexcept { return convert_ieee754(double(rhs)); }

	/// Convert an IEEE-754 value to the logarithmic takum encoding.
	///
	/// |x| = sqrt(e)^l  =>  l = log_sqrt(e)|x| = 2 ln|x|.  The pair (c, m) with
	/// c = floor(l) and m = l - c is exactly what the shared codec encodes, so
	/// everything from here -- rounding, carry, saturation -- is common with the
	/// linear takum.
	template<typename Real>
	CONSTEXPRESSION takum_log& convert_ieee754(Real rhs) noexcept {
		static_assert(nbits <= 64, "takum_log > 64 bits not yet supported");

		if (rhs != rhs) { setnar(); return *this; }
		if constexpr (std::numeric_limits<Real>::has_infinity) {
			if (rhs ==  std::numeric_limits<Real>::infinity()) { setnar(); return *this; }
			if (rhs == -std::numeric_limits<Real>::infinity()) { setnar(); return *this; }
		}
		if (rhs == Real(0)) { setzero(); return *this; }

		bool s = (rhs < Real(0));
		double abs_v = static_cast<double>(s ? -rhs : rhs);

		// l = 2 ln|x|, the logarithmic value in units of the base sqrt(e).
		// std::log is not constexpr, so constant evaluation uses the library's
		// constexpr log; at runtime std::log is both available and more accurate.
		double l;
		if (std::is_constant_evaluated()) {
			l = 2.0 * sw::math::constexpr_math::log(abs_v);
		}
		else {
			l = 2.0 * std::log(abs_v);
		}

		// Split into characteristic and mantissa: c = floor(l), m = l - c in [0,1)
		int64_t c = static_cast<int64_t>(l);
		if (l < 0.0 && static_cast<double>(c) != l) --c;          // floor toward -inf
		double m = l - static_cast<double>(c);
		if (m < 0.0) m = 0.0;                                      // guard against fp drift
		if (m >= 1.0) { m = 0.0; ++c; }

		// Evaluating 2 ln|x| carries roughly |l| * 2^-52 of absolute error, so a
		// value sitting exactly on a characteristic boundary can land a hair below
		// it and be truncated to the previous characteristic.  Snap to the boundary
		// when within that error.  The tolerance stays far below the smallest
		// resolvable mantissa step 2^-p for every p a double can actually carry, so
		// this cannot absorb a genuinely distinct value.
		const double tol = ((l < 0.0) ? -l : l) * 4e-16 + 4e-16;
		if (m > 1.0 - tol) { m = 0.0; ++c; }
		else if (m < tol)  { m = 0.0; }

		auto enc = Codec::encode_rounded(c, m);
		if (enc.overflowed()) {
			if (s) maxneg(); else maxpos();
			return *this;
		}
		if (enc.underflowed()) { setzero(); return *this; }

		// The codec never sets the sign bit (I4); two's-complement negate here.
		setbits(s ? (((~enc.magnitude) + 1ull) & nbits_mask()) : enc.magnitude);
		return *this;
	}

	template<typename SignedInt>
	CONSTEXPRESSION typename std::enable_if<std::is_integral<SignedInt>::value && std::is_signed<SignedInt>::value, SignedInt>::type
		to_signed() const noexcept { return SignedInt(to_ieee754<double>()); }
	template<typename UnsignedInt>
	CONSTEXPRESSION typename std::enable_if<std::is_integral<UnsignedInt>::value && std::is_unsigned<UnsignedInt>::value, UnsignedInt>::type
		to_unsigned() const noexcept { return UnsignedInt(to_ieee754<double>()); }

	/// Decode to an IEEE-754 value: |x| = sqrt(e)^l = e^(l/2).
	template<typename TargetFloat>
	CONSTEXPRESSION TargetFloat to_ieee754() const noexcept {
		if (iszero()) return TargetFloat(0);
		if (isnar())  return std::numeric_limits<TargetFloat>::quiet_NaN();
		static_assert(nbits <= 64, "takum_log > 64 bits not yet supported");

		bool s = sign();

		// Shared codec: magnitude -> (c, m).  Only the value map below is
		// specific to the logarithmic takum.
		auto d = Codec::decode(magnitude_bits());
		double l = static_cast<double>(d.c) + d.fraction();

		// LOGARITHMIC value map: |value| = sqrt(e)^l = e^(l/2).
		// As in convert_ieee754, std::exp is not constexpr, so constant evaluation
		// uses the library's constexpr exp and runtime uses the more accurate one.
		double magnitude;
		if (std::is_constant_evaluated()) {
			magnitude = sw::math::constexpr_math::exp(l * 0.5);
		}
		else {
			magnitude = std::exp(l * 0.5);
		}
		TargetFloat value = static_cast<TargetFloat>(magnitude);
		if (s) value = -value;
		return value;
	}

private:
	BlockBinary _block;

	template<unsigned nn, unsigned nr, typename nb>
	friend std::ostream& operator<< (std::ostream& ostr, const takum_log<nn, nr, nb>& r);
	template<unsigned nn, unsigned nr, typename nb>
	friend std::istream& operator>> (std::istream& istr, takum_log<nn, nr, nb>& r);
	template<unsigned nn, unsigned nr, typename nb>
	friend constexpr bool operator==(const takum_log<nn, nr, nb>& lhs, const takum_log<nn, nr, nb>& rhs) noexcept;
	template<unsigned nn, unsigned nr, typename nb>
	friend constexpr bool operator< (const takum_log<nn, nr, nb>& lhs, const takum_log<nn, nr, nb>& rhs) noexcept;
};

// return the Unit in the Last Position
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum_log<nbits, rbits, bt> ulp(const takum_log<nbits, rbits, bt>& a) {
	takum_log<nbits, rbits, bt> b(a);
	return ++b - a;
}




template<unsigned nn, unsigned nr, typename nb>
inline constexpr bool operator==(const takum_log<nn, nr, nb>& lhs, const takum_log<nn, nr, nb>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return false;
	return (lhs._block == rhs._block);
}
template<unsigned nn, unsigned nr, typename nb>
inline constexpr bool operator!=(const takum_log<nn, nr, nb>& lhs, const takum_log<nn, nr, nb>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return true;
	return !operator==(lhs, rhs);
}
template<unsigned nn, unsigned nr, typename nb>
inline constexpr bool operator< (const takum_log<nn, nr, nb>& lhs, const takum_log<nn, nr, nb>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return false;
	// Two's-complement ordering equals value ordering for real encodings (Prop. 4).
	uint64_t l = lhs.raw_bits(), r = rhs.raw_bits();
	uint64_t sign_ext = (nn < 64) ? ~((1ull << nn) - 1) : 0ull;
	int64_t ls = (l & (1ull << (nn - 1))) ? static_cast<int64_t>(l | sign_ext) : static_cast<int64_t>(l);
	int64_t rs = (r & (1ull << (nn - 1))) ? static_cast<int64_t>(r | sign_ext) : static_cast<int64_t>(r);
	return ls < rs;
}
template<unsigned nn, unsigned nr, typename nb>
inline constexpr bool
operator> (const takum_log<nn, nr, nb>& lhs, const takum_log<nn, nr, nb>& rhs) noexcept {
	return operator< (rhs, lhs);
}
template<unsigned nn, unsigned nr, typename nb>
inline constexpr bool operator<=(const takum_log<nn, nr, nb>& lhs, const takum_log<nn, nr, nb>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return false;
	return !operator>(lhs, rhs);
}
template<unsigned nn, unsigned nr, typename nb>
inline constexpr bool operator>=(const takum_log<nn, nr, nb>& lhs, const takum_log<nn, nr, nb>& rhs) noexcept {
	if (lhs.isnar() || rhs.isnar()) return false;
	return !operator<(lhs, rhs);
}

template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum_log<nbits, rbits, bt> operator+(const takum_log<nbits, rbits, bt>& lhs,
	const takum_log<nbits, rbits, bt>& rhs) {
	takum_log<nbits, rbits, bt> sum(lhs); sum += rhs; return sum;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum_log<nbits, rbits, bt> operator-(const takum_log<nbits, rbits, bt>& lhs,
	const takum_log<nbits, rbits, bt>& rhs) {
	takum_log<nbits, rbits, bt> diff(lhs); diff -= rhs; return diff;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum_log<nbits, rbits, bt> operator*(const takum_log<nbits, rbits, bt>& lhs,
	const takum_log<nbits, rbits, bt>& rhs) {
	takum_log<nbits, rbits, bt> mul(lhs); mul *= rhs; return mul;
}
template<unsigned nbits, unsigned rbits, typename bt>
inline CONSTEXPRESSION takum_log<nbits, rbits, bt> operator/(const takum_log<nbits, rbits, bt>& lhs,
	const takum_log<nbits, rbits, bt>& rhs) {
	takum_log<nbits, rbits, bt> ratio(lhs); ratio /= rhs; return ratio;
}

template<unsigned nbits, unsigned rbits, typename bt>
constexpr takum_log<nbits, rbits, bt> abs(const takum_log<nbits, rbits, bt>& v) noexcept {
	return v.isneg() ? -v : v;
}

}} // namespace sw::universal
