// takum16.cpp: performance of takum<16,3,uint16_t>, generic or fast (TAKUM_FAST_TAKUM_16)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Built twice by CMake: benchmark_takum_takum16 (generic) and benchmark_takum_takum16_fast.
// "narrow" operands lie in [-2, 2]; "wide" ones are spread over 2^+/-60, so the fast
// path's 512 KB decode table is touched across its whole range.
#include <universal/number/takum/takum.hpp>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

template<typename F> double ns_per_op(F f, double nops, int reps) {
	double best = 1e30;
	for (int r = 0; r < reps; ++r) {
		auto t0 = std::chrono::steady_clock::now();
		f();
		double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / nops;
		if (ns < best) best = ns;
	}
	return best;
}

volatile double sink;

template<typename T> void run(const char* name, bool wide) {
	std::mt19937_64 rng(1);
	std::uniform_real_distribution<double> ud(-2, 2);
	auto gen = [&] { return wide ? ud(rng) * std::ldexp(1.0, int(rng() % 120) - 60) : ud(rng); };
	const std::size_t N = 4096, BIG = 1000000, M = 64, K = 50;
	std::vector<T> a(BIG), b(BIG), c(N);
	std::vector<double> d(N);
	for (std::size_t i = 0; i < BIG; ++i) { a[i] = T(gen()); b[i] = T(gen()); }
	for (auto& x : d) x = gen();
	double mul = ns_per_op([&] { for (std::size_t k = 0; k < K; ++k) for (std::size_t i = 0; i < N; ++i) c[i] = a[i] * b[(i + k) % N]; sink = double(c[7]); }, double(K * N), 5);
	double add = ns_per_op([&] { for (std::size_t k = 0; k < K; ++k) for (std::size_t i = 0; i < N; ++i) c[i] = a[i] + b[(i + k) % N]; sink = double(c[7]); }, double(K * N), 5);
	double madd = ns_per_op([&] { T acc(0); for (std::size_t k = 0; k < K; ++k) for (std::size_t i = 0; i < N; ++i) acc += a[i] * b[i]; sink = double(acc); }, double(K * N), 5);
	double dot = ns_per_op([&] { T acc(0); for (std::size_t i = 0; i < BIG; ++i) acc += a[i] * b[i]; sink = double(acc); }, double(BIG), 3);
	double mm = ns_per_op([&] {
		for (std::size_t i = 0; i < M; ++i) for (std::size_t j = 0; j < M; ++j) {
			T s(0);
			for (std::size_t k = 0; k < M; ++k) s += a[i * M + k] * b[k * M + j];
			c[i * M + j] = s;
		}
		sink = double(c[5]); }, double(M * M * M), 3);
	double cvt = ns_per_op([&] { for (std::size_t k = 0; k < K; ++k) for (std::size_t i = 0; i < N; ++i) c[i] = T(d[i]); sink = double(c[9]); }, double(K * N), 5);
	std::printf("%-22s %-6s  mul %7.2f  add %7.2f  madd %7.2f  dot1e6 %7.2f  matmul64 %7.2f  from_double %7.2f  ns/op\n",
	            name, wide ? "wide" : "narrow", mul, add, madd, dot, mm, cvt);
}

int main() {
	using T = sw::universal::takum<16, 3, std::uint16_t>;
	const char* name = TAKUM_FAST_TAKUM_16 ? "takum16 fast" : "takum16 generic";
	run<T>(name, false);
	run<T>(name, true);
	return EXIT_SUCCESS;
}
