// mpfr_gap.cpp: efloat against MPFR called from C++, at 128, 512, 2048 and 8192 bits (#1763)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The same operations and operands as ../mpmath_gap.cpp, timed for efloat and for MPFR in one
// process, followed by the ratio efloat / MPFR: the figure #1763 drives to 1 or below. MPFR is
// called directly, so unlike the gmpy2 table of mpmath_gap.py there is no Python call in its
// figures; this is the stretch target of #1763.
//
// MPFR is a dependency of this benchmark only, never of the library: the CMakeLists.txt next to
// this file builds it when it finds mpfr.h, libmpfr and libgmp (Debian/Ubuntu: libmpfr-dev).
// Both libraries round to nearest here; the figures are timings only, results are not compared.

#include <universal/utility/directives.hpp>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <universal/number/efloat/efloat.hpp>
#include <mpfr.h>

namespace sw { namespace universal {

	const char* const A = "0.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274";
	const char* const B = "1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727";

	// mean microseconds per call of f over reps calls
	template<typename F>
	double microseconds(F&& f, long reps) {
		auto t0 = std::chrono::steady_clock::now();
		for (long i = 0; i < reps; ++i) f();
		auto t1 = std::chrono::steady_clock::now();
		return std::chrono::duration<double, std::micro>(t1 - t0).count() / static_cast<double>(reps);
	}

	constexpr int columns = 8;
	struct Timings {
		double t[columns];   // a+b, a*b, b/a, a/7, exp, log, sin, sqrt
	};

	// repetitions fall with the square of the precision, so the 8192-bit row takes seconds
	long arithmetic_reps(unsigned bits) { long s = (bits / 512) * (bits / 512); return std::max(200L, 2000000L / std::max(1L, s)); }
	long function_reps(unsigned bits)   { long s = (bits / 512) * (bits / 512); return std::max(4L, 20000L / std::max(1L, s)); }

	template<unsigned nlimbs>
	Timings measure_efloat() {
		using Real = efloat<nlimbs>;
		constexpr unsigned bits = 32 * nlimbs;
		Real a, b, s;
		parse<16384>(A, a);
		parse<16384>(B, b);
		const Real seven(7);
		const Real x = Real(1) + a;
		volatile double sink = 0.0;
		sink = sink + double(exp(a)) + double(log(x)) + double(sin(a));   // cache ln 2 and pi first
		const long n = arithmetic_reps(bits) / 10, m = function_reps(bits) / 10 + 1;
		Timings t{};
		t.t[0] = microseconds([&] { s = a + b; sink = sink + double(s); }, n);
		t.t[1] = microseconds([&] { s = a * b; sink = sink + double(s); }, n);
		t.t[2] = microseconds([&] { s = b / a; sink = sink + double(s); }, n);
		t.t[3] = microseconds([&] { s = a / seven; sink = sink + double(s); }, n);
		t.t[4] = microseconds([&] { s = exp(a); sink = sink + double(s); }, m);
		t.t[5] = microseconds([&] { s = log(x); sink = sink + double(s); }, m);
		t.t[6] = microseconds([&] { s = sin(a); sink = sink + double(s); }, m);
		t.t[7] = microseconds([&] { s = sqrt(b); sink = sink + double(s); }, n / 5 + 1);
		(void)sink;
		return t;
	}

	Timings measure_mpfr(unsigned bits) {
		mpfr_t a, b, x, s, seven;
		mpfr_inits2(static_cast<mpfr_prec_t>(bits), a, b, x, s, seven, static_cast<mpfr_ptr>(nullptr));
		mpfr_set_str(a, A, 10, MPFR_RNDN);
		mpfr_set_str(b, B, 10, MPFR_RNDN);
		mpfr_set_ui(seven, 7, MPFR_RNDN);
		mpfr_add_ui(x, a, 1, MPFR_RNDN);
		volatile double sink = 0.0;
		mpfr_exp(s, a, MPFR_RNDN); mpfr_log(s, x, MPFR_RNDN); mpfr_sin(s, a, MPFR_RNDN);   // cache ln 2 and pi first
		const long n = arithmetic_reps(bits), m = function_reps(bits);
		Timings t{};
		t.t[0] = microseconds([&] { mpfr_add(s, a, b, MPFR_RNDN); sink = sink + mpfr_get_d(s, MPFR_RNDN); }, n);
		t.t[1] = microseconds([&] { mpfr_mul(s, a, b, MPFR_RNDN); sink = sink + mpfr_get_d(s, MPFR_RNDN); }, n);
		t.t[2] = microseconds([&] { mpfr_div(s, b, a, MPFR_RNDN); sink = sink + mpfr_get_d(s, MPFR_RNDN); }, n);
		t.t[3] = microseconds([&] { mpfr_div(s, a, seven, MPFR_RNDN); sink = sink + mpfr_get_d(s, MPFR_RNDN); }, n);
		t.t[4] = microseconds([&] { mpfr_exp(s, a, MPFR_RNDN); sink = sink + mpfr_get_d(s, MPFR_RNDN); }, m);
		t.t[5] = microseconds([&] { mpfr_log(s, x, MPFR_RNDN); sink = sink + mpfr_get_d(s, MPFR_RNDN); }, m);
		t.t[6] = microseconds([&] { mpfr_sin(s, a, MPFR_RNDN); sink = sink + mpfr_get_d(s, MPFR_RNDN); }, m);
		t.t[7] = microseconds([&] { mpfr_sqrt(s, b, MPFR_RNDN); sink = sink + mpfr_get_d(s, MPFR_RNDN); }, n / 5 + 1);
		(void)sink;
		mpfr_clears(a, b, x, s, seven, static_cast<mpfr_ptr>(nullptr));
		return t;
	}

	void header() {
		std::printf("%6s %-8s %9s %9s %9s %9s %10s %10s %10s %9s\n",
		            "bits", "", "a+b", "a*b", "b/a", "a/7", "exp", "log", "sin", "sqrt");
	}

	void row(unsigned bits, const char* label, const Timings& t) {
		std::printf("%6u %-8s %9.3f %9.3f %9.3f %9.3f %10.2f %10.2f %10.2f %9.3f\n",
		            bits, label, t.t[0], t.t[1], t.t[2], t.t[3], t.t[4], t.t[5], t.t[6], t.t[7]);
	}

	void ratio(unsigned bits, const Timings& e, const Timings& m) {
		std::printf("%6u %-8s", bits, "ratio");
		for (int i = 0; i < columns; ++i) {
			const int width = (i >= 4 && i < 7) ? 9 : 8;   // the function columns are one wider
			std::printf(" %*.1fx", width, e.t[i] / m.t[i]);
		}
		std::printf("\n");
	}

	template<unsigned nlimbs>
	void compare() {
		constexpr unsigned bits = 32 * nlimbs;
		const Timings e = measure_efloat<nlimbs>();
		const Timings m = measure_mpfr(bits);
		row(bits, "efloat", e);
		row(bits, "MPFR", m);
		ratio(bits, e, m);
	}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::printf("efloat vs MPFR %s (GMP %s) called from C++: microseconds per operation (#1763)\n",
	            mpfr_get_version(), gmp_version);
	header();
	compare<4>();
	compare<16>();
	compare<64>();
	compare<256>();
	std::printf("\nratio = efloat / MPFR; #1763 is met where every ratio is at or below 1.0\n");
	return EXIT_SUCCESS;
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
