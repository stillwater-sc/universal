// cordic.cpp: the decimal CORDIC engine for dfixpnt: sin, cos, atan, atan2, hypot (#2042)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// cordic::sincos/sin/cos (rotation mode) and cordic::atan2/atan/hypot/polar (vectoring mode) model a
// decimal shift-and-add CORDIC unit in dfixpnt's own decimal arithmetic. With the default guard digits and
// iteration count, every result is within 1 ulp (one unit in the last decimal digit, 10^-radix) of the exact
// value, at any width; the measured worst case is printed for each configuration.
//
// The references are independent of the engine: double for formats of at most 12 digits, whose values a
// double holds to far better than their ulp, and qd (~62 decimal digits) beyond that, with the dfixpnt
// value converted to qd from its digits, 15 at a time.
//
// The error is measured against the exact value after the type's arithmetic: Saturate clamps the exact
// value to [maxneg, maxpos], and Modulo measures the distance modulo 10^idigits, so a result that rounds up
// across maxpos and wraps (0.99996 -> 1.0000 -> 0.0000 in dfixpnt<4,4>) counts as the 0.4 ulp it is.
//
// The engine works on magnitudes and applies the signs last, so sin and atan2 must be exactly odd and cos
// exactly even, and it reads and writes digits, so BCD, BID and DPD must give identical results.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;
	// the double overloads, ahead of everything: unqualified, abs(double) would find an integer abs
	using std::abs; using std::floor; using std::pow;
	using std::sin; using std::cos; using std::atan; using std::atan2; using std::sqrt;

	// the exact value of a dfixpnt as a qd: its digits as an integer, 15 at a time, over 10^radix
	template<typename Fixed>
	qd to_qd(const Fixed& f) {
		qd N(0.0);
		unsigned i = Fixed::ndigits;
		while (i > 0) {
			const unsigned take = (i >= 15u) ? 15u : i;
			double chunk = 0.0;
			for (unsigned k = 0; k < take; ++k) chunk = chunk * 10.0 + static_cast<double>(f.digit(i - 1u - k));
			qd scale(1.0);
			for (unsigned k = 0; k < take; ++k) scale *= qd(10.0);
			N = N * scale + qd(chunk);
			i -= take;
		}
		qd p(1.0);
		for (unsigned k = 0; k < Fixed::radix; ++k) p *= qd(10.0);
		const qd v = N / p;
		return f.sign() ? -v : v;
	}

	// the reference value type: double for formats of at most 12 digits, qd beyond
	template<typename Fixed>
	using Ref = std::conditional_t<(Fixed::ndigits <= 12u), double, qd>;

	template<typename Fixed>
	Ref<Fixed> exact_of(const Fixed& f) {
		if constexpr (std::is_same_v<Ref<Fixed>, double>) return double(f); else return to_qd(f);
	}

	template<typename R>
	R pow10(int e) {
		R r(1.0);
		for (int k = 0; k < (e < 0 ? -e : e); ++k) r *= R(10.0);
		return (e < 0) ? R(1.0) / r : r;
	}

	// the error of got against the exact value e, in ulps, after the type's arithmetic
	template<typename Fixed, typename R>
	double ulp_error(const Fixed& got, R e) {
		const R ulps = pow10<R>(static_cast<int>(Fixed::radix));
		const R g = exact_of(got);
		if constexpr (Fixed::arithmetic == Saturate) {
			const R hi = exact_of(Fixed(SpecificValue::maxpos)), lo = exact_of(Fixed(SpecificValue::maxneg));
			if (e > hi) e = hi;
			if (e < lo) e = lo;
			return double(abs(R(g - e)) * ulps);
		}
		else {
			const R period = pow10<R>(static_cast<int>(Fixed::idigits));
			R d = g - e;
			d = d - period * floor(d / period + R(0.5));     // the distance modulo 10^idigits
			return double(abs(d) * ulps);
		}
	}

	// a random value: random digits, with a random number of the top ones zeroed, so the magnitudes are
	// spread over every decade, and a random sign
	template<typename Fixed>
	Fixed random_dfixpnt(std::mt19937_64& rng) {
		Fixed f(0);
		const unsigned top = static_cast<unsigned>(rng() % (Fixed::ndigits + 1u));
		for (unsigned i = 0; i < top; ++i) f.setdigit(i, static_cast<unsigned>(rng() % 10u));
		if (!f.iszero() && (rng() & 1u)) f.setsign(true);
		return f;
	}

	// the k-th value of an exhaustive sweep: k = 2 m + s for the magnitude m and the sign s (no -0)
	template<typename Fixed>
	Fixed nth_dfixpnt(std::uint64_t k) {
		Fixed f(0);
		std::uint64_t m = k / 2u;
		for (unsigned i = 0; i < Fixed::ndigits; ++i) { f.setdigit(i, static_cast<unsigned>(m % 10u)); m /= 10u; }
		if ((k & 1u) && !f.iszero()) f.setsign(true);
		return f;
	}
	template<typename Fixed>
	constexpr std::uint64_t encodings_of() {
		std::uint64_t n = 2;
		for (unsigned i = 0; i < Fixed::ndigits; ++i) n *= 10u;
		return n;
	}

	// the reference functions on a double or a qd
	template<typename R> R ref_atan2(const R& y, const R& x) { return (x == R(0.0) && y == R(0.0)) ? R(0.0) : atan2(y, x); }
	template<typename R> R ref_hypot(const R& x, const R& y) { return sqrt(x * x + y * y); }

	struct Worst { double sin = 0, cos = 0, atan = 0, atan2 = 0, hypot = 0; };

	// sin, cos and atan of every value (ndigits <= 6), or of samples; sin and atan must be odd, cos even
	template<typename Fixed>
	int MeasureUnary(Worst& w, unsigned long long samples, bool reportTestCases) {
		int fails = 0;
		auto one = [&](const Fixed& x) {
			const auto e = exact_of(x);
			Fixed s, c;
			cordic::sincos(x, s, c);
			const Fixed a = cordic::atan(x);
			Fixed sn, cn;
			cordic::sincos(-x, sn, cn);
			if (sn != -s || cn != c || cordic::atan(-x) != -a) {
				if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(x) << " symmetry broken at " << x << '\n';
				++fails;
			}
			w.sin  = std::max(w.sin,  ulp_error(s, sin(e)));
			w.cos  = std::max(w.cos,  ulp_error(c, cos(e)));
			w.atan = std::max(w.atan, ulp_error(a, atan(e)));
		};
		if (samples == 0) {
			for (std::uint64_t k = 0; k < encodings_of<Fixed>(); ++k) one(nth_dfixpnt<Fixed>(k));
			return fails;
		}
		std::mt19937_64 rng(2042);
		for (unsigned long long i = 0; i < samples; ++i) one(random_dfixpnt<Fixed>(rng));
		return fails;
	}

	// atan2 and hypot of every pair (ndigits <= 2), or of sampled pairs; polar must agree with both, and
	// atan2 must be odd in y (except on the negative x axis: dfixpnt has no -0, so atan2(-0, -x) is pi)
	template<typename Fixed>
	int MeasureBinary(Worst& w, unsigned long long samples, bool reportTestCases) {
		int fails = 0;
		auto one = [&](const Fixed& x, const Fixed& y) {
			const auto ex = exact_of(x), ey = exact_of(y);
			const Fixed t = cordic::atan2(y, x), h = cordic::hypot(x, y);
			Fixed r, theta;
			cordic::polar(x, y, r, theta);
			if (r != h || theta != t || (!y.iszero() && cordic::atan2(-y, x) != -t)) {
				if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(x) << " polar/atan2 disagree or atan2 not odd at (" << x << ", " << y << ")\n";
				++fails;
			}
			w.atan2 = std::max(w.atan2, ulp_error(t, ref_atan2(ey, ex)));
			w.hypot = std::max(w.hypot, ulp_error(h, ref_hypot(ex, ey)));
		};
		if (samples == 0) {
			for (std::uint64_t a = 0; a < encodings_of<Fixed>(); ++a) {
				for (std::uint64_t b = 0; b < encodings_of<Fixed>(); ++b) one(nth_dfixpnt<Fixed>(a), nth_dfixpnt<Fixed>(b));
			}
			return fails;
		}
		std::mt19937_64 rng(2043);
		for (unsigned long long i = 0; i < samples; ++i) {
			const Fixed x = random_dfixpnt<Fixed>(rng), y = random_dfixpnt<Fixed>(rng);
			one(x, y);
		}
		return fails;
	}

	// every function within 1 ulp at the default guard digits and iteration count
	template<typename Fixed>
	int VerifyCordic(const std::string& tag, bool reportTestCases, unsigned long long unarySamples, unsigned long long binarySamples) {
		Worst w;
		int fails = MeasureUnary<Fixed>(w, unarySamples, reportTestCases);
		fails += MeasureBinary<Fixed>(w, binarySamples, reportTestCases);
		std::cout << std::left << std::setw(24) << tag << std::fixed << std::setprecision(3)
		          << " worst ulp: sin " << w.sin << "  cos " << w.cos << "  atan " << w.atan
		          << "  atan2 " << w.atan2 << "  hypot " << w.hypot
		          << "   (g = " << cordic::decimal_guard_digits<Fixed> << ", n = " << cordic::decimal_rotation_iterations<Fixed>
		          << " / " << cordic::decimal_vectoring_iterations<Fixed> << ")\n";
		std::cout.unsetf(std::ios::fixed);
		for (double e : { w.sin, w.cos, w.atan, w.atan2, w.hypot }) {
			if (e > 1.0) {
				if (reportTestCases) std::cerr << "FAIL: " << tag << " error " << e << " ulp exceeds 1 ulp\n";
				++fails;
			}
		}
		return fails;
	}

	// BCD, BID and DPD give identical results: the engine reads and writes digits
	template<unsigned nd, unsigned rx, bool arith>
	int VerifyEncodings(bool reportTestCases, unsigned long long samples) {
		using B = dfixpnt<nd, rx, DecimalEncoding::BCD, arith>;
		using I = dfixpnt<nd, rx, DecimalEncoding::BID, arith>;
		using D = dfixpnt<nd, rx, DecimalEncoding::DPD, arith>;
		int fails = 0;
		auto same = [](const auto& a, const auto& b, const auto& c) {
			for (unsigned i = 0; i < nd; ++i) if (a.digit(i) != b.digit(i) || a.digit(i) != c.digit(i)) return false;
			return a.sign() == b.sign() && a.sign() == c.sign();
		};
		std::mt19937_64 rng(2044);
		for (unsigned long long k = 0; k < samples; ++k) {
			const B x = random_dfixpnt<B>(rng), y = random_dfixpnt<B>(rng);
			I xi, yi;
			D xd, yd;
			xi.assign(x.to_string()); yi.assign(y.to_string());
			xd.assign(x.to_string()); yd.assign(y.to_string());
			B sb, cb; I si, ci; D sd, cd;
			cordic::sincos(x, sb, cb); cordic::sincos(xi, si, ci); cordic::sincos(xd, sd, cd);
			const bool ok = same(sb, si, sd) && same(cb, ci, cd)
			             && same(cordic::atan2(y, x), cordic::atan2(yi, xi), cordic::atan2(yd, xd))
			             && same(cordic::hypot(x, y), cordic::hypot(xi, yi), cordic::hypot(xd, yd))
			             && same(cordic::atan(x), cordic::atan(xi), cordic::atan(xd));
			if (!ok) {
				if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(x) << " the encodings disagree at (" << x << ", " << y << ")\n";
				++fails;
			}
		}
		return fails;
	}

	// the error falls about one decimal digit per digit position, until the guard digits run out
	template<typename Fixed>
	int VerifyIterationCount(bool reportTestCases) {
		constexpr unsigned radix = Fixed::radix;
		int fails = 0;
		double previous = 0.0;
		std::cout << type_tag(Fixed()) << " worst sin/atan error against the digit positions:\n";
		for (unsigned n = 1; n <= radix + 1u; ++n) {
			double ws = 0.0, wa = 0.0;
			for (std::uint64_t k = 0; k < encodings_of<Fixed>(); ++k) {
				const Fixed x = nth_dfixpnt<Fixed>(k);
				const double e = double(x);
				ws = std::max(ws, ulp_error(cordic::sin(x, n), std::sin(e)));
				wa = std::max(wa, ulp_error(cordic::atan(x, n), std::atan(e)));
			}
			std::cout << "  n = " << n << "  sin " << std::setw(10) << ws << "  atan " << std::setw(10) << wa << '\n';
			// the angle left after n positions is at most atan(10^-(n-1)): 10^(radix - n + 1) ulps
			const double worst = std::max(ws, wa);
			const double bound = std::pow(10.0, static_cast<double>(radix) - static_cast<double>(n) + 1.0) + 1.0;
			if (worst > bound || (n > 1 && n < radix && worst > previous / 5.0)) {
				if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " n = " << n << " error " << worst << " above " << bound << " or not falling a digit\n";
				++fails;
			}
			previous = worst;
		}
		return fails;
	}

	// the guard digits set the floor: with too few the truncation error shows
	template<typename Fixed>
	int VerifyGuardWidth(bool reportTestCases) {
		int fails = 0;
		auto worst = [](auto guardTag) {
			using G = decltype(guardTag);
			double w = 0.0;
			for (std::uint64_t k = 0; k < encodings_of<Fixed>(); ++k) {
				const Fixed x = nth_dfixpnt<Fixed>(k);
				const double e = double(x);
				w = std::max(w, ulp_error(cordic::sin<G::value>(x), std::sin(e)));
				w = std::max(w, ulp_error(cordic::atan<G::value>(x), std::atan(e)));
			}
			return w;
		};
		const double g0 = worst(std::integral_constant<unsigned, 0>{});
		const double g1 = worst(std::integral_constant<unsigned, 1>{});
		const double g2 = worst(std::integral_constant<unsigned, 2>{});
		const double ga = worst(std::integral_constant<unsigned, cordic::auto_guard>{});
		std::cout << type_tag(Fixed()) << " worst sin/atan error at the default iteration count: g = 0: " << g0
		          << "  g = 1: " << g1 << "  g = 2: " << g2 << "  g = " << cordic::decimal_guard_digits<Fixed> << ": " << ga << '\n';
		if (!(g0 > g1 && g1 > g2 && g2 >= ga && g0 > 1.0 && ga <= 1.0)) {
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " the error does not fall with the guard digits\n";
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
		for (double k : { 0.25, 0.5, 0.75, 1.0, -0.5, -1.0, 2.0 }) {     // k pi: the quadrant boundaries
			const Fixed theta(double(pi) * k);                      // the nearest dfixpnt, as an argument
			cordic::sincos(theta, s, c);
			near(s, sin(exact_of(theta)), "sin(k pi)");
			near(c, cos(exact_of(theta)), "cos(k pi)");
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
		expect(cordic::hypot(zero, maxneg), maxpos, "hypot(0, maxneg)");
		near(cordic::hypot(maxpos, maxpos), exact_of(maxpos) * sqrt(R(2.0)), "hypot(maxpos, maxpos)");   // saturates or wraps
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

	std::string test_suite  = "dfixpnt decimal CORDIC engine (#2042)";
	std::string test_tag    = "cordic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	constexpr auto BCD = DecimalEncoding::BCD;
	constexpr auto BID = DecimalEncoding::BID;
	constexpr auto DPD = DecimalEncoding::DPD;

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<4, 3, BCD, Saturate>>("dfixpnt<4,3,sat>", reportTestCases, 0, 20000)), "dfixpnt<4,3,sat>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// exhaustive: every value, and every pair for the 2-digit formats; both arithmetic behaviours, the three
	// encodings, a format that holds only (-1, 1) (no range reduction), and an integer format
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<2, 1, BCD, Saturate>>("dfixpnt<2,1,sat>", reportTestCases, 0, 0)), "dfixpnt<2,1,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<2, 2, BCD, Modulo>>("dfixpnt<2,2,mod>", reportTestCases, 0, 0)), "dfixpnt<2,2,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<3, 2, BCD, Modulo>>("dfixpnt<3,2,mod>", reportTestCases, 0, 20000)), "dfixpnt<3,2,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<3, 2, DPD, Saturate>>("dfixpnt<3,2,sat,dpd>", reportTestCases, 0, 20000)), "dfixpnt<3,2,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<4, 3, BID, Modulo>>("dfixpnt<4,3,mod,bid>", reportTestCases, 0, 20000)), "dfixpnt<4,3,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<4, 3, BCD, Saturate>>("dfixpnt<4,3,sat>", reportTestCases, 0, 20000)), "dfixpnt<4,3,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<4, 4, BCD, Modulo>>("dfixpnt<4,4,mod>", reportTestCases, 0, 20000)), "dfixpnt<4,4,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<4, 4, DPD, Saturate>>("dfixpnt<4,4,sat,dpd>", reportTestCases, 0, 20000)), "dfixpnt<4,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<4, 1, BCD, Saturate>>("dfixpnt<4,1,sat>", reportTestCases, 0, 20000)), "dfixpnt<4,1,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<3, 0, BCD, Saturate>>("dfixpnt<3,0,sat>", reportTestCases, 0, 20000)), "dfixpnt<3,0,sat>", test_tag);

	// sampled: large arguments through the range reduction, and widths past what double carries
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<8, 6, BCD, Saturate>>("dfixpnt<8,6,sat>", reportTestCases, 20000, 20000)), "dfixpnt<8,6,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<12, 4, BCD, Modulo>>("dfixpnt<12,4,mod>", reportTestCases, 10000, 10000)), "dfixpnt<12,4,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<20, 18, BCD, Saturate>>("dfixpnt<20,18,sat>", reportTestCases, 1000, 1000)), "dfixpnt<20,18,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<34, 30, BCD, Modulo>>("dfixpnt<34,30,mod>", reportTestCases, 300, 300)), "dfixpnt<34,30,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<50, 40, BCD, Saturate>>("dfixpnt<50,40,sat>", reportTestCases, 100, 100)), "dfixpnt<50,40,sat>", test_tag);

	// the encodings agree digit for digit
	nrOfFailedTestCases += ReportTestResult((VerifyEncodings<6, 4, Saturate>(reportTestCases, 2000)), "encodings <6,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyEncodings<8, 2, Modulo>(reportTestCases, 2000)), "encodings <8,2,mod>", test_tag);

	// the hardware-model knobs
	nrOfFailedTestCases += ReportTestResult((VerifyIterationCount<dfixpnt<4, 4, BCD, Saturate>>(reportTestCases)), "iteration count", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyGuardWidth<dfixpnt<4, 3, BCD, Saturate>>(reportTestCases)), "guard width", test_tag);

	// special arguments
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<dfixpnt<8, 4, BCD, Saturate>>(reportTestCases)), "dfixpnt<8,4,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<dfixpnt<12, 9, DPD, Saturate>>(reportTestCases)), "dfixpnt<12,9,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<dfixpnt<34, 30, BCD, Saturate>>(reportTestCases)), "dfixpnt<34,30,sat> special", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<5, 4, BCD, Saturate>>("dfixpnt<5,4,sat>", reportTestCases, 0, 50000)), "dfixpnt<5,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<5, 2, BID, Modulo>>("dfixpnt<5,2,mod,bid>", reportTestCases, 0, 50000)), "dfixpnt<5,2,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<50, 40, BCD, Modulo>>("dfixpnt<50,40,mod>", reportTestCases, 300, 300)), "dfixpnt<50,40,mod>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<6, 4, BCD, Saturate>>("dfixpnt<6,4,sat>", reportTestCases, 0, 200000)), "dfixpnt<6,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<20, 18, BCD, Modulo>>("dfixpnt<20,18,mod>", reportTestCases, 10000, 10000)), "dfixpnt<20,18,mod>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult((VerifyCordic<dfixpnt<34, 30, BCD, Saturate>>("dfixpnt<34,30,sat>", reportTestCases, 5000, 5000)), "dfixpnt<34,30,sat>", test_tag);
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
