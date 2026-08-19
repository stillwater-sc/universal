// performance.cpp: elreal Phase 9 (#933) evaluation -- block-shape design study.
//
// This benchmark is the design-data generator for the McCleeary LFPERA elreal
// type (dissertation section 5.1). It measures, across the candidate storage
// block shapes (host FpType = half / bfloat16 / float / double), the quantities
// that decide a hardware block shape:
//
//   A. memory footprint per block shape (sizeof, significand width k, bits/block);
//   B. convergence rate: blocks needed to reach 50/100/200/320 decimal digits for
//      pi (Machin), e (Taylor), sqrt(2) (Newton), vs the 320-digit references;
//   C. time-to-first-block for the transcendental generators (latency);
//   D. stream-wise ZBCL dot-product throughput (vector lengths 16/64/256);
//   E. precision ceiling vs qd (quad-double, ~63 digits) -- elreal is unbounded;
//   F. cancellation-stressed accumulation (#1187) -- the regime exact accumulation
//      exists for, scored against the exact dyadic value of the same terms.
//
// Narrow hosts (half, bfloat16) are exercised best-effort: their block layer is
// validated but the division-based series can hit the host's exponent range, so
// each (host, workload) cell is guarded and reports what it actually achieved.
// This "bfloat16 saturates at N digits" datum is itself a design finding.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <utility>
#include <vector>
#include <string>
#include <string_view>
#include <random>
#include <limits>

#include <universal/number/cfloat/cfloat.hpp>          // half = cfloat<16,5,...>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/elreal_reference_digits.hpp>   // zbcl_to_dyadic, agreed_decimal_digits
#include <math/constants/reference_constants.hpp>               // s_pi, s_e, s_sqrt2

namespace {

	using namespace sw::universal;

	// median wall-clock seconds of `reps` runs of f()
	template<typename F>
	double time_seconds(F&& f, int reps = 1) {
		std::vector<double> t;
		t.reserve(static_cast<std::size_t>(reps));
		for (int i = 0; i < reps; ++i) {
			auto a = std::chrono::steady_clock::now();
			f();
			auto b = std::chrono::steady_clock::now();
			t.push_back(std::chrono::duration<double>(b - a).count());
		}
		std::sort(t.begin(), t.end());
		return t[t.size() / 2];
	}

	// a coarse depth ladder keeps the sweep O(ladder) rather than O(maxdepth)
	const std::size_t kDepthLadder[] = { 2, 4, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512 };
	const int kDigitTargets[] = { 50, 100, 200, 320 };
	// the recommendation matrix's rows: precision a caller might actually ask for
	const int kMatrixTargets[] = { 16, 32, 64, 100, 200, 300 };

	// ---- A. memory footprint per block shape ---------------------------------
	template<typename FpType>
	void report_footprint(const char* host) {
		const int k = std::numeric_limits<FpType>::digits;                 // significand width incl. hidden bit
		std::cout << "  " << std::left << std::setw(10) << host
		          << "  k(sig bits) = " << std::setw(3) << k
		          << "  sizeof(block) = " << std::setw(3) << sizeof(block<FpType>) << " B"
		          << "  payload bits/block ~ " << k << '\n';
	}

	// ---- B. convergence: blocks needed to reach digit targets ----------------
	// gen(depth) -> a ZBCL approximation of the constant. We sweep the depth
	// ladder, and for each digit target record the smallest block count whose
	// approximation agrees with the reference to at least that many digits.
	template<typename FpType, typename Gen>
	void convergence_row(const char* host, const char* cname, Gen gen, std::string_view ref) {
		int    blocksAt[4] = { -1, -1, -1, -1 };   // blocks to reach kDigitTargets[i]
		int    maxDigits = 0;
		int    maxBlocks = 0;
		bool   failed = false;
		try {
			// Sweep the full ladder: a plateau at one depth does not imply saturation
			// (a later depth may cross the next threshold), so we do not early-exit on
			// an unchanged digit count -- only when the reference precision is reached.
			for (std::size_t depth : kDepthLadder) {
				ZBCL<FpType> z = gen(depth);
				int digits = agreed_decimal_digits(z, ref);
				int blocks = static_cast<int>(z.take(1024).size());
				if (digits > maxDigits) { maxDigits = digits; maxBlocks = blocks; }
				for (int i = 0; i < 4; ++i)
					if (blocksAt[i] < 0 && digits >= kDigitTargets[i]) blocksAt[i] = blocks;
				if (digits >= 320) break;
			}
		}
		catch (const std::exception& e) { failed = true; (void)e; }

		std::cout << "  " << std::left << std::setw(10) << host << std::setw(8) << cname;
		auto cell = [](int v) { std::cout << "  " << std::right << std::setw(6) << (v < 0 ? std::string("  -  ") : std::to_string(v)); };
		for (int i = 0; i < 4; ++i) cell(blocksAt[i]);
		std::cout << "   | max " << std::right << std::setw(3) << maxDigits << " digits @ " << std::setw(4) << maxBlocks << " blocks";
		if (failed)          std::cout << "  (host exponent range exceeded)";
		else if (maxDigits == 0) std::cout << "  (does not converge on this host)";
		std::cout << '\n';
	}

