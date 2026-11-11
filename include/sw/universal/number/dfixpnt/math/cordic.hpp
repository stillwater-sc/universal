#pragma once
// cordic.hpp: a decimal CORDIC engine for dfixpnt, for hardware codesign (#2042)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// dfixpnt has no trigonometry of its own. This engine models a decimal shift-and-add CORDIC unit digit for
// digit, in dfixpnt's own decimal arithmetic, to any width, with a chosen iteration count and guard width.
// It is the decimal counterpart of the binary engine for fixpnt (fixpnt/math/cordic.hpp, #2015):
//
//   cordic::sincos(theta, s, c)            rotation mode: s = sin(theta), c = cos(theta)
//   cordic::sin(theta), cordic::cos(theta)
//   cordic::atan2(y, x), cordic::atan(x)   vectoring mode
//   cordic::hypot(x, y)
//   cordic::polar(x, y, r, theta)          r = hypot(x, y), theta = atan2(y, x)
//
// Every function takes an optional iteration count, the number of digit positions, as its last argument,
// and the number of guard digits as an optional first template argument:
//
//   cordic::sincos<2>(theta, s, c, 5);      2 guard digits, 5 digit positions
//
// The algorithm. In decimal, a shift by one digit divides by 10 exactly up to truncation, so the elementary
// angles are atan(10^-j). Position 0 is one rotation by +-atan(1) = +-pi/4, and every position j >= 1 is
// nine rotations by +-atan(10^-j), each in the direction that drives z (rotation) or y (vectoring) to 0:
//
//   x <- x - d (y >> j),   y <- y + d (x >> j),   z <- z - d atan(10^-j),   d = +-1
//
// where >> j is a shift by j digits. Every rotation is taken, in one direction or the other, so the gain
// is a constant for each count of positions n, K_n = sqrt(2) prod_{1 <= j < n} (1 + 10^-2j)^(9/2)
// -> 1.4796485628..., and rotation can start from (1/K_n, 0, theta) and end at (cos theta, sin theta), as in
// binary CORDIC. The rotations converge because each angle is at most the sum of all that follow it:
// atan(t)/t decreases, so 9 sum_{k>j} atan(10^-k) >= atan(10^-j) (1 - 10^-(n-1-j)), and the last angle
// covers the rest. The range is pi/4 + 9 sum_{j>=1} atan(10^-j) = 1.7824130..., and the angle left after n
// positions is at most atan(10^-(n-1)). This is the fixed-count form of the repeated-rotation scheme of
// the calculators (Meggitt 1962; Schmid and Bogacki 1973), which instead take each angle 0 to 9 times
// while it fits: that halves the rotations, but the gain then depends on the data, and the unit yields
// tan theta = y/x, from which sin and cos need a division and a square root.
//
// Sign-magnitude. dfixpnt stores a sign and a magnitude, so a digit shift truncates toward zero, not toward
// minus infinity as fixpnt's arithmetic shift does. The engine computes on |theta|, |x| and |y| and applies
// the signs at the end, so sin and atan2 are exactly odd and cos exactly even.
//
// The datapath. The iterations and the range reduction run in wider registers with g guard fraction digits,
// whose arithmetic is exactly that of a BCD dfixpnt with Modulo arithmetic (decimal_register: one byte per
// digit, so a rotation does not pay for dfixpnt's bitwise nibble access; the results are identical to a
// dfixpnt datapath digit for digit). Each shift truncates, and the result is rounded to nearest, ties to
// even, once, at the end. The
// result does not depend on the target's encoding (BCD, BID or DPD): the engine reads and writes digits.
// Then it follows the type's arithmetic: a result the type cannot hold (cos 0 = 1 in dfixpnt<4,4>)
// saturates under Saturate and wraps under Modulo, keeping the low ndigits digits of the magnitude.
//
//   rotation:   radix + g fraction digits, positions 0 .. radix + g - 1 by default
//   vectoring:  ndigits + 1 + g fraction digits, the inputs scaled by a power of ten so the larger lies in
//               [1/10, 1), which keeps the angle accurate for small inputs and gives hypot its relative
//               precision; positions 0 .. ndigits + g by default
//
// The default guard width is g = (the number of digits of 9 (radix + 4)) + 1: the truncation error grows
// by about one unit of the datapath per rotation, and there are 9 rotations per position.
//
// Range reduction. A type that holds only (-1, 1) needs none, and the choice is made at compile time. Any
// wider type reduces |theta| = q pi/2 + r, with |r| <= pi/4, in a type whose pi/2 carries the integer digits
// of q in addition to the datapath's fraction digits.
//
// The constants. The atan(10^-j) table, 1/K_n for every n, pi/2, pi and 2/pi are computed once per datapath
// type, with 8 digits beyond it, by series in dfixpnt arithmetic (atan by its Taylor series, pi/4 by
// Machin's formula, 1/K_n by a Newton iteration for the reciprocal square root that multiplies only), and
// then rounded to the datapath. Nothing goes through double beyond the Newton iteration's 15-digit seed,
// so the engine is correct beyond 15 digits.
#include <array>
#include <cmath>
#include <cstdint>
#include <type_traits>

