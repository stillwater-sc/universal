#pragma once
 // conv2d-1.cpp: mixed-precision convolution benchmark
 //
 // Copyright (C) 2017 Stillwater Supercomputing, Inc.
 // SPDX-License-Identifier: MIT
 //
 // This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <random>
#include <execution>
#include <cstring>

namespace sw::universal {

struct ConvolutionParameters2D {
    size_t batch_size;
    size_t in_channels;
    size_t out_channels;
    size_t in_height;
    size_t in_width;
    size_t kernel_height;
    size_t kernel_width;
    size_t stride_h;
    size_t stride_w;
    size_t pad_h;
    size_t pad_w;
    size_t dilation_h;
    size_t dilation_w;

    // helper functions to initialize input/output data structures
    size_t input_size() {
        return batch_size * in_channels * in_height * in_width;
    }
    size_t weight_size() {
        return out_channels * in_channels * kernel_height * kernel_width;
    }
};

/**
 * Generic 2D Convolution Kernel with Mixed Precision Support
 * 
 * @tparam InputType - Number type for input tensor (e.g., posit<16,1>)
 * @tparam WeightType - Number type for weights (e.g., posit<8,0>)
 * @tparam AccumType - Number type for accumulation (e.g., posit<32,2>)
 * @tparam OutputType - Number type for output (e.g., posit<16,1>)
 */
template<typename InputType, typename WeightType, typename AccumType, typename OutputType>
class Conv2D {
public:
    //using input_type = InputType;
    //using weight_type = WeightType;
    //using accumulator_type = AccumType;
    //using output_type = OutputType;
	typedef InputType input_type;
	typedef WeightType weight_type;
	typedef AccumType accumulator_type;
	typedef OutputType output_type;

    Conv2D(const ConvolutionParameters2D& p) : params(p) {
        // Calculate output dimensions
        out_height = (params.in_height + 2 * params.pad_h - 
                     params.dilation_h * (params.kernel_height - 1) - 1) / 
                     params.stride_h + 1;
        out_width = (params.in_width + 2 * params.pad_w - 
                    params.dilation_w * (params.kernel_width - 1) - 1) / 
                    params.stride_w + 1;
        
        // Allocate weights [out_channels, in_channels, kernel_h, kernel_w]
        size_t weight_size = params.out_channels * params.in_channels * 
                           params.kernel_height * params.kernel_width;
        weights.resize(weight_size);
        bias.resize(params.out_channels);
    }
    
    /**
     * Direct convolution - Energy efficient, no data duplication
     * Processes one spatial position at a time to minimize memory traffic
     */
    void forward_direct(
        const std::vector<InputType>& input,
        std::vector<OutputType>& output,
        bool use_bias = true
    ) {
        const size_t batch_stride = params.in_channels * params.in_height * params.in_width;
        const size_t out_batch_stride = params.out_channels * out_height * out_width;
        
        output.resize(params.batch_size * out_batch_stride);
        
        // Process each batch
        //#pragma omp parallel for collapse(2)
        for (size_t b = 0; b < params.batch_size; ++b) {
            for (size_t oc = 0; oc < params.out_channels; ++oc) {
                
                // Process each output spatial position
                for (size_t oh = 0; oh < out_height; ++oh) {
                    for (size_t ow = 0; ow < out_width; ++ow) {
                        AccumType sum = use_bias ? bias[oc] : AccumType(0);
                        
                        // Convolution for this output position
                        for (size_t ic = 0; ic < params.in_channels; ++ic) {
                            for (size_t kh = 0; kh < params.kernel_height; ++kh) {
                                for (size_t kw = 0; kw < params.kernel_width; ++kw) {
                                    // Calculate input position
                                    int ih = oh * params.stride_h - params.pad_h + 
                                            kh * params.dilation_h;
                                    int iw = ow * params.stride_w - params.pad_w + 
                                            kw * params.dilation_w;
                                    
                                    // Check bounds
                                    if (ih >= 0 && ih < static_cast<int>(params.in_height) && 
                                        iw >= 0 && iw < static_cast<int>(params.in_width)) {
                                        
                                        size_t input_idx = b * batch_stride +
                                                         ic * params.in_height * params.in_width +
                                                         ih * params.in_width + iw;
                                        
                                        size_t weight_idx = oc * params.in_channels * 
                                                          params.kernel_height * params.kernel_width +
                                                          ic * params.kernel_height * params.kernel_width +
                                                          kh * params.kernel_width + kw;
                                        
                                        // Mixed precision: multiply in lower precision, accumulate in higher
                                        AccumType input_val = static_cast<AccumType>(input[input_idx]);
                                        AccumType weight_val = static_cast<AccumType>(weights[weight_idx]);
                                        sum += input_val * weight_val;
                                    }
                                }
                            }
                        }
                        
                        // Store output (with precision conversion)
                        size_t out_idx = b * out_batch_stride +
                                       oc * out_height * out_width +
                                       oh * out_width + ow;
                        output[out_idx] = static_cast<OutputType>(sum);
                    }
                }
            }
        }
    }
    