	// ---- C. time-to-first-block (latency of the transcendental generators) ---
	template<typename FpType, typename Gen>
	void first_block_latency(const char* host, const char* cname, Gen gen) {
		double secs = -1.0;
		try {
			secs = time_seconds([&]() { ZBCL<FpType> z = gen(16); volatile std::size_t n = z.take(1).size(); (void)n; }, 5);
		}
		catch (const std::exception&) { }
		std::cout << "  " << std::left << std::setw(10) << host << std::setw(8) << cname << "  ";
		if (secs < 0) std::cout << "   n/a (host range exceeded)\n";
		else std::cout << std::right << std::setw(10) << std::fixed << std::setprecision(2) << (secs * 1e6) << " us to first block\n";
	}

	// ---- D. stream-wise ZBCL dot-product throughput --------------------------
	template<typename FpType>
	void dot_throughput(const char* host, std::size_t N, std::size_t depth) {
		std::mt19937_64 rng(0xD07 + N);
		std::vector<ZBCL<FpType>> a, b;
		a.reserve(N); b.reserve(N);
		for (std::size_t i = 0; i < N; ++i) {
			double x = static_cast<double>(static_cast<std::int64_t>(rng() % 20000) - 10000) / 128.0;
			double y = static_cast<double>(static_cast<std::int64_t>(rng() % 20000) - 10000) / 128.0;
			a.push_back(from_native<FpType>(x));
			b.push_back(from_native<FpType>(y));
		}
		double secs = time_seconds([&]() {
			ZBCL<FpType> acc = from_native<FpType>(0.0);
			for (std::size_t i = 0; i < N; ++i) acc = add(acc, mul(a[i], b[i], depth));
			volatile std::size_t n = acc.take(1).size(); (void)n;
		}, 5);
		double dotsPerSec = 1.0 / secs;
		std::cout << "  " << std::left << std::setw(10) << host << "  N = " << std::right << std::setw(4) << N
		          << "  " << std::setw(10) << std::fixed << std::setprecision(2) << (secs * 1e6) << " us/dot"
		          << "  (" << std::setprecision(0) << dotsPerSec << " dots/s)\n";
	}

	// ---- E. precision ceiling: elreal vs qd ----------------------------------
	void precision_ceiling() {
		std::cout << "\nE. precision ceiling vs qd (pi, digits agreeing with the 320-digit reference)\n";
		// elreal<double> pushed to high depth
		ZBCL<double> zpi = pi_zbcl<double>(96);
		int elrealDigits = agreed_decimal_digits(zpi, s_pi);
		// qd pi: convert the qd's four stored limbs to a dyadic and compare
		qd qpi = qd_pi;   // qd's built-in pi constant (4x double)
		dyadic dqpi = dyadic::from_double(qpi[0]) + dyadic::from_double(qpi[1]) + dyadic::from_double(qpi[2]) + dyadic::from_double(qpi[3]);
		int qdDigits = agreed_decimal_digits(dqpi, s_pi);
		std::cout << "  qd            : " << std::right << std::setw(3) << qdDigits << " digits  (fixed ceiling, ~4x double = 63 digits)\n";
		std::cout << "  elreal<double>: " << std::right << std::setw(3) << elrealDigits << " digits  (unbounded; depth 96 shown)\n";
	}

	// ---- F. cancellation-stressed accumulation (#1187) -----------------------
	// Two workloads in the regime the type exists for: sums whose individual terms
	// dwarf their own total. Both are scored against the exact dyadic value of the
	// *same* terms, so what is being measured is the accumulator and nothing else.

	// a qd is a 4-limb non-overlapping expansion; its exact value is the sum of
	// its limbs, which is a dyadic rational
	dyadic qd_to_dyadic(const qd& q) {
		dyadic d;
		for (int i = 0; i < 4; ++i) d = d + dyadic::from_double(q[i]);
		return d;
	}

