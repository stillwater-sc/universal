#pragma once
// energy_model.hpp: base interface for energy cost models
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Energy cost models provide per-operation and per-memory-access energy
// estimates in picojoules (pJ). These models enable energy-aware algorithm
// design by quantifying the energy cost of different precision choices.
//
// Data sources:
// - Horowitz, M. (2014). "Computing's Energy Problem" ISSCC.
// - ITRS International Technology Roadmap for Semiconductors
// - Architecture-specific measurements and specifications
//
// Note: Energy values are approximate and vary with:
// - Process technology (nm)
// - Operating voltage
// - Clock frequency
// - Temperature
// - Specific microarchitecture

#include <cstdint>
#include <string>

namespace sw { namespace universal { namespace energy {

/// Bit-width categories for energy lookup
enum class BitWidth : uint8_t {
    bits_8  = 8,
    bits_16 = 16,
    bits_32 = 32,
    bits_64 = 64
};

/// Memory hierarchy levels
enum class MemoryLevel : uint8_t {
    Register,
    L1_Cache,
    L2_Cache,
    L3_Cache,
    DRAM
};

/// Operation types for energy lookup
enum class Operation : uint8_t {
    IntegerAdd,
    IntegerSubtract,
    IntegerMultiply,
    IntegerDivide,
    FloatAdd,
    FloatSubtract,
    FloatMultiply,
    FloatDivide,
    FloatFMA,
    FloatSqrt,
    Comparison,
    BitwiseLogic,
    Shift
};

/// Energy cost model interface
/// All energy values are in picojoules (pJ)
struct EnergyCostModel {
    std::string name;
    std::string description;
    unsigned process_nm;  // Process technology in nanometers

    // Per-operation energy costs by bit-width (in picojoules)
    struct OperationCosts {
        double int_add[4];      // 8, 16, 32, 64 bit
        double int_sub[4];
        double int_mul[4];
        double int_div[4];
        double fp_add[4];       // 8 (not common), 16, 32, 64 bit
        double fp_sub[4];
        double fp_mul[4];
        double fp_div[4];
        double fp_fma[4];
        double fp_sqrt[4];
        double compare[4];
        double logic[4];        // AND, OR, XOR, NOT
        double shift[4];
    } ops;

    // Memory access energy costs (in picojoules per access)
    struct MemoryCosts {
        double reg_read;
        double reg_write;
        double l1_read;
        double l1_write;
        double l2_read;
        double l2_write;
        double l3_read;
        double l3_write;
        double dram_read;
        double dram_write;
    } mem;

    // Data movement costs (pJ per bit)
    struct DataMovementCosts {
        double on_chip_per_bit;
        double off_chip_per_bit;
    } data_movement;

    /// Get operation energy for given operation and bit-width
    constexpr double operationEnergy(Operation op, BitWidth width) const noexcept {
        int idx = (width == BitWidth::bits_8) ? 0 :
                  (width == BitWidth::bits_16) ? 1 :
                  (width == BitWidth::bits_32) ? 2 : 3;

        switch (op) {
            case Operation::IntegerAdd:      return ops.int_add[idx];
            case Operation::IntegerSubtract: return ops.int_sub[idx];
            case Operation::IntegerMultiply: return ops.int_mul[idx];
            case Operation::IntegerDivide:   return ops.int_div[idx];
            case Operation::FloatAdd:        return ops.fp_add[idx];
            case Operation::FloatSubtract:   return ops.fp_sub[idx];
            case Operation::FloatMultiply:   return ops.fp_mul[idx];
            case Operation::FloatDivide:     return ops.fp_div[idx];
            case Operation::FloatFMA:        return ops.fp_fma[idx];
            case Operation::FloatSqrt:       return ops.fp_sqrt[idx];
            case Operation::Comparison:      return ops.compare[idx];
            case Operation::BitwiseLogic:    return ops.logic[idx];
            case Operation::Shift:           return ops.shift[idx];
            default:                         return 0.0;
        }
    }

    /// Get memory access energy for given level
    constexpr double memoryReadEnergy(MemoryLevel level) const noexcept {
        switch (level) {
            case MemoryLevel::Register: return mem.reg_read;
            case MemoryLevel::L1_Cache: return mem.l1_read;
            case MemoryLevel::L2_Cache: return mem.l2_read;
            case MemoryLevel::L3_Cache: return mem.l3_read;
            case MemoryLevel::DRAM:     return mem.dram_read;
            default:                    return 0.0;
        }
    }

    constexpr double memoryWriteEnergy(MemoryLevel level) const noexcept {
        switch (level) {
            case MemoryLevel::Register: return mem.reg_write;
            case MemoryLevel::L1_Cache: return mem.l1_write;
            case MemoryLevel::L2_Cache: return mem.l2_write;
            case MemoryLevel::L3_Cache: return mem.l3_write;
            case MemoryLevel::DRAM:     return mem.dram_write;
            default:                    return 0.0;
        }
    }

    /// Calculate total energy for N operations at given bit-width
    constexpr double totalOperationEnergy(Operation op, BitWidth width, uint64_t count) const noexcept {
        return operationEnergy(op, width) * static_cast<double>(count);
    }

    /// Calculate memory transfer energy for N bytes at given level
    constexpr double memoryTransferEnergy(MemoryLevel level, uint64_t bytes, bool isWrite) const noexcept {
        // Approximate: energy per access * (bytes / typical cache line size)
        // This is a simplification; actual energy depends on access patterns
        double energy_per_access = isWrite ? memoryWriteEnergy(level) : memoryReadEnergy(level);
        // Assume 64-byte cache lines for L1/L2/L3, 8-byte accesses for registers/DRAM
        uint64_t access_size = (level == MemoryLevel::Register || level == MemoryLevel::DRAM) ? 8 : 64;
        uint64_t accesses = (bytes + access_size - 1) / access_size;
        return energy_per_access * static_cast<double>(accesses);
    }
};

}}} // namespace sw::universal::energy
