#pragma once
// cordic.hpp: a circular CORDIC engine for fixed-points, for hardware codesign (#2015)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The default fixpnt sin, cos, atan, atan2 and hypot (trigonometry.hpp, hypot.hpp) compute through double.
// They serve software experiments, and they carry at most 53 correct bits. This engine is separate, with
// explicit functions in namespace cordic: it models a shift-and-add CORDIC unit bit for bit, in the type's
// own arithmetic, to any width, with a chosen iteration count and guard width.
//
//   cordic::sincos(theta, s, c)            rotation mode: s = sin(theta), c = cos(theta)
//   cordic::sin(theta), cordic::cos(theta)
//   cordic::atan2(y, x), cordic::atan(x)   vectoring mode
//   cordic::hypot(x, y)
//   cordic::polar(x, y, r, theta)          r = hypot(x, y), theta = atan2(y, x)
//
// Every function takes an optional iteration count as its last argument, and the guard width as an
// optional first template argument:
//
//   cordic::sincos<4>(theta, s, c, 12);     4 guard bits, 12 iterations
//
// The algorithm. Iterate i = 0 .. n-1:
//
//   x_{i+1} = x_i - d_i * (y_i >> i)
//   y_{i+1} = y_i + d_i * (x_i >> i)
//   z_{i+1} = z_i - d_i * atan(2^-i)
//
// with d_i = sign(z_i) in rotation mode (z -> 0) and d_i = -sign(y_i) in vectoring mode (y -> 0).
// Rotation starts at (1/K_n, 0, theta) and ends at (cos theta, sin theta). Vectoring starts at (x, y, 0)
// and ends at (K_n * hypot(x, y), 0, atan2(y, x)). K_n = prod_{i<n} sqrt(1 + 2^-2i) -> 1.6467602581...
//
// The datapath. The iterations run in a wider fixpnt with g guard fraction bits, Modulo arithmetic and
// enough integer bits that nothing wraps; each shift truncates (an arithmetic shift, as in hardware), and
// the result is rounded to nearest once, at the end. Then it follows the type's arithmetic: a result the
// type cannot hold (cos 0 = 1 in fixpnt<8,7>, atan2(0, -1) = pi in fixpnt<8,6>) saturates under Saturate
// and wraps under Modulo, as a conversion from double does.
//
//   rotation:   sign, 2 integer bits, rbits + g fraction bits
//   vectoring:  sign, 2 integer bits, nbits + 1 + g fraction bits; the inputs are scaled by a power of two
//               so the larger magnitude lies in [1/2, 1), which keeps the angle accurate for small inputs
//
// The default guard width is g = ceil(log2(rbits + 8)) + 2, and the default iteration count is rbits + g.
// The truncation error grows by about one unit of the datapath per iteration, and the g guard bits absorb
// it; the angle error after n iterations is at most atan(2^-(n-1)) ~= 2^-(n-1).
//
// Range reduction. Rotation converges for |theta| <= sum_i atan(2^-i) = 1.7432866... A fixpnt that holds
// only [-1, 1) or less needs no reduction, and the choice is made at compile time. Any wider type reduces
// theta = q * pi/2 + r with |r| <= pi/4, in a type whose pi/2 carries the integer bits of q in addition to
// the datapath's fraction bits, so r is accurate to the datapath whatever the size of theta.
//
// The constants. The atan(2^-i) table, 1/K_n for every n, pi/2 and 2/pi are computed once per datapath
// type, with 16 bits beyond it, by series in the fixpnt arithmetic itself (atan by its Taylor series,
// pi/4 by Machin's formula, 1/K_n = 1/sqrt(prod (1 + 4^-i)) with the correctly rounded fixpnt sqrt), and
// then rounded to the datapath. Nothing goes through double, so the engine is correct beyond 52 bits.
#include <array>
#include <cstdint>

namespace sw { namespace universal { namespace cordic {

// the guard width sentinel: pick g = ceil(log2(rbits + 8)) + 2
inline constexpr unsigned auto_guard = 0xFFFF'FFFFu;

namespace detail {