	// A generated term is only usable if it survives binary64 intact: neither factor
	// flushed to zero, the product finite and non-zero, and the 26 x 26 -> 52-bit
	// integer product exact. Checked with integers rather than an fma residual,
	// because a platform with a sloppy software fma would false-positive.
	bool representable_term(double x, double y) {
		if (x == 0.0 || y == 0.0) return false;
		double p = x * y;
		if (!std::isfinite(p) || p == 0.0) return false;
		int ex = 0, ey = 0;
		double mx = std::frexp(x, &ex), my = std::frexp(y, &ey);
		// mantissas are 26-bit integers scaled into [0.5,1); their product needs 52
		// bits, which binary64 carries exactly so long as nothing under/overflowed
		std::uint64_t ix = static_cast<std::uint64_t>(std::ldexp(std::fabs(mx), 26));
		std::uint64_t iy = static_cast<std::uint64_t>(std::ldexp(std::fabs(my), 26));
		return (ix * iy) < (1ull << 53);
	}

	// F1. naive Taylor exp(-40): the textbook catastrophic-cancellation series.
	// Terms are generated by the naive recurrence t_k = t_{k-1} * (-40/k) in
	// double, which is what "naive" means here -- each term arrives pre-rounded.
	void cancellation_taylor() {
		std::cout << "\nF1. naive Taylor exp(-40) -- cancellation in the summation\n";

		std::vector<double> terms;
		double t = 1.0;
		terms.push_back(t);
		for (int k = 1; k <= 200; ++k) {
			t *= -40.0 / static_cast<double>(k);
			terms.push_back(t);
			if (k > 60 && std::fabs(t) < 1e-40) break;
		}
		double maxTerm = 0.0;
		for (double v : terms) maxTerm = std::max(maxTerm, std::fabs(v));

		dyadic exactSum;                                  // exact sum of those doubles
		for (double v : terms) exactSum = exactSum + dyadic::from_double(v);

		double naive = 0.0;
		for (double v : terms) naive += v;
		qd q(0.0);
		for (double v : terms) q += qd(v);
		ZBCL<double> z = from_native<double>(0.0);
		for (double v : terms) z = add(z, from_native<double>(v));

		const double trueValue = std::exp(-40.0);
		dyadic trueDyadic = dyadic::from_double(trueValue);

		std::cout << "  " << terms.size() << " terms, max|term| = " << std::scientific
		          << std::setprecision(3) << maxTerm << ", exp(-40) = " << trueValue
		          << ", condition ~ " << (maxTerm / trueValue) << "\n";
		std::cout << "  digits agreeing with the EXACT SUM OF THE SAME TERMS:\n";
		std::cout << "    double " << agreed_decimal_digits(dyadic::from_double(naive), exactSum)
		          << "    qd " << agreed_decimal_digits(qd_to_dyadic(q), exactSum)
		          << "    elreal " << agreed_decimal_digits(zbcl_to_dyadic(z), exactSum) << "\n";
		std::cout << "  but that exact sum agrees with exp(-40) to only "
		          << agreed_decimal_digits(exactSum, trueDyadic) << " digits:\n"
		          << "    the terms were rounded before any accumulator saw them, so exact\n"
		          << "    accumulation is necessary here and nowhere near sufficient.\n";

		// elreal's own exp() reduces the argument instead of summing naively, so it
		// never walks into this cancellation at all.
		ZBCL<double> ex = sw::universal::exp(from_native<double>(-40.0), 8);
		std::cout << "  elreal exp(-40) (argument-reduced, not naive) agrees with std::exp(-40)\n"
		          << "    to " << agreed_decimal_digits(zbcl_to_dyadic(ex), trueDyadic)
		          << " digits -- all the double reference carries.\n";
	}

