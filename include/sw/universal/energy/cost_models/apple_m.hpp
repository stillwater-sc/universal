#pragma once
// apple_m.hpp: energy cost models for Apple Silicon (M1, M2, M3 series)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Apple Silicon energy models:
// - M1 (5nm TSMC N5): First Apple Silicon for Macs
// - M2 (5nm TSMC N5P): Improved efficiency and performance
// - M3 (3nm TSMC N3B): Latest generation with hardware ray tracing
//
// Estimated values based on:
// - Published power measurements and reviews
// - Apple efficiency claims and benchmarks
// - ARM Cortex-A scaling data
// - TSMC process node characteristics
//
// Note: Apple Silicon uses a heterogeneous design with:
// - Performance cores (Firestorm/Avalanche/Everest)
// - Efficiency cores (Icestorm/Blizzard/Sawtooth)
// - Neural Engine (ANE) for ML workloads
// - GPU cores (unified memory architecture)
//
// These models represent the performance cores (P-cores).
// E-cores are approximately 3-4x more efficient but lower throughput.
//
// All values in picojoules (pJ).

#include "energy_model.hpp"

namespace sw { namespace universal { namespace energy {

/// Apple M1 (5nm TSMC N5) Performance Cores - Firestorm
/// Very wide OoO design with excellent energy efficiency
inline constexpr EnergyCostModel apple_m1_model() {
    EnergyCostModel model;

    model.name = "Apple M1 (5nm)";
    model.description = "Energy model for Apple M1 Firestorm performance cores";
    model.process_nm = 5;

    // Apple Silicon is extremely efficient due to:
    // - 5nm process
    // - Custom ARM design optimized for efficiency
    // - Unified memory architecture (no discrete GPU memory transfers)
    // - Native FP16 support in NEON/AMX

    // Integer operations (pJ)
    model.ops.int_add[0] = 0.0025;   // 8-bit
    model.ops.int_add[1] = 0.004;    // 16-bit
    model.ops.int_add[2] = 0.008;    // 32-bit
    model.ops.int_add[3] = 0.016;    // 64-bit

    model.ops.int_sub[0] = 0.0025;
    model.ops.int_sub[1] = 0.004;
    model.ops.int_sub[2] = 0.008;
    model.ops.int_sub[3] = 0.016;

    model.ops.int_mul[0] = 0.016;    // 8-bit
    model.ops.int_mul[1] = 0.08;     // 16-bit
    model.ops.int_mul[2] = 0.25;     // 32-bit
    model.ops.int_mul[3] = 1.0;      // 64-bit

    model.ops.int_div[0] = 0.08;     // 8-bit
    model.ops.int_div[1] = 0.32;     // 16-bit
    model.ops.int_div[2] = 1.25;     // 32-bit
    model.ops.int_div[3] = 5.0;      // 64-bit

    // Floating-point operations (pJ)
    // M1 has excellent FP16 support via NEON and AMX
    model.ops.fp_add[0] = 0.016;     // 8-bit (emulated)
    model.ops.fp_add[1] = 0.025;     // 16-bit (native FP16)
    model.ops.fp_add[2] = 0.075;     // 32-bit
    model.ops.fp_add[3] = 0.15;      // 64-bit

    model.ops.fp_sub[0] = 0.016;
    model.ops.fp_sub[1] = 0.025;
    model.ops.fp_sub[2] = 0.075;
    model.ops.fp_sub[3] = 0.15;

    // Multiplication
    model.ops.fp_mul[0] = 0.04;      // 8-bit
    model.ops.fp_mul[1] = 0.06;      // 16-bit (native)
    model.ops.fp_mul[2] = 0.3;       // 32-bit
    model.ops.fp_mul[3] = 1.25;      // 64-bit

    // Division
    model.ops.fp_div[0] = 0.16;      // 8-bit
    model.ops.fp_div[1] = 0.32;      // 16-bit
    model.ops.fp_div[2] = 1.6;       // 32-bit
    model.ops.fp_div[3] = 6.5;       // 64-bit

    // FMA: Apple has efficient FMA in NEON
    model.ops.fp_fma[0] = 0.05;      // 8-bit
    model.ops.fp_fma[1] = 0.075;     // 16-bit (native)
    model.ops.fp_fma[2] = 0.38;      // 32-bit
    model.ops.fp_fma[3] = 1.35;      // 64-bit

    // Square root
    model.ops.fp_sqrt[0] = 0.25;     // 8-bit
    model.ops.fp_sqrt[1] = 0.5;      // 16-bit
    model.ops.fp_sqrt[2] = 3.2;      // 32-bit
    model.ops.fp_sqrt[3] = 12.5;     // 64-bit

    // Comparison
    model.ops.compare[0] = 0.0025;
    model.ops.compare[1] = 0.004;
    model.ops.compare[2] = 0.008;
    model.ops.compare[3] = 0.016;

    // Bitwise logic
    model.ops.logic[0] = 0.0008;
    model.ops.logic[1] = 0.0016;
    model.ops.logic[2] = 0.0032;
    model.ops.logic[3] = 0.0065;

    // Shift operations
    model.ops.shift[0] = 0.0016;
    model.ops.shift[1] = 0.0025;
    model.ops.shift[2] = 0.005;
    model.ops.shift[3] = 0.01;

    // Memory access costs (pJ per access)
    // M1: 192KB L1 (128KB I + 64KB D), 12MB shared L2
    // Unified memory architecture - no separate GPU memory
    model.mem.reg_read  = 0.08;
    model.mem.reg_write = 0.08;

    model.mem.l1_read   = 0.8;       // 64KB L1D per P-core
    model.mem.l1_write  = 0.8;

    model.mem.l2_read   = 4.0;       // 12MB shared L2
    model.mem.l2_write  = 4.0;

    model.mem.l3_read   = 16.0;      // System Level Cache (SLC)
    model.mem.l3_write  = 16.0;

    // LPDDR4X unified memory: very efficient
    model.mem.dram_read  = 320.0;
    model.mem.dram_write = 320.0;

    // Data movement costs (pJ per bit)
    // Unified memory architecture reduces data movement
    model.data_movement.on_chip_per_bit  = 0.008;
    model.data_movement.off_chip_per_bit = 2.0;

    return model;
}

/// Apple M1 Efficiency Cores - Icestorm
/// Smaller, more power-efficient cores for background tasks
inline constexpr EnergyCostModel apple_m1_efficiency_model() {
    EnergyCostModel model;

    model.name = "Apple M1 E-core (5nm)";
    model.description = "Energy model for Apple M1 Icestorm efficiency cores";
    model.process_nm = 5;

    // E-cores are ~3-4x more efficient but lower throughput

    // Integer operations (pJ) - ~3x more efficient
    model.ops.int_add[0] = 0.0008;
    model.ops.int_add[1] = 0.0013;
    model.ops.int_add[2] = 0.0027;
    model.ops.int_add[3] = 0.0053;

    model.ops.int_sub[0] = 0.0008;
    model.ops.int_sub[1] = 0.0013;
    model.ops.int_sub[2] = 0.0027;
    model.ops.int_sub[3] = 0.0053;

    model.ops.int_mul[0] = 0.0053;
    model.ops.int_mul[1] = 0.027;
    model.ops.int_mul[2] = 0.083;
    model.ops.int_mul[3] = 0.33;

    model.ops.int_div[0] = 0.027;
    model.ops.int_div[1] = 0.11;
    model.ops.int_div[2] = 0.42;
    model.ops.int_div[3] = 1.67;

    // Floating-point operations (pJ)
    model.ops.fp_add[0] = 0.0053;
    model.ops.fp_add[1] = 0.0083;
    model.ops.fp_add[2] = 0.025;
    model.ops.fp_add[3] = 0.05;

    model.ops.fp_sub[0] = 0.0053;
    model.ops.fp_sub[1] = 0.0083;
    model.ops.fp_sub[2] = 0.025;
    model.ops.fp_sub[3] = 0.05;

    model.ops.fp_mul[0] = 0.013;
    model.ops.fp_mul[1] = 0.02;
    model.ops.fp_mul[2] = 0.1;
    model.ops.fp_mul[3] = 0.42;

    model.ops.fp_div[0] = 0.053;
    model.ops.fp_div[1] = 0.11;
    model.ops.fp_div[2] = 0.53;
    model.ops.fp_div[3] = 2.17;

    model.ops.fp_fma[0] = 0.017;
    model.ops.fp_fma[1] = 0.025;
    model.ops.fp_fma[2] = 0.127;
    model.ops.fp_fma[3] = 0.45;

    model.ops.fp_sqrt[0] = 0.083;
    model.ops.fp_sqrt[1] = 0.167;
    model.ops.fp_sqrt[2] = 1.07;
    model.ops.fp_sqrt[3] = 4.17;

    // Comparison
    model.ops.compare[0] = 0.0008;
    model.ops.compare[1] = 0.0013;
    model.ops.compare[2] = 0.0027;
    model.ops.compare[3] = 0.0053;

    // Bitwise logic
    model.ops.logic[0] = 0.00027;
    model.ops.logic[1] = 0.00053;
    model.ops.logic[2] = 0.0011;
    model.ops.logic[3] = 0.0022;

    // Shift operations
    model.ops.shift[0] = 0.00053;
    model.ops.shift[1] = 0.00083;
    model.ops.shift[2] = 0.0017;
    model.ops.shift[3] = 0.0033;

    // Memory access costs (pJ per access)
    // E-cores: smaller 32KB L1D, shared L2
    model.mem.reg_read  = 0.027;
    model.mem.reg_write = 0.027;

    model.mem.l1_read   = 0.27;
    model.mem.l1_write  = 0.27;

    model.mem.l2_read   = 1.33;      // Shared with P-cores
    model.mem.l2_write  = 1.33;

    model.mem.l3_read   = 5.33;
    model.mem.l3_write  = 5.33;

    // LPDDR4X (shared)
    model.mem.dram_read  = 320.0;    // Same as P-core
    model.mem.dram_write = 320.0;

    // Data movement
    model.data_movement.on_chip_per_bit  = 0.0027;
    model.data_movement.off_chip_per_bit = 2.0;

    return model;
}

/// Apple M2 (5nm TSMC N5P) Performance Cores - Avalanche
/// Improved from M1 with better memory bandwidth
inline constexpr EnergyCostModel apple_m2_model() {
    EnergyCostModel model;

    model.name = "Apple M2 (5nm+)";
    model.description = "Energy model for Apple M2 Avalanche performance cores";
    model.process_nm = 5;

    // M2 is ~10-15% more efficient than M1

    // Integer operations (pJ)
    model.ops.int_add[0] = 0.0022;
    model.ops.int_add[1] = 0.0035;
    model.ops.int_add[2] = 0.007;
    model.ops.int_add[3] = 0.014;

    model.ops.int_sub[0] = 0.0022;
    model.ops.int_sub[1] = 0.0035;
    model.ops.int_sub[2] = 0.007;
    model.ops.int_sub[3] = 0.014;

    model.ops.int_mul[0] = 0.014;
    model.ops.int_mul[1] = 0.07;
    model.ops.int_mul[2] = 0.22;
    model.ops.int_mul[3] = 0.88;

    model.ops.int_div[0] = 0.07;
    model.ops.int_div[1] = 0.28;
    model.ops.int_div[2] = 1.1;
    model.ops.int_div[3] = 4.4;

    // Floating-point operations (pJ)
    model.ops.fp_add[0] = 0.014;
    model.ops.fp_add[1] = 0.022;
    model.ops.fp_add[2] = 0.066;
    model.ops.fp_add[3] = 0.132;

    model.ops.fp_sub[0] = 0.014;
    model.ops.fp_sub[1] = 0.022;
    model.ops.fp_sub[2] = 0.066;
    model.ops.fp_sub[3] = 0.132;

    model.ops.fp_mul[0] = 0.035;
    model.ops.fp_mul[1] = 0.053;
    model.ops.fp_mul[2] = 0.264;
    model.ops.fp_mul[3] = 1.1;

    model.ops.fp_div[0] = 0.14;
    model.ops.fp_div[1] = 0.28;
    model.ops.fp_div[2] = 1.4;
    model.ops.fp_div[3] = 5.7;

    model.ops.fp_fma[0] = 0.044;
    model.ops.fp_fma[1] = 0.066;
    model.ops.fp_fma[2] = 0.33;
    model.ops.fp_fma[3] = 1.2;

    model.ops.fp_sqrt[0] = 0.22;
    model.ops.fp_sqrt[1] = 0.44;
    model.ops.fp_sqrt[2] = 2.8;
    model.ops.fp_sqrt[3] = 11.0;

    // Comparison
    model.ops.compare[0] = 0.0022;
    model.ops.compare[1] = 0.0035;
    model.ops.compare[2] = 0.007;
    model.ops.compare[3] = 0.014;

    // Bitwise logic
    model.ops.logic[0] = 0.0007;
    model.ops.logic[1] = 0.0014;
    model.ops.logic[2] = 0.0028;
    model.ops.logic[3] = 0.0057;

    // Shift operations
    model.ops.shift[0] = 0.0014;
    model.ops.shift[1] = 0.0022;
    model.ops.shift[2] = 0.0044;
    model.ops.shift[3] = 0.0088;

    // Memory access costs (pJ per access)
    // M2: 192KB L1, 16MB shared L2
    model.mem.reg_read  = 0.07;
    model.mem.reg_write = 0.07;

    model.mem.l1_read   = 0.7;
    model.mem.l1_write  = 0.7;

    model.mem.l2_read   = 3.5;       // 16MB shared L2
    model.mem.l2_write  = 3.5;

    model.mem.l3_read   = 14.0;
    model.mem.l3_write  = 14.0;

    // LPDDR5 unified memory: more efficient than LPDDR4X
    model.mem.dram_read  = 280.0;
    model.mem.dram_write = 280.0;

    // Data movement
    model.data_movement.on_chip_per_bit  = 0.007;
    model.data_movement.off_chip_per_bit = 1.8;

    return model;
}

/// Apple M3 (3nm TSMC N3B) Performance Cores - Everest
/// Latest generation with hardware ray tracing, mesh shaders
inline constexpr EnergyCostModel apple_m3_model() {
    EnergyCostModel model;

    model.name = "Apple M3 (3nm)";
    model.description = "Energy model for Apple M3 Everest performance cores";
    model.process_nm = 3;

    // 3nm provides ~30% power reduction vs 5nm

    // Integer operations (pJ)
    model.ops.int_add[0] = 0.0015;
    model.ops.int_add[1] = 0.0025;
    model.ops.int_add[2] = 0.005;
    model.ops.int_add[3] = 0.01;

    model.ops.int_sub[0] = 0.0015;
    model.ops.int_sub[1] = 0.0025;
    model.ops.int_sub[2] = 0.005;
    model.ops.int_sub[3] = 0.01;

    model.ops.int_mul[0] = 0.01;
    model.ops.int_mul[1] = 0.05;
    model.ops.int_mul[2] = 0.15;
    model.ops.int_mul[3] = 0.6;

    model.ops.int_div[0] = 0.05;
    model.ops.int_div[1] = 0.2;
    model.ops.int_div[2] = 0.75;
    model.ops.int_div[3] = 3.0;

    // Floating-point operations (pJ)
    model.ops.fp_add[0] = 0.01;
    model.ops.fp_add[1] = 0.015;
    model.ops.fp_add[2] = 0.046;
    model.ops.fp_add[3] = 0.092;

    model.ops.fp_sub[0] = 0.01;
    model.ops.fp_sub[1] = 0.015;
    model.ops.fp_sub[2] = 0.046;
    model.ops.fp_sub[3] = 0.092;

    model.ops.fp_mul[0] = 0.025;
    model.ops.fp_mul[1] = 0.037;
    model.ops.fp_mul[2] = 0.185;
    model.ops.fp_mul[3] = 0.77;

    model.ops.fp_div[0] = 0.1;
    model.ops.fp_div[1] = 0.2;
    model.ops.fp_div[2] = 1.0;
    model.ops.fp_div[3] = 4.0;

    model.ops.fp_fma[0] = 0.031;
    model.ops.fp_fma[1] = 0.046;
    model.ops.fp_fma[2] = 0.23;
    model.ops.fp_fma[3] = 0.84;

    model.ops.fp_sqrt[0] = 0.15;
    model.ops.fp_sqrt[1] = 0.31;
    model.ops.fp_sqrt[2] = 2.0;
    model.ops.fp_sqrt[3] = 7.7;

    // Comparison
    model.ops.compare[0] = 0.0015;
    model.ops.compare[1] = 0.0025;
    model.ops.compare[2] = 0.005;
    model.ops.compare[3] = 0.01;

    // Bitwise logic
    model.ops.logic[0] = 0.0005;
    model.ops.logic[1] = 0.001;
    model.ops.logic[2] = 0.002;
    model.ops.logic[3] = 0.004;

    // Shift operations
    model.ops.shift[0] = 0.001;
    model.ops.shift[1] = 0.0015;
    model.ops.shift[2] = 0.003;
    model.ops.shift[3] = 0.006;

    // Memory access costs (pJ per access)
    // M3: 192KB L1, up to 24MB shared L2
    model.mem.reg_read  = 0.05;
    model.mem.reg_write = 0.05;

    model.mem.l1_read   = 0.5;
    model.mem.l1_write  = 0.5;

    model.mem.l2_read   = 2.5;
    model.mem.l2_write  = 2.5;

    model.mem.l3_read   = 10.0;
    model.mem.l3_write  = 10.0;

    // LPDDR5 (improved)
    model.mem.dram_read  = 250.0;
    model.mem.dram_write = 250.0;

    // Data movement
    model.data_movement.on_chip_per_bit  = 0.005;
    model.data_movement.off_chip_per_bit = 1.5;

    return model;
}

/// Convenience functions
inline const EnergyCostModel& getAppleM1Model() {
    static const EnergyCostModel model = apple_m1_model();
    return model;
}

inline const EnergyCostModel& getAppleM1EfficiencyModel() {
    static const EnergyCostModel model = apple_m1_efficiency_model();
    return model;
}

inline const EnergyCostModel& getAppleM2Model() {
    static const EnergyCostModel model = apple_m2_model();
    return model;
}

inline const EnergyCostModel& getAppleM3Model() {
    static const EnergyCostModel model = apple_m3_model();
    return model;
}

}}} // namespace sw::universal::energy
