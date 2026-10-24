// mpmath_gap.cpp: efloat's cost per operation at 128, 512, 2048 and 8192 bits, for comparison with mpmath and gmpy2 (#1714, #1763)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// mpmath's mpf is the same structure as efloat -- a sign, a big-integer mantissa and an
// exponent -- yet on its pure-Python backend it evaluated exp ~90x and log ~900x faster than
// compiled efloat at 600 bits. mpmath_gap.py next to this file prints the same table for
// mpmath, and for gmpy2 (MPFR) when it is installed; run both on one machine and compare.
//
// The precisions step by 4x: 2, 8, 32 and 128 64-bit limbs, so a change of multiplication
// algorithm shows in the growth from row to row, and 8192 bits is where Karatsuba, Toom-3
// and binary splitting pay off (#1763). #1714-#1760 measured at 608 bits (efloat<19>), just
// past mpmath's exp switch at 600 bits.
//
// Each figure is a mean over repeated calls whose results all reach a volatile sink.
//
// The guards are ratios, so that they hold at any optimization level and on any runner:
//   - division may cost at most 15 multiplies at 512 bits. Long division a word at a time costs
//     ~3; the bit-serial division it replaced cost ~40.
//   - at 128 bits a difference and a product may each cost at most 4 sums, and at 512 bits a
//     square root at most 8 products (#1826): 1.1, 1.4 and 3.7 at -O3, 1.2, 1.3 and 4.1 at -O0.
//     They catch a regression of several times, such as an operation that falls back to a
//     bit-serial or allocating routine, not the 1.2-1.5x a lost fast path costs: mpfr_bands
//     measures that.

#include <universal/utility/directives.hpp>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <universal/number/efloat/efloat.hpp>

namespace sw { namespace universal {

	// mean microseconds per call of f over reps calls
	template<typename F>
	double microseconds(F&& f, int reps) {
		auto t0 = std::chrono::steady_clock::now();
		for (int i = 0; i < reps; ++i) f();
		auto t1 = std::chrono::steady_clock::now();
		return std::chrono::duration<double, std::micro>(t1 - t0).count() / reps;
	}

	struct Timings {
		double add, mul, div, div7, exp, log, sin, sqrt, sub;
	};

	// reps scales the repetition counts down for the expensive operations at high precision
	template<unsigned nlimbs>
	Timings measure(int reps) {
		using Real = efloat<nlimbs>;
		Real a, b;
		parse<16384>("0.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274", a);
		parse<16384>("1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727", b);
		const Real seven(7);
		const Real x = Real(1) + a;
		Real s = a;
		volatile double sink = 0.0;

		// The first call at a precision computes ln 2 and pi and caches them, as mpmath caches
		// its constants: call each function once before timing, so few repetitions still
		// measure the steady state.
		sink = sink + double(exp(a)) + double(log(x)) + double(sin(a));

		// add and multiply run dependent chains that stay in range; the rest take the arguments
		// mpmath_gap.py uses, and each result feeds the sink, so no call can be hoisted
		Timings t{};
		t.add  = microseconds([&] { s = s + b; }, 100 * reps);              sink = sink + double(s); s = a;
		t.mul  = microseconds([&] { s = s * a; s = s * b; }, 50 * reps) / 2; sink = sink + double(s);
		t.div  = microseconds([&] { s = b / a; sink = sink + double(s); }, 10 * reps);
		t.div7 = microseconds([&] { s = a / seven; sink = sink + double(s); }, 10 * reps);
		t.exp  = microseconds([&] { s = exp(a); sink = sink + double(s); }, reps);
		t.log  = microseconds([&] { s = log(x); sink = sink + double(s); }, reps);
		t.sin  = microseconds([&] { s = sin(a); sink = sink + double(s); }, reps);
		t.sqrt = microseconds([&] { s = sqrt(b); sink = sink + double(s); }, 2 * reps);
		t.sub  = microseconds([&] { s = b - a; sink = sink + double(s); }, 100 * reps);
		(void)sink;
		return t;
	}

	void report(unsigned bits, const Timings& t) {
		std::printf("%6u %9.2f %9.2f %9.2f %9.2f %10.2f %10.2f %10.2f %9.2f\n",
		            bits, t.add, t.mul, t.div, t.div7, t.exp, t.log, t.sin, t.sqrt);
	}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "efloat cost per operation in microseconds (#1714; compare mpmath_gap.py)\n";
	std::printf("%6s %9s %9s %9s %9s %10s %10s %10s %9s\n",
	            "bits", "a+b", "a*b", "b/a", "a/7", "exp", "log", "sin", "sqrt");
	const Timings t128 = measure<4>(200);
	report(128, t128);
	const Timings t512 = measure<16>(50);
	report(512, t512);
	report(2048, measure<64>(5));
	report(8192, measure<256>(4));

	int failures = 0;
	const double ratio = t512.div / t512.mul;
	std::printf("\ndivision / multiplication at 512 bits: %.1f (guard: below 15)\n", ratio);
	if (!(ratio < 15.0)) {
		std::cout << "  *** REGRESSION: division costs " << ratio << " multiplies -- a bit-serial division may have returned (#1714) ***\n";
		++failures;
	}

	// ratios to an operation of about the same cost (#1826)
	struct Guard { const char* what; double ratio, bound; };
	const Guard guards[] = {
		{ "difference / sum at 128 bits",          t128.sub / t128.add,  4.0 },
		{ "product / sum at 128 bits",             t128.mul / t128.add,  4.0 },
		{ "square root / product at 512 bits",     t512.sqrt / t512.mul, 8.0 },
	};
	for (const Guard& g : guards) {
		std::printf("%s: %.1f (guard: below %.0f)\n", g.what, g.ratio, g.bound);
		if (!(g.ratio < g.bound)) {
			std::cout << "  *** REGRESSION: " << g.what << " -- several times its cost at the time of #1826 ***\n";
			++failures;
		}
	}

	std::cout << "\n" << (failures == 0 ? "PASS" : "FAIL") << " efloat operation costs within bounds\n";
	return (failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
