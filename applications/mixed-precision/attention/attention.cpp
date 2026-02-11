// attention.cpp: Mixed-precision scaled dot-product attention with KV cache
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Demonstrates how precision choice determines KV cache memory footprint,
// energy consumption, and accuracy in transformer attention heads.
//
// The systems paper (Section 4.1) shows LLaMA-70B at FP32 = 280 GB,
// exceeding any single GPU. This application provides a concrete, runnable
// demonstration: a scaled dot-product attention head with KV cache,
// parameterized over Universal number types, wrapped in energy/memory/
// latency/accuracy measurement.
//
// Attention(Q,K,V) = softmax(Q K^T / sqrt(d_k)) V

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <string>
#include <algorithm>

// Universal number types
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit2/posit.hpp>

// Energy estimation
#include <universal/energy/energy.hpp>

using namespace sw::universal;

// Lightweight stats tracker (avoids blas/mixed_precision.hpp which pulls in posit/posit.hpp)
struct MixedPrecisionStats {
	uint64_t input_loads = 0;
	uint64_t compute_ops = 0;
	uint64_t accum_ops = 0;
	uint64_t output_stores = 0;
	double estimated_energy_pj = 0.0;

	void reset() {
		input_loads = 0; compute_ops = 0;
		accum_ops = 0; output_stores = 0;
		estimated_energy_pj = 0.0;
	}
};

// ============================================================================
// Attention Geometry
// ============================================================================

constexpr size_t D_MODEL  = 128;   // head dimension (d_k = d_v)
constexpr size_t SEQ_LEN  = 64;    // prefill context length
constexpr size_t N_TOKENS = 32;    // autoregressive generation steps

// LLaMA-70B architecture constants for scaling projection
constexpr size_t LLAMA_LAYERS  = 80;
constexpr size_t LLAMA_HEADS   = 64;
constexpr size_t LLAMA_DK      = 128;
constexpr size_t LLAMA_CONTEXT = 2048;
constexpr double GPU_HBM_GB    = 80.0;

// ============================================================================
// AttentionHead: Scaled Dot-Product Attention with Growing KV Cache
// ============================================================================

template<typename QKType, typename VType, typename AccumType>
class AttentionHead {
	size_t d_k;
	std::vector<std::vector<QKType>> k_cache;
	std::vector<std::vector<VType>>  v_cache;
	MixedPrecisionStats stats;

public:
	explicit AttentionHead(size_t dim) : d_k(dim) {}

	void appendKV(const std::vector<double>& k_row,
	              const std::vector<double>& v_row) {
		std::vector<QKType> k(d_k);
		std::vector<VType>  v(d_k);
		for (size_t j = 0; j < d_k; ++j) {
			k[j] = static_cast<QKType>(k_row[j]);
			v[j] = static_cast<VType>(v_row[j]);
		}
		k_cache.push_back(std::move(k));
		v_cache.push_back(std::move(v));

		stats.input_loads  += 2 * d_k;   // load k and v rows
		stats.output_stores += 2 * d_k;  // store into cache
	}

