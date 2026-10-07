#pragma once
// poxel_impl.hpp: implementation of the poxel, a posit with an uncertainty bit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A poxel<nbits, es> is the lattice of a posit<nbits - 1, es> with one trailing
// uncertainty bit (#1631).  It is what Gustafson calls a TILE (Posits4, Sec. 3.6):
//
//   [ sign | regime | exponent | fraction | ubit ]
//     \________ posit<nbits-1, es> ________/
//
//   ubit 0   exactly the posit value p
//   ubit 1   the open interval (p, next(p)) to the next posit on the projective circle
//
// Read as a two's-complement integer, the nbits pattern T = 2 P + u orders the tiles
// around the ring, alternating exact value and open interval; negation is the
// two's-complement negation of T.  Every real maps to exactly one tile, so conversion
// never rounds: the exact tile if the value is a lattice point, otherwise the one open
// tile that contains it.  At the ends that gives (maxpos, inf) above maxpos, (0, minpos)
// below minpos, and (-inf, -maxpos) below -maxpos -- never a silent 0 or maxpos.
//
// Arithmetic has areal's sticky-flag semantics: it computes on the operands' stored
// lower endpoints, places the exact result in its tile, and sets the ubit if that result
// is inexact OR either operand was an open tile.  The ubit is then an honest "not exact"
// flag; it does not guarantee that the tile still contains the true value once an
// inexact operand took part -- containment is the job of a pair of tiles (#1637).
#if !defined(POXEL_THROW_ARITHMETIC_EXCEPTION)
#define POXEL_THROW_ARITHMETIC_EXCEPTION 0
#endif
#if !defined(BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION)
#define BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION POXEL_THROW_ARITHMETIC_EXCEPTION
#endif
#if !defined(BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION)
#define BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION POXEL_THROW_ARITHMETIC_EXCEPTION
#endif

#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <universal/utility/bit_cast.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/number/poxel/exceptions.hpp>

namespace sw { namespace universal {

template<unsigned _nbits, unsigned _es, typename bt = std::uint8_t>
class poxel {
public:
	static_assert(_nbits <= 64, "poxel: the codec is 64-bit, nbits must be <= 64");
	static_assert(_nbits >= _es + 4, "poxel: the posit lattice needs nbits - 1 >= es + 3");
	static_assert(_es < 16, "poxel: es must keep the scale inside int");

	static constexpr unsigned nbits = _nbits;
	static constexpr unsigned es    = _es;
	static constexpr unsigned lbits = nbits - 1;                     // the posit lattice: posit<lbits, es>
	static constexpr unsigned fbits = lbits - 3 - es;                // its most fraction bits (2-bit regime)
	static constexpr int maxscale   = static_cast<int>(lbits - 2) * (1 << es);   // maxpos = 2^maxscale
	static constexpr int minscale   = -maxscale;                                  // minpos = 2^minscale

	using BlockType   = bt;
	using BlockBinary = blockbinary<nbits, bt, BinaryNumberType::Signed>;

	// trivially constructible: no member initializers
	poxel() = default;
	constexpr poxel(const poxel&) = default;
	constexpr poxel(poxel&&) = default;
	constexpr poxel& operator=(const poxel&) = default;
	constexpr poxel& operator=(poxel&&) = default;

	constexpr poxel(const SpecificValue code) noexcept : _block{} {
		switch (code) {
		case SpecificValue::maxpos: maxpos(); break;
		case SpecificValue::minpos: minpos(); break;
		case SpecificValue::minneg: minneg(); break;
		case SpecificValue::maxneg: maxneg(); break;
		case SpecificValue::infpos: setbits(tile_pattern(lattice_maxpos, true)); break;          // (maxpos, inf)
		case SpecificValue::infneg: setbits(tile_pattern(lattice_nar, true)); break;             // (-inf, -maxpos)
		case SpecificValue::qnan:
		case SpecificValue::snan:
		case SpecificValue::nar:    setnar(); break;
		case SpecificValue::zero:
		default:                    setzero(); break;
		}
	}

