// rapl_measurement.cpp: test and demonstration of RAPL energy measurement
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// This benchmark demonstrates hardware energy measurement using Intel RAPL
// (Running Average Power Limit) via the Linux powercap sysfs interface.
//
// Requirements:
//   - Linux with kernel >= 3.13 and powercap support
//   - Intel or AMD processor with RAPL support
//   - Read access to /sys/class/powercap/intel-rapl/
//
// On non-Linux platforms, this will compile but report RAPL as unavailable.

#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cmath>

#include <universal/energy/energy.hpp>

using namespace sw::universal::energy;

// Simple compute workload for energy measurement
void computeWorkload(size_t iterations) {
    volatile double sum = 0.0;
    for (size_t i = 0; i < iterations; ++i) {
        sum += std::sin(static_cast<double>(i) * 0.001);
        sum += std::cos(static_cast<double>(i) * 0.001);
    }
    // Use the result to prevent optimization
    if (sum == 0.0) std::cout << "";
}

// Memory-intensive workload
void memoryWorkload(size_t size) {
    std::vector<double> data(size);

    // Write
    for (size_t i = 0; i < size; ++i) {
        data[i] = static_cast<double>(i) * 1.001;
    }

    // Read and accumulate
    volatile double sum = 0.0;
    for (size_t i = 0; i < size; ++i) {
        sum += data[i];
    }
    if (sum == 0.0) std::cout << "";
}

void demonstrateRaplAvailability() {
    std::cout << "========================================\n";
    std::cout << "RAPL Availability Check\n";
    std::cout << "========================================\n\n";

    if (RaplReader::isAvailable()) {
        std::cout << "RAPL is AVAILABLE on this system\n\n";

        RaplReader rapl;
        std::cout << rapl.systemInfo() << "\n";

        std::cout << "Domain availability:\n";
        std::cout << "  Package: " << (rapl.hasPackage() ? "YES" : "NO") << "\n";
        std::cout << "  Cores (PP0): " << (rapl.hasCores() ? "YES" : "NO") << "\n";
        std::cout << "  Uncore (PP1): " << (rapl.hasUncore() ? "YES" : "NO") << "\n";
        std::cout << "  DRAM: " << (rapl.hasDram() ? "YES" : "NO") << "\n";
    } else {
        std::cout << "RAPL is NOT AVAILABLE on this system\n";
        std::cout << "Possible reasons:\n";
        std::cout << "  - Not running on Linux\n";
        std::cout << "  - Linux kernel < 3.13 or powercap not enabled\n";
        std::cout << "  - CPU does not support RAPL (older Intel, non-Intel)\n";
        std::cout << "  - No read access to /sys/class/powercap/intel-rapl/\n";
        std::cout << "    (try: sudo chmod -R a+r /sys/class/powercap/intel-rapl/)\n";
    }
}

void demonstrateBasicMeasurement() {
    std::cout << "\n========================================\n";
    std::cout << "Basic RAPL Measurement\n";
    std::cout << "========================================\n\n";

    if (!RaplReader::isAvailable()) {
        std::cout << "RAPL not available, skipping measurement demo\n";
        return;
    }

    RaplReader rapl;

    // Measure compute workload
    std::cout << "Measuring compute workload (1M sin/cos operations)...\n";
    rapl.start();
    computeWorkload(1000000);
    RaplEnergy result = rapl.stop();

    result.report(std::cout);

    // Measure memory workload
    std::cout << "\nMeasuring memory workload (10M element array)...\n";
    rapl.start();
    memoryWorkload(10000000);
    result = rapl.stop();

    result.report(std::cout);
}

void demonstrateScopedMeasurement() {
    std::cout << "\n========================================\n";
    std::cout << "Scoped RAPL Measurement (RAII)\n";
    std::cout << "========================================\n\n";

    if (!RaplReader::isAvailable()) {
        std::cout << "RAPL not available, skipping scoped demo\n";
        return;
    }

    std::cout << "Using ScopedRaplMeasurement for automatic start/stop:\n\n";

    {
        ScopedRaplMeasurement measure("Compute 500K");
        computeWorkload(500000);
    }  // Measurement printed on destruction

    {
        ScopedRaplMeasurement measure("Memory 5M");
        memoryWorkload(5000000);
    }  // Measurement printed on destruction
}

void demonstrateEnergyComparison() {
    std::cout << "\n========================================\n";
    std::cout << "Energy Comparison: Compute vs Memory\n";
    std::cout << "========================================\n\n";

    if (!RaplReader::isAvailable()) {
        std::cout << "RAPL not available, skipping comparison demo\n";
        return;
    }

    RaplReader rapl;

    // Run multiple iterations for more stable measurements
    constexpr int trials = 3;
    double compute_energy_sum = 0.0;
    double memory_energy_sum = 0.0;

    std::cout << "Running " << trials << " trials of each workload...\n\n";

    for (int i = 0; i < trials; ++i) {
        rapl.start();
        computeWorkload(500000);
        RaplEnergy result = rapl.stop();
        compute_energy_sum += result.package_uj;

        rapl.start();
        memoryWorkload(5000000);
        result = rapl.stop();
        memory_energy_sum += result.package_uj;
    }

    double compute_avg = compute_energy_sum / trials;
    double memory_avg = memory_energy_sum / trials;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average Package Energy:\n";
    std::cout << "  Compute workload: " << compute_avg << " uJ\n";
    std::cout << "  Memory workload:  " << memory_avg << " uJ\n";
    std::cout << "  Memory/Compute ratio: " << (memory_avg / compute_avg) << "x\n";
}

