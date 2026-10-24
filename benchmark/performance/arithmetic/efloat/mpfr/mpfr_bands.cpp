// mpfr_bands.cpp: efloat against MPFR called from C++, at every precision from 64 to 16384 bits (#1763, #1826)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// mpfr_gap measures four precisions, all whole numbers of 64-bit limbs. This table measures the
// primitives and three functions at the powers of 2 from 64 to 16384 bits and the half-steps
// between them, among them precisions of an odd number of 32-bit limbs (96, 160, 544, 1056),
// which take other paths through efloat's arithmetic, so that the thresholds and the fast paths
// are chosen from data and a precision between two measured ones cannot hide a cliff (#1826).
//
//   mpfr_bands                      every operation at every precision
//   mpfr_bands a+b,b-a              the named operations only
//   mpfr_bands all 1.5              and exit nonzero when a ratio exceeds 1.5
//
// The operands are random significands that fill their precision, a in [1/2, 1) and b in [1, 2),
// the same values in both libraries. Each figure is the best of seven measurements of at least
// 5 ms each, after one warming call. Timings vary by core on a hybrid processor: pin the process
// (taskset -c 2 on Linux) before comparing runs.
//
// MPFR is a dependency of this benchmark only, never of the library: see mpfr_gap.cpp and
// docs/licensing/mpfr-gmp-lgpl.md.

#include <universal/utility/directives.hpp>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <mpfr.h>

namespace sw { namespace universal {

	const char* const operations[] = { "a+b", "b-a", "a*b", "b/a", "a/7", "sqrt", "fma", "exp", "log", "sin" };
	constexpr int count = static_cast<int>(sizeof(operations) / sizeof(operations[0]));
	constexpr int first_function = 7;               // exp, log and sin: up to function_limit bits
	constexpr unsigned function_limit = 8192;

	const unsigned precisions[] = { 64, 96, 128, 160, 192, 256, 384, 512, 544, 768, 1024, 1056, 1536,
	                                2048, 3072, 4096, 6144, 8192, 12288, 16384 };

	// microseconds per call of f: the best of seven measurements of at least 5 ms each
	double microseconds(const std::function<void()>& f) {
		f();
		double best = 1e300;
		for (int trial = 0; trial < 7; ++trial) {
			long reps = 1;
			for (;;) {
				const auto t0 = std::chrono::steady_clock::now();
				for (long i = 0; i < reps; ++i) f();
				const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
				if (s >= 0.005) { best = std::min(best, 1e6 * s / static_cast<double>(reps)); break; }
				reps *= (s < 0.0005) ? 10 : 2;
			}
		}
		return best;
	}

	bool selected(const std::string& selection, const char* op) {
		if (selection == "all") return true;
		return ("," + selection + ",").find("," + std::string(op) + ",") != std::string::npos;
	}

	using Real = efloat<1024>;

	// a value of `bits` random bits at the given scale, its leading and last bits set
	Real random_value(std::mt19937_64& rng, unsigned bits, int64_t scale) {
		std::vector<std::uint64_t> w((bits + 63) / 64);
		for (auto& limb : w) limb = rng();
		w.back() |= std::uint64_t(1) << 63;
		Real v(1);
		v.set_precision(bits);
		v.set_significand(w.data(), w.size());
		v.setexponent(scale);
		v.set_precision(bits);   // rounds to the precision, which leaves its last bit set or a carry
		return v;
	}

	// the same value in MPFR, exactly: its 32-bit limbs, most significant first, summed
	void to_mpfr(mpfr_t m, const Real& v) {
		mpfr_set_ui(m, 0, MPFR_RNDN);
		const std::vector<uint32_t> limbs = v.bits();
		mpfr_t t;
		mpfr_init2(t, 64);
		for (std::size_t i = 0; i < limbs.size(); ++i) {
			mpfr_set_ui_2exp(t, limbs[i], static_cast<long>(v.scale() - 31 - 32 * static_cast<int64_t>(i)), MPFR_RNDN);
			mpfr_add(m, m, t, MPFR_RNDN);
		}
		mpfr_clear(t);
	}

