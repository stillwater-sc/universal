#pragma once
// hfloat_impl.hpp: implementation of IBM System/360 hexadecimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// IBM System/360 Hexadecimal Floating-Point (1964):
//   Format: [sign(1)] [exponent(es)] [hex_fraction(ndigits*4 bits)]
//   Value:  (-1)^sign * 16^(exponent - bias) * 0.f1f2...fn
//
// Key properties:
//   - No hidden bit (fraction always has explicit leading hex digit)
//   - No NaN, no infinity, no subnormals
//   - Truncation rounding only (never rounds up)
//   - Overflow saturates to maxpos/maxneg
//   - Zero: sign=0, exponent=0, fraction=0
//   - Wobbling precision: 0-3 leading zero bits in MSB hex digit
//
// Standard configurations:
//   Short:    hfloat<6, 7>  = 1+7+24 = 32 bits
//   Long:     hfloat<14, 7> = 1+7+56 = 64 bits
//   Extended: hfloat<28, 7> = 1+7+112 = 120 bits (stored in 128)


// Behavioural switches default HERE, beside the code they govern, rather than in the
// hfloat.hpp umbrella: including core.hpp directly would otherwise leave them
// undefined, #if would evaluate them as 0, and the switch would silently flip on that
// path -- the trap #1390 hit with POSIT_ENABLE_LITERALS (#1334, #1436).
#if !defined(HFLOAT_ENABLE_LITERALS)
#define HFLOAT_ENABLE_LITERALS 1
#endif
#if !defined(HFLOAT_THROW_ARITHMETIC_EXCEPTION)
#define HFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#endif
#include <universal/utility/exceptions_check.hpp>  // reject a throw switch without exceptions (#1925)
#if !defined(HFLOAT_EXCEPT)
#if HFLOAT_THROW_ARITHMETIC_EXCEPTION
#define HFLOAT_EXCEPT
#else
#define HFLOAT_EXCEPT noexcept
#endif
#endif

#include <cctype>
#include <cstdint>
#include <cstdio>        // std::snprintf in str(); keeps <sstream> out of the core
#include <cstring>
#include <cmath>
#include <limits>        // std::numeric_limits
#include <string>
#include <string_view>
#include <type_traits>
#include <algorithm>

// supporting types and functions
#include <universal/native/ieee754_core.hpp>   // the stream-free half of <universal/native/ieee754.hpp>
#include <universal/utility/decimal_to_binary.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/hfloat/hfloat_fraction.hpp>   // the wide fraction register (#2105)
#include <universal/native/constexpr_ldexp.hpp>          // the exact scaling of the rounded double
// hfloat exception structure
#include <universal/number/hfloat/exceptions.hpp>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////
// hfloat: IBM System/360 hexadecimal floating-point number
//
// Template parameters:
//   ndigits  - number of hexadecimal fraction digits
//   es       - exponent bits (7 for standard IBM HFP)
//   bt       - block type for storage
//
template<unsigned _ndigits, unsigned _es, typename bt = std::uint32_t>
class hfloat {
public:
	static constexpr unsigned ndigits     = _ndigits;            // hex fraction digits
	static constexpr unsigned es          = _es;                 // exponent bits
	static constexpr unsigned fbits       = ndigits * 4u;        // fraction bits
	static constexpr unsigned nbits       = 1u + es + fbits;     // total bits
	static constexpr int      bias        = (1 << (es - 1));     // exponent bias (64 for es=7)
	static constexpr int      emax        = (1 << es) - 1 - bias; // max unbiased exponent
	static constexpr int      emin        = -bias;                // min unbiased exponent

	typedef bt BlockType;

	static constexpr unsigned bitsInByte  = 8u;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks    = 1u + ((nbits - 1u) / bitsInBlock);
	static constexpr unsigned MSU         = nrBlocks - 1u;

	static constexpr bt ALL_ONES  = bt(~0);
	static constexpr bt MSU_MASK  = (nrBlocks * bitsInBlock == nbits) ? ALL_ONES : bt((1ull << (nbits % bitsInBlock)) - 1u);
	static constexpr bt BLOCK_MASK = bt(~0);

	// working registers (#2105): the full fraction, the whole encoding, and a 64-bit source
	using frac_t = hfloat_detail::wide<fbits>;
	using raw_t  = hfloat_detail::wide<nbits>;
	static constexpr unsigned wide64 = (fbits > 64u) ? fbits : 64u;

	/// trivial constructor
	hfloat() = default;

	hfloat(const hfloat&) = default;
	hfloat(hfloat&&) = default;

	hfloat& operator=(const hfloat&) = default;
	hfloat& operator=(hfloat&&) = default;