namespace sw { namespace universal { namespace cordic {

// the guard width sentinel, shared with the binary engine
#ifndef SW_UNIVERSAL_CORDIC_AUTO_GUARD
#define SW_UNIVERSAL_CORDIC_AUTO_GUARD
inline constexpr unsigned auto_guard = 0xFFFF'FFFFu;
#endif

namespace decimal_detail {

	constexpr unsigned digits_of(unsigned v) noexcept {
		unsigned d = 1;
		while (v >= 10u) { v /= 10u; ++d; }
		return d;
	}

	// the shape of the datapaths for a target dfixpnt<ndigits, radix> and a guard request
	template<unsigned ndigits, unsigned radix, unsigned guard>
	struct config {
		static constexpr unsigned idigits = ndigits - radix;
		static constexpr unsigned g = (guard == auto_guard) ? digits_of(9u * (radix + 4u)) + 1u : guard;
		// rotation: |x|, |y| <= 1 and |z| <= 1.79, one integer digit, and one more of margin
		static constexpr unsigned rotR = radix + g;
		static constexpr unsigned rotN = rotR + 2u;
		static constexpr unsigned rot_iterations = rotR;
		// vectoring: inputs of ndigits + 1 digits (atan's 1 = 10^radix needs the extra one when radix ==
		// ndigits), scaled into [1/10, 1); x grows to K sqrt(2) < 3
		static constexpr unsigned vecR = ndigits + 1u + g;
		static constexpr unsigned vecN = vecR + 2u;
		static constexpr unsigned vec_iterations = vecR;
		// range reduction: |theta| < 10^idigits
		static constexpr bool reduce = (idigits >= 1u);
		static constexpr unsigned redR = rotR + idigits + 2u;
		static constexpr unsigned redN = redR + idigits + 2u;
	};

	// the reduction and generator datapath: a BCD dfixpnt with Modulo arithmetic
	template<unsigned N, unsigned R, typename bt>
	using datapath = dfixpnt<N, R, DecimalEncoding::BCD, Modulo, bt>;

	// the block type of a datapath: a dfixpnt's own, and uint8_t for a register
	template<typename W, typename = void> struct bt_of { using type = std::uint8_t; };
	template<typename W> struct bt_of<W, std::void_t<typename W::BlockType>> { using type = typename W::BlockType; };
	template<typename W> using bt_of_t = typename bt_of<W>::type;

