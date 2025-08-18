#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <execution>
#include <type_traits>
#include <concepts>
#include <span>
#include <memory>
#include <chrono>

namespace sw::universal {

    // Concept to constrain template parameters to Universal number types
    //     // this pattern only matched native arithmetic types, not universal numbers
    //template<typename T>
    //concept UniversalNumberType = requires {
    //    typename T::value_type;
    //    std::is_arithmetic_v<typename T::value_type>;
    //} || std::is_arithmetic_v<T>;

    // Mixed-precision configuration for different stages of convolution
    //template<UniversalNumberType InputT, UniversalNumberType WeightT, UniversalNumberType AccumT, UniversalNumberType OutputT>
    //struct MixedPrecisionConfig {
    //    using input_type = InputT;
    //    using weight_type = WeightT;
    //    using accumulator_type = AccumT;
    //    using output_type = OutputT;
    //    
    //    static constexpr bool uses_mixed_precision = 
    //        !std::is_same_v<InputT, WeightT> || 
    //        !std::is_same_v<WeightT, AccumT> || 
    //        !std::is_same_v<AccumT, OutputT>;
    //};

    template<typename InputT, typename WeightT, typename AccumT, typename OutputT>
    struct MixedPrecisionConfig {
        using input_type = InputT;
        using weight_type = WeightT;
        using accumulator_type = AccumT;
        using output_type = OutputT;

        static constexpr bool uses_mixed_precision =
            !std::is_same_v<InputT, WeightT> ||
            !std::is_same_v<WeightT, AccumT> ||
            !std::is_same_v<AccumT, OutputT>;
    };

    // Tensor layout abstraction
    enum class TensorLayout {
        NCHW,  // Batch, Channel, Height, Width
        NHWC,  // Batch, Height, Width, Channel
        CHW,   // Channel, Height, Width (single batch)
        HWC    // Height, Width, Channel (single batch)
    };

    template<typename T>
    class Tensor4D {
    private:
        std::vector<T> data_;
        std::array<size_t, 4> shape_;  // N, C, H, W
        TensorLayout layout_;

    public:
        Tensor4D(size_t n, size_t c, size_t h, size_t w, TensorLayout layout = TensorLayout::NCHW)
            : data_(n* c* h* w), shape_{ n, c, h, w }, layout_(layout) {
        }

        T& operator()(size_t n, size_t c, size_t h, size_t w) {
            return data_[compute_index(n, c, h, w)];
        }

        const T& operator()(size_t n, size_t c, size_t h, size_t w) const {
            return data_[compute_index(n, c, h, w)];
        }

        std::span<T> data() { return std::span{ data_ }; }
        std::span<const T> data() const { return std::span{ data_ }; }

        const auto& shape() const { return shape_; }
        TensorLayout layout() const { return layout_; }
        size_t size() const { return data_.size(); }

    private:
        size_t compute_index(size_t n, size_t c, size_t h, size_t w) const {
            switch (layout_) {
            case TensorLayout::NCHW:
                return n * shape_[1] * shape_[2] * shape_[3] +
                    c * shape_[2] * shape_[3] +
                    h * shape_[3] + w;
            case TensorLayout::NHWC:
                return n * shape_[2] * shape_[3] * shape_[1] +
                    h * shape_[3] * shape_[1] +
                    w * shape_[1] + c;
            default:
                throw std::runtime_error("Unsupported tensor layout");
            }
        }
    };

    // Direct convolution implementation (no img2col)
    template<typename Config>
    class DirectConv2D {
    public:
        using input_type = typename Config::input_type;
        using weight_type = typename Config::weight_type;
        using accumulator_type = typename Config::accumulator_type;
        using output_type = typename Config::output_type;

        struct ConvParams {
            size_t stride_h = 1, stride_w = 1;
            size_t pad_h = 0, pad_w = 0;
            size_t dilation_h = 1, dilation_w = 1;
            bool use_parallel = true;
        };