	struct Row { unsigned bits; std::vector<double> efloat, mpfr; };

	Row measure(unsigned bits, const std::string& selection) {
		std::mt19937_64 rng(bits);
		const Real a = random_value(rng, bits, -1);
		const Real b = random_value(rng, bits, 0);
		Real s;
		mpfr_t ma, mb, ms;
		mpfr_inits2(static_cast<mpfr_prec_t>(bits), ma, mb, ms, static_cast<mpfr_ptr>(nullptr));
		to_mpfr(ma, a);
		to_mpfr(mb, b);
		volatile int sink = 0;
		const mpfr_rnd_t r = MPFR_RNDN;
		const std::function<void()> e[count] = {
			[&] { s = a + b; sink = sink + s.isneg(); },
			[&] { s = b - a; sink = sink + s.isneg(); },
			[&] { s = a * b; sink = sink + s.isneg(); },
			[&] { s = b / a; sink = sink + s.isneg(); },
			[&] { s = a / 7; sink = sink + s.isneg(); },
			[&] { s = sqrt(b); sink = sink + s.isneg(); },
			[&] { s = fma(a, b, a); sink = sink + s.isneg(); },
			[&] { s = exp(a); sink = sink + s.isneg(); },
			[&] { s = log(b); sink = sink + s.isneg(); },
			[&] { s = sin(a); sink = sink + s.isneg(); },
		};
		const std::function<void()> m[count] = {
			[&] { mpfr_add(ms, ma, mb, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_sub(ms, mb, ma, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_mul(ms, ma, mb, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_div(ms, mb, ma, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_div_ui(ms, ma, 7, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_sqrt(ms, mb, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_fma(ms, ma, mb, ma, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_exp(ms, ma, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_log(ms, mb, r); sink = sink + mpfr_signbit(ms); },
			[&] { mpfr_sin(ms, ma, r); sink = sink + mpfr_signbit(ms); },
		};
		Row row{ bits, std::vector<double>(count, 0.0), std::vector<double>(count, 0.0) };
		for (int i = 0; i < count; ++i) {
			if (!selected(selection, operations[i]) || (i >= first_function && bits > function_limit)) continue;
			row.efloat[i] = microseconds(e[i]);
			row.mpfr[i]   = microseconds(m[i]);
		}
		(void)sink;
		mpfr_clears(ma, mb, ms, static_cast<mpfr_ptr>(nullptr));
		return row;
	}

}} // namespace sw::universal

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	const std::string selection = (argc > 1) ? argv[1] : "all";
	const double limit = (argc > 2) ? std::atof(argv[2]) : 0.0;
	std::printf("efloat vs MPFR %s (GMP %s) called from C++ across precisions (#1826): efloat us, efloat / MPFR\n\n",
	            mpfr_get_version(), gmp_version);
	std::printf("%6s", "bits");
	for (int i = 0; i < count; ++i) if (selected(selection, operations[i])) std::printf(" | %15s", operations[i]);
	std::printf("\n");
	int over = 0;
	double worst = 0.0;
	for (unsigned bits : precisions) {
		const Row row = measure(bits, selection);
		std::printf("%6u", bits);
		for (int i = 0; i < count; ++i) {
			if (!selected(selection, operations[i])) continue;
			if (row.mpfr[i] <= 0.0) { std::printf(" | %15s", "-"); continue; }
			const double ratio = row.efloat[i] / row.mpfr[i];
			worst = std::max(worst, ratio);
			const bool flagged = (limit > 0.0 && ratio > limit);
			if (flagged) ++over;
			std::printf(" | %8.3f %5.2f%c", row.efloat[i], ratio, flagged ? '*' : 'x');
		}
		std::printf("\n");
	}
	std::printf("\nlargest ratio %.2f", worst);
	if (limit > 0.0) std::printf(", %d cells above %.2f (marked *)", over, limit);
	std::printf("\n");
	return (over > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