	// specific value constructor
	constexpr hfloat(const SpecificValue code) noexcept : _block{} {
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
			maxpos();  // no infinity in HFP, saturate to maxpos
			break;
		case SpecificValue::infneg:
			maxneg();  // no infinity in HFP, saturate to maxneg
			break;
		case SpecificValue::nar:
		case SpecificValue::qnan:
		case SpecificValue::snan:
			zero();    // no NaN in HFP, map to zero
			break;
		}
	}

	// initializers for native types
	constexpr explicit hfloat(signed char iv)           noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(short iv)                 noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(int iv)                   noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(long iv)                  noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(long long iv)             noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(char iv)                  noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(unsigned short iv)        noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(unsigned int iv)          noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(unsigned long iv)         noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(unsigned long long iv)    noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(float iv)                 noexcept : _block{} { *this = iv; }
	constexpr explicit hfloat(double iv)                noexcept : _block{} { *this = iv; }

	// assignment operators for native types
	constexpr hfloat& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr hfloat& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr hfloat& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr hfloat& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr hfloat& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	// Plain `char` may be signed or unsigned per platform; route through
	// the signed conversion via integer promotion so hfloat(char(-1)) on
	// signed-char targets yields -1, not UCHAR_MAX (CodeRabbit finding
	// from PR #805 applied to the sibling type).
	constexpr hfloat& operator=(char rhs)               noexcept { return convert_signed(static_cast<int>(rhs)); }
	constexpr hfloat& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr hfloat& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr hfloat& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr hfloat& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr hfloat& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr hfloat& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators
	constexpr explicit operator float()           const noexcept { return float(convert_to_double()); }
	constexpr explicit operator double()          const noexcept { return convert_to_double(); }

#if LONG_DOUBLE_SUPPORT
	constexpr explicit hfloat(long double iv)           noexcept : _block{} { *this = iv; }
	constexpr hfloat& operator=(long double rhs)        noexcept { return convert_long_double(rhs); }
	constexpr explicit operator long double()     const noexcept { return (long double)convert_to_double(); }
