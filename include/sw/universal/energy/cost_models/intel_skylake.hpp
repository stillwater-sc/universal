#pragma once
// intel_skylake.hpp: energy cost model for Intel Skylake microarchitecture
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Intel Skylake (6th-10th gen Core, Xeon Scalable) - 14nm process
// Estimated values based on:
// - Intel optimization manuals
// - Published measurements (Leng et al., ISCA 2013; Rotem et al., ISSCC 2015)
// - Scaling from 45nm baseline (~3x improvement per 2 process generations)
//
// Note: Actual energy varies significantly with:
// - Operating frequency (higher freq = higher voltage = higher energy)
// - Vector width (SIMD operations amortize overhead)
// - Port utilization and microarchitectural state
//
// All values in picojoules (pJ).

#include "energy_model.hpp"

namespace sw { namespace universal { namespace energy {

/// Intel Skylake (14nm) energy model
/// Scaled from 45nm baseline with ~3x improvement
inline constexpr EnergyCostModel intel_skylake_model() {
    EnergyCostModel model;

    model.name = "Intel Skylake (14nm)";
    model.description = "Energy model for Intel Skylake microarchitecture";
    model.process_nm = 14;

    // Scaling factor from 45nm: approximately 3x reduction
    // (45nm -> 32nm -> 22nm -> 14nm, ~30% per node)
    // Values below are already scaled; this constant documents the methodology
    // constexpr double scale = 0.33;

    // Integer operations (pJ) - scaled from 45nm
    model.ops.int_add[0] = 0.01;   // 8-bit
    model.ops.int_add[1] = 0.017;  // 16-bit
    model.ops.int_add[2] = 0.033;  // 32-bit
    model.ops.int_add[3] = 0.066;  // 64-bit

    model.ops.int_sub[0] = 0.01;
    model.ops.int_sub[1] = 0.017;
    model.ops.int_sub[2] = 0.033;
    model.ops.int_sub[3] = 0.066;

    model.ops.int_mul[0] = 0.066;  // 8-bit
    model.ops.int_mul[1] = 0.33;   // 16-bit
    model.ops.int_mul[2] = 1.0;    // 32-bit
    model.ops.int_mul[3] = 4.0;    // 64-bit

    model.ops.int_div[0] = 0.33;   // 8-bit
    model.ops.int_div[1] = 1.3;    // 16-bit
    model.ops.int_div[2] = 5.0;    // 32-bit (IDIV is expensive)
    model.ops.int_div[3] = 20.0;   // 64-bit

    // Floating-point operations (pJ)
    // Skylake has very efficient FP units with 2x 256-bit FMA
    model.ops.fp_add[0] = 0.066;   // 8-bit (estimated, not native)
    model.ops.fp_add[1] = 0.13;    // 16-bit (FP16 via conversion)
    model.ops.fp_add[2] = 0.3;     // 32-bit
    model.ops.fp_add[3] = 0.6;     // 64-bit

    model.ops.fp_sub[0] = 0.066;
    model.ops.fp_sub[1] = 0.13;
    model.ops.fp_sub[2] = 0.3;
    model.ops.fp_sub[3] = 0.6;

    model.ops.fp_mul[0] = 0.17;    // 8-bit
    model.ops.fp_mul[1] = 0.37;    // 16-bit
    model.ops.fp_mul[2] = 1.2;     // 32-bit
    model.ops.fp_mul[3] = 5.0;     // 64-bit

    // Division on Skylake: ~14-23 cycles for float, ~14-45 for double
    model.ops.fp_div[0] = 0.66;    // 8-bit
    model.ops.fp_div[1] = 1.7;     // 16-bit
    model.ops.fp_div[2] = 6.6;     // 32-bit
    model.ops.fp_div[3] = 26.0;    // 64-bit

    // FMA: single operation, very efficient on Skylake
    model.ops.fp_fma[0] = 0.2;     // 8-bit
    model.ops.fp_fma[1] = 0.47;    // 16-bit
    model.ops.fp_fma[2] = 1.5;     // 32-bit
    model.ops.fp_fma[3] = 5.3;     // 64-bit

    // Square root: ~12-18 cycles
    model.ops.fp_sqrt[0] = 1.0;    // 8-bit
    model.ops.fp_sqrt[1] = 3.3;    // 16-bit
    model.ops.fp_sqrt[2] = 13.0;   // 32-bit
    model.ops.fp_sqrt[3] = 50.0;   // 64-bit

    // Comparison
    model.ops.compare[0] = 0.01;
    model.ops.compare[1] = 0.017;
    model.ops.compare[2] = 0.033;
    model.ops.compare[3] = 0.066;

    // Bitwise logic
    model.ops.logic[0] = 0.003;
    model.ops.logic[1] = 0.007;
    model.ops.logic[2] = 0.013;
    model.ops.logic[3] = 0.026;

    // Shift operations
    model.ops.shift[0] = 0.007;
    model.ops.shift[1] = 0.01;
    model.ops.shift[2] = 0.02;
    model.ops.shift[3] = 0.04;

    // Memory access costs (pJ per access)
    // Skylake: 32KB L1D, 256KB L2, ~1.375MB L3 per core (shared)
    model.mem.reg_read  = 0.33;    // Register file
    model.mem.reg_write = 0.33;

    model.mem.l1_read   = 3.3;     // 32KB L1 cache
    model.mem.l1_write  = 3.3;

    model.mem.l2_read   = 17.0;    // 256KB L2 cache
    model.mem.l2_write  = 17.0;

    model.mem.l3_read   = 66.0;    // Shared L3 (LLC)
    model.mem.l3_write  = 66.0;

    // DDR4 DRAM: improved from DDR3, but still expensive
    model.mem.dram_read  = 650.0;
    model.mem.dram_write = 650.0;

    // Data movement costs (pJ per bit)
    model.data_movement.on_chip_per_bit  = 0.033;
    model.data_movement.off_chip_per_bit = 5.0;

    return model;
}

/// Convenience: get the Intel Skylake model
inline const EnergyCostModel& getIntelSkylakeModel() {
    static const EnergyCostModel model = intel_skylake_model();
    return model;
}

}}} // namespace sw::universal::energy
