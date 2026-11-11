#pragma once
// cordic.hpp: a CORDIC engine for fixed-points, for hardware codesign (#2015, #2040, #2041)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The default fixpnt sin, cos, tan, atan, atan2, hypot, exp, log, sqrt and the hyperbolic functions compute
// through double (or, for sqrt, by an integer root). They serve software experiments, and the double ones
// carry at most 53 correct bits. This engine is separate, with
// explicit functions in namespace cordic: it models a shift-and-add CORDIC unit bit for bit, in the type's
// own arithmetic, to any width, with a chosen iteration count and guard width.
//
//   cordic::sincos(theta, s, c)            rotation mode: s = sin(theta), c = cos(theta)
//   cordic::sin(theta), cordic::cos(theta)
//   cordic::atan2(y, x), cordic::atan(x)   vectoring mode
//   cordic::hypot(x, y)
//   cordic::polar(x, y, r, theta)          r = hypot(x, y), theta = atan2(y, x)
//   cordic::tan(theta)                     circular rotation, then linear vectoring: sin / cos
//   cordic::mul(x, z)                      linear rotation mode: x * z
//   cordic::div(y, x)                      linear vectoring mode: y / x
//   cordic::exp(z)                         hyperbolic rotation mode: cosh + sinh
//   cordic::sinhcosh(z, s, c)              s = sinh(z), c = cosh(z)
//   cordic::sinh(z), cordic::cosh(z)
//   cordic::tanh(z)                        hyperbolic rotation, then linear vectoring: sinh / cosh
//   cordic::atanh(t), cordic::log(w)       hyperbolic vectoring mode
//   cordic::sqrt(w)
//
// Every function takes an optional iteration count as its last argument, and the guard width as an
// optional first template argument:
//
//   cordic::sincos<4>(theta, s, c, 12);     4 guard bits, 12 iterations
//
// The algorithm (Volder 1959; unified by Walther 1971). Iterate i = 0 .. n-1:
//
//   x_{i+1} = x_i - m * d_i * (y_i >> i)
//   y_{i+1} = y_i + d_i * (x_i >> i)
//   z_{i+1} = z_i - d_i * alpha_i
//
// with d_i = sign(z_i) in rotation mode (z -> 0) and d_i = -sign(y_i) in vectoring mode (y -> 0). The
// coordinate system m picks the geometry, and with it the elementary angles alpha_i and the gain:
//
//   m =  1  circular    alpha_i = atan(2^-i)   preserves x^2 + y^2 (up to K)   sin, cos, atan2, hypot
//   m =  0  linear      alpha_i = 2^-i         preserves x                     x * z, y / x
//   m = -1  hyperbolic  alpha_i = atanh(2^-i)  preserves x^2 - y^2 (up to K)   exp, sinh, cosh, atanh, log, sqrt
//
// detail::step is that one iteration, written once for all three; docs/algorithmic-details/cordic.md
// tells the history and derives the three from one rotation.
//
// Circular rotation starts at (1/K_n, 0, theta) and ends at (cos theta, sin theta). Circular vectoring
// starts at (x, y, 0) and ends at (K_n * hypot(x, y), 0, atan2(y, x)), where
// K_n = prod_{i<n} sqrt(1 + 2^-2i) -> 1.6467602581... Linear rotation starts at (x, 0, z) and ends at
// (x, x * z, 0); linear vectoring starts at (x, y, 0) and ends at (x, 0, y / x). The linear gain is 1.
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
// The constants. The atan(2^-i) and atanh(2^-i) tables, 1/K_n for every n, pi/2, 2/pi, ln 2 and 1/ln 2 are
// computed once per datapath type, with 16 bits beyond it, by series in the fixpnt arithmetic itself (atan
// and atanh by their Taylor series, pi/4 by Machin's formula, ln 2 = 2 atanh(1/3), 1/K_n = 1/sqrt(P_n) with
// the correctly rounded fixpnt sqrt), and then rounded to the datapath. Nothing goes through double, so the engine is correct beyond 52 bits.
//
// Linear mode. mul and div scale both operands by powers of two into [1/2, 1), as a floating-point unit
// aligns significands, so the error is relative to the operands' size and not to the datapath's: linear
// CORDIC converges for |z| < 2 (rotation) and |y / x| < 2 (vectoring), and the normalized operands lie
// inside. The datapath has B + g fraction bits, where B is the number of bits the result can need:
//
//   mul, Saturate:  B = nbits + 1        a larger product saturates, and is detected from the exponents
//   mul, Modulo:    B = nbits + ib       every bit of a wrapped product, ib = nbits - rbits
//   div, Saturate:  B = nbits
//   div, Modulo:    B = nbits + rbits    every bit of a wrapped quotient
//
// with g = ceil(log2(B + 8)) + 2 guard bits for mul, and one more for div, whose error is divided by
// x >= 1/2. The default iteration count is B + g. Division by zero follows fixpnt's operator/: it throws
// fixpnt_divide_by_zero, or with FIXPNT_THROW_DIVIDE_BY_ZERO 0 gives maxpos or maxneg by the sign of the
// dividend under Saturate and 0 under Modulo.
//
// Hyperbolic mode. The iterations start at shift 1, since atanh(1) is infinite, and repeat the shifts 4, 13,
// 40, ... (r -> 3r + 1), without which the remaining angle cannot always be absorbed; the iteration count
// counts the repeats. Rotation from (1/K_n, 0, r) ends at (cosh r, sinh r), with
// K_n = prod sqrt(1 - 4^-shift) -> 0.8281593609..., and converges for |r| <= 1.1181730... Vectoring from
// (x, y, 0) ends at (K_n sqrt(x^2 - y^2), 0, atanh(y / x)), for |y / x| <= 0.8068... The functions reduce
// their arguments into those ranges:
//
//   exp, sinh, cosh, tanh  z = q ln 2 + r, |r| <= ln 2 / 2, in a type whose ln 2 carries the bits of q; 2^q
//                          is a shift, and sinh and cosh are 2^(q-1) (e^r -+ 4^-q e^-r) with e^+-r = cosh r
//                          +- sinh r, computed for |z| (cosh is even, sinh odd)
//   log                    w = m 2^e, m in [3/4, 3/2): ln w = e ln 2 + 2 atanh((m - 1) / (m + 1))
//   sqrt                   w = m 4^k, m in [1/4, 1): sqrt w = 2^k x_n / K_n from (m + 1/4, m - 1/4, 0)
//   atanh                  1 - |t| = 2^-e [1, 2): atanh |t| = e ln 2 / 2 + atanh of a ratio in (-1/3, 1/3)
//
// One datapath serves the family: B = max(nbits, rbits + 2) fraction bits, since exp's result reaches
// 2^(ib-1) and needs all of them, and atanh's 1 +- t must be exact; enough integer bits to hold ln(maxpos)
// and ln(minpos); g = ceil(log2(B + 8)) + 3 guard bits; and, by default, every iteration whose shift is at
// most B + g. tan has its own circular datapath of rbits + 2 (ib - 1) + g fraction bits, since the absolute
// error of cos is amplified by 1 / cos^2 <= 2^(2 ib - 2) where tan still fits.
//
// Overflow and the domain. exp, sinh, cosh and tan are not bounded, and wrapping them exactly would need a
// datapath as wide as the result (exp of fixpnt<32,0>'s largest argument has about 3e9 integer bits): a
// result beyond the type saturates in either arithmetic, as an overflow detect in hardware does. sqrt
// saturates as fixpnt's own sqrt does. tanh, atanh and log are bounded and follow the type's arithmetic.
// fixpnt has no NaN, so the domain errors follow <cmath>'s errno convention: log(0) and atanh(+-1) are poles,
// errno = ERANGE with maxneg or maxpos; log of a negative and atanh beyond +-1 are domain errors, errno = EDOM
// with 0; sqrt of a negative follows fixpnt's sqrt, which throws fixpnt_negative_sqrt_arg under
// FIXPNT_THROW_ARITHMETIC_EXCEPTION and otherwise sets errno = EDOM and gives 0.
#include <array>
#include <cerrno>
#include <cstdint>