        static void forward(
            const Tensor4D<input_type>& input,     // [N, C_in, H, W]
            const Tensor4D<weight_type>& weight,   // [C_out, C_in, K_h, K_w]
            Tensor4D<output_type>& output,         // [N, C_out, H_out, W_out]
            const ConvParams& params = {}) {

            const auto [N, C_in, H_in, W_in] = input.shape();
            const auto [C_out, C_in_w, K_h, K_w] = weight.shape();

            if (C_in != C_in_w) {
                throw std::invalid_argument("Input channels mismatch");
            }

            const size_t H_out = (H_in + 2 * params.pad_h - params.dilation_h * (K_h - 1) - 1) / params.stride_h + 1;
            const size_t W_out = (W_in + 2 * params.pad_w - params.dilation_w * (K_w - 1) - 1) / params.stride_w + 1;

            if (output.shape()[0] != N || output.shape()[1] != C_out ||
                output.shape()[2] != H_out || output.shape()[3] != W_out) {
                throw std::invalid_argument("Output tensor shape mismatch");
            }

            auto conv_fn = [&](size_t n_start, size_t n_end) {
                for (size_t n = n_start; n < n_end; ++n) {
                    for (size_t c_out = 0; c_out < C_out; ++c_out) {
                        for (size_t h_out = 0; h_out < H_out; ++h_out) {
                            for (size_t w_out = 0; w_out < W_out; ++w_out) {
                                accumulator_type acc{ 0 };

                                for (size_t c_in = 0; c_in < C_in; ++c_in) {
                                    for (size_t k_h = 0; k_h < K_h; ++k_h) {
                                        for (size_t k_w = 0; k_w < K_w; ++k_w) {
                                            const int64_t h_in = static_cast<int64_t>(h_out * params.stride_h)
                                                - static_cast<int64_t>(params.pad_h)
                                                + static_cast<int64_t>(k_h * params.dilation_h);
                                            const int64_t w_in = static_cast<int64_t>(w_out * params.stride_w)
                                                - static_cast<int64_t>(params.pad_w)
                                                + static_cast<int64_t>(k_w * params.dilation_w);

                                            if (h_in >= 0 && h_in < static_cast<int64_t>(H_in) &&
                                                w_in >= 0 && w_in < static_cast<int64_t>(W_in)) {

                                                // Mixed-precision multiply-accumulate
                                                const auto input_val = static_cast<accumulator_type>(
                                                    input(n, c_in, static_cast<size_t>(h_in), static_cast<size_t>(w_in)));
                                                const auto weight_val = static_cast<accumulator_type>(
                                                    weight(c_out, c_in, k_h, k_w));

                                                acc += input_val * weight_val;
                                            }
                                        }
                                    }
                                }

                                output(n, c_out, h_out, w_out) = static_cast<output_type>(acc);
                            }
                        }
                    }
                }
                };

#ifdef FUTURES_SUPPORTED
            if (params.use_parallel && N > 1) {
                // Parallel execution across batch dimension
                const size_t num_threads = std::thread::hardware_concurrency();
                const size_t batch_per_thread = (N + num_threads - 1) / num_threads;

                std::vector<std::future<void>> futures;
                for (size_t t = 0; t < num_threads; ++t) {
                    size_t n_start = t * batch_per_thread;
                    size_t n_end = std::min(n_start + batch_per_thread, N);
                    if (n_start < n_end) {
                        futures.emplace_back(std::async(std::launch::async, conv_fn, n_start, n_end));
                    }
                }

                for (auto& future : futures) {
                    future.wait();
                }
            }
            else {
                conv_fn(0, N);
            }
#else
            if (params.use_parallel && N > 1) {
                // Parallel execution across batch dimension using thread pool
                const size_t num_threads = std::thread::hardware_concurrency();
                const size_t batch_per_thread = (N + num_threads - 1) / num_threads;

                std::vector<std::thread> threads;
                threads.reserve(num_threads);

                for (size_t t = 0; t < num_threads; ++t) {
                    size_t n_start = t * batch_per_thread;
                    size_t n_end = std::min(n_start + batch_per_thread, N);
                    if (n_start < n_end) {
                        threads.emplace_back([conv_fn, n_start, n_end]() {
                            conv_fn(n_start, n_end);
                            });
                    }
                }

                // Wait for all threads to complete
                for (auto& thread : threads) {
                    if (thread.joinable()) {
                        thread.join();
                    }
                }
            }
            else {
                conv_fn(0, N);
            }
#endif

            // Alternative: Exception-safe RAII version
#if 0
            if (params.use_parallel && N > 1) {
                const size_t num_threads = std::thread::hardware_concurrency();
                const size_t batch_per_thread = (N + num_threads - 1) / num_threads;

                std::vector<std::unique_ptr<std::thread>> threads;
                threads.reserve(num_threads);

                try {
                    for (size_t t = 0; t < num_threads; ++t) {
                        size_t n_start = t * batch_per_thread;
                        size_t n_end = std::min(n_start + batch_per_thread, N);
                        if (n_start < n_end) {
                            threads.emplace_back(std::make_unique<std::thread>(
                                [conv_fn, n_start, n_end]() {
                                    conv_fn(n_start, n_end);
                                }
                            ));
                        }
                    }

                    // Wait for all threads to complete
                    for (auto& thread_ptr : threads) {
                        if (thread_ptr && thread_ptr->joinable()) {
                            thread_ptr->join();
                        }
                    }
                }
                catch (...) {
                    // Ensure threads are joined even if exception occurs
                    for (auto& thread_ptr : threads) {
                        if (thread_ptr && thread_ptr->joinable()) {
                            thread_ptr->join();
                        }
                    }
                    throw;
                }
            }
            else {
                conv_fn(0, N);
            }
#endif
        }
    };

