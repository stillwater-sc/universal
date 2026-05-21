// performance.cpp : performance benchmarking for elreal (lazy exact real arithmetic)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase I baseline for epic #873.
//
// Goals
// =====
// 1. Throughput-per-operation for elreal at three refinement budgets:
//      - depth-0  : operator only, result consumed via at(0)
//      - depth-1  : operator + at(1) to force the generator (default elreal
//                   refinement depth as shipped through Phase G)
//      - refine_to(B) : sweep B over the budget range to show that beyond
//                   depth 1 the refinement cost is currently O(1) per call
//                   (most operators clamp at depth 1).
//
// 2. Matched-precision comparison with ereal<N>:
//      - ereal<2>  ~ 106 bits (the dd-equivalent baseline)
//      - ereal<4>  ~ 212 bits (the qd-equivalent baseline)
//      - ereal<8>  ~ 424 bits (the default ereal max)
//    elreal's default budget (depth 1, ~ 53 bits per component, ~ 106 bits
//    cumulative for typical operators) is the natural comparison point
//    against ereal<2>.
//
// Workload design note
// --------------------
// Each iteration constructs *fresh* operands from double literals rather
// than accumulating a result back into one of the operands. This is the
// right choice for measuring per-operator throughput: elreal's lazy
// generators capture their inputs by value, so a feedback loop
// (`a = a + b`) grows the captured-ancestor chain linearly with iteration
// count, and the at(1) walk through that chain dominates the timing
// at large NR_OPS -- which would be measuring chain-walk cost, not
// per-operation cost. The chain-walk scaling is documented separately
// in the baseline writeup.

#include <universal/utility/directives.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/benchmark/performance_runner.hpp>

namespace {

	// elreal workloads ----------------------------------------------------

