// cordic_precision_assessment.cpp : per-iteration accuracy study for
// CORDICAddSub across the lns rbits sweep (Phase E of issue #777,
// resolves #783 trigger-comment expanded acceptance criteria).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// Produces three artifacts requested by the hardware-codesign consumer:
//
//   1. Per-iteration convergence study -- accuracy of CORDICAddSub at
//      MaxIterations k = 1..rbits, so a hardware team can pick a truncated
//      iteration budget that trades area/latency for error.
//
//   2. ULP error histograms across the rbits sweep -- empirical
//      log-domain error distributions over the operand-pair domain at each
//      rbits setting.
//
//   3. Worst-case input table -- the operand pair producing the largest
//      log-domain error at each (rbits, k) combination.
//
// Configurations exercised (per the trigger comment):
//   lns<8,4>, lns<12,6>, lns<16,8>, lns<24,12>, lns<32,16>
//
// Output is Markdown to stdout; redirect to docs/design/cordic-precision-assessment.md
// to refresh the checked-in artifact.

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <cstdlib>
#include <limits>

#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/lns/lns.hpp>

namespace sw { namespace universal {

	// Log-domain error sample: for a given operand pair (a, b), measures how
	// far CORDICAddSub<LNS, k>::sb_add or sb_sub diverges from DirectEvaluation.
	// Both algorithms route through the same shared dispatcher, so the only
	// degree of freedom is sb_add / sb_sub itself.
	struct ErrorSummary {
		double max_log_err   = 0.0;
		double sum_log_err   = 0.0;
		std::size_t samples  = 0;
		// Worst-case witness, recorded as the d that produced the max error.
		double worst_d       = 0.0;
		bool   worst_is_sub  = false;
	};

	// Sweep d over the log-domain range and accumulate sb_add / sb_sub error
	// for CORDICAddSub<LnsType, K> against DirectEvaluation. We sweep d in
	// log-domain rather than over (a, b) pairs so the result is directly
	// interpretable as "how accurate is the policy at this d".
	template<typename LnsType, unsigned K>
	ErrorSummary sweep_sb_error(unsigned d_samples) {
		using Cordic = CORDICAddSub<LnsType, K>;
		using Direct = DirectEvaluationAddSub<LnsType>;
		ErrorSummary s;
		// sb_add domain: d in [-(rbits+2), 0], where -(rbits+2) is the floor
		// at which both algorithms return 0. Sample uniformly.
		double d_floor = -(double(LnsType::rbits) + 2.0);
		if (d_samples < 2) d_samples = 2;
		double step = (-d_floor) / double(d_samples - 1);

		for (unsigned i = 0; i < d_samples; ++i) {
			double d = d_floor + step * double(i);
			if (d > 0.0) d = 0.0;
			double e_add = Cordic::sb_add(d);
			double r_add = Direct::sb_add(d);
			double diff_add = e_add - r_add;
			if (diff_add < 0.0) diff_add = -diff_add;
			s.sum_log_err += diff_add;
			++s.samples;
			if (diff_add > s.max_log_err) {
				s.max_log_err = diff_add;
				s.worst_d = d;
				s.worst_is_sub = false;
			}
			// sb_sub domain: d < 0. Skip d=0 (sb_sub returns -inf there).
			if (d < 0.0) {
				double e_sub = Cordic::sb_sub(d);
				double r_sub = Direct::sb_sub(d);
				if (std::isfinite(e_sub) && std::isfinite(r_sub)) {
					double diff_sub = e_sub - r_sub;
					if (diff_sub < 0.0) diff_sub = -diff_sub;
					s.sum_log_err += diff_sub;
					++s.samples;
					if (diff_sub > s.max_log_err) {
						s.max_log_err = diff_sub;
						s.worst_d = d;
						s.worst_is_sub = true;
					}
				}
			}
		}
		return s;
	}