	constexpr poxel(signed char v)        noexcept : _block{} { *this = v; }
	constexpr poxel(short v)              noexcept : _block{} { *this = v; }
	constexpr poxel(int v)                noexcept : _block{} { *this = v; }
	constexpr poxel(long v)               noexcept : _block{} { *this = v; }
	constexpr poxel(long long v)          noexcept : _block{} { *this = v; }
	constexpr poxel(char v)               noexcept : _block{} { *this = v; }
	constexpr poxel(unsigned char v)      noexcept : _block{} { *this = v; }
	constexpr poxel(unsigned short v)     noexcept : _block{} { *this = v; }
	constexpr poxel(unsigned int v)       noexcept : _block{} { *this = v; }
	constexpr poxel(unsigned long v)      noexcept : _block{} { *this = v; }
	constexpr poxel(unsigned long long v) noexcept : _block{} { *this = v; }
	CONSTEXPRESSION poxel(float v)        noexcept : _block{} { *this = v; }
	CONSTEXPRESSION poxel(double v)       noexcept : _block{} { *this = v; }
	poxel(long double v)                  noexcept : _block{} { *this = v; }

	constexpr poxel& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr poxel& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr poxel& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr poxel& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr poxel& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr poxel& operator=(char rhs)               noexcept {
		if constexpr (std::is_signed_v<char>) return convert_signed(rhs);
		else return convert_unsigned(static_cast<unsigned char>(rhs));
	}
	constexpr poxel& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr poxel& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr poxel& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr poxel& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr poxel& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	CONSTEXPRESSION poxel& operator=(float rhs)        noexcept { return convert_double(static_cast<double>(rhs)); }
	CONSTEXPRESSION poxel& operator=(double rhs)       noexcept { return convert_double(rhs); }
	poxel& operator=(long double rhs)                  noexcept { return convert_long_double(rhs); }

	// Conversion out yields the tile's lower endpoint, as areal does; (-inf, -maxpos)
	// gives -infinity.  lower() and upper() name both ends explicitly.
	explicit operator float()       const noexcept { return static_cast<float>(lower<double>()); }
	explicit operator double()      const noexcept { return lower<double>(); }
	explicit operator long double() const noexcept { return lower<long double>(); }
	explicit operator int()         const noexcept { return static_cast<int>(lower<double>()); }
	explicit operator long long()   const noexcept { return static_cast<long long>(lower<long double>()); }

	template<typename Real = double>
	Real lower() const noexcept {
		if (isnar()) return std::numeric_limits<Real>::quiet_NaN();
		const std::int64_t P = lattice();
		if (P == lattice_nar) return -std::numeric_limits<Real>::infinity();   // the (-inf, -maxpos) tile
		return lattice_value<Real>(P);
	}
	template<typename Real = double>
	Real upper() const noexcept {
		if (isnar()) return std::numeric_limits<Real>::quiet_NaN();
		const std::int64_t P = lattice();
		if (!ubit()) return lattice_value<Real>(P);
		if (P == lattice_maxpos) return std::numeric_limits<Real>::infinity();  // the (maxpos, inf) tile
		return lattice_value<Real>(next_lattice(P));
	}

	// arithmetic: sticky-flag semantics (see the file comment)
	constexpr poxel operator-() const noexcept {
		poxel r;
		r.setbits(negate_pattern(raw()));                     // NaR and exact zero map to themselves
		return r;
	}
	constexpr poxel& operator+=(const poxel& rhs) {
		if (isnar() || rhs.isnar()) return nar_operand();
		const bool uncertain = ubit() || rhs.ubit();
		if (lower_is_zero() && rhs.lower_is_zero()) { setbits(tile_pattern(0, uncertain)); return *this; }
		if (lower_is_zero()) { *this = rhs; return mark(uncertain); }
		if (rhs.lower_is_zero()) return mark(uncertain);
		blocktriple<fbits, BlockTripleOperator::ADD, bt> a, b, sum;
		normalize(a);
		rhs.normalize(b);
		sum.add(a, b);
		return assign(sum, uncertain);
	}
	constexpr poxel& operator-=(const poxel& rhs) { return *this += -rhs; }
	constexpr poxel& operator*=(const poxel& rhs) {
		if (isnar() || rhs.isnar()) return nar_operand();
		const bool uncertain = ubit() || rhs.ubit();
		if (lower_is_zero() || rhs.lower_is_zero()) { setbits(tile_pattern(0, uncertain)); return *this; }
		blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;
		normalize(a);
		rhs.normalize(b);
		product.mul(a, b);
		return assign(product, uncertain);
	}
	constexpr poxel& operator/=(const poxel& rhs) {
#if POXEL_THROW_ARITHMETIC_EXCEPTION
		if (rhs.lower_is_zero()) throw poxel_divide_by_zero{};     // exact zero and the (0, minpos) tile alike
		if (rhs.isnar())  throw poxel_divide_by_nar{};
		if (isnar())      throw poxel_numerator_is_nar{};
#else
		if (rhs.iszero() || rhs.isnar() || isnar()) { setnar(); return *this; }
#endif
		const bool uncertain = ubit() || rhs.ubit();
		if (rhs.lower_is_zero()) { setnar(); return *this; }        // (0, minpos) as divisor: its representative is 0
		if (lower_is_zero()) { setbits(tile_pattern(0, uncertain)); return *this; }
		blocktriple<fbits, BlockTripleOperator::DIV, bt> a, b, ratio;
		normalize(a);
		rhs.normalize(b);
		ratio.div(a, b);
		return assign(ratio, uncertain);
	}

