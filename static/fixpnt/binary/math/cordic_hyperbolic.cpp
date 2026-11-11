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
// 1 ulp of the exact value, at any width; the measured worst case is printed for each configuration.
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
	using std::atanh; using std::tan;

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

	// every function within 1 ulp at the default guard width and iteration count
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
			if (e > 1.0) {
				if (reportTestCases) std::cerr << "FAIL: " << tag << " error " << e << " ulp exceeds 1 ulp\n";
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
		if (!(g0 > g2 && g2 > g4 && g4 >= ga && g0 > 1.0 && ga <= 1.0)) {
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " the error does not fall with the guard width\n";
			++fails;
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

	// the hardware-model knobs
	nrOfFailedTestCases += ReportTestResult((VerifyIterationCount<fixpnt<16, 12, Saturate, std::uint16_t>>(reportTestCases)), "iteration count", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyGuardWidth<fixpnt<16, 12, Saturate, std::uint16_t>>(reportTestCases)), "guard width", test_tag);

	// special arguments
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<16, 8, Saturate, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<16, 8, Modulo, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,mod> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<32, 24, Saturate, std::uint32_t>>(reportTestCases)), "fixpnt<32,24,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<128, 120, Saturate, std::uint32_t>>(reportTestCases)), "fixpnt<128,120,sat> special", test_tag);
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