	constexpr unsigned clog2(unsigned v) noexcept {
		unsigned r = 0;
		while ((1ull << r) < v) ++r;
		return r;
	}

	// the shape of the datapaths for a target fixpnt<nbits, rbits> and a guard request
	template<unsigned nbits, unsigned rbits, unsigned guard>
	struct config {
		static_assert(nbits >= 2, "cordic: the target fixpnt needs at least 2 bits");
		static_assert(rbits <= nbits, "cordic: rbits cannot exceed nbits");
		static constexpr unsigned g = (guard == auto_guard) ? clog2(rbits + 8u) + 2u : guard;
		static constexpr unsigned ib = nbits - rbits;                // integer bits, the sign included
		static constexpr unsigned iterations = rbits + g;            // the default iteration count

		// rotation datapath
		static constexpr unsigned rotR = rbits + g;
		static constexpr unsigned rotN = rotR + 3u;

		// vectoring datapath: one bit more than the widest input, 1 = 2^nbits in atan(x) for rbits == nbits
		static constexpr unsigned vecR = nbits + 1u + g;
		static constexpr unsigned vecN = vecR + 3u;

		// range reduction: |theta| < 2^M; needed when 2^M exceeds the convergence range 1.74...
		static constexpr bool reduce = (ib >= 2u);
		static constexpr unsigned M = reduce ? ib - 1u : 0u;
		static constexpr unsigned redR = rotR + M + 2u;
		static constexpr unsigned redN = redR + M + 3u;
	};

	// the generator: the datapath's fraction bits plus 16, with integer bits for the series' divisors
	template<typename Work> struct generator;
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	struct generator<fixpnt<nbits, rbits, arithmetic, bt>> {
		using type = fixpnt<rbits + 16u + 20u, rbits + 16u, Modulo, bt>;
	};

	// atan(1/m) = sum_k (-1)^k / ((2k + 1) m^(2k+1)), for an integer m >= 2
	template<typename G>
	G atan_inverse(unsigned m) {
		G p = G(1) / G(static_cast<int>(m));
		const G m2(static_cast<int>(m * m));
		G sum = p;
		for (unsigned k = 1; ; ++k) {
			p = p / m2;
			if (p.iszero()) break;
			const G term = p / G(static_cast<int>(2u * k + 1u));
			if (k & 1u) sum -= term; else sum += term;
		}
		return sum;
	}

	// atan(2^-i) for i >= 1 = sum_k (-1)^k 2^-i(2k+1) / (2k + 1)
	template<typename G>
	G atan_pow2(unsigned i) {
		G p = G(1) >> static_cast<int>(i);
		G sum = p;
		for (unsigned k = 1; ; ++k) {
			p >>= static_cast<int>(2u * i);
			if (p.iszero()) break;
			const G term = p / G(static_cast<int>(2u * k + 1u));
			if (k & 1u) sum -= term; else sum += term;
		}
		return sum;
	}

	// pi/4 = 4 atan(1/5) - atan(1/239)
	template<typename G>
	G pi_over_4() {
		G four_atan5 = atan_inverse<G>(5u);
		four_atan5 <<= 2;
		return four_atan5 - atan_inverse<G>(239u);
	}

	// the iteration tables of a datapath type: atan(2^-i) for i < R, and 1/K_n for n <= R
	template<typename Work>
	struct tables {
		static constexpr unsigned R = Work::rbits;
		std::array<Work, R> atan{};
		std::array<Work, R + 1u> invK{};
		Work pi_over_2{};