	// a register of the CORDIC unit: a sign and a magnitude of N decimal digits, R of them fraction digits.
	// Its arithmetic is exactly that of a BCD dfixpnt<N, R> with Modulo arithmetic (a sum of magnitudes wraps
	// at N digits, a difference takes the sign of the larger, a digit shift truncates toward zero, and zero
	// is +0), with one byte per digit, so a rotation does not pay for dfixpnt's bitwise nibble access.
	template<unsigned N, unsigned R>
	struct decimal_register {
		static constexpr unsigned ndigits = N;
		static constexpr unsigned radix = R;
		std::array<std::uint8_t, N> d;
		bool neg;

		decimal_register() : d{}, neg(false) {}
		explicit decimal_register(int units) : d{}, neg(false) {          // a small nonnegative integer
			for (unsigned i = R; i < N && units > 0; ++i) { d[i] = static_cast<std::uint8_t>(units % 10); units /= 10; }
		}
		unsigned digit(unsigned i) const { return d[i]; }
		void setdigit(unsigned i, unsigned v) { d[i] = static_cast<std::uint8_t>(v); }
		bool sign() const { return neg; }
		bool isneg() const { return neg; }
		void setsign(bool s) { neg = s && !iszero(); }
		bool iszero() const {
			for (unsigned i = 0; i < N; ++i) if (d[i] != 0) return false;
			return true;
		}
		decimal_register& maxpos() { d.fill(9); neg = false; return *this; }
		decimal_register operator-() const { decimal_register r(*this); r.neg = !neg && !iszero(); return r; }

		// -1, 0, 1 as |this| <, ==, > |b|
		int compare_magnitude(const decimal_register& b) const {
			for (unsigned i = N; i-- > 0;) if (d[i] != b.d[i]) return d[i] < b.d[i] ? -1 : 1;
			return 0;
		}
		// this + (-1)^negate b, in sign-magnitude
		void accumulate(const decimal_register& b, bool negate) {
			const bool bneg = (b.neg != negate) && !b.iszero();
			if (neg == bneg) {
				unsigned carry = 0;
				for (unsigned i = 0; i < N; ++i) {
					unsigned v = d[i] + b.d[i] + carry;
					carry = (v > 9u) ? 1u : 0u;
					d[i] = static_cast<std::uint8_t>(carry ? v - 10u : v);
				}
				if (iszero()) neg = false;                           // a wrap to 0 is +0
				return;
			}
			const int c = compare_magnitude(b);
			if (c == 0) { d.fill(0); neg = false; return; }
			const std::array<std::uint8_t, N>& big = (c > 0) ? d : b.d;
			const std::array<std::uint8_t, N>& small = (c > 0) ? b.d : d;
			std::array<std::uint8_t, N> r{};
			int borrow = 0;
			for (unsigned i = 0; i < N; ++i) {
				int v = static_cast<int>(big[i]) - static_cast<int>(small[i]) - borrow;
				borrow = (v < 0) ? 1 : 0;
				r[i] = static_cast<std::uint8_t>(borrow ? v + 10 : v);
			}
			d = r;
			neg = (c > 0) ? neg : bneg;
		}
		decimal_register& operator+=(const decimal_register& b) { accumulate(b, false); return *this; }
		decimal_register& operator-=(const decimal_register& b) { accumulate(b, true); return *this; }
		friend decimal_register operator+(decimal_register a, const decimal_register& b) { a += b; return a; }
		friend decimal_register operator-(decimal_register a, const decimal_register& b) { a -= b; return a; }
		// the product, truncated to R fraction digits and wrapped to N digits, as dfixpnt's operator* does
		friend decimal_register operator*(const decimal_register& a, const decimal_register& b) {
			std::array<std::uint32_t, 2u * N> p{};
			for (unsigned i = 0; i < N; ++i) {
				if (a.d[i] == 0) continue;
				for (unsigned j = 0; j < N; ++j) p[i + j] += static_cast<std::uint32_t>(a.d[i]) * b.d[j];
			}
			for (unsigned k = 0; k + 1u < 2u * N; ++k) { p[k + 1u] += p[k] / 10u; p[k] %= 10u; }
			decimal_register r;
			for (unsigned i = 0; i < N; ++i) r.d[i] = static_cast<std::uint8_t>(p[i + R]);
			r.setsign(a.neg != b.neg);
			return r;
		}
	};

