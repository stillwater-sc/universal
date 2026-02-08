#pragma once
// occurrence_energy.hpp: extend operation counting with energy estimation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// This utility combines operation counting (from occurrence.hpp) with
// energy cost models to estimate the energy consumption of computations.
//
// Usage:
//   #include <universal/energy/occurrence_energy.hpp>
//
//   using namespace sw::universal;
//   using namespace sw::universal::energy;
//
//   // Get operation counts from your number system
//   occurrence<MyType> ops = getOperationCounts();
//
//   // Calculate energy using a cost model
//   const auto& model = getDefaultModel();
//   double energy_pj = calculateEnergy(ops, model, BitWidth::bits_32);
//
//   // Or use the convenience wrapper
//   OccurrenceEnergy<MyType> energy_tracker(model, BitWidth::bits_32);
//   energy_tracker.setOccurrence(ops);
//   std::cout << energy_tracker.totalEnergyPJ() << " pJ\n";

#include <iostream>
#include <iomanip>
#include <cstdint>

#include "../utility/occurrence.hpp"
#include "energy.hpp"

namespace sw { namespace universal { namespace energy {

/// Calculate total energy from operation counts
/// @param ops Operation occurrence counts
/// @param model Energy cost model to use
/// @param width Bit-width of operations (determines energy per op)
/// @param mem_level Memory level for load/store operations
/// @return Total energy in picojoules
template<typename NumberSystem>
double calculateEnergy(const occurrence<NumberSystem>& ops,
                       const EnergyCostModel& model,
                       BitWidth width,
                       MemoryLevel mem_level = MemoryLevel::L1_Cache) {
    double total = 0.0;

    // Arithmetic operations
    total += model.totalOperationEnergy(Operation::FloatAdd, width, ops.add);
    total += model.totalOperationEnergy(Operation::FloatSubtract, width, ops.sub);
    total += model.totalOperationEnergy(Operation::FloatMultiply, width, ops.mul);
    total += model.totalOperationEnergy(Operation::FloatDivide, width, ops.div);
    total += model.totalOperationEnergy(Operation::FloatSqrt, width, ops.sqrt);

    // Remainder is typically implemented as div + mul + sub
    double rem_energy = model.operationEnergy(Operation::FloatDivide, width) +
                        model.operationEnergy(Operation::FloatMultiply, width) +
                        model.operationEnergy(Operation::FloatSubtract, width);
    total += rem_energy * static_cast<double>(ops.rem);

    // Memory operations (load/store)
    // Estimate bytes based on bit-width
    unsigned bytes_per_element = (width == BitWidth::bits_8) ? 1 :
                                 (width == BitWidth::bits_16) ? 2 :
                                 (width == BitWidth::bits_32) ? 4 : 8;

    total += model.memoryTransferEnergy(mem_level, ops.load * bytes_per_element, false);
    total += model.memoryTransferEnergy(mem_level, ops.store * bytes_per_element, true);

    return total;
}

/// Calculate energy breakdown by operation type
template<typename NumberSystem>
struct EnergyBreakdown {
    double add_energy;
    double sub_energy;
    double mul_energy;
    double div_energy;
    double rem_energy;
    double sqrt_energy;
    double load_energy;
    double store_energy;
    double total_energy;

    double computeEnergy() const {
        return add_energy + sub_energy + mul_energy + div_energy + rem_energy + sqrt_energy;
    }

    double memoryEnergy() const {
        return load_energy + store_energy;
    }
};

template<typename NumberSystem>
EnergyBreakdown<NumberSystem> calculateEnergyBreakdown(
        const occurrence<NumberSystem>& ops,
        const EnergyCostModel& model,
        BitWidth width,
        MemoryLevel mem_level = MemoryLevel::L1_Cache) {

    EnergyBreakdown<NumberSystem> breakdown;

    breakdown.add_energy = model.totalOperationEnergy(Operation::FloatAdd, width, ops.add);
    breakdown.sub_energy = model.totalOperationEnergy(Operation::FloatSubtract, width, ops.sub);
    breakdown.mul_energy = model.totalOperationEnergy(Operation::FloatMultiply, width, ops.mul);
    breakdown.div_energy = model.totalOperationEnergy(Operation::FloatDivide, width, ops.div);
    breakdown.sqrt_energy = model.totalOperationEnergy(Operation::FloatSqrt, width, ops.sqrt);

    double rem_per_op = model.operationEnergy(Operation::FloatDivide, width) +
                        model.operationEnergy(Operation::FloatMultiply, width) +
                        model.operationEnergy(Operation::FloatSubtract, width);
    breakdown.rem_energy = rem_per_op * static_cast<double>(ops.rem);

    unsigned bytes = (width == BitWidth::bits_8) ? 1 :
                     (width == BitWidth::bits_16) ? 2 :
                     (width == BitWidth::bits_32) ? 4 : 8;

    breakdown.load_energy = model.memoryTransferEnergy(mem_level, ops.load * bytes, false);
    breakdown.store_energy = model.memoryTransferEnergy(mem_level, ops.store * bytes, true);

    breakdown.total_energy = breakdown.computeEnergy() + breakdown.memoryEnergy();

    return breakdown;
}

/// Wrapper class combining occurrence counting with energy estimation
template<typename NumberSystem>
class OccurrenceEnergy {
public:
    OccurrenceEnergy(const EnergyCostModel& model = getDefaultModel(),
                     BitWidth width = BitWidth::bits_32,
                     MemoryLevel mem_level = MemoryLevel::L1_Cache)
        : model_(model), width_(width), mem_level_(mem_level), ops_() {}