namespace sw { namespace universal { namespace cordic {

// the guard width sentinel: pick g = ceil(log2(rbits + 8)) + 2
inline constexpr unsigned auto_guard = 0xFFFF'FFFFu;

// the coordinate system m of the unified CORDIC iteration x_{i+1} = x_i - m * d_i * (y_i >> i)
enum class coordinates : int { hyperbolic = -1, linear = 0, circular = 1 };

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
	};

	// the shape of the tan datapath: tan = sin / cos is held to 2^(ib-1), so cos >= 2^-(ib-1), and the
	// absolute error of cos is amplified by 1 / cos^2 <= 2^(2 ib - 2): B = rbits + 2 (ib - 1)
	template<unsigned nbits, unsigned rbits, unsigned guard>
	struct tan_config {
		static_assert(nbits >= 2, "cordic: the target fixpnt needs at least 2 bits");
		static_assert(rbits <= nbits, "cordic: rbits cannot exceed nbits");
		static constexpr unsigned ib = nbits - rbits;
		static constexpr unsigned B = rbits + 2u * (ib >= 1u ? ib - 1u : 0u);
		static constexpr unsigned g = (guard == auto_guard) ? clog2(B + 8u) + 3u : guard;
		static constexpr unsigned R = B + g;
		static constexpr unsigned N = R + 3u;
	};

	// the number of hyperbolic iterations whose shift is at most R: shifts 1 .. R, with 4, 13, 40, ... repeated
	constexpr unsigned hyperbolic_steps(unsigned R) noexcept {
		unsigned n = R;
		for (unsigned r = 4u; r <= R; r = 3u * r + 1u) ++n;
		return n;
	}

	// the shape of the hyperbolic datapath, shared by exp, sinh, cosh, tanh, atanh, log and sqrt.
	// B fraction bits carry the largest result's significance (exp up to 2^(ib-1), sqrt up to
	// 2^((ib-1)/2)), and at least rbits + 2 so atanh's 1 +- t is exact; I integer bits hold
	// ln(maxpos) and ln(minpos), up to (nbits + 1) ln 2.
	template<unsigned nbits, unsigned rbits, unsigned guard>
	struct hyperbolic_config {
		static_assert(nbits >= 2, "cordic: the target fixpnt needs at least 2 bits");
		static_assert(rbits <= nbits, "cordic: rbits cannot exceed nbits");
		static constexpr unsigned ib = nbits - rbits;
		static constexpr unsigned B = (nbits > rbits + 2u) ? nbits : rbits + 2u;
		static constexpr unsigned g = (guard == auto_guard) ? clog2(B + 8u) + 3u : guard;
		static constexpr unsigned R = B + g;
		static constexpr unsigned I = (clog2(nbits + 2u) + 1u > 3u) ? clog2(nbits + 2u) + 1u : 3u;
		static constexpr unsigned N = R + I + 1u;
		static constexpr unsigned steps = hyperbolic_steps(R);
		static constexpr unsigned iterations = steps;                    // the default iteration count
		// the exp reduction z = q ln 2 + r: |q| is needed up to max(ib, rbits + 2), and |z / ln 2| < 2^ib
		static constexpr unsigned qmax = (ib > rbits + 2u) ? ib : rbits + 2u;
		static constexpr unsigned redR = R + clog2(qmax + 1u) + 2u;
		static constexpr unsigned redN = redR + ib + 3u;
	};