#endif

	// Assign from a normalized 64-bit binary mantissa plus its "frexp" exponent:
	// value = mantissa64 * 2^(bin_exp - 64), truncated to ndigits hex digits. parse() uses it
	// for fbits <= 64.
	constexpr hfloat& assign_from_mantissa64(bool negative, int bin_exp, uint64_t mantissa64) noexcept {
		if (mantissa64 == 0) { setzero(); return *this; }
		normalize_chop(negative, 0, hfloat_detail::wide<wide64>::from_u64(mantissa64), 64 - bin_exp);
		return *this;
	}

	// Assign from a wide decimal_to_binary result (target_mantissa_bits == fbits), used by
	// parse() when fbits > 64: d.mantissa has its MSB at bit fbits - 1 of weight 2^(bin_exp - 1),
	// so value = mantissa * 2^(bin_exp - fbits), truncated to ndigits hex digits.
	template<unsigned BigBits>
	constexpr hfloat& assign_from_wide_mantissa(bool negative, int bin_exp,
	    const ::sw::universal::decimal_to_binary::basic_result<BigBits>& d) noexcept {
		frac_t m;
		for (unsigned i = 0; i < fbits; ++i) {
			if (d.mantissa.at(i)) m.set(i);
		}
		normalize_chop(negative, 0, m, static_cast<int>(fbits) - bin_exp);
		return *this;
	}

	// prefix operators
	constexpr hfloat operator-() const {
		hfloat negated(*this);
		if (!negated.iszero()) {
			negated.setsign(!negated.sign());
		}
		return negated;
	}

	// arithmetic operators
	//
	// Every result is the exact value truncated toward zero to ndigits hex digits (#2105). The
	// operands keep all fbits fraction bits, hfp128's 112 included; they were cut to the low 64.
	//
	// Addition aligns the smaller operand in a register with two guard digits. Bits shifted
	// past it are folded into a sticky bit: a sum floors the same way without them, and a
	// difference takes one guard unit off for them, which floors the exact difference. A shift
	// of 16 or more hex digits used to shift a uint64_t by >= 64 bits (UB), and every bit that
	// fell off the operand was lost even when it decided the truncated result.
	constexpr hfloat& operator+=(const hfloat& rhs) {
		bool as, bs; int ae, be; frac_t fa, fb;
		decode_normalized(as, ae, fa);
		rhs.decode_normalized(bs, be, fb);
		if (fb.iszero()) return *this;
		if (fa.iszero()) { *this = rhs; return *this; }

		constexpr unsigned guard = 8u;                        // two guard hex digits
		using acc_t = hfloat_detail::wide<fbits + 4u + guard>; // and one carry digit
		acc_t a = fa.template resize<fbits + 4u + guard>();
		acc_t b = fb.template resize<fbits + 4u + guard>();
		a.shl(guard);
		b.shl(guard);
		int e = ae;
		int d = ae - be;
		if (d < 0) {
			acc_t t = a; a = b; b = t;
			bool ts = as; as = bs; bs = ts;
			e = be;
			d = -d;
		}
		const bool sticky = b.shr(4u * static_cast<unsigned>(d));
		bool rs = as;
		if (as == bs) {
			a.add(b);
		}
		else {
			const int c = a.compare(b);
			if (c == 0 && !sticky) { setzero(); return *this; }
			if (c < 0) {                // only for a larger operand that is not normalized
				b.sub(a);
				a = b;
				rs = bs;
			}
			else {
				a.sub(b);
				if (sticky) a.decrement();
			}
		}
		normalize_chop(rs, e, a, static_cast<int>(fbits + guard));
		return *this;
	}

	constexpr hfloat& operator-=(const hfloat& rhs) {
		hfloat neg(rhs);
		if (!neg.iszero()) neg.setsign(!neg.sign());
		return operator+=(neg);
	}

	// the exact 2*fbits-bit product, truncated
	constexpr hfloat& operator*=(const hfloat& rhs) {
		bool as, bs; int ae, be; frac_t fa, fb;
		decode_normalized(as, ae, fa);
		rhs.decode_normalized(bs, be, fb);
		if (fa.iszero() || fb.iszero()) { setzero(); return *this; }
		const auto p = hfloat_detail::multiply(fa, fb);       // 0.fa * 0.fb = p * 2^(-2 fbits)
		normalize_chop(as != bs, ae + be, p, static_cast<int>(2u * fbits));
		return *this;
	}

	// q = floor(fa * 2^(fbits + 4) / fb) has at least fbits + 1 bits (fa / fb > 1/16), so its
	// truncation to ndigits hex digits is the truncation of the exact quotient
	constexpr hfloat& operator/=(const hfloat& rhs) {
		if (rhs.iszero()) {
#if HFLOAT_THROW_ARITHMETIC_EXCEPTION
			// Throwing in a constant expression is ill-formed; fence the
			// throw under runtime to keep operator/= constexpr-callable.
			// At constant-eval, divide-by-zero falls through to setzero()
			// (IBM HFP has no NaN/inf, so saturation is the closest
			// constexpr-safe behavior).
			if (!std::is_constant_evaluated()) {
				throw hfloat_divide_by_zero();
			}
#endif
			setzero();
			return *this;
		}
		bool as, bs; int ae, be; frac_t fa, fb;
		decode_normalized(as, ae, fa);
		rhs.decode_normalized(bs, be, fb);
		if (fa.iszero() || fb.iszero()) { setzero(); return *this; }
		using num_t = hfloat_detail::wide<2u * fbits + 4u>;
		num_t n = fa.template resize<2u * fbits + 4u>();
		n.shl(fbits + 4u);
		const num_t q = hfloat_detail::divide(n, fb);
		normalize_chop(as != bs, ae - be, q, static_cast<int>(fbits + 4u));
		return *this;
	}

	// unary operators
	constexpr hfloat& operator++() {
		*this += hfloat(1);
		return *this;
	}
	constexpr hfloat operator++(int) {
		hfloat tmp(*this);
		operator++();
		return tmp;
	}
	constexpr hfloat& operator--() {
		*this -= hfloat(1);
		return *this;
	}
	constexpr hfloat operator--(int) {
		hfloat tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear() noexcept {
		for (unsigned i = 0; i < nrBlocks; ++i) _block[i] = bt(0);
	}
	constexpr void setzero() noexcept { clear(); }

	constexpr void setsign(bool negative = true) noexcept {
		setbit(nbits - 1, negative);
	}

	// use un-interpreted raw bits to set the value
	constexpr void setbits(uint64_t value) noexcept {
		clear();
		for (unsigned i = 0; i < nrBlocks; ++i) {
			_block[i] = bt(value & BLOCK_MASK);
			// a 64-bit block takes the whole value: shifting a uint64_t by 64 is UB (#2105)
			if constexpr (bitsInBlock < 64u) value >>= bitsInBlock;
			else value = 0;
		}
		_block[MSU] &= MSU_MASK;
	}

	// create specific number system values of interest
	//
	// IBM HFP has no subnormals (has_denorm == denorm_absent), so minpos
	// and denorm_min coincide at the smallest normalized positive value:
	// leading hex digit = 1, smallest exponent. maxpos sets every fraction
	// bit (all 28 hex digits = F for hfp128).
	constexpr hfloat& maxpos() noexcept {
		frac_t f;
		for (unsigned i = 0; i < fbits; ++i) f.set(i);
		encode(false, emax, f);
		return *this;
	}
	constexpr hfloat& minpos() noexcept {
		// Smallest normalized positive: leading hex digit = 1, i.e. bit (fbits - 4) set, all
		// other fraction bits zero. Value = 16^(emin-1).
		frac_t f;
		f.set(fbits - 4u);
		encode(false, emin, f);
		return *this;
	}
	constexpr hfloat& zero() noexcept {
		clear();
		return *this;
	}
	constexpr hfloat& minneg() noexcept {
		frac_t f;
		f.set(fbits - 4u);
		encode(true, emin, f);
		return *this;
	}
	constexpr hfloat& maxneg() noexcept {
		frac_t f;
		for (unsigned i = 0; i < fbits; ++i) f.set(i);
		encode(true, emax, f);
		return *this;
	}

	// selectors
	constexpr bool sign() const noexcept {
		return getbit(nbits - 1);
	}

	constexpr bool iszero() const noexcept {
		// zero when fraction is all zeros (exponent and sign don't matter)
		// In IBM HFP, zero is represented as all-zeros
		for (unsigned i = 0; i < nrBlocks; ++i) {
			bt mask = (i == MSU) ? bt(MSU_MASK & ~(bt(1) << ((nbits - 1) % bitsInBlock))) : BLOCK_MASK;
			if ((_block[i] & mask) != 0) return false;
		}
		return true;
	}

	constexpr bool isone() const noexcept {
		bool s; int e; frac_t f;
		decode(s, e, f);
		// 1.0 = 0.1 * 16^1: e = 1 and only the low bit of the leading hex digit set
		frac_t one;
		one.set(fbits - 4u);
		return !s && (e == 1) && (f.compare(one) == 0);
	}

	constexpr bool ispos() const noexcept { return !sign(); }
	constexpr bool isneg() const noexcept { return sign(); }

	// IBM HFP has no NaN or infinity
	constexpr bool isinf() const noexcept { return false; }
	constexpr bool isnan() const noexcept { return false; }
	constexpr bool isnan(int) const noexcept { return false; }

	constexpr int scale() const noexcept {
		bool s; int e; frac_t f;
		decode(s, e, f);
		(void)s;
		const int leading = f.msb();
		if (leading < 0) return 0;
		// value = 0.f * 16^e: the leading bit at fraction position `leading` weighs 2^(4e + leading - fbits)
		return 4 * e + leading - static_cast<int>(fbits);
	}

	// convert to string
	//
	// This was `std::stringstream ss; if (nrDigits) ss << std::setprecision(nrDigits);
	// ss << d;`. That is the only reason hfloat's core would have needed <sstream>, and
	// str() is core surface -- operator<< formats through it. std::ostream's
	// `defaultfloat` presentation of a double IS printf's %g at the stream's precision
	// (which defaults to 6), so snprintf("%.*g") is the same formatter reached without a
	// stream -- the <cstdio> idiom Phase 0 adopted. Verified byte-identical over 604,742
	// (value, precision) pairs spanning the full double exponent range on gcc and clang;
	// see the differential in the PR (#1334, #1444).
	std::string str(size_t nrDigits = 0) const {
		if (iszero()) return sign() ? std::string("-0") : std::string("0");

		double d = convert_to_double();
		constexpr size_t maxPrecision = 512;   // guard the size_t -> int narrowing
		const int precision = (nrDigits > 0)
			? static_cast<int>(nrDigits < maxPrecision ? nrDigits : maxPrecision)
			: 6;                               // std::ios_base's default precision
		const int len = std::snprintf(nullptr, 0, "%.*g", precision, d);
		if (len < 0) return std::string{};
		std::string text(static_cast<size_t>(len), '\0');
		std::snprintf(text.data(), static_cast<size_t>(len) + 1u, "%.*g", precision, d);
		return text;
	}

	///////////////////////////////////////////////////////////////////
	// Bit access (public for free functions)
	constexpr bool getbit(unsigned pos) const noexcept {
		if (pos >= nbits) return false;
		unsigned block_idx = pos / bitsInBlock;
		unsigned bit_idx = pos % bitsInBlock;
		return (_block[block_idx] >> bit_idx) & 1;
	}

	///////////////////////////////////////////////////////////////////
	// Decode into sign, unbiased exponent and the full fraction
	constexpr void decode(bool& s, int& exponent, frac_t& fraction) const noexcept {
		const raw_t r = load();
		s = r.test(nbits - 1u);
		unsigned exp_field = 0;
		for (unsigned i = 0; i < es; ++i) {
			if (r.test(fbits + i)) exp_field |= (1u << i);
		}
		exponent = static_cast<int>(exp_field) - bias;
		fraction = r.template resize<fbits>();
		if (fraction.iszero()) exponent = 0;
	}

	// decode() with the fraction shifted left by whole hex digits until its leading digit is not
	// zero, and the exponent lowered to match: the same value. setbits() accepts encodings whose
	// leading digit is zero, and the arithmetic and the comparisons rely on a normalized
	// fraction (a quotient of at least 1/16, a difference that cannot cancel to below a guard
	// unit, a (exponent, fraction) order that is the value order).
	constexpr void decode_normalized(bool& s, int& exponent, frac_t& fraction) const noexcept {
		decode(s, exponent, fraction);
		const int m = fraction.msb();
		if (m < 0) return;
		const int top = static_cast<int>(fbits) - 4;
		if (m >= top) return;
		const unsigned digits = static_cast<unsigned>((top - m + 3) / 4);
		fraction.shl(4u * digits);
		exponent -= static_cast<int>(digits);
	}

	// The legacy view: sign, unbiased exponent and the LOW 64 fraction bits (the whole fraction
	// when fbits <= 64). decode() gives the full fraction.
	constexpr void unpack(bool& s, int& exponent, uint64_t& fraction) const noexcept {
		frac_t f;
		decode(s, exponent, f);
		fraction = f.low64();
		if constexpr (fbits < 64) fraction &= ((uint64_t(1) << fbits) - 1u);
	}

protected:
	bt _block[nrBlocks];

	///////////////////////////////////////////////////////////////////
	// Bit manipulation helpers
	constexpr void setbit(unsigned pos, bool value) noexcept {
		if (pos >= nbits) return;
		unsigned block_idx = pos / bitsInBlock;
		unsigned bit_idx = pos % bitsInBlock;
		if (value) {
			_block[block_idx] |= bt(1ull << bit_idx);
		}
		else {
			_block[block_idx] &= bt(~(1ull << bit_idx));
		}
	}

	///////////////////////////////////////////////////////////////////
	// Storage access: the encoding as one wide register, gathered and scattered a block at a
	// time (it was one bit per call through getbit/setbit, two or three times per operation)
	constexpr raw_t load() const noexcept {
		raw_t r;
		limb::gather<bt>([this](unsigned i) { return _block[i]; }, nrBlocks, r.w, raw_t::L);
		r.truncate(nbits);
		return r;
	}
	constexpr void store(const raw_t& r) noexcept {
		limb::scatter<bt>(r.w, raw_t::L, [this](unsigned i, bt b) { _block[i] = b; }, nrBlocks);
		_block[MSU] &= MSU_MASK;
	}

	// write sign, unbiased exponent and fraction (the low fbits bits of F)
	template<unsigned C>
	constexpr void encode(bool s, int exponent, const hfloat_detail::wide<C>& F) noexcept {
		raw_t r = F.template resize<nbits>();
		r.truncate(fbits);
		const unsigned biased = static_cast<unsigned>(exponent + bias);
		for (unsigned i = 0; i < es; ++i) {
			if ((biased >> i) & 1u) r.set(fbits + i);
		}
		if (s) r.set(nbits - 1u);
		store(r);
	}

	static constexpr int floor_div4(int b) noexcept { return (b >= 0) ? (b / 4) : -((-b + 3) / 4); }

	///////////////////////////////////////////////////////////////////
	// The one rounding step of every operation: f * 2^-P * 16^e, an exact value, becomes the
	// hfloat with a non-zero leading hex digit whose fraction is the value truncated toward zero
	// to ndigits hex digits (IBM HFP truncation). Past emax it saturates to maxpos / maxneg; below
	// the smallest normal it is zero (HFP has no subnormals).
	template<unsigned C>
	constexpr void normalize_chop(bool s, int e, hfloat_detail::wide<C> f, int P) noexcept {
		static_assert(C >= fbits, "normalize_chop: the register must hold a full fraction");
		const int m = f.msb();
		if (m < 0) { setzero(); return; }
		const int b  = m - P + 4 * e;          // the value lies in [2^b, 2^(b+1))
		const int er = floor_div4(b) + 1;      // 16^(er-1) <= value < 16^er
		if (er > emax) { if (s) maxneg(); else maxpos(); return; }
		if (er < emin) { setzero(); return; }
		// fraction = floor(value * 16^-er * 2^fbits): its top bit lands in the leading hex digit
		const int sh = 4 * e - P - 4 * er + static_cast<int>(fbits);
		if (sh >= 0) f.shl(static_cast<unsigned>(sh));
		else         f.shr(static_cast<unsigned>(-sh));
		encode(s, er, f);
	}

	///////////////////////////////////////////////////////////////////
	// Conversion helpers

	///////////////////////////////////////////////////////////////////
	// Conversion helpers

	// Convert IEEE-754 double to hfloat
	//
	// IBM HFP has no NaN and no infinity:
	//   NaN double  -> hfloat zero
	//   +/- inf     -> hfloat maxpos/maxneg (saturation)
	// A finite double is m * 2^E exactly; the hfloat is that value truncated to ndigits hex
	// digits. A subnormal double is converted too: it is above 16^(emin-1) when es > 8.
	// sw::bit_cast is constexpr on toolchains exposing std::bit_cast or __builtin_bit_cast.
	constexpr hfloat& convert_ieee754(double rhs) noexcept {
		if (rhs != rhs) {                       // NaN
			setzero();
			return *this;
		}
		if (rhs == 0.0) {
			setzero();
			return *this;
		}
		constexpr double dbl_max = std::numeric_limits<double>::max();
		if (rhs >  dbl_max) { maxpos(); return *this; }   // +inf
		if (rhs < -dbl_max) { maxneg(); return *this; }   // -inf

		const bool negative = (rhs < 0);
		const double abs_val = negative ? -rhs : rhs;
		const uint64_t bits = sw::bit_cast<uint64_t>(abs_val);
		const uint64_t rawExp  = (bits >> 52) & 0x7FFu;
		const uint64_t rawFrac = bits & ((uint64_t(1) << 52) - 1u);
		if (rawExp == 0) return assign_scaled(negative, rawFrac, -1074);
		return assign_scaled(negative, (uint64_t(1) << 52) | rawFrac, static_cast<int>(rawExp) - 1075);
	}

	// Convert hfloat to IEEE-754 double, rounded once to nearest, ties to even.
	//
	// The fraction is reduced to 64 bits with round-to-odd (the bits below fold into the last
	// one), then rounded once to the bits the double keeps: 53 for a normal result, fewer for a
	// subnormal one (formats with es > 8 reach below 2^-1022), and the kept integer is scaled
	// exactly. hfp128's 112 bits used to be cut to their low 64.
	constexpr double convert_to_double() const noexcept {
		bool s; int e; frac_t f;
		decode_normalized(s, e, f);   // the 64-bit window below needs the leading digit at the top
		if (f.iszero()) return s ? -0.0 : 0.0;
		uint64_t top = 0;
		int shift = 0;                                   // value = top * 2^shift
		if constexpr (fbits > 64) {
			const bool sticky = f.shr(fbits - 64u);
			top = f.low64() | (sticky ? 1u : 0u);
			shift = 4 * e - 64;
		}
		else {
			top = f.low64();
			shift = 4 * e - static_cast<int>(fbits);
		}
		int m = 63;
		while (((top >> m) & 1u) == 0u) --m;
		const int bs = m + shift;                        // 2^bs <= |value| < 2^(bs+1)
		const double inf = std::numeric_limits<double>::infinity();
		if (bs > 1023) return s ? -inf : inf;
		const int p = (bs >= -1022) ? 53 : 53 - (-1022 - bs);   // the bits the double keeps
		if (p < 0) return s ? -0.0 : 0.0;
		const int drop = m + 1 - p;
		uint64_t kept = top;
		if (drop > 0) {
			bool round_bit = false, rest = false;
			if (drop > 63) {                             // p == 0: only the round bit is left
				kept = 0;
				round_bit = true;
				rest = (top & ~(uint64_t(1) << m)) != 0;
			}
			else {
				kept = top >> drop;
				round_bit = ((top >> (drop - 1)) & 1u) != 0;
				rest = (top & ((uint64_t(1) << (drop - 1)) - 1u)) != 0;
			}
			if (round_bit && (rest || (kept & 1u) != 0)) ++kept;
			shift += drop;
		}
		const double result = sw::universal::constexpr_ldexp(static_cast<double>(kept), shift);
		return s ? -result : result;
	}

	// long double: its significand is read exactly, in two 64-bit pieces (binary128 carries 113
	// bits, IBM double-double 106, x87 64), so a long double wider than a double is not first rounded
	// to 53 bits. Any bits past 128 would be truncated, which composes with hfloat's own truncation.
	// At run time frexp / ldexp split it; a constant evaluation scales by exact powers of two instead.
	// It used to go through double there, which rounds to nearest before hfloat truncates and can
	// land on the wrong side of a hex digit: 1 - 2^-60 became 1.0 at hfp64 instead of 1 - 2^-56 (#2133).
	constexpr hfloat& convert_long_double(long double rhs) noexcept {
		if (rhs != rhs || rhs == 0.0L) { setzero(); return *this; }
		const long double ldmax = std::numeric_limits<long double>::max();
		if (rhs >  ldmax) { maxpos(); return *this; }
		if (rhs < -ldmax) { maxneg(); return *this; }
		const bool negative = rhs < 0.0L;
		int e = 0;
		long double fr = negative ? -rhs : rhs;
		long double top = 0.0L;
		if (std::is_constant_evaluated()) {
			// fr = |rhs| * 2^-e in [0.5, 1), by exact multiplications by powers of two
			constexpr long double two32 = 4294967296.0L;
			while (fr >= two32) { fr /= two32; e += 32; }
			while (fr >= 1.0L) { fr /= 2.0L; ++e; }
			while (fr < 1.0L / two32) { fr *= two32; e -= 32; }
			while (fr < 0.5L) { fr *= 2.0L; --e; }
			top = fr * two32 * two32;                                     // [2^63, 2^64)
		}
		else {
			fr = std::frexp(fr, &e);                                      // [0.5, 1)
			top = std::ldexp(fr, 64);                                     // [2^63, 2^64)
		}
		const uint64_t hi = static_cast<uint64_t>(top);                   // truncates toward zero
		const long double rest = top - static_cast<long double>(hi);      // exact, in [0, 1)
		uint64_t lo = 0;
		if (std::is_constant_evaluated()) lo = static_cast<uint64_t>(rest * 4294967296.0L * 4294967296.0L);
		else lo = static_cast<uint64_t>(std::ldexp(rest, 64));
		using w128 = hfloat_detail::wide<(fbits > 128u ? fbits : 128u)>;
		w128 m = w128::from_u64(hi);
		m.shl(64);
		m.add(w128::from_u64(lo));
		normalize_chop(negative, 0, m, 128 - e);                           // value = m * 2^(e - 128)
		return *this;
	}

	// Integers convert exactly and truncate to ndigits hex digits, at every fbits: hfp128 used to
	// go through a double and lose the bits past 53.
	constexpr hfloat& convert_signed(int64_t v) noexcept {
		if (v == 0) {
			setzero();
			return *this;
		}
		const bool negative = (v < 0);
		// |INT64_MIN| = -(v + 1) + 1, each step stays in range
		const uint64_t abs_v = negative ? (static_cast<uint64_t>(-(v + 1)) + 1ull) : static_cast<uint64_t>(v);
		return assign_scaled(negative, abs_v, 0);
	}

	constexpr hfloat& convert_unsigned(uint64_t v) noexcept {
		if (v == 0) {
			setzero();
			return *this;
		}
		return assign_scaled(false, v, 0);
	}

	// the value m * 2^E, truncated to ndigits hex digits
	constexpr hfloat& assign_scaled(bool negative, uint64_t m, int E) noexcept {
		if (m == 0) { setzero(); return *this; }
		normalize_chop(negative, 0, hfloat_detail::wide<wide64>::from_u64(m), -E);
		return *this;
	}

private:

	// hfloat - hfloat logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend constexpr bool operator==(const hfloat<N, E, B>& lhs, const hfloat<N, E, B>& rhs);

	// hfloat - literal logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend constexpr bool operator==(const hfloat<N, E, B>& lhs, const double rhs);

	// literal - hfloat logic comparisons
	template<unsigned N, unsigned E, typename B>
	friend constexpr bool operator==(const double lhs, const hfloat<N, E, B>& rhs);
};


