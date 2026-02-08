#pragma once
// arm_cortex_a.hpp: energy cost model for ARM Cortex-A series processors
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// ARM Cortex-A series (A72/A76/A78 class) - 7nm-16nm process
// Estimated values based on:
// - ARM technical reference manuals
// - Published measurements (Blem et al., ISCA 2013)
// - Mobile SoC power analysis papers
//
// ARM cores prioritize power efficiency over peak performance.
// They typically operate at lower voltages and frequencies than
// Intel desktop/server parts.
//
// All values in picojoules (pJ).

#include "energy_model.hpp"

namespace sw { namespace universal { namespace energy {

/// ARM Cortex-A76/A78 class energy model (7nm)
/// These are high-performance mobile cores (big cores in big.LITTLE)
inline constexpr EnergyCostModel arm_cortex_a76_model() {
    EnergyCostModel model;

    model.name = "ARM Cortex-A76/A78 (7nm)";
    model.description = "Energy model for ARM high-performance mobile cores";
    model.process_nm = 7;

    // ARM cores are more power-efficient than x86 cores due to:
    // - Simpler decode (fixed-width instructions)
    // - Lower operating voltage
    // - Smaller out-of-order window
    // Typically ~2-3x more efficient per operation than Intel desktop

    // Integer operations (pJ)
    model.ops.int_add[0] = 0.005;  // 8-bit
    model.ops.int_add[1] = 0.008;  // 16-bit
    model.ops.int_add[2] = 0.015;  // 32-bit
    model.ops.int_add[3] = 0.03;   // 64-bit

    model.ops.int_sub[0] = 0.005;
    model.ops.int_sub[1] = 0.008;
    model.ops.int_sub[2] = 0.015;
    model.ops.int_sub[3] = 0.03;

    model.ops.int_mul[0] = 0.03;   // 8-bit
    model.ops.int_mul[1] = 0.15;   // 16-bit
    model.ops.int_mul[2] = 0.5;    // 32-bit
    model.ops.int_mul[3] = 2.0;    // 64-bit

    // ARM integer division is relatively efficient
    model.ops.int_div[0] = 0.15;   // 8-bit
    model.ops.int_div[1] = 0.6;    // 16-bit
    model.ops.int_div[2] = 2.5;    // 32-bit (SDIV/UDIV)
    model.ops.int_div[3] = 10.0;   // 64-bit

    // Floating-point operations (pJ)
    // ARM NEON/FP units are power-optimized
    model.ops.fp_add[0] = 0.03;    // 8-bit (estimated)
    model.ops.fp_add[1] = 0.06;    // 16-bit (native FP16 on A76+)
    model.ops.fp_add[2] = 0.15;    // 32-bit
    model.ops.fp_add[3] = 0.3;     // 64-bit

    model.ops.fp_sub[0] = 0.03;
    model.ops.fp_sub[1] = 0.06;
    model.ops.fp_sub[2] = 0.15;
    model.ops.fp_sub[3] = 0.3;

    // ARM has native FP16 support from A76 onwards
    model.ops.fp_mul[0] = 0.08;    // 8-bit
    model.ops.fp_mul[1] = 0.17;    // 16-bit (native)
    model.ops.fp_mul[2] = 0.6;     // 32-bit
    model.ops.fp_mul[3] = 2.5;     // 64-bit

    model.ops.fp_div[0] = 0.3;     // 8-bit
    model.ops.fp_div[1] = 0.8;     // 16-bit
    model.ops.fp_div[2] = 3.0;     // 32-bit
    model.ops.fp_div[3] = 12.0;    // 64-bit

    // FMA: native support via NEON/FP
    model.ops.fp_fma[0] = 0.1;     // 8-bit
    model.ops.fp_fma[1] = 0.22;    // 16-bit (native)
    model.ops.fp_fma[2] = 0.75;    // 32-bit
    model.ops.fp_fma[3] = 2.6;     // 64-bit

    // Square root
    model.ops.fp_sqrt[0] = 0.5;    // 8-bit
    model.ops.fp_sqrt[1] = 1.5;    // 16-bit
    model.ops.fp_sqrt[2] = 6.0;    // 32-bit
    model.ops.fp_sqrt[3] = 25.0;   // 64-bit

    // Comparison
    model.ops.compare[0] = 0.005;
    model.ops.compare[1] = 0.008;
    model.ops.compare[2] = 0.015;
    model.ops.compare[3] = 0.03;

    // Bitwise logic
    model.ops.logic[0] = 0.002;
    model.ops.logic[1] = 0.003;
    model.ops.logic[2] = 0.006;
    model.ops.logic[3] = 0.012;

    // Shift operations
    model.ops.shift[0] = 0.003;
    model.ops.shift[1] = 0.005;
    model.ops.shift[2] = 0.01;
    model.ops.shift[3] = 0.02;

    // Memory access costs (pJ per access)
    // Mobile SoCs have smaller caches but lower power
    // Typical A76: 64KB L1D, 256KB-512KB L2, 2-4MB shared L3
    model.mem.reg_read  = 0.15;    // Register file
    model.mem.reg_write = 0.15;

    model.mem.l1_read   = 1.5;     // 64KB L1 cache
    model.mem.l1_write  = 1.5;

    model.mem.l2_read   = 8.0;     // 256-512KB L2 cache
    model.mem.l2_write  = 8.0;

    model.mem.l3_read   = 30.0;    // Shared L3 (when present)
    model.mem.l3_write  = 30.0;

    // LPDDR4/5 DRAM: lower power than desktop DDR4
    model.mem.dram_read  = 400.0;
    model.mem.dram_write = 400.0;

    // Data movement costs (pJ per bit)
    model.data_movement.on_chip_per_bit  = 0.015;
    model.data_movement.off_chip_per_bit = 3.0;

    return model;
}

/// ARM Cortex-A55 class energy model (7nm)
/// These are efficiency cores (LITTLE cores in big.LITTLE)
inline constexpr EnergyCostModel arm_cortex_a55_model() {
    EnergyCostModel model;

    model.name = "ARM Cortex-A55 (7nm)";
    model.description = "Energy model for ARM efficiency mobile cores";
    model.process_nm = 7;

    // A55 is an in-order core, much simpler and lower power than A76
    // Typically ~3-4x more efficient than A76 at same workload

    // Integer operations (pJ) - very efficient
    model.ops.int_add[0] = 0.002;  // 8-bit
    model.ops.int_add[1] = 0.003;  // 16-bit
    model.ops.int_add[2] = 0.006;  // 32-bit
    model.ops.int_add[3] = 0.012;  // 64-bit

    model.ops.int_sub[0] = 0.002;
    model.ops.int_sub[1] = 0.003;
    model.ops.int_sub[2] = 0.006;
    model.ops.int_sub[3] = 0.012;

    model.ops.int_mul[0] = 0.012;  // 8-bit
    model.ops.int_mul[1] = 0.06;   // 16-bit
    model.ops.int_mul[2] = 0.2;    // 32-bit
    model.ops.int_mul[3] = 0.8;    // 64-bit

    model.ops.int_div[0] = 0.06;   // 8-bit
    model.ops.int_div[1] = 0.24;   // 16-bit
    model.ops.int_div[2] = 1.0;    // 32-bit
    model.ops.int_div[3] = 4.0;    // 64-bit

    // Floating-point operations (pJ)
    model.ops.fp_add[0] = 0.012;   // 8-bit
    model.ops.fp_add[1] = 0.024;   // 16-bit
    model.ops.fp_add[2] = 0.06;    // 32-bit
    model.ops.fp_add[3] = 0.12;    // 64-bit

    model.ops.fp_sub[0] = 0.012;
    model.ops.fp_sub[1] = 0.024;
    model.ops.fp_sub[2] = 0.06;
    model.ops.fp_sub[3] = 0.12;

    model.ops.fp_mul[0] = 0.03;    // 8-bit
    model.ops.fp_mul[1] = 0.07;    // 16-bit
    model.ops.fp_mul[2] = 0.24;    // 32-bit
    model.ops.fp_mul[3] = 1.0;     // 64-bit

    model.ops.fp_div[0] = 0.12;    // 8-bit
    model.ops.fp_div[1] = 0.32;    // 16-bit
    model.ops.fp_div[2] = 1.2;     // 32-bit
    model.ops.fp_div[3] = 5.0;     // 64-bit

    model.ops.fp_fma[0] = 0.04;    // 8-bit
    model.ops.fp_fma[1] = 0.09;    // 16-bit
    model.ops.fp_fma[2] = 0.3;     // 32-bit
    model.ops.fp_fma[3] = 1.1;     // 64-bit

    model.ops.fp_sqrt[0] = 0.2;    // 8-bit
    model.ops.fp_sqrt[1] = 0.6;    // 16-bit
    model.ops.fp_sqrt[2] = 2.4;    // 32-bit
    model.ops.fp_sqrt[3] = 10.0;   // 64-bit

    model.ops.compare[0] = 0.002;
    model.ops.compare[1] = 0.003;
    model.ops.compare[2] = 0.006;
    model.ops.compare[3] = 0.012;

    model.ops.logic[0] = 0.001;
    model.ops.logic[1] = 0.0015;
    model.ops.logic[2] = 0.003;
    model.ops.logic[3] = 0.006;

    model.ops.shift[0] = 0.0015;
    model.ops.shift[1] = 0.002;
    model.ops.shift[2] = 0.004;
    model.ops.shift[3] = 0.008;

    // Memory access costs (pJ per access)
    // A55: 32KB L1D, 128KB-256KB L2
    model.mem.reg_read  = 0.06;
    model.mem.reg_write = 0.06;

    model.mem.l1_read   = 0.6;     // 32KB L1
    model.mem.l1_write  = 0.6;

    model.mem.l2_read   = 3.2;     // 128-256KB L2
    model.mem.l2_write  = 3.2;

    model.mem.l3_read   = 12.0;    // Shared L3 (if present)
    model.mem.l3_write  = 12.0;

    model.mem.dram_read  = 400.0;  // Same DRAM
    model.mem.dram_write = 400.0;

    model.data_movement.on_chip_per_bit  = 0.006;
    model.data_movement.off_chip_per_bit = 3.0;

    return model;
}

/// Convenience: get ARM A76 model (default high-performance mobile)
inline const EnergyCostModel& getArmCortexA76Model() {
    static const EnergyCostModel model = arm_cortex_a76_model();
    return model;
}

/// Convenience: get ARM A55 model (efficiency core)
inline const EnergyCostModel& getArmCortexA55Model() {
    static const EnergyCostModel model = arm_cortex_a55_model();
    return model;
}

}}} // namespace sw::universal::energy