	// ++ and -- step to the adjacent tile on the ring
	constexpr poxel& operator++() noexcept { setbits(raw() + 1ull); return *this; }
	constexpr poxel  operator++(int) noexcept { poxel t(*this); ++(*this); return t; }
	constexpr poxel& operator--() noexcept { setbits(raw() - 1ull); return *this; }
	constexpr poxel  operator--(int) noexcept { poxel t(*this); --(*this); return t; }

	// queries
	constexpr bool ubit()    const noexcept { return (raw() & 1ull) != 0ull; }
	constexpr bool isexact() const noexcept { return !ubit(); }
	constexpr bool isnar()   const noexcept { return raw() == nar_pattern; }
	constexpr bool iszero()  const noexcept { return raw() == 0ull; }           // exactly zero
	constexpr bool sign()    const noexcept { return _block.test(nbits - 1); }   // the lower end is negative
	constexpr bool isneg()   const noexcept { return sign(); }
	constexpr bool ispos()   const noexcept { return !sign(); }
	// the lattice index P of T = 2 P + u, as a signed lbits-bit integer
	constexpr std::int64_t lattice() const noexcept { return signed_value(raw()) >> 1; }

	// setters and bit access
	constexpr void clear()   noexcept { _block.clear(); }
	constexpr void setzero() noexcept { _block.clear(); }
	constexpr void setnar()  noexcept { setbits(nar_pattern); }
	constexpr poxel& maxpos() noexcept { setbits(tile_pattern(lattice_maxpos, false)); return *this; }
	constexpr poxel& minpos() noexcept { setbits(tile_pattern(1, false)); return *this; }
	constexpr poxel& minneg() noexcept { setbits(tile_pattern(-1, false)); return *this; }
	constexpr poxel& maxneg() noexcept { setbits(tile_pattern(-lattice_maxpos, false)); return *this; }
	constexpr poxel& setbits(std::uint64_t value) noexcept { _block.setbits(value & pattern_mask); return *this; }
	constexpr poxel& setbit(unsigned index, bool v = true) noexcept { _block.setbit(index, v); return *this; }
	constexpr poxel& setubit(bool v = true) noexcept { _block.setbit(0, v); return *this; }
	constexpr BlockBinary bits() const noexcept { return _block; }
	constexpr std::uint64_t raw() const noexcept { return _block.to_ull() & pattern_mask; }

	// The fields of a lattice point P (non-zero, non-NaR), as a posit<lbits, es> reads them.
	struct fields {
		bool          negative;
		int           k;          // regime value
		unsigned      rlen;       // regime size in bits
		unsigned      ebits;      // exponent bits present (fewer than es at the extremes)
		int           scale;      // k 2^es + e, the truncated exponent padded with zeros
		std::uint64_t frac;       // fraction field, nf bits
		unsigned      nf;         // fraction width
	};
	// the real value of lattice point P (NaN for the NaR index)
	template<typename Real>
	static Real lattice_point(std::int64_t P) noexcept { return lattice_value<Real>(P); }

	static constexpr fields decode_lattice(std::int64_t P) noexcept {
		fields d{};
		d.negative = P < 0;
		const std::uint64_t a = static_cast<std::uint64_t>(d.negative ? -P : P);   // magnitude pattern, lbits - 1 bits
		const unsigned W = lbits - 1;                                              // payload after the sign
		const unsigned top = W - 1;
		const bool first = ((a >> top) & 1ull) != 0ull;
		unsigned run = 1;
		while (run < W && (((a >> (top - run)) & 1ull) != 0ull) == first) ++run;
		d.rlen = (run < W) ? run + 1u : W;
		d.k = first ? static_cast<int>(run) - 1 : -static_cast<int>(run);
		const unsigned remaining = W - d.rlen;
		d.ebits = (remaining < es) ? remaining : es;
		d.nf = remaining - d.ebits;
		const unsigned e = (d.ebits == 0) ? 0u : static_cast<unsigned>((a >> d.nf) & ((1ull << d.ebits) - 1ull)) << (es - d.ebits);
		d.frac = (d.nf == 0) ? 0ull : (a & ((1ull << d.nf) - 1ull));
		d.scale = d.k * (1 << es) + static_cast<int>(e);
		return d;
	}

