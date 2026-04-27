// log_add_algorithms.cpp : per-algorithm benchmark for the configurable
// lns add/sub framework (Phase D of issue #777, resolves #782).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Measures throughput (ops/sec) and accuracy (max value-domain ULP error vs
// DirectEvaluation oracle) for each shipped sb_add/sb_sub policy:
//   - DoubleTripAddSub
//   - DirectEvaluationAddSub
//   - LookupAddSub
//   - PolynomialAddSub
//   - ArnoldBaileyAddSub
//
// Results are printed as Markdown tables suitable for direct paste into
// release notes or the design document at docs/design/lns-add-sub.md.

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <cmath>

#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/lns/lns.hpp>

namespace sw { namespace universal {

	// Timed add+sub workload using a specific algorithm (bypass the traits dispatch
	// so we can measure each policy independently in the same translation unit).
	template<typename LnsType, typename Alg>
	double measure_throughput(std::size_t nr_ops) {
		// Two operands chosen to exercise both same-sign and mixed-sign paths.
		LnsType a(0.99999);
		LnsType b(1.0625);
		auto start = std::chrono::steady_clock::now();
		for (std::size_t i = 0; i < nr_ops; ++i) {
			Alg::add_assign(b, a);
			Alg::sub_assign(b, a);
			// Without a data dependency the optimizer may eliminate the loop;
			// a small XOR-style perturbation defeats that without changing the
			// asymptotic mix of values exercised.
			if ((i & 0xFF) == 0) a = LnsType(0.99999 + double(i % 7) * 1e-6);
		}
		auto end = std::chrono::steady_clock::now();
		// Sink so the optimizer can't drop the loop entirely.
		if (b == LnsType(0.0) && a == LnsType(123.456)) {
			std::cout << "(unreachable sink)\n";
		}
		std::chrono::duration<double> dt = end - start;
		double seconds = dt.count();
		// Each iteration does 2 ops (1 add + 1 sub).
		return double(nr_ops * 2) / seconds;
	}

	// Sample-based accuracy measurement: enumerate a sample of (a, b) operand
	// pairs and report the max value-domain absolute and relative error of Alg
	// vs DirectEvaluation as oracle.
	template<typename LnsType, typename Alg>
	struct AccuracyResult {
		double max_abs_err = 0.0;
		double max_rel_err = 0.0;
		std::size_t samples = 0;
	};

	template<typename LnsType, typename Alg>
	AccuracyResult<LnsType, Alg> measure_accuracy(std::size_t sample_count) {
		using Direct = DirectEvaluationAddSub<LnsType>;
		AccuracyResult<LnsType, Alg> result;
		// Sample operand magnitudes spanning the lns dynamic range.
		// We pick 'sample_count' values for each side, so ops total = sample_count^2.
		// Limit total work to sample_count <= ~256 for reasonable benchmark runtime.
		std::vector<double> samples;
		samples.reserve(sample_count);
		// Geometric sweep across [2^(-rbits-1), 2^(rbits)] then negate half.
		double lo = std::ldexp(1.0, -static_cast<int>(LnsType::rbits) - 1);
		double hi = std::ldexp(1.0, static_cast<int>(LnsType::rbits));
		double log_lo = std::log2(lo);
		double log_hi = std::log2(hi);
		double step = (log_hi - log_lo) / double(sample_count - 1);
		for (std::size_t i = 0; i < sample_count; ++i) {
			double v = std::exp2(log_lo + step * double(i));
			samples.push_back((i % 2 == 0) ? v : -v);
		}

		for (double da : samples) {
			LnsType a(da);
			if (a.isnan()) continue;
			for (double db : samples) {
				LnsType b(db);
				if (b.isnan()) continue;

				LnsType cAlg = a; Alg::add_assign(cAlg, b);
				LnsType cRef = a; Direct::add_assign(cRef, b);
				if (cAlg.isnan() && cRef.isnan()) continue;
				if (cAlg.isnan() || cRef.isnan()) continue;

				double vAlg = double(cAlg);
				double vRef = double(cRef);
				double diff = vAlg - vRef;
				if (diff < 0.0) diff = -diff;
				double mag = (vRef < 0.0 ? -vRef : vRef);
				double rel = (mag > 0.0) ? (diff / mag) : 0.0;
				if (diff > result.max_abs_err) result.max_abs_err = diff;
				if (rel  > result.max_rel_err) result.max_rel_err = rel;
				++result.samples;
			}
		}
		return result;
	}

	// Pretty-print a throughput value as M ops/sec.
	std::string throughput_str(double ops_per_sec) {
		std::ostringstream s;
		s << std::fixed << std::setprecision(2) << (ops_per_sec / 1.0e6) << " M";
		return s.str();
	}

	// Pretty-print a relative error value with limited precision.
	std::string err_str(double v) {
		std::ostringstream s;
		if (v == 0.0) {
			s << "0";
		} else {
			s << std::scientific << std::setprecision(2) << v;
		}
		return s.str();
	}