// to_binary(), to_hex() and to_native() moved to manipulators.hpp in #1334: all three
// format through a std::stringstream, which is what keeps them out of the core. str()
// and parse() stay here -- str() now formats with snprintf, and parse() converts through
// decimal_to_binary, so neither opens a stream.

////////////////////////    HFLOAT functions   /////////////////////////////////

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> abs(const hfloat<ndigits, es, BlockType>& a) {
	hfloat<ndigits, es, BlockType> result(a);
	result.setsign(false);
	return result;
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> fabs(hfloat<ndigits, es, BlockType> a) {
	a.setsign(false);
	return a;
}


// operator<< and operator>> moved to iostream.hpp in #1334. hfloat.hpp includes it, so
// callers that stream an hfloat are unaffected.

////////////////// string operators

// Parse a decimal floating-point literal into an hfloat. hfloat has no NaN
// or Inf encoding, so "nan" / "inf" / "infinity" tokens are rejected. The
// decimal payload is converted via decimal_to_binary with 64 bits of
// precision and assembled directly into the hfloat hex fraction -- this
// gives bit-exact correctly-rounded conversion for hfloat_long's full 56
// fbits, which the via-double path could not deliver (double has only 53).
//
// Resolves #849.
template<unsigned ndigits, unsigned es, typename BlockType>
bool parse(const std::string& number, hfloat<ndigits, es, BlockType>& value) {
	if (number.empty()) return false;

	// Reject special-value tokens up front (hfloat has no NaN or Inf
	// encoding) and pre-validate the input grammar so malformed tokens are
	// signaled via false return rather than silently mapped to zero by the
	// d2b path.
	std::size_t pos = 0;
	while (pos < number.size() && std::isspace(static_cast<unsigned char>(number[pos]))) ++pos;
	if (pos >= number.size()) return false;
	const std::size_t numeric_start = pos;
	if (number[pos] == '+' || number[pos] == '-') ++pos;
	if (pos + 3 <= number.size()) {
		char a = static_cast<char>(std::tolower(static_cast<unsigned char>(number[pos])));
		char b = static_cast<char>(std::tolower(static_cast<unsigned char>(number[pos + 1])));
		char c = static_cast<char>(std::tolower(static_cast<unsigned char>(number[pos + 2])));
		if ((a == 'n' && b == 'a' && c == 'n')
		 || (a == 'i' && b == 'n' && c == 'f')) {
			return false;  // hfloat cannot represent these
		}
	}
	// Pre-validate the rest as a decimal floating-point literal:
	// digits[.digits][eE[+-]digits] or .digits[eE[+-]digits].
	bool seen_digit = false;
	bool seen_dot   = false;
	if (pos >= number.size()) return false;
	while (pos < number.size()) {
		char ch = number[pos];
		if (ch >= '0' && ch <= '9') { seen_digit = true; ++pos; continue; }
		if (ch == '.') {
			if (seen_dot) return false;
			seen_dot = true;
			++pos;
			continue;
		}
		break;
	}
	if (!seen_digit) return false;
	if (pos < number.size() && (number[pos] == 'e' || number[pos] == 'E')) {
		++pos;
		if (pos < number.size() && (number[pos] == '+' || number[pos] == '-')) ++pos;
		bool seen_exp_digit = false;
		while (pos < number.size() && number[pos] >= '0' && number[pos] <= '9') {
			seen_exp_digit = true;
			++pos;
		}
		if (!seen_exp_digit) return false;
	}
	if (pos != number.size()) return false;  // trailing junk

	// Hand the trimmed payload (post-whitespace) to decimal_to_binary.
	// Request fbits of precision when fbits > 64 so wide-fraction configs
	// (hfloat<28,*> with fbits=112) get bit-exact conversion through the
	// d2b multi-limb mantissa instead of being clipped to double-precision
	// like the legacy via-double path. For fbits <= 64 stay at 64 bits --
	// that pinned the hfp64 0.1 truncation test under issue #849 and gives
	// hfp32 a few guard bits for the d2b path's rounding to settle.
	using H = hfloat<ndigits, es, BlockType>;
	constexpr unsigned target_bits = (H::fbits > 64u) ? H::fbits : 64u;
	std::string_view payload(number.data() + numeric_start, number.size() - numeric_start);
	auto d = ::sw::universal::decimal_to_binary::convert(payload, target_bits);
	if (!d.valid) return false;
	if (d.is_zero) {
		value = H(SpecificValue::zero);
		return true;
	}

	// d2b's binary_scale is the exponent of mantissa's MSB, so the
	// "frexp" exponent (value = 1.frac * 2^(bin_exp - 1)) is binary_scale + 1.
	int bin_exp = static_cast<int>(d.binary_scale) + 1;

	if constexpr (H::fbits > 64u) {
		// Wide-fraction path: feed d2b's full multi-limb mantissa directly
		// into the hex-aligned fraction. This is what makes parse() deliver
		// precision beyond what double / long double can carry; the legacy
		// uint64_t intermediate would truncate to ~64 bits and defeat the
		// purpose of routing decimal literals through d2b in the first place.
		value.assign_from_wide_mantissa(d.negative, bin_exp, d);
	}
	else {
		// Extract the 64-bit normalized mantissa (MSB at bit 63) from d2b's
		// bigint. With target_mantissa_bits == 64, mantissa.bit[63] is the
		// implicit-1 of the binary expansion and bits [0, 62] are the fraction.
		std::uint64_t mantissa64 = 0;
		for (unsigned i = 0; i < 64; ++i) {
			if (d.mantissa.at(i)) mantissa64 |= (std::uint64_t{1} << i);
		}
		value.assign_from_mantissa64(d.negative, bin_exp, mantissa64);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - hfloat binary logic operators

// hfloat values are normalized by normalize_chop so that the leading
// hex digit of the fraction is non-zero -- the (sign, exponent, fraction)
// tuple is unique per value.  We compare tuples directly instead of via
// double() to preserve precision for hfloat_long (56-bit fraction) and
// hfloat_extended, where the double round-trip would lose bits below the
// 53-bit IEEE 754 mantissa.
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator==(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	bool lz = lhs.iszero();
	bool rz = rhs.iszero();
	if (lz && rz) return true;       // +0 == -0 in HFP (no signed-zero distinction)
	if (lz || rz) return false;
	if (lhs.sign() != rhs.sign()) return false;
	bool ts; int le = 0, re = 0;
	typename hfloat<ndigits, es, BlockType>::frac_t lf, rf;
	lhs.decode_normalized(ts, le, lf);
	rhs.decode_normalized(ts, re, rf);
	return le == re && lf.compare(rf) == 0;
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator!=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator< (const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	bool lz = lhs.iszero();
	bool rz = rhs.iszero();
	if (lz && rz) return false;
	bool ls = lhs.sign();
	bool rs = rhs.sign();
	if (lz) return !rs;              // 0 < +x; 0 not < -x
	if (rz) return ls;               // -x < 0; +x not < 0
	if (ls != rs) return ls;         // negative < positive
	// Same sign, both non-zero.  unpack returns (sign, biased-exp-removed,
	// fraction).  For positives, lhs < rhs iff (le < re) || tie + (lf < rf).
	// For negatives, the ordering reverses: lhs < rhs iff |lhs| > |rhs|.
	bool ts; int le = 0, re = 0;
	typename hfloat<ndigits, es, BlockType>::frac_t lf, rf;
	lhs.decode_normalized(ts, le, lf);
	rhs.decode_normalized(ts, re, rf);
	const int fc = lf.compare(rf);   // the full fraction: hfp128's 112 bits were cut to the low 64
	if (ls) {
		// both negative: lhs < rhs iff |lhs| > |rhs|
		return (le > re) || (le == re && fc > 0);
	}
	// both positive
	return (le < re) || (le == re && fc < 0);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator> (const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator<=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator>=(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - literal binary logic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator==(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator==(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator!=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator< (const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator<(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator> (const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (hfloat<ndigits, es, BlockType>(rhs), lhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator<=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator>=(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - hfloat binary logic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator==(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator==(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator!=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator< (const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator<(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator> (const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator<=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr bool operator>=(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - hfloat binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator+(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> sum(lhs);
	sum += rhs;
	return sum;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator-(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> diff(lhs);
	diff -= rhs;
	return diff;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator*(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> mul(lhs);
	mul *= rhs;
	return mul;
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator/(const hfloat<ndigits, es, BlockType>& lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	hfloat<ndigits, es, BlockType> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// hfloat - literal binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator+(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator+(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator-(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator-(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator*(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator*(lhs, hfloat<ndigits, es, BlockType>(rhs));
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator/(const hfloat<ndigits, es, BlockType>& lhs, const double rhs) {
	return operator/(lhs, hfloat<ndigits, es, BlockType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - hfloat binary arithmetic operators
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator+(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator+(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator-(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator-(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator*(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator*(hfloat<ndigits, es, BlockType>(lhs), rhs);
}
template<unsigned ndigits, unsigned es, typename BlockType>
inline constexpr hfloat<ndigits, es, BlockType> operator/(const double lhs, const hfloat<ndigits, es, BlockType>& rhs) {
	return operator/(hfloat<ndigits, es, BlockType>(lhs), rhs);
}

}} // namespace sw::universal
