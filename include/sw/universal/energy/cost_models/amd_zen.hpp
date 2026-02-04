#pragma once
// amd_zen.hpp: energy cost models for AMD Zen microarchitecture family
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// AMD Zen family energy models:
// - Zen 2 (7nm): Ryzen 3000, EPYC Rome
// - Zen 3 (7nm): Ryzen 5000, EPYC Milan
// - Zen 4 (5nm): Ryzen 7000, EPYC Genoa
//
// Estimated values based on:
// - AMD optimization guides and whitepapers
// - Published measurements and academic papers
// - Process node scaling from Intel Skylake (14nm)
//
// Note: AMD Zen uses a chiplet design (CCD + IOD) which affects
// memory access energy differently than monolithic designs.
//
// All values in picojoules (pJ).

#include "energy_model.hpp"

namespace sw { namespace universal { namespace energy {

/// AMD Zen 2 (7nm TSMC) energy model
/// Ryzen 3000 series, EPYC Rome
/// ~2x improvement from 14nm Skylake due to 7nm process
inline constexpr EnergyCostModel amd_zen2_model() {
    EnergyCostModel model;

    model.name = "AMD Zen 2 (7nm)";
    model.description = "Energy model for AMD Zen 2 (Ryzen 3000, EPYC Rome)";
    model.process_nm = 7;

    // Scaling: 7nm vs 14nm gives roughly 2x power improvement
    // Zen 2 has 2x 256-bit FMA units per core (like Skylake)

    // Integer operations (pJ)
    model.ops.int_add[0] = 0.005;   // 8-bit
    model.ops.int_add[1] = 0.009;   // 16-bit
    model.ops.int_add[2] = 0.017;   // 32-bit
    model.ops.int_add[3] = 0.033;   // 64-bit

    model.ops.int_sub[0] = 0.005;
    model.ops.int_sub[1] = 0.009;
    model.ops.int_sub[2] = 0.017;
    model.ops.int_sub[3] = 0.033;

    model.ops.int_mul[0] = 0.033;   // 8-bit
    model.ops.int_mul[1] = 0.17;    // 16-bit
    model.ops.int_mul[2] = 0.5;     // 32-bit
    model.ops.int_mul[3] = 2.0;     // 64-bit

    model.ops.int_div[0] = 0.17;    // 8-bit
    model.ops.int_div[1] = 0.66;    // 16-bit
    model.ops.int_div[2] = 2.5;     // 32-bit
    model.ops.int_div[3] = 10.0;    // 64-bit

    // Floating-point operations (pJ)
    // Zen 2 has excellent FP performance with 2x 256-bit FMA
    model.ops.fp_add[0] = 0.033;    // 8-bit (estimated, not native)
    model.ops.fp_add[1] = 0.066;    // 16-bit
    model.ops.fp_add[2] = 0.15;     // 32-bit
    model.ops.fp_add[3] = 0.3;      // 64-bit

    model.ops.fp_sub[0] = 0.033;
    model.ops.fp_sub[1] = 0.066;
    model.ops.fp_sub[2] = 0.15;
    model.ops.fp_sub[3] = 0.3;

    model.ops.fp_mul[0] = 0.085;    // 8-bit
    model.ops.fp_mul[1] = 0.19;     // 16-bit
    model.ops.fp_mul[2] = 0.6;      // 32-bit
    model.ops.fp_mul[3] = 2.5;      // 64-bit

    model.ops.fp_div[0] = 0.33;     // 8-bit
    model.ops.fp_div[1] = 0.85;     // 16-bit
    model.ops.fp_div[2] = 3.3;      // 32-bit
    model.ops.fp_div[3] = 13.0;     // 64-bit

    // FMA: highly efficient on Zen 2
    model.ops.fp_fma[0] = 0.1;      // 8-bit
    model.ops.fp_fma[1] = 0.24;     // 16-bit
    model.ops.fp_fma[2] = 0.75;     // 32-bit
    model.ops.fp_fma[3] = 2.7;      // 64-bit

    // Square root
    model.ops.fp_sqrt[0] = 0.5;     // 8-bit
    model.ops.fp_sqrt[1] = 1.65;    // 16-bit
    model.ops.fp_sqrt[2] = 6.5;     // 32-bit
    model.ops.fp_sqrt[3] = 25.0;    // 64-bit

    // Comparison
    model.ops.compare[0] = 0.005;
    model.ops.compare[1] = 0.009;
    model.ops.compare[2] = 0.017;
    model.ops.compare[3] = 0.033;

    // Bitwise logic
    model.ops.logic[0] = 0.0015;
    model.ops.logic[1] = 0.0035;
    model.ops.logic[2] = 0.007;
    model.ops.logic[3] = 0.013;

    // Shift operations
    model.ops.shift[0] = 0.0035;
    model.ops.shift[1] = 0.005;
    model.ops.shift[2] = 0.01;
    model.ops.shift[3] = 0.02;

    // Memory access costs (pJ per access)
    // Zen 2: 32KB L1D, 512KB L2 per core, 32MB L3 per CCD (shared)
    model.mem.reg_read  = 0.17;
    model.mem.reg_write = 0.17;

    model.mem.l1_read   = 1.65;     // 32KB L1
    model.mem.l1_write  = 1.65;

    model.mem.l2_read   = 8.5;      // 512KB L2 (larger than Skylake)
    model.mem.l2_write  = 8.5;

    model.mem.l3_read   = 33.0;     // 32MB L3 per CCD
    model.mem.l3_write  = 33.0;

    // DDR4 DRAM (chiplet design adds some latency but similar energy)
    model.mem.dram_read  = 500.0;
    model.mem.dram_write = 500.0;

    // Data movement costs (pJ per bit)
    model.data_movement.on_chip_per_bit  = 0.017;
    model.data_movement.off_chip_per_bit = 3.5;

    return model;
}

/// AMD Zen 3 (7nm+ TSMC) energy model
/// Ryzen 5000 series, EPYC Milan
/// Unified 8-core CCX with shared L3, improved IPC
inline constexpr EnergyCostModel amd_zen3_model() {
    EnergyCostModel model;

    model.name = "AMD Zen 3 (7nm+)";
    model.description = "Energy model for AMD Zen 3 (Ryzen 5000, EPYC Milan)";
    model.process_nm = 7;

    // Zen 3 is ~10% more efficient than Zen 2 with unified CCX design

    // Integer operations (pJ)
    model.ops.int_add[0] = 0.0045;
    model.ops.int_add[1] = 0.008;
    model.ops.int_add[2] = 0.015;
    model.ops.int_add[3] = 0.03;

    model.ops.int_sub[0] = 0.0045;
    model.ops.int_sub[1] = 0.008;
    model.ops.int_sub[2] = 0.015;
    model.ops.int_sub[3] = 0.03;

    model.ops.int_mul[0] = 0.03;
    model.ops.int_mul[1] = 0.15;
    model.ops.int_mul[2] = 0.45;
    model.ops.int_mul[3] = 1.8;

    model.ops.int_div[0] = 0.15;
    model.ops.int_div[1] = 0.6;
    model.ops.int_div[2] = 2.3;
    model.ops.int_div[3] = 9.0;

    // Floating-point operations (pJ)
    model.ops.fp_add[0] = 0.03;
    model.ops.fp_add[1] = 0.06;
    model.ops.fp_add[2] = 0.14;
    model.ops.fp_add[3] = 0.27;

    model.ops.fp_sub[0] = 0.03;
    model.ops.fp_sub[1] = 0.06;
    model.ops.fp_sub[2] = 0.14;
    model.ops.fp_sub[3] = 0.27;

    model.ops.fp_mul[0] = 0.075;
    model.ops.fp_mul[1] = 0.17;
    model.ops.fp_mul[2] = 0.54;
    model.ops.fp_mul[3] = 2.3;

    model.ops.fp_div[0] = 0.3;
    model.ops.fp_div[1] = 0.77;
    model.ops.fp_div[2] = 3.0;
    model.ops.fp_div[3] = 12.0;

    // FMA
    model.ops.fp_fma[0] = 0.09;
    model.ops.fp_fma[1] = 0.22;
    model.ops.fp_fma[2] = 0.68;
    model.ops.fp_fma[3] = 2.4;

    // Square root
    model.ops.fp_sqrt[0] = 0.45;
    model.ops.fp_sqrt[1] = 1.5;
    model.ops.fp_sqrt[2] = 5.9;
    model.ops.fp_sqrt[3] = 23.0;

    // Comparison
    model.ops.compare[0] = 0.0045;
    model.ops.compare[1] = 0.008;
    model.ops.compare[2] = 0.015;
    model.ops.compare[3] = 0.03;

    // Bitwise logic
    model.ops.logic[0] = 0.0014;
    model.ops.logic[1] = 0.0032;
    model.ops.logic[2] = 0.006;
    model.ops.logic[3] = 0.012;

    // Shift operations
    model.ops.shift[0] = 0.0032;
    model.ops.shift[1] = 0.0045;
    model.ops.shift[2] = 0.009;
    model.ops.shift[3] = 0.018;

    // Memory access costs (pJ per access)
    // Zen 3: 32KB L1D, 512KB L2, unified 32MB L3 per CCX
    model.mem.reg_read  = 0.15;
    model.mem.reg_write = 0.15;

    model.mem.l1_read   = 1.5;
    model.mem.l1_write  = 1.5;

    model.mem.l2_read   = 7.7;
    model.mem.l2_write  = 7.7;

    model.mem.l3_read   = 30.0;     // Unified L3 improves access
    model.mem.l3_write  = 30.0;

    // DDR4 DRAM
    model.mem.dram_read  = 480.0;
    model.mem.dram_write = 480.0;

    // Data movement costs (pJ per bit)
    model.data_movement.on_chip_per_bit  = 0.015;
    model.data_movement.off_chip_per_bit = 3.3;

    return model;
}

/// AMD Zen 4 (5nm TSMC) energy model
/// Ryzen 7000 series, EPYC Genoa
/// AVX-512 support, DDR5, improved efficiency
inline constexpr EnergyCostModel amd_zen4_model() {
    EnergyCostModel model;

    model.name = "AMD Zen 4 (5nm)";
    model.description = "Energy model for AMD Zen 4 (Ryzen 7000, EPYC Genoa)";
    model.process_nm = 5;

    // 5nm provides ~40% power reduction vs 7nm

    // Integer operations (pJ)
    model.ops.int_add[0] = 0.003;
    model.ops.int_add[1] = 0.005;
    model.ops.int_add[2] = 0.01;
    model.ops.int_add[3] = 0.02;

    model.ops.int_sub[0] = 0.003;
    model.ops.int_sub[1] = 0.005;
    model.ops.int_sub[2] = 0.01;
    model.ops.int_sub[3] = 0.02;

    model.ops.int_mul[0] = 0.02;
    model.ops.int_mul[1] = 0.1;
    model.ops.int_mul[2] = 0.3;
    model.ops.int_mul[3] = 1.2;

    model.ops.int_div[0] = 0.1;
    model.ops.int_div[1] = 0.4;
    model.ops.int_div[2] = 1.5;
    model.ops.int_div[3] = 6.0;

    // Floating-point operations (pJ)
    // Zen 4 has native AVX-512 with 2x 256-bit FMA (fused for 512-bit)
    model.ops.fp_add[0] = 0.02;
    model.ops.fp_add[1] = 0.04;
    model.ops.fp_add[2] = 0.09;
    model.ops.fp_add[3] = 0.18;

    model.ops.fp_sub[0] = 0.02;
    model.ops.fp_sub[1] = 0.04;
    model.ops.fp_sub[2] = 0.09;
    model.ops.fp_sub[3] = 0.18;

    model.ops.fp_mul[0] = 0.05;
    model.ops.fp_mul[1] = 0.11;
    model.ops.fp_mul[2] = 0.36;
    model.ops.fp_mul[3] = 1.5;

    model.ops.fp_div[0] = 0.2;
    model.ops.fp_div[1] = 0.5;
    model.ops.fp_div[2] = 2.0;
    model.ops.fp_div[3] = 8.0;

    // FMA: highly efficient
    model.ops.fp_fma[0] = 0.06;
    model.ops.fp_fma[1] = 0.14;
    model.ops.fp_fma[2] = 0.45;
    model.ops.fp_fma[3] = 1.6;

    // Square root
    model.ops.fp_sqrt[0] = 0.3;
    model.ops.fp_sqrt[1] = 1.0;
    model.ops.fp_sqrt[2] = 3.9;
    model.ops.fp_sqrt[3] = 15.0;

    // Comparison
    model.ops.compare[0] = 0.003;
    model.ops.compare[1] = 0.005;
    model.ops.compare[2] = 0.01;
    model.ops.compare[3] = 0.02;

    // Bitwise logic
    model.ops.logic[0] = 0.001;
    model.ops.logic[1] = 0.002;
    model.ops.logic[2] = 0.004;
    model.ops.logic[3] = 0.008;

    // Shift operations
    model.ops.shift[0] = 0.002;
    model.ops.shift[1] = 0.003;
    model.ops.shift[2] = 0.006;
    model.ops.shift[3] = 0.012;

    // Memory access costs (pJ per access)
    // Zen 4: 32KB L1D, 1MB L2 per core, 32MB L3 per CCD
    model.mem.reg_read  = 0.1;
    model.mem.reg_write = 0.1;

    model.mem.l1_read   = 1.0;
    model.mem.l1_write  = 1.0;

    model.mem.l2_read   = 5.0;      // Larger 1MB L2
    model.mem.l2_write  = 5.0;

    model.mem.l3_read   = 20.0;
    model.mem.l3_write  = 20.0;

    // DDR5 DRAM: higher bandwidth, slightly better energy efficiency
    model.mem.dram_read  = 420.0;
    model.mem.dram_write = 420.0;

    // Data movement costs (pJ per bit)
    model.data_movement.on_chip_per_bit  = 0.01;
    model.data_movement.off_chip_per_bit = 2.5;

    return model;
}

/// Convenience functions
inline const EnergyCostModel& getAmdZen2Model() {
    static const EnergyCostModel model = amd_zen2_model();
    return model;
}

inline const EnergyCostModel& getAmdZen3Model() {
    static const EnergyCostModel model = amd_zen3_model();
    return model;
}

inline const EnergyCostModel& getAmdZen4Model() {
    static const EnergyCostModel model = amd_zen4_model();
    return model;
}

}}} // namespace sw::universal::energy