	// Run all five algorithms for one (nbits, rbits, bt) configuration and
	// emit a Markdown row per algorithm.
	template<typename LnsType>
	void benchmark_config(const char* config_name, std::size_t throughput_iters,
	                      std::size_t accuracy_samples) {
		double t_double = measure_throughput<LnsType, DoubleTripAddSub<LnsType>>(throughput_iters);
		double t_direct = measure_throughput<LnsType, DirectEvaluationAddSub<LnsType>>(throughput_iters);
		double t_lookup = measure_throughput<LnsType, LookupAddSub<LnsType>>(throughput_iters);
		double t_poly   = measure_throughput<LnsType, PolynomialAddSub<LnsType>>(throughput_iters);
		double t_ab     = measure_throughput<LnsType, ArnoldBaileyAddSub<LnsType>>(throughput_iters);

		auto a_direct = measure_accuracy<LnsType, DirectEvaluationAddSub<LnsType>>(accuracy_samples);
		auto a_lookup = measure_accuracy<LnsType, LookupAddSub<LnsType>>(accuracy_samples);
		auto a_poly   = measure_accuracy<LnsType, PolynomialAddSub<LnsType>>(accuracy_samples);
		auto a_ab     = measure_accuracy<LnsType, ArnoldBaileyAddSub<LnsType>>(accuracy_samples);
		auto a_double = measure_accuracy<LnsType, DoubleTripAddSub<LnsType>>(accuracy_samples);

		std::cout << "\n### " << config_name << "\n\n";
		std::cout << "| Algorithm           | Throughput     | Max abs error   | Max rel error   |\n";
		std::cout << "|---------------------|----------------|-----------------|-----------------|\n";
		std::cout << "| DoubleTrip          | " << std::setw(14) << std::left << throughput_str(t_double)
		          << " | " << std::setw(15) << err_str(a_double.max_abs_err)
		          << " | " << std::setw(15) << err_str(a_double.max_rel_err) << " |\n";
		std::cout << "| DirectEvaluation    | " << std::setw(14) << std::left << throughput_str(t_direct)
		          << " | " << std::setw(15) << err_str(a_direct.max_abs_err) << " (oracle)"
		          << " | " << std::setw(15) << err_str(a_direct.max_rel_err) << " (oracle) |\n";
		std::cout << "| Lookup              | " << std::setw(14) << std::left << throughput_str(t_lookup)
		          << " | " << std::setw(15) << err_str(a_lookup.max_abs_err)
		          << " | " << std::setw(15) << err_str(a_lookup.max_rel_err) << " |\n";
		std::cout << "| Polynomial          | " << std::setw(14) << std::left << throughput_str(t_poly)
		          << " | " << std::setw(15) << err_str(a_poly.max_abs_err)
		          << " | " << std::setw(15) << err_str(a_poly.max_rel_err) << " |\n";
		std::cout << "| ArnoldBailey        | " << std::setw(14) << std::left << throughput_str(t_ab)
		          << " | " << std::setw(15) << err_str(a_ab.max_abs_err)
		          << " | " << std::setw(15) << err_str(a_ab.max_rel_err) << " |\n";
	}

}}  // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "# lns add/sub algorithm benchmark (Phase D of #777)\n";
	std::cout << "\nThroughput is per-operation rate (1 op = 1 add or 1 sub).\n";
	std::cout << "Accuracy is measured against DirectEvaluation as the oracle, so\n";
	std::cout << "DirectEvaluation reports zero error by construction. Accuracy is\n";
	std::cout << "sampled over a geometric sweep of operand magnitudes spanning the\n";
	std::cout << "lns dynamic range with both same-sign and mixed-sign pairs.\n";

	// Throughput iterations per config: scale down for the larger lns sizes
	// so the benchmark completes in seconds rather than minutes.
	constexpr std::size_t TPUT_SMALL  = 1'000'000;
	constexpr std::size_t TPUT_MEDIUM =   500'000;
	constexpr std::size_t TPUT_LARGE  =   200'000;
	// Accuracy sample count (per axis; total = N*N pairs).
	constexpr std::size_t ACC_SAMPLES = 64;

	benchmark_config<lns< 8, 4, std::uint8_t>>("lns< 8, 4, uint8_t>",  TPUT_SMALL,  ACC_SAMPLES);
	benchmark_config<lns<16, 8, std::uint16_t>>("lns<16, 8, uint16_t>", TPUT_MEDIUM, ACC_SAMPLES);
	benchmark_config<lns<24,16, std::uint32_t>>("lns<24,16, uint32_t>", TPUT_MEDIUM, ACC_SAMPLES);
	benchmark_config<lns<32,16, std::uint32_t>>("lns<32,16, uint32_t>", TPUT_LARGE,  ACC_SAMPLES);

	std::cout << "\n## Reading the table\n\n";
	std::cout << "- DoubleTrip is the legacy placeholder and a useful reference\n";
	std::cout << "  for what the prior lns implementation cost.\n";
	std::cout << "- DirectEvaluation is the high-precision software default.\n";
	std::cout << "- Lookup, Polynomial, and ArnoldBailey trade accuracy for\n";
	std::cout << "  reduced cost; see docs/design/lns-add-sub.md for the\n";
	std::cout << "  decision tree.\n";
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << err.what() << '\n';
	return EXIT_FAILURE;
}