	// the shape of the linear datapaths: B result bits plus g guard bits, see the header comment
	template<unsigned nbits, unsigned rbits, bool arithmetic, unsigned guard>
	struct linear_config {
		static_assert(nbits >= 2, "cordic: the target fixpnt needs at least 2 bits");
		static_assert(rbits <= nbits, "cordic: rbits cannot exceed nbits");
		static constexpr unsigned ib = nbits - rbits;
		static constexpr unsigned mulB = (arithmetic == Saturate) ? nbits + 1u : nbits + ib;
		static constexpr unsigned mulG = (guard == auto_guard) ? clog2(mulB + 8u) + 2u : guard;
		static constexpr unsigned mulR = mulB + mulG;
		static constexpr unsigned mulN = mulR + 3u;
		static constexpr unsigned divB = (arithmetic == Saturate) ? nbits : nbits + rbits;
		static constexpr unsigned divG = (guard == auto_guard) ? clog2(divB + 8u) + 3u : guard;
		static constexpr unsigned divR = divB + divG;
		static constexpr unsigned divN = divR + 3u;
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

	// atanh(2^-i) for i >= 1 = sum_k 2^-i(2k+1) / (2k + 1)
	template<typename G>
	G atanh_pow2(unsigned i) {
		G p = G(1) >> static_cast<int>(i);
		G sum = p;
		for (unsigned k = 1; ; ++k) {
			p >>= static_cast<int>(2u * i);
			if (p.iszero()) break;
			sum += p / G(static_cast<int>(2u * k + 1u));
		}
		return sum;
	}

