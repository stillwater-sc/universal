// log_add_algorithms.cpp : per-algorithm benchmark for the configurable
// lns add/sub framework (Phase D of issue #777, resolves #782).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Measures throughput (ops/sec) and accuracy (max value-domain absolute and
// relative error vs DirectEvaluation oracle) for each shipped sb_add/sb_sub
// policy:
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
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <limits>

#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/lns/lns.hpp>

namespace sw { namespace universal {

	// Timed add+sub workload using a specific algorithm (bypass the traits dispatch
	// so we can measure each policy independently in the same translation unit).
	//
	// We rotate through a small pre-encoded operand pool so:
	// (1) approximate policies don't accumulate rounding drift over nr_ops
	//     iterations (which would conflate per-op cost with cumulative drift);
	// (2) the optimizer can't constant-fold the loop body away;
	// (3) operand construction stays out of the hot loop -- only the
	//     pool-indexed copy and the algorithm calls are timed.
	template<typename LnsType, typename Alg>
	double measure_throughput(std::size_t nr_ops) {
		constexpr std::size_t POOL_SIZE = 8;  // power of 2 -> cheap masking
		std::array<LnsType, POOL_SIZE> ops_a;
		std::array<LnsType, POOL_SIZE> ops_b;
		for (std::size_t i = 0; i < POOL_SIZE; ++i) {
			ops_a[i] = LnsType(0.99999 + double(i) * 1.0e-6);
			ops_b[i] = LnsType(1.0625  + double(i) * 1.0e-6);
		}
		LnsType acc;
		auto start = std::chrono::steady_clock::now();
		for (std::size_t i = 0; i < nr_ops; ++i) {
			std::size_t idx = i & (POOL_SIZE - 1);
			acc = ops_b[idx];
			Alg::add_assign(acc, ops_a[idx]);
			Alg::sub_assign(acc, ops_a[idx]);
		}
		auto end = std::chrono::steady_clock::now();
		// Sink so the optimizer can't drop the loop entirely.
		if (acc == LnsType(0.0) && ops_a[0] == LnsType(123.456)) {
			std::cout << "(unreachable sink)\n";
		}
		std::chrono::duration<double> dt = end - start;
		double seconds = dt.count();
		// Floor at 1 ns so the division stays finite *and* sane if the loop
		// happens to be unmeasurably fast (clock granularity, optimizer
		// elimination). numeric_limits<double>::min() is too small -- with
		// nr_ops on the order of 1e6 it would still overflow to inf.
		if (seconds < 1e-9) seconds = 1e-9;
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
		// Flagged divergences -- counted, not masked.
		//   nan_mismatches:    pairs where exactly one side produced NaN
		//   zero_ref_mismatches: pairs where oracle was 0 but algorithm was non-zero
		//   inf_both:          pairs where BOTH algorithms decoded to +/-inf
		//                      (extreme-magnitude decode overflow, expected when
		//                      the lns dynamic range exceeds double's; not an
		//                      algorithm bug)
		//   inf_one_sided:     pairs where EXACTLY ONE algorithm decoded to
		//                      +/-inf -- a real catastrophic divergence; we also
		//                      force max_abs_err to infinity so the regression
		//                      surfaces in the abs-error column
		std::size_t nan_mismatches = 0;
		std::size_t zero_ref_mismatches = 0;
		std::size_t inf_both = 0;
		std::size_t inf_one_sided = 0;
	};