	// the generator for the constants of a datapath type: 8 more fraction digits, 4 integer digits for the
	// series' divisors
	template<typename Work>
	using generator = datapath<Work::radix + 8u + 4u, Work::radix + 8u, bt_of_t<Work>>;

	// v * 10^-j, truncated toward zero, as a digit shift does
	template<unsigned N, unsigned R>
	decimal_register<N, R> shifted_right(const decimal_register<N, R>& v, unsigned j) {
		decimal_register<N, R> r;
		if (j >= N) return r;
		for (unsigned i = 0; i + j < N; ++i) r.d[i] = v.d[i + j];
		r.setsign(v.neg);
		return r;
	}
	template<typename D>
	D shifted_right(const D& v, unsigned j) {
		D r(0);
		if (j >= D::ndigits) return r;
		for (unsigned i = 0; i + j < D::ndigits; ++i) r.setdigit(i, v.digit(i + j));
		if (!r.iszero()) r.setsign(v.sign());
		return r;
	}

	// 10^-j, for j <= D::radix
	template<typename D>
	D pow10_negative(unsigned j) {
		D r(0);
		r.setdigit(D::radix - j, 1);
		return r;
	}

	// the To whose magnitude digits are round(|v| digits * 10^-shift), to nearest, ties to even, with the
	// sign of v; a magnitude beyond To clamps to its largest when saturate, and otherwise wraps, keeping the
	// low To::ndigits digits
	template<typename To, bool saturate, typename From>
	To round_digits(const From& v, unsigned shift) {
		constexpr unsigned M = To::ndigits;
		std::array<std::uint8_t, M> d{};
		bool overflow = false;
		for (unsigned i = 0; i < M; ++i) d[i] = static_cast<std::uint8_t>((i + shift < From::ndigits) ? v.digit(i + shift) : 0u);
		for (unsigned i = M + shift; i < From::ndigits; ++i) {
			if (v.digit(i) != 0) { overflow = true; break; }
		}
		if (shift > 0 && shift <= From::ndigits) {
			const unsigned first = v.digit(shift - 1u);
			bool sticky = false;
			for (unsigned i = 0; i + 1u < shift; ++i) {
				if (v.digit(i) != 0) { sticky = true; break; }
			}
			const bool up = first > 5u || (first == 5u && (sticky || (d[0] & 1u)));
			if (up) {
				unsigned i = 0;
				for (; i < M; ++i) {
					if (d[i] < 9u) { ++d[i]; break; }
					d[i] = 0;
				}
				if (i == M) overflow = true;                    // carried out of the magnitude
			}
		}
		To t(0);
		if constexpr (saturate) {
			if (overflow) {
				t.maxpos();
				if (v.sign()) t.setsign(true);
				return t;
			}
		}
		for (unsigned i = 0; i < M; ++i) t.setdigit(i, d[i]);
		if (!t.iszero()) t.setsign(v.sign());
		return t;
	}

	// the magnitude of v placed in D with its digits shifted up by shift positions: |v| * 10^shift raw
	template<typename D, typename Src>
	D placed(const Src& v, unsigned shift) {
		D r(0);
		for (unsigned i = 0; i < Src::ndigits && i + shift < D::ndigits; ++i) r.setdigit(i + shift, v.digit(i));
		return r;
	}

	// atan(1/m) = sum_k (-1)^k / ((2k + 1) m^(2k+1)), for an integer m >= 2
	template<typename G>
	G atan_inverse(int m) {
		const G gm(m);
		G p = G(1) / gm;
		G sum = p;
		for (int k = 1; ; ++k) {
			p = p / gm;
			p = p / gm;
			if (p.iszero()) break;
			const G term = p / G(2 * k + 1);
			if (k & 1) sum -= term; else sum += term;
		}
		return sum;
	}