void demonstrateModelValidation() {
    std::cout << "\n========================================\n";
    std::cout << "Understanding Model vs RAPL Measurements\n";
    std::cout << "========================================\n\n";

    if (!RaplReader::isAvailable()) {
        std::cout << "RAPL not available, skipping model validation demo\n";
        return;
    }

    const auto& model = getDefaultModel();
    std::cout << "Using model: " << model.name << "\n\n";

    // Estimate energy for 1M FP32 FMAs
    constexpr uint64_t N = 1000000;
    double estimated_pj = model.totalOperationEnergy(Operation::FloatFMA, BitWidth::bits_32, N);
    double estimated_uj = estimated_pj / 1e6;

    // Measure actual energy
    RaplReader rapl;
    rapl.start();
    computeWorkload(N);
    RaplEnergy measured = rapl.stop();

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "For " << N << " iterations:\n";
    std::cout << "  Model estimate (1M FP32 FMAs only):  " << estimated_uj << " uJ\n";
    std::cout << "  RAPL measured (total package):       " << measured.package_uj << " uJ\n";
    std::cout << "  Elapsed time:                        " << measured.elapsed_ms << " ms\n";
    std::cout << "  Average package power:               " << measured.averagePowerWatts() << " W\n";

    std::cout << "\n** Why the large difference? **\n\n";
    std::cout << "The model estimates MARGINAL energy (just the FMA unit transistors).\n";
    std::cout << "RAPL measures TOTAL PACKAGE energy, which includes:\n";
    std::cout << "  - Static/leakage power (~10-30W just being on)\n";
    std::cout << "  - Instruction fetch, decode, retire pipeline\n";
    std::cout << "  - L1/L2/L3 cache access energy\n";
    std::cout << "  - Out-of-order execution machinery\n";
    std::cout << "  - Memory controller, ring bus, uncore\n";
    std::cout << "  - All CPU cores (not just the active one)\n";
    std::cout << "\nAlso: sin()/cos() are NOT single FMAs - each requires ~10-20 FP ops.\n";

    std::cout << "\n** What are the models useful for? **\n\n";
    std::cout << "Relative comparisons between precisions and operations:\n";
    double e8  = model.operationEnergy(Operation::FloatFMA, BitWidth::bits_8);
    double e16 = model.operationEnergy(Operation::FloatFMA, BitWidth::bits_16);
    double e32 = model.operationEnergy(Operation::FloatFMA, BitWidth::bits_32);
    double e64 = model.operationEnergy(Operation::FloatFMA, BitWidth::bits_64);
    std::cout << "  FP8  FMA: " << e8  << " pJ (saves " << (e32/e8)  << "x vs FP32)\n";
    std::cout << "  FP16 FMA: " << e16 << " pJ (saves " << (e32/e16) << "x vs FP32)\n";
    std::cout << "  FP32 FMA: " << e32 << " pJ (baseline)\n";
    std::cout << "  FP64 FMA: " << e64 << " pJ (costs " << (e64/e32) << "x vs FP32)\n";

    // Demonstrate that RAPL can show relative differences
    std::cout << "\n** Using RAPL for relative measurements **\n\n";
    std::cout << "Running same workload 3x to show RAPL consistency:\n";
    for (int i = 1; i <= 3; ++i) {
        rapl.start();
        computeWorkload(N);
        RaplEnergy r = rapl.stop();
        std::cout << "  Trial " << i << ": " << r.package_uj << " uJ, "
                  << r.averagePowerWatts() << " W\n";
    }
}

int main()
try {
    std::cout << "Universal Numbers Library: RAPL Energy Measurement\n";
    std::cout << "===================================================\n\n";

    demonstrateRaplAvailability();
    demonstrateBasicMeasurement();
    demonstrateScopedMeasurement();
    demonstrateEnergyComparison();
    demonstrateModelValidation();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. RAPL measures TOTAL package energy (10-100W), not per-operation\n";
    std::cout << "2. Cost models estimate MARGINAL per-operation energy (picojoules)\n";
    std::cout << "3. Use cost models for: comparing precisions, algorithm design decisions\n";
    std::cout << "4. Use RAPL for: measuring actual system energy, validating optimizations\n";
    std::cout << "5. Memory access dominates: 1 DRAM read ≈ 400 FP32 FMAs in energy\n";

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
