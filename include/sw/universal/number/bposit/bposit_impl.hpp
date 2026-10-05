#pragma once
// bposit_impl.hpp: implementation of the bounded posit, Gustafson's b-posit <N, rS, eS>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Encoding (docs/number-systems/bposit.md, #1251):
//
//   [ sign | regime: 2..rs bits | exponent: es bits | fraction: the rest ]
//
// The regime is a run of identical bits that COUNTS its terminating bit: a run shorter
// than rs ends with one opposite bit, and a run that reaches rs ends with none.  So the
// regime takes 2..rs bits, its value r = run - 1 (ones) or -run (zeros) lies in
// [-rs, rs - 1], and the exponent is always the full es bits.  Every encoding keeps at
// least F_min = nbits - 1 - rs - es >= 1 fraction bits.
//
//   x = (-1)^sign (1 + f) 2^(r 2^es + e),   scale in [-rs 2^es, rs 2^es - 1]
//
// A negative value is the two's complement of its magnitude's encoding; zero is 0...0
// and NaR is 10...0.  Encodings are monotonic, so integer order is value order.
//
// Rounding and saturation follow the posit standard: round to nearest, ties to the even
// encoding, never round a nonzero value to 0 or a finite one to NaR, and clamp to
// +/-minpos and +/-maxpos.
//
// The codec works on uint64_t, which is why nbits is limited to 64; that covers the
// standard 16/32/64-bit configurations.  Arithmetic decodes into blocktriple, which
// carries the guard and sticky information, and encodes the result once.
#if !defined(BPOSIT_THROW_ARITHMETIC_EXCEPTION)
#define BPOSIT_THROW_ARITHMETIC_EXCEPTION 0
#endif
#if !defined(BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION)
#define BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION BPOSIT_THROW_ARITHMETIC_EXCEPTION
#endif
#if !defined(BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION)
#define BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION BPOSIT_THROW_ARITHMETIC_EXCEPTION
#endif

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>
#include <universal/utility/bit_cast.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/bposit/exceptions.hpp>

namespace sw { namespace universal {

template<unsigned _nbits, unsigned _rs, unsigned _es, typename bt = std::uint8_t>
class bposit {
public:
	static_assert(_nbits <= 64, "bposit: the codec is 64-bit, nbits must be <= 64");
	static_assert(_rs >= 2, "bposit: the maximum regime size rs must be at least 2");
	static_assert(_rs < _nbits - 1, "bposit: a b-posit requires rs < nbits - 1");
	static_assert(_nbits > 1 + _rs + _es, "bposit: nbits must exceed 1 + rs + es, so every encoding keeps a fraction bit");
	static_assert(_es < 31, "bposit: es must leave the scale inside int");

	static constexpr unsigned nbits    = _nbits;
	static constexpr unsigned rs       = _rs;                         // maximum regime size
	static constexpr unsigned es       = _es;                         // exponent size, always present
	static constexpr unsigned rbits    = rs;
	static constexpr unsigned ebits    = es;
	static constexpr unsigned fbits    = nbits - 3 - es;              // most fraction bits (2-bit regime)
	static constexpr unsigned fbitsmin = nbits - 1 - rs - es;         // fraction bits at every magnitude
	static constexpr int      maxscale = static_cast<int>(rs << es) - 1;
	static constexpr int      minscale = -static_cast<int>(rs << es);

	using BlockType   = bt;
	using BlockBinary = blockbinary<nbits, bt, BinaryNumberType::Signed>;

	// trivially constructible: no member initializers
	bposit() = default;
	constexpr bposit(const bposit&) = default;
	constexpr bposit(bposit&&) = default;
	constexpr bposit& operator=(const bposit&) = default;
	constexpr bposit& operator=(bposit&&) = default;

	constexpr bposit(const SpecificValue code) noexcept : _block{} {
		switch (code) {
		case SpecificValue::infpos:
		case SpecificValue::maxpos:  maxpos();  break;
		case SpecificValue::minpos:  minpos();  break;
		case SpecificValue::minneg:  minneg();  break;
		case SpecificValue::infneg:
		case SpecificValue::maxneg:  maxneg();  break;
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:     setnar();  break;
		case SpecificValue::zero:
		default:                     setzero(); break;
		}
	}