	// ln 2 = 2 atanh(1/3) = 2 sum_k 3^-(2k+1) / (2k + 1)
	template<typename G>
	G ln2_series() {
		G p = G(1) / G(3);
		const G nine(9);
		G sum = p;
		for (unsigned k = 1; ; ++k) {
			p = p / nine;
			if (p.iszero()) break;
			sum += p / G(static_cast<int>(2u * k + 1u));
		}
		sum <<= 1;
		return sum;
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

	// the elementary angles of linear CORDIC, alpha_i = 2^-i, exact in the datapath
	template<typename Work>
	struct linear_tables {
		static constexpr unsigned R = Work::rbits;
		std::array<Work, R> pow2{};
		linear_tables() {
			const Work one(1);
			for (unsigned i = 0; i < R; ++i) pow2[i] = one >> static_cast<int>(i);
		}
	};
	template<typename Work>
	const linear_tables<Work>& linear_tables_of() {
		static const linear_tables<Work> t;
		return t;
	}

	// the iteration tables of a hyperbolic datapath: the shift of every iteration (1, 2, 3, 4, 4, 5, ..., 13,
	// 13, ...), its angle atanh(2^-shift), 1/K_n = 1/prod_{k<n} sqrt(1 - 4^-shift_k) for every n, and ln 2
	// in the generator type, so e ln 2 is exact to the datapath for every exponent e
	template<typename Work>
	struct hyperbolic_tables {
		using G = typename generator<Work>::type;
		static constexpr unsigned R = Work::rbits;
		static constexpr unsigned NS = hyperbolic_steps(R);
		std::array<unsigned, NS> shift{};
		std::array<Work, NS> atanh{};
		std::array<Work, NS + 1u> invK{};
		G ln2{};

		hyperbolic_tables() {
			unsigned k = 0, repeat = 4u;
			for (unsigned i = 1; i <= R; ++i) {
				const Work a = atanh_pow2<G>(i);
				shift[k] = i; atanh[k] = a; ++k;
				if (i == repeat) { shift[k] = i; atanh[k] = a; ++k; repeat = 3u * repeat + 1u; }
			}
			// P_n = prod_{k<n} (1 - 4^-shift_k), by shift and subtract; 1/K_n = 1/sqrt(P_n)
			G P(1);
			invK[0] = Work(1);
			for (unsigned n = 0; n < NS; ++n) {
				P -= (P >> static_cast<int>(2u * shift[n]));
				invK[n + 1u] = G(1) / sqrt(P);
			}
			ln2 = ln2_series<G>();
		}
	};
	template<typename Work>
	const hyperbolic_tables<Work>& hyperbolic_tables_of() {
		static const hyperbolic_tables<Work> t;
		return t;
	}

	// ln 2 and 1/ln 2 in the exp reduction type
	template<typename Red>
	struct ln2_reduction {
		Red ln2{};
		Red inv_ln2{};
		ln2_reduction() {
			using G = typename generator<Red>::type;
			const G l = ln2_series<G>();
			ln2 = l;
			inv_ln2 = G(1) / l;
		}
	};
	template<typename Red>
	const ln2_reduction<Red>& ln2_reduction_of() {
		static const ln2_reduction<Red> t;
		return t;
	}

	// one CORDIC iteration in coordinate system m, with shift i, elementary angle alpha and d = up ? +1 : -1:
	//   x <- x - m d (y >> i),   y <- y + d (x >> i),   z <- z - d alpha
	// The shifts truncate, as an arithmetic shift does in hardware, and both use the values before the step.
	template<coordinates m, typename Work>
	void step(Work& x, Work& y, Work& z, unsigned i, const Work& alpha, bool up) {
		const Work xs = x >> static_cast<int>(i);
		if constexpr (m == coordinates::linear) {                    // m = 0: x does not change
			if (up) { y += xs; z -= alpha; }
			else    { y -= xs; z += alpha; }
		}
		else {
			const Work ys = y >> static_cast<int>(i);
			constexpr bool circular = (m == coordinates::circular);
			if (up) { if constexpr (circular) x -= ys; else x += ys; y += xs; z -= alpha; }
			else    { if constexpr (circular) x += ys; else x -= ys; y -= xs; z += alpha; }
		}
	}

	// rotation mode: drive z to 0, d_i = sign(z_i)
	template<coordinates m, typename Work, std::size_t N>
	void rotate(Work& x, Work& y, Work& z, unsigned n, const std::array<Work, N>& A) {
		for (unsigned i = 0; i < n; ++i) step<m>(x, y, z, i, A[i], !z.isneg());
	}

	// vectoring mode: drive y to 0, d_i = -sign(y_i)
	template<coordinates m, typename Work, std::size_t N>
	void vector(Work& x, Work& y, Work& z, unsigned n, const std::array<Work, N>& A) {
		for (unsigned i = 0; i < n; ++i) step<m>(x, y, z, i, A[i], y.isneg());
	}

	// hyperbolic rotation and vectoring follow the schedule of shifts, with its repeated iterations
	template<typename Work>
	void rotate_hyperbolic(Work& x, Work& y, Work& z, unsigned n, const hyperbolic_tables<Work>& T) {
		for (unsigned k = 0; k < n; ++k) step<coordinates::hyperbolic>(x, y, z, T.shift[k], T.atanh[k], !z.isneg());
	}
	template<typename Work>
	void vector_hyperbolic(Work& x, Work& y, Work& z, unsigned n, const hyperbolic_tables<Work>& T) {
		for (unsigned k = 0; k < n; ++k) step<coordinates::hyperbolic>(x, y, z, T.shift[k], T.atanh[k], y.isneg());
	}

	// round raw * 2^-shift to nearest, ties to even, in place
	template<unsigned W, typename bt>
	void round_shift(blockbinary<W, bt, BinaryNumberType::Signed>& raw, unsigned shift) {
		if (shift == 0) return;
		const bool up = raw.roundingMode(shift);
		raw >>= static_cast<int>(shift);
		if (up) ++raw;
	}

	// the Target whose raw value is round(raw * 2^-shift), saturated or wrapped by Target's arithmetic;
	// saturate = true clamps in either arithmetic, for the results that are not bounded (exp, sinh, cosh, tan)
	template<typename Target, bool saturate = (Target::arithmetic == Saturate), unsigned W, typename bt>
	Target fit(const blockbinary<W, bt, BinaryNumberType::Signed>& rawIn, unsigned shift) {
		constexpr unsigned tbits = Target::nbits;
		constexpr unsigned WW = (W > tbits ? W : tbits) + 1u;
		using Wide = blockbinary<WW, bt, BinaryNumberType::Signed>;
		Wide raw(rawIn);                                    // sign-extends
		round_shift(raw, shift);
		Target t;
		if constexpr (saturate) {
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
		vector<coordinates::circular>(x, y, z, n, T.atan);
		v.x = x;
		v.z = z;
		return v;
	}

	template<typename Fixed>
	blockbinary<Fixed::nbits + 2u, typename Fixed::BlockType, BinaryNumberType::Signed> widen(const Fixed& f) {
		return blockbinary<Fixed::nbits + 2u, typename Fixed::BlockType, BinaryNumberType::Signed>(f.bits());
	}

	constexpr unsigned clamp_iterations(unsigned n, unsigned maxN) noexcept { return n < maxN ? n : maxN; }

	// the index of the most significant set bit of a nonzero magnitude
	template<unsigned W, typename bt>
	unsigned msb_of(const blockbinary<W, bt, BinaryNumberType::Signed>& a) {
		for (unsigned i = W; i-- > 0;) {
			if (a.at(i)) return i;
		}
		return 0;
	}

	// a nonzero target value as a magnitude in [2^e, 2^(e+1)) raw units, with e and the sign
	template<typename Fixed>
	struct magnitude {
		blockbinary<Fixed::nbits + 2u, typename Fixed::BlockType, BinaryNumberType::Signed> raw;
		unsigned e;
		bool negative;
		explicit magnitude(const Fixed& f) : raw(widen(f)), e(0), negative(raw.isneg()) {
			if (negative) raw.twosComplement();              // maxneg's magnitude fits in nbits + 2 bits
			e = msb_of(raw);
		}
	};

	// the datapath value of a magnitude scaled into [1/2, 1)
	template<typename Lin, typename M>
	Lin normalized(const M& a) {
		Lin v;
		v = a.raw;
		v <<= static_cast<int>(Lin::rbits - a.e - 1u);
		return v;
	}

	// the Target whose raw value is round(v * 2^E), for a datapath value 0 <= |v| < 2 with Lin::rbits >= E
	template<typename Target, bool saturate = (Target::arithmetic == Saturate), typename Lin>
	Target scaled_fit(Lin v, int E, bool negative) {
		if (E <= -2) return Target(0);                       // |v * 2^E| < 1/2 rounds to 0
		if (negative) v = -v;
		return fit<Target, saturate>(v.bits(), static_cast<unsigned>(static_cast<int>(Lin::rbits) - E));
	}

	// circular rotation of a target angle in the datapath Rot, through the range reduction when the target
	// holds angles beyond the convergence range 1.74...: S = sin(theta), C = cos(theta)
	template<typename Rot, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	void circular_sincos(const fixpnt<nbits, rbits, arithmetic, bt>& theta, unsigned n, Rot& S, Rot& C) {
		constexpr unsigned R  = Rot::rbits;
		constexpr unsigned ib = nbits - rbits;
		// range reduction: |theta| < 2^M
		constexpr bool reduce   = (ib >= 2u);
		constexpr unsigned M    = reduce ? ib - 1u : 0u;
		constexpr unsigned redR = R + M + 2u;
		constexpr unsigned redN = redR + M + 3u;
		const auto& T = tables_of<Rot>();

		Rot r;
		unsigned quadrant = 0;
		if constexpr (!reduce) {
			r = theta.bits();                                   // |theta| <= 1: inside the convergence range
			r <<= static_cast<int>(R - rbits);
		}
		else {
			// theta = q pi/2 + r, with q = round(theta 2/pi)
			using Red = fixpnt<redN, redR, Modulo, bt>;
			using RedBits = blockbinary<redN, bt, BinaryNumberType::Signed>;
			const auto& RC = reduction_of<Red>();
			Red xr;
			xr = theta.bits();
			xr <<= static_cast<int>(redR - rbits);
			RedBits q = (xr * RC.two_over_pi).bits();
			round_shift(q, redR);
			quadrant = (q.at(0) ? 1u : 0u) | (q.at(1) ? 2u : 0u);
			Red qf;
			qf = q;
			qf <<= static_cast<int>(redR);
			RedBits rr = (xr - qf * RC.pi_over_2).bits();
			round_shift(rr, redR - R);
			r = rr;                                             // |r| <= pi/4: the low bits hold it
		}

		Rot x = T.invK[n], y(0), z = r;
		rotate<coordinates::circular>(x, y, z, n, T.atan);

		switch (quadrant) {
		case 0:  S = y;  C = x;  break;
		case 1:  S = x;  C = -y; break;
		case 2:  S = -y; C = -x; break;
		default: S = -x; C = y;  break;
		}
	}

	// a datapath value as m 2^e with |m| in [1/2, 1), for a nonzero v
	template<typename Work>
	struct split {
		Work m{};
		int e{ 0 };
		bool negative{ false };
		explicit split(Work v) {
			negative = v.isneg();
			if (negative) v = -v;
			const unsigned p = msb_of(v.bits());
			const int s = static_cast<int>(Work::rbits) - 1 - static_cast<int>(p);
			if (s >= 0) v <<= s; else v >>= -s;
			m = v;
			e = static_cast<int>(p) + 1 - static_cast<int>(Work::rbits);
		}
	};

	// the integer value of a raw integer, clamped to [-limit, limit]
	template<unsigned W, typename bt>
	int clamped_int(blockbinary<W, bt, BinaryNumberType::Signed> v, int limit) {
		const bool negative = v.isneg();
		if (negative) v.twosComplement();                    // the most negative value keeps its top bit: large
		long long value = 0;
		for (unsigned i = W; i-- > 0;) {
			if (!v.at(i)) continue;
			if (i >= 40u) return negative ? -limit : limit;
			value |= (1ll << i);
		}
		if (value > limit) value = limit;
		return static_cast<int>(negative ? -value : value);
	}

	// z = q ln 2 + r, q = round(z / ln 2), |r| <= ln 2 / 2 plus a unit of the datapath; with absolute, of |z|.
	// q is clamped to [-limit, limit], and r is meaningful only inside.
	template<typename H>
	struct ln2_reduced {
		int q{ 0 };
		H r{};
	};
	template<typename Cfg, typename H, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	ln2_reduced<H> reduce_ln2(const fixpnt<nbits, rbits, arithmetic, bt>& z, bool absolute, int limit) {
		using Red = fixpnt<Cfg::redN, Cfg::redR, Modulo, bt>;
		using RedBits = blockbinary<Cfg::redN, bt, BinaryNumberType::Signed>;
		const auto& C = ln2_reduction_of<Red>();
		Red xr;
		xr = z.bits();                                      // sign-extends
		xr <<= static_cast<int>(Cfg::redR - rbits);
		if (absolute && xr.isneg()) xr = -xr;               // |maxneg| = 2^(ib-1) fits the ib + 3 integer bits
		RedBits q = (xr * C.inv_ln2).bits();
		round_shift(q, Cfg::redR);
		ln2_reduced<H> out;
		out.q = clamped_int(q, limit);
		if (out.q <= -limit || out.q >= limit) return out;
		Red qf;
		qf = q;
		qf <<= static_cast<int>(Cfg::redR);
		RedBits rr = (xr - qf * C.ln2).bits();
		round_shift(rr, Cfg::redR - Cfg::R);
		out.r = rr;                                         // |r| <= ln 2 / 2: the low bits hold it
		return out;
	}

	// e ln 2 * 2^-halve in the datapath, rounded once from the generator's precision
	template<typename H>
	H times_ln2(int e, unsigned halve = 0) {
		using G = typename generator<H>::type;
		G v = G(e) * hyperbolic_tables_of<H>().ln2;
		v >>= static_cast<int>(halve);
		H h = v;
		return h;
	}

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
	const unsigned n = detail::clamp_iterations(iterations, Cfg::rotR);
	Rot S, Co;
	detail::circular_sincos<Rot>(theta, n, S, Co);
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

// linear rotation mode: x * z. Both operands are scaled into [1/2, 1), the iterations accumulate the
// shifted x under the signed binary digits of z, and the product is scaled back and rounded once.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> mul(const fixpnt<nbits, rbits, arithmetic, bt>& x, const fixpnt<nbits, rbits, arithmetic, bt>& z,
                                         unsigned iterations = detail::linear_config<nbits, rbits, arithmetic, guard>::mulR) {
	using Cfg    = detail::linear_config<nbits, rbits, arithmetic, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using Lin    = fixpnt<Cfg::mulN, Cfg::mulR, Modulo, bt>;
	if (x.iszero() || z.iszero()) return Target(0);
	const detail::magnitude<Target> ax(x), az(z);
	const bool negative = (ax.negative != az.negative);
	// x * z = x' z' 2^E raw units, with x', z' in [1/2, 1)
	const int E = static_cast<int>(ax.e + az.e + 2u) - static_cast<int>(rbits);
	if constexpr (arithmetic == Saturate) {
		// |x' z'| >= 1/4, so the product is at least 2^(E-2) >= 2^nbits raw units: beyond maxpos
		if (E >= static_cast<int>(nbits) + 2) return negative ? Target(SpecificValue::maxneg) : Target(SpecificValue::maxpos);
	}
	const unsigned n = detail::clamp_iterations(iterations, Cfg::mulR);
	Lin xv = detail::normalized<Lin>(ax), yv(0), zv = detail::normalized<Lin>(az);
	detail::rotate<coordinates::linear>(xv, yv, zv, n, detail::linear_tables_of<Lin>().pow2);
	return detail::scaled_fit<Target>(yv, E, negative);
}

// linear vectoring mode: y / x. Both operands are scaled into [1/2, 1), so y' / x' lies in (1/2, 2),
// inside the convergence range; the iterations subtract the shifted x from y and count it in z.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> div(const fixpnt<nbits, rbits, arithmetic, bt>& y, const fixpnt<nbits, rbits, arithmetic, bt>& x,
                                         unsigned iterations = detail::linear_config<nbits, rbits, arithmetic, guard>::divR) {
	using Cfg    = detail::linear_config<nbits, rbits, arithmetic, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using Lin    = fixpnt<Cfg::divN, Cfg::divR, Modulo, bt>;
	if (x.iszero()) {
#if FIXPNT_THROW_DIVIDE_BY_ZERO
		throw fixpnt_divide_by_zero();
#else
		if constexpr (arithmetic == Saturate) {
			return y.isneg() ? Target(SpecificValue::maxneg) : Target(SpecificValue::maxpos);
		}
		else {
			return Target(0);
		}
#endif
	}
	if (y.iszero()) return Target(0);
	const detail::magnitude<Target> ay(y), ax(x);
	const bool negative = (ay.negative != ax.negative);
	// y / x = (y' / x') 2^E raw units, with y', x' in [1/2, 1)
	const int E = static_cast<int>(ay.e + rbits) - static_cast<int>(ax.e);
	if constexpr (arithmetic == Saturate) {
		// y' / x' > 1/2, so the quotient exceeds 2^(E-1) >= 2^(nbits-1) raw units: beyond maxpos and maxneg
		if (E >= static_cast<int>(nbits)) return negative ? Target(SpecificValue::maxneg) : Target(SpecificValue::maxpos);
	}
	const unsigned n = detail::clamp_iterations(iterations, Cfg::divR);
	Lin xv = detail::normalized<Lin>(ax), yv = detail::normalized<Lin>(ay), zv(0);
	detail::vector<coordinates::linear>(xv, yv, zv, n, detail::linear_tables_of<Lin>().pow2);
	return detail::scaled_fit<Target>(zv, E, negative);
}

// tan = sin / cos: circular rotation, then linear vectoring on the normalized sin and cos. The rotation
// datapath is wider than sincos's, since the absolute error of cos is amplified by 1 / cos^2. tan is not
// bounded: a result beyond the type saturates in either arithmetic. The iteration count sets both passes.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> tan(const fixpnt<nbits, rbits, arithmetic, bt>& theta,
                                         unsigned iterations = detail::tan_config<nbits, rbits, guard>::R) {
	using Cfg    = detail::tan_config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using Rot    = fixpnt<Cfg::N, Cfg::R, Modulo, bt>;
	const unsigned n = detail::clamp_iterations(iterations, Cfg::R);
	Rot S, C;
	detail::circular_sincos<Rot>(theta, n, S, C);
	if (S.iszero()) return Target(0);
	if (C.iszero()) return S.isneg() ? Target(SpecificValue::maxneg) : Target(SpecificValue::maxpos);
	const detail::split<Rot> s(S), c(C);
	const bool negative = (s.negative != c.negative);
	const int E = s.e - c.e;                                    // tan = (ms / mc) 2^E, ms / mc in (1/2, 2)
	if (E >= static_cast<int>(Cfg::ib)) return negative ? Target(SpecificValue::maxneg) : Target(SpecificValue::maxpos);
	Rot x = c.m, y = s.m, z(0);
	detail::vector<coordinates::linear>(x, y, z, n, detail::linear_tables_of<Rot>().pow2);
	return detail::scaled_fit<Target, true>(z, E + static_cast<int>(rbits), negative);
}

// the default iteration count and the guard width of the hyperbolic engine, the repeated iterations included
template<typename Fixed, unsigned guard = auto_guard>
inline constexpr unsigned hyperbolic_iterations = detail::hyperbolic_config<Fixed::nbits, Fixed::rbits, guard>::iterations;
template<typename Fixed, unsigned guard = auto_guard>
inline constexpr unsigned hyperbolic_guard_bits = detail::hyperbolic_config<Fixed::nbits, Fixed::rbits, guard>::g;

// hyperbolic rotation mode: exp z = 2^q (cosh r + sinh r), with z = q ln 2 + r. exp is not bounded: a
// result beyond maxpos saturates in either arithmetic, and one below half a unit is 0.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> exp(const fixpnt<nbits, rbits, arithmetic, bt>& z,
                                         unsigned iterations = detail::hyperbolic_config<nbits, rbits, guard>::iterations) {
	using Cfg    = detail::hyperbolic_config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using H      = fixpnt<Cfg::N, Cfg::R, Modulo, bt>;
	const auto& T = detail::hyperbolic_tables_of<H>();
	const unsigned n = detail::clamp_iterations(iterations, Cfg::steps);
	const auto rd = detail::reduce_ln2<Cfg, H>(z, false, static_cast<int>(Cfg::qmax));
	// e^r >= 1/sqrt(2): q >= ib exceeds 2^(ib-1); e^r <= sqrt(2): q <= -(rbits + 2) is below half a unit
	if (rd.q >= static_cast<int>(Cfg::ib)) return Target(SpecificValue::maxpos);
	if (rd.q <= -static_cast<int>(rbits + 2u)) return Target(0);
	H x = T.invK[n], y(0), zz = rd.r;
	detail::rotate_hyperbolic(x, y, zz, n, T);
	const H e = x + y;
	return detail::fit<Target, true>(e.bits(), static_cast<unsigned>(static_cast<int>(Cfg::R - rbits) - rd.q));
}

// hyperbolic rotation mode: s = sinh(z) and c = cosh(z). With |z| = q ln 2 + r,
// cosh |z| = 2^(q-1) (e^r + 4^-q e^-r) and sinh |z| = 2^(q-1) (e^r - 4^-q e^-r), where e^r and e^-r are
// cosh r +- sinh r; for q = 0 they are cosh r and sinh r themselves. A result beyond the type saturates.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
void sinhcosh(const fixpnt<nbits, rbits, arithmetic, bt>& z,
              fixpnt<nbits, rbits, arithmetic, bt>& s, fixpnt<nbits, rbits, arithmetic, bt>& c,
              unsigned iterations = detail::hyperbolic_config<nbits, rbits, guard>::iterations) {
	using Cfg    = detail::hyperbolic_config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using H      = fixpnt<Cfg::N, Cfg::R, Modulo, bt>;
	const auto& T = detail::hyperbolic_tables_of<H>();
	const unsigned n = detail::clamp_iterations(iterations, Cfg::steps);
	const bool negative = z.isneg();
	// q >= ib + 2: both exceed 2^(q-1) (e^r - e^-r / 16) >= 2^(ib+1) (1/sqrt(2) - sqrt(2)/16) > 2^(ib-1);
	// at q = ib + 1, sinh |z| can still fit, as sinh(0.35) does in fixpnt<8,8>
	const auto rd = detail::reduce_ln2<Cfg, H>(z, true, static_cast<int>(Cfg::ib) + 2);
	if (rd.q >= static_cast<int>(Cfg::ib) + 2) {
		c = Target(SpecificValue::maxpos);
		s = negative ? Target(SpecificValue::maxneg) : Target(SpecificValue::maxpos);
		return;
	}
	H x = T.invK[n], y(0), zz = rd.r;
	detail::rotate_hyperbolic(x, y, zz, n, T);
	const H ep = x + y;
	H em = x - y;
	const unsigned twoq = 2u * static_cast<unsigned>(rd.q);
	if (twoq >= Cfg::R + 4u) em.setzero(); else em >>= static_cast<int>(twoq);
	const H C = ep + em;
	H S = ep - em;
	if (negative) S = -S;
	const unsigned shift = static_cast<unsigned>(static_cast<int>(Cfg::R - rbits) - rd.q + 1);
	s = detail::fit<Target, true>(S.bits(), shift);
	c = detail::fit<Target, true>(C.bits(), shift);
}

template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> sinh(const fixpnt<nbits, rbits, arithmetic, bt>& z,
                                          unsigned iterations = detail::hyperbolic_config<nbits, rbits, guard>::iterations) {
	fixpnt<nbits, rbits, arithmetic, bt> s, c;
	sinhcosh<guard>(z, s, c, iterations);
	return s;
}

template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> cosh(const fixpnt<nbits, rbits, arithmetic, bt>& z,
                                          unsigned iterations = detail::hyperbolic_config<nbits, rbits, guard>::iterations) {
	fixpnt<nbits, rbits, arithmetic, bt> s, c;
	sinhcosh<guard>(z, s, c, iterations);
	return c;
}

// tanh = sinh / cosh: hyperbolic rotation, then linear vectoring of (e^r - 4^-q e^-r) / (e^r + 4^-q e^-r),
// in which 2^(q-1) cancels. Once 4^-q is below the datapath, tanh is +-1 to the datapath.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> tanh(const fixpnt<nbits, rbits, arithmetic, bt>& z,
                                          unsigned iterations = detail::hyperbolic_config<nbits, rbits, guard>::iterations) {
	using Cfg    = detail::hyperbolic_config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using H      = fixpnt<Cfg::N, Cfg::R, Modulo, bt>;
	const auto& T = detail::hyperbolic_tables_of<H>();
	const unsigned n = detail::clamp_iterations(iterations, Cfg::steps);
	const bool negative = z.isneg();
	constexpr int qbig = static_cast<int>(Cfg::R + 4u) / 2;     // 2q >= R + 4: 1 - tanh < 2^-(R+1)
	const auto rd = detail::reduce_ln2<Cfg, H>(z, true, qbig);
	H t;
	if (rd.q >= qbig) {
		t = H(1);
	}
	else {
		H x = T.invK[n], y(0), zz = rd.r;
		detail::rotate_hyperbolic(x, y, zz, n, T);
		const H ep = x + y;
		H em = x - y;
		em >>= static_cast<int>(2u * static_cast<unsigned>(rd.q));
		H D = ep + em, Nm = ep - em;                             // D in (1/2, 3), Nm / D in [0, 1)
		t.setzero();
		detail::vector<coordinates::linear>(D, Nm, t, detail::clamp_iterations(iterations, Cfg::R), detail::linear_tables_of<H>().pow2);
	}
	if (negative) t = -t;
	return detail::fit<Target>(t.bits(), Cfg::R - rbits);
}

