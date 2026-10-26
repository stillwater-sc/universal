// mpfr_gap.cpp: efloat against MPFR called from C++, at 128, 512, 2048 and 8192 bits (#1763, #1782)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every operation of efloat's arithmetic and math library, timed for efloat and for MPFR in one
// process, followed by the ratio efloat / MPFR: the figure #1763 drives to 1 or below. The first
// eight rows are the operations and operands of ../mpmath_gap.cpp; MPFR is called directly, so
// unlike the gmpy2 table of mpmath_gap.py there is no Python call in its figures.
//
//   mpfr_gap                 every operation at 128, 512, 2048 and 8192 bits
//   mpfr_gap exp,log,sin     the named operations only
//
// Each figure is the mean over repeated calls, the repetitions growing until a measurement takes
// at least 20 ms; the best of three such measurements is reported. Every call runs once before it
// is timed, so the caches of ln 2, pi and the log and Bernoulli tables are warm, as they are in an
// application that calls a function more than once. Results reach a volatile sink.
//
// All rows share one translation unit, so a change to one function can move another row by a
// few percent through code layout alone (#1790: +-4% at 128 bits under g++ on rows the change
// did not touch). Before reading such a move as a cost, time the row in a translation unit of
// its own, or under clang.
//
// MPFR is a dependency of this benchmark only, never of the library: the CMakeLists.txt next to
// this file builds it when it finds mpfr.h, libmpfr and libgmp (Debian/Ubuntu: libmpfr-dev).
// Both libraries round to nearest here; the figures are timings only. mpfr_check, in
// benchmark/accuracy/efloat, compares the results.

#include <universal/utility/directives.hpp>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <mpfr.h>

namespace sw { namespace universal {

	const char* const A = "0.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274";
	const char* const B = "1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727";

	// the operations, in the order of the tables; a = 0.718..., b = 1.414..., x = 1 + a
	const char* const operations[] = {
		"a+b", "a*b", "b/a", "a/7", "exp(a)", "log(x)", "sin(a)", "sqrt(b)",
		"fma(a,b,a)", "hypot(a,b)", "cbrt(b)",
		"exp2(a)", "exp10(a)", "expm1(a)", "log2(x)", "log10(x)", "log1p(a)",
		"cos(a)", "tan(a)", "asin(a)", "acos(a)", "atan(a)", "atan2(a,b)",
		"sinh(a)", "cosh(a)", "tanh(a)", "asinh(a)", "acosh(b)", "atanh(a)",
		"pow(b,a)", "pow(b,7)", "tgamma(b)", "lgamma(b)", "erf(a)", "erfc(a)",
	};
	constexpr int count = static_cast<int>(sizeof(operations) / sizeof(operations[0]));

	// mean microseconds per call of f: the best of three measurements of at least 20 ms each
	double microseconds(const std::function<void()>& f) {
		f();   // warm the caches
		double best = 1e300;
		for (int trial = 0; trial < 3; ++trial) {
			long reps = 1;
			for (;;) {
				const auto t0 = std::chrono::steady_clock::now();
				for (long i = 0; i < reps; ++i) f();
				const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
				if (s >= 0.02) { best = std::min(best, 1e6 * s / static_cast<double>(reps)); break; }
				reps *= (s < 0.002) ? 10 : 2;
			}
		}
		return best;
	}

	bool selected(const std::string& selection, const char* op) {
		if (selection == "all") return true;
		const std::string name(op);
		const std::string stem = name.substr(0, name.find('('));
		const std::string list = "," + selection + ",";
		return list.find("," + name + ",") != std::string::npos || list.find("," + stem + ",") != std::string::npos;
	}