	void ElrealAdditionWorkload_Depth0(std::size_t NR_OPS) {
		using sw::universal::elreal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(1.0625 + double(i & 0xFF) * 1e-12);
			elreal b(0.99999);
			elreal c = a + b;
			sink += c.at(0);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealAdditionWorkload_Depth1(std::size_t NR_OPS) {
		using sw::universal::elreal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(1.0625 + double(i & 0xFF) * 1e-12);
			elreal b(0.99999);
			elreal c = a + b;
			sink += c.at(0) + c.at(1);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealSubtractionWorkload_Depth1(std::size_t NR_OPS) {
		using sw::universal::elreal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(2.0 + double(i & 0xFF) * 1e-12);
			elreal b(0.0625);
			elreal c = a - b;
			sink += c.at(0) + c.at(1);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealMultiplicationWorkload_Depth1(std::size_t NR_OPS) {
		using sw::universal::elreal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(1.0001 + double(i & 0xFF) * 1e-12);
			elreal b(0.9999);
			elreal c = a * b;
			sink += c.at(0) + c.at(1);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealDivisionWorkload_Depth0(std::size_t NR_OPS) {
		using sw::universal::elreal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(2.0 + double(i & 0xFF) * 1e-12);
			elreal b(1.0625);
			elreal c = a / b;
			sink += c.at(0);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealSqrtWorkload_Depth1(std::size_t NR_OPS) {
		using sw::universal::elreal;
		using sw::universal::sqrt;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(2.0 + double(i & 0xFF) * 1e-12);
			elreal c = sqrt(a);
			sink += c.at(0) + c.at(1);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealExpWorkload_Depth1(std::size_t NR_OPS) {
		using sw::universal::elreal;
		using sw::universal::exp;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(0.5 + double(i & 0xFF) * 1e-12);
			elreal c = exp(a);
			sink += c.at(0) + c.at(1);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealLogWorkload_Depth1(std::size_t NR_OPS) {
		using sw::universal::elreal;
		using sw::universal::log;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(2.5 + double(i & 0xFF) * 1e-12);
			elreal c = log(a);
			sink += c.at(0) + c.at(1);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealRefineToWorkload_106(std::size_t NR_OPS) {
		using sw::universal::elreal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(1.0625 + double(i & 0xFF) * 1e-12);
			elreal b(0.99999);
			elreal c = a + b;
			c.refine_to(106);
			sink += c.at(0) + c.at(1);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	void ElrealRefineToWorkload_212(std::size_t NR_OPS) {
		using sw::universal::elreal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			elreal a(1.0625 + double(i & 0xFF) * 1e-12);
			elreal b(0.99999);
			elreal c = a + b;
			c.refine_to(212);
			sink += c.at(0) + c.at(1);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	// ereal<N> workloads --------------------------------------------------

	// Note: ereal<N> uses std::vector<double> for component storage, so
	// declaring `a, b` *inside* the loop allocates and frees on every
	// iteration (matching the per-iteration allocation pattern used in
	// the elreal workloads above). This keeps both sides on an equal
	// footing for the per-operation-cost comparison.
	template<unsigned N>
	void ErealAdditionWorkload(std::size_t NR_OPS) {
		using sw::universal::ereal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			ereal<N> a; a = 1.0625 + double(i & 0xFF) * 1e-12;
			ereal<N> b; b = 0.99999;
			ereal<N> c = a + b;
			sink += double(c);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	template<unsigned N>
	void ErealSubtractionWorkload(std::size_t NR_OPS) {
		using sw::universal::ereal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			ereal<N> a; a = 2.0 + double(i & 0xFF) * 1e-12;
			ereal<N> b; b = 0.0625;
			ereal<N> c = a - b;
			sink += double(c);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	template<unsigned N>
	void ErealMultiplicationWorkload(std::size_t NR_OPS) {
		using sw::universal::ereal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			ereal<N> a; a = 1.0001 + double(i & 0xFF) * 1e-12;
			ereal<N> b; b = 0.9999;
			ereal<N> c = a * b;
			sink += double(c);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

	template<unsigned N>
	void ErealDivisionWorkload(std::size_t NR_OPS) {
		using sw::universal::ereal;
		double sink = 0.0;
		for (std::size_t i = 0; i < NR_OPS; ++i) {
			ereal<N> a; a = 2.0 + double(i & 0xFF) * 1e-12;
			ereal<N> b; b = 1.0625;
			ereal<N> c = a / b;
			sink += double(c);
		}
		if (sink == 0.0) std::cout << "dummy case to fool the optimizer\n";
	}

}  // namespace

int main() {
	using namespace sw::universal;
	std::cout << "elreal performance baseline (Phase I, epic #873)\n";
	std::cout << "===============================================\n\n";

	// Each elreal operator allocates a std::vector (component storage) +
	// a std::function (generator capture) on the heap, plus copies of
	// both input elreals into the generator capture. Under -O3 this
	// lands the per-op cost in the tens to hundreds of nanoseconds range
	// for arithmetic and math functions (corresponding to the 5-35 Mops/s
	// throughput reported in the baseline). 50k iterations is the sweet
	// spot for stable timing without long runs.
	constexpr std::size_t ELREAL_OPS_ARITH = 50000;
	constexpr std::size_t ELREAL_OPS_MATH  = 50000;
	constexpr std::size_t EREAL_OPS        = 100000;

	std::cout << "[elreal] arithmetic at depth 1 (default refinement)\n";
	PerformanceRunner("elreal +          (depth 0)  ", ElrealAdditionWorkload_Depth0,        ELREAL_OPS_ARITH);
	PerformanceRunner("elreal +          (depth 1)  ", ElrealAdditionWorkload_Depth1,        ELREAL_OPS_ARITH);
	PerformanceRunner("elreal -          (depth 1)  ", ElrealSubtractionWorkload_Depth1,     ELREAL_OPS_ARITH);
	PerformanceRunner("elreal *          (depth 1)  ", ElrealMultiplicationWorkload_Depth1,  ELREAL_OPS_ARITH);
	PerformanceRunner("elreal /          (depth 0)  ", ElrealDivisionWorkload_Depth0,        ELREAL_OPS_ARITH);
	std::cout << '\n';

	std::cout << "[elreal] math at depth 1 (derivative correction)\n";
	PerformanceRunner("elreal sqrt       (depth 1)  ", ElrealSqrtWorkload_Depth1, ELREAL_OPS_MATH);
	PerformanceRunner("elreal exp        (depth 1)  ", ElrealExpWorkload_Depth1,  ELREAL_OPS_MATH);
	PerformanceRunner("elreal log        (depth 1)  ", ElrealLogWorkload_Depth1,  ELREAL_OPS_MATH);
	std::cout << '\n';

	std::cout << "[elreal] refine_to budget sweep on +\n";
	std::cout << "  depth-1 generators clamp at one residual, so depth >= 2 is O(1) padding today\n";
	PerformanceRunner("elreal +  refine_to(106)     ", ElrealRefineToWorkload_106, ELREAL_OPS_ARITH);
	PerformanceRunner("elreal +  refine_to(212)     ", ElrealRefineToWorkload_212, ELREAL_OPS_ARITH);
	std::cout << '\n';

	std::cout << "[ereal<N>] matched-precision comparison\n";
	std::cout << "  ereal<2>  ~ 106 bits  (dd-equivalent)\n";
	std::cout << "  ereal<4>  ~ 212 bits  (qd-equivalent)\n";
	std::cout << "  ereal<8>  ~ 424 bits  (default ereal max)\n";
	PerformanceRunner("ereal<2> +                   ", ErealAdditionWorkload<2>,       EREAL_OPS);
	PerformanceRunner("ereal<4> +                   ", ErealAdditionWorkload<4>,       EREAL_OPS);
	PerformanceRunner("ereal<8> +                   ", ErealAdditionWorkload<8>,       EREAL_OPS);
	PerformanceRunner("ereal<2> -                   ", ErealSubtractionWorkload<2>,    EREAL_OPS);
	PerformanceRunner("ereal<4> -                   ", ErealSubtractionWorkload<4>,    EREAL_OPS);
	PerformanceRunner("ereal<8> -                   ", ErealSubtractionWorkload<8>,    EREAL_OPS);
	PerformanceRunner("ereal<2> *                   ", ErealMultiplicationWorkload<2>, EREAL_OPS);
	PerformanceRunner("ereal<4> *                   ", ErealMultiplicationWorkload<4>, EREAL_OPS);
	PerformanceRunner("ereal<8> *                   ", ErealMultiplicationWorkload<8>, EREAL_OPS);
	PerformanceRunner("ereal<2> /                   ", ErealDivisionWorkload<2>,       EREAL_OPS);
	PerformanceRunner("ereal<4> /                   ", ErealDivisionWorkload<4>,       EREAL_OPS);
	PerformanceRunner("ereal<8> /                   ", ErealDivisionWorkload<8>,       EREAL_OPS);

	return EXIT_SUCCESS;
}