// hyperbolic vectoring mode: atanh(t) = (1/2) ln((1 + t) / (1 - t)). With 1 - |t| = 2^-e [1, 2),
// atanh |t| = e ln 2 / 2 + atanh(((1 + |t|) - (1 - |t|) 2^e) / ((1 + |t|) + (1 - |t|) 2^e)), whose ratio
// lies in (-1/3, 1/3), inside the convergence range, for every |t| < 1. atanh(+-1) is a pole: errno =
// ERANGE and maxpos or maxneg. |t| > 1 is outside the domain: errno = EDOM and 0, as fixpnt has no NaN.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> atanh(const fixpnt<nbits, rbits, arithmetic, bt>& t,
                                           unsigned iterations = detail::hyperbolic_config<nbits, rbits, guard>::iterations) {
	using Cfg    = detail::hyperbolic_config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using H      = fixpnt<Cfg::N, Cfg::R, Modulo, bt>;
	if (t.iszero()) return Target(0);
	const detail::magnitude<Target> a(t);
	blockbinary<nbits + 2u, bt, BinaryNumberType::Signed> one;
	one.clear();
	one.setbit(rbits);
	if (a.raw == one) {
		errno = ERANGE;
		return a.negative ? Target(SpecificValue::maxneg) : Target(SpecificValue::maxpos);
	}
	if (one < a.raw) {
		errno = EDOM;
		return Target(0);
	}
	const auto& T = detail::hyperbolic_tables_of<H>();
	const unsigned n = detail::clamp_iterations(iterations, Cfg::steps);
	const H unit(1);
	H u;
	u = a.raw;
	u <<= static_cast<int>(Cfg::R - rbits);
	const H v = unit - u;                                       // 1 - |t| in (0, 1], exact
	u += unit;                                                  // 1 + |t| in [1, 2), exact
	const unsigned e = Cfg::R - detail::msb_of(v.bits());       // v in [2^-e, 2^(1-e))
	H ve = v;
	ve <<= static_cast<int>(e);                                 // in [1, 2), exact
	H x = u + ve, y = u - ve, z(0);
	detail::vector_hyperbolic(x, y, z, n, T);
	H r = detail::times_ln2<H>(static_cast<int>(e), 1u);
	r += z;
	if (a.negative) r = -r;
	return detail::fit<Target>(r.bits(), Cfg::R - rbits);
}