    // Benchmarking utilities
    template<typename Config>
    class ConvolutionBenchmark {
    public:
        using Clock = std::chrono::high_resolution_clock;
        using Duration = std::chrono::nanoseconds;

        struct BenchmarkResult {
            Duration elapsed_time;
            double gflops;
            size_t memory_footprint_bytes;
            std::string precision_config;
        };

        static BenchmarkResult benchmark_direct_conv(
            size_t N, size_t C_in, size_t H_in, size_t W_in,
            size_t C_out, size_t K_h, size_t K_w,
            size_t iterations = 10) {

            using input_type = typename Config::input_type;
            using weight_type = typename Config::weight_type;
            using output_type = typename Config::output_type;

            // Create tensors
            Tensor4D<input_type> input(N, C_in, H_in, W_in);
            Tensor4D<weight_type> weight(C_out, C_in, K_h, K_w);
            Tensor4D<output_type> output(N, C_out, H_in - K_h + 1, W_in - K_w + 1);

            // Initialize with random-ish data (for energy measurement consistency)
            std::generate(input.data().begin(), input.data().end(),
                []() { return static_cast<input_type>(0.1); });
            std::generate(weight.data().begin(), weight.data().end(),
                []() { return static_cast<weight_type>(0.01); });

            // Warmup
            DirectConv2D<Config>::forward(input, weight, output);

            // Benchmark
            auto start = Clock::now();
            for (size_t i = 0; i < iterations; ++i) {
                DirectConv2D<Config>::forward(input, weight, output);
            }
            auto end = Clock::now();

            Duration elapsed = std::chrono::duration_cast<Duration>(end - start);

            // Calculate GFLOPS
            const size_t H_out = H_in - K_h + 1;
            const size_t W_out = W_in - K_w + 1;
            const size_t ops_per_iteration = 2 * N * C_out * H_out * W_out * C_in * K_h * K_w;
            const double total_ops = static_cast<double>(ops_per_iteration * iterations);
            const double gflops = total_ops / (elapsed.count() * 1e-9) / 1e9;

            // Memory footprint
            const size_t memory_footprint =
                input.size() * sizeof(input_type) +
                weight.size() * sizeof(weight_type) +
                output.size() * sizeof(output_type);

            return BenchmarkResult{
                elapsed,
                gflops,
                memory_footprint,
                get_precision_config_string<Config>()
            };
        }

    private:
        template<typename T>
        static std::string get_precision_config_string() {
            return "Input:" + get_type_name<typename T::input_type>() +
                " Weight:" + get_type_name<typename T::weight_type>() +
                " Accum:" + get_type_name<typename T::accumulator_type>() +
                " Output:" + get_type_name<typename T::output_type>();
        }

        //template<typename T>
        //static std::string get_type_name() {
        //    if constexpr (std::is_same_v<T, float>) return "fp32";
        //    else if constexpr (std::is_same_v<T, double>) return "fp64";
        //    else if constexpr (std::is_same_v<T, sw::universal::posit<16, 2> >) return "posit<16,2>";
        //    else if constexpr (std::is_same_v<T, sw::universal::posit<32, 2> >) return "posit<32,2>";
        //    else if constexpr (std::is_same_v<T, sw::universal::cfloat<16, 5> >) return "cfloat<16,5>";
        //    else return "unknown";
        //}
        template<typename T>
        static std::string get_type_name() {
            return type_tag(T());
        }
    };

}