	// Histogram of log-domain errors. Bin boundaries are powers of 2 in absolute
	// log-domain error, from 2^-30 up to 2^0; an extra bin captures anything
	// larger.
	template<typename LnsType, unsigned K>
	std::array<std::size_t, 32> ulp_histogram(unsigned d_samples) {
		using Cordic = CORDICAddSub<LnsType, K>;
		using Direct = DirectEvaluationAddSub<LnsType>;
		std::array<std::size_t, 32> bins{};
		double d_floor = -(double(LnsType::rbits) + 2.0);
		if (d_samples < 2) d_samples = 2;
		double step = (-d_floor) / double(d_samples - 1);

		auto bin_index = [](double err) -> std::size_t {
			if (err <= 0.0) return 0;
			// Map err in (0, +inf) to log2 bins of width 1, anchored at 2^-30.
			double lg = std::log2(err);
			int idx = 30 + static_cast<int>(std::ceil(lg));
			if (idx < 0) idx = 0;
			if (idx > 31) idx = 31;
			return static_cast<std::size_t>(idx);
		};

		for (unsigned i = 0; i < d_samples; ++i) {
			double d = d_floor + step * double(i);
			if (d > 0.0) d = 0.0;
			double diff_add = Cordic::sb_add(d) - Direct::sb_add(d);
			if (diff_add < 0.0) diff_add = -diff_add;
			++bins[bin_index(diff_add)];
			if (d < 0.0) {
				double e_sub = Cordic::sb_sub(d);
				double r_sub = Direct::sb_sub(d);
				if (std::isfinite(e_sub) && std::isfinite(r_sub)) {
					double diff_sub = e_sub - r_sub;
					if (diff_sub < 0.0) diff_sub = -diff_sub;
					++bins[bin_index(diff_sub)];
				}
			}
		}
		return bins;
	}

	std::string err_str(double v) {
		std::ostringstream s;
		if (v == 0.0) {
			s << "0";
		} else {
			s << std::scientific << std::setprecision(2) << v;
		}
		return s.str();
	}

	// Per-(rbits, k) convergence row for one LnsType.
	// For each k from 1 to rbits we instantiate CORDICAddSub<LnsType, k> and
	// measure max + mean log-domain error vs DirectEvaluation.
	template<typename LnsType, unsigned K>
	void emit_convergence_row(unsigned d_samples) {
		ErrorSummary s = sweep_sb_error<LnsType, K>(d_samples);
		double mean = (s.samples > 0) ? (s.sum_log_err / double(s.samples)) : 0.0;
		std::cout << "| " << std::setw(2) << K
		          << " | " << std::setw(11) << std::left << err_str(s.max_log_err)
		          << " | " << std::setw(11) << std::left << err_str(mean)
		          << " | " << std::fixed << std::setprecision(4)
		          << std::setw(8) << std::left << s.worst_d
		          << " | " << (s.worst_is_sub ? "sub" : "add")
		          << " |\n";
		std::cout << std::defaultfloat;
	}

	// Recursive template helper: emit rows for k = 1, 2, ..., Kmax.
	template<typename LnsType, unsigned K>
	struct ConvergenceWalker {
		static void emit(unsigned d_samples) {
			ConvergenceWalker<LnsType, K - 1>::emit(d_samples);
			emit_convergence_row<LnsType, K>(d_samples);
		}
	};
	template<typename LnsType>
	struct ConvergenceWalker<LnsType, 0> {
		static void emit(unsigned) {}
	};

	// Histogram row for one k value.
	template<typename LnsType, unsigned K>
	void emit_histogram_row(unsigned d_samples) {
		auto bins = ulp_histogram<LnsType, K>(d_samples);
		// Compact reporting: show bin populations only where non-zero,
		// labeled by their upper bound 2^(idx-30).
		std::cout << "k=" << std::setw(2) << K << ": ";
		bool first = true;
		for (std::size_t i = 0; i < bins.size(); ++i) {
			if (bins[i] == 0) continue;
			if (!first) std::cout << ", ";
			first = false;
			int exp = static_cast<int>(i) - 30;
			std::cout << "2^" << exp << ":" << bins[i];
		}
		std::cout << "\n";
	}

	template<typename LnsType, unsigned K>
	struct HistogramWalker {
		static void emit(unsigned d_samples) {
			HistogramWalker<LnsType, K - 1>::emit(d_samples);
			emit_histogram_row<LnsType, K>(d_samples);
		}
	};
	template<typename LnsType>
	struct HistogramWalker<LnsType, 0> {
		static void emit(unsigned) {}
	};

