#pragma once
// energy.hpp: main include for energy cost modeling
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Energy cost modeling enables energy-aware mixed-precision algorithm design
// by providing per-operation and per-memory-access energy estimates.
//
// Usage:
//   #include <universal/energy/energy.hpp>
//
//   using namespace sw::universal::energy;
//
//   // Get a specific model
//   const auto& model = getIntelSkylakeModel();
//
//   // Or use auto-detection
//   const auto& model = getDefaultModel();
//
//   // Query operation energy
//   double mul32_energy = model.operationEnergy(Operation::FloatMultiply, BitWidth::bits_32);
//
//   // Calculate total energy for N operations
//   double total = model.totalOperationEnergy(Operation::FloatFMA, BitWidth::bits_16, 1000000);

#include "cost_models/energy_model.hpp"
#include "cost_models/generic_45nm.hpp"
#include "cost_models/intel_skylake.hpp"
#include "cost_models/arm_cortex_a.hpp"

namespace sw { namespace universal { namespace energy {

/// Architecture enumeration for model selection
enum class Architecture {
    Generic,        // Generic 45nm baseline
    IntelSkylake,   // Intel Skylake (14nm desktop/server)
    ArmCortexA76,   // ARM Cortex-A76/A78 (7nm mobile high-perf)
    ArmCortexA55    // ARM Cortex-A55 (7nm mobile efficiency)
};

/// Get energy model for specified architecture
inline const EnergyCostModel& getModel(Architecture arch) {
    switch (arch) {
        case Architecture::IntelSkylake: return getIntelSkylakeModel();
        case Architecture::ArmCortexA76: return getArmCortexA76Model();
        case Architecture::ArmCortexA55: return getArmCortexA55Model();
        case Architecture::Generic:
        default:                         return getGenericModel();
    }
}

/// Auto-detect architecture and return appropriate model
/// Falls back to generic model if detection fails
inline const EnergyCostModel& getDefaultModel() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    // x86/x64: assume Skylake-class (most common modern Intel/AMD)
    return getIntelSkylakeModel();
#elif defined(__aarch64__) || defined(_M_ARM64)
    // ARM64: assume Cortex-A76 class (common in modern mobile/server)
    return getArmCortexA76Model();
#elif defined(__arm__) || defined(_M_ARM)
    // ARM32: assume efficiency core
    return getArmCortexA55Model();
#else
    // Unknown: use generic model
    return getGenericModel();
#endif
}

/// Energy estimator helper class
/// Accumulates energy for a sequence of operations
class EnergyEstimator {
public:
    explicit EnergyEstimator(const EnergyCostModel& model = getDefaultModel())
        : model_(model), total_energy_pj_(0.0) {}

    /// Add energy for N operations of given type and width
    void addOperations(Operation op, BitWidth width, uint64_t count) {
        total_energy_pj_ += model_.totalOperationEnergy(op, width, count);
    }

    /// Add energy for memory reads at given level
    void addMemoryReads(MemoryLevel level, uint64_t bytes) {
        total_energy_pj_ += model_.memoryTransferEnergy(level, bytes, false);
    }

    /// Add energy for memory writes at given level
    void addMemoryWrites(MemoryLevel level, uint64_t bytes) {
        total_energy_pj_ += model_.memoryTransferEnergy(level, bytes, true);
    }

    /// Get total accumulated energy in picojoules
    double totalEnergyPJ() const { return total_energy_pj_; }

    /// Get total accumulated energy in nanojoules
    double totalEnergyNJ() const { return total_energy_pj_ / 1000.0; }

    /// Get total accumulated energy in microjoules
    double totalEnergyUJ() const { return total_energy_pj_ / 1000000.0; }

    /// Get total accumulated energy in millijoules
    double totalEnergyMJ() const { return total_energy_pj_ / 1000000000.0; }

    /// Get total accumulated energy in joules
    double totalEnergyJ() const { return total_energy_pj_ / 1000000000000.0; }

    /// Reset accumulator
    void reset() { total_energy_pj_ = 0.0; }

    /// Get the model being used
    const EnergyCostModel& model() const { return model_; }

private:
    const EnergyCostModel& model_;
    double total_energy_pj_;
};

/// Calculate energy ratio between two bit-widths for an operation
/// Returns how much more energy the wider operation uses
inline double energyRatio(const EnergyCostModel& model, Operation op,
                          BitWidth narrow, BitWidth wide) {
    double narrow_e = model.operationEnergy(op, narrow);
    double wide_e = model.operationEnergy(op, wide);
    return (narrow_e > 0) ? (wide_e / narrow_e) : 0.0;
}

/// Calculate potential energy savings from precision reduction
/// Returns energy saved per operation in picojoules
inline double energySavings(const EnergyCostModel& model, Operation op,
                            BitWidth from, BitWidth to) {
    return model.operationEnergy(op, from) - model.operationEnergy(op, to);
}

/// Print energy model summary to stream
inline void printModelSummary(std::ostream& os, const EnergyCostModel& model) {
    os << "Energy Model: " << model.name << "\n";
    os << "Description: " << model.description << "\n";
    os << "Process: " << model.process_nm << "nm\n\n";

    os << "Operation Energy (pJ):\n";
    os << "                    8-bit    16-bit    32-bit    64-bit\n";
    os << "  Integer Add:    " << model.ops.int_add[0] << "      "
       << model.ops.int_add[1] << "      " << model.ops.int_add[2] << "      "
       << model.ops.int_add[3] << "\n";
    os << "  Integer Mul:    " << model.ops.int_mul[0] << "      "
       << model.ops.int_mul[1] << "      " << model.ops.int_mul[2] << "      "
       << model.ops.int_mul[3] << "\n";
    os << "  Float Add:      " << model.ops.fp_add[0] << "      "
       << model.ops.fp_add[1] << "      " << model.ops.fp_add[2] << "      "
       << model.ops.fp_add[3] << "\n";
    os << "  Float Mul:      " << model.ops.fp_mul[0] << "      "
       << model.ops.fp_mul[1] << "      " << model.ops.fp_mul[2] << "      "
       << model.ops.fp_mul[3] << "\n";
    os << "  Float FMA:      " << model.ops.fp_fma[0] << "      "
       << model.ops.fp_fma[1] << "      " << model.ops.fp_fma[2] << "      "
       << model.ops.fp_fma[3] << "\n\n";

    os << "Memory Access Energy (pJ):\n";
    os << "  Register:  " << model.mem.reg_read << "\n";
    os << "  L1 Cache:  " << model.mem.l1_read << "\n";
    os << "  L2 Cache:  " << model.mem.l2_read << "\n";
    os << "  L3 Cache:  " << model.mem.l3_read << "\n";
    os << "  DRAM:      " << model.mem.dram_read << "\n";
}

}}} // namespace sw::universal::energy

// Include occurrence_energy after getDefaultModel() is declared
#include "occurrence_energy.hpp"