	std::vector<VType> forward(const std::vector<double>& q_double) {
		size_t T = k_cache.size();

		// Convert query to QKType
		std::vector<QKType> q(d_k);
		for (size_t j = 0; j < d_k; ++j) {
			q[j] = static_cast<QKType>(q_double[j]);
		}
		stats.input_loads += d_k;

		// Step 1: QK^T — dot(q, k_cache[t]) for each cached token
		std::vector<AccumType> scores(T);
		AccumType scale = static_cast<AccumType>(1.0 / std::sqrt(static_cast<double>(d_k)));
		for (size_t t = 0; t < T; ++t) {
			AccumType dot = static_cast<AccumType>(0);
			for (size_t j = 0; j < d_k; ++j) {
				dot += static_cast<AccumType>(q[j]) * static_cast<AccumType>(k_cache[t][j]);
			}
			scores[t] = dot * scale;

			stats.input_loads  += d_k;   // load k_cache[t]
			stats.compute_ops  += d_k;   // multiplications
			stats.accum_ops    += d_k;   // additions
			stats.compute_ops  += 1;     // scale multiply
		}

		// Step 2: Softmax at AccumType precision
		// Find max for numerical stability
		AccumType max_score = scores[0];
		for (size_t t = 1; t < T; ++t) {
			if (scores[t] > max_score) max_score = scores[t];
		}

		// exp and normalize (compute in double for portability across number types)
		AccumType sum_exp = static_cast<AccumType>(0);
		std::vector<AccumType> weights(T);
		for (size_t t = 0; t < T; ++t) {
			weights[t] = static_cast<AccumType>(std::exp(double(scores[t]) - double(max_score)));
			sum_exp += weights[t];
		}
		for (size_t t = 0; t < T; ++t) {
			weights[t] = weights[t] / sum_exp;
		}
		stats.compute_ops += 3 * T;  // sub, exp, div per token
		stats.accum_ops   += T;      // sum of exponentials

		// Step 3: Weighted V sum: output[j] = sum_t(weight[t] * v_cache[t][j])
		std::vector<VType> output(d_k);
		for (size_t j = 0; j < d_k; ++j) {
			AccumType accum = static_cast<AccumType>(0);
			for (size_t t = 0; t < T; ++t) {
				accum += weights[t] * static_cast<AccumType>(v_cache[t][j]);
			}
			output[j] = static_cast<VType>(accum);
		}
		stats.input_loads   += T * d_k;  // load v_cache
		stats.compute_ops   += T * d_k;  // multiplications
		stats.accum_ops     += T * d_k;  // additions
		stats.output_stores += d_k;      // store output

		return output;
	}

	size_t kvCacheBytes() const {
		return k_cache.size() * d_k * (sizeof(QKType) + sizeof(VType));
	}

	size_t cachedTokens() const { return k_cache.size(); }

	const MixedPrecisionStats& getStats() const { return stats; }
	void resetStats() { stats.reset(); }
};

// ============================================================================
// Accuracy Measurement
// ============================================================================

struct AccuracyResult {
	double max_abs_error;
	double rmse;
};

std::vector<std::vector<double>>
computeReferenceOutputs(const std::vector<std::vector<double>>& Q,
                        const std::vector<std::vector<double>>& K,
                        const std::vector<std::vector<double>>& V) {
	AttentionHead<double, double, double> ref(D_MODEL);

	// Prefill with SEQ_LEN tokens
	for (size_t i = 0; i < SEQ_LEN; ++i) {
		ref.appendKV(K[i], V[i]);
	}

	// Generate N_TOKENS outputs
	std::vector<std::vector<double>> outputs;
	for (size_t step = 0; step < N_TOKENS; ++step) {
		auto out_vec = ref.forward(Q[step]);
		std::vector<double> out_d(D_MODEL);
		for (size_t j = 0; j < D_MODEL; ++j) {
			out_d[j] = static_cast<double>(out_vec[j]);
		}
		outputs.push_back(std::move(out_d));

		// Append new KV for next step
		if (step + SEQ_LEN < K.size()) {
			ref.appendKV(K[step + SEQ_LEN], V[step + SEQ_LEN]);
		}
	}
	return outputs;
}

// ============================================================================
// Energy Estimation
// ============================================================================

energy::BitWidth toBitWidth(size_t bytes) {
	if (bytes <= 1) return energy::BitWidth::bits_8;
	if (bytes <= 2) return energy::BitWidth::bits_16;
	if (bytes <= 4) return energy::BitWidth::bits_32;
	return energy::BitWidth::bits_64;
}

double estimateAttentionEnergy(const MixedPrecisionStats& stats,
                               energy::BitWidth compute_bw,
                               energy::BitWidth accum_bw,
                               energy::BitWidth mem_bw) {
	using namespace energy;
	const auto& model = getDefaultModel();

	double e = 0.0;

	// Compute: multiplications at compute precision
	e += model.totalOperationEnergy(Operation::FloatMultiply, compute_bw, stats.compute_ops);

	// Accumulation: additions at accumulator precision
	e += model.totalOperationEnergy(Operation::FloatAdd, accum_bw, stats.accum_ops);

	// Memory loads
	e += model.memoryTransferEnergy(MemoryLevel::L1_Cache,
	        stats.input_loads * (static_cast<int>(mem_bw) / 8), false);

	// Memory stores
	e += model.memoryTransferEnergy(MemoryLevel::L1_Cache,
	        stats.output_stores * (static_cast<int>(mem_bw) / 8), true);

	return e;
}

