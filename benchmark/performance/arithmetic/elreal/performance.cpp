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
//   E. precision ceiling vs qd (quad-double, ~63 digits) -- elreal is unbounded.
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
#include <chrono>
#include <iostream>
#include <iomanip>
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
		int    stall = 0;                          // ladder steps with no digit gain
		bool   failed = false;
		try {
			for (std::size_t depth : kDepthLadder) {
				ZBCL<FpType> z = gen(depth);
				int digits = agreed_decimal_digits(z, ref);
				int blocks = static_cast<int>(z.take(1024).size());
				if (digits > maxDigits) { maxDigits = digits; maxBlocks = blocks; stall = 0; }
				else ++stall;
				for (int i = 0; i < 4; ++i)
					if (blocksAt[i] < 0 && digits >= kDigitTargets[i]) blocksAt[i] = blocks;
				if (digits >= 320) break;
				if (stall >= 2) break;             // saturated: more depth adds no precision
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

	std::cout << "\ndone.\n";
	return EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
