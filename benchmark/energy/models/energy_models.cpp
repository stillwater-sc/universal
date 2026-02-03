// energy_models.cpp: test and demonstration of energy cost models
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <iostream>
#include <iomanip>
#include <string>

#include <universal/energy/energy.hpp>

using namespace sw::universal::energy;

void printEnergyTable(const EnergyCostModel& model) {
    std::cout << "\n========================================\n";
    std::cout << "Energy Model: " << model.name << "\n";
    std::cout << "Process: " << model.process_nm << "nm\n";
    std::cout << "========================================\n\n";

    std::cout << std::fixed << std::setprecision(3);

    // Operation energy table
    std::cout << "Operation Energy (picojoules):\n";
    std::cout << std::setw(20) << "Operation"
              << std::setw(12) << "8-bit"
              << std::setw(12) << "16-bit"
              << std::setw(12) << "32-bit"
              << std::setw(12) << "64-bit"
              << std::setw(12) << "32/8 ratio" << "\n";
    std::cout << std::string(80, '-') << "\n";

    auto printRow = [&](const char* name, const double* vals) {
        std::cout << std::setw(20) << name;
        for (int i = 0; i < 4; ++i) {
            std::cout << std::setw(12) << vals[i];
        }
        double ratio = (vals[0] > 0) ? vals[2] / vals[0] : 0;
        std::cout << std::setw(12) << ratio << "x\n";
    };

    printRow("Integer Add", model.ops.int_add);
    printRow("Integer Mul", model.ops.int_mul);
    printRow("Integer Div", model.ops.int_div);
    printRow("Float Add", model.ops.fp_add);
    printRow("Float Mul", model.ops.fp_mul);
    printRow("Float FMA", model.ops.fp_fma);
    printRow("Float Div", model.ops.fp_div);
    printRow("Float Sqrt", model.ops.fp_sqrt);

    // Memory energy table
    std::cout << "\nMemory Access Energy (picojoules per access):\n";
    std::cout << std::setw(20) << "Level"
              << std::setw(12) << "Read"
              << std::setw(12) << "Write"
              << std::setw(15) << "vs Register" << "\n";
    std::cout << std::string(60, '-') << "\n";

    auto printMemRow = [&](const char* name, double read, double write, double reg) {
        std::cout << std::setw(20) << name
                  << std::setw(12) << read
                  << std::setw(12) << write
                  << std::setw(12) << (reg > 0 ? read / reg : 0) << "x\n";
    };

    double reg = model.mem.reg_read;
    printMemRow("Register", model.mem.reg_read, model.mem.reg_write, reg);
    printMemRow("L1 Cache", model.mem.l1_read, model.mem.l1_write, reg);
    printMemRow("L2 Cache", model.mem.l2_read, model.mem.l2_write, reg);
    printMemRow("L3 Cache", model.mem.l3_read, model.mem.l3_write, reg);
    printMemRow("DRAM", model.mem.dram_read, model.mem.dram_write, reg);
}

void demonstrateMixedPrecisionSavings() {
    std::cout << "\n\n========================================\n";
    std::cout << "Mixed-Precision Energy Savings Analysis\n";
    std::cout << "========================================\n\n";

    const auto& model = getDefaultModel();
    std::cout << "Using model: " << model.name << "\n\n";

    // Scenario: 1 million FMA operations
    constexpr uint64_t N = 1000000;

    std::cout << "Scenario: " << N << " FMA operations\n\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::setw(15) << "Precision"
              << std::setw(15) << "Energy (uJ)"
              << std::setw(15) << "Savings vs 32"
              << std::setw(15) << "Savings vs 64" << "\n";
    std::cout << std::string(60, '-') << "\n";

    double e8  = model.totalOperationEnergy(Operation::FloatFMA, BitWidth::bits_8, N) / 1e6;
    double e16 = model.totalOperationEnergy(Operation::FloatFMA, BitWidth::bits_16, N) / 1e6;
    double e32 = model.totalOperationEnergy(Operation::FloatFMA, BitWidth::bits_32, N) / 1e6;
    double e64 = model.totalOperationEnergy(Operation::FloatFMA, BitWidth::bits_64, N) / 1e6;

    auto printSavings = [&](const char* name, double energy) {
        std::cout << std::setw(15) << name
                  << std::setw(15) << energy
                  << std::setw(14) << (e32 / energy) << "x"
                  << std::setw(14) << (e64 / energy) << "x\n";
    };

    printSavings("8-bit", e8);
    printSavings("16-bit", e16);
    printSavings("32-bit", e32);
    printSavings("64-bit", e64);

    // Memory-dominated scenario
    std::cout << "\n\nScenario: Matrix multiply 1000x1000 (memory-bound)\n";
    std::cout << "Assuming all data from L2 cache\n\n";

    constexpr uint64_t matrix_elements = 1000 * 1000;
    constexpr uint64_t matrix_ops = 1000ULL * 1000 * 1000; // N^3 for matmul

    std::cout << std::setw(15) << "Precision"
              << std::setw(15) << "Compute (uJ)"
              << std::setw(15) << "Memory (uJ)"
              << std::setw(15) << "Total (uJ)" << "\n";
    std::cout << std::string(60, '-') << "\n";

    auto analyzeMatmul = [&](const char* name, BitWidth width) {
        int bytes = (width == BitWidth::bits_8) ? 1 :
                    (width == BitWidth::bits_16) ? 2 :
                    (width == BitWidth::bits_32) ? 4 : 8;

        double compute = model.totalOperationEnergy(Operation::FloatFMA, width, matrix_ops) / 1e6;
        // 3 matrices (A, B, C) read from L2
        double memory = model.memoryTransferEnergy(MemoryLevel::L2_Cache,
                                                    3 * matrix_elements * bytes, false) / 1e6;

        std::cout << std::setw(15) << name
                  << std::setw(15) << compute
                  << std::setw(15) << memory
                  << std::setw(15) << (compute + memory) << "\n";
    };

    analyzeMatmul("8-bit", BitWidth::bits_8);
    analyzeMatmul("16-bit", BitWidth::bits_16);
    analyzeMatmul("32-bit", BitWidth::bits_32);
    analyzeMatmul("64-bit", BitWidth::bits_64);
}