	// atan(10^-j) for j >= 1 = sum_k (-1)^k 10^-j(2k+1) / (2k + 1)
	template<typename G>
	G atan_pow10(unsigned j) {
		G p = pow10_negative<G>(j);
		G sum = p;
		for (int k = 1; ; ++k) {
			p = shifted_right(p, 2u * j);
			if (p.iszero()) break;
			const G term = p / G(2 * k + 1);
			if (k & 1) sum -= term; else sum += term;
		}
		return sum;
	}

	// pi/4 = 4 atan(1/5) - atan(1/239)
	template<typename G>
	G pi_over_4() {
		const G a5 = atan_inverse<G>(5);
		G four_a5 = a5 + a5;
		four_a5 = four_a5 + four_a5;
		return four_a5 - atan_inverse<G>(239);
	}

	// 1/sqrt(P) for P in [1, 10), by the Newton iteration y <- y (3 - P y^2) / 2, which multiplies only; the
	// seed comes from double, and each step doubles its 15 correct digits
	template<typename G>
	G inverse_sqrt(const G& P) {
		G y(1.0 / std::sqrt(double(P)));                       // a seed: its digits past the 15th are noise
		const G three(3);
		G half(0);
		half.setdigit(G::radix - 1u, 5);                        // exact: G(0.5) would go through double
		unsigned steps = 3;
		for (unsigned d = 15; d < G::radix; d *= 2u) ++steps;
		for (unsigned s = 0; s < steps; ++s) y = y * (three - P * y * y) * half;
		return y;
	}

	// the iteration tables of a datapath type: atan(10^-j) for j < R, 1/K_n for n <= R, pi/2 and pi
	template<typename Work>
	struct tables {
		static constexpr unsigned R = Work::radix;
		std::array<Work, R> atan;
		std::array<Work, R + 1u> invK;
		Work pi_over_2;
		Work pi;

		tables() : atan{}, invK{}, pi_over_2(0), pi(0) {
			using G = generator<Work>;
			const G quarter_pi = pi_over_4<G>();
			atan[0] = round_digits<Work, false>(quarter_pi, 8u);
			for (unsigned j = 1; j < R; ++j) atan[j] = round_digits<Work, false>(atan_pow10<G>(j), 8u);
			const G half_pi = quarter_pi + quarter_pi;
			pi_over_2 = round_digits<Work, false>(half_pi, 8u);
			pi = round_digits<Work, false>(half_pi + half_pi, 8u);
			// P_n = K_n^2 = 2 prod_{1 <= j < n} (1 + 10^-2j)^9, by shift and add; 1/K_n = 1/sqrt(P_n)
			invK[0] = Work(1);
			G P(2);
			invK[1] = round_digits<Work, false>(inverse_sqrt(P), 8u);
			for (unsigned n = 2; n <= R; ++n) {
				for (unsigned r = 0; r < 9u; ++r) P += shifted_right(P, 2u * (n - 1u));
				invK[n] = round_digits<Work, false>(inverse_sqrt(P), 8u);
			}
		}
	};
	template<typename Work>
	const tables<Work>& tables_of() {
		static const tables<Work> t;
		return t;
	}

	// the reduction constants pi/2 and 2/pi, in the reduction type
	template<typename Red>
	struct reduction {
		Red pi_over_2;
		Red two_over_pi;
		reduction() : reduction(pi_over_4<generator<Red>>()) {}
		explicit reduction(const generator<Red>& quarter_pi)
			: pi_over_2(round_digits<Red, false>(quarter_pi + quarter_pi, 8u)),
			  two_over_pi(round_digits<Red, false>(generator<Red>(1) / (quarter_pi + quarter_pi), 8u)) {}
	};
	template<typename Red>
	const reduction<Red>& reduction_of() {
		static const reduction<Red> t;
		return t;
	}