	// The tile of (-1)^neg (1 + frac / 2^q) 2^scale, with `sticky` standing in for nonzero
	// bits below frac: truncate the posit encoding of the magnitude and note whether any bit
	// was dropped.  Exact gives T = 2 P; inexact gives the open tile 2 P + 1 whose interval
	// contains the value -- above the truncated magnitude when positive, below it when
	// negative.  Returns the nbits pattern.
	static constexpr std::uint64_t encode(bool neg, int scale, std::uint64_t frac, unsigned q, bool sticky) noexcept {
		constexpr unsigned W = lbits - 1;
		constexpr int useed = 1 << es;
		int k = scale / useed;
		int e = scale % useed;
		if (e < 0) { e += useed; --k; }                            // floor division
		std::uint64_t m = 0;                                       // truncated magnitude pattern
		bool dropped = sticky;
		if (k >= static_cast<int>(W)) {                           // above maxpos
			m = static_cast<std::uint64_t>(lattice_maxpos);
			dropped = true;
		}
		else if (-k >= static_cast<int>(W)) {                     // below minpos
			m = 0;
			dropped = true;
		}
		else {
			unsigned pos = W;                                      // bits still free, filled MSB first
			auto put = [&](std::uint64_t value, unsigned len) {
				if (len == 0) return;
				if (pos == 0) { dropped = dropped || value != 0ull; return; }
				if (len <= pos) { m |= value << (pos - len); pos -= len; return; }
				const unsigned cut = len - pos;
				m |= value >> cut;
				dropped = dropped || (value & ((1ull << cut) - 1ull)) != 0ull;
				pos = 0;
			};
			if (k >= 0) put(((1ull << (k + 1)) - 1ull) << 1, static_cast<unsigned>(k) + 2u);   // 1..10
			else        put(1ull, static_cast<unsigned>(-k) + 1u);                                // 0..01
			put(static_cast<std::uint64_t>(e), es);
			put(frac, q);
		}
		if (!neg) return tile_pattern(static_cast<std::int64_t>(m), dropped);
		// negative: the exact tile is -P; an inexact value lies below -(magnitude), in the open
		// tile that starts at -(m + 1) -- NaR, the (-inf, -maxpos) tile, when m is maxpos
		const std::int64_t P = static_cast<std::int64_t>(m);
		return dropped ? tile_pattern(negate_lattice(P + 1), true) : tile_pattern(negate_lattice(P), false);
	}