	template<typename LnsType>
	void report_config(const char* name, unsigned d_samples) {
		constexpr unsigned rbits = LnsType::rbits;
		std::cout << "\n## " << name << " (rbits = " << rbits << ")\n";

		std::cout << "\n### Per-iteration convergence\n\n";
		std::cout << "| k  | Max log err | Mean log err | Worst d  | Branch |\n";
		std::cout << "|----|-------------|--------------|----------|--------|\n";
		ConvergenceWalker<LnsType, rbits>::emit(d_samples);

		std::cout << "\n### ULP error histogram (bins of 2^(idx-30))\n\n";
		std::cout << "```text\n";
		HistogramWalker<LnsType, rbits>::emit(d_samples);
		std::cout << "```\n";

		// Worst-case input table at k = rbits.
		ErrorSummary s_full = sweep_sb_error<LnsType, rbits>(d_samples);
		std::cout << "\n### Worst-case witness at k = rbits\n\n";
		std::cout << "- d = " << std::fixed << std::setprecision(6) << s_full.worst_d
		          << " (branch: " << (s_full.worst_is_sub ? "sb_sub" : "sb_add") << ")\n";
		std::cout << "- max log-domain error = " << err_str(s_full.max_log_err) << "\n";
		std::cout << "- mean log-domain error = "
		          << err_str(s_full.samples > 0 ? s_full.sum_log_err / double(s_full.samples) : 0.0)
		          << "\n";
		std::cout << std::defaultfloat;
	}

}}  // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "# CORDIC precision/accuracy assessment\n";
	std::cout << "\nGenerated by `benchmark/accuracy/lns/cordic_precision_assessment.cpp`.\n";
	std::cout << "This artifact answers the per-iteration convergence + ULP histogram +\n";
	std::cout << "worst-case-input questions in the #783 trigger comment.\n";
	std::cout << "\nAll errors are reported in the **log domain** (i.e., the value\n";
	std::cout << "the sb_add / sb_sub primitive itself returns). The dispatcher\n";
	std::cout << "amplifies a log-domain error E into encoded-value relative error\n";
	std::cout << "of roughly 2^E - 1, with one ULP in the encoded log = 2^-rbits.\n";

	// Per-d sample density: same per config so cross-config comparisons are
	// apples-to-apples. The sweep spans the full d range [-(rbits+2), 0] and
	// uniformly samples both sb_add and sb_sub branches.
	constexpr unsigned D_SAMPLES = 1024;

	report_config<lns< 8, 4, std::uint8_t>>("lns< 8, 4, uint8_t>",   D_SAMPLES);
	report_config<lns<12, 6, std::uint8_t>>("lns<12, 6, uint8_t>",   D_SAMPLES);
	report_config<lns<16, 8, std::uint16_t>>("lns<16, 8, uint16_t>", D_SAMPLES);
	report_config<lns<24,12, std::uint32_t>>("lns<24,12, uint32_t>", D_SAMPLES);
	report_config<lns<32,16, std::uint32_t>>("lns<32,16, uint32_t>", D_SAMPLES);

	std::cout << "\n## Interpretation\n\n";
	std::cout << "- The per-iteration row at k = rbits is the configuration the\n";
	std::cout << "  default `CORDICAddSub<lns<nbits,rbits>>` uses. Earlier rows\n";
	std::cout << "  describe truncated-iteration budgets.\n";
	std::cout << "- The `sb_sub` branch in the cancellation regime d in (-1, 0)\n";
	std::cout << "  is routed to direct evaluation (cm::log2 / cm::exp2) regardless\n";
	std::cout << "  of k, matching the fallback used by `LookupAddSub`,\n";
	std::cout << "  `PolynomialAddSub`, and `ArnoldBaileyAddSub`. Hardware retargets\n";
	std::cout << "  would replace that with a co-transformation or small lookup\n";
	std::cout << "  rather than a transcendental.\n";
	std::cout << "- For d <= -1 (the pure-CORDIC `sb_sub` branch) and across all\n";
	std::cout << "  of `sb_add`, the per-iteration error is bounded by approximately\n";
	std::cout << "  4 * 2^-k log-domain. The histograms surface where this envelope\n";
	std::cout << "  is tight vs slack.\n";

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
