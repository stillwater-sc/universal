// division.cpp: cost of multi-block integer and blockbinary division (#1744)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// performance.cpp's DivisionWorkload divides by Scalar(1.00001f), which an integer holds as 1, so
// it never exercises the long division. This divides random full-width dividends by random
// half-width divisors, the shape that costs the most quotient digits.
//
// The one guard is a ratio, so that it holds at any optimization level and on any runner: at
// 1024 bits a division may cost at most 5 multiplications of the same operands. Long division a
// word at a time costs less than one; the bit-serial loop it replaced cost about 20.

#include <universal/utility/directives.hpp>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <universal/number/integer/integer.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {

	// mean microseconds of one a / b over reps divisions of K prepared pairs
	template<typename Number>
	double DivisionMicroseconds(int reps) {
		constexpr int K = 64;
		std::mt19937_64 rng(1744);
		Number a[K], b[K];
		for (int i = 0; i < K; ++i) {
			a[i].clear();
			b[i].clear();
			for (unsigned k = 0; k + 1 < Number::nbits; ++k) a[i].setbit(k, (rng() & 1u) != 0);   // positive
			for (unsigned k = 0; k < Number::nbits / 2; ++k) b[i].setbit(k, (rng() & 1u) != 0);
			b[i].setbit(0);
		}
		volatile unsigned sink = 0;
		Number q;
		const auto t0 = std::chrono::steady_clock::now();
		for (int i = 0; i < reps; ++i) {
			q = a[i % K] / b[i % K];
			sink = sink + static_cast<unsigned>(q.block(0));
		}
		const auto t1 = std::chrono::steady_clock::now();
		(void)sink;
		return std::chrono::duration<double, std::micro>(t1 - t0).count() / reps;
	}

	// mean microseconds of one a * b on the same kind of operands
	template<typename Number>
	double MultiplicationMicroseconds(int reps) {
		constexpr int K = 64;
		std::mt19937_64 rng(1744);
		Number a[K], b[K];
		for (int i = 0; i < K; ++i) {
			a[i].clear();
			b[i].clear();
			for (unsigned k = 0; k + 1 < Number::nbits; ++k) a[i].setbit(k, (rng() & 1u) != 0);
			for (unsigned k = 0; k < Number::nbits / 2; ++k) b[i].setbit(k, (rng() & 1u) != 0);
			b[i].setbit(0);
		}
		volatile unsigned sink = 0;
		Number p;
		const auto t0 = std::chrono::steady_clock::now();
		for (int i = 0; i < reps; ++i) {
			p = a[i % K] * b[i % K];
			sink = sink + static_cast<unsigned>(p.block(0));
		}
		const auto t1 = std::chrono::steady_clock::now();
		(void)sink;
		return std::chrono::duration<double, std::micro>(t1 - t0).count() / reps;
	}

	template<typename Number>
	double Report(const char* name, int reps) {
		const double us = DivisionMicroseconds<Number>(reps);
		std::printf("%-30s %10.3f us\n", name, us);
		return us;
	}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;
	constexpr BinaryNumberType Signed = BinaryNumberType::Signed;

	std::cout << "multi-block division, random full-width dividend / half-width divisor (#1744)\n";
	Report<integer<  64, std::uint32_t>>("integer<64,uint32_t>",   200000);
	Report<integer< 128, std::uint32_t>>("integer<128,uint32_t>", 100000);
	Report<integer< 256, std::uint32_t>>("integer<256,uint32_t>",   50000);
	const double t1024 = Report<integer<1024, std::uint32_t>>("integer<1024,uint32_t>", 10000);
	Report<integer<4096, std::uint32_t>>("integer<4096,uint32_t>",   1000);
	Report<integer< 128, std::uint8_t >>("integer<128,uint8_t>",    50000);
	Report<integer< 128, std::uint64_t>>("integer<128,uint64_t>",  100000);
	Report<blockbinary< 256, std::uint32_t, Signed>>("blockbinary<256,uint32_t>", 50000);
	Report<blockbinary<1024, std::uint32_t, Signed>>("blockbinary<1024,uint32_t>", 5000);

	int failures = 0;
	const double mul1024 = MultiplicationMicroseconds<integer<1024, std::uint32_t>>(10000);
	const double ratio = t1024 / mul1024;
	std::printf("\ninteger<1024> division / multiplication: %.1f (guard: below 5)\n", ratio);
	if (!(ratio < 5.0)) {
		std::cout << "  *** REGRESSION: a 1024-bit division costs " << ratio << " multiplications -- a bit-serial division may have returned (#1744) ***\n";
		++failures;
	}

	std::cout << "\n" << (failures == 0 ? "PASS" : "FAIL") << " integer division costs within bounds\n";
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