// hyperbolic vectoring mode: ln w = e ln 2 + 2 atanh((m - 1) / (m + 1)), with w = m 2^e and m in [3/4, 3/2),
// from the start (m + 1, m - 1, 0). ln 0 is a pole: errno = ERANGE and maxneg. A negative w is outside the
// domain: errno = EDOM and 0, as fixpnt has no NaN.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> log(const fixpnt<nbits, rbits, arithmetic, bt>& w,
                                         unsigned iterations = detail::hyperbolic_config<nbits, rbits, guard>::iterations) {
	using Cfg    = detail::hyperbolic_config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using H      = fixpnt<Cfg::N, Cfg::R, Modulo, bt>;
	if (w.iszero()) {
		errno = ERANGE;
		return Target(SpecificValue::maxneg);
	}
	if (w.isneg()) {
		errno = EDOM;
		return Target(0);
	}
	const auto& T = detail::hyperbolic_tables_of<H>();
	const unsigned n = detail::clamp_iterations(iterations, Cfg::steps);
	const detail::magnitude<Target> a(w);
	H m = detail::normalized<H>(a);                             // [1/2, 1)
	int e = static_cast<int>(a.e) + 1 - static_cast<int>(rbits);
	if (a.e > 0 && !a.raw.at(a.e - 1u)) { m <<= 1; --e; }       // [1/2, 3/4) -> [1, 3/2)
	const H unit(1);
	H x = m + unit, y = m - unit, z(0);
	detail::vector_hyperbolic(x, y, z, n, T);
	H l = z;
	l <<= 1;
	if (e != 0) l += detail::times_ln2<H>(e);
	return detail::fit<Target>(l.bits(), Cfg::R - rbits);
}

