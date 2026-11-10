// cordic_linear.cpp: the linear CORDIC engine for fixpnt: multiplication and division by shift and add (#2040)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// cordic::mul (linear rotation mode) and cordic::div (linear vectoring mode) model a shift-and-add CORDIC
// unit in the type's own arithmetic. With the default guard width and iteration count, every result is
// within 1 ulp of the exact value, at any width; the measured worst case is printed for each configuration.
//
// The reference is exact and independent of the engine and of fixpnt's own operator* and operator/: the
// exact result is the rational N / D in raw units (x * z: N = X Z, D = 2^rbits; y / x: N = Y 2^rbits,
// D = |X|, with X, Y, Z the raw integers), and the error of a raw result C is |C D - N| / D ulps, computed
// in integer<> arithmetic. Saturate measures against the exact value clamped to [maxneg, maxpos]; Modulo
// measures modulo 2^nbits, so a wrapped result counts the distance to the wrapped exact value.
//
// The circular engine (sin, cos, atan2, hypot) has its own tests in cordic.cpp.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;
	using std::abs;

	// an integer wide enough for C D, N and 2^nbits D of every configuration under test
	template<typename Fixed>
	using Wide = integer<3u * Fixed::nbits + 8u, std::uint32_t>;

	// the raw value of a fixpnt as a signed integer
	template<typename Fixed>
	Wide<Fixed> raw_of(const Fixed& f) {
		auto raw = f.bits();
		const bool negative = raw.isneg();
		if (negative) raw.twosComplement();             // maxneg stays maxneg, whose unsigned reading is its magnitude
		Wide<Fixed> sum(0);
		for (unsigned c = 0; c < Fixed::nbits; c += 32u) {
			std::uint64_t chunk = 0;
			for (unsigned i = 0; i < 32u && c + i < Fixed::nbits; ++i) if (raw.at(c + i)) chunk |= (1ull << i);
			Wide<Fixed> t(static_cast<long long>(chunk));
			t <<= static_cast<int>(c);
			sum += t;
		}
		return negative ? -sum : sum;
	}

	template<typename Fixed>
	Wide<Fixed> pow2(unsigned e) {
		Wide<Fixed> p(1);
		p <<= static_cast<int>(e);
		return p;
	}

	// the error, in ulps, of a result whose exact raw value is N / D, with D > 0
	template<typename Fixed>
	double ulp_error(const Fixed& got, const Wide<Fixed>& N, const Wide<Fixed>& D) {
		using W = Wide<Fixed>;
		const W C = raw_of(got);
		W r;
		if constexpr (Fixed::arithmetic == Saturate) {
			const W hi = raw_of(Fixed(SpecificValue::maxpos)), lo = raw_of(Fixed(SpecificValue::maxneg));
			if (N > hi * D)      r = C - hi;               // the exact value clamps to maxpos: measure in whole ulps
			else if (N < lo * D) r = C - lo;
			else return std::abs(double(C * D - N)) / double(D);
			return std::abs(double(r));
		}
		else {
			const W M = pow2<Fixed>(Fixed::nbits) * D;      // 2^nbits ulps, in units of 1/D
			r = (C * D - N) % M;                             // truncates toward zero: |r| < M
			const W half = M / W(2);
			if (r > half) r -= M;
			if (r < -half) r += M;
			return std::abs(double(r)) / double(D);
		}
	}

	template<typename Fixed>
	double mul_error(const Fixed& got, const Fixed& x, const Fixed& z) {
		return ulp_error(got, raw_of(x) * raw_of(z), pow2<Fixed>(Fixed::rbits));
	}

	template<typename Fixed>
	double div_error(const Fixed& got, const Fixed& y, const Fixed& x) {
		using W = Wide<Fixed>;
		const W X = raw_of(x);
		W N = raw_of(y) * pow2<Fixed>(Fixed::rbits);
		if (X < W(0)) N = -N;
		return ulp_error(got, N, X < W(0) ? -X : X);
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

	// a random fixpnt with a random number of leading sign bits, so small operands and the boundaries
	// of the normalization are reached as often as large ones
	template<typename Fixed>
	Fixed random_scaled(std::mt19937_64& rng) {
		Fixed f = random_fixpnt<Fixed>(rng);
		const unsigned s = static_cast<unsigned>(rng() % Fixed::nbits);
		auto raw = f.bits();
		raw >>= static_cast<int>(s);                     // arithmetic: keeps the sign
		f.setbits(0);
		for (unsigned i = 0; i < Fixed::nbits; ++i) f.setbit(i, raw.at(i));
		return f;
	}

	struct Worst { double mul = 0, div = 0; };

	// every pair of encodings (nbits <= 8), or sampled pairs. Both orders of a product are measured: the
	// iterations consume the digits of z and shift x, so x * z and z * x can differ in the last bit where
	// the exact product lies near a rounding tie, and each must be within the bound.
	// The engine works on magnitudes, so mul and div are odd in each operand: -x gives the negated result,
	// except where Saturate's asymmetric range (maxneg = -maxpos - ulp) clamps one of the two.
	template<typename Fixed>
	int Measure(Worst& w, unsigned long long samples, bool reportTestCases) {
		int fails = 0;
		const Fixed maxneg(SpecificValue::maxneg), maxpos(SpecificValue::maxpos);
		auto odd = [&](const char* what, const Fixed& a, const Fixed& b, const Fixed& r, const Fixed& rneg) {
			if (a == maxneg || r == maxneg || r == maxpos || rneg == maxneg || rneg == maxpos) return;
			if (rneg != -r) {
				if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(a) << ' ' << what << '(' << a << ", " << b << ") is not odd\n";
				++fails;
			}
		};
		auto one = [&](const Fixed& a, const Fixed& b) {
			const Fixed p = cordic::mul(a, b);
			w.mul = std::max(w.mul, mul_error(p, a, b));
			w.mul = std::max(w.mul, mul_error(cordic::mul(b, a), b, a));
			if (a != maxneg) odd("mul", a, b, p, cordic::mul(Fixed(-a), b));
			if (!b.iszero()) {
				const Fixed q = cordic::div(a, b);
				w.div = std::max(w.div, div_error(q, a, b));
				if (a != maxneg) odd("div", a, b, q, cordic::div(Fixed(-a), b));
			}
		};
		if constexpr (Fixed::nbits <= 8u) {
			if (samples == 0) {
				for (std::uint64_t i = 0; i < (1ull << Fixed::nbits); ++i) {
					for (std::uint64_t j = 0; j < (1ull << Fixed::nbits); ++j) {
						Fixed a, b;
						a.setbits(i);
						b.setbits(j);
						one(a, b);
					}
				}
				return fails;
			}
		}
		std::mt19937_64 rng(2040);
		for (unsigned long long i = 0; i < samples; ++i) {
			const Fixed a = (i & 1u) ? random_scaled<Fixed>(rng) : random_fixpnt<Fixed>(rng);
			const Fixed b = (i & 2u) ? random_scaled<Fixed>(rng) : random_fixpnt<Fixed>(rng);
			one(a, b);
		}
		return fails;
	}

	// every pair or samples, with the worst errors, at most 1 ulp
	template<typename Fixed>
	int VerifyLinear(const std::string& tag, bool reportTestCases, unsigned long long samples) {
		Worst w;
		int fails = Measure<Fixed>(w, samples, reportTestCases);
		const double bound = 1.0;
		auto check = [&](const char* name, double e) {
			if (e > bound) {
				if (reportTestCases) std::cerr << "FAIL: " << tag << ' ' << name << " worst error " << e << " ulp exceeds " << bound << '\n';
				++fails;
			}
		};
		check("mul", w.mul);
		check("div", w.div);
		if (reportTestCases) {
			std::cout << std::left << std::setw(22) << tag << std::right << std::fixed << std::setprecision(3)
			          << " worst ulp:  mul " << w.mul << "  div " << w.div << '\n';
			std::cout.unsetf(std::ios_base::floatfield);
		}
		return fails;
	}

	// the worst error over samples at a given iteration count, or guard width
	template<typename Fixed, unsigned guard = cordic::auto_guard>
	Worst WorstAt(unsigned iterations, unsigned long long samples) {
		Worst w;
		std::mt19937_64 rng(2040);
		for (unsigned long long i = 0; i < samples; ++i) {
			const Fixed a = random_fixpnt<Fixed>(rng), b = random_scaled<Fixed>(rng);
			w.mul = std::max(w.mul, mul_error(cordic::mul<guard>(a, b, iterations), a, b));
			if (!b.iszero()) w.div = std::max(w.div, div_error(cordic::div<guard>(a, b, iterations), a, b));
		}
		return w;
	}

	// the error halves with each iteration until the guard bits run out: each iteration consumes one
	// signed binary digit of the normalized operand, and leaves at most 2^-(n-1) of it unconsumed
	template<typename Fixed>
	int VerifyIterationCount(bool reportTestCases) {
		int fails = 0;
		constexpr unsigned full = cordic::detail::linear_config<Fixed::nbits, Fixed::rbits, Fixed::arithmetic, cordic::auto_guard>::mulR;
		double prevMul = 1e300, prevDiv = 1e300;
		if (reportTestCases) std::cout << type_tag(Fixed()) << " worst ulp against the iteration count:\n";
		for (unsigned n : { 4u, 8u, 12u, 16u, 20u, full }) {
			const Worst w = WorstAt<Fixed>(n, 4000);
			if (reportTestCases) std::cout << "  n = " << std::setw(3) << n << ":  mul " << w.mul << "  div " << w.div << '\n';
			if (w.mul > prevMul * 1.01 || w.div > prevDiv * 1.01) {
				if (reportTestCases) std::cerr << "FAIL: the error grew from the previous iteration count at n = " << n << '\n';
				++fails;
			}
			prevMul = w.mul;
			prevDiv = w.div;
		}
		// 4 iterations leave a large error; the full count is within 1 ulp
		const Worst w4 = WorstAt<Fixed>(4, 4000), wf = WorstAt<Fixed>(full, 4000);
		if (w4.mul < 16.0 || w4.div < 16.0 || wf.mul > 1.0 || wf.div > 1.0) {
			if (reportTestCases) std::cerr << "FAIL: the iteration count does not control the error\n";
			++fails;
		}
		return fails;
	}

	// the floor that the guard width sets, at each guard width's own full iteration count
	template<typename Fixed>
	int VerifyGuardWidth(bool reportTestCases) {
		int fails = 0;
		const Worst g0 = WorstAt<Fixed, 0>(~0u, 4000);
		const Worst g2 = WorstAt<Fixed, 2>(~0u, 4000);
		const Worst g4 = WorstAt<Fixed, 4>(~0u, 4000);
		const Worst gd = WorstAt<Fixed>(~0u, 4000);
		if (reportTestCases) {
			std::cout << type_tag(Fixed()) << " worst ulp against the guard width:\n";
			std::cout << "  g = 0: mul " << g0.mul << "  div " << g0.div << '\n';
			std::cout << "  g = 2: mul " << g2.mul << "  div " << g2.div << '\n';
			std::cout << "  g = 4: mul " << g4.mul << "  div " << g4.div << '\n';
			std::cout << "  default: mul " << gd.mul << "  div " << gd.div << '\n';
		}
		if (!(g0.mul > g4.mul && g0.div > g4.div) || gd.mul > 1.0 || gd.div > 1.0) {
			if (reportTestCases) std::cerr << "FAIL: the guard width does not control the error floor\n";
			++fails;
		}
		return fails;
	}

	// exact results: 0, x * 1 = x, x / 1 = x, x / x = 1, x * -1 = -x, saturation, and division by zero
	template<typename Fixed>
	int VerifySpecialArguments(bool reportTestCases) {
		int fails = 0;
		auto expect = [&](const char* what, const Fixed& got, const Fixed& want) {
			if (got != want) {
				if (reportTestCases && fails < 8) std::cerr << "FAIL: " << type_tag(got) << ' ' << what << " = " << got << ", expected " << want << '\n';
				++fails;
			}
		};
		const Fixed zero(0), one(1), minus_one(-1);
		const Fixed maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
		std::mt19937_64 rng(2040);
		for (unsigned k = 0; k < 2000; ++k) {
			const Fixed x = (k & 1u) ? random_scaled<Fixed>(rng) : random_fixpnt<Fixed>(rng);
			expect("x * 0", cordic::mul(x, zero), zero);
			expect("0 * x", cordic::mul(zero, x), zero);
			expect("x * 1", cordic::mul(x, one), x);
			expect("x / 1", cordic::div(x, one), x);
			if (x != maxneg) {                                 // -maxneg is out of range
				expect("x * -1", cordic::mul(x, minus_one), -x);
				expect("x / -1", cordic::div(x, minus_one), -x);
			}
			if (!x.iszero()) {
				expect("x / x", cordic::div(x, x), one);
				expect("0 / x", cordic::div(zero, x), zero);
			}
		}
		if constexpr (Fixed::arithmetic == Saturate) {
			expect("maxpos * maxpos", cordic::mul(maxpos, maxpos), maxpos);
			expect("maxpos * maxneg", cordic::mul(maxpos, maxneg), maxneg);
			expect("maxneg * maxneg", cordic::mul(maxneg, maxneg), maxpos);
			const Fixed tiny(SpecificValue::minpos);
			expect("maxpos / minpos", cordic::div(maxpos, tiny), maxpos);
			expect("maxneg / minpos", cordic::div(maxneg, tiny), maxneg);
			expect("minpos / maxpos", cordic::div(tiny, maxpos), zero);
		}
#if FIXPNT_THROW_DIVIDE_BY_ZERO
		bool threw = false;
		try { (void)cordic::div(one, zero); }
		catch (const fixpnt_divide_by_zero&) { threw = true; }
		if (!threw) {
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(one) << " div(1, 0) did not throw fixpnt_divide_by_zero\n";
			++fails;
		}
#else
		if constexpr (Fixed::arithmetic == Saturate) {
			expect("1 / 0", cordic::div(one, zero), maxpos);
			expect("-1 / 0", cordic::div(minus_one, zero), maxneg);
		}
		else {
			expect("1 / 0", cordic::div(one, zero), zero);
		}
#endif
		return fails;
	}

}  // namespace

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

	std::string test_suite  = "fixpnt linear CORDIC engine (#2040)";
	std::string test_tag    = "cordic linear";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 4, Saturate, std::uint8_t>>("fixpnt<8,4,sat>", reportTestCases, 0)), "fixpnt<8,4,sat>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// exhaustive: every pair of encodings; both arithmetic behaviours, a format that holds only [-1, 1),
	// one with a single fraction bit, and one with no integer bit
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 4, Modulo, std::uint8_t>>("fixpnt<8,4,mod>", reportTestCases, 0)), "fixpnt<8,4,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 4, Saturate, std::uint8_t>>("fixpnt<8,4,sat>", reportTestCases, 0)), "fixpnt<8,4,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 7, Modulo, std::uint8_t>>("fixpnt<8,7,mod>", reportTestCases, 0)), "fixpnt<8,7,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 7, Saturate, std::uint8_t>>("fixpnt<8,7,sat>", reportTestCases, 0)), "fixpnt<8,7,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 1, Modulo, std::uint8_t>>("fixpnt<8,1,mod>", reportTestCases, 0)), "fixpnt<8,1,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 1, Saturate, std::uint8_t>>("fixpnt<8,1,sat>", reportTestCases, 0)), "fixpnt<8,1,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 8, Modulo, std::uint8_t>>("fixpnt<8,8,mod>", reportTestCases, 0)), "fixpnt<8,8,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<8, 8, Saturate, std::uint8_t>>("fixpnt<8,8,sat>", reportTestCases, 0)), "fixpnt<8,8,sat>", test_tag);

	// sampled: wider formats, past what double carries
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<12, 8, Modulo, std::uint8_t>>("fixpnt<12,8,mod>", reportTestCases, 20000)), "fixpnt<12,8,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<16, 12, Saturate, std::uint16_t>>("fixpnt<16,12,sat>", reportTestCases, 20000)), "fixpnt<16,12,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<32, 16, Modulo, std::uint32_t>>("fixpnt<32,16,mod>", reportTestCases, 5000)), "fixpnt<32,16,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<32, 16, Saturate, std::uint32_t>>("fixpnt<32,16,sat>", reportTestCases, 5000)), "fixpnt<32,16,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<64, 60, Saturate, std::uint32_t>>("fixpnt<64,60,sat>", reportTestCases, 1000)), "fixpnt<64,60,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<64, 32, Modulo, std::uint64_t>>("fixpnt<64,32,mod>", reportTestCases, 500)), "fixpnt<64,32,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<128, 120, Saturate, std::uint32_t>>("fixpnt<128,120,sat>", reportTestCases, 200)), "fixpnt<128,120,sat>", test_tag);

	// the hardware-model knobs
	nrOfFailedTestCases += ReportTestResult((VerifyIterationCount<fixpnt<16, 12, Saturate, std::uint16_t>>(reportTestCases)), "iteration count", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyGuardWidth<fixpnt<16, 12, Saturate, std::uint16_t>>(reportTestCases)), "guard width", test_tag);

	// special arguments
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<16, 8, Saturate, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,sat> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<16, 8, Modulo, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,mod> special", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySpecialArguments<fixpnt<64, 32, Saturate, std::uint32_t>>(reportTestCases)), "fixpnt<64,32,sat> special", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<16, 4, Modulo, std::uint16_t>>("fixpnt<16,4,mod>", reportTestCases, 50000)), "fixpnt<16,4,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<64, 60, Modulo, std::uint64_t>>("fixpnt<64,60,mod>", reportTestCases, 2000)), "fixpnt<64,60,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<128, 64, Saturate, std::uint32_t>>("fixpnt<128,64,sat>", reportTestCases, 1000)), "fixpnt<128,64,sat>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<12, 8, Saturate, std::uint16_t>>("fixpnt<12,8,sat>", reportTestCases, 1000000)), "fixpnt<12,8,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<128, 120, Modulo, std::uint64_t>>("fixpnt<128,120,mod>", reportTestCases, 2000)), "fixpnt<128,120,mod>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult((VerifyLinear<fixpnt<64, 32, Saturate, std::uint32_t>>("fixpnt<64,32,sat>", reportTestCases, 100000)), "fixpnt<64,32,sat>", test_tag);
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