	// decompose the lower endpoint for blocktriple arithmetic (hidden bit at fbits)
	template<BlockTripleOperator op>
	constexpr void normalize(blocktriple<fbits, op, bt>& tgt) const noexcept {
		std::int64_t P = lattice();
		if (P == lattice_nar) P = -lattice_maxpos;               // (-inf, -maxpos): its finite end represents it
		if (P == 0) { tgt.setzero(); return; }
		const fields d = decode_lattice(P);
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

	// place an exact blocktriple result in its tile; `uncertain` forces the ubit
	template<BlockTripleOperator op>
	constexpr poxel& assign(const blocktriple<fbits, op, bt>& v, bool uncertain) noexcept {
		if (v.isnan() || v.isinf()) { setnar(); return *this; }
		if (v.iszero()) { setbits(tile_pattern(0, uncertain)); return *this; }
		using BT = blocktriple<fbits, op, bt>;
		const int sigScale = v.significandscale();
		const int msbPos = static_cast<int>(BT::radix) + sigScale;
		constexpr unsigned q = fbits + 2u;
		std::uint64_t frac = 0;
		for (unsigned i = 0; i < q; ++i) {
			const int src = msbPos - 1 - static_cast<int>(i);
			const bool b = (src >= 0 && src < static_cast<int>(BT::bfbits)) ? v.at(static_cast<unsigned>(src)) : false;
			frac = (frac << 1) | (b ? 1ull : 0ull);
		}
		const int below = msbPos - 1 - static_cast<int>(q);
		const bool sticky = (below >= 0) && v.any(static_cast<unsigned>(below));
		setbits(encode(v.sign(), v.scale() + sigScale, frac, q, sticky));
		return mark(uncertain);
	}

	static constexpr std::uint64_t pattern_mask   = (nbits == 64) ? ~0ull : ((1ull << nbits) - 1ull);
	static constexpr std::uint64_t nar_pattern    = 1ull << (nbits - 1);                 // the exact NaR tile
	static constexpr std::int64_t  lattice_nar    = -(std::int64_t(1) << (lbits - 1));  // P of NaR
	static constexpr std::int64_t  lattice_maxpos = (std::int64_t(1) << (lbits - 1)) - 1;

	static constexpr std::uint64_t negate_pattern(std::uint64_t p) noexcept { return (~p + 1ull) & pattern_mask; }
	static constexpr std::uint64_t tile_pattern(std::int64_t P, bool u) noexcept {
		return ((static_cast<std::uint64_t>(P) << 1) | (u ? 1ull : 0ull)) & pattern_mask;
	}
	// signed value of an nbits pattern
	static constexpr std::int64_t signed_value(std::uint64_t p) noexcept {
		if constexpr (nbits == 64) return static_cast<std::int64_t>(p);
		else return ((p >> (nbits - 1)) & 1ull) ? static_cast<std::int64_t>(p | ~pattern_mask) : static_cast<std::int64_t>(p);
	}

private:
	BlockBinary _block;

	constexpr poxel& mark(bool uncertain) noexcept { if (uncertain) setubit(true); return *this; }
	// the stored lower endpoint is zero: exact 0, or the (0, minpos) tile
	constexpr bool lower_is_zero() const noexcept { return lattice() == 0; }

	constexpr poxel& nar_operand() {
#if POXEL_THROW_ARITHMETIC_EXCEPTION
		throw poxel_operand_is_nar{};
#else
		setnar();
		return *this;
#endif
	}

	// lattice arithmetic on lbits-bit two's complement
	static constexpr std::int64_t negate_lattice(std::int64_t P) noexcept {
		if (P == lattice_nar) return lattice_nar;
		if (P == lattice_maxpos + 1) return lattice_nar;          // -(maxpos + 1): NaR
		return -P;
	}
	static constexpr std::int64_t next_lattice(std::int64_t P) noexcept {
		return (P == lattice_maxpos) ? lattice_nar : P + 1;
	}
	template<typename Real>
	static Real lattice_value(std::int64_t P) noexcept {
		if (P == 0) return Real(0);
		if (P == lattice_nar) return std::numeric_limits<Real>::quiet_NaN();
		const fields d = decode_lattice(P);
		const Real v = std::ldexp(static_cast<Real>((1ull << d.nf) | d.frac), d.scale - static_cast<int>(d.nf));
		return d.negative ? -v : v;
	}

	template<typename UnsignedInt>
	constexpr poxel& convert_magnitude(bool neg, UnsignedInt v) noexcept {
		const std::uint64_t mag = static_cast<std::uint64_t>(v);
		if (mag == 0ull) { setzero(); return *this; }
		const unsigned msb = static_cast<unsigned>(std::bit_width(mag)) - 1u;
		const std::uint64_t frac = (msb == 0u) ? 0ull : (mag & ((1ull << msb) - 1ull));
		setbits(encode(neg, static_cast<int>(msb), frac, msb, false));
		return *this;
	}
	template<typename SignedInt>
	constexpr poxel& convert_signed(SignedInt v) noexcept {
		const bool neg = v < 0;
		const std::uint64_t mag = neg ? (0ull - static_cast<std::uint64_t>(static_cast<long long>(v))) : static_cast<std::uint64_t>(v);
		return convert_magnitude(neg, mag);
	}
	template<typename UnsignedInt>
	constexpr poxel& convert_unsigned(UnsignedInt v) noexcept { return convert_magnitude(false, v); }

	CONSTEXPRESSION poxel& convert_double(double v) noexcept {
		const std::uint64_t u = sw::bit_cast<std::uint64_t>(v);
		const bool neg = (u >> 63) != 0ull;
		const unsigned biased = static_cast<unsigned>((u >> 52) & 0x7FFull);
		std::uint64_t frac = u & 0x000F'FFFF'FFFF'FFFFull;
		if (biased == 0x7FFu) {
			if (frac != 0ull) { setnar(); return *this; }                 // NaN
			setbits(neg ? tile_pattern(lattice_nar, true) : tile_pattern(lattice_maxpos, true));   // +/-inf: the end tiles
			return *this;
		}
		if (biased == 0u) {
			if (frac == 0ull) { setzero(); return *this; }
			const unsigned msb = static_cast<unsigned>(std::bit_width(frac)) - 1u;
			frac &= (msb == 0u) ? 0ull : ((1ull << msb) - 1ull);
			setbits(encode(neg, static_cast<int>(msb) - 1074, frac, msb, false));
			return *this;
		}
		setbits(encode(neg, static_cast<int>(biased) - 1023, frac, 52u, false));
		return *this;
	}
	poxel& convert_long_double(long double v) noexcept {
		if constexpr (std::numeric_limits<long double>::digits <= 53) {
			return convert_double(static_cast<double>(v));
		}
		else {
			if (v != v) { setnar(); return *this; }
			if (v == std::numeric_limits<long double>::infinity())  { setbits(tile_pattern(lattice_maxpos, true)); return *this; }
			if (v == -std::numeric_limits<long double>::infinity()) { setbits(tile_pattern(lattice_nar, true)); return *this; }
			if (v == 0.0l) { setzero(); return *this; }
			const bool neg = v < 0.0l;
			int exp = 0;
			const long double m = std::frexp(neg ? -v : v, &exp);
			const long double scaled = std::ldexp(m, 64);
			const long double whole  = std::floor(scaled);
			const std::uint64_t sig  = static_cast<std::uint64_t>(whole);
			setbits(encode(neg, exp - 1, sig & ~(1ull << 63), 63u, scaled != whole));
			return *this;
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
// free operators

template<unsigned nbits, unsigned es, typename bt>
constexpr poxel<nbits, es, bt> operator+(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) { poxel<nbits, es, bt> r(a); return r += b; }
template<unsigned nbits, unsigned es, typename bt>
constexpr poxel<nbits, es, bt> operator-(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) { poxel<nbits, es, bt> r(a); return r -= b; }
template<unsigned nbits, unsigned es, typename bt>
constexpr poxel<nbits, es, bt> operator*(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) { poxel<nbits, es, bt> r(a); return r *= b; }
template<unsigned nbits, unsigned es, typename bt>
constexpr poxel<nbits, es, bt> operator/(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) { poxel<nbits, es, bt> r(a); return r /= b; }

template<unsigned nbits, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr poxel<nbits, es, bt> operator+(const poxel<nbits, es, bt>& a, Native b) { return a + poxel<nbits, es, bt>(b); }
template<unsigned nbits, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr poxel<nbits, es, bt> operator+(Native a, const poxel<nbits, es, bt>& b) { return poxel<nbits, es, bt>(a) + b; }
template<unsigned nbits, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr poxel<nbits, es, bt> operator-(const poxel<nbits, es, bt>& a, Native b) { return a - poxel<nbits, es, bt>(b); }
template<unsigned nbits, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr poxel<nbits, es, bt> operator-(Native a, const poxel<nbits, es, bt>& b) { return poxel<nbits, es, bt>(a) - b; }
template<unsigned nbits, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr poxel<nbits, es, bt> operator*(const poxel<nbits, es, bt>& a, Native b) { return a * poxel<nbits, es, bt>(b); }
template<unsigned nbits, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr poxel<nbits, es, bt> operator*(Native a, const poxel<nbits, es, bt>& b) { return poxel<nbits, es, bt>(a) * b; }
template<unsigned nbits, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr poxel<nbits, es, bt> operator/(const poxel<nbits, es, bt>& a, Native b) { return a / poxel<nbits, es, bt>(b); }
template<unsigned nbits, unsigned es, typename bt, typename Native, typename = std::enable_if_t<std::is_arithmetic_v<Native>>>
constexpr poxel<nbits, es, bt> operator/(Native a, const poxel<nbits, es, bt>& b) { return poxel<nbits, es, bt>(a) / b; }

// Order is tile order around the ring: integer order of the patterns.  An exact value and
// the open tile above it are distinct; NaR is the most negative pattern.
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator==(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept { return a.raw() == b.raw(); }
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator!=(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept { return !(a == b); }
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator< (const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept {
	using P = poxel<nbits, es, bt>;
	return P::signed_value(a.raw()) < P::signed_value(b.raw());
}
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator> (const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept { return b < a; }
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator<=(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept { return !(b < a); }
template<unsigned nbits, unsigned es, typename bt>
constexpr bool operator>=(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept { return !(a < b); }

template<unsigned nbits, unsigned es, typename bt>
constexpr poxel<nbits, es, bt> abs(const poxel<nbits, es, bt>& v) noexcept { return (v.isneg() && !v.isnar()) ? -v : v; }

}} // namespace sw::universal
