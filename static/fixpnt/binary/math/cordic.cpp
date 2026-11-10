// cordic.cpp: the circular CORDIC engine for fixpnt: sin, cos, atan, atan2, hypot (#2015)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// cordic::sincos/sin/cos (rotation mode) and cordic::atan2/atan/hypot/polar (vectoring mode) model a
// shift-and-add CORDIC unit in the type's own arithmetic. With the default guard width and iteration
// count, every result is within 1 ulp of the exact value, at any width; the measured worst case is
// printed for each configuration.
//
// The references are independent of the engine: double for formats of at most 32 bits, whose values a
// double holds exactly and whose ulp is far above double's error, and qd (~212 bits) beyond that, with
// the fixpnt value converted to qd exactly, 32 bits at a time.
//
// The error is measured against the exact value after the type's arithmetic: Saturate clamps the exact
// value to [maxneg, maxpos], and Modulo measures the distance modulo 2^nbits, so a result that rounds up
// across maxpos and wraps (7.968 -> 8 -> -8 in fixpnt<8,4>) counts as the 1/2 ulp it is.
//
// The double-path sin/cos/atan/atan2/hypot keep their own tests in trigonometry.cpp.
#include <universal/utility/directives.hpp>
#include <algorithm>
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
	using std::sin; using std::cos; using std::atan; using std::atan2; using std::sqrt;

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

	// the error of got against the exact value e, in ulps, after the type's arithmetic
	template<typename Fixed, typename R>
	double ulp_error(const Fixed& got, R e) {
		constexpr int rbits = static_cast<int>(Fixed::rbits);
		const R g = exact_of(got);
		if constexpr (Fixed::arithmetic == Saturate) {
			const R hi = exact_of(Fixed(SpecificValue::maxpos)), lo = exact_of(Fixed(SpecificValue::maxneg));
			if (e > hi) e = hi;
			if (e < lo) e = lo;
			return double(abs(R(g - e)) * ldexp(R(1.0), rbits));
		}
		else {
			const R period = ldexp(R(1.0), static_cast<int>(Fixed::nbits) - rbits);
			R d = g - e;
			d = d - period * floor(d / period + R(0.5));     // the distance modulo 2^nbits ulps
			return double(abs(d) * ldexp(R(1.0), rbits));
		}
	}

	template<typename Fixed>
	Fixed random_fixpnt(std::mt19937_64& rng) {
		Fixed f;
		f.clear();
		for (unsigned b = 0; b < Fixed::nbits; b += 64u) {
			const std::uint64_t word = rng();
			for (unsigned k = 0; k < 64u && b + k < Fixed::nbits; ++k) f.setbit(b + k, (word >> k) & 1u);
		}
		return f;
	}

	// the reference functions on a double or a qd
	template<typename R> R ref_sin(const R& x) { return sin(x); }
	template<typename R> R ref_cos(const R& x) { return cos(x); }
	template<typename R> R ref_atan(const R& x) { return atan(x); }
	template<typename R> R ref_atan2(const R& y, const R& x) { return (x == R(0.0) && y == R(0.0)) ? R(0.0) : atan2(y, x); }
	template<typename R> R ref_hypot(const R& x, const R& y) { return sqrt(x * x + y * y); }

	struct Worst { double sin = 0, cos = 0, atan = 0, atan2 = 0, hypot = 0; };

	// sin, cos and atan of every encoding (nbits <= 16), or of samples
	template<typename Fixed>
	void MeasureUnary(Worst& w, unsigned long long samples) {
		auto one = [&](const Fixed& x) {
			const auto e = exact_of(x);
			Fixed s, c;
			cordic::sincos(x, s, c);
			w.sin  = std::max(w.sin,  ulp_error(s, ref_sin(e)));
			w.cos  = std::max(w.cos,  ulp_error(c, ref_cos(e)));
			w.atan = std::max(w.atan, ulp_error(cordic::atan(x), ref_atan(e)));
		};
		if constexpr (Fixed::nbits <= 16u) {
			if (samples == 0) {
				for (std::uint64_t b = 0; b < (1ull << Fixed::nbits); ++b) { Fixed x; x.setbits(b); one(x); }
				return;
			}
		}
		std::mt19937_64 rng(2015);
		for (unsigned long long i = 0; i < samples; ++i) one(random_fixpnt<Fixed>(rng));
	}

	// atan2 and hypot of every pair (nbits <= 8), or of sampled pairs; polar must agree with both
	template<typename Fixed>
	int MeasureBinary(Worst& w, unsigned long long samples, bool reportTestCases) {
		int fails = 0;
		auto one = [&](const Fixed& x, const Fixed& y) {
			const auto ex = exact_of(x), ey = exact_of(y);
			const Fixed t = cordic::atan2(y, x), h = cordic::hypot(x, y);
			Fixed r, theta;
			cordic::polar(x, y, r, theta);
			if (r != h || theta != t) {
				if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(x) << " polar(" << x << ", " << y << ") disagrees with hypot/atan2\n";
				++fails;
			}
			w.atan2 = std::max(w.atan2, ulp_error(t, ref_atan2(ey, ex)));
			w.hypot = std::max(w.hypot, ulp_error(h, ref_hypot(ex, ey)));
		};
		if constexpr (Fixed::nbits <= 8u) {
			if (samples == 0) {
				for (std::uint64_t a = 0; a < (1ull << Fixed::nbits); ++a) {
					for (std::uint64_t b = 0; b < (1ull << Fixed::nbits); ++b) {
						Fixed x, y;
						x.setbits(a);
						y.setbits(b);
						one(x, y);
					}
				}
				return fails;
			}
		}
		std::mt19937_64 rng(2016);
		for (unsigned long long i = 0; i < samples; ++i) {
			const Fixed x = random_fixpnt<Fixed>(rng), y = random_fixpnt<Fixed>(rng);
			one(x, y);
		}
		return fails;
	}

	// every function within 1 ulp at the default guard width and iteration count
	template<typename Fixed>
	int VerifyCordic(const std::string& tag, bool reportTestCases, unsigned long long unarySamples, unsigned long long binarySamples) {
		Worst w;
		MeasureUnary<Fixed>(w, unarySamples);
		int fails = MeasureBinary<Fixed>(w, binarySamples, reportTestCases);
		std::cout << std::left << std::setw(30) << tag << std::fixed << std::setprecision(3)
		          << " worst ulp: sin " << w.sin << "  cos " << w.cos << "  atan " << w.atan
		          << "  atan2 " << w.atan2 << "  hypot " << w.hypot
		          << "   (g = " << cordic::guard_bits<Fixed> << ", n = " << cordic::default_iterations<Fixed> << ")\n";
		std::cout.unsetf(std::ios::fixed);
		for (double e : { w.sin, w.cos, w.atan, w.atan2, w.hypot }) {
			if (e > 1.0) {
				if (reportTestCases) std::cerr << "FAIL: " << tag << " error " << e << " ulp exceeds 1 ulp\n";
				++fails;
			}
		}
		return fails;
	}

	// the error falls about one bit per iteration, until the guard bits run out
	template<typename Fixed>
	int VerifyIterationCount(bool reportTestCases) {
		constexpr unsigned rbits = Fixed::rbits;
		int fails = 0;
		double previous = 0.0;
		std::cout << type_tag(Fixed()) << " worst sin/atan error against the iteration count:\n";
		for (unsigned n = 2; n <= rbits; n += 2) {
			double ws = 0.0, wa = 0.0;
			for (std::uint64_t b = 0; b < (1ull << Fixed::nbits); ++b) {
				Fixed x;
				x.setbits(b);
				const double e = double(x);
				ws = std::max(ws, ulp_error(cordic::sin(x, n), std::sin(e)));
				wa = std::max(wa, ulp_error(cordic::atan(x, n), std::atan(e)));
			}
			std::cout << "  n = " << std::setw(2) << n << "  sin " << std::setw(10) << ws << "  atan " << std::setw(10) << wa << '\n';
			// the residual angle after n iterations is at most atan(2^-(n-1)): 2^(rbits - n + 1) ulps
			const double bound = std::ldexp(1.0, static_cast<int>(rbits - n + 1u)) + 1.0;
			const double floor_ = std::ldexp(1.0, static_cast<int>(rbits) - static_cast<int>(n) - 2);
			const double worst = std::max(ws, wa);
			if (worst > bound || worst < floor_ || (n > 2 && worst >= previous)) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " n = " << n << " error " << worst << " outside [" << floor_ << ", " << bound << "] or not falling\n";
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
		auto worst_sin = [](auto guardTag) {
			constexpr unsigned g = decltype(guardTag)::value;
			double w = 0.0;
			for (std::uint64_t b = 0; b < (1ull << Fixed::nbits); ++b) {
				Fixed x;
				x.setbits(b);
				w = std::max(w, ulp_error(cordic::sin<g>(x), std::sin(double(x))));
			}
			return w;
		};
		const double g0 = worst_sin(std::integral_constant<unsigned, 0>{});
		const double g2 = worst_sin(std::integral_constant<unsigned, 2>{});
		const double g4 = worst_sin(std::integral_constant<unsigned, 4>{});
		const double ga = worst_sin(std::integral_constant<unsigned, cordic::auto_guard>{});
		std::cout << type_tag(Fixed()) << " worst sin error at the default iteration count: g = 0: " << g0
		          << "  g = 2: " << g2 << "  g = 4: " << g4 << "  g = " << cordic::guard_bits<Fixed> << ": " << ga << '\n';
		if (!(g0 > g2 && g2 > g4 && g4 >= ga && g0 > 1.0 && ga <= 1.0)) {
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " the error does not fall with the guard width\n";
			++fails;
		}
		return fails;
	}

	// the special arguments: 0, the quadrant boundaries, the axes, (0, 0), hypot at the top of the range.
	// A result with an exact answer must be exact; the others must be within 3/4 ulp of the exact value.
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
			const double e = ulp_error(got, exact);
			if (e > 0.75) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(got) << ' ' << what << " = " << got << " is " << e << " ulp from the exact value\n";
				++fails;
			}
		};
		const R pi = R(4.0) * atan(R(1.0));
		const Fixed zero(0), one(1), two(2), three(3), four(4), five(5);
		const Fixed maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
		Fixed s, c;
		cordic::sincos(zero, s, c);
		expect(s, zero, "sin(0)");
		expect(c, one, "cos(0)");
		for (double k : { 0.25, 0.5, 0.75, 1.0, -0.5, -1.0 }) {        // k pi: the quadrant boundaries
			const Fixed theta(double(pi) * k);                      // the nearest fixpnt, as an argument
			cordic::sincos(theta, s, c);
			near(s, ref_sin(exact_of(theta)), "sin(k pi)");
			near(c, ref_cos(exact_of(theta)), "cos(k pi)");
		}
		expect(cordic::atan(zero), zero, "atan(0)");
		near(cordic::atan(one), pi / R(4.0), "atan(1)");
		near(cordic::atan(-one), -pi / R(4.0), "atan(-1)");
		// atan2 on the four axes, the diagonals and at the origin
		expect(cordic::atan2(zero, two), zero, "atan2(0, 2)");
		near(cordic::atan2(two, zero), pi / R(2.0), "atan2(2, 0)");
		near(cordic::atan2(zero, -two), pi, "atan2(0, -2)");
		near(cordic::atan2(-two, zero), -pi / R(2.0), "atan2(-2, 0)");
		expect(cordic::atan2(zero, zero), zero, "atan2(0, 0)");
		near(cordic::atan2(two, two), pi / R(4.0), "atan2(2, 2)");
		near(cordic::atan2(-two, -two), R(-3.0) * pi / R(4.0), "atan2(-2, -2)");
		near(cordic::atan2(maxneg, maxneg), R(-3.0) * pi / R(4.0), "atan2(maxneg, maxneg)");
		// hypot
		expect(cordic::hypot(zero, zero), zero, "hypot(0, 0)");
		expect(cordic::hypot(three, four), five, "hypot(3, 4)");
		expect(cordic::hypot(-three, -four), five, "hypot(-3, -4)");
		expect(cordic::hypot(maxpos, zero), maxpos, "hypot(maxpos, 0)");
		expect(cordic::hypot(zero, maxneg), maxpos, "hypot(0, maxneg)");         // |maxneg| = 2^(ib-1) saturates
		expect(cordic::hypot(maxpos, maxpos), maxpos, "hypot(maxpos, maxpos)");   // the K growth, then saturation
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

	std::string test_suite  = "fixpnt circular CORDIC engine (#2015)";
	std::string test_tag    = "cordic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<8, 4, Saturate, std::uint8_t>>("fixpnt<8,4,sat>", reportTestCases, 0, 0)), "fixpnt<8,4,sat>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// exhaustive: every encoding, and every pair for the 8-bit formats; both arithmetic behaviours, a format
	// that holds only [-1, 1) (no range reduction), one with a single fraction bit, and one with no integer bit
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<8, 4, Modulo, std::uint8_t>>("fixpnt<8,4,mod>", reportTestCases, 0, 0)), "fixpnt<8,4,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<8, 4, Saturate, std::uint8_t>>("fixpnt<8,4,sat>", reportTestCases, 0, 0)), "fixpnt<8,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<8, 7, Modulo, std::uint8_t>>("fixpnt<8,7,mod>", reportTestCases, 0, 0)), "fixpnt<8,7,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<8, 7, Saturate, std::uint8_t>>("fixpnt<8,7,sat>", reportTestCases, 0, 0)), "fixpnt<8,7,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<8, 1, Saturate, std::uint8_t>>("fixpnt<8,1,sat>", reportTestCases, 0, 0)), "fixpnt<8,1,sat>", test_tag);
	// no integer bit: [-1/2, 1/2), where atan(x) needs 1 = 2^nbits in the datapath
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<8, 8, Modulo, std::uint8_t>>("fixpnt<8,8,mod>", reportTestCases, 0, 0)), "fixpnt<8,8,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<8, 8, Saturate, std::uint8_t>>("fixpnt<8,8,sat>", reportTestCases, 0, 0)), "fixpnt<8,8,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<12, 8, Modulo, std::uint8_t>>("fixpnt<12,8,mod>", reportTestCases, 0, 20000)), "fixpnt<12,8,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<12, 8, Saturate, std::uint16_t>>("fixpnt<12,8,sat>", reportTestCases, 0, 20000)), "fixpnt<12,8,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<16, 12, Modulo, std::uint16_t>>("fixpnt<16,12,mod>", reportTestCases, 0, 20000)), "fixpnt<16,12,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<16, 12, Saturate, std::uint8_t>>("fixpnt<16,12,sat>", reportTestCases, 0, 20000)), "fixpnt<16,12,sat>", test_tag);

	// sampled: large arguments through the range reduction, and widths past what double carries
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<32, 16, Saturate, std::uint32_t>>("fixpnt<32,16,sat>", reportTestCases, 5000, 5000)), "fixpnt<32,16,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<64, 60, Saturate, std::uint32_t>>("fixpnt<64,60,sat>", reportTestCases, 1000, 1000)), "fixpnt<64,60,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<64, 60, Modulo, std::uint64_t>>("fixpnt<64,60,mod>", reportTestCases, 500, 500)), "fixpnt<64,60,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<64, 32, Saturate, std::uint32_t>>("fixpnt<64,32,sat>", reportTestCases, 500, 500)), "fixpnt<64,32,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<128, 120, Saturate, std::uint32_t>>("fixpnt<128,120,sat>", reportTestCases, 200, 200)), "fixpnt<128,120,sat>", test_tag);

	// the hardware-model knobs
	nrOfFailedTestCases += ReportTestResult((VerifyIterationCount<fixpnt<16, 12, Saturate, std::uint16_t>>(reportTestCases)), "iteration count", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyGuardWidth<fixpnt<16, 12, Saturate, std::uint16_t>>(reportTestCases)), "guard width", test_tag);

	// special arguments
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<16, 8, Saturate, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<32, 24, Saturate, std::uint32_t>>(reportTestCases)), "fixpnt<32,24,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<128, 120, Saturate, std::uint32_t>>(reportTestCases)), "fixpnt<128,120,sat> special", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<16, 4, Saturate, std::uint16_t>>("fixpnt<16,4,sat>", reportTestCases, 0, 50000)), "fixpnt<16,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<16, 15, Modulo, std::uint16_t>>("fixpnt<16,15,mod>", reportTestCases, 0, 50000)), "fixpnt<16,15,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<128, 64, Saturate, std::uint32_t>>("fixpnt<128,64,sat>", reportTestCases, 1000, 1000)), "fixpnt<128,64,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<128, 120, Modulo, std::uint64_t>>("fixpnt<128,120,mod>", reportTestCases, 1000, 1000)), "fixpnt<128,120,mod>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<12, 8, Saturate, std::uint8_t>>("fixpnt<12,8,sat> pairs", reportTestCases, 0, 1000000)), "fixpnt<12,8,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<64, 60, Saturate, std::uint32_t>>("fixpnt<64,60,sat>", reportTestCases, 20000, 20000)), "fixpnt<64,60,sat>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<fixpnt<128, 120, Saturate, std::uint32_t>>("fixpnt<128,120,sat>", reportTestCases, 20000, 20000)), "fixpnt<128,120,sat>", test_tag);
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