	// one rotation by +-atan(10^-j): x <- x - d (y >> j), y <- y + d (x >> j), z <- z - d alpha
	template<typename Work>
	void step(Work& x, Work& y, Work& z, unsigned j, const Work& alpha, bool up) {
		const Work xs = shifted_right(x, j), ys = shifted_right(y, j);
		if (up) { x -= ys; y += xs; z -= alpha; }
		else    { x += ys; y -= xs; z += alpha; }
	}

	// the number of rotations at digit position j
	constexpr unsigned rotations_at(unsigned j) noexcept { return (j == 0u) ? 1u : 9u; }

	// rotation mode: drive z to 0, d = sign(z)
	template<typename Work>
	void rotate(Work& x, Work& y, Work& z, unsigned n, const tables<Work>& T) {
		for (unsigned j = 0; j < n; ++j) {
			for (unsigned r = 0; r < rotations_at(j); ++r) step(x, y, z, j, T.atan[j], !z.isneg());
		}
	}

	// vectoring mode: drive y to 0, d = -sign(y)
	template<typename Work>
	void vector(Work& x, Work& y, Work& z, unsigned n, const tables<Work>& T) {
		for (unsigned j = 0; j < n; ++j) {
			for (unsigned r = 0; r < rotations_at(j); ++r) step(x, y, z, j, T.atan[j], y.isneg());
		}
	}

	constexpr unsigned clamp_iterations(unsigned n, unsigned maxN) noexcept { return n < maxN ? n : maxN; }

	// a magnitude of up to M digits, read from a target value
	template<unsigned M>
	struct digits {
		std::array<std::uint8_t, M> d{};
		static constexpr unsigned ndigits = M;
		unsigned digit(unsigned i) const { return d[i]; }
		bool iszero() const {
			for (unsigned i = 0; i < M; ++i) if (d[i] != 0) return false;
			return true;
		}
		int msd() const {                                    // the most significant nonzero digit, or -1
			for (unsigned i = M; i-- > 0;) if (d[i] != 0) return static_cast<int>(i);
			return -1;
		}
	};
	template<unsigned M, typename Fixed>
	digits<M> magnitude_of(const Fixed& f) {
		digits<M> m;
		for (unsigned i = 0; i < Fixed::ndigits && i < M; ++i) m.d[i] = static_cast<std::uint8_t>(f.digit(i));
		return m;
	}