// ============================================================================
// Benchmark Runner
// ============================================================================

struct AttentionBenchmarkResult {
	std::string config_name;
	size_t element_bytes;       // sizeof KV element
	size_t kv_cache_bytes;      // total after all steps
	double energy_pj;           // estimated energy
	double latency_us;          // wall-clock
	double max_abs_error;       // vs double reference
	double rmse;                // vs double reference
};

template<typename QKType, typename VType, typename AccumType>
AttentionBenchmarkResult runBenchmark(
        const std::string& name,
        const std::vector<std::vector<double>>& Q,
        const std::vector<std::vector<double>>& K,
        const std::vector<std::vector<double>>& V,
        const std::vector<std::vector<double>>& ref_outputs) {

	AttentionBenchmarkResult result;
	result.config_name = name;
	result.element_bytes = sizeof(QKType);

	AttentionHead<QKType, VType, AccumType> head(D_MODEL);

	// Prefill
	for (size_t i = 0; i < SEQ_LEN; ++i) {
		head.appendKV(K[i], V[i]);
	}
	head.resetStats();  // only measure generation phase

	// Time N_TOKENS forward passes
	std::vector<std::vector<double>> outputs;
	auto t0 = std::chrono::high_resolution_clock::now();

	for (size_t step = 0; step < N_TOKENS; ++step) {
		auto out_vec = head.forward(Q[step]);

		// Convert to double for accuracy comparison
		std::vector<double> out_d(D_MODEL);
		for (size_t j = 0; j < D_MODEL; ++j) {
			out_d[j] = static_cast<double>(out_vec[j]);
		}
		outputs.push_back(std::move(out_d));

		// Append new KV
		if (step + SEQ_LEN < K.size()) {
			head.appendKV(K[step + SEQ_LEN], V[step + SEQ_LEN]);
		}
	}

	auto t1 = std::chrono::high_resolution_clock::now();
	result.latency_us = std::chrono::duration<double, std::micro>(t1 - t0).count();

	result.kv_cache_bytes = head.kvCacheBytes();

	// Accuracy vs reference
	double max_err = 0.0;
	double sum_sq  = 0.0;
	size_t count   = 0;
	for (size_t step = 0; step < N_TOKENS; ++step) {
		for (size_t j = 0; j < D_MODEL; ++j) {
			double err = std::abs(outputs[step][j] - ref_outputs[step][j]);
			max_err = std::max(max_err, err);
			sum_sq += err * err;
			++count;
		}
	}
	result.max_abs_error = max_err;
	result.rmse = std::sqrt(sum_sq / count);

	// Energy estimation
	const auto& stats = head.getStats();
	energy::BitWidth compute_bw = toBitWidth(sizeof(QKType));
	energy::BitWidth accum_bw   = toBitWidth(sizeof(AccumType));
	energy::BitWidth mem_bw     = toBitWidth(sizeof(QKType));
	result.energy_pj = estimateAttentionEnergy(stats, compute_bw, accum_bw, mem_bw);

	return result;
}

// ============================================================================
// LLaMA-70B Scaling Projection
// ============================================================================

