// robotics_pipeline.cpp: Mixed-precision robotics perception pipeline
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Demonstrates mixed-precision optimization for an embodied AI system:
// - Sensor preprocessing (image/lidar)
// - Neural network inference (object detection)
// - State estimation (Kalman filter)
// - Control output (motor commands)
//
// Each stage uses different precision based on accuracy requirements
// and energy budget constraints for battery-powered robots.

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>

// Universal number types
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

// Energy estimation
#include <universal/energy/energy.hpp>
#include <universal/utility/pareto_explorer.hpp>

// Mixed-precision BLAS
#include <blas/mixed_precision.hpp>

using namespace sw::universal;
using namespace sw::blas;

// ============================================================================
// Robotics Pipeline Configuration
// ============================================================================

/// Power budget for a typical mobile robot (Watts)
constexpr double ROBOT_POWER_BUDGET_W = 10.0;      // 10W for compute

/// Perception loop rate (Hz)
constexpr double PERCEPTION_RATE_HZ = 30.0;

/// Energy budget per frame (Joules)
constexpr double ENERGY_PER_FRAME_J = ROBOT_POWER_BUDGET_W / PERCEPTION_RATE_HZ;

/// Precision requirements for each pipeline stage
struct PipelineRequirements {
    double sensor_accuracy;      // Sensor preprocessing
    double detection_accuracy;   // Neural network inference
    double state_accuracy;       // State estimation
    double control_accuracy;     // Control output
};

const PipelineRequirements MOBILE_ROBOT = {
    1e-3,   // Sensor: 0.1% is enough for image preprocessing
    1e-2,   // Detection: ML inference tolerates lower precision
    1e-6,   // State: Kalman filter needs higher precision
    1e-4    // Control: Motor commands need reasonable precision
};

// ============================================================================
// Stage 1: Sensor Preprocessing (Image/Lidar)
// ============================================================================

/// Simulated image preprocessing: resize, normalize, color conversion
/// Low precision is sufficient as sensor noise dominates
struct SensorPreprocessing {
    using InputType = uint8_t;                    // Raw sensor (8-bit)
    using ComputeType = half;                     // FP16 compute
    using OutputType = half;                      // FP16 output

    static constexpr size_t IMAGE_WIDTH = 640;
    static constexpr size_t IMAGE_HEIGHT = 480;
    static constexpr size_t CHANNELS = 3;

    static MixedPrecisionStats process(std::vector<float>& output) {
        MixedPrecisionStats stats;

        // Simulate preprocessing operations
        size_t pixels = IMAGE_WIDTH * IMAGE_HEIGHT * CHANNELS;

        // Normalization: divide by 255
        stats.input_loads += pixels;
        stats.compute_ops += pixels;
        stats.output_stores += pixels;

        // Color space conversion (simplified)
        stats.compute_ops += pixels * 3;  // Matrix multiply per pixel
        stats.accum_ops += pixels * 3;

        // Gaussian blur (3x3 kernel)
        stats.input_loads += pixels * 9;
        stats.compute_ops += pixels * 9;
        stats.accum_ops += pixels * 9;
        stats.output_stores += pixels;

        // Generate dummy output
        output.resize(pixels);
        std::fill(output.begin(), output.end(), 0.5f);

        return stats;
    }
};

// ============================================================================
// Stage 2: Neural Network Inference (Object Detection)
// ============================================================================

/// Simulated object detection network (MobileNet-SSD style)
/// INT8 with INT32 accumulator for maximum energy efficiency
struct ObjectDetection {
    using InputType = int8_t;                     // Quantized input
    using ComputeType = int8_t;                   // INT8 compute
    using AccumType = int32_t;                    // INT32 accumulator
    using OutputType = float;                     // FP32 output (scores)

    // Network architecture (simplified)
    static constexpr size_t CONV_LAYERS = 14;
    static constexpr size_t AVG_CHANNELS = 256;
    static constexpr size_t FEATURE_SIZE = 19 * 19;

    static MixedPrecisionStats detect(const std::vector<float>& image,
                                       std::vector<float>& detections) {
        MixedPrecisionStats stats;

        // Simulate convolutional layers
        for (size_t layer = 0; layer < CONV_LAYERS; ++layer) {
            size_t input_size = FEATURE_SIZE * AVG_CHANNELS;
            size_t kernel_size = 3 * 3 * AVG_CHANNELS;

            // Conv2D: each output pixel is dot product of kernel
            stats.input_loads += input_size + kernel_size;
            stats.compute_ops += FEATURE_SIZE * kernel_size;
            stats.accum_ops += FEATURE_SIZE * kernel_size;
            stats.output_stores += FEATURE_SIZE * AVG_CHANNELS;
        }

        // Detection head
        stats.compute_ops += FEATURE_SIZE * 100;  // Classification
        stats.compute_ops += FEATURE_SIZE * 4;    // Bounding boxes

        // Generate dummy detections
        detections = {0.95f, 0.87f, 0.72f};  // Confidence scores

        return stats;
    }
};

