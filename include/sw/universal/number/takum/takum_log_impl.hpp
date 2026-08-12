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

#include <cassert>
#include <limits>
#include <cmath>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <math/constexpr_math/exp.hpp>
#include <math/constexpr_math/log.hpp>

#include <universal/internal/bit_manipulation.hpp>
#include <universal/number/takum/takum_codec.hpp>

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

	constexpr takum_log(signed char initial_value)        noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(short initial_value)              noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(int initial_value)                noexcept : _block{} { *this = initial_value; }
	constexpr takum_log(long long initial_value)          noexcept : _block{} { *this = initial_value; }
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

	// in-place arithmetic via double conversion, matching takum<>'s current
	// approach.  Native logarithmic arithmetic -- where multiply/divide become
	// fixed-point add/subtract on l -- is tracked in #1297.
	CONSTEXPRESSION takum_log& operator+=(const takum_log& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		return convert_ieee754(double(*this) + double(rhs));
	}
	CONSTEXPRESSION takum_log& operator+=(double rhs) { return *this += takum_log(rhs); }
	CONSTEXPRESSION takum_log& operator-=(const takum_log& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		return convert_ieee754(double(*this) - double(rhs));
	}
	CONSTEXPRESSION takum_log& operator-=(double rhs) { return *this -= takum_log(rhs); }
	CONSTEXPRESSION takum_log& operator*=(const takum_log& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		return convert_ieee754(double(*this) * double(rhs));
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
		return convert_ieee754(double(*this) / double(rhs));
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
	CONSTEXPRESSION int scale() const noexcept {
		if (iszero() || isnar()) return 0;
		double log2_magnitude = logarithmic_value() * log2_of_base;
		int    s = static_cast<int>(log2_magnitude);
		if (log2_magnitude < 0.0 && static_cast<double>(s) != log2_magnitude) --s;  // floor
		return s;
	}

	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false;
		bt word = _block[bitIndex / bitsInBlock];
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
	constexpr takum_log& flip() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) _block.setblock(i, bt(~_block[i]));
		_block.setblock(MSU, bt(_block[MSU] & MSU_MASK));
		return *this;
	}
	static constexpr uint64_t nbits_mask() noexcept { return Codec::nbits_mask(); }

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

template<unsigned nbits, unsigned rbits, typename bt>
std::string to_binary(const takum_log<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	using T = takum_log<nbits, rbits, bt>;
	std::stringstream s;
	uint64_t mag = number.magnitude_bits();

	s << "0b";
	s << (number.sign() ? "1." : "0.");
	bool D = static_cast<bool>((mag >> (nbits - 2)) & 1);
	s << (D ? "1." : "0.");

	unsigned regime = static_cast<unsigned>((mag >> (nbits - T::overhead)) & T::r_mask);
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) s << ((regime >> i) & 1 ? '1' : '0');
	s << '.';

	auto g = T::Codec::layout_of(number.dr_field());
	int bit = static_cast<int>(nbits) - static_cast<int>(T::overhead) - 1;
	for (unsigned i = 0; i < g.c_stored_bits && bit >= 0; ++i) {
		s << ((mag >> bit) & 1 ? '1' : '0');
		--bit;
		if (i < g.c_stored_bits - 1 && ((g.c_stored_bits - 1 - i) % 4) == 0 && nibbleMarker) s << '\'';
	}
	s << '.';
	for (unsigned i = 0; i < g.p && bit >= 0; ++i) {
		s << ((mag >> bit) & 1 ? '1' : '0');
		if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
		--bit;
	}
	return s.str();
}

template<unsigned nbits, unsigned rbits, typename bt>
std::string to_native(const takum_log<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
}

////////////////////// operators
template<unsigned nn, unsigned nr, typename nb>
inline std::ostream& operator<<(std::ostream& ostr, const takum_log<nn, nr, nb>& v) { ostr << double(v); return ostr; }
template<unsigned nn, unsigned nr, typename nb>
inline std::istream&
operator>>(std::istream& istr, takum_log<nn, nr, nb>& v) {
	double d; istr >> d; v = d; return istr;
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