// hyperbolic vectoring mode: sqrt w = 2^k sqrt(m) with w = m 4^k and m in [1/4, 1). From the start
// (m + 1/4, m - 1/4, 0), x_n = K_n sqrt((m + 1/4)^2 - (m - 1/4)^2) = K_n sqrt(m). A negative w follows
// fixpnt's sqrt: it throws fixpnt_negative_sqrt_arg under FIXPNT_THROW_ARITHMETIC_EXCEPTION, and otherwise
// sets errno = EDOM and gives 0; a root the type cannot hold saturates, as it does there.
template<unsigned guard = auto_guard, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> sqrt(const fixpnt<nbits, rbits, arithmetic, bt>& w,
                                          unsigned iterations = detail::hyperbolic_config<nbits, rbits, guard>::iterations) {
	using Cfg    = detail::hyperbolic_config<nbits, rbits, guard>;
	using Target = fixpnt<nbits, rbits, arithmetic, bt>;
	using H      = fixpnt<Cfg::N, Cfg::R, Modulo, bt>;
	if (w.iszero()) return Target(0);
	if (w.isneg()) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		throw fixpnt_negative_sqrt_arg();
#else
		errno = EDOM;
		return Target(0);
#endif
	}
	const auto& T = detail::hyperbolic_tables_of<H>();
	const unsigned n = detail::clamp_iterations(iterations, Cfg::steps);
	const detail::magnitude<Target> a(w);
	H m = detail::normalized<H>(a);                             // [1/2, 1)
	int E = static_cast<int>(a.e) + 1 - static_cast<int>(rbits);  // w = m 2^E
	if (E % 2 != 0) { m >>= 1; ++E; }                           // [1/4, 1/2), exact: the low bits are zero
	const int k = E / 2;
	H quarter(1);
	quarter >>= 2;
	H x = m + quarter, y = m - quarter, z(0);
	detail::vector_hyperbolic(x, y, z, n, T);
	const H r = x * T.invK[n];
	return detail::fit<Target, true>(r.bits(), static_cast<unsigned>(static_cast<int>(Cfg::R - rbits) - k));
}

}}} // namespace sw::universal::cordic