	constexpr bposit(signed char v)        noexcept : _block{} { *this = v; }
	constexpr bposit(short v)              noexcept : _block{} { *this = v; }
	constexpr bposit(int v)                noexcept : _block{} { *this = v; }
	constexpr bposit(long v)               noexcept : _block{} { *this = v; }
	constexpr bposit(long long v)          noexcept : _block{} { *this = v; }
	constexpr bposit(char v)               noexcept : _block{} { *this = v; }
	constexpr bposit(unsigned char v)      noexcept : _block{} { *this = v; }
	constexpr bposit(unsigned short v)     noexcept : _block{} { *this = v; }
	constexpr bposit(unsigned int v)       noexcept : _block{} { *this = v; }
	constexpr bposit(unsigned long v)      noexcept : _block{} { *this = v; }
	constexpr bposit(unsigned long long v) noexcept : _block{} { *this = v; }
	CONSTEXPRESSION bposit(float v)        noexcept : _block{} { *this = v; }
	CONSTEXPRESSION bposit(double v)       noexcept : _block{} { *this = v; }
	bposit(long double v)                  noexcept : _block{} { *this = v; }

	constexpr bposit& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr bposit& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr bposit& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr bposit& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr bposit& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr bposit& operator=(char rhs)               noexcept {
		if constexpr (std::is_signed_v<char>) return convert_signed(rhs);
		else return convert_unsigned(static_cast<unsigned char>(rhs));
	}
	constexpr bposit& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr bposit& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr bposit& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr bposit& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr bposit& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	CONSTEXPRESSION bposit& operator=(float rhs)        noexcept { return convert_double(static_cast<double>(rhs)); }
	CONSTEXPRESSION bposit& operator=(double rhs)       noexcept { return convert_double(rhs); }
	bposit& operator=(long double rhs)                  noexcept { return convert_long_double(rhs); }

	// conversion out
	explicit operator float()       const noexcept { return static_cast<float>(to_native<double>()); }
	explicit operator double()      const noexcept { return to_native<double>(); }
	explicit operator long double() const noexcept { return to_native<long double>(); }
	explicit operator int()         const noexcept { return static_cast<int>(to_native<double>()); }
	explicit operator long()        const noexcept { return static_cast<long>(to_native<long double>()); }
	explicit operator long long()   const noexcept { return static_cast<long long>(to_native<long double>()); }
	explicit operator unsigned int()       const noexcept { return static_cast<unsigned int>(to_native<double>()); }
	explicit operator unsigned long()      const noexcept { return static_cast<unsigned long>(to_native<long double>()); }
	explicit operator unsigned long long() const noexcept { return static_cast<unsigned long long>(to_native<long double>()); }

	// arithmetic
	constexpr bposit operator-() const noexcept {
		bposit r;
		r.setbits(negate_pattern(raw()));                    // NaR and zero map to themselves
		return r;
	}
	constexpr bposit& operator+=(const bposit& rhs) {
		if (isnar() || rhs.isnar()) return nar_operand();
		if (iszero()) { *this = rhs; return *this; }
		if (rhs.iszero()) return *this;
		blocktriple<fbits, BlockTripleOperator::ADD, bt> a, b, sum;
		normalize(a);
		rhs.normalize(b);
		sum.add(a, b);
		return assign(sum);
	}
	constexpr bposit& operator-=(const bposit& rhs) { return *this += -rhs; }
	constexpr bposit& operator*=(const bposit& rhs) {
		if (isnar() || rhs.isnar()) return nar_operand();
		if (iszero() || rhs.iszero()) { setzero(); return *this; }
		blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;
		normalize(a);
		rhs.normalize(b);
		product.mul(a, b);
		return assign(product);
	}
	constexpr bposit& operator/=(const bposit& rhs) {
#if BPOSIT_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero()) throw bposit_divide_by_zero{};
		if (rhs.isnar())  throw bposit_divide_by_nar{};
		if (isnar())      throw bposit_numerator_is_nar{};
#else
		if (rhs.iszero() || rhs.isnar() || isnar()) { setnar(); return *this; }
#endif
		if (iszero()) return *this;
		blocktriple<fbits, BlockTripleOperator::DIV, bt> a, b, ratio;
		normalize(a);
		rhs.normalize(b);
		ratio.div(a, b);
		return assign(ratio);
	}

