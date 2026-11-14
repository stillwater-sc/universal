// cordic_hyperbolic.cpp: the hyperbolic CORDIC engine for fixpnt: exp, sinh, cosh, tanh, atanh, log, sqrt,
// and tan, which completes the circular engine (#2041)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// cordic::exp, sinh, cosh, sinhcosh and tanh (hyperbolic rotation), cordic::atanh, log and sqrt (hyperbolic
// vectoring), and cordic::tan (circular rotation, then linear vectoring) model a shift-and-add CORDIC unit
// in the type's own arithmetic. With the default guard width and iteration count, every result is within
// kWorst = 0.6 ulp of the exact value, at any width; the measured worst case is printed for each
// configuration. A final step that truncates instead of rounding to nearest exceeds that bound.
//
// The references are independent of the engine: double for formats of at most 32 bits, whose values a
// double holds exactly and whose ulp is far above double's error, and qd (~212 bits) beyond that, with the
// fixpnt value converted to qd exactly, 32 bits at a time.
//
// The error is measured in ulps of the target, which for a fixed-point type is an absolute error, after
// the type's arithmetic. The unbounded results, exp, sinh, cosh, tan, and sqrt as fixpnt's own sqrt does,
// saturate in either arithmetic, so they are measured against the exact value clamped to [maxneg, maxpos].
// The bounded ones, tanh, atanh and log, follow the arithmetic: Saturate clamps, and Modulo measures the
// distance modulo 2^nbits.
//
// The double-path exp, log, sinh, cosh, tanh, atanh and sqrt keep their own tests.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;
	// the double overloads, ahead of everything: unqualified, abs(double) would find an integer abs
	using std::abs; using std::floor; using std::ldexp;
	using std::exp; using std::log; using std::sqrt; using std::sinh; using std::cosh; using std::tanh;
	using std::atanh; using std::tan; using std::atan;

	// the exact value of a fixpnt as a qd, 32 bits at a time
	template<typename Fixed>
	qd to_qd(const Fixed& f) {
		auto raw = f.bits();
		const bool negative = raw.isneg();
		if (negative) raw.twosComplement();             // maxneg stays maxneg, whose unsigned reading is its magnitude
		qd sum(0.0);
		for (unsigned c = 0; c < Fixed::nbits; c += 32u) {
			std::uint64_t chunk = 0;
			for (unsigned i = 0; i < 32u && c + i < Fixed::nbits; ++i) if (raw.at(c + i)) chunk |= (1ull << i);
			sum += qd(std::ldexp(static_cast<double>(chunk), static_cast<int>(c) - static_cast<int>(Fixed::rbits)));
		}
		return negative ? -sum : sum;
	}

	// the reference value type: double for formats of at most 32 bits, qd beyond
	template<typename Fixed>
	using Ref = std::conditional_t<(Fixed::nbits <= 32u), double, qd>;

	template<typename Fixed>
	Ref<Fixed> exact_of(const Fixed& f) {
		if constexpr (std::is_same_v<Ref<Fixed>, double>) return double(f); else return to_qd(f);
	}

	// the largest encoding at or below v, from its raw value floor(v 2^rbits), bit by bit; |v| < 2^(nbits-rbits-1)
	template<typename Fixed, typename R>
	Fixed floor_of(const R& v) {
		R u = floor(v * ldexp(R(1.0), static_cast<int>(Fixed::rbits)));
		const bool negative = u < R(0.0);
		if (negative) u = -u;                               // an integer: the raw magnitude, exactly
		Fixed f;
		f.clear();
		for (unsigned i = Fixed::nbits - 1u; i-- > 0;) {
			const R w = ldexp(R(1.0), static_cast<int>(i));
			if (!(u < w)) { f.setbit(i, true); u -= w; }
		}
		return negative ? Fixed(-f) : f;
	}

	// the error of got against the exact value e, in ulps, after the type's arithmetic; clamp measures an
	// unbounded function, which saturates in either arithmetic
	template<typename Fixed, typename R>
	double ulp_error(const Fixed& got, R e, bool clamp = false) {
		constexpr int rbits = static_cast<int>(Fixed::rbits);
		const R g = exact_of(got);
		if (clamp || Fixed::arithmetic == Saturate) {
			const R hi = exact_of(Fixed(SpecificValue::maxpos)), lo = exact_of(Fixed(SpecificValue::maxneg));
			if (!(e < hi)) e = hi;                      // also catches an infinite reference
			if (!(e > lo)) e = lo;
			return double(abs(R(g - e)) * ldexp(R(1.0), rbits));
		}
		else {
			const R period = ldexp(R(1.0), static_cast<int>(Fixed::nbits) - rbits);
			R d = g - e;
			d = d - period * floor(d / period + R(0.5));     // the distance modulo 2^nbits ulps
			return double(abs(d) * ldexp(R(1.0), rbits));
		}
	}

	// a random encoding shifted right by a random amount, so the magnitudes are spread over every binade
	template<typename Fixed>
	Fixed random_fixpnt(std::mt19937_64& rng) {
		Fixed f;
		f.clear();
		for (unsigned b = 0; b < Fixed::nbits; b += 64u) {
			const std::uint64_t word = rng();
			for (unsigned k = 0; k < 64u && b + k < Fixed::nbits; ++k) f.setbit(b + k, (word >> k) & 1u);
		}
		f >>= static_cast<int>(rng() % Fixed::nbits);
		return f;
	}

	struct Worst { double exp = 0, sinh = 0, cosh = 0, tanh = 0, atanh = 0, log = 0, sqrt = 0, tan = 0; };

	// every function of every encoding (nbits <= 16), or of samples; sinhcosh must agree with sinh and cosh
	template<typename Fixed>
	int Measure(Worst& w, unsigned long long samples, bool reportTestCases) {
		using R = Ref<Fixed>;
		int fails = 0;
		auto one = [&](const Fixed& x) {
			const R e = exact_of(x);
			Fixed s, c;
			cordic::sinhcosh(x, s, c);
			if (s != cordic::sinh(x) || c != cordic::cosh(x)) {
				if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(x) << " sinhcosh(" << x << ") disagrees with sinh/cosh\n";
				++fails;
			}
			w.exp  = std::max(w.exp,  ulp_error(cordic::exp(x), R(exp(e)), true));
			w.sinh = std::max(w.sinh, ulp_error(s, R(sinh(e)), true));
			w.cosh = std::max(w.cosh, ulp_error(c, R(cosh(e)), true));
			w.tanh = std::max(w.tanh, ulp_error(cordic::tanh(x), R(tanh(e))));
			w.tan  = std::max(w.tan,  ulp_error(cordic::tan(x), R(tan(e)), true));
			if (abs(e) < R(1.0)) w.atanh = std::max(w.atanh, ulp_error(cordic::atanh(x), R(atanh(e))));
			if (e > R(0.0)) {
				w.log  = std::max(w.log,  ulp_error(cordic::log(x), R(log(e))));
				w.sqrt = std::max(w.sqrt, ulp_error(cordic::sqrt(x), R(sqrt(e)), true));
			}
		};
		if constexpr (Fixed::nbits <= 16u) {
			if (samples == 0) {
				for (std::uint64_t b = 0; b < (1ull << Fixed::nbits); ++b) { Fixed x; x.setbits(b); one(x); }
				return fails;
			}
		}
		std::mt19937_64 rng(2041);
		for (unsigned long long i = 0; i < samples; ++i) one(random_fixpnt<Fixed>(rng));
		return fails;
	}

	// every function within the bound at the default guard width and iteration count. The measured worst
	// over every configuration below, all four levels, is 0.566 ulp (tan in fixpnt<16,15>); kWorst leaves a
	// little room for samples, and a final step that truncates exceeds it.
	constexpr double kWorst = 0.6;
	template<typename Fixed>
	int VerifyHyperbolic(const std::string& tag, bool reportTestCases, unsigned long long samples) {
		Worst w;
		int fails = Measure<Fixed>(w, samples, reportTestCases);
		std::cout << std::left << std::setw(22) << tag << std::fixed << std::setprecision(3)
		          << " worst ulp: exp " << w.exp << "  sinh " << w.sinh << "  cosh " << w.cosh << "  tanh " << w.tanh
		          << "  atanh " << w.atanh << "  log " << w.log << "  sqrt " << w.sqrt << "  tan " << w.tan
		          << "   (g = " << cordic::hyperbolic_guard_bits<Fixed> << ", n = " << cordic::hyperbolic_iterations<Fixed> << ")\n";
		std::cout.unsetf(std::ios::fixed);
		for (double e : { w.exp, w.sinh, w.cosh, w.tanh, w.atanh, w.log, w.sqrt, w.tan }) {
			if (e > kWorst) {
				if (reportTestCases) std::cerr << "FAIL: " << tag << " error " << e << " ulp exceeds " << kWorst << " ulp\n";
				++fails;
			}
		}
		return fails;
	}

	// the shift of hyperbolic iteration k (0-based): 1, 2, 3, 4, 4, 5, ..., 13, 13, ...
	inline unsigned shift_of(unsigned k) {
		unsigned i = 1, repeat = 4;
		for (unsigned j = 0; j < k; ++j) {
			if (i == repeat) { repeat = 3u * repeat + 1u; ++j; if (j == k) return i; }
			++i;
		}
		return i;
	}

	// the error falls about one bit per iteration, until the guard bits run out; a repeated iteration
	// (the 5th, with shift 4 again) does not halve it
	template<typename Fixed>
	int VerifyIterationCount(bool reportTestCases) {
		constexpr unsigned rbits = Fixed::rbits;
		int fails = 0;
		double previous = 0.0;
		std::cout << type_tag(Fixed()) << " worst exp/log error against the iteration count (shift of the last iteration):\n";
		for (unsigned n = 2; n <= rbits + 2u; ++n) {
			double we = 0.0, wl = 0.0;
			for (std::uint64_t b = 0; b < (1ull << Fixed::nbits); ++b) {
				Fixed x;
				x.setbits(b);
				const double e = double(x);
				we = std::max(we, ulp_error(cordic::exp(x, n), std::exp(e), true));
				if (e > 0.0) wl = std::max(wl, ulp_error(cordic::log(x, n), std::log(e)));
			}
			const unsigned s = shift_of(n - 1u);
			std::cout << "  n = " << std::setw(2) << n << " (shift " << std::setw(2) << s << ")  exp " << std::setw(10) << we
			          << "  log " << std::setw(10) << wl << '\n';
			// the residual angle after the last shift s is at most about 2^-(s-1): exp is up to 2^(ib-1) times
			// that, and log twice it
			const double worst = std::max(we, wl);
			const double bound = std::ldexp(1.0, static_cast<int>(Fixed::nbits) - static_cast<int>(s) + 1) + 1.0;
			if (worst > bound || (n > 2 && worst > previous * 1.0001 + 0.5)) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " n = " << n << " error " << worst << " above " << bound << " or rising\n";
				++fails;
			}
			previous = worst;
		}
		return fails;
	}

	// the guard width sets the floor: with too few guard bits the truncation error shows
	template<typename Fixed>
	int VerifyGuardWidth(bool reportTestCases) {
		int fails = 0;
		auto worst = [](auto guardTag) {
			using G = decltype(guardTag);
			double w = 0.0;
			for (std::uint64_t b = 0; b < (1ull << Fixed::nbits); ++b) {
				Fixed x;
				x.setbits(b);
				const double e = double(x);
				w = std::max(w, ulp_error(cordic::exp<G::value>(x), std::exp(e), true));
				if (e > 0.0) w = std::max(w, ulp_error(cordic::log<G::value>(x), std::log(e)));
			}
			return w;
		};
		const double g0 = worst(std::integral_constant<unsigned, 0>{});
		const double g2 = worst(std::integral_constant<unsigned, 2>{});
		const double g4 = worst(std::integral_constant<unsigned, 4>{});
		const double ga = worst(std::integral_constant<unsigned, cordic::auto_guard>{});
		std::cout << type_tag(Fixed()) << " worst exp/log error at the default iteration count: g = 0: " << g0
		          << "  g = 2: " << g2 << "  g = 4: " << g4 << "  g = " << cordic::hyperbolic_guard_bits<Fixed> << ": " << ga << '\n';
		if (!(g0 > g2 && g2 > g4 && g4 >= ga && g0 > 1.0 && ga <= kWorst)) {
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " the error does not fall with the guard width\n";
			++fails;
		}
		return fails;
	}

	// the worst error of sinh, cosh, tanh, atanh, sqrt and tan over every encoding, at an iteration count
	// and a guard width; tan has its own circular datapath, and its count sets both of its passes
	template<typename Fixed, unsigned guard = cordic::auto_guard>
	Worst MeasureOthers(unsigned n, unsigned nTan) {
		Worst w;
		for (std::uint64_t b = 0; b < (1ull << Fixed::nbits); ++b) {
			Fixed x;
			x.setbits(b);
			const double e = double(x);
			w.sinh = std::max(w.sinh, ulp_error(cordic::sinh<guard>(x, n), std::sinh(e), true));
			w.cosh = std::max(w.cosh, ulp_error(cordic::cosh<guard>(x, n), std::cosh(e), true));
			w.tanh = std::max(w.tanh, ulp_error(cordic::tanh<guard>(x, n), std::tanh(e)));
			if (std::abs(e) < 1.0) w.atanh = std::max(w.atanh, ulp_error(cordic::atanh<guard>(x, n), std::atanh(e)));
			if (e > 0.0) w.sqrt = std::max(w.sqrt, ulp_error(cordic::sqrt<guard>(x, n), std::sqrt(e), true));
			w.tan = std::max(w.tan, ulp_error(cordic::tan<guard>(x, nTan), std::tan(e), true));
		}
		return w;
	}

	inline std::array<double, 6> others_of(const Worst& w) { return { w.sinh, w.cosh, w.tanh, w.atanh, w.sqrt, w.tan }; }
	constexpr const char* kOtherNames[6] = { "sinh", "cosh", "tanh", "atanh", "sqrt", "tan" };

	// the iteration count controls sinh, cosh, tanh, atanh, sqrt and tan as it does exp and log: two
	// iterations leave every function far off, and the default count is within the bound. The hyperbolic
	// functions' error does not rise with more iterations. tan's can: near its pole the error of cos is
	// amplified by 1 / cos^2, and a count too short for that flips a saturated result between maxpos and
	// maxneg, so for tan the default count must instead be the best of all of them.
	template<typename Fixed>
	int VerifyIterationCountOthers(bool reportTestCases) {
		using HCfg = cordic::detail::hyperbolic_config<Fixed::nbits, Fixed::rbits, cordic::auto_guard>;
		using TCfg = cordic::detail::tan_config<Fixed::nbits, Fixed::rbits, cordic::auto_guard>;
		int fails = 0;
		std::array<double, 6> previous{};
		double bestTan = 1e300;
		std::cout << type_tag(Fixed()) << " worst sinh/cosh/tanh/atanh/sqrt/tan error against the iteration count:\n";
		// the hyperbolic and the tan counts advance together, each up to its own default
		const unsigned last = (HCfg::iterations > TCfg::R) ? HCfg::iterations : TCfg::R;
		for (unsigned n = 2; n <= last; ++n) {
			const std::array<double, 6> w = others_of(MeasureOthers<Fixed>(n, n));
			std::cout << "  n = " << std::setw(2) << n;
			for (unsigned f = 0; f < 6; ++f) std::cout << "  " << kOtherNames[f] << ' ' << std::setw(9) << w[f];
			std::cout << '\n';
			for (unsigned f = 0; f < 6; ++f) {
				const bool rising = (f != 5) && (n > 2) && (w[f] > previous[f] * 1.0001 + 0.5);
				const bool loose  = (n == 2) && !(w[f] > 2.0);
				if (rising || loose) {
					if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << ' ' << kOtherNames[f] << " n = " << n << " error " << w[f]
					                               << (rising ? " rose from " : " is already small at 2 iterations: ") << previous[f] << '\n';
					++fails;
				}
			}
			if (n < last) bestTan = std::min(bestTan, w[5]);
			previous = w;
		}
		if (previous[5] > bestTan) {
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " tan at the default count " << previous[5] << " is worse than at a shorter count " << bestTan << '\n';
			++fails;
		}
		for (unsigned f = 0; f < 6; ++f) {
			if (previous[f] > kWorst) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << ' ' << kOtherNames[f] << " at the default count: " << previous[f] << " ulp\n";
				++fails;
			}
		}
		return fails;
	}

	// the guard width sets the floor for sinh, cosh, tanh, atanh, sqrt and tan, at the default count: the
	// error does not rise with more guard bits, and none is at least 0.1 ulp worse than the default width.
	// The bounded tanh and atanh stay below 1 ulp even with none; the others exceed it.
	template<typename Fixed>
	int VerifyGuardWidthOthers(bool reportTestCases) {
		int fails = 0;
		const std::array<double, 6> g0 = others_of(MeasureOthers<Fixed, 0>(~0u, ~0u));
		const std::array<double, 6> g2 = others_of(MeasureOthers<Fixed, 2>(~0u, ~0u));
		const std::array<double, 6> g4 = others_of(MeasureOthers<Fixed, 4>(~0u, ~0u));
		const std::array<double, 6> ga = others_of(MeasureOthers<Fixed>(~0u, ~0u));
		std::cout << type_tag(Fixed()) << " worst error at the default iteration count against the guard width (g = 0, 2, 4, default):\n";
		for (unsigned f = 0; f < 6; ++f) {
			std::cout << "  " << std::setw(5) << kOtherNames[f] << ' ' << std::setw(9) << g0[f] << ' ' << std::setw(9) << g2[f]
			          << ' ' << std::setw(9) << g4[f] << ' ' << std::setw(9) << ga[f] << '\n';
			const bool bounded = (f == 2 || f == 3);
			if (!((bounded || g0[f] > 1.0) && g0[f] >= ga[f] + 0.1 && g0[f] >= g2[f] && g2[f] >= g4[f] && g4[f] >= ga[f] && ga[f] <= kWorst)) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << ' ' << kOtherNames[f] << " the error does not fall with the guard width\n";
				++fails;
			}
		}
		return fails;
	}

	// the special arguments. A result with an exact answer must be exact; the others must be within 3/4 ulp.
	template<typename Fixed>
	int VerifySpecialArguments(bool reportTestCases) {
		using R = Ref<Fixed>;
		int fails = 0;
		auto expect = [&](const Fixed& got, const Fixed& want, const char* what) {
			if (got != want) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(got) << ' ' << what << " = " << got << " expected " << want << '\n';
				++fails;
			}
		};
		auto near = [&](const Fixed& got, const R& exact, const char* what) {
			const double e = ulp_error(got, exact, true);
			if (e > 0.75) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(got) << ' ' << what << " = " << got << " is " << e << " ulp from the exact value\n";
				++fails;
			}
		};
		auto expect_errno = [&](int want, const char* what) {
			if (errno != want) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << ' ' << what << " errno = " << errno << " expected " << want << '\n';
				++fails;
			}
			errno = 0;
		};
		const Fixed zero(0), one(1), two(2), quarter(0.25), half(0.5);
		const Fixed maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg), minpos(SpecificValue::minpos);
		expect(cordic::exp(zero), one, "exp(0)");
		expect(cordic::sinh(zero), zero, "sinh(0)");
		expect(cordic::cosh(zero), one, "cosh(0)");
		expect(cordic::tanh(zero), zero, "tanh(0)");
		expect(cordic::atanh(zero), zero, "atanh(0)");
		expect(cordic::log(one), zero, "log(1)");
		expect(cordic::sqrt(zero), zero, "sqrt(0)");
		expect(cordic::tan(zero), zero, "tan(0)");
		// the perfect squares
		expect(cordic::sqrt(one), one, "sqrt(1)");
		expect(cordic::sqrt(quarter), half, "sqrt(1/4)");
		for (int k = 2; k <= 11; ++k) expect(cordic::sqrt(Fixed(k * k)), Fixed(k), "sqrt(k^2)");
		// log of the powers of two is k ln 2
		const R ln2 = log(R(2.0));
		for (int k : { -8, -1, 1, 2, 6 }) near(cordic::log(Fixed(std::ldexp(1.0, k))), R(double(k)) * ln2, "log(2^k)");
		// exp around the overflow threshold ln(maxpos): the last value that fits, and the first that saturates
		const R lnmax = log(exact_of(maxpos));
		const Fixed below(double(lnmax) - 0.01), above(double(lnmax) + 0.01);
		near(cordic::exp(below), exp(exact_of(below)), "exp(ln maxpos - 0.01)");
		expect(cordic::exp(above), maxpos, "exp(ln maxpos + 0.01)");
		expect(cordic::exp(maxpos), maxpos, "exp(maxpos)");
		expect(cordic::exp(maxneg), zero, "exp(maxneg)");
		expect(cordic::cosh(maxneg), maxpos, "cosh(maxneg)");
		expect(cordic::sinh(maxneg), maxneg, "sinh(maxneg)");
		expect(cordic::tanh(maxpos), one, "tanh(maxpos)");
		expect(cordic::tanh(maxneg), -one, "tanh(maxneg)");
		// log and sqrt at the ends of the range
		near(cordic::log(minpos), log(exact_of(minpos)), "log(minpos)");
		near(cordic::log(maxpos), lnmax, "log(maxpos)");
		near(cordic::sqrt(minpos), sqrt(exact_of(minpos)), "sqrt(minpos)");
		near(cordic::sqrt(maxpos), sqrt(exact_of(maxpos)), "sqrt(maxpos)");
		// atanh next to its poles
		const Fixed nearly_one = one - minpos;
		near(cordic::atanh(nearly_one), atanh(exact_of(nearly_one)), "atanh(1 - ulp)");
		near(cordic::atanh(-nearly_one), atanh(exact_of(-nearly_one)), "atanh(-1 + ulp)");
		// tan around its pole at pi/2, where it changes sign through +-inf, and at the ends of the range,
		// through the range reduction of the largest arguments; the unbounded results saturate
		const R pi = R(4.0) * atan(R(1.0));
		const R pi_over_2 = pi / R(2.0);
		for (const R& pole : { pi_over_2, R(-1.0) * pi_over_2, R(3.0) * pi_over_2 }) {
			// the two encodings on each side of the pole, quantized from the reference (a double would put
			// a wide type's neighbours all on one side)
			const Fixed below = floor_of<Fixed>(pole), above = below + minpos;
			if (!(exact_of(below) < pole && pole < exact_of(above))) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(below) << " the arguments do not straddle the pole\n";
				++fails;
			}
			for (const Fixed& t : { below - minpos, below, above, above + minpos }) near(cordic::tan(t), tan(exact_of(t)), "tan next to an odd multiple of pi/2");
		}
		near(cordic::tan(maxpos), tan(exact_of(maxpos)), "tan(maxpos)");
		near(cordic::tan(maxneg), tan(exact_of(maxneg)), "tan(maxneg)");
		// the domain errors: fixpnt has no NaN
		errno = 0;
		expect(cordic::log(zero), maxneg, "log(0)");
		expect_errno(ERANGE, "log(0)");
		expect(cordic::log(-one), zero, "log(-1)");
		expect_errno(EDOM, "log(-1)");
		expect(cordic::atanh(one), maxpos, "atanh(1)");
		expect_errno(ERANGE, "atanh(1)");
		expect(cordic::atanh(-one), maxneg, "atanh(-1)");
		expect_errno(ERANGE, "atanh(-1)");
		expect(cordic::atanh(two), zero, "atanh(2)");
		expect_errno(EDOM, "atanh(2)");
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
		try {
			(void)cordic::sqrt(-one);
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " sqrt(-1) did not throw\n";
			++fails;
		}
		catch (const fixpnt_negative_sqrt_arg&) {}
