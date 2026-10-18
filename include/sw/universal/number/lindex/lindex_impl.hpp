#pragma once
// lindex_impl.hpp: implementation of the level-index number system lindex<nbits, lbits, bt>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Level-index (Clenshaw and Olver 1984) represents X >= 0 by its generalized logarithm
// psi(X) = l + f, the integer level l and the index f in [0, 1), with the value map
//
//     phi(x) = x               for 0 <= x < 1
//     phi(x) = e^phi(x - 1)    for x >= 1
//
// The encoding (docs/level-index-design.md, "Special values and the encoding"):
//
//     [ s : 1 ][ l + f : lbits integer bits . fbits fraction bits ]      fbits = nbits - 1 - lbits
//
//   - the magnitude is one unsigned fixed-point number l + f, so the level bits are its
//     integer part and the index bits its fraction
//   - a negative value is the two's complement of the whole nbits pattern, so comparing
//     two encodings as signed integers orders them by value
//   - zero is all zeros, NaR is 1000...0, and there is no infinity
//
// The core type is #1349 part 3 (#1694); + - * / and the reciprocal are part 4 (#1695).
//
// Conversion rounds psi(|v|) to nearest, ties to even, on the 2^-fbits grid, and
// saturates at maxpos.  Below minpos/2 a value rounds to 0: level 0 is a plain fixed-point
// fraction, so LI does not avoid underflow (that is what SLI, #1350, is for).  Conversion
// to a native type rounds once, at the end, and gives +-inf past the native range: no
// IEEE type reaches level 5 (psi(DBL_MAX) = 4.632).
//
// Both directions evaluate phi and psi in dd_cascade at every width.  psi(X) is then good
// to about 2^-104 absolute and phi(x) to about 2^-96 relative through level 4, so the one
// rounding is correct except within that distance of a tie; double would miss fbits 60 by
// hundreds of ulps.
//
// Arithmetic (#1695) stays in the psi domain and never forms an operand value, so no
// intermediate overflows at any level:
//
//   x + y, x - y   the Clenshaw-Olver sequences of number/shared/level_index_kernel.hpp,
//                  with the cancellation-free expm1 form for a difference
//   x * y          one level down:  psi(XY) = 1 + psi(phi(x - 1) + phi(y - 1))  for x, y >= 1
//   x / y          psi(X/Y) = 1 + psi(phi(x - 1) - phi(y - 1))  for x >= y >= 1, and X/Y = e^-D
//                  at level 0 when x < y
//
// An operand at level 0 is its own value g < 1, and ln g < 0 takes the place of phi(. - 1):
// a product with it is a difference one level down while it stays above 1, and is X * g at
// level 0 once it falls below.  Two level-0 operands multiply, and divide to a quotient
// below 1, as exact fixed-point rationals, rounded exactly, in integers.  The product is the
// only result that can sit exactly on a rounding midpoint; the quotient cannot, since
// 2^(fbits + 1) a / b odd would need 2^(fbits + 1) to divide b < 2^fbits.
//
// Working precision follows the design doc: double for nbits <= 32, dd_cascade above.  Every
// result is rounded once, to nearest even on the psi grid, and saturates at maxpos.  The
// double path is accurate to about 2^-49 in psi; a result that lands within 2^-40 of a
// midpoint is evaluated again in dd_cascade, whose error is near 2^-100, and one within 2^-90
// once more in qd_cascade, near 2^-200 (round_psi).  At nbits = 32 the dd_cascade stage runs
// for about 1 result in 1200 and changes about 3 in 10^8.  The qd_cascade stage is for the
// structured results of operands one ulp from 1/2, 1 or a level boundary, which land within
// about 2^-122 of a midpoint at fbits 60.
#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>
#include <type_traits>

#if !defined(LINDEX_THROW_ARITHMETIC_EXCEPTION)
// default is to not throw: an invalid operation produces NaR
#define LINDEX_THROW_ARITHMETIC_EXCEPTION 0
#endif

#include <universal/utility/directives.hpp>   // LONG_DOUBLE_SUPPORT
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/bit_manipulation.hpp>
#include <universal/utility/icf_array_bounds.hpp>
// the working precision of conversion; core.hpp and these functions are I/O-free
#include <universal/number/dd_cascade/core.hpp>
#include <universal/number/dd_cascade/math/constants/dd_cascade_constants.hpp>
#include <universal/number/dd_cascade/math/functions/numerics.hpp>
#include <universal/number/dd_cascade/math/functions/exponent.hpp>
#include <universal/number/dd_cascade/math/functions/logarithm.hpp>
#include <universal/number/dd_cascade/math/functions/truncate.hpp>
// the last stage of the rounding retry, for results within 2^-90 of a midpoint
#include <universal/number/qd_cascade/core.hpp>
#include <universal/number/qd_cascade/math/constants/qd_cascade_constants.hpp>
#include <universal/number/qd_cascade/math/functions/numerics.hpp>
#include <universal/number/qd_cascade/math/functions/exponent.hpp>
#include <universal/number/qd_cascade/math/functions/logarithm.hpp>
#include <universal/number/qd_cascade/math/functions/truncate.hpp>
#include <universal/number/shared/level_index_kernel.hpp>