	// ++ and -- step to the adjacent encoding; as for posits, maxpos++ is NaR
	constexpr bposit& operator++() noexcept { setbits(raw() + 1ull); return *this; }
	constexpr bposit  operator++(int) noexcept { bposit t(*this); ++(*this); return t; }
	constexpr bposit& operator--() noexcept { setbits(raw() - 1ull); return *this; }
	constexpr bposit  operator--(int) noexcept { bposit t(*this); --(*this); return t; }

	// queries
	constexpr bool sign()   const noexcept { return _block.test(nbits - 1); }
	constexpr bool isneg()  const noexcept { return sign(); }
	constexpr bool ispos()  const noexcept { return !sign(); }
	constexpr bool iszero() const noexcept { return raw() == 0ull; }
	constexpr bool isnar()  const noexcept { return raw() == nar_pattern; }
	constexpr bool isone()  const noexcept { return raw() == one_pattern; }
	constexpr bool isminusone() const noexcept { return raw() == negate_pattern(one_pattern); }
	constexpr bool ispowerof2() const noexcept {
		if (iszero() || isnar()) return false;
		return decode_fields().frac == 0ull;
	}
	// regime value r in [-rs, rs - 1], its size in bits (2..rs), and the scale r 2^es + e
	constexpr int      regime()      const noexcept { return (iszero() || isnar()) ? 0 : decode_fields().r; }
	constexpr unsigned regime_size() const noexcept { return (iszero() || isnar()) ? 0u : decode_fields().rlen; }
	constexpr int      scale()       const noexcept { return (iszero() || isnar()) ? 0 : decode_fields().scale; }

	// setters and bit access
	constexpr void clear()   noexcept { _block.clear(); }
	constexpr void setzero() noexcept { _block.clear(); }
	constexpr void setnar()  noexcept { setbits(nar_pattern); }
	constexpr bposit& maxpos() noexcept { setbits(maxpos_pattern); return *this; }
	constexpr bposit& minpos() noexcept { setbits(1ull); return *this; }
	constexpr bposit& zero()   noexcept { setzero(); return *this; }
	constexpr bposit& minneg() noexcept { setbits(negate_pattern(1ull)); return *this; }
	constexpr bposit& maxneg() noexcept { setbits(negate_pattern(maxpos_pattern)); return *this; }
	constexpr bposit& setbits(std::uint64_t value) noexcept { _block.setbits(value & pattern_mask); return *this; }
	constexpr bposit& setbit(unsigned index, bool v = true) noexcept { _block.setbit(index, v); return *this; }
	constexpr BlockBinary bits() const noexcept { return _block; }
	constexpr std::uint64_t raw() const noexcept { return _block.to_ull() & pattern_mask; }

	// 2^k, exactly when -rs 2^es < k < rs 2^es, saturated otherwise
	static constexpr bposit pow2(int k) noexcept {
		bposit r;
		r.setbits(encode(false, k, 0ull, 0u, false));
		return r;
	}

	// The fields of a non-zero, non-NaR encoding, read from its magnitude.
	struct fields {
		bool          negative;
		int           r;          // regime value
		unsigned      rlen;       // regime size in bits, 2..rs
		unsigned      e;          // exponent field
		int           scale;      // r 2^es + e
		std::uint64_t frac;       // fraction field, nf bits
		unsigned      nf;         // fraction width
	};
	constexpr fields decode_fields() const noexcept { return decode(raw()); }

