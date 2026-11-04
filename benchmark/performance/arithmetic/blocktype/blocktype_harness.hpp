#pragma once
// blocktype_harness.hpp: measure the limb-processing cost of one number type stored in different BlockTypes (#1904)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A cfloat or posit of nbits bits stored in BlockType bt is processed as nrBlocks = ceil(nbits / bits(bt))
// limbs. This harness times the same operations on the same values for every BlockType at a given nbits,
// so that the only thing that changes between the rows of a table is the limb layout.
//
// Methodology
//   - operands: a pool of N values per nbits, built at run time from a seeded std::mt19937_64 as a sum
//     of overlapping random doubles, so the significand is random across its full width, not only in its
//     top 53 bits. The pool is built in the target type itself, so every BlockType sees the same values
//     (checked: see below).
//   - dead-code elimination: every pass stores its results to memory and then executes a compiler
//     barrier that makes that memory observable; comparisons accumulate into a volatile sink.
//   - timing: one warming pass, then the number of passes per sample is calibrated so that one sample
//     takes at least min_sample_seconds; the reported figure is the median of `samples` samples, in
//     nanoseconds per operation.
//   - cross-check: after timing, the results of every operation on the first values of the pool are
//     digested from their bit patterns. The digests must be identical for every BlockType at one nbits,
//     because the BlockType changes the storage, not the value. A difference is reported as a failure.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <random>
#include <string>
#include <vector>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace sw { namespace universal { namespace blocktype_benchmark {

	// make the memory behind p observable, so the stores of a pass cannot be removed or hoisted
	inline void escape(const void* p) {
#if defined(__GNUC__) || defined(__clang__)
		asm volatile("" : : "g"(p) : "memory");
#else
		static const void* volatile observed;
		observed = p;
		_ReadWriteBarrier();
#endif
	}

	inline volatile std::uint64_t sink = 0;

	enum Op : unsigned { Add, Sub, Mul, Div, Lt, Copy, FromDouble, ToDouble, Sqrt, NrOps };
	inline const char* const opName[NrOps] = { "add", "sub", "mul", "div", "lt", "copy", "fromdbl", "todbl", "sqrt" };

	inline const char* const btName[4] = { "uint8_t", "uint16_t", "uint32_t", "uint64_t" };
	constexpr unsigned nrBlockTypes = 4;

	struct Options {
		std::size_t poolSize;          // N values per operand array
		int         samples;           // the median is taken over this many samples
		double      minSampleSeconds;  // the passes per sample are calibrated to at least this
	};

	// one row of a table: the median ns/op of every operation, and the digest of its results
	struct Row {
		double        ns[NrOps];
		std::uint64_t digest[NrOps];
	};

	// one table: rows nbits x BlockType
	struct Table {
		std::vector<unsigned> nbits;
		std::vector<Row>      rows;    // rows[n * nrBlockTypes + b]
	};

	// the random terms that build one pool: values[i] = sum over k of terms[i][k], in the target type
	using Terms = std::vector<std::vector<double>>;

	// operand terms for an nbits-bit type: a leading term sign * [1, 2) * 2^e with e in [-16, 16],
	// followed by random terms 50 bits further down each, until the type's significand is covered
	inline Terms makeTerms(unsigned nbits, std::size_t N, std::uint64_t seed) {
		std::mt19937_64 rng(seed + nbits);
		std::uniform_real_distribution<double> lead(1.0, 2.0);
		std::uniform_real_distribution<double> tail(-1.0, 1.0);
		const unsigned K = nbits / 50 + 2;
		Terms terms(N, std::vector<double>(K));
		for (auto& t : terms) {
			const int e = int(rng() % 33) - 16;
			const double s = (rng() & 1) ? -1.0 : 1.0;
			t[0] = s * std::ldexp(lead(rng), e);
			for (unsigned k = 1; k < K; ++k) t[k] = std::ldexp(tail(rng), e - int(50 * k));
		}
		return terms;
	}

	// median nanoseconds per operation of `pass`, which performs opsPerPass operations
	template<typename Pass>
	double medianNs(Pass&& pass, std::size_t opsPerPass, const Options& opt) {
		using clock = std::chrono::steady_clock;
		pass();   // warm the caches and the branch predictors
		long passes = 1;
		for (;;) {
			const auto t0 = clock::now();
			for (long p = 0; p < passes; ++p) pass();
			const double s = std::chrono::duration<double>(clock::now() - t0).count();
			if (s >= opt.minSampleSeconds) break;
			passes *= (s < opt.minSampleSeconds / 10.0) ? 10 : 2;
		}
		std::vector<double> sample(static_cast<std::size_t>(opt.samples));
		for (auto& v : sample) {
			const auto t0 = clock::now();
			for (long p = 0; p < passes; ++p) pass();
			const double ns = std::chrono::duration<double, std::nano>(clock::now() - t0).count();
			v = ns / (double(passes) * double(opsPerPass));
		}
		std::nth_element(sample.begin(), sample.begin() + sample.size() / 2, sample.end());
		return sample[sample.size() / 2];
	}

	// FNV-1a over the bits of to_binary(v), ignoring the separators, so the digest depends on the
	// encoding only and not on how the type prints it
	template<typename T>
	void digestBits(std::uint64_t& h, const T& v) {
		for (char c : to_binary(v)) {
			if (c != '0' && c != '1') continue;
			h ^= static_cast<std::uint64_t>(c);
			h *= 0x100000001b3ull;
		}
	}
	inline void digestWord(std::uint64_t& h, std::uint64_t w) {
		for (int i = 0; i < 8; ++i) { h ^= (w >> (8 * i)) & 0xFFu; h *= 0x100000001b3ull; }
	}

	// time every operation of type T on the pool built from terms
	template<typename T>
	Row measure(const Terms& terms, const Options& opt) {
		const std::size_t N = terms.size();
		std::vector<T> a(N), b(N), pos(N), c(N);
		std::vector<double> d(N), out(N);
		for (std::size_t i = 0; i < N; ++i) {
			const auto& ta = terms[i];
			const auto& tb = terms[(i + N / 2) % N];
			T x(ta[0]), y(tb[0]);
			for (std::size_t k = 1; k < ta.size(); ++k) { x += T(ta[k]); y += T(tb[k]); }
			a[i] = x;
			b[i] = y;
			pos[i] = (x < T(0)) ? -x : x;
			d[i] = ta[0] + ta[1];
		}

		Row row{};
		auto binary = [&](auto&& f) {
			return [&, f]() {
				escape(a.data()); escape(b.data());
				for (std::size_t i = 0; i < N; ++i) c[i] = f(a[i], b[i]);
				escape(c.data());
			};
		};
		row.ns[Add] = medianNs(binary([](const T& x, const T& y) { return x + y; }), N, opt);
		row.ns[Sub] = medianNs(binary([](const T& x, const T& y) { return x - y; }), N, opt);
		row.ns[Mul] = medianNs(binary([](const T& x, const T& y) { return x * y; }), N, opt);
		row.ns[Div] = medianNs(binary([](const T& x, const T& y) { return x / y; }), N, opt);
		row.ns[Lt] = medianNs([&]() {
			escape(a.data()); escape(b.data());
			std::uint64_t n = 0;
			for (std::size_t i = 0; i < N; ++i) n += (a[i] < b[i]) ? 1u : 0u;
			sink = sink + n;
		}, N, opt);
		row.ns[Copy] = medianNs([&]() {
			escape(a.data());
			for (std::size_t i = 0; i < N; ++i) c[i] = a[i];
			escape(c.data());
		}, N, opt);
		row.ns[FromDouble] = medianNs([&]() {
			escape(d.data());
			for (std::size_t i = 0; i < N; ++i) c[i] = T(d[i]);
			escape(c.data());
		}, N, opt);
		row.ns[ToDouble] = medianNs([&]() {
			escape(a.data());
			for (std::size_t i = 0; i < N; ++i) out[i] = double(a[i]);
			escape(out.data());
		}, N, opt);
		row.ns[Sqrt] = medianNs([&]() {
			escape(pos.data());
			for (std::size_t i = 0; i < N; ++i) c[i] = sqrt(pos[i]);
			escape(c.data());
		}, N, opt);

		// digest the results on the first values of the pool
		const std::size_t M = std::min<std::size_t>(N, 64);
		for (unsigned op = 0; op < NrOps; ++op) row.digest[op] = 0xcbf29ce484222325ull;
		for (std::size_t i = 0; i < M; ++i) {
			digestBits(row.digest[Add], T(a[i] + b[i]));
			digestBits(row.digest[Sub], T(a[i] - b[i]));
			digestBits(row.digest[Mul], T(a[i] * b[i]));
			digestBits(row.digest[Div], T(a[i] / b[i]));
			digestWord(row.digest[Lt], (a[i] < b[i]) ? 1u : 0u);
			digestBits(row.digest[Copy], a[i]);
			digestBits(row.digest[FromDouble], T(d[i]));
			double dv = double(a[i]);
			std::uint64_t w = 0;
			static_assert(sizeof(w) == sizeof(dv), "double is not 64 bits");
			std::memcpy(&w, &dv, sizeof(w));
			digestWord(row.digest[ToDouble], w);
			digestBits(row.digest[Sqrt], T(sqrt(pos[i])));
		}
		return row;
	}

	// measure T<bt> for the four BlockTypes at one nbits; Make<bt> names the type
	template<unsigned nbits, template<typename> class Make>
	void measureAll(Table& table, const Options& opt, std::uint64_t seed) {
		const Terms terms = makeTerms(nbits, opt.poolSize, seed);
		table.nbits.push_back(nbits);
		table.rows.push_back(measure<typename Make<std::uint8_t>::type>(terms, opt));
		table.rows.push_back(measure<typename Make<std::uint16_t>::type>(terms, opt));
		table.rows.push_back(measure<typename Make<std::uint32_t>::type>(terms, opt));
		table.rows.push_back(measure<typename Make<std::uint64_t>::type>(terms, opt));
		std::fprintf(stderr, "  measured nbits = %u\n", nbits);
	}

	// the number of (nbits, op) cells whose digests differ between BlockTypes
	inline int crossCheck(const char* typeName, const Table& table) {
		int failures = 0;
		for (std::size_t n = 0; n < table.nbits.size(); ++n) {
			const Row& ref = table.rows[n * nrBlockTypes + nrBlockTypes - 1];
			for (unsigned b = 0; b + 1 < nrBlockTypes; ++b) {
				const Row& row = table.rows[n * nrBlockTypes + b];
				for (unsigned op = 0; op < NrOps; ++op) {
					if (row.digest[op] != ref.digest[op]) {
						std::printf("FAIL: %s nbits %u %s: %s results differ from uint64_t\n",
						            typeName, table.nbits[n], btName[b], opName[op]);
						++failures;
					}
				}
			}
		}
		return failures;
	}

	inline std::string compilerName() {
#if defined(__clang__)
		return std::string("clang ") + __clang_version__;
#elif defined(__GNUC__)
		return std::string("gcc ") + __VERSION__;
#elif defined(_MSC_VER)
		return "MSVC " + std::to_string(_MSC_FULL_VER);
#else
		return "unknown compiler";
#endif
	}

	inline std::string cpuName() {
		std::ifstream cpuinfo("/proc/cpuinfo");
		std::string line;
		while (std::getline(cpuinfo, line)) {
			if (line.rfind("model name", 0) == 0) {
				auto colon = line.find(':');
				if (colon != std::string::npos) return line.substr(colon + 2);
			}
		}
		return "unknown CPU";
	}

	inline void printHeader(const char* title, const Options& opt) {
		std::printf("%s\n", title);
		std::printf("compiler : %s\n", compilerName().c_str());
		std::printf("cpu      : %s\n", cpuName().c_str());
#ifdef NDEBUG
		std::printf("build    : NDEBUG (optimized)\n");
#else
		std::printf("build    : no NDEBUG: timings are not representative of a Release build\n");
#endif
		std::printf("method   : median of %d samples of >= %.0f ms, pool of %zu values per operand\n\n",
		            opt.samples, 1000.0 * opt.minSampleSeconds, opt.poolSize);
	}

	// one table of ns/op and one of the ratio to the uint64_t row at the same nbits
	inline void printTable(const char* typeName, const Table& table) {
		std::printf("%s: ns/op\n", typeName);
		std::printf("%5s %-9s", "nbits", "bt");
		for (unsigned op = 0; op < NrOps; ++op) std::printf(" %9s", opName[op]);
		std::printf("\n");
		for (std::size_t n = 0; n < table.nbits.size(); ++n) {
			for (unsigned b = 0; b < nrBlockTypes; ++b) {
				const Row& row = table.rows[n * nrBlockTypes + b];
				std::printf("%5u %-9s", table.nbits[n], btName[b]);
				for (unsigned op = 0; op < NrOps; ++op) std::printf(" %9.2f", row.ns[op]);
				std::printf("\n");
			}
		}
		std::printf("\n%s: ratio to the uint64_t row at the same nbits\n", typeName);
		std::printf("%5s %-9s", "nbits", "bt");
		for (unsigned op = 0; op < NrOps; ++op) std::printf(" %9s", opName[op]);
		std::printf("\n");
		for (std::size_t n = 0; n < table.nbits.size(); ++n) {
			const Row& ref = table.rows[n * nrBlockTypes + nrBlockTypes - 1];
			for (unsigned b = 0; b < nrBlockTypes; ++b) {
				const Row& row = table.rows[n * nrBlockTypes + b];
				std::printf("%5u %-9s", table.nbits[n], btName[b]);
				for (unsigned op = 0; op < NrOps; ++op) std::printf(" %9.2f", row.ns[op] / ref.ns[op]);
				std::printf("\n");
			}
		}
		std::printf("\n");
	}

}}}  // namespace sw::universal::blocktype_benchmark