    /// Set occurrence counts (copy from external source)
    void setOccurrence(const occurrence<NumberSystem>& ops) {
        ops_ = ops;
    }

    /// Get reference to internal occurrence (for direct manipulation)
    occurrence<NumberSystem>& ops() { return ops_; }
    const occurrence<NumberSystem>& ops() const { return ops_; }

    /// Reset all counts
    void reset() { ops_.reset(); }

    /// Calculate total energy in picojoules
    double totalEnergyPJ() const {
        return calculateEnergy(ops_, model_, width_, mem_level_);
    }

    /// Calculate total energy in nanojoules
    double totalEnergyNJ() const { return totalEnergyPJ() / 1000.0; }

    /// Calculate total energy in microjoules
    double totalEnergyUJ() const { return totalEnergyPJ() / 1000000.0; }

    /// Get detailed energy breakdown
    EnergyBreakdown<NumberSystem> breakdown() const {
        return calculateEnergyBreakdown(ops_, model_, width_, mem_level_);
    }

    /// Get the energy model
    const EnergyCostModel& model() const { return model_; }

    /// Get/set bit-width
    BitWidth width() const { return width_; }
    void setWidth(BitWidth w) { width_ = w; }

    /// Get/set memory level
    MemoryLevel memoryLevel() const { return mem_level_; }
    void setMemoryLevel(MemoryLevel level) { mem_level_ = level; }

    /// Report operation counts and energy
    void report(std::ostream& ostr) const {
        auto bd = breakdown();

        ostr << "Operation Counts and Energy Estimates\n";
        ostr << "Model: " << model_.name << " (" << model_.process_nm << "nm)\n";
        ostr << "Bit-width: " << static_cast<int>(width_) << "-bit\n";
        ostr << std::string(50, '-') << "\n";

        ostr << std::fixed << std::setprecision(2);
        ostr << std::setw(12) << "Operation"
             << std::setw(12) << "Count"
             << std::setw(15) << "Energy (pJ)" << "\n";
        ostr << std::string(40, '-') << "\n";

        ostr << std::setw(12) << "Load" << std::setw(12) << ops_.load
             << std::setw(15) << bd.load_energy << "\n";
        ostr << std::setw(12) << "Store" << std::setw(12) << ops_.store
             << std::setw(15) << bd.store_energy << "\n";
        ostr << std::setw(12) << "Add" << std::setw(12) << ops_.add
             << std::setw(15) << bd.add_energy << "\n";
        ostr << std::setw(12) << "Sub" << std::setw(12) << ops_.sub
             << std::setw(15) << bd.sub_energy << "\n";
        ostr << std::setw(12) << "Mul" << std::setw(12) << ops_.mul
             << std::setw(15) << bd.mul_energy << "\n";
        ostr << std::setw(12) << "Div" << std::setw(12) << ops_.div
             << std::setw(15) << bd.div_energy << "\n";
        ostr << std::setw(12) << "Rem" << std::setw(12) << ops_.rem
             << std::setw(15) << bd.rem_energy << "\n";
        ostr << std::setw(12) << "Sqrt" << std::setw(12) << ops_.sqrt
             << std::setw(15) << bd.sqrt_energy << "\n";

        ostr << std::string(40, '-') << "\n";
        ostr << std::setw(12) << "Compute" << std::setw(12) << ""
             << std::setw(15) << bd.computeEnergy() << "\n";
        ostr << std::setw(12) << "Memory" << std::setw(12) << ""
             << std::setw(15) << bd.memoryEnergy() << "\n";
        ostr << std::setw(12) << "TOTAL" << std::setw(12) << ""
             << std::setw(15) << bd.total_energy << " pJ\n";

        ostr << "\nEnergy in other units:\n";
        ostr << "  " << totalEnergyNJ() << " nJ\n";
        ostr << "  " << totalEnergyUJ() << " uJ\n";
    }

private:
    const EnergyCostModel& model_;
    BitWidth width_;
    MemoryLevel mem_level_;
    occurrence<NumberSystem> ops_;
};

/// Compare energy between two precision configurations
template<typename NumberSystem>
void compareEnergyByPrecision(const occurrence<NumberSystem>& ops,
                              const EnergyCostModel& model,
                              std::ostream& ostr = std::cout) {
    ostr << "Energy Comparison by Precision\n";
    ostr << "Model: " << model.name << "\n";
    ostr << std::string(60, '-') << "\n";

    ostr << std::fixed << std::setprecision(2);
    ostr << std::setw(12) << "Precision"
         << std::setw(15) << "Energy (pJ)"
         << std::setw(15) << "vs 32-bit"
         << std::setw(15) << "vs 64-bit" << "\n";
    ostr << std::string(60, '-') << "\n";

    double e8  = calculateEnergy(ops, model, BitWidth::bits_8);
    double e16 = calculateEnergy(ops, model, BitWidth::bits_16);
    double e32 = calculateEnergy(ops, model, BitWidth::bits_32);
    double e64 = calculateEnergy(ops, model, BitWidth::bits_64);

    auto printRow = [&](const char* name, double energy) {
        ostr << std::setw(12) << name
             << std::setw(15) << energy
             << std::setw(14) << (e32 / energy) << "x"
             << std::setw(14) << (e64 / energy) << "x\n";
    };

    printRow("8-bit", e8);
    printRow("16-bit", e16);
    printRow("32-bit", e32);
    printRow("64-bit", e64);
}

}}} // namespace sw::universal::energy