	// the result of the vectoring datapath on two magnitudes: x_n = K_n hypot, z_n = atan(y / x) in
	// [0, pi/2], and the scale: the inputs' digits were shifted up by shift positions
	template<typename Vec>
	struct vectored {
		Vec x;
		Vec z;
		unsigned shift;
		bool zero;
	};
	template<typename Vec, unsigned M>
	vectored<Vec> vector_magnitudes(const digits<M>& ax, const digits<M>& ay, unsigned n) {
		vectored<Vec> v{ Vec(0), Vec(0), 0u, false };
		const int p = (ax.msd() > ay.msd()) ? ax.msd() : ay.msd();
		if (p < 0) { v.zero = true; return v; }
		v.shift = (Vec::radix - 1u) - static_cast<unsigned>(p);   // the larger lies in [1/10, 1)
		Vec x = placed<Vec>(ax, v.shift), y = placed<Vec>(ay, v.shift), z(0);
		vector(x, y, z, n, tables_of<Vec>());
		v.x = x;
		v.z = z;
		return v;
	}

}  // namespace decimal_detail

// the default iteration counts, in digit positions, and the guard width of a configuration
template<typename Fixed, unsigned guard = auto_guard>
inline constexpr unsigned decimal_rotation_iterations = decimal_detail::config<Fixed::ndigits, Fixed::radix, guard>::rot_iterations;
template<typename Fixed, unsigned guard = auto_guard>
inline constexpr unsigned decimal_vectoring_iterations = decimal_detail::config<Fixed::ndigits, Fixed::radix, guard>::vec_iterations;
template<typename Fixed, unsigned guard = auto_guard>
inline constexpr unsigned decimal_guard_digits = decimal_detail::config<Fixed::ndigits, Fixed::radix, guard>::g;

// rotation mode: s = sin(theta) and c = cos(theta)
template<unsigned guard = auto_guard, unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
void sincos(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& theta,
            dfixpnt<ndigits, radix, encoding, arithmetic, bt>& s, dfixpnt<ndigits, radix, encoding, arithmetic, bt>& c,
            unsigned iterations = decimal_detail::config<ndigits, radix, guard>::rot_iterations) {
	using Cfg    = decimal_detail::config<ndigits, radix, guard>;
	using Target = dfixpnt<ndigits, radix, encoding, arithmetic, bt>;
	using Rot    = decimal_detail::decimal_register<Cfg::rotN, Cfg::rotR>;
	constexpr bool saturate = (arithmetic == Saturate);
	const auto& T = decimal_detail::tables_of<Rot>();
	const unsigned n = decimal_detail::clamp_iterations(iterations, Cfg::rotR);
	const bool negative = theta.isneg();

	Rot r(0);
	unsigned quadrant = 0;
	if constexpr (!Cfg::reduce) {
		r = decimal_detail::placed<Rot>(theta, Cfg::g);       // |theta| < 1: inside the convergence range
	}
	else {
		// |theta| = q pi/2 + r, with q = round(|theta| 2/pi)
		using Red = decimal_detail::decimal_register<Cfg::redN, Cfg::redR>;
		const auto& C = decimal_detail::reduction_of<Red>();
		const Red xr = decimal_detail::placed<Red>(theta, Cfg::redR - radix);
		const Red t = xr * C.two_over_pi;
		Red q(0);
		for (unsigned i = Cfg::redR; i < Red::ndigits; ++i) q.setdigit(i, t.digit(i));
		if (t.digit(Cfg::redR - 1u) >= 5u) q += Red(1);
		quadrant = (10u * q.digit(Cfg::redR + 1u) + q.digit(Cfg::redR)) % 4u;
		const Red rr = xr - q * C.pi_over_2;
		r = decimal_detail::round_digits<Rot, false>(rr, Cfg::redR - Cfg::rotR);   // |r| <= pi/4
	}

	Rot x = T.invK[n], y(0), z = r;
	decimal_detail::rotate(x, y, z, n, T);

	Rot S(0), Co(0);
	switch (quadrant) {
	case 0:  S = y;  Co = x;  break;
	case 1:  S = x;  Co = -y; break;
	case 2:  S = -y; Co = -x; break;
	default: S = -x; Co = y;  break;
	}
	if (negative) S = -S;                                   // sin is odd, cos is even
	s = decimal_detail::round_digits<Target, saturate>(S, Cfg::g);
	c = decimal_detail::round_digits<Target, saturate>(Co, Cfg::g);
}

template<unsigned guard = auto_guard, unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt> sin(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& theta,
                                                      unsigned iterations = decimal_detail::config<ndigits, radix, guard>::rot_iterations) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> s, c;
	sincos<guard>(theta, s, c, iterations);
	return s;
}

template<unsigned guard = auto_guard, unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt> cos(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& theta,
                                                      unsigned iterations = decimal_detail::config<ndigits, radix, guard>::rot_iterations) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> s, c;
	sincos<guard>(theta, s, c, iterations);
	return c;
}

// vectoring mode: r = hypot(x, y) and theta = atan2(y, x), in (-pi, pi]; (0, 0) gives r = theta = 0. The
// datapath vectors |x| and |y| in the first quadrant, then theta = pi - theta for a negative x, and -theta
// for a negative y.
template<unsigned guard = auto_guard, unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
void polar(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& x, const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& y,
           dfixpnt<ndigits, radix, encoding, arithmetic, bt>& r, dfixpnt<ndigits, radix, encoding, arithmetic, bt>& theta,
           unsigned iterations = decimal_detail::config<ndigits, radix, guard>::vec_iterations) {
	using Cfg    = decimal_detail::config<ndigits, radix, guard>;
	using Target = dfixpnt<ndigits, radix, encoding, arithmetic, bt>;
	using Vec    = decimal_detail::decimal_register<Cfg::vecN, Cfg::vecR>;
	constexpr bool saturate = (arithmetic == Saturate);
	const auto& T = decimal_detail::tables_of<Vec>();
	const unsigned n = decimal_detail::clamp_iterations(iterations, Cfg::vecR);
	const auto v = decimal_detail::vector_magnitudes<Vec>(decimal_detail::magnitude_of<ndigits + 1u>(x),
	                                                      decimal_detail::magnitude_of<ndigits + 1u>(y), n);
	if (v.zero) { r = Target(0); theta = Target(0); return; }
	Vec t = v.z;
	if (x.isneg()) t = T.pi - t;
	if (y.isneg()) t = -t;
	const Vec h = v.x * T.invK[n];
	r     = decimal_detail::round_digits<Target, saturate>(h, v.shift);
	theta = decimal_detail::round_digits<Target, saturate>(t, Cfg::vecR - radix);
}

template<unsigned guard = auto_guard, unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt> atan2(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& y,
                                                        const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& x,
                                                        unsigned iterations = decimal_detail::config<ndigits, radix, guard>::vec_iterations) {
	using Cfg    = decimal_detail::config<ndigits, radix, guard>;
	using Target = dfixpnt<ndigits, radix, encoding, arithmetic, bt>;
	using Vec    = decimal_detail::decimal_register<Cfg::vecN, Cfg::vecR>;
	const unsigned n = decimal_detail::clamp_iterations(iterations, Cfg::vecR);
	const auto v = decimal_detail::vector_magnitudes<Vec>(decimal_detail::magnitude_of<ndigits + 1u>(x),
	                                                      decimal_detail::magnitude_of<ndigits + 1u>(y), n);
	if (v.zero) return Target(0);
	Vec t = v.z;
	if (x.isneg()) t = decimal_detail::tables_of<Vec>().pi - t;
	if (y.isneg()) t = -t;
	return decimal_detail::round_digits<Target, arithmetic == Saturate>(t, Cfg::vecR - radix);
}

// atan(x) = atan2(x, 1), with the 1 held in the datapath, so it works where the type cannot hold 1
template<unsigned guard = auto_guard, unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt> atan(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& x,
                                                       unsigned iterations = decimal_detail::config<ndigits, radix, guard>::vec_iterations) {
	using Cfg    = decimal_detail::config<ndigits, radix, guard>;
	using Target = dfixpnt<ndigits, radix, encoding, arithmetic, bt>;
	using Vec    = decimal_detail::decimal_register<Cfg::vecN, Cfg::vecR>;
	const unsigned n = decimal_detail::clamp_iterations(iterations, Cfg::vecR);
	decimal_detail::digits<ndigits + 1u> one;
	one.d[radix] = 1;
	const auto v = decimal_detail::vector_magnitudes<Vec>(one, decimal_detail::magnitude_of<ndigits + 1u>(x), n);
	Vec t = v.z;
	if (x.isneg()) t = -t;
	return decimal_detail::round_digits<Target, arithmetic == Saturate>(t, Cfg::vecR - radix);
}

template<unsigned guard = auto_guard, unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt> hypot(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& x,
                                                        const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& y,
                                                        unsigned iterations = decimal_detail::config<ndigits, radix, guard>::vec_iterations) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> r, theta;
	polar<guard>(x, y, r, theta, iterations);
	return r;
}

}}} // namespace sw::universal::cordic