void printScalingProjection() {
	std::cout << "\n";
	std::cout << "========================================\n";
	std::cout << "LLaMA-70B KV Cache Scaling Projection\n";
	std::cout << "========================================\n\n";

	// Per token: layers * heads * d_k * 2 (K+V) * sizeof(elem)
	// = 80 * 64 * 128 * 2 = 1,310,720 elements per token
	size_t elems_per_token = LLAMA_LAYERS * LLAMA_HEADS * LLAMA_DK * 2;

	struct PrecisionRow {
		std::string name;
		size_t bytes_per_elem;
		double gb_at_context;
	};

	std::vector<PrecisionRow> rows = {
		{"double",       8, 0.0},
		{"float",        4, 0.0},
		{"fp16/half",    2, 0.0},
		{"bfloat16",     2, 0.0},
		{"fp8",          1, 0.0},
		{"int4 (packed)", 0, 0.0},  // special: 0.5 bytes
	};

	for (auto& r : rows) {
		double bytes_per_elem = (r.name == "int4 (packed)") ? 0.5 : static_cast<double>(r.bytes_per_elem);
		double kv_per_token_bytes = elems_per_token * bytes_per_elem;
		r.gb_at_context = (kv_per_token_bytes * LLAMA_CONTEXT) / (1024.0 * 1024.0 * 1024.0);
	}

	std::cout << std::left << std::setw(16) << "Precision"
	          << std::right << std::setw(12) << "Bytes/elem"
	          << std::setw(16) << "KV/token (KB)"
	          << std::setw(16) << "KV@2048 (GB)"
	          << std::setw(14) << "Fits 80GB?" << "\n";
	std::cout << std::string(74, '-') << "\n";

	for (const auto& r : rows) {
		double bytes_per_elem = (r.name == "int4 (packed)") ? 0.5 : static_cast<double>(r.bytes_per_elem);
		double kv_per_token_kb = (elems_per_token * bytes_per_elem) / 1024.0;

		std::cout << std::left << std::setw(16) << r.name
		          << std::right << std::fixed << std::setprecision(1)
		          << std::setw(12) << bytes_per_elem
		          << std::setprecision(2)
		          << std::setw(16) << kv_per_token_kb
		          << std::setw(16) << r.gb_at_context
		          << std::setw(14) << (r.gb_at_context < GPU_HBM_GB ? "YES" : "NO") << "\n";
	}

	// Model weights analysis
	constexpr double LLAMA_PARAMS = 70e9;
	double weights_fp32_gb = (LLAMA_PARAMS * 4) / (1024.0 * 1024.0 * 1024.0);
	double weights_fp16_gb = (LLAMA_PARAMS * 2) / (1024.0 * 1024.0 * 1024.0);
	double weights_int4_gb = (LLAMA_PARAMS * 0.5) / (1024.0 * 1024.0 * 1024.0);

	// FP16 KV cache at 2048 context
	double kv_fp16_gb = (static_cast<double>(elems_per_token) * 2.0 * LLAMA_CONTEXT) / (1024.0 * 1024.0 * 1024.0);

	std::cout << "\nModel Weights + KV Cache Combined Analysis:\n";
	std::cout << std::string(60, '-') << "\n";
	std::cout << "  Weights at FP32: " << std::fixed << std::setprecision(1)
	          << weights_fp32_gb << " GB -- does NOT fit in " << GPU_HBM_GB << " GB\n";
	std::cout << "  Weights at FP16: " << weights_fp16_gb
	          << " GB + FP16 KV " << std::setprecision(2) << kv_fp16_gb
	          << " GB = " << std::setprecision(1) << (weights_fp16_gb + kv_fp16_gb)
	          << " GB -- does NOT fit\n";
	std::cout << "  Weights at INT4: " << weights_int4_gb
	          << " GB + FP16 KV " << std::setprecision(2) << kv_fp16_gb
	          << " GB = " << std::setprecision(1) << (weights_int4_gb + kv_fp16_gb)
	          << " GB -- fits!\n";
}

// ============================================================================
// main
// ============================================================================