	// Encode (-1)^neg (1 + frac / 2^q) 2^scale, frac < 2^q, with `sticky` standing in
	// for any nonzero bits below frac: round to nearest even, saturate per the posit
	// standard.  Returns the nbits pattern.
	static constexpr std::uint64_t encode(bool neg, int scale, std::uint64_t frac, unsigned q, bool sticky) noexcept {
		std::uint64_t m = 0;
		if (scale > maxscale) {
			m = maxpos_pattern;
		}
		else if (scale < minscale) {
			m = 1ull;                                               // never round a nonzero value to 0
		}
		else {
			constexpr int useed = 1 << es;
			int r = scale / useed;
			int e = scale % useed;
			if (e < 0) { e += useed; --r; }                         // floor division
			unsigned rlen = 0;
			std::uint64_t regime = 0;
			if (r >= 0) {
				const unsigned run = static_cast<unsigned>(r) + 1u;
				if (run < rs) { rlen = run + 1u; regime = ((1ull << run) - 1ull) << 1; }   // 1..10
				else          { rlen = rs;       regime = (1ull << rs) - 1ull; }          // capped: no terminator
			}
			else {
				const unsigned run = static_cast<unsigned>(-r);
				if (run < rs) { rlen = run + 1u; regime = 1ull; }                         // 0..01
				else          { rlen = rs;       regime = 0ull; }                         // capped: no terminator
			}
			const unsigned nf = nbits - 1u - rlen - es;             // >= fbitsmin >= 1
			const std::uint64_t head = (regime << es) | static_cast<std::uint64_t>(e);
			std::uint64_t keep = 0;
			bool guard = false, rest = sticky;
			if (q <= nf) {
				keep = frac << (nf - q);
			}
			else {
				const unsigned drop = q - nf;
				keep  = frac >> drop;
				guard = ((frac >> (drop - 1u)) & 1ull) != 0ull;
				rest  = rest || ((drop > 1u) && (frac & ((1ull << (drop - 1u)) - 1ull)) != 0ull);
			}
			m = (head << nf) | keep;
			if (guard && (rest || (m & 1ull))) ++m;                 // encodings are monotonic: +1 is the next value
			if (m > maxpos_pattern) m = maxpos_pattern;             // a carry past maxpos stays at maxpos
			if (m == 0ull) m = 1ull;                                // 2^minscale itself is the zero pattern
		}
		return neg ? negate_pattern(m) : m;
	}

	static constexpr fields decode(std::uint64_t pattern) noexcept {
		fields d{};
		d.negative = ((pattern >> (nbits - 1)) & 1ull) != 0ull;
		const std::uint64_t a = d.negative ? negate_pattern(pattern) : pattern;
		const unsigned top = nbits - 2;                             // most significant payload bit
		const bool first = ((a >> top) & 1ull) != 0ull;
		unsigned run = 1;
		while (run < rs && (((a >> (top - run)) & 1ull) != 0ull) == first) ++run;
		d.rlen = (run == rs) ? rs : run + 1u;
		d.r = first ? static_cast<int>(run) - 1 : -static_cast<int>(run);
		const unsigned remaining = nbits - 1u - d.rlen;
		d.nf = remaining - es;
		d.e = (es == 0) ? 0u : static_cast<unsigned>((a >> d.nf) & ((1ull << es) - 1ull));
		d.frac = a & ((1ull << d.nf) - 1ull);
		d.scale = d.r * (1 << es) + static_cast<int>(d.e);
		return d;
	}

	// Decompose into blocktriple for arithmetic: hidden bit at position fbits, then the
	// operator's alignment shift (ADD: rounding bits; DIV: divshift; MUL: none).
	template<BlockTripleOperator op>
	constexpr void normalize(blocktriple<fbits, op, bt>& tgt) const noexcept {
		if (isnar())  { tgt.setnan(); return; }
		if (iszero()) { tgt.setzero(); return; }
		const fields d = decode_fields();
		const std::uint64_t sig = ((1ull << d.nf) | d.frac) << (fbits - d.nf);
		tgt.clear();
		tgt.setnormal();
		tgt.setsign(d.negative);
		tgt.setscale(d.scale);
		tgt.setradix();
		for (unsigned i = 0; i <= fbits; ++i) {
			if ((sig >> i) & 1ull) tgt.setbit(i);
		}
		using BT = blocktriple<fbits, op, bt>;
		if constexpr (op == BlockTripleOperator::ADD) tgt.bitShift(static_cast<int>(BT::rbits));
		else if constexpr (op == BlockTripleOperator::DIV) tgt.bitShift(static_cast<int>(BT::divshift));
	}

