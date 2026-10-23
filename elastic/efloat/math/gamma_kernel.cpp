// gamma_kernel.cpp: regression tests for efloat's lgamma and tgamma kernels (#1797)
//
// lgamma and tgamma evaluate lgamma x in lgamma_abs at a working precision W:
//     lgamma x = (z - 1/2) log z - z + log(2 pi) / 2 + sum_k c_k / z^(2k - 1) - log(x (x + 1) ... (x + N - 1)),
// z = x + N, with the Stirling sum by a tapered Horner scheme in 1 / z^2 (stirling_sum), the
// shift's product in groups of factors (shift_product), and the reflection below 0. W is
// predicted from x before the first pass (lgamma_precision, tgamma_precision). Ziv's strategy
// retries an evaluation it cannot round, and lgamma_working retries a pass whose cancellation
// exceeds W, so a kernel that broke its bound, or a prediction that fell short, could still give
// correctly rounded results, only slower: the rounded results do not test them. This test does,
// against Spouge's formula (1994), which shares none of the kernel's code:
//     gamma(z + 1) = (z + a)^(z + 1/2) e^-(z + a) [c_0 + sum_{k=1}^{a-1} c_k / (z + k) + eps],
//     c_0 = sqrt(2 pi),   c_k = (-1)^(k-1) (a - k)^(k - 1/2) e^(a - k) / (k - 1)!,
// |eps| below a^(-1/2) (2 pi)^-(a + 1/2) of the bracket for z > 0, so a = B / log2(2 pi) + 2
// gives B bits below 1. Its terms cancel by up to about 2a bits, which it carries as well:
//   - lgamma_abs(x, W) within 2^(E - W + gamma_error_bits) of lgamma x at the predicted W, and
//     the sign of gamma x;
//   - the predicted W: one pass is enough, W >= E - scale(lgamma x) + needed for lgamma and
//     W >= E + needed for tgamma, wherever the prediction is meant to hold (all x > 0, and below
//     0 away from lgamma's zeros there);
//   - lgamma_working within 2^(scale - target - exp_log_guard_bits), and tgamma_working within
//     2^-(target + exp_log_guard_bits - exp_error_bits - 1) relative;
//   - x random over many binades, next to 1 and 2 down to 1 +- 2^-P, tiny and large x, below 0
//     next to the poles, integers, and either side of the shift's end z0, where the product has
//     0 to 9 factors.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 6 precisions from 32 to 1024 bits, and 2048 bits.
//   LEVEL 2 -- four times the random cases.
//   LEVEL 3 -- and 4096 bits.
//   LEVEL 4 -- sixteen times the random cases, and 8192 bits.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using E = sw::universal::efloat<3200>;

	// Spouge's coefficients for B bits, at the precision spouge_bits(B), cached
	struct Spouge {
		unsigned       a    = 0;
		unsigned       bits = 0;
		std::vector<E> c;
	};
	unsigned spouge_terms(unsigned B) { return static_cast<unsigned>(B / 2.6514961294723187) + 2u; }
	unsigned spouge_bits(unsigned B) { return B + 3u * spouge_terms(B) + 64u; }

	const Spouge& spouge_coefficients(unsigned B) {
		static Spouge s;
		if (s.bits == B) return s;
		s.bits = B;
		s.a    = spouge_terms(B);
		const unsigned W = spouge_bits(B);
		s.c.assign(s.a, E(0));
		E one(1);
		one.set_precision(W);
		const E e    = exp(one);
		const E inve = one / e;
		E ea(one);   // e^(a - k)
		for (unsigned k = 1; k < s.a; ++k) ea *= e;
		E factorial(one);   // (k - 1)!
		for (unsigned k = 1; k < s.a; ++k) {
			if (k > 1) factorial *= static_cast<int>(k - 1);
			E base(one);
			base *= static_cast<int>(s.a - k);
			E power(one), square(base);   // (a - k)^k by squaring
			for (unsigned n = k; n != 0; n >>= 1) {
				if (n & 1u) power *= square;
				if (n > 1) square *= square;
			}
			power /= sqrt(base);
			const E ck = power * ea / factorial;
			s.c[k] = (k % 2 == 1) ? ck : -ck;
			ea *= inve;
		}
		E two_pi = sw::universal::efloat_detail::pi<3200>(W);
		two_pi.setexponent(two_pi.scale() + 1);
		s.c[0] = sqrt(two_pi);
		return s;
	}

	// lgamma x for x > 1, within about 2^-B of it below the scale of its parts
	E spouge_lgamma(const E& x, unsigned B) {
		const Spouge&  s = spouge_coefficients(B);
		const unsigned W = spouge_bits(B);
		E z(x);
		z.set_precision(W);
		z -= 1;
		E sum(s.c[0]);
		for (unsigned k = 1; k < s.a; ++k) {
			E d(z);
			d += static_cast<int>(k);
			sum += s.c[k] / d;
		}
		E za(z);
		za += static_cast<int>(s.a);
		E half(0.5);
		return (z + half) * log(za) - za + log(sum);
	}

	// lgamma x, absolute within about 2^-B below the scale of its parts: for x <= 1 from
	// lgamma(x + 1) - log x, below 0 from the reflection log pi - log |sin pi x| - lgamma(1 - x)
	E reference_lgamma(const E& x, unsigned B) {
		const unsigned W = spouge_bits(B);
		E one(1);
		one.set_precision(W);
		E xw(x);
		xw.set_precision(W);
		if (x.sign() == -1) {
			const E n = round(x);
			const E pi = sw::universal::efloat_detail::pi<3200>(W);
			E s = sin(pi * (xw - n));
			if (s.sign() == -1) s = -s;
			return log(pi) - log(s) - reference_lgamma(one - xw, B);
		}
		if (xw > one) return spouge_lgamma(xw, B);
		return spouge_lgamma(xw + one, B) - log(xw);
	}

	// a full-width P-bit value in [1, 2) times 2^e
	E random_value(std::mt19937_64& rng, unsigned P, int64_t e) {
		E v(1);
		v.set_precision(P);
		for (unsigned i = 0; i < (P + 31) / 32; ++i) v.setlimb(i, static_cast<uint32_t>(rng()) | (i == 0 ? 0x80000000u : 0u));
		v.set_precision(P);
		v.setexponent(e);
		return v;
	}

	// c + d or c - d for d a random P-bit value below 2^-e
	E next_to(std::mt19937_64& rng, unsigned P, const E& c, int64_t e) {
		const E d = random_value(rng, P, -e);
		E x(c);
		x.set_precision(P);
		x = (rng() & 1u) ? x + d : x - d;
		x.set_precision(P);
		return x;
	}

	// the zero of lgamma next to the double x0, to P bits, by the secant method on the reference
	E lgamma_zero(double x0, unsigned P) {
		const unsigned B = 2u * P + 64u;
		E a(x0), b(x0);
		a.set_precision(B);
		b.set_precision(B);
		b += E(std::ldexp(1.0, -40));
		E fa = reference_lgamma(a, B), fb = reference_lgamma(b, B);
		for (int i = 0; i < 64 && !fb.iszero(); ++i) {
			const E step = fb * (b - a) / (fb - fa);
			a  = b;
			fa = fb;
			b -= step;
			b.set_precision(B);
			fb = reference_lgamma(b, B);
			if (step.iszero() || step.scale() < b.scale() - static_cast<int64_t>(P) - 8) break;
		}
		b.set_precision(P);
		return b;
	}

	// gamma x < 0: x in (-(n + 1), -n) for an odd n + 1
	bool gamma_negative(const E& x) {
		if (x.sign() != -1) return false;
		E n = floor(-x);   // x in (-(n + 1), -n), gamma x < 0 for an even n
		if (n.iszero()) return true;
		n.setexponent(n.scale() - 1);
		return sw::universal::efloat_detail::is_integer(n);
	}

	int fails_reported = 0;
	void report(bool reportTestCases, const char* what, unsigned P, const E& x, int64_t got, int64_t bound) {
		if (reportTestCases && fails_reported++ < 40) {
			std::cerr << "FAIL: " << what << " at P = " << P << ", x = " << double(x) << " (scale " << x.scale() << "): 2^" << got
			          << ", bound 2^" << bound << '\n';
		}
	}

	// one x at precision P: the lgamma_abs contract, the predictions, and both working functions
	int check_gamma(const E& x, unsigned P, bool predicted, bool reportTestCases) {
		using namespace sw::universal::efloat_detail;
		if (x == E(1) || x == E(2)) return 0;   // lgamma is exactly 0 there, outside the kernels' scope
		int fails = 0;
		const int64_t needed = static_cast<int64_t>(P + exp_log_guard_bits + gamma_error_bits);
		const int64_t W      = lgamma_precision(x, needed);
		const unsigned B     = static_cast<unsigned>(W) + 96u + (predicted ? 0u : P);   // where W may fall short of scale(lgamma x)
		const E ref          = reference_lgamma(x, B);
		const int64_t s_ref  = ref.scale();

		// lgamma_abs at the predicted W
		int64_t Eparts = 0;
		bool negative  = false;
		const E y = lgamma_abs(x, static_cast<unsigned>(W), Eparts, negative);
		const E err = y - ref;
		const int64_t bound = Eparts - W + static_cast<int64_t>(gamma_error_bits);
		if (!err.iszero() && err.scale() >= bound) { ++fails; report(reportTestCases, "lgamma_abs error", P, x, err.scale(), bound); }
		if (negative != gamma_negative(x)) { ++fails; report(reportTestCases, "lgamma_abs sign", P, x, negative, gamma_negative(x)); }
		if (predicted && W < Eparts - s_ref + needed) { ++fails; report(reportTestCases, "lgamma_precision short", P, x, W, Eparts - s_ref + needed); }

		// lgamma_working, relative
		const E lw = lgamma_working(x, P);
		const E lerr = lw - ref;
		const int64_t lbound = s_ref - static_cast<int64_t>(P + exp_log_guard_bits);
		if (!lerr.iszero() && lerr.scale() >= lbound) { ++fails; report(reportTestCases, "lgamma_working error", P, x, lerr.scale(), lbound); }

		// tgamma_working, relative, as log |tgamma| - lgamma, where exp's range allows
		if (x.scale() < 50 && ref.scale() < 50) {
			const int64_t tneeded = needed + 1;
			const int64_t Wt = tgamma_precision(x, tneeded);
			int64_t Et = 0;
			bool tneg = false;
			(void)lgamma_abs(x, static_cast<unsigned>(Wt), Et, tneg);
			if (predicted && Wt < Et + tneeded) { ++fails; report(reportTestCases, "tgamma_precision short", P, x, Wt, Et + tneeded); }
			E t = tgamma_working(x, P);
			if ((t.sign() == -1) != gamma_negative(x)) { ++fails; report(reportTestCases, "tgamma_working sign", P, x, t.sign(), 0); }
			if (t.sign() == -1) t = -t;
			t.set_precision(spouge_bits(B));
			const E terr = log(t) - ref;
			const int64_t tbound = -static_cast<int64_t>(P + exp_log_guard_bits - exp_error_bits - 1u);
			if (!terr.iszero() && terr.scale() >= tbound) { ++fails; report(reportTestCases, "tgamma_working error", P, x, terr.scale(), tbound); }
		}
		return fails;
	}

	int VerifyGammaKernel(const std::vector<unsigned>& precisions, unsigned cases, bool reportTestCases) {
		using namespace sw::universal::efloat_detail;
		int fails = 0;
		std::mt19937_64 rng(1797);
		for (unsigned P : precisions) {
			const unsigned n  = (P > 1024) ? std::max(2u, cases / 4u) : cases;
			const int64_t  Ps = static_cast<int64_t>(P);
			auto check = [&](const E& x, bool predicted = true) { fails += check_gamma(x, P, predicted, reportTestCases); };
			const E one(1), two(2);
			for (unsigned c = 0; c < n; ++c) {
				check(random_value(rng, P, static_cast<int64_t>(rng() % 11) - 6));               // 2^-6 .. 32
				check(next_to(rng, P, (rng() & 1u) ? one : two, 1 + static_cast<int64_t>(rng() % P)));   // next to the zeros
				E neg = random_value(rng, P, static_cast<int64_t>(rng() % 4) - 1);                  // below 0, off the poles
				neg = -neg;
				if (!is_integer(neg)) {
					const E l = reference_lgamma(neg, 64u);   // the prediction holds away from lgamma's zeros
					check(neg, l.scale() >= -2);
				}
			}
			for (int64_t e : { int64_t(1), int64_t(9), int64_t(10), Ps / 2, Ps - 1, Ps }) {
				check(next_to(rng, P, one, e));
				check(next_to(rng, P, two, e));
			}
			// tiny, where log x outweighs the shift (2^-1000000), 1e6, 1e100
			for (int64_t e : { int64_t(-20), -Ps - 10, int64_t(-2000), int64_t(-1000000), int64_t(20), int64_t(332) }) check(random_value(rng, P, e));
			for (int pole : { 1, 2, 5 }) {
				for (int64_t e : { int64_t(4), Ps / 2, Ps - 2 }) {
					const E x = next_to(rng, P, E(-pole), e);
					if (!is_integer(x)) check(x);
				}
			}
			// on lgamma's zero near -2.457, to P bits: a double puts lgamma there near 2^-54, not
			// 2^-P, so the prediction falls short and the pass is repeated
			const E zero = lgamma_zero(-2.4570247382208006, P);
			check(zero, false);
			check(next_to(rng, P, zero, Ps / 2), false);
			for (int v : { 3, 20, 100, 1000 }) check(E(v));   // integers, 1000 past the exact factorial
			// either side of the shift's end: the product has 0 to 9 factors
			const double z0 = std::max(16.0, std::ceil(gamma_shift_per_bit * static_cast<double>(lgamma_precision(E(2.5), Ps + 86))));
			for (double t : { -0.5, 0.5, 1.5, 2.5, 8.5, 9.5 }) {
				E x(z0 - t);
				x.set_precision(P);
				check(x);
			}
		}
		return fails;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat lgamma and tgamma kernels";
	std::string test_tag            = "gamma kernel";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	const std::vector<unsigned> precisions = { 32, 64, 128, 256, 512, 1024 };

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyGammaKernel({ 128 }, 8, reportTestCases), "efloat", "gamma kernel 128 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyGammaKernel(precisions, 8, reportTestCases), "efloat", "error bounds, 32 to 1024 bits");
	// from 2048 bits the shift's groups shorten, their coefficients reaching 2^62
	nrOfFailedTestCases += ReportTestResult(VerifyGammaKernel({ 2048 }, 8, reportTestCases), "efloat", "error bounds, 2048 bits");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyGammaKernel(precisions, 32, reportTestCases), "efloat", "error bounds, 4x cases");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyGammaKernel({ 4096 }, 8, reportTestCases), "efloat", "error bounds, 4096 bits");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyGammaKernel(precisions, 128, reportTestCases), "efloat", "error bounds, 16x cases");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaKernel({ 8192 }, 8, reportTestCases), "efloat", "error bounds, 8192 bits");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
