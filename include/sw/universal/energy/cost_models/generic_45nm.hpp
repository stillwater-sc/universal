#pragma once
// generic_45nm.hpp: generic energy cost model based on 45nm CMOS
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Reference: Horowitz, M. (2014). "Computing's Energy Problem (and what
// we can do about it)." ISSCC 2014.
//
// These are widely-cited baseline values for 45nm CMOS technology.
// Energy scales approximately with voltage^2 and inversely with process
// node (smaller nodes are more efficient per operation but may have
// higher leakage).
//
// All values in picojoules (pJ).

#include "energy_model.hpp"

namespace sw { namespace universal { namespace energy {

/// Generic 45nm CMOS energy model
/// Based on Horowitz ISSCC 2014 data
inline constexpr EnergyCostModel generic_45nm_model() {
    EnergyCostModel model;

    model.name = "Generic 45nm CMOS";
    model.description = "Baseline energy model from Horowitz ISSCC 2014";
    model.process_nm = 45;

    // Integer operations (pJ)
    // From Horowitz: 8-bit add ~0.03 pJ, 32-bit add ~0.1 pJ
    // Multiply energy scales roughly as n^1.5 to n^2
    model.ops.int_add[0] = 0.03;   // 8-bit
    model.ops.int_add[1] = 0.05;   // 16-bit
    model.ops.int_add[2] = 0.1;    // 32-bit
    model.ops.int_add[3] = 0.2;    // 64-bit

    model.ops.int_sub[0] = 0.03;   // Same as add (subtraction via complement)
    model.ops.int_sub[1] = 0.05;
    model.ops.int_sub[2] = 0.1;
    model.ops.int_sub[3] = 0.2;

    // From Horowitz: 8-bit mul ~0.2 pJ, 32-bit mul ~3.1 pJ
    model.ops.int_mul[0] = 0.2;    // 8-bit
    model.ops.int_mul[1] = 1.0;    // 16-bit
    model.ops.int_mul[2] = 3.1;    // 32-bit
    model.ops.int_mul[3] = 12.0;   // 64-bit (estimated)

    // Division is significantly more expensive (iterative)
    model.ops.int_div[0] = 1.0;    // 8-bit
    model.ops.int_div[1] = 4.0;    // 16-bit
    model.ops.int_div[2] = 15.0;   // 32-bit
    model.ops.int_div[3] = 60.0;   // 64-bit (estimated)

    // Floating-point operations (pJ)
    // From Horowitz: 16-bit FP add ~0.4 pJ, 32-bit FP add ~0.9 pJ
    model.ops.fp_add[0] = 0.2;     // 8-bit (rare, estimated)
    model.ops.fp_add[1] = 0.4;     // 16-bit (FP16/half)
    model.ops.fp_add[2] = 0.9;     // 32-bit (float)
    model.ops.fp_add[3] = 1.8;     // 64-bit (double)

    model.ops.fp_sub[0] = 0.2;     // Same as add
    model.ops.fp_sub[1] = 0.4;
    model.ops.fp_sub[2] = 0.9;
    model.ops.fp_sub[3] = 1.8;

    // From Horowitz: 16-bit FP mul ~1.1 pJ, 32-bit FP mul ~3.7 pJ
    model.ops.fp_mul[0] = 0.5;     // 8-bit (rare, estimated)
    model.ops.fp_mul[1] = 1.1;     // 16-bit
    model.ops.fp_mul[2] = 3.7;     // 32-bit
    model.ops.fp_mul[3] = 15.0;    // 64-bit (estimated)

    // FP division is expensive (iterative or table-based)
    model.ops.fp_div[0] = 2.0;     // 8-bit (estimated)
    model.ops.fp_div[1] = 5.0;     // 16-bit
    model.ops.fp_div[2] = 20.0;    // 32-bit
    model.ops.fp_div[3] = 80.0;    // 64-bit (estimated)

    // FMA is slightly less than add + mul due to shared exponent handling
    model.ops.fp_fma[0] = 0.6;     // 8-bit
    model.ops.fp_fma[1] = 1.4;     // 16-bit
    model.ops.fp_fma[2] = 4.5;     // 32-bit
    model.ops.fp_fma[3] = 16.0;    // 64-bit

    // Square root (iterative, expensive)
    model.ops.fp_sqrt[0] = 3.0;    // 8-bit
    model.ops.fp_sqrt[1] = 10.0;   // 16-bit
    model.ops.fp_sqrt[2] = 40.0;   // 32-bit
    model.ops.fp_sqrt[3] = 150.0;  // 64-bit

    // Comparison (cheap, similar to subtraction)
    model.ops.compare[0] = 0.03;
    model.ops.compare[1] = 0.05;
    model.ops.compare[2] = 0.1;
    model.ops.compare[3] = 0.2;

    // Bitwise logic (very cheap)
    model.ops.logic[0] = 0.01;
    model.ops.logic[1] = 0.02;
    model.ops.logic[2] = 0.04;
    model.ops.logic[3] = 0.08;

    // Shift operations (cheap, barrel shifter)
    model.ops.shift[0] = 0.02;
    model.ops.shift[1] = 0.03;
    model.ops.shift[2] = 0.06;
    model.ops.shift[3] = 0.12;

    // Memory access costs (pJ per access)
    // From Horowitz: SRAM (8KB) read ~10 pJ, DRAM read ~1.3-2.6 nJ (1300-2600 pJ)
    model.mem.reg_read  = 1.0;     // Register file access
    model.mem.reg_write = 1.0;

    model.mem.l1_read   = 10.0;    // ~32KB L1 cache
    model.mem.l1_write  = 10.0;

    model.mem.l2_read   = 50.0;    // ~256KB L2 cache
    model.mem.l2_write  = 50.0;

    model.mem.l3_read   = 200.0;   // ~8MB L3 cache (shared)
    model.mem.l3_write  = 200.0;

    model.mem.dram_read  = 1300.0; // Off-chip DRAM
    model.mem.dram_write = 1300.0;

    // Data movement costs (pJ per bit)
    model.data_movement.on_chip_per_bit  = 0.1;   // On-chip wire
    model.data_movement.off_chip_per_bit = 10.0;  // Off-chip I/O

    return model;
}

/// Convenience: get the default generic model
inline const EnergyCostModel& getGenericModel() {
    static const EnergyCostModel model = generic_45nm_model();
    return model;
}

}}} // namespace sw::universal::energy