	template<unsigned nlimbs>
	std::vector<double> measure_efloat(const std::string& selection) {
		using Real = efloat<nlimbs>;
		Real a, b, s;
		parse<16384>(A, a);
		parse<16384>(B, b);
		const Real x = Real(1) + a;
		volatile double sink = 0.0;
		const std::function<void()> f[count] = {
			[&] { s = a + b; sink = sink + double(s); },
			[&] { s = a * b; sink = sink + double(s); },
			[&] { s = b / a; sink = sink + double(s); },
			[&] { s = a / 7; sink = sink + double(s); },
			[&] { s = exp(a); sink = sink + double(s); },
			[&] { s = log(x); sink = sink + double(s); },
			[&] { s = sin(a); sink = sink + double(s); },
			[&] { s = sqrt(b); sink = sink + double(s); },
			[&] { s = fma(a, b, a); sink = sink + double(s); },
			[&] { s = hypot(a, b); sink = sink + double(s); },
			[&] { s = cbrt(b); sink = sink + double(s); },
			[&] { s = exp2(a); sink = sink + double(s); },
			[&] { s = exp10(a); sink = sink + double(s); },
			[&] { s = expm1(a); sink = sink + double(s); },
			[&] { s = log2(x); sink = sink + double(s); },
			[&] { s = log10(x); sink = sink + double(s); },
			[&] { s = log1p(a); sink = sink + double(s); },
			[&] { s = cos(a); sink = sink + double(s); },
			[&] { s = tan(a); sink = sink + double(s); },
			[&] { s = asin(a); sink = sink + double(s); },
			[&] { s = acos(a); sink = sink + double(s); },
			[&] { s = atan(a); sink = sink + double(s); },
			[&] { s = atan2(a, b); sink = sink + double(s); },
			[&] { s = sinh(a); sink = sink + double(s); },
			[&] { s = cosh(a); sink = sink + double(s); },
			[&] { s = tanh(a); sink = sink + double(s); },
			[&] { s = asinh(a); sink = sink + double(s); },
			[&] { s = acosh(b); sink = sink + double(s); },
			[&] { s = atanh(a); sink = sink + double(s); },
			[&] { s = pow(b, a); sink = sink + double(s); },
			[&] { s = pow(b, 7); sink = sink + double(s); },
			[&] { s = tgamma(b); sink = sink + double(s); },
			[&] { s = lgamma(b); sink = sink + double(s); },
			[&] { s = erf(a); sink = sink + double(s); },
			[&] { s = erfc(a); sink = sink + double(s); },
		};
		std::vector<double> t(count, 0.0);
		for (int i = 0; i < count; ++i) if (selected(selection, operations[i])) t[i] = microseconds(f[i]);
		(void)sink;
		return t;
	}

	std::vector<double> measure_mpfr(unsigned bits, const std::string& selection) {
		mpfr_t a, b, x, s;
		mpfr_inits2(static_cast<mpfr_prec_t>(bits), a, b, x, s, static_cast<mpfr_ptr>(nullptr));
		mpfr_set_str(a, A, 10, MPFR_RNDN);
		mpfr_set_str(b, B, 10, MPFR_RNDN);
		mpfr_add_ui(x, a, 1, MPFR_RNDN);
		volatile double sink = 0.0;
		int sign = 0;
		const mpfr_rnd_t r = MPFR_RNDN;
		const std::function<void()> f[count] = {
			[&] { mpfr_add(s, a, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_mul(s, a, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_div(s, b, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_div_ui(s, a, 7, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_exp(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_log(s, x, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_sin(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_sqrt(s, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_fma(s, a, b, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_hypot(s, a, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_cbrt(s, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_exp2(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_exp10(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_expm1(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_log2(s, x, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_log10(s, x, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_log1p(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_cos(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_tan(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_asin(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_acos(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_atan(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_atan2(s, a, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_sinh(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_cosh(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_tanh(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_asinh(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_acosh(s, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_atanh(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_pow(s, b, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_pow_si(s, b, 7, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_gamma(s, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_lgamma(s, &sign, b, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_erf(s, a, r); sink = sink + mpfr_get_d(s, r); },
			[&] { mpfr_erfc(s, a, r); sink = sink + mpfr_get_d(s, r); },
		};
		std::vector<double> t(count, 0.0);
		for (int i = 0; i < count; ++i) if (selected(selection, operations[i])) t[i] = microseconds(f[i]);
		(void)sink;
		mpfr_clears(a, b, x, s, static_cast<mpfr_ptr>(nullptr));
		return t;
	}

	struct Column { unsigned bits; std::vector<double> efloat, mpfr; };

	template<unsigned nlimbs>
	Column measure(const std::string& selection) {
		return { 32 * nlimbs, measure_efloat<nlimbs>(selection), measure_mpfr(32 * nlimbs, selection) };
	}

	// microseconds with three significant digits, in a field of 9
	void print_time(double us) {
		if (us >= 1000.0) std::printf(" %9.0f", us);
		else if (us >= 100.0) std::printf(" %9.1f", us);
		else if (us >= 10.0) std::printf(" %9.2f", us);
		else std::printf(" %9.3f", us);
	}

}} // namespace sw::universal

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	const std::string selection = (argc > 1) ? argv[1] : "all";
	std::printf("efloat vs MPFR %s (GMP %s) called from C++: microseconds per operation (#1763)\n\n",
	            mpfr_get_version(), gmp_version);
	const std::vector<Column> columns = { measure<4>(selection), measure<16>(selection), measure<64>(selection), measure<256>(selection) };

	std::printf("%-11s", "operation");
	for (const Column& c : columns) std::printf(" | %5u: %9s %9s %6s", c.bits, "efloat", "MPFR", "ratio");
	std::printf("\n");
	for (int i = 0; i < count; ++i) {
		if (!selected(selection, operations[i])) continue;
		std::printf("%-11s", operations[i]);
		for (const Column& c : columns) {
			std::printf(" |       ");
			print_time(c.efloat[i]);
			print_time(c.mpfr[i]);
			std::printf(" %5.1fx", c.efloat[i] / c.mpfr[i]);
		}
		std::printf("\n");
	}
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