	// F2. ill-conditioned dot product. The answer is deliberately spread over
	// `chunks` 52-bit pieces separated by 100 bits, so past four chunks a 4-limb
	// type cannot represent the answer even in principle; large +/- pairs supply
	// the cancellation. Every product is 26 bits x 26 bits, hence exact in double,
	// so product rounding is not part of what is being measured.
	void cancellation_dot() {
		std::cout << "\nF2. ill-conditioned dot product -- digits agreeing with the exact dot\n";
		std::cout << "  chunks  answer spans   double     qd   elreal\n";
		for (int chunks : { 2, 4, 6, 8, 12 }) {
			std::mt19937_64 rng(0xBEEF + static_cast<unsigned>(chunks));
			std::vector<double> X, Y;
			dyadic D;
			int unrepresentable = 0;
			// Centre the answer on 2^0. Anchoring the top chunk at 2^0 instead would
			// push the bottom one to 2^(-100*(chunks-1)), and at 12 chunks that is
			// 2^-1100 -- below the smallest subnormal, so the chunk would quietly
			// round to zero and the row would report a span it does not have.
			const int hi = 50 * (chunks - 1);
			for (int j = 0; j < chunks; ++j) {
				double xv = std::ldexp(static_cast<double>((rng() & 0x3FFFFFFull) | (1ull << 25)), hi - 100 * j - 25);
				double yv = static_cast<double>((rng() & 0x3FFFFFFull) | (1ull << 25));
				if (!representable_term(xv, yv)) ++unrepresentable;
				X.push_back(xv); Y.push_back(yv);
				D = D + dyadic::from_double(xv) * dyadic::from_double(yv);
			}
			for (int j = 0; j < 12; ++j) {                     // exact cancellation well above the answer
				double b  = std::ldexp(static_cast<double>((rng() % 4000) + 1), 850);
				double yb = static_cast<double>((rng() % 4000) + 1);
				if (!representable_term(b, yb)) ++unrepresentable;
				X.push_back(b);  Y.push_back(yb);
				X.push_back(-b); Y.push_back(yb);
			}
			// shuffle jointly: adjacent +P/-P would cancel with no rounding at all,
			// which is the opposite of the stress this benchmark applies
			for (std::size_t i = X.size(); i > 1; --i) {
				std::size_t k = static_cast<std::size_t>(rng() % i);
				std::swap(X[i - 1], X[k]); std::swap(Y[i - 1], Y[k]);
			}

			double dn = 0.0;
			for (std::size_t i = 0; i < X.size(); ++i) dn += X[i] * Y[i];
			qd dq(0.0);
			for (std::size_t i = 0; i < X.size(); ++i) dq += qd(X[i]) * qd(Y[i]);
			ZBCL<double> dz = from_native<double>(0.0);
			for (std::size_t i = 0; i < X.size(); ++i)
				dz = add(dz, mul(from_native<double>(X[i]), from_native<double>(Y[i]), 8));

			std::cout << std::setw(8) << chunks
			          << std::setw(11) << (100 * (chunks - 1) + 52) << " bits"
			          << std::setw(9) << agreed_decimal_digits(dyadic::from_double(dn), D)
			          << std::setw(7) << agreed_decimal_digits(qd_to_dyadic(dq), D)
			          << std::setw(9) << agreed_decimal_digits(zbcl_to_dyadic(dz), D);
			if (unrepresentable) std::cout << "   !! " << unrepresentable << " terms not representable";
			std::cout << '\n';
		}
		std::cout << "  (320 is the reference cap, i.e. exact as far as the oracle can see)\n";
	}

	// ---- H. precision target -> latency (#1188, partial) ---------------------
	// The block-shape recommendation asks "for X precision at Y latency, pick Z".
	// Sections B and C answer that in two halves -- blocks-to-precision and
	// time-to-first-block -- and this joins them: the wall time to actually
	// produce pi at a given number of correct digits, per host.
	//
	// One pass over the depth ladder per host, reading every target off the
	// resulting curve, rather than a search per target. A single timing rep: the
	// useful signal here spans two orders of magnitude between hosts, and the
	// deep-double cells cost most of a second each.
	template<typename FpType>
	void precision_latency_curve(const char* host) {
		const std::size_t ladder[] = { 2, 4, 6, 8, 12, 16, 20 };
		std::vector<std::pair<int, double>> pts;   // (digits reached, seconds)
		for (std::size_t d : ladder) {
			int digits = 0;
			try { digits = agreed_decimal_digits(pi_zbcl<FpType>(d), s_pi); }
			catch (const std::exception&) { break; }
			double secs = time_seconds([&] {
				ZBCL<FpType> z = pi_zbcl<FpType>(d);
				volatile std::size_t n = z.take(1024).size(); (void)n;
			}, 1);
			pts.push_back({ digits, secs });
			if (digits >= 320) break;
		}
		for (int target : kMatrixTargets) {
			const auto it = std::find_if(pts.begin(), pts.end(),
				[&](const auto& p) { return p.first >= target; });
			std::cout << "  " << std::left << std::setw(11) << host
			          << std::right << std::setw(6) << target << "   ";
			if (it == pts.end()) std::cout << "     unreachable\n";
			else std::cout << std::fixed << std::setprecision(1) << std::setw(12) << (it->second * 1e6) << " us\n";
		}
	}