    /**
     * Tiled convolution - Better cache locality for energy efficiency
     * Processes tiles to maximize data reuse in cache
     */
    void forward_tiled(
        const std::vector<InputType>& input,
        std::vector<OutputType>& output,
        size_t tile_size = 8,
        bool use_bias = true
    ) {
        const size_t batch_stride = params.in_channels * params.in_height * params.in_width;
        const size_t out_batch_stride = params.out_channels * out_height * out_width;
        
        output.resize(params.batch_size * out_batch_stride);
        std::fill(output.begin(), output.end(), OutputType(0));
        
        // Initialize output with bias if needed
        if (use_bias) {
            //#pragma omp parallel for
            for (size_t b = 0; b < params.batch_size; ++b) {
                for (size_t oc = 0; oc < params.out_channels; ++oc) {
                    OutputType bias_val = static_cast<OutputType>(bias[oc]);
                    for (size_t oh = 0; oh < out_height; ++oh) {
                        for (size_t ow = 0; ow < out_width; ++ow) {
                            size_t out_idx = b * out_batch_stride +
                                           oc * out_height * out_width +
                                           oh * out_width + ow;
                            output[out_idx] = bias_val;
                        }
                    }
                }
            }
        }
        
        // Process in tiles for better cache locality
        //#pragma omp parallel for collapse(3)
        for (size_t b = 0; b < params.batch_size; ++b) {
            for (size_t oc_tile = 0; oc_tile < params.out_channels; oc_tile += tile_size) {
                for (size_t ic_tile = 0; ic_tile < params.in_channels; ic_tile += tile_size) {
                    
                    size_t oc_end = std::min(oc_tile + tile_size, params.out_channels);
                    size_t ic_end = std::min(ic_tile + tile_size, params.in_channels);
                    
                    // Process tile
                    for (size_t oc = oc_tile; oc < oc_end; ++oc) {
                        for (size_t ic = ic_tile; ic < ic_end; ++ic) {
                            
                            // Convolution for this channel pair
                            for (size_t oh = 0; oh < out_height; ++oh) {
                                for (size_t ow = 0; ow < out_width; ++ow) {
                                    
                                    AccumType partial_sum(0);
                                    
                                    for (size_t kh = 0; kh < params.kernel_height; ++kh) {
                                        for (size_t kw = 0; kw < params.kernel_width; ++kw) {
                                            int ih = oh * params.stride_h - params.pad_h + 
                                                    kh * params.dilation_h;
                                            int iw = ow * params.stride_w - params.pad_w + 
                                                    kw * params.dilation_w;
                                            
                                            if (ih >= 0 && ih < static_cast<int>(params.in_height) && 
                                                iw >= 0 && iw < static_cast<int>(params.in_width)) {
                                                
                                                size_t input_idx = b * batch_stride +
                                                                 ic * params.in_height * params.in_width +
                                                                 ih * params.in_width + iw;
                                                
                                                size_t weight_idx = oc * params.in_channels * 
                                                                  params.kernel_height * params.kernel_width +
                                                                  ic * params.kernel_height * params.kernel_width +
                                                                  kh * params.kernel_width + kw;
                                                
                                                AccumType input_val = static_cast<AccumType>(input[input_idx]);
                                                AccumType weight_val = static_cast<AccumType>(weights[weight_idx]);
                                                partial_sum += input_val * weight_val;
                                            }
                                        }
                                    }
                                    
                                    // Accumulate to output
                                    size_t out_idx = b * out_batch_stride +
                                                   oc * out_height * out_width +
                                                   oh * out_width + ow;
                                    
                                    //#pragma omp atomic
                                    output[out_idx] += static_cast<OutputType>(partial_sum);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    /**
     * Depthwise separable convolution - Most energy efficient for mobile
     * Splits convolution into depthwise and pointwise stages
     */
    void forward_depthwise_separable(
        const std::vector<InputType>& input,
        std::vector<OutputType>& output,
        const std::vector<WeightType>& pointwise_weights,
        bool use_bias = true
    ) {
        // This would implement depthwise separable convolution
        // First: depthwise convolution (each input channel with its own kernel)
        // Second: 1x1 pointwise convolution to mix channels
        // Most energy efficient for many mobile scenarios
        
        // Implementation left as exercise - follows similar pattern
        // but processes channels independently first
    }
    
    // Utility functions
    void set_weights(const std::vector<WeightType>& w) {
        weights = w;
    }
    
    void set_bias(const std::vector<AccumType>& b) {
        bias = b;
    }
    
    size_t get_output_height() const { return out_height; }
    size_t get_output_width() const { return out_width; }
    
    // Energy metrics estimation
    struct EnergyMetrics {
        size_t memory_reads;
        size_t memory_writes;
        size_t multiply_ops;
        size_t accumulate_ops;
    };
    
    EnergyMetrics estimate_energy_direct() const {
        EnergyMetrics metrics;
        
        // Per output element
        size_t ops_per_output = params.in_channels * params.kernel_height * params.kernel_width;
        size_t total_outputs = params.batch_size * params.out_channels * out_height * out_width;
        
        metrics.multiply_ops = total_outputs * ops_per_output;
        metrics.accumulate_ops = metrics.multiply_ops;
        
        // Memory access pattern (assuming no caching)
        metrics.memory_reads = 
            params.batch_size * params.in_channels * params.in_height * params.in_width + // Input
            params.out_channels * params.in_channels * params.kernel_height * params.kernel_width; // Weights
        
        metrics.memory_writes = total_outputs;
        
        return metrics;
    }

private:
    ConvolutionParameters2D params;
    std::vector<WeightType> weights;
    std::vector<AccumType> bias;

    // Precomputed values for efficiency
    size_t out_height;
    size_t out_width;
};

class BenchmarkRunner {
private:
    std::mt19937 gen{ 42 }; // Fixed seed for reproducibility

    template<typename T>
    void fill_random(std::vector<T>& vec, T min_val = T(-1), T max_val = T(1)) {
        std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
        for (auto& val : vec) {
            val = T(dis(gen));
        }
    }

    template<typename ConvType>
    void benchmark_conv_method(
        const std::string& method_name,
        const std::string& precision_name,
        ConvType& conv,
        const auto& input,
        auto& output,
        int num_iterations = 10
    ) {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Benchmarking: " << precision_name << " - " << method_name << "\n";
        std::cout << std::string(60, '=') << "\n";

        // Warmup
        if (method_name == "Direct") {
            conv.forward_direct(input, output);
        }
        else if (method_name == "Tiled") {
            conv.forward_tiled(input, output, 8);
        }

        // Benchmark timing
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_iterations; ++i) {
            if (method_name == "Direct") {
                conv.forward_direct(input, output);
            }
            else if (method_name == "Tiled") {
                conv.forward_tiled(input, output, 8);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        double avg_time_ms = duration.count() / (1000.0 * num_iterations);

        // Get energy metrics
        auto metrics = conv.estimate_energy_direct();

        // Calculate throughput metrics
        size_t total_ops = metrics.multiply_ops + metrics.accumulate_ops;
        double gops = (total_ops / 1e9) / (avg_time_ms / 1000.0);

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Average time per iteration: " << avg_time_ms << " ms\n";
        std::cout << "Total operations: " << total_ops << "\n";
        std::cout << "GOPS (Giga Operations/sec): " << gops << "\n";
        std::cout << "Memory reads: " << metrics.memory_reads << "\n";
        std::cout << "Memory writes: " << metrics.memory_writes << "\n";
        std::cout << "Multiply operations: " << metrics.multiply_ops << "\n";
        std::cout << "Accumulate operations: " << metrics.accumulate_ops << "\n";

        // Energy efficiency metric (operations per memory access)
        double ops_per_memory_access = static_cast<double>(total_ops) /
            (metrics.memory_reads + metrics.memory_writes);
        std::cout << "Energy efficiency (ops/memory_access): " << ops_per_memory_access << "\n";

        // Verify output is reasonable (check for NaN/Inf)
        bool output_valid = true;
        size_t checked_samples = std::min(output.size(), size_t(100));
        for (size_t i = 0; i < checked_samples; ++i) {
            if (!std::isfinite(static_cast<float>(output[i]))) {
                output_valid = false;
                break;
            }
        }
        std::cout << "Output validation: " << (output_valid ? "PASS" : "FAIL") << "\n";
    }

    // Recursive helper for MSVC compatibility
    template<typename FirstConv, typename... RestConvs>
    void benchmark_types_helper(const ConvolutionParameters2D& params) {
        conv2D<FirstConv>(params);

        if constexpr (sizeof...(RestConvs) > 0) {
            benchmark_types_helper<RestConvs...>(params);
        }
    }

public:
    template<typename ConvType>
    void conv2D(const ConvolutionParameters2D& params) {
        using InputType = typename ConvType::input_type;
        using WeightType = typename ConvType::weight_type;
        using AccumType = typename ConvType::accumulator_type;
        using OutputType = typename ConvType::output_type;

        // Generate type description using Universal's type_tag
        std::string type_desc =
            "\nInput:" + type_tag(InputType()) +
            "\nWeight:" + type_tag(WeightType()) +
            "\nAccum:" + type_tag(AccumType()) +
            "\nOutput:" + type_tag(OutputType());

        ConvType conv(params);

        // Calculate tensor sizes
        size_t input_size = params.batch_size * params.in_channels *
            params.in_height * params.in_width;
        size_t weight_size = params.out_channels * params.in_channels *
            params.kernel_height * params.kernel_width;

        // Allocate and initialize tensors
        std::vector<InputType> input(input_size);
        std::vector<WeightType> weights(weight_size);
        std::vector<AccumType> bias(params.out_channels);
        std::vector<OutputType> output;

        fill_random(input);
        fill_random(weights);
        fill_random(bias);

        conv.set_weights(weights);
        conv.set_bias(bias);

        // Benchmark both algorithms
        benchmark_conv_method("Direct", type_desc, conv, input, output);
        benchmark_conv_method("Tiled", type_desc, conv, input, output);
    }

    template<typename... ConvTypes>
    void run_comprehensive_benchmark() {
        static_assert(sizeof...(ConvTypes) > 0, "At least one Conv2D type must be provided");

        std::cout << "Conv2D Energy Efficiency Benchmark\n";
        std::cout << "===================================\n\n";

        //using FirstConvType = std::tuple_element_t<0, std::tuple<ConvTypes...>>;

        // Define convolution parameters for a realistic mobile CNN layer
        ConvolutionParameters2D params{
            .batch_size = 1,
            .in_channels = 32,
            .out_channels = 64,
            .in_height = 56,
            .in_width = 56,
            .kernel_height = 3,
            .kernel_width = 3,
            .stride_h = 1,
            .stride_w = 1,
            .pad_h = 1,
            .pad_w = 1,
            .dilation_h = 1,
            .dilation_w = 1
        };

        std::cout << "Convolution Parameters:\n";
        std::cout << "Input: " << params.batch_size << "x" << params.in_channels
            << "x" << params.in_height << "x" << params.in_width << "\n";
        std::cout << "Kernel: " << params.kernel_height << "x" << params.kernel_width << "\n";
        std::cout << "Output channels: " << params.out_channels << "\n";
        std::cout << "Stride: " << params.stride_h << "x" << params.stride_w << "\n\n";

        // Benchmark each convolution type - use recursive helper for MSVC compatibility
        benchmark_types_helper<ConvTypes...>(params);

        std::cout << "\n" << std::string(80, '=') << "\n";
        std::cout << "BENCHMARK SUMMARY\n";
        std::cout << std::string(80, '=') << "\n";
        std::cout << "Algorithm Comparison:\n";
        std::cout << "- Direct: Lower memory overhead, simpler cache pattern\n";
        std::cout << "- Tiled: Better cache locality for larger inputs\n\n";
        std::cout << "For mobile/edge deployment, consider:\n";
        std::cout << "- Lower precision arithmetic for energy efficiency\n";
        std::cout << "- Tiled approach for larger feature maps\n";
        std::cout << "- Direct approach for smaller, latency-critical layers\n";
    }

    void run_tile_size_analysis() {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Tile Size Analysis for Cache Optimization\n";
        std::cout << std::string(60, '=') << "\n";

        ConvolutionParameters2D params{
            .batch_size = 1, .in_channels = 64, .out_channels = 128,
            .in_height = 28, .in_width = 28,
            .kernel_height = 3, .kernel_width = 3,
            .stride_h = 1, .stride_w = 1, .pad_h = 1, .pad_w = 1,
            .dilation_h = 1, .dilation_w = 1
        };

        Conv2D<float, float, float, float> conv_float(params);

        size_t input_size = params.batch_size * params.in_channels *
            params.in_height * params.in_width;
        size_t weight_size = params.out_channels * params.in_channels *
            params.kernel_height * params.kernel_width;

        std::vector<float> input_float(input_size);
        std::vector<float> weights_float(weight_size);
        std::vector<float> bias_float(params.out_channels);
        std::vector<float> output_float;

        fill_random(input_float);
        fill_random(weights_float);
        fill_random(bias_float);

        conv_float.set_weights(weights_float);
        conv_float.set_bias(bias_float);

        std::vector<size_t> tile_sizes = { 2, 4, 8, 16, 32, 64 };

        for (size_t tile_size : tile_sizes) {
            auto start = std::chrono::high_resolution_clock::now();
            conv_float.forward_tiled(input_float, output_float, tile_size);
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            double time_ms = duration.count() / 1000.0;

            std::cout << "Tile size " << std::setw(2) << tile_size
                << ": " << std::fixed << std::setprecision(2)
                << time_ms << " ms\n";
        }
    }
};


} // namespace sw::universal