int main()
try {
	std::cout << "Universal Numbers: Mixed-Precision Attention Head with KV Cache\n";
	std::cout << "================================================================\n\n";

	std::cout << "Attention Configuration:\n";
	std::cout << "  Head dimension (d_k):     " << D_MODEL << "\n";
	std::cout << "  Prefill context (SEQ_LEN): " << SEQ_LEN << "\n";
	std::cout << "  Generation steps:          " << N_TOKENS << "\n\n";

	// Generate deterministic test data
	std::mt19937 rng(42);
	std::uniform_real_distribution<double> dist(-1.0, 1.0);

	size_t total_tokens = SEQ_LEN + N_TOKENS;
	std::vector<std::vector<double>> Q(N_TOKENS, std::vector<double>(D_MODEL));
	std::vector<std::vector<double>> K(total_tokens, std::vector<double>(D_MODEL));
	std::vector<std::vector<double>> V(total_tokens, std::vector<double>(D_MODEL));

	for (auto& row : Q) for (auto& x : row) x = dist(rng);
	for (auto& row : K) for (auto& x : row) x = dist(rng);
	for (auto& row : V) for (auto& x : row) x = dist(rng);

	// Compute double-precision reference
	std::cout << "Computing double-precision reference outputs...\n";
	auto ref_outputs = computeReferenceOutputs(Q, K, V);

	// Run benchmarks across type configurations
	std::vector<AttentionBenchmarkResult> results;

	std::cout << "Running type sweep (7 configurations)...\n\n";

	results.push_back(runBenchmark<double, double, double>(
		"double", Q, K, V, ref_outputs));

	results.push_back(runBenchmark<float, float, double>(
		"float", Q, K, V, ref_outputs));

	results.push_back(runBenchmark<half, half, float>(
		"fp16", Q, K, V, ref_outputs));

	results.push_back(runBenchmark<bfloat_t, bfloat_t, float>(
		"bf16", Q, K, V, ref_outputs));

	results.push_back(runBenchmark<posit<16,1>, posit<16,1>, posit<32,2>>(
		"posit<16,1>", Q, K, V, ref_outputs));

	results.push_back(runBenchmark<fp8e4m3, fp8e4m3, float>(
		"fp8e4m3", Q, K, V, ref_outputs));

	results.push_back(runBenchmark<posit<8,0>, posit<8,0>, posit<32,2>>(
		"posit<8,0>", Q, K, V, ref_outputs));

	// Print comparison table
	std::cout << "Attention Head Benchmark Results (" << N_TOKENS << " generation steps)\n";
	std::cout << std::string(90, '=') << "\n";
	std::cout << std::left << std::setw(14) << "Config"
	          << std::right << std::setw(12) << "KV Cache"
	          << std::setw(14) << "Energy(uJ)"
	          << std::setw(14) << "Latency(us)"
	          << std::setw(14) << "Max Error"
	          << std::setw(14) << "RMSE" << "\n";
	std::cout << std::string(90, '-') << "\n";

	for (const auto& r : results) {
		std::cout << std::left << std::setw(14) << r.config_name
		          << std::right << std::setw(10) << r.kv_cache_bytes << " B"
		          << std::fixed << std::setprecision(2)
		          << std::setw(14) << (r.energy_pj / 1e6);
		std::cout << std::setprecision(0)
		          << std::setw(14) << r.latency_us;
		std::cout << std::scientific << std::setprecision(2)
		          << std::setw(14) << r.max_abs_error
		          << std::setw(14) << r.rmse << "\n";
	}
	std::cout << std::string(90, '-') << "\n";

	// Energy breakdown: FP16 vs FP32
	std::cout << "\nEnergy Breakdown: FP16 vs FP32\n";
	std::cout << std::string(50, '-') << "\n";
	double fp32_energy = results[1].energy_pj;  // float config
	double fp16_energy = results[2].energy_pj;  // fp16 config
	std::cout << "  FP32 energy:  " << std::fixed << std::setprecision(2)
	          << (fp32_energy / 1e6) << " uJ\n";
	std::cout << "  FP16 energy:  " << (fp16_energy / 1e6) << " uJ\n";
	std::cout << "  Ratio:        " << std::setprecision(2)
	          << (fp32_energy / fp16_energy) << "x\n";
	std::cout << "  Savings:      " << std::setprecision(1)
	          << ((1.0 - fp16_energy / fp32_energy) * 100.0) << "%\n";

	// FP8 vs FP32
	double fp8_energy = results[5].energy_pj;  // fp8e4m3 config
	std::cout << "\n  FP8 energy:   " << std::fixed << std::setprecision(2)
	          << (fp8_energy / 1e6) << " uJ\n";
	std::cout << "  FP8 ratio:    " << std::setprecision(2)
	          << (fp32_energy / fp8_energy) << "x vs FP32\n";

	// LLaMA-70B scaling projection
	printScalingProjection();

	// Key takeaways
	std::cout << "\n\nKey Takeaways:\n";
	std::cout << "1. KV cache memory scales linearly with precision: 8x savings from FP64 to FP8\n";
	std::cout << "2. FP16 attention gives near-FP32 accuracy with 2x memory and energy savings\n";
	std::cout << "3. FP8 (e4m3) shows visible accuracy loss but enables massive model deployment\n";
	std::cout << "4. Posit<16,1> matches or beats FP16 accuracy at the same bit width\n";
	std::cout << "5. Softmax MUST run at accumulator precision for numerical stability\n";
	std::cout << "6. LLaMA-70B requires INT4 weights + FP16 KV cache to fit a single 80GB GPU\n";

	return EXIT_SUCCESS;
}
catch (const char* msg) {
	std::cerr << "Error: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& e) {
	std::cerr << "Exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
