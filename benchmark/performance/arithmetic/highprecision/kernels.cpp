//  kernels.cpp : composite kernel performance of the multi-component types: dd, dd_cascade, td_cascade, qd, qd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// issue #1315: scalar operator latency (scalar.cpp) is only half of the comparison. The cost of
// a multi-component implementation is dominated by renormalization, and renormalization behaves
// differently inside a kernel than it does in a dependent chain:
//
//   - dot and axpy stream independent multiplies, so a core can overlap them; a slower scalar
//     operation with more independent instructions can win back some of the gap
//   - matmul adds working-set pressure: a qd is 32 bytes per element, so a 32x32 tile is 32KB and
//     no longer fits the way a double tile does
//   - Horner is a strictly dependent chain with no ILP to hide latency, the worst case for a
//     renormalization-heavy implementation
//
// All kernels report cost per elementary multiply-add (one 'element op'), so the numbers are
// directly comparable across kernel shapes and against the scalar multiply and add.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "hp_types.hpp"

namespace {

	// operands live near 1.0, so a dot product of length N lands near N and never overflows
	template<typename Scalar>
	std::vector<Scalar> vectorOf(std::size_t N, std::uint64_t seed) {
		std::vector<double> data = hpbench::sampleData(N, seed);
		std::vector<Scalar> v(N);
		for (std::size_t i = 0; i < N; ++i) v[i] = Scalar(data[i]);
		return v;
	}

	// a kernel over constant operands computes the same answer on every call, and a compiler that
	// notices is free to hoist the whole thing out of the timing loop (clang does exactly that to
	// the double Horner evaluation). Seeding each call with a different starting value makes every
	// call a distinct computation without adding any work to the inner loop.
	constexpr std::size_t NR_SEEDS = 8;

	template<typename Scalar>
	const std::vector<Scalar>& seeds() {
		static const std::vector<Scalar> table = vectorOf<Scalar>(NR_SEEDS, 0x8888ull);
		return table;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	/// kernels
	///
	/// nrOps counts elementary multiply-add operations, so each kernel runs nrOps/workPerCall
	/// invocations of itself. That keeps ns/op comparable across kernels of different sizes.

	template<typename Scalar, std::size_t N>
	void DotWorkload(std::size_t nrOps) {
		static const std::vector<Scalar> x = vectorOf<Scalar>(N, 0x1111ull);
		static const std::vector<Scalar> y = vectorOf<Scalar>(N, 0x2222ull);
		const std::vector<Scalar>& seed = seeds<Scalar>();
		std::size_t nrCalls = nrOps / N;
		for (std::size_t call = 0; call < nrCalls; ++call) {
			Scalar sum(seed[call & (NR_SEEDS - 1)]);
			for (std::size_t i = 0; i < N; ++i) {
				sum = sum + x[i] * y[i];
			}
			hpbench::consume(sum);
		}
	}

	template<typename Scalar, std::size_t N>
	void AxpyWorkload(std::size_t nrOps) {
		static const std::vector<Scalar> x = vectorOf<Scalar>(N, 0x3333ull);
		std::vector<Scalar> y = vectorOf<Scalar>(N, 0x4444ull);
		// a is tiny, so repeated passes over y cannot run away
		const Scalar a(1.0 / 1048576.0);
		std::size_t nrCalls = nrOps / N;
		for (std::size_t call = 0; call < nrCalls; ++call) {
			for (std::size_t i = 0; i < N; ++i) {
				y[i] = a * x[i] + y[i];
			}
			hpbench::consume(y[call % N]);
		}
	}

	// a small dense matmul: the tile is deliberately sized so that a double tile fits in L1 and a
	// qd tile does not, which is exactly the effect a multi-component type has to pay for
	template<typename Scalar, std::size_t N>
	void MatmulWorkload(std::size_t nrOps) {
		static const std::vector<Scalar> A = vectorOf<Scalar>(N * N, 0x5555ull);
		static const std::vector<Scalar> B = vectorOf<Scalar>(N * N, 0x6666ull);
		std::vector<Scalar> C(N * N, Scalar(0.0));
		const std::vector<Scalar>& seed = seeds<Scalar>();
		// the harness never calibrates below one full tile (see the minOps argument at the call site)
		std::size_t nrCalls = nrOps / (N * N * N);
		for (std::size_t call = 0; call < nrCalls; ++call) {
			for (std::size_t i = 0; i < N; ++i) {
				for (std::size_t j = 0; j < N; ++j) {
					Scalar sum(seed[call & (NR_SEEDS - 1)]);
					for (std::size_t k = 0; k < N; ++k) {
						sum = sum + A[i * N + k] * B[k * N + j];
					}
					C[i * N + j] = sum;
				}
			}
			hpbench::consume(C[call % (N * N)]);
		}
	}

	// Horner: a strictly dependent chain of multiply-add, no instruction level parallelism
	template<typename Scalar, std::size_t DEGREE>
	void HornerWorkload(std::size_t nrOps) {
		static const std::vector<Scalar> coefficients = vectorOf<Scalar>(DEGREE + 1, 0x7777ull);
		// |x| < 1 so that a degree-20 evaluation stays bounded
		static const Scalar x = Scalar(0.5);
		const std::vector<Scalar>& seed = seeds<Scalar>();
		std::size_t nrCalls = nrOps / DEGREE;
		for (std::size_t call = 0; call < nrCalls; ++call) {
			Scalar p = seed[call & (NR_SEEDS - 1)];
			for (std::size_t i = DEGREE; i > 0; --i) {
				p = p * x + coefficients[i - 1];
			}
			hpbench::consume(p);
		}
	}

	template<typename Scalar>
	void measureType(hpbench::Suite& suite, const std::string& type) {
		suite.measure("dot N=16", type, DotWorkload<Scalar, 16>, 16);
		suite.measure("dot N=256", type, DotWorkload<Scalar, 256>, 256);
		suite.measure("dot N=4096", type, DotWorkload<Scalar, 4096>, 4096);
		suite.measure("axpy N=4096", type, AxpyWorkload<Scalar, 4096>, 4096);
		suite.measure("matmul 32x32", type, MatmulWorkload<Scalar, 32>, 32 * 32 * 32);
		suite.measure("horner deg 20", type, HornerWorkload<Scalar, 20>, 20);
	}

}  // anonymous namespace

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	hpbench::Suite suite("multi-component kernel performance (universal#1315)", hpbench::targetWindow(argc, argv));
	suite.reportEnvironment();
	std::cout << "  cost unit      : one elementary multiply-add (element op), so every kernel is on the same scale\n\n";

	measureType<double>(suite, "double");
	measureType<dd>(suite, "dd");
	measureType<dd_cascade>(suite, "dd_cascade");
	measureType<td_cascade>(suite, "td_cascade");
	measureType<qd>(suite, "qd");
	measureType<qd_cascade>(suite, "qd_cascade");

	suite.reportLatency();
	suite.reportThroughput();
	suite.reportRatios(hpbench::cascadeVsClassic());

	std::cout << "\ndone.\n";
	return EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