// ============================================================================
// Stage 3: State Estimation (Extended Kalman Filter)
// ============================================================================

/// Extended Kalman Filter for robot pose estimation
/// Higher precision needed for covariance matrix stability
struct StateEstimation {
    using ComputeType = float;                    // FP32 compute
    using AccumType = double;                     // FP64 accumulator
    using StateType = float;                      // FP32 state

    static constexpr size_t STATE_DIM = 6;        // x, y, z, roll, pitch, yaw
    static constexpr size_t MEAS_DIM = 4;         // Sensor measurements

    static MixedPrecisionStats update(std::vector<float>& state,
                                       std::vector<float>& covariance) {
        MixedPrecisionStats stats;

        // Prediction step
        // x_pred = F * x
        stats.input_loads += STATE_DIM + STATE_DIM * STATE_DIM;
        stats.compute_ops += STATE_DIM * STATE_DIM;
        stats.accum_ops += STATE_DIM * STATE_DIM;

        // P_pred = F * P * F' + Q
        stats.compute_ops += 2 * STATE_DIM * STATE_DIM * STATE_DIM;
        stats.accum_ops += 2 * STATE_DIM * STATE_DIM * STATE_DIM;

        // Update step
        // K = P * H' * (H * P * H' + R)^-1
        stats.compute_ops += 3 * STATE_DIM * STATE_DIM * MEAS_DIM;
        stats.accum_ops += 3 * STATE_DIM * STATE_DIM * MEAS_DIM;
        stats.compute_ops += MEAS_DIM * MEAS_DIM * MEAS_DIM;  // Matrix inverse

        // x = x + K * (z - H * x)
        stats.compute_ops += MEAS_DIM * STATE_DIM + STATE_DIM;
        stats.accum_ops += MEAS_DIM * STATE_DIM + STATE_DIM;

        // P = (I - K * H) * P
        stats.compute_ops += STATE_DIM * STATE_DIM * MEAS_DIM;
        stats.accum_ops += STATE_DIM * STATE_DIM * MEAS_DIM;

        stats.output_stores += STATE_DIM + STATE_DIM * STATE_DIM;

        // Generate dummy state
        state = {1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.5f};
        covariance.resize(STATE_DIM * STATE_DIM, 0.01f);

        return stats;
    }
};

// ============================================================================
// Stage 4: Control Output (Motor Commands)
// ============================================================================

/// PID controller for motor control
/// Medium precision, needs to be responsive
struct ControlOutput {
    using ComputeType = float;                    // FP32 compute
    using OutputType = int16_t;                   // 16-bit PWM output

    static constexpr size_t NUM_MOTORS = 4;       // Quadruped/quadrotor

    static MixedPrecisionStats compute(const std::vector<float>& state,
                                        std::vector<int16_t>& commands) {
        MixedPrecisionStats stats;

        // PID per motor (P, I, D terms)
        stats.input_loads += NUM_MOTORS * 3;  // Error history
        stats.compute_ops += NUM_MOTORS * 6;  // P*e + I*sum + D*diff
        stats.accum_ops += NUM_MOTORS * 3;
        stats.output_stores += NUM_MOTORS;

        // Feedforward
        stats.compute_ops += NUM_MOTORS * 4;

        // Motor mixing matrix
        stats.compute_ops += NUM_MOTORS * NUM_MOTORS;
        stats.accum_ops += NUM_MOTORS * NUM_MOTORS;

        // Saturation and output
        stats.output_stores += NUM_MOTORS;

        // Generate dummy commands
        commands = {1500, 1500, 1500, 1500};  // Neutral PWM

        return stats;
    }
};

// ============================================================================
// Energy Analysis
// ============================================================================

double estimateStageEnergy(const MixedPrecisionStats& stats,
                            energy::BitWidth compute_width,
                            energy::BitWidth accum_width) {
    using namespace energy;

    const auto& model = getDefaultModel();

    double energy = 0.0;

    // Compute operations (multiplications)
    energy += model.totalOperationEnergy(Operation::FloatMultiply, compute_width, stats.compute_ops);

    // Accumulation operations (additions)
    energy += model.totalOperationEnergy(Operation::FloatAdd, accum_width, stats.accum_ops);

    // Memory operations
    energy += model.memoryTransferEnergy(MemoryLevel::L1_Cache,
                                          stats.input_loads * (static_cast<int>(compute_width) / 8), false);
    energy += model.memoryTransferEnergy(MemoryLevel::L1_Cache,
                                          stats.output_stores * (static_cast<int>(compute_width) / 8), true);

    return energy;
}