	// Round a blocktriple result into this bposit, once.
	template<BlockTripleOperator op>
	constexpr bposit& assign(const blocktriple<fbits, op, bt>& v) noexcept {
		if (v.iszero()) { setzero(); return *this; }
		if (v.isnan() || v.isinf()) { setnar(); return *this; }
		using BT = blocktriple<fbits, op, bt>;
		const int sigScale = v.significandscale();
		const int msbPos = static_cast<int>(BT::radix) + sigScale;  // the hidden bit
		constexpr unsigned q = fbits + 2u;                          // every fraction bit, a guard and one more
		std::uint64_t frac = 0;
		for (unsigned i = 0; i < q; ++i) {
			const int src = msbPos - 1 - static_cast<int>(i);
			const bool b = (src >= 0 && src < static_cast<int>(BT::bfbits)) ? v.at(static_cast<unsigned>(src)) : false;
			frac = (frac << 1) | (b ? 1ull : 0ull);
		}
		const int below = msbPos - 1 - static_cast<int>(q);         // highest bit not extracted
		const bool sticky = (below >= 0) && v.any(static_cast<unsigned>(below));
		setbits(encode(v.sign(), v.scale() + sigScale, frac, q, sticky));
		return *this;
	}

	static constexpr std::uint64_t pattern_mask   = (nbits == 64) ? ~0ull : ((1ull << nbits) - 1ull);
	static constexpr std::uint64_t nar_pattern    = 1ull << (nbits - 1);
	static constexpr std::uint64_t maxpos_pattern = nar_pattern - 1ull;
	static constexpr std::uint64_t one_pattern    = 1ull << (nbits - 2);   // regime 10, all else zero

	static constexpr std::uint64_t negate_pattern(std::uint64_t p) noexcept { return (~p + 1ull) & pattern_mask; }

private:
	BlockBinary _block;

	constexpr bposit& nar_operand() {
#if BPOSIT_THROW_ARITHMETIC_EXCEPTION
		throw bposit_operand_is_nar{};
#else
		setnar();
		return *this;
#endif
	}

	template<typename UnsignedInt>
	constexpr bposit& convert_magnitude(bool neg, UnsignedInt v) noexcept {
		const std::uint64_t mag = static_cast<std::uint64_t>(v);
		if (mag == 0ull) { setzero(); return *this; }
		const unsigned msb = static_cast<unsigned>(std::bit_width(mag)) - 1u;
		const std::uint64_t frac = (msb == 0u) ? 0ull : (mag & ((1ull << msb) - 1ull));
		setbits(encode(neg, static_cast<int>(msb), frac, msb, false));
		return *this;
	}
	template<typename SignedInt>
	constexpr bposit& convert_signed(SignedInt v) noexcept {
		const bool neg = v < 0;
		const std::uint64_t mag = neg ? (0ull - static_cast<std::uint64_t>(static_cast<long long>(v))) : static_cast<std::uint64_t>(v);
		return convert_magnitude(neg, mag);
	}
	template<typename UnsignedInt>
	constexpr bposit& convert_unsigned(UnsignedInt v) noexcept { return convert_magnitude(false, v); }

	// exact decomposition of an IEEE double: no rounding until encode
	CONSTEXPRESSION bposit& convert_double(double v) noexcept {
		const std::uint64_t u = sw::bit_cast<std::uint64_t>(v);
		const bool neg = (u >> 63) != 0ull;
		const unsigned biased = static_cast<unsigned>((u >> 52) & 0x7FFull);
		std::uint64_t frac = u & 0x000F'FFFF'FFFF'FFFFull;
		if (biased == 0x7FFu) { setnar(); return *this; }        // inf and NaN are not reals
		if (biased == 0u) {
			if (frac == 0ull) { setzero(); return *this; }
			const unsigned msb = static_cast<unsigned>(std::bit_width(frac)) - 1u;   // subnormal: normalize
			const int scale = static_cast<int>(msb) - 1074;
			frac &= (msb == 0u) ? 0ull : ((1ull << msb) - 1ull);
			setbits(encode(neg, scale, frac, msb, false));
			return *this;
		}
		setbits(encode(neg, static_cast<int>(biased) - 1023, frac, 52u, false));
		return *this;
	}
	bposit& convert_long_double(long double v) noexcept {
		if constexpr (std::numeric_limits<long double>::digits <= 53) {
			return convert_double(static_cast<double>(v));
		}
		else {
			if (v != v || v == std::numeric_limits<long double>::infinity() || v == -std::numeric_limits<long double>::infinity()) { setnar(); return *this; }
			if (v == 0.0l) { setzero(); return *this; }
			const bool neg = v < 0.0l;
			int exp = 0;
			const long double m = std::frexp(neg ? -v : v, &exp);  // m in [0.5, 1)
			const std::uint64_t sig = static_cast<std::uint64_t>(std::ldexp(m, 64));   // exact: 64-bit significand
			const std::uint64_t frac = sig & ~(1ull << 63);
			setbits(encode(neg, exp - 1, frac, 63u, false));
			return *this;
		}
	}