namespace sw { namespace universal {

template<unsigned _nbits, unsigned _lbits = 3, typename bt = std::uint8_t>
class lindex {
public:
	static_assert(_nbits <= 64, "lindex: nbits > 64 is not supported");
	static_assert(_lbits >= 1 && _lbits <= 4, "lindex: lbits must be in [1, 4], the levels the kernel works with");
	static_assert(_nbits >= _lbits + 2, "lindex: nbits must leave at least one index bit");
	static_assert(std::is_unsigned_v<bt>, "lindex: the block type must be an unsigned integer");

	typedef bt BlockType;
	// the working type of conversion
	using Working = dd_cascade;
	// the working type of arithmetic (design doc, "Working precision")
	using ArithmeticWorking = std::conditional_t<(_nbits <= 32), double, dd_cascade>;

	static constexpr unsigned nbits     = _nbits;
	static constexpr unsigned lbits     = _lbits;
	static constexpr unsigned fbits     = nbits - 1u - lbits;   // index bits
	static constexpr unsigned nr_levels = 1u << lbits;          // levels 0 .. nr_levels - 1

	static constexpr unsigned bitsInByte  = 8u;
	static constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	static constexpr unsigned nrBlocks    = 1u + ((nbits - 1u) / bitsInBlock);
	static constexpr unsigned MSU         = nrBlocks - 1u;
	static constexpr std::uint64_t storageMask = (0xFFFF'FFFF'FFFF'FFFFull >> (64u - bitsInBlock));
	static constexpr bt MSU_MASK = bt(bt(~0) >> (nrBlocks * bitsInBlock - nbits));

	// the encoding as a uint64_t
	static constexpr std::uint64_t nbits_mask  = (nbits == 64) ? ~0ull : ((1ull << nbits) - 1ull);
	static constexpr std::uint64_t sign_mask   = 1ull << (nbits - 1u);
	static constexpr std::uint64_t index_mask  = (1ull << fbits) - 1ull;
	static constexpr std::uint64_t maxpos_bits = sign_mask - 1ull;   // 0111...1

	using BlockBinary = blockbinary<nbits, bt, BinaryNumberType::Unsigned>;

	lindex() = default;
	lindex(const lindex&) = default;
	lindex(lindex&&) = default;
	lindex& operator=(const lindex&) = default;
	lindex& operator=(lindex&&) = default;

	constexpr lindex(const SpecificValue code) noexcept : _block{} {
		switch (code) {
		case SpecificValue::maxpos: maxpos(); break;
		case SpecificValue::minpos: minpos(); break;
		case SpecificValue::zero:
		default:                    zero();   break;
		case SpecificValue::minneg: minneg(); break;
		case SpecificValue::maxneg: maxneg(); break;
		// there is no infinity: converting one gives NaR, so naming one does too
		case SpecificValue::infpos:
		case SpecificValue::infneg:
		case SpecificValue::nar:
		case SpecificValue::qnan:
		case SpecificValue::snan:   setnar(); break;
		}
	}

	// Conversion evaluates psi in dd_cascade, whose log is not constexpr, so none of these is
	lindex(signed char initial_value)        noexcept : _block{} { *this = initial_value; }
	lindex(short initial_value)              noexcept : _block{} { *this = initial_value; }
	lindex(int initial_value)                noexcept : _block{} { *this = initial_value; }
	lindex(long initial_value)               noexcept : _block{} { *this = initial_value; }
	lindex(long long initial_value)          noexcept : _block{} { *this = initial_value; }
	lindex(char initial_value)               noexcept : _block{} { *this = initial_value; }
	lindex(unsigned short initial_value)     noexcept : _block{} { *this = initial_value; }
	lindex(unsigned int initial_value)       noexcept : _block{} { *this = initial_value; }
	lindex(unsigned long initial_value)      noexcept : _block{} { *this = initial_value; }
	lindex(unsigned long long initial_value) noexcept : _block{} { *this = initial_value; }
	lindex(float initial_value)              noexcept : _block{} { *this = initial_value; }
	lindex(double initial_value)             noexcept : _block{} { *this = initial_value; }

	lindex& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	lindex& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	lindex& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	lindex& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	lindex& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	lindex& operator=(char rhs)               noexcept {
		if constexpr (std::is_signed_v<char>) return convert_signed(rhs); else return convert_unsigned(rhs);
	}
	lindex& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	lindex& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	lindex& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	lindex& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	lindex& operator=(float rhs)              noexcept { return convert_ieee754(double(rhs)); }
	lindex& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	explicit operator int()                const noexcept { return to_signed<int>(); }
	explicit operator long()               const noexcept { return to_signed<long>(); }
	explicit operator long long()          const noexcept { return to_signed<long long>(); }
	explicit operator unsigned int()       const noexcept { return to_unsigned<unsigned int>(); }
	explicit operator unsigned long()      const noexcept { return to_unsigned<unsigned long>(); }
	explicit operator unsigned long long() const noexcept { return to_unsigned<unsigned long long>(); }
	explicit operator float()              const noexcept { return to_ieee754<float>(); }
	explicit operator double()             const noexcept { return to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	lindex(long double initial_value)  noexcept : _block{} { *this = initial_value; }
	lindex& operator=(long double rhs) noexcept { return convert_long_double(rhs); }
	explicit operator long double() const noexcept { return to_ieee754<long double>(); }
#endif

	// negation is the two's complement of the whole pattern; 0 and NaR are their own
	constexpr lindex operator-() const noexcept {
		lindex result;
		result.setbits(((~raw_bits()) + 1ull) & nbits_mask);
		return result;
	}

	// step to the next encoding, saturating at maxpos and maxneg; NaR stays NaR
	constexpr lindex& operator++() noexcept {
		const std::uint64_t raw = raw_bits();
		if (raw == sign_mask || raw == maxpos_bits) return *this;
		setbits((raw + 1ull) & nbits_mask);
		return *this;
	}
	constexpr lindex operator++(int) noexcept { lindex tmp(*this); operator++(); return tmp; }
	constexpr lindex& operator--() noexcept {
		const std::uint64_t raw = raw_bits();
		if (raw == sign_mask || raw == (sign_mask | 1ull)) return *this;
		setbits((raw - 1ull) & nbits_mask);
		return *this;
	}
	constexpr lindex operator--(int) noexcept { lindex tmp(*this); operator--(); return tmp; }

	// arithmetic: NaR propagates, results saturate at maxpos and underflow to 0 below minpos/2
	lindex& operator+=(const lindex& rhs) noexcept { return add_signed(rhs, false); }
	lindex& operator-=(const lindex& rhs) noexcept { return add_signed(rhs, true); }
	lindex& operator*=(const lindex& rhs) noexcept {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (iszero() || rhs.iszero()) { setzero(); return *this; }
		return set_magnitude(evaluate(Op::mul, magnitude_bits(), rhs.magnitude_bits()), sign() != rhs.sign());
	}
	// x / 0 is NaR, or throws lindex_divide_by_zero under LINDEX_THROW_ARITHMETIC_EXCEPTION
	lindex& operator/=(const lindex& rhs) {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (rhs.iszero()) {
#if LINDEX_THROW_ARITHMETIC_EXCEPTION
			throw lindex_divide_by_zero();
#else
			setnar();
			return *this;
#endif
		}
		if (iszero()) return *this;
		return set_magnitude(evaluate(Op::div, magnitude_bits(), rhs.magnitude_bits()), sign() != rhs.sign());
	}
	lindex& operator+=(double rhs) noexcept { return *this += lindex(rhs); }
	lindex& operator-=(double rhs) noexcept { return *this -= lindex(rhs); }
	lindex& operator*=(double rhs) noexcept { return *this *= lindex(rhs); }
	lindex& operator/=(double rhs) { return *this /= lindex(rhs); }

	// 1 / x, correctly rounded.  This is the awkward operation of level-index: 1/X for a
	// large X falls into level 0, a plain fixed-point fraction with a floor of 2^-fbits, so
	// it rounds to 0 for every X from about 2^(fbits + 1) up -- 52.65% of the positive
	// encodings of lindex<16,3> (design doc, [C8]).  There is no exact reciprocal path, as
	// takum_log has.  Giving values below 1 the level structure of values above it is what
	// the reciprocal bit of SLI (#1350) is for.
	lindex reciprocal() const {
		lindex one;
		one.setbits(1ull << fbits);
		return one /= *this;
	}

	// modifiers
	constexpr void clear()                           noexcept { _block.clear(); }
	constexpr void setzero()                         noexcept { _block.clear(); }
	constexpr void setnar()                          noexcept { setbits(sign_mask); }
	constexpr void setnan(bool sign = false)         noexcept { (void)sign; setnar(); }
	constexpr void setbit(unsigned i, bool v = true) noexcept {
		if (i < nbits) {
			const unsigned blockIndex = i / bitsInBlock;
			const bt block = _block[blockIndex];
			const bt null  = bit_clear_mask<bt>(i, bitsInBlock);
			const bt mask  = bt(bt(v ? 1 : 0) << (i % bitsInBlock));
			_block.setblock(blockIndex, bt((block & null) | mask));
		}
	}
	// set the raw encoding; bits above nbits are ignored
	constexpr lindex& setbits(std::uint64_t value) noexcept {
		if constexpr (1 == nrBlocks) {
			_block.setblock(0, bt(value & storageMask));
		}
		else {
			for (unsigned i = 0; i < nrBlocks; ++i) {
				_block.setblock(i, bt(value & storageMask));
				value = (bitsInBlock < 64u) ? (value >> (bitsInBlock % 64u)) : 0ull;
			}
		}
		_block.setblock(MSU, bt(_block[MSU] & MSU_MASK));
		return *this;
	}

	constexpr lindex& maxpos() noexcept { return setbits(maxpos_bits); }                              // 0111...1
	constexpr lindex& minpos() noexcept { return setbits(1ull); }                                     // 0000...1
	constexpr lindex& zero()   noexcept { return setbits(0ull); }
	constexpr lindex& minneg() noexcept { return setbits(nbits_mask); }                               // 1111...1
	constexpr lindex& maxneg() noexcept { return setbits((sign_mask | 1ull) & nbits_mask); }          // 1000...1

	// selectors
	constexpr bool iszero() const noexcept { return raw_bits() == 0ull; }
	constexpr bool isnar()  const noexcept { return raw_bits() == sign_mask; }
	constexpr bool isnan()  const noexcept { return isnar(); }
	constexpr bool isinf()  const noexcept { return false; }
	constexpr bool sign()   const noexcept { return (raw_bits() & sign_mask) != 0ull; }
	constexpr bool isneg()  const noexcept { return sign() && !isnar(); }
	constexpr bool ispos()  const noexcept { return !sign() && !iszero(); }

	// the raw encoding, and the encoding of |value| (the two's complement of a negative)
	constexpr std::uint64_t raw_bits() const noexcept {
		std::uint64_t raw = 0;
		for (unsigned i = 0; i < nrBlocks; ++i) {
			raw |= (static_cast<std::uint64_t>(_block[i]) << ((i * bitsInBlock) % 64u));
		}
		return raw;
	}
	constexpr std::uint64_t magnitude_bits() const noexcept {
		const std::uint64_t raw = raw_bits();
		return (raw & sign_mask) ? (((~raw) + 1ull) & nbits_mask) : raw;
	}

	// the fields of |value|: psi(|value|) = level() + index(), index() exact for fbits <= 53
	constexpr int           level()      const noexcept { return isnar() ? 0 : static_cast<int>(magnitude_bits() >> fbits); }
	constexpr std::uint64_t index_bits() const noexcept { return isnar() ? 0ull : (magnitude_bits() & index_mask); }
	double index() const noexcept { return std::ldexp(static_cast<double>(index_bits()), -static_cast<int>(fbits)); }
	// psi(|value|) as a double; 0 for NaR
	double psi() const noexcept { return static_cast<double>(level()) + index(); }

	// the binary exponent floor(log2 |value|), saturated to int64_t: from level 5 up it is
	// past 2^63 for nearly every encoding.  0 for zero and NaR.
	int64_t scale() const noexcept {
		if (iszero() || isnar()) return 0;
		const std::uint64_t mag = magnitude_bits();
		const int l = static_cast<int>(mag >> fbits);
		if (l == 0) {
			// |value| = index_bits * 2^-fbits exactly
			int msb = 63;
			while (((mag >> msb) & 1ull) == 0ull) --msb;
			return static_cast<int64_t>(msb) - static_cast<int64_t>(fbits);
		}
		// ln |value| = phi(l - 1 + f), which is finite through level 5 for a small enough f
		const Working lnX = level_index::phi(level_index::point<Working>{ l - 1, index_in<Working>(mag) });
		const Working log2X = lnX / ddc_ln2;
		if (!(log2X.high() < 9.2e18)) return std::numeric_limits<int64_t>::max();
		const Working s = floor(log2X);
		return static_cast<int64_t>(s.high()) + static_cast<int64_t>(s.low());
	}

	constexpr bool at(unsigned bitIndex) const noexcept {
		if (bitIndex >= nbits) return false;
		UNIVERSAL_ICF_ARRAY_BOUNDS_PUSH
		const bt word = _block[bitIndex / bitsInBlock];
		UNIVERSAL_ICF_ARRAY_BOUNDS_POP
		return (word & bt(1ull << (bitIndex % bitsInBlock))) != 0;
	}
	constexpr bool test(unsigned bitIndex) const noexcept { return at(bitIndex); }
	constexpr bt block(unsigned b) const noexcept { return (b < nrBlocks) ? _block[b] : bt(0); }
	constexpr std::uint8_t nibble(unsigned n) const noexcept {
		return (n < (1u + ((nbits - 1u) >> 2))) ? static_cast<std::uint8_t>((raw_bits() >> (4u * n)) & 0xFull) : 0u;
	}

protected:
	static std::int64_t integer_of(double v) noexcept { return static_cast<std::int64_t>(v); }
	static std::int64_t integer_of(const dd_cascade& v) noexcept {
		return static_cast<std::int64_t>(v.high()) + static_cast<std::int64_t>(v.low());
	}
	static std::int64_t integer_of(const qd_cascade& v) noexcept {
		return static_cast<std::int64_t>(v[0]) + static_cast<std::int64_t>(v[1]) + static_cast<std::int64_t>(v[2]) + static_cast<std::int64_t>(v[3]);
	}
	// the half-width of the band around a midpoint, in psi, inside which W cannot decide the
	// rounding: well outside W's error in every operation (about 2^-49 for double, 2^-100 for
	// dd_cascade); qd_cascade, near 2^-200, is the last stage and decides
	template<typename W>
	static constexpr int tie_band_bits() noexcept {
		if constexpr (std::is_same_v<W, double>) return 40;
		else if constexpr (std::is_same_v<W, dd_cascade>) return 90;
		else return 0;
	}

	// The magnitude encoding of the psi point { level, index }, rounded to nearest even and
	// saturated.  near_tie reports a remainder within tie_band_bits<W>() of the midpoint,
	// where W cannot decide the rounding.
	template<typename W>
	static std::uint64_t round_point(int level, W index, bool& near_tie) noexcept {
		using std::floor;
		near_tie = false;
		if (level >= static_cast<int>(nr_levels)) return maxpos_bits;
		if (index < W(0)) index = W(0);   // log(1 + tiny) may round below 0
		// scaling by 2^fbits is exact, so is the floor, and so is the remainder
		const W scaled = index * W(std::ldexp(1.0, static_cast<int>(fbits)));
		const W fl = floor(scaled);
		const W rem = scaled - fl;
		std::uint64_t q = static_cast<std::uint64_t>(integer_of(fl));
		W off = rem - W(0.5);
		if (off < W(0)) off = -off;
		if constexpr (tie_band_bits<W>() > 0) near_tie = off < W(std::ldexp(1.0, static_cast<int>(fbits) - tie_band_bits<W>()));
		if (rem > W(0.5) || (rem == W(0.5) && (q & 1ull))) ++q;   // q may carry into the next level
		const std::uint64_t mag = (static_cast<std::uint64_t>(level) << fbits) + q;
		return (mag > maxpos_bits) ? maxpos_bits : mag;
	}

	// set the magnitude encoding mag, negated when negative
	lindex& set_magnitude(std::uint64_t mag, bool negative) noexcept {
		setbits((negative && mag != 0ull) ? (((~mag) + 1ull) & nbits_mask) : mag);
		return *this;
	}

	// set the encoding of the psi point { level, index } of |v|, rounded to nearest even
	lindex& encode(int level, Working index, bool negative) noexcept {
		bool near_tie = false;
		return set_magnitude(round_point<Working>(level, index, near_tie), negative);
	}

	// |v| = X as a Working value, X > 0 finite
	lindex& convert_magnitude(const Working& X, bool negative) noexcept {
		const level_index::point<Working> p = level_index::psi(X);
		return encode(p.level, p.index, negative);
	}

	template<typename SignedInt>
	lindex& convert_signed(SignedInt rhs) noexcept {
		const long long v = static_cast<long long>(rhs);
		if (v == 0) { setzero(); return *this; }
		// |v| as an unsigned, which also holds |INT64_MIN|
		const unsigned long long m = (v < 0) ? (0ull - static_cast<unsigned long long>(v)) : static_cast<unsigned long long>(v);
		return convert_integer_magnitude(m, v < 0);
	}
	template<typename UnsignedInt>
	lindex& convert_unsigned(UnsignedInt rhs) noexcept {
		const unsigned long long v = static_cast<unsigned long long>(rhs);
		if (v == 0) { setzero(); return *this; }
		return convert_integer_magnitude(v, false);
	}
	// a 64-bit integer is exact in Working as two 32-bit halves
	lindex& convert_integer_magnitude(unsigned long long m, bool negative) noexcept {
		const Working X = Working(static_cast<double>(m >> 32)) * Working(4294967296.0) + Working(static_cast<double>(m & 0xFFFF'FFFFull));
		return convert_magnitude(X, negative);
	}

	lindex& convert_ieee754(double rhs) noexcept {
		if (std::isnan(rhs) || std::isinf(rhs)) { setnar(); return *this; }
		if (rhs == 0.0) { setzero(); return *this; }
		return convert_magnitude(Working(std::fabs(rhs)), rhs < 0.0);
	}

#if LONG_DOUBLE_SUPPORT
	lindex& convert_long_double(long double rhs) noexcept {
		if (std::isnan(rhs) || std::isinf(rhs)) { setnar(); return *this; }
		if (rhs == 0.0l) { setzero(); return *this; }
		const bool negative = rhs < 0.0l;
		const long double a = std::fabs(rhs);
		if (a > static_cast<long double>(std::numeric_limits<double>::max())) {
			// past double: take the first level in long double, psi(X) = 1 + psi(ln X)
			const long double L = std::log(a);
			const double hi = static_cast<double>(L);
			const level_index::point<Working> p = level_index::psi(Working(hi) + Working(static_cast<double>(L - hi)));
			return encode(p.level + 1, p.index, negative);
		}
		// a long double significand is at most 64 bits, so a hi + lo split is exact
		const double hi = static_cast<double>(a);
		return convert_magnitude(Working(hi) + Working(static_cast<double>(a - hi)), negative);
	}
#endif

public:
	////////////////////// the psi domain (#1695, #1696)
	// The building blocks of the arithmetic and of the math library (math/*.hpp): psi points
	// of unrounded results, the level-index operations on them, and the one rounding.  A
	// function composes these in a working type W and rounds once through round_psi().  Not
	// part of the value API.
	enum class Op { add, sub, mul, div };

	static constexpr std::uint64_t one_bits = 1ull << fbits;   // the encoding of 1.0, psi = 1

	// the index of a magnitude, exact in W for fbits <= 53 (double) or <= 60 (dd_cascade):
	// two 30-bit halves keep each product exact
	template<typename W>
	static W index_in(std::uint64_t mag) noexcept {
		const std::uint64_t r = mag & index_mask;
		const W hi = W(static_cast<double>(r >> 30)) * W(std::ldexp(1.0, 30 - static_cast<int>(fbits)));
		const W lo = W(static_cast<double>(r & ((1ull << 30) - 1ull))) * W(std::ldexp(1.0, -static_cast<int>(fbits)));
		return hi + lo;
	}
	// the psi point of a magnitude encoding, exact
	template<typename W>
	static level_index::point<W> point_of(std::uint64_t mag) noexcept {
		return { static_cast<int>(mag >> fbits), index_in<W>(mag) };
	}

	// a result before rounding: a psi point, or a magnitude that is already exact
	template<typename W>
	struct outcome {
		bool exact;
		std::uint64_t mag;
		level_index::point<W> p;
	};
	template<typename W>
	static outcome<W> from_point(const level_index::point<W>& p) noexcept { return { false, 0ull, p }; }
	template<typename W>
	static outcome<W> up_a_level(const level_index::point<W>& p) noexcept { return { false, 0ull, { p.level + 1, p.index } }; }
	template<typename W>
	static outcome<W> exact_magnitude(std::uint64_t mag) noexcept { return { true, mag > maxpos_bits ? maxpos_bits : mag, { 0, W(0) } }; }
	// psi + 1 and psi - 1: exact on a point, and on an exact magnitude up to saturation
	template<typename W>
	static outcome<W> up_a_level(const outcome<W>& r) noexcept {
		return r.exact ? exact_magnitude<W>(r.mag > maxpos_bits - one_bits ? maxpos_bits : r.mag + one_bits) : up_a_level(r.p);
	}
	template<typename W>
	static outcome<W> down_a_level(const outcome<W>& r) noexcept {
		return r.exact ? exact_magnitude<W>(r.mag - one_bits) : from_point(level_index::point<W>{ r.p.level - 1, r.p.index });
	}

	template<typename W>
	static bool point_less(const level_index::point<W>& a, const level_index::point<W>& b) noexcept {
		return (a.level < b.level) || (a.level == b.level && a.index < b.index);
	}
	template<typename W>
	static bool is_zero_point(const level_index::point<W>& a) noexcept { return a.level == 0 && a.index == W(0); }

	// psi(phi(a) + phi(b)) and psi(|phi(a) - phi(b)|) for points in any order
	template<typename W>
	static outcome<W> add_points(const level_index::point<W>& a, const level_index::point<W>& b) noexcept {
		return from_point(point_less(a, b) ? level_index::add(b, a) : level_index::add(a, b));
	}
	template<typename W>
	static outcome<W> sub_points(const level_index::point<W>& a, const level_index::point<W>& b) noexcept {
		return from_point(point_less(a, b) ? level_index::sub(b, a) : level_index::sub(a, b));
	}

	// psi(phi(A) * phi(B)) for points A, B >= 0, in W throughout
	template<typename W>
	static outcome<W> mul_points(level_index::point<W> A, level_index::point<W> B) noexcept {
		namespace li = level_index;
		using std::log;
		if (point_less(A, B)) { const li::point<W> t = A; A = B; B = t; }
		if (is_zero_point(B)) return from_point(li::point<W>{ 0, W(0) });
		if (A.level == 0) return from_point(li::psi(W(A.index * B.index)));      // both below 1
		if (B.level == 0) {
			// X * g, g < 1: ln(Xg) = phi(x - 1) - ln(1/g)
			const li::point<W> p{ A.level - 1, A.index };
			const li::point<W> q = li::psi(W(-log(B.index)));
			if (point_less(q, p)) return up_a_level(li::sub(p, q));
			// the product is at most 1, so X <= 1/g is finite
			return from_point(li::psi(W(li::phi(A) * B.index)));
		}
		return up_a_level(add_points(li::point<W>{ A.level - 1, A.index }, li::point<W>{ B.level - 1, B.index }).p);
	}

	// psi(phi(A) / phi(B)) for points A >= 0, B > 0, in W throughout
	template<typename W>
	static outcome<W> div_points(const level_index::point<W>& A, const level_index::point<W>& B) noexcept {
		namespace li = level_index;
		using std::exp;
		using std::log;
		if (is_zero_point(A)) return from_point(li::point<W>{ 0, W(0) });
		if (A.level == B.level && A.index == B.index) return from_point(li::point<W>{ 1, W(0) });   // X / X = 1
		if (A.level == 0 && B.level == 0) return from_point(li::psi(W(A.index / B.index)));
		if (B.level == 0) {
			// X / g: ln(X/g) = phi(x - 1) + ln(1/g)
			return up_a_level(add_points(li::point<W>{ A.level - 1, A.index }, li::psi(W(-log(B.index)))).p);
		}
		if (A.level == 0) {
			// g / Y = g e^-L with L = phi(y - 1) >= 0; L past W is inf, and e^-inf = 0
			const W L = li::phi(li::point<W>{ B.level - 1, B.index });
			return from_point(li::psi(W(A.index * exp(-L))));
		}
		const li::point<W> p{ A.level - 1, A.index }, q{ B.level - 1, B.index };
		if (point_less(B, A)) return up_a_level(li::sub(p, q));
		// X / Y = e^-D with D = phi(y - 1) - phi(x - 1) > 0: the quotient is at level 0
		const W D = li::phi(li::sub(q, p));
		return from_point(li::psi(W(exp(-D))));
	}

	// the magnitude of a outcome, rounded to nearest even
	template<typename W>
	static std::uint64_t round_outcome(const outcome<W>& r, bool& near_tie) noexcept {
		near_tie = false;
		return r.exact ? r.mag : round_point<W>(r.p.level, r.p.index, near_tie);
	}

	// The lindex of the outcome compute(W{}) returns, rounded once.  compute is a generic
	// callable evaluated in ArithmeticWorking; a result that lands inside W's band around a
	// midpoint is evaluated again one stage wider: double, then dd_cascade, then qd_cascade.
	// Random results almost never need the third stage, but structured ones do: e^(2^-60) / 2
	// = 1/2 + 2^-61 + 2^-122 puts sqrt of the encoding one ulp above level 2 of lindex<64,3>
	// within 2^-122 of a midpoint, which dd_cascade cannot see.
	template<typename Compute>
	static lindex round_psi(Compute&& compute, bool negative) noexcept {
		bool near_tie = false;
		std::uint64_t mag = round_outcome(compute(ArithmeticWorking{}), near_tie);
		if constexpr (std::is_same_v<ArithmeticWorking, double>) {
			if (near_tie) mag = round_outcome(compute(dd_cascade{}), near_tie);
		}
		if (near_tie) mag = round_outcome(compute(qd_cascade{}), near_tie);
		lindex r;
		r.set_magnitude(mag, negative);
		return r;
	}

	// the lindex nearest the Working value +-X, X > 0 finite
	static lindex from_value(const Working& X, bool negative) noexcept {
		lindex r;
		r.convert_magnitude(X, negative);
		return r;
	}

protected:
	// a * b * 2^-fbits for level-0 magnitudes a, b < 2^fbits, rounded to nearest even
	static std::uint64_t fixed_multiply(std::uint64_t a, std::uint64_t b) noexcept {
		// the 128-bit product in two 64-bit halves, without a native 128-bit type (MSVC)
		const std::uint64_t a0 = a & 0xFFFF'FFFFull, a1 = a >> 32, b0 = b & 0xFFFF'FFFFull, b1 = b >> 32;
		const std::uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
		const std::uint64_t mid = (p00 >> 32) + (p01 & 0xFFFF'FFFFull) + (p10 & 0xFFFF'FFFFull);
		const std::uint64_t lo = (p00 & 0xFFFF'FFFFull) | (mid << 32);
		const std::uint64_t hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
		// shift right by fbits, 1 <= fbits <= 61; the product is below 2^(2 fbits)
		std::uint64_t q = (hi << (64u - fbits)) | (lo >> fbits);
		const std::uint64_t r = lo & index_mask, half = 1ull << (fbits - 1u);
		if (r > half || (r == half && (q & 1ull))) ++q;
		return q;
	}
	// a * 2^fbits / b for level-0 magnitudes a < b, rounded to nearest even, by long division
	static std::uint64_t fixed_divide(std::uint64_t a, std::uint64_t b) noexcept {
		std::uint64_t q = 0, r = a;   // r < b < 2^fbits <= 2^61, so 2r never overflows
		for (unsigned i = 0; i < fbits; ++i) {
			r <<= 1;
			q <<= 1;
			if (r >= b) { r -= b; q |= 1ull; }
		}
		if (2 * r > b || (2 * r == b && (q & 1ull))) ++q;
		return q;
	}

	// |X| op |Y| for nonzero magnitude encodings (a > b for sub, a >= b for add).  Two
	// level-0 operands multiply, and divide to a quotient below 1, exactly in integers.
	template<typename W>
	static outcome<W> compute(Op op, std::uint64_t a, std::uint64_t b) noexcept {
		namespace li = level_index;
		const bool both_level0 = (a < one_bits) && (b < one_bits);
		switch (op) {
		case Op::add:
			return from_point(li::add(point_of<W>(a), point_of<W>(b)));
		case Op::sub:
			return from_point(li::sub(point_of<W>(a), point_of<W>(b)));
		case Op::mul:
			if (both_level0) return exact_magnitude<W>(fixed_multiply(a, b));
			return mul_points(point_of<W>(a), point_of<W>(b));
		case Op::div:
		default:
			if (a == b) return exact_magnitude<W>(one_bits);                      // X / X = 1
			if (both_level0 && a < b) return exact_magnitude<W>(fixed_divide(a, b));
			return div_points(point_of<W>(a), point_of<W>(b));
		}
	}

	// The magnitude of a op b, with one retry in dd_cascade near a midpoint
	static std::uint64_t evaluate(Op op, std::uint64_t a, std::uint64_t b) noexcept {
		return round_psi([&](auto w) { return compute<decltype(w)>(op, a, b); }, false).raw_bits();
	}

	// x + y, or x - y when subtract
	lindex& add_signed(const lindex& rhs, bool subtract) noexcept {
		if (isnar() || rhs.isnar()) { setnar(); return *this; }
		if (rhs.iszero()) return *this;
		const bool sy = rhs.sign() != subtract;                             // the sign of the addend
		if (iszero()) return set_magnitude(rhs.magnitude_bits(), sy);
		const std::uint64_t a = magnitude_bits(), b = rhs.magnitude_bits();
		const bool sx = sign();
		if (sx == sy) return set_magnitude(evaluate(Op::add, a > b ? a : b, a > b ? b : a), sx);
		if (a == b) { setzero(); return *this; }                            // x - x is exactly 0
		return set_magnitude(evaluate(Op::sub, a > b ? a : b, a > b ? b : a), a > b ? sx : sy);
	}

	// Working rounded to Real once
	template<typename Real>
	static Real narrow(const Working& v) noexcept {
		if constexpr (sizeof(Real) > sizeof(double)) {
			return static_cast<Real>(v.high()) + static_cast<Real>(v.low());
		}
		else if constexpr (std::numeric_limits<Real>::digits + 2 <= std::numeric_limits<double>::digits) {
			// high() is v rounded to double, and rounding that again can land on a midpoint of
			// Real that v itself is not on.  Round to odd instead: when low() is not 0 and high()
			// is even, step to the other double around v, the one toward low().  An odd double
			// is never a midpoint of a type at least 2 bits narrower, so the cast rounds once.
			double d = v.high();
			if (v.low() != 0.0 && std::isfinite(d)) {
				std::uint64_t bits = 0;
				std::memcpy(&bits, &d, sizeof(d));
				if ((bits & 1ull) == 0ull) d = std::nextafter(d, v.low() > 0.0 ? std::numeric_limits<double>::infinity()
				                                                              : -std::numeric_limits<double>::infinity());
			}
			return static_cast<Real>(d);
		}
		else {
			return static_cast<Real>(v.high());
		}
	}

	template<typename Real>
	Real to_ieee754() const noexcept {
		if (iszero()) return Real(0);
		if (isnar())  return std::numeric_limits<Real>::quiet_NaN();
		const std::uint64_t mag = magnitude_bits();
		const int l = static_cast<int>(mag >> fbits);
		const Working f = index_in<Working>(mag);
		Real v;
		if (l <= 3) {
			// phi(4) = 3814279.1, inside every native range
			v = narrow<Real>(level_index::phi(level_index::point<Working>{ l, f }));
		}
		else if (l == 4) {
			// X = e^L with L = phi(3 + f) < 3814279.1.  As 2^k e^r, r = L - k ln 2, exp only
			// sees r in [0, ln 2), and the scaling by 2^k is exact or overflows to inf.
			const Working L = level_index::phi(level_index::point<Working>{ 3, f });
			const double k = std::floor((L / ddc_ln2).high());
			if (k > static_cast<double>(std::numeric_limits<Real>::max_exponent)) {
				v = std::numeric_limits<Real>::infinity();
			}
			else {
				const Working r = L - ddc_ln2 * Working(k);
				v = std::ldexp(narrow<Real>(exp(r)), static_cast<int>(k));
			}
		}
		else {
			v = std::numeric_limits<Real>::infinity();   // phi(5) = 10^1656520.4
		}
		return sign() ? -v : v;
	}

	// Integer conversion saturates; it truncates toward zero, as a native cast does.
	// NaR has no integer: it gives the most negative one, the place NaR has in the order.
	template<typename SignedInt>
	SignedInt to_signed() const noexcept {
		if (isnar()) return std::numeric_limits<SignedInt>::min();
		const double d = to_ieee754<double>();
		const double limit = std::ldexp(1.0, std::numeric_limits<SignedInt>::digits);
		if (d >= limit)  return std::numeric_limits<SignedInt>::max();
		if (d < -limit)  return std::numeric_limits<SignedInt>::min();
		return static_cast<SignedInt>(d);
	}
	template<typename UnsignedInt>
	UnsignedInt to_unsigned() const noexcept {
		if (isnar()) return UnsignedInt(0);
		const double d = to_ieee754<double>();
		if (d <= 0.0) return UnsignedInt(0);
		if (d >= std::ldexp(1.0, std::numeric_limits<UnsignedInt>::digits)) return std::numeric_limits<UnsignedInt>::max();
		return static_cast<UnsignedInt>(d);
	}

private:
	BlockBinary _block;
};

////////////////////// comparison
// Comparing two encodings as signed integers orders them by value, because phi is
// increasing and negatives are two's complements (design doc).  NaR, 1000...0, compares
// equal to itself and below every other value: the posit convention.
template<unsigned nbits, unsigned lbits, typename bt>
constexpr int64_t signed_pattern(const lindex<nbits, lbits, bt>& v) noexcept {
	const std::uint64_t raw = v.raw_bits();
	if constexpr (nbits == 64) return static_cast<int64_t>(raw);
	else return (raw & lindex<nbits, lbits, bt>::sign_mask) ? static_cast<int64_t>(raw | ~lindex<nbits, lbits, bt>::nbits_mask)
	                                                        : static_cast<int64_t>(raw);
}

template<unsigned nbits, unsigned lbits, typename bt>
constexpr bool operator==(const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) noexcept { return a.raw_bits() == b.raw_bits(); }
template<unsigned nbits, unsigned lbits, typename bt>
constexpr bool operator!=(const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) noexcept { return !(a == b); }
template<unsigned nbits, unsigned lbits, typename bt>
constexpr bool operator< (const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) noexcept { return signed_pattern(a) < signed_pattern(b); }
template<unsigned nbits, unsigned lbits, typename bt>
constexpr bool operator> (const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) noexcept { return b < a; }
template<unsigned nbits, unsigned lbits, typename bt>
constexpr bool operator<=(const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) noexcept { return !(b < a); }
template<unsigned nbits, unsigned lbits, typename bt>
constexpr bool operator>=(const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) noexcept { return !(a < b); }

////////////////////// arithmetic operators
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator+(const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) { lindex<nbits, lbits, bt> r(a); return r += b; }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator-(const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) { lindex<nbits, lbits, bt> r(a); return r -= b; }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator*(const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) { lindex<nbits, lbits, bt> r(a); return r *= b; }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator/(const lindex<nbits, lbits, bt>& a, const lindex<nbits, lbits, bt>& b) { lindex<nbits, lbits, bt> r(a); return r /= b; }

template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator+(const lindex<nbits, lbits, bt>& a, double b) { return a + lindex<nbits, lbits, bt>(b); }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator-(const lindex<nbits, lbits, bt>& a, double b) { return a - lindex<nbits, lbits, bt>(b); }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator*(const lindex<nbits, lbits, bt>& a, double b) { return a * lindex<nbits, lbits, bt>(b); }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator/(const lindex<nbits, lbits, bt>& a, double b) { return a / lindex<nbits, lbits, bt>(b); }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator+(double a, const lindex<nbits, lbits, bt>& b) { return lindex<nbits, lbits, bt>(a) + b; }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator-(double a, const lindex<nbits, lbits, bt>& b) { return lindex<nbits, lbits, bt>(a) - b; }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator*(double a, const lindex<nbits, lbits, bt>& b) { return lindex<nbits, lbits, bt>(a) * b; }
template<unsigned nbits, unsigned lbits, typename bt>
inline lindex<nbits, lbits, bt> operator/(double a, const lindex<nbits, lbits, bt>& b) { return lindex<nbits, lbits, bt>(a) / b; }

template<unsigned nbits, unsigned lbits, typename bt>
constexpr lindex<nbits, lbits, bt> abs(const lindex<nbits, lbits, bt>& v) noexcept {
	return v.isneg() ? -v : v;
}

}} // namespace sw::universal