	void precision_latency_matrix() {
		std::cout << "\nH. wall time to produce pi at a given precision, per host\n";
		std::cout << "  host        digits           time\n";
		precision_latency_curve<float>("float");
		precision_latency_curve<bfloat16>("bfloat16");
		precision_latency_curve<double>("double");

		// the fixed-size alternatives, for the same targets
		dyadic ddpi = dyadic::from_double(dd_pi[0]) + dyadic::from_double(dd_pi[1]);
		dyadic qdpi;
		for (int i = 0; i < 4; ++i) qdpi = qdpi + dyadic::from_double(qd_pi[i]);
		std::cout << "  fixed-size alternatives, as compile-time constants:\n"
		          << "    double " << agreed_decimal_digits(dyadic::from_double(3.141592653589793), s_pi)
		          << " digits,  dd " << agreed_decimal_digits(ddpi, s_pi)
		          << " digits,  qd " << agreed_decimal_digits(qdpi, s_pi) << " digits\n";
	}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	std::cout << "elreal Phase 9 (#933) block-shape design study\n";
	std::cout << "==============================================\n";

	std::cout << "\nA. memory footprint per block shape\n";
	report_footprint<half>("half");
	report_footprint<bfloat16>("bfloat16");
	report_footprint<float>("float");
	report_footprint<double>("double");

	std::cout << "\nB. convergence: blocks needed to reach {50,100,200,320} decimal digits\n";
	std::cout << "  host      const     b@50    b@100   b@200   b@320   | saturation\n";
	// pi (Machin), e (Taylor), sqrt2 (Newton)
	convergence_row<half>    ("half",     "pi",    [](std::size_t d) { return pi_zbcl<half>(d); },    s_pi);
	convergence_row<bfloat16>("bfloat16", "pi",    [](std::size_t d) { return pi_zbcl<bfloat16>(d); },s_pi);
	convergence_row<float>   ("float",    "pi",    [](std::size_t d) { return pi_zbcl<float>(d); },   s_pi);
	convergence_row<double>  ("double",   "pi",    [](std::size_t d) { return pi_zbcl<double>(d); },  s_pi);
	convergence_row<half>    ("half",     "e",     [](std::size_t d) { return e_zbcl<half>(d); },     s_e);
	convergence_row<bfloat16>("bfloat16", "e",     [](std::size_t d) { return e_zbcl<bfloat16>(d); }, s_e);
	convergence_row<float>   ("float",    "e",     [](std::size_t d) { return e_zbcl<float>(d); },    s_e);
	convergence_row<double>  ("double",   "e",     [](std::size_t d) { return e_zbcl<double>(d); },   s_e);
	convergence_row<half>    ("half",     "sqrt2", [](std::size_t d) { return sqrt2_zbcl<half>(d); }, s_sqrt2);
	convergence_row<bfloat16>("bfloat16", "sqrt2", [](std::size_t d) { return sqrt2_zbcl<bfloat16>(d); }, s_sqrt2);
	convergence_row<float>   ("float",    "sqrt2", [](std::size_t d) { return sqrt2_zbcl<float>(d); }, s_sqrt2);
	convergence_row<double>  ("double",   "sqrt2", [](std::size_t d) { return sqrt2_zbcl<double>(d); }, s_sqrt2);

	std::cout << "\nC. time-to-first-block (transcendental generators, depth 16)\n";
	first_block_latency<float> ("float",  "pi",    [](std::size_t d) { return pi_zbcl<float>(d); });
	first_block_latency<double>("double", "pi",    [](std::size_t d) { return pi_zbcl<double>(d); });
	first_block_latency<float> ("float",  "e",     [](std::size_t d) { return e_zbcl<float>(d); });
	first_block_latency<double>("double", "e",     [](std::size_t d) { return e_zbcl<double>(d); });
	first_block_latency<float> ("float",  "sqrt2", [](std::size_t d) { return sqrt2_zbcl<float>(d); });
	first_block_latency<double>("double", "sqrt2", [](std::size_t d) { return sqrt2_zbcl<double>(d); });

	std::cout << "\nD. stream-wise ZBCL dot-product throughput (depth 32)\n";
	for (std::size_t N : { std::size_t(16), std::size_t(64), std::size_t(256) }) dot_throughput<float> ("float",  N, 32);
	for (std::size_t N : { std::size_t(16), std::size_t(64), std::size_t(256) }) dot_throughput<double>("double", N, 32);

	precision_ceiling();

	cancellation_taylor();
	cancellation_dot();
	precision_latency_matrix();

	std::cout << "\ndone.\n";
	return EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