#else
		expect(cordic::sqrt(-one), zero, "sqrt(-1)");
		expect_errno(EDOM, "sqrt(-1)");
#endif
		return fails;
	}

}  // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixpnt hyperbolic CORDIC engine (#2041)";
	std::string test_tag    = "cordic hyperbolic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<8, 4, Saturate, std::uint8_t>>("fixpnt<8,4,sat>", reportTestCases, 0)), "fixpnt<8,4,sat>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// exhaustive: every encoding, both arithmetic behaviours, one format with a single fraction bit, one that
	// holds only [-1, 1), and one with no integer bit
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<8, 4, Modulo, std::uint8_t>>("fixpnt<8,4,mod>", reportTestCases, 0)), "fixpnt<8,4,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<8, 4, Saturate, std::uint8_t>>("fixpnt<8,4,sat>", reportTestCases, 0)), "fixpnt<8,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<8, 7, Modulo, std::uint8_t>>("fixpnt<8,7,mod>", reportTestCases, 0)), "fixpnt<8,7,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<8, 7, Saturate, std::uint8_t>>("fixpnt<8,7,sat>", reportTestCases, 0)), "fixpnt<8,7,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<8, 1, Saturate, std::uint8_t>>("fixpnt<8,1,sat>", reportTestCases, 0)), "fixpnt<8,1,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<8, 8, Modulo, std::uint8_t>>("fixpnt<8,8,mod>", reportTestCases, 0)), "fixpnt<8,8,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<8, 8, Saturate, std::uint8_t>>("fixpnt<8,8,sat>", reportTestCases, 0)), "fixpnt<8,8,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<12, 8, Modulo, std::uint8_t>>("fixpnt<12,8,mod>", reportTestCases, 0)), "fixpnt<12,8,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<12, 8, Saturate, std::uint16_t>>("fixpnt<12,8,sat>", reportTestCases, 0)), "fixpnt<12,8,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<16, 12, Saturate, std::uint16_t>>("fixpnt<16,12,sat>", reportTestCases, 0)), "fixpnt<16,12,sat>", test_tag);

	// sampled: large arguments through the range reductions, and widths past what double carries
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<32, 16, Saturate, std::uint32_t>>("fixpnt<32,16,sat>", reportTestCases, 5000)), "fixpnt<32,16,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<64, 60, Saturate, std::uint32_t>>("fixpnt<64,60,sat>", reportTestCases, 500)), "fixpnt<64,60,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<64, 32, Modulo, std::uint64_t>>("fixpnt<64,32,mod>", reportTestCases, 500)), "fixpnt<64,32,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<128, 120, Saturate, std::uint32_t>>("fixpnt<128,120,sat>", reportTestCases, 100)), "fixpnt<128,120,sat>", test_tag);
	// byte limbs in a wide type: every carry and shift crosses many block boundaries
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<72, 40, Modulo, std::uint8_t>>("fixpnt<72,40,mod,u8>", reportTestCases, 300)), "fixpnt<72,40,mod,u8>", test_tag);

	// the hardware-model knobs
	nrOfFailedTestCases += ReportTestResult((VerifyIterationCount<fixpnt<16, 12, Saturate, std::uint16_t>>(reportTestCases)), "iteration count", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyGuardWidth<fixpnt<16, 12, Saturate, std::uint16_t>>(reportTestCases)), "guard width", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyIterationCountOthers<fixpnt<12, 8, Saturate, std::uint16_t>>(reportTestCases)), "iteration count, other functions", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyGuardWidthOthers<fixpnt<12, 8, Saturate, std::uint16_t>>(reportTestCases)), "guard width, other functions", test_tag);

	// special arguments
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<16, 8, Saturate, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<16, 8, Modulo, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,mod> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<32, 24, Saturate, std::uint32_t>>(reportTestCases)), "fixpnt<32,24,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<128, 120, Saturate, std::uint32_t>>(reportTestCases)), "fixpnt<128,120,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<72, 40, Saturate, std::uint8_t>>(reportTestCases)), "fixpnt<72,40,sat,u8> special", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<16, 12, Modulo, std::uint16_t>>("fixpnt<16,12,mod>", reportTestCases, 0)), "fixpnt<16,12,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<16, 4, Saturate, std::uint16_t>>("fixpnt<16,4,sat>", reportTestCases, 0)), "fixpnt<16,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<16, 15, Modulo, std::uint16_t>>("fixpnt<16,15,mod>", reportTestCases, 0)), "fixpnt<16,15,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<64, 60, Modulo, std::uint64_t>>("fixpnt<64,60,mod>", reportTestCases, 1000)), "fixpnt<64,60,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<128, 64, Saturate, std::uint32_t>>("fixpnt<128,64,sat>", reportTestCases, 300)), "fixpnt<128,64,sat>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<32, 16, Modulo, std::uint32_t>>("fixpnt<32,16,mod>", reportTestCases, 100000)), "fixpnt<32,16,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<64, 60, Saturate, std::uint32_t>>("fixpnt<64,60,sat>", reportTestCases, 10000)), "fixpnt<64,60,sat>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult((VerifyHyperbolic<fixpnt<128, 120, Saturate, std::uint32_t>>("fixpnt<128,120,sat>", reportTestCases, 5000)), "fixpnt<128,120,sat>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