	template<typename Real>
	Real to_native() const noexcept {
		if (iszero()) return Real(0);
		if (isnar())  return std::numeric_limits<Real>::quiet_NaN();
		const fields d = decode_fields();
		const std::uint64_t sig = (1ull << d.nf) | d.frac;          // < 2^62: one rounding at most
		const Real v = std::ldexp(static_cast<Real>(sig), d.scale - static_cast<int>(d.nf));
		return d.negative ? -v : v;
	}
};

////////////////////////////////////////////////////////////////////////////////
// free operators

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bposit<nbits, rs, es, bt> operator+(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) { bposit<nbits, rs, es, bt> r(a); return r += b; }
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bposit<nbits, rs, es, bt> operator-(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) { bposit<nbits, rs, es, bt> r(a); return r -= b; }
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bposit<nbits, rs, es, bt> operator*(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) { bposit<nbits, rs, es, bt> r(a); return r *= b; }
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bposit<nbits, rs, es, bt> operator/(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) { bposit<nbits, rs, es, bt> r(a); return r /= b; }

// mixed with a native arithmetic literal
template<unsigned nbits, unsigned rs, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr bposit<nbits, rs, es, bt> operator+(const bposit<nbits, rs, es, bt>& a, Native b) { return a + bposit<nbits, rs, es, bt>(b); }
template<unsigned nbits, unsigned rs, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr bposit<nbits, rs, es, bt> operator+(Native a, const bposit<nbits, rs, es, bt>& b) { return bposit<nbits, rs, es, bt>(a) + b; }
template<unsigned nbits, unsigned rs, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr bposit<nbits, rs, es, bt> operator-(const bposit<nbits, rs, es, bt>& a, Native b) { return a - bposit<nbits, rs, es, bt>(b); }
template<unsigned nbits, unsigned rs, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr bposit<nbits, rs, es, bt> operator-(Native a, const bposit<nbits, rs, es, bt>& b) { return bposit<nbits, rs, es, bt>(a) - b; }
template<unsigned nbits, unsigned rs, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr bposit<nbits, rs, es, bt> operator*(const bposit<nbits, rs, es, bt>& a, Native b) { return a * bposit<nbits, rs, es, bt>(b); }
template<unsigned nbits, unsigned rs, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr bposit<nbits, rs, es, bt> operator*(Native a, const bposit<nbits, rs, es, bt>& b) { return bposit<nbits, rs, es, bt>(a) * b; }
template<unsigned nbits, unsigned rs, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr bposit<nbits, rs, es, bt> operator/(const bposit<nbits, rs, es, bt>& a, Native b) { return a / bposit<nbits, rs, es, bt>(b); }
template<unsigned nbits, unsigned rs, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr bposit<nbits, rs, es, bt> operator/(Native a, const bposit<nbits, rs, es, bt>& b) { return bposit<nbits, rs, es, bt>(a) / b; }

// Comparison is integer comparison of the two's-complement patterns: encodings are
// monotonic, and NaR (10...0) is the most negative pattern, below every real, equal
// only to itself -- the posit standard's total order.
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr std::int64_t signed_pattern(const bposit<nbits, rs, es, bt>& v) noexcept {
	const std::uint64_t p = v.raw();
	if constexpr (nbits == 64) return static_cast<std::int64_t>(p);
	else return ((p >> (nbits - 1)) & 1ull) ? static_cast<std::int64_t>(p | ~((1ull << nbits) - 1ull)) : static_cast<std::int64_t>(p);
}
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bool operator==(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) noexcept { return a.raw() == b.raw(); }
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bool operator!=(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) noexcept { return !(a == b); }
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bool operator< (const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) noexcept { return signed_pattern(a) < signed_pattern(b); }
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bool operator> (const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) noexcept { return b < a; }
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bool operator<=(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) noexcept { return !(b < a); }
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bool operator>=(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) noexcept { return !(a < b); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
constexpr bposit<nbits, rs, es, bt> abs(const bposit<nbits, rs, es, bt>& v) noexcept { return v.isneg() && !v.isnar() ? -v : v; }

}} // namespace sw::universal