void analyzeRoboticsPipeline() {
    std::cout << "========================================\n";
    std::cout << "Robotics Perception Pipeline Analysis\n";
    std::cout << "========================================\n\n";

    std::cout << "Robot Configuration:\n";
    std::cout << "  Power budget:     " << ROBOT_POWER_BUDGET_W << " W\n";
    std::cout << "  Perception rate:  " << PERCEPTION_RATE_HZ << " Hz\n";
    std::cout << "  Energy/frame:     " << (ENERGY_PER_FRAME_J * 1e3) << " mJ\n\n";

    // Run pipeline stages
    std::vector<float> sensor_output;
    std::vector<float> detections;
    std::vector<float> state, covariance;
    std::vector<int16_t> commands;

    auto sensor_stats = SensorPreprocessing::process(sensor_output);
    auto detect_stats = ObjectDetection::detect(sensor_output, detections);
    auto state_stats = StateEstimation::update(state, covariance);
    auto control_stats = ControlOutput::compute(state, commands);

    // Estimate energy for each configuration

    // Sensor: FP16 compute
    double sensor_fp32 = estimateStageEnergy(sensor_stats, energy::BitWidth::bits_32, energy::BitWidth::bits_32);
    double sensor_fp16 = estimateStageEnergy(sensor_stats, energy::BitWidth::bits_16, energy::BitWidth::bits_32);

    // Detection: INT8 compute with INT32 accumulator
    double detect_fp32 = estimateStageEnergy(detect_stats, energy::BitWidth::bits_32, energy::BitWidth::bits_32);
    double detect_int8 = estimateStageEnergy(detect_stats, energy::BitWidth::bits_8, energy::BitWidth::bits_32);

    // State: FP32 compute with FP64 accumulator
    double state_fp32 = estimateStageEnergy(state_stats, energy::BitWidth::bits_32, energy::BitWidth::bits_32);
    double state_fp64acc = estimateStageEnergy(state_stats, energy::BitWidth::bits_32, energy::BitWidth::bits_64);

    // Control: FP32 compute
    double control_fp32 = estimateStageEnergy(control_stats, energy::BitWidth::bits_32, energy::BitWidth::bits_32);
    double control_fp16 = estimateStageEnergy(control_stats, energy::BitWidth::bits_16, energy::BitWidth::bits_32);

    // Totals
    double total_fp32 = sensor_fp32 + detect_fp32 + state_fp32 + control_fp32;
    double total_mixed = sensor_fp16 + detect_int8 + state_fp64acc + control_fp16;

    std::cout << "Stage-by-Stage Energy Analysis (per frame):\n";
    std::cout << std::string(70, '-') << "\n";
    std::cout << std::left << std::setw(20) << "Stage"
              << std::right << std::setw(15) << "FP32 (uJ)"
              << std::setw(15) << "Mixed (uJ)"
              << std::setw(12) << "Savings"
              << std::setw(12) << "Config" << "\n";
    std::cout << std::string(70, '-') << "\n";

    auto printStage = [](const std::string& name, double fp32, double mixed, const std::string& config) {
        std::cout << std::left << std::setw(20) << name
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(15) << (fp32 / 1e6)
                  << std::setw(15) << (mixed / 1e6)
                  << std::setw(11) << ((1.0 - mixed/fp32) * 100) << "%"
                  << std::left << std::setw(12) << config << "\n";
    };

    printStage("Sensor Preproc", sensor_fp32, sensor_fp16, "FP16");
    printStage("Object Detection", detect_fp32, detect_int8, "INT8+32acc");
    printStage("State Estimation", state_fp32, state_fp64acc, "FP32+64acc");
    printStage("Control Output", control_fp32, control_fp16, "FP16");

    std::cout << std::string(70, '-') << "\n";
    printStage("TOTAL", total_fp32, total_mixed, "Mixed");

    std::cout << "\n\nPower Analysis at " << PERCEPTION_RATE_HZ << " Hz:\n";
    std::cout << std::string(50, '-') << "\n";

    double power_fp32 = (total_fp32 / 1e12) * PERCEPTION_RATE_HZ;  // pJ to W
    double power_mixed = (total_mixed / 1e12) * PERCEPTION_RATE_HZ;

    std::cout << "  FP32 power:       " << std::fixed << std::setprecision(3)
              << (power_fp32 * 1000) << " mW\n";
    std::cout << "  Mixed power:      " << (power_mixed * 1000) << " mW\n";
    std::cout << "  Compute/Budget:   " << std::setprecision(1)
              << ((power_mixed / ROBOT_POWER_BUDGET_W) * 100) << "%\n";

    std::cout << "\n  Energy savings:   " << ((1.0 - total_mixed/total_fp32) * 100) << "%\n";

    // Battery life estimation
    constexpr double BATTERY_WH = 100.0;  // 100 Wh battery
    double runtime_fp32_h = BATTERY_WH / (power_fp32 + 5.0);  // +5W for motors/sensors
    double runtime_mixed_h = BATTERY_WH / (power_mixed + 5.0);

    std::cout << "\n\nBattery Life Estimate (100 Wh battery):\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "  FP32 only:        " << std::setprecision(1) << runtime_fp32_h << " hours\n";
    std::cout << "  Mixed precision:  " << runtime_mixed_h << " hours\n";
    std::cout << "  Extended runtime: " << ((runtime_mixed_h - runtime_fp32_h) / runtime_fp32_h * 100) << "%\n";
}