		tables() {
			using G = typename generator<Work>::type;
			const G quarter_pi = pi_over_4<G>();
			atan[0] = quarter_pi;
			for (unsigned i = 1; i < R; ++i) atan[i] = atan_pow2<G>(i);
			G half_pi = quarter_pi;
			half_pi <<= 1;
			pi_over_2 = half_pi;
			// P_n = prod_{i<n} (1 + 4^-i), by shift and add; 1/K_n = 1/sqrt(P_n)
			G P(1);
			invK[0] = Work(1);
			for (unsigned n = 1; n <= R; ++n) {
				P += (P >> static_cast<int>(2u * (n - 1u)));
				invK[n] = G(1) / sqrt(P);
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
		Red pi_over_2{};
		Red two_over_pi{};
		reduction() {
			using G = typename generator<Red>::type;
			G half_pi = pi_over_4<G>();
			half_pi <<= 1;
			pi_over_2 = half_pi;
			two_over_pi = G(1) / half_pi;
		}
	};
	template<typename Red>
	const reduction<Red>& reduction_of() {
		static const reduction<Red> t;
		return t;
	}

	// rotation mode: drive z to 0
	template<typename Work, std::size_t N>
	void rotate(Work& x, Work& y, Work& z, unsigned n, const std::array<Work, N>& A) {
		for (unsigned i = 0; i < n; ++i) {
			const Work xs = x >> static_cast<int>(i);
			const Work ys = y >> static_cast<int>(i);
			if (!z.isneg()) { x -= ys; y += xs; z -= A[i]; }
			else            { x += ys; y -= xs; z += A[i]; }
		}
	}

	// vectoring mode: drive y to 0
	template<typename Work, std::size_t N>
	void vector(Work& x, Work& y, Work& z, unsigned n, const std::array<Work, N>& A) {
		for (unsigned i = 0; i < n; ++i) {
			const Work xs = x >> static_cast<int>(i);
			const Work ys = y >> static_cast<int>(i);
			if (y.isneg()) { x -= ys; y += xs; z -= A[i]; }
			else           { x += ys; y -= xs; z += A[i]; }
		}
	}

	// round raw * 2^-shift to nearest, ties to even, in place
	template<unsigned W, typename bt>
	void round_shift(blockbinary<W, bt, BinaryNumberType::Signed>& raw, unsigned shift) {
		if (shift == 0) return;
		const bool up = raw.roundingMode(shift);
		raw >>= static_cast<int>(shift);
		if (up) ++raw;
	}

	// the Target whose raw value is round(raw * 2^-shift), saturated or wrapped by Target's arithmetic
	template<typename Target, unsigned W, typename bt>
	Target fit(const blockbinary<W, bt, BinaryNumberType::Signed>& rawIn, unsigned shift) {
		constexpr unsigned tbits = Target::nbits;
		constexpr unsigned WW = (W > tbits ? W : tbits) + 1u;
		using Wide = blockbinary<WW, bt, BinaryNumberType::Signed>;
		Wide raw(rawIn);                                    // sign-extends
		round_shift(raw, shift);
		Target t;
		if constexpr (Target::arithmetic == Saturate) {
			const Wide maxp(Target(SpecificValue::maxpos).bits());
			const Wide maxn(Target(SpecificValue::maxneg).bits());
			if (maxp < raw) return t.maxpos();
			if (raw < maxn) return t.maxneg();
		}
		t.clear();
		for (unsigned i = 0; i < tbits; ++i) t.setbit(i, raw.at(i));   // Modulo: keep the low nbits
		return t;
	}

	// the result of the vectoring datapath: x_n, z_n and the normalization shift
	template<typename Vec>
	struct vectored {
		Vec x{};
		Vec z{};
		unsigned shift{ 0 };
		bool zero{ false };
	};

	// vectoring on raw target values, given in nbits + 2 bits so atan(x) can pass 1 = 2^rbits, even
	// when rbits == nbits and 1 = 2^nbits
	template<typename Vec, unsigned IN, typename bt>
	vectored<Vec> vector_raw(const blockbinary<IN, bt, BinaryNumberType::Signed>& rx,
	                         const blockbinary<IN, bt, BinaryNumberType::Signed>& ry, unsigned n) {
		constexpr unsigned R = Vec::rbits;
		vectored<Vec> v;
		if (rx.iszero() && ry.iszero()) { v.zero = true; return v; }
		// scale both by 2^s so the larger magnitude lies in [1/2, 1) of the datapath
		blockbinary<IN, bt, BinaryNumberType::Signed> ax(rx), ay(ry);
		if (ax.isneg()) ax.twosComplement();
		if (ay.isneg()) ay.twosComplement();
		unsigned msb = 0;
		for (unsigned i = IN; i-- > 0;) {
			if (ax.at(i) || ay.at(i)) { msb = i; break; }
		}
		v.shift = (R - 1u) - msb;
		Vec x, y;
		x = rx;
		y = ry;
		x <<= static_cast<int>(v.shift);
		y <<= static_cast<int>(v.shift);
		// rotate into the right half-plane by -+pi/2
		const auto& T = tables_of<Vec>();
		Vec z(0);
		if (x.isneg()) {
			const Vec ox = x;
			if (!y.isneg()) { x = y;  y = -ox; z = T.pi_over_2; }
			else            { x = -y; y = ox;  z = -T.pi_over_2; }
		}
		vector(x, y, z, n, T.atan);
		v.x = x;
		v.z = z;
		return v;
	}

	template<typename Fixed>
	blockbinary<Fixed::nbits + 2u, typename Fixed::BlockType, BinaryNumberType::Signed> widen(const Fixed& f) {
		return blockbinary<Fixed::nbits + 2u, typename Fixed::BlockType, BinaryNumberType::Signed>(f.bits());
	}

	constexpr unsigned clamp_iterations(unsigned n, unsigned maxN) noexcept { return n < maxN ? n : maxN; }

}  // namespace detail

// the default iteration count and the guard width of a configuration
template<typename Fixed, unsigned guard = auto_guard>
inline constexpr unsigned default_iterations = detail::config<Fixed::nbits, Fixed::rbits, guard>::iterations;
template<typename Fixed, unsigned guard = auto_guard>
inline constexpr unsigned guard_bits = detail::config<Fixed::nbits, Fixed::rbits, guard>::g;

// rotation mode: s = sin(theta) and c = cos(theta)
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
void sincos(const fixpnt<nbits, rbits, arithmetic, bt>& theta,
            fixpnt<nbits, rbits, arithmetic, bt>& s, fixpnt<nbits, rbits, arithmetic, bt>& c,
            unsigned iterations = detail::config<nbits, rbits, guard>::iterations) {
	using Cfg    = detail::config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using Rot    = fixpnt<Cfg::rotN, Cfg::rotR, Modulo, bt>;
	const auto& T = detail::tables_of<Rot>();
	const unsigned n = detail::clamp_iterations(iterations, Cfg::rotR);

	Rot r;
	unsigned quadrant = 0;
	if constexpr (!Cfg::reduce) {
		r = theta.bits();                                   // |theta| <= 1: inside the convergence range
		r <<= static_cast<int>(Cfg::rotR - rbits);
	}
	else {
		// theta = q pi/2 + r, with q = round(theta 2/pi)
		using Red = fixpnt<Cfg::redN, Cfg::redR, Modulo, bt>;
		using RedBits = blockbinary<Cfg::redN, bt, BinaryNumberType::Signed>;
		const auto& C = detail::reduction_of<Red>();
		Red xr;
		xr = theta.bits();
		xr <<= static_cast<int>(Cfg::redR - rbits);
		RedBits q = (xr * C.two_over_pi).bits();
		detail::round_shift(q, Cfg::redR);
		quadrant = (q.at(0) ? 1u : 0u) | (q.at(1) ? 2u : 0u);
		Red qf;
		qf = q;
		qf <<= static_cast<int>(Cfg::redR);
		RedBits rr = (xr - qf * C.pi_over_2).bits();
		detail::round_shift(rr, Cfg::redR - Cfg::rotR);
		r = rr;                                             // |r| <= pi/4: the low bits hold it
	}

	Rot x = T.invK[n], y(0), z = r;
	detail::rotate(x, y, z, n, T.atan);

	Rot S, Co;
	switch (quadrant) {
	case 0:  S = y;  Co = x;  break;
	case 1:  S = x;  Co = -y; break;
	case 2:  S = -y; Co = -x; break;
	default: S = -x; Co = y;  break;
	}
	s = detail::fit<Target>(S.bits(), Cfg::rotR - rbits);
	c = detail::fit<Target>(Co.bits(), Cfg::rotR - rbits);
}

template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> sin(const fixpnt<nbits, rbits, arithmetic, bt>& theta,
                                         unsigned iterations = detail::config<nbits, rbits, guard>::iterations) {
	fixpnt<nbits, rbits, arithmetic, bt> s, c;
	sincos<guard>(theta, s, c, iterations);
	return s;
}

template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> cos(const fixpnt<nbits, rbits, arithmetic, bt>& theta,
                                         unsigned iterations = detail::config<nbits, rbits, guard>::iterations) {
	fixpnt<nbits, rbits, arithmetic, bt> s, c;
	sincos<guard>(theta, s, c, iterations);
	return c;
}

// vectoring mode: r = hypot(x, y) and theta = atan2(y, x), in (-pi, pi]; (0, 0) gives r = theta = 0
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
void polar(const fixpnt<nbits, rbits, arithmetic, bt>& x, const fixpnt<nbits, rbits, arithmetic, bt>& y,
           fixpnt<nbits, rbits, arithmetic, bt>& r, fixpnt<nbits, rbits, arithmetic, bt>& theta,
           unsigned iterations = detail::config<nbits, rbits, guard>::iterations) {
	using Cfg    = detail::config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using Vec    = fixpnt<Cfg::vecN, Cfg::vecR, Modulo, bt>;
	const unsigned n = detail::clamp_iterations(iterations, Cfg::vecR);
	const auto v = detail::vector_raw<Vec>(detail::widen(x), detail::widen(y), n);
	if (v.zero) { r = Target(0); theta = Target(0); return; }
	const Vec h = v.x * detail::tables_of<Vec>().invK[n];
	r     = detail::fit<Target>(h.bits(), v.shift);
	theta = detail::fit<Target>(v.z.bits(), Cfg::vecR - rbits);
}

template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> atan2(const fixpnt<nbits, rbits, arithmetic, bt>& y, const fixpnt<nbits, rbits, arithmetic, bt>& x,
                                           unsigned iterations = detail::config<nbits, rbits, guard>::iterations) {
	using Cfg = detail::config<nbits, rbits, guard>;
	using Vec = fixpnt<Cfg::vecN, Cfg::vecR, Modulo, bt>;
	const unsigned n = detail::clamp_iterations(iterations, Cfg::vecR);
	const auto v = detail::vector_raw<Vec>(detail::widen(x), detail::widen(y), n);
	if (v.zero) return fixpnt<nbits, rbits, arithmetic, bt>(0);
	return detail::fit<fixpnt<nbits, rbits, arithmetic, bt>>(v.z.bits(), Cfg::vecR - rbits);
}

// atan(x) = atan2(x, 1), with the 1 held in the datapath, so it works where the type cannot hold 1
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> atan(const fixpnt<nbits, rbits, arithmetic, bt>& x,
                                          unsigned iterations = detail::config<nbits, rbits, guard>::iterations) {
	using Cfg = detail::config<nbits, rbits, guard>;
	using Vec = fixpnt<Cfg::vecN, Cfg::vecR, Modulo, bt>;
	const unsigned n = detail::clamp_iterations(iterations, Cfg::vecR);
	blockbinary<nbits + 2u, bt, BinaryNumberType::Signed> one;
	one.clear();
	one.setbit(rbits);
	const auto v = detail::vector_raw<Vec>(one, detail::widen(x), n);
	return detail::fit<fixpnt<nbits, rbits, arithmetic, bt>>(v.z.bits(), Cfg::vecR - rbits);
}

template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> hypot(const fixpnt<nbits, rbits, arithmetic, bt>& x, const fixpnt<nbits, rbits, arithmetic, bt>& y,
                                           unsigned iterations = detail::config<nbits, rbits, guard>::iterations) {
	fixpnt<nbits, rbits, arithmetic, bt> r, theta;
	polar<guard>(x, y, r, theta, iterations);
	return r;
}

}}} // namespace sw::universal::cordic