void demonstrateEnergyEstimator() {
    std::cout << "\n\n========================================\n";
    std::cout << "EnergyEstimator Class Demo\n";
    std::cout << "========================================\n\n";

    const auto& model = getIntelSkylakeModel();
    EnergyEstimator estimator(model);

    std::cout << "Simulating a simple dot product of 1000 elements (FP32):\n";
    std::cout << "  - 1000 loads from L1\n";
    std::cout << "  - 1000 FMA operations\n";
    std::cout << "  - 1 store to L1\n\n";

    estimator.addMemoryReads(MemoryLevel::L1_Cache, 1000 * 4);  // 1000 floats = 4KB
    estimator.addOperations(Operation::FloatFMA, BitWidth::bits_32, 1000);
    estimator.addMemoryWrites(MemoryLevel::L1_Cache, 4);  // 1 float result

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total energy: " << estimator.totalEnergyPJ() << " pJ\n";
    std::cout << "            = " << estimator.totalEnergyNJ() << " nJ\n";
    std::cout << "            = " << estimator.totalEnergyUJ() << " uJ\n";

    // Compare with FP16
    estimator.reset();
    estimator.addMemoryReads(MemoryLevel::L1_Cache, 1000 * 2);  // 1000 half = 2KB
    estimator.addOperations(Operation::FloatFMA, BitWidth::bits_16, 1000);
    estimator.addMemoryWrites(MemoryLevel::L1_Cache, 2);

    std::cout << "\nSame operation with FP16:\n";
    std::cout << "Total energy: " << estimator.totalEnergyPJ() << " pJ\n";
    std::cout << "            = " << estimator.totalEnergyNJ() << " nJ\n";
}

void compareArchitectures() {
    std::cout << "\n\n========================================\n";
    std::cout << "Architecture Comparison\n";
    std::cout << "========================================\n\n";

    std::cout << "32-bit Float FMA energy (pJ):\n";
    std::cout << std::setw(25) << "Architecture" << std::setw(15) << "Energy (pJ)" << "\n";
    std::cout << std::string(40, '-') << "\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << std::setw(25) << "Generic (45nm)"
              << std::setw(15) << getGenericModel().operationEnergy(Operation::FloatFMA, BitWidth::bits_32) << "\n";
    std::cout << std::setw(25) << "Intel Skylake (14nm)"
              << std::setw(15) << getIntelSkylakeModel().operationEnergy(Operation::FloatFMA, BitWidth::bits_32) << "\n";
    std::cout << std::setw(25) << "ARM Cortex-A76 (7nm)"
              << std::setw(15) << getArmCortexA76Model().operationEnergy(Operation::FloatFMA, BitWidth::bits_32) << "\n";
    std::cout << std::setw(25) << "ARM Cortex-A55 (7nm)"
              << std::setw(15) << getArmCortexA55Model().operationEnergy(Operation::FloatFMA, BitWidth::bits_32) << "\n";

    std::cout << "\nDRAM access energy (pJ):\n";
    std::cout << std::setw(25) << "Architecture" << std::setw(15) << "Energy (pJ)" << "\n";
    std::cout << std::string(40, '-') << "\n";

    std::cout << std::setw(25) << "Generic (45nm)"
              << std::setw(15) << getGenericModel().memoryReadEnergy(MemoryLevel::DRAM) << "\n";
    std::cout << std::setw(25) << "Intel Skylake (14nm)"
              << std::setw(15) << getIntelSkylakeModel().memoryReadEnergy(MemoryLevel::DRAM) << "\n";
    std::cout << std::setw(25) << "ARM Cortex-A76 (7nm)"
              << std::setw(15) << getArmCortexA76Model().memoryReadEnergy(MemoryLevel::DRAM) << "\n";
}

int main()
try {
    std::cout << "Universal Numbers Library: Energy Cost Models\n";
    std::cout << "=============================================\n";

    // Print detailed tables for each model
    printEnergyTable(getGenericModel());
    printEnergyTable(getIntelSkylakeModel());
    printEnergyTable(getArmCortexA76Model());
    printEnergyTable(getArmCortexA55Model());

    // Demonstrate mixed-precision savings
    demonstrateMixedPrecisionSavings();

    // Demonstrate EnergyEstimator
    demonstrateEnergyEstimator();

    // Compare architectures
    compareArchitectures();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. 8-bit operations use ~5-10x less energy than 32-bit\n";
    std::cout << "2. Memory access (especially DRAM) dominates compute energy\n";
    std::cout << "3. Reducing precision saves both compute AND memory energy\n";
    std::cout << "4. ARM efficiency cores (A55) use ~3x less energy than performance cores\n";

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