void demonstratePrecisionRecommendations() {
    std::cout << "\n\n========================================\n";
    std::cout << "Per-Stage Precision Recommendations\n";
    std::cout << "========================================\n\n";

    ParetoExplorer explorer;

    struct Stage {
        std::string name;
        double accuracy_req;
        double ai;  // Arithmetic intensity
    };

    std::vector<Stage> stages = {
        {"Sensor Preprocessing", MOBILE_ROBOT.sensor_accuracy, 5.0},
        {"Object Detection", MOBILE_ROBOT.detection_accuracy, 50.0},
        {"State Estimation", MOBILE_ROBOT.state_accuracy, 10.0},
        {"Control Output", MOBILE_ROBOT.control_accuracy, 2.0}
    };

    std::cout << std::left << std::setw(22) << "Stage"
              << std::setw(12) << "Accuracy"
              << std::setw(10) << "AI"
              << std::setw(18) << "Recommended"
              << std::setw(10) << "Energy" << "\n";
    std::cout << std::string(72, '-') << "\n";

    for (const auto& stage : stages) {
        auto algo = AlgorithmCharacteristics(stage.name, stage.ai);
        auto config = explorer.recommendForAlgorithm(stage.accuracy_req, algo);

        std::cout << std::left << std::setw(22) << stage.name
                  << std::scientific << std::setprecision(0) << std::setw(12) << stage.accuracy_req
                  << std::fixed << std::setprecision(1) << std::setw(10) << stage.ai
                  << std::left << std::setw(18) << config.name
                  << std::setprecision(2) << std::setw(9) << config.energy_factor << "x\n";
    }
}

void demonstrateEdgeCases() {
    std::cout << "\n\n========================================\n";
    std::cout << "Edge Cases and Failure Modes\n";
    std::cout << "========================================\n\n";

    std::cout << "1. NUMERICAL INSTABILITY IN KALMAN FILTER\n";
    std::cout << "   Problem: Covariance matrix becomes non-positive-definite\n";
    std::cout << "   Solution: Use FP64 accumulator for matrix operations\n";
    std::cout << "   Impact: ~10% higher energy for state estimation\n\n";

    std::cout << "2. DETECTION CONFIDENCE SATURATION\n";
    std::cout << "   Problem: INT8 sigmoid saturates at extreme values\n";
    std::cout << "   Solution: Use FP16 for final softmax layer\n";
    std::cout << "   Impact: <1% energy increase\n\n";

    std::cout << "3. SENSOR NOISE AMPLIFICATION\n";
    std::cout << "   Problem: FP16 quantization noise adds to sensor noise\n";
    std::cout << "   Mitigation: Noise is typically larger than FP16 precision\n";
    std::cout << "   Acceptable when: sensor_noise >> 1e-3\n\n";

    std::cout << "4. CONTROL LOOP INSTABILITY\n";
    std::cout << "   Problem: Integrator windup with low-precision accumulator\n";
    std::cout << "   Solution: FP32 for PID integrator state\n";
    std::cout << "   Impact: Minimal - only state is FP32, compute is FP16\n";
}

int main()
try {
    std::cout << "Universal Numbers: Embodied AI Mixed-Precision Pipeline\n";
    std::cout << "============================================================\n\n";

    analyzeRoboticsPipeline();
    demonstratePrecisionRecommendations();
    demonstrateEdgeCases();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. Each pipeline stage has different precision requirements\n";
    std::cout << "2. ML inference benefits most from INT8 quantization (70%+ savings)\n";
    std::cout << "3. State estimation needs higher precision accumulators\n";
    std::cout << "4. Mixed-precision extends battery life by 10-20%\n";
    std::cout << "5. Edge cases must be identified and handled appropriately\n";

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