	template<typename LnsType, typename Alg>
	AccuracyResult<LnsType, Alg> measure_accuracy(std::size_t sample_count) {
		using Direct = DirectEvaluationAddSub<LnsType>;
		AccuracyResult<LnsType, Alg> result;
		if (sample_count < 2) sample_count = 2;  // step calc needs >= 2 samples
		// Sample operand magnitudes spanning the LnsType's actual dynamic
		// range, then negate half so the cross product covers same-sign and
		// mixed-sign pairs. Distinct (nbits, rbits) pairs with the same rbits
		// have very different ranges (lns<24,16> covers 2^[-64, 63] while
		// lns<32,16> covers 2^[-16384, 16383]); using rbits alone would miss
		// that. We also clamp to double's representable range so 2^large
		// doesn't overflow during sample generation.
		std::vector<double> samples;
		samples.reserve(sample_count);
		constexpr int dbl_min_exp = std::numeric_limits<double>::min_exponent;  // -1021
		constexpr int dbl_max_exp = std::numeric_limits<double>::max_exponent;  // 1024
		int lns_min_exp = static_cast<int>(LnsType::min_exponent);
		int lns_max_exp = static_cast<int>(LnsType::max_exponent);
		int min_exp = (lns_min_exp > dbl_min_exp + 1) ? lns_min_exp : dbl_min_exp + 1;
		int max_exp = (lns_max_exp < dbl_max_exp - 1) ? lns_max_exp : dbl_max_exp - 1;
		double lo = std::ldexp(1.0, min_exp);
		double hi = std::ldexp(1.0, max_exp);
		double log_lo = std::log2(lo);
		double log_hi = std::log2(hi);
		double step = (log_hi - log_lo) / double(sample_count - 1);
		for (std::size_t i = 0; i < sample_count; ++i) {
			double v = std::exp2(log_lo + step * double(i));
			samples.push_back((i % 2 == 0) ? v : -v);
		}

		auto record = [&](const LnsType& cAlg, const LnsType& cRef) {
			bool alg_nan = cAlg.isnan();
			bool ref_nan = cRef.isnan();
			if (alg_nan && ref_nan) return;
			if (alg_nan != ref_nan) {
				// One-sided NaN: a real divergence between the algorithm and
				// the oracle. Count it as a sample (so 'samples' is total
				// evaluated comparisons) and increment the dedicated counter.
				++result.nan_mismatches;
				++result.samples;
				return;
			}
			double vAlg = double(cAlg);
			double vRef = double(cRef);
			// At extreme magnitudes (e.g., lns<32, 16> covers 2^[-16384, 16383]
			// while double caps at 2^1024), decode-back-to-double can produce
			// +/-inf. Distinguish:
			//   both inf: extreme-magnitude overflow on both sides, expected
			//             behavior, not an algorithm bug
			//   one-sided inf: catastrophic algorithm divergence, must surface
			constexpr double dinf = std::numeric_limits<double>::infinity();
			bool alg_inf = (vAlg == dinf || vAlg == -dinf);
			bool ref_inf = (vRef == dinf || vRef == -dinf);
			if (alg_inf && ref_inf) {
				++result.inf_both;
				++result.samples;
				return;
			}
			if (alg_inf != ref_inf) {
				++result.inf_one_sided;
				++result.samples;
				// Force the absolute-error column to surface this as a
				// regression rather than reporting a finite max abs from
				// other samples.
				result.max_abs_err = dinf;
				return;
			}
			double diff = vAlg - vRef;
			if (diff < 0.0) diff = -diff;
			double mag = (vRef < 0.0 ? -vRef : vRef);
			double rel;
			if (mag > 0.0) {
				rel = diff / mag;
			} else if (diff > 0.0) {
				// Oracle is 0 but algorithm produced non-zero: an unbounded
				// relative error. Count separately so the rel-error column
				// isn't poisoned by INFINITY, and surface it in the report.
				++result.zero_ref_mismatches;
				rel = 0.0;
			} else {
				rel = 0.0;
			}
			if (diff > result.max_abs_err) result.max_abs_err = diff;
			if (rel  > result.max_rel_err) result.max_rel_err = rel;
			++result.samples;
		};

		for (double da : samples) {
			LnsType a(da);
			if (a.isnan()) continue;
			for (double db : samples) {
				LnsType b(db);
				if (b.isnan()) continue;

				// add_assign
				LnsType cAlgAdd = a; Alg::add_assign(cAlgAdd, b);
				LnsType cRefAdd = a; Direct::add_assign(cRefAdd, b);
				record(cAlgAdd, cRefAdd);

				// sub_assign (uses sb_sub via the dispatcher's mixed-sign path)
				LnsType cAlgSub = a; Alg::sub_assign(cAlgSub, b);
				LnsType cRefSub = a; Direct::sub_assign(cRefSub, b);
				record(cAlgSub, cRefSub);
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

		auto fmt_row = [](const char* name, double tput, const auto& acc) {
			std::cout << "| " << std::setw(19) << std::left << name
			          << " | " << std::setw(14) << std::left << throughput_str(tput)
			          << " | " << std::setw(15) << err_str(acc.max_abs_err)
			          << " | " << std::setw(15) << err_str(acc.max_rel_err)
			          << " | " << std::setw(11) << acc.nan_mismatches
			          << " | " << std::setw(11) << acc.zero_ref_mismatches
			          << " | " << std::setw(11) << acc.inf_both
			          << " | " << std::setw(11) << acc.inf_one_sided
			          << " |\n";
		};

		std::cout << "\n### " << config_name << "\n\n";
		std::cout << "| Algorithm           | Throughput     | Max abs error   | Max rel error   | NaN mismatch | Zero-ref div | Inf both    | Inf 1-sided |\n";
		std::cout << "|---------------------|----------------|-----------------|-----------------|--------------|--------------|-------------|-------------|\n";
		fmt_row("DoubleTrip",       t_double, a_double);
		fmt_row("DirectEvaluation", t_direct, a_direct);
		fmt_row("Lookup",           t_lookup, a_lookup);
		fmt_row("Polynomial",       t_poly,   a_poly);
		fmt_row("ArnoldBailey",     t_ab,     a_ab);
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
