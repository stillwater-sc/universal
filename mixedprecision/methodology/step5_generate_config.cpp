// step5_generate_config.cpp: Generate C++ headers for mixed-precision implementations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// STEP 5 OF MIXED-PRECISION METHODOLOGY:
// Generate ready-to-use C++ headers with type aliases for your
// mixed-precision algorithm implementation.
//
// Key concepts:
// - PrecisionConfigGenerator creates type alias headers
// - Configures InputType, ComputeType, AccumulatorType, OutputType
// - Generates example usage code
// - Documents rationale and expected energy savings
//
// Build: part of mp_step5_config target
// Run:   ./mp_step5_config

#include <universal/utility/directives.hpp>
#include <universal/utility/precision_config_generator.hpp>
#include <iostream>
#include <fstream>

using namespace sw::universal;

int main()
try {
    std::cout << "Step 5: Generate Configuration with PrecisionConfigGenerator\n";
    std::cout << std::string(60, '=') << "\n\n";

    // =========================================
    // Example 1: Generate config for GEMM
    // =========================================
    std::cout << "Example 1: GEMM Configuration (1e-4 accuracy, 50% energy)\n";
    std::cout << std::string(50, '-') << "\n\n";

    PrecisionConfigGenerator gemm_gen;
    gemm_gen.setAlgorithm("GEMM");
    gemm_gen.setAccuracyRequirement(1e-4);
    gemm_gen.setEnergyBudget(0.5);
    gemm_gen.setProblemSize("1024x1024");

    std::cout << gemm_gen.generateConfigHeader();

    // =========================================
    // Example 2: Generate config for dot product
    // =========================================
    std::cout << "\n\nExample 2: Dot Product Configuration (1e-6 accuracy, 30% energy)\n";
    std::cout << std::string(50, '-') << "\n\n";

    PrecisionConfigGenerator dot_gen;
    dot_gen.setAlgorithm("DotProduct");
    dot_gen.setAccuracyRequirement(1e-6);
    dot_gen.setEnergyBudget(0.3);
    dot_gen.setProblemSize("N=1000000");

    std::cout << dot_gen.generateConfigHeader();

    // =========================================
    // Example 3: Generate example usage code
    // =========================================
    std::cout << "\n\nExample 3: Usage Code Template\n";
    std::cout << std::string(50, '-') << "\n\n";

    std::cout << gemm_gen.generateExampleCode();

    // =========================================
    // Example 4: Comparison report
    // =========================================
    std::cout << "\n\nExample 4: Comparison at Different Accuracy Levels\n";
    std::cout << std::string(50, '-') << "\n\n";

    std::cout << gemm_gen.generateComparisonReport();

    // =========================================
    // Example 5: Full analysis output
    // =========================================
    std::cout << "\n\nExample 5: Full Analysis for Conv2D\n";
    std::cout << std::string(50, '-') << "\n\n";

    PrecisionConfigGenerator conv_gen;
    conv_gen.setAlgorithm("Conv2D");
    conv_gen.setAccuracyRequirement(1e-3);  // ML inference
    conv_gen.setEnergyBudget(0.25);         // Aggressive energy target
    conv_gen.setProblemSize("224x224x64");

    conv_gen.printAnalysis(std::cout);

    // =========================================
    // Example 6: Write config to file
    // =========================================
    std::cout << "\n\nExample 6: Writing Configuration to File\n";
    std::cout << std::string(50, '-') << "\n\n";

    // In a real application, you would write to a file:
    // std::ofstream out("gemm_precision_config.hpp");
    // out << gemm_gen.generateConfigHeader();
    // out.close();

    std::cout << "To save configuration to file:\n";
    std::cout << "  std::ofstream out(\"gemm_precision_config.hpp\");\n";
    std::cout << "  out << generator.generateConfigHeader();\n";
    std::cout << "  out.close();\n";

    // =========================================
    // Example 7: Show how to use generated config
    // =========================================
    std::cout << "\n\nExample 7: Using Generated Configuration\n";
    std::cout << std::string(50, '-') << "\n\n";

    std::cout << R"(
// In your algorithm file:

#include "gemm_precision_config.hpp"
#include <universal/blas/blas.hpp>

using namespace gemm_config;

void optimized_gemm(const std::vector<InputType>& A,
                    const std::vector<InputType>& B,
                    std::vector<OutputType>& C,
                    size_t M, size_t N, size_t K) {

    // Convert inputs to compute precision
    std::vector<ComputeType> work_A(A.begin(), A.end());
    std::vector<ComputeType> work_B(B.begin(), B.end());
    std::vector<AccumulatorType> work_C(M * N, AccumulatorType(0));

    // Compute with accumulator precision
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            AccumulatorType sum = 0;
            for (size_t k = 0; k < K; ++k) {
                sum += AccumulatorType(work_A[i*K + k]) *
                       AccumulatorType(work_B[k*N + j]);
            }
            work_C[i*N + j] = sum;
        }
    }

    // Convert to output precision
    C.resize(M * N);
    for (size_t i = 0; i < M * N; ++i) {
        C[i] = OutputType(work_C[i]);
    }
}
)";

    // =========================================
    // Summary
    // =========================================
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Key Insights:\n";
    std::cout << "  - Generated headers provide ready-to-use type aliases\n";
    std::cout << "  - Use InputType for loading data (can be lower precision)\n";
    std::cout << "  - Use ComputeType for arithmetic (balances accuracy/energy)\n";
    std::cout << "  - Use AccumulatorType for reductions (prevents error growth)\n";
    std::cout << "  - Use OutputType for storing results (meets accuracy target)\n";
    std::cout << "\nComplete Workflow:\n";
    std::cout << "  1. Profile operations (step1)\n";
    std::cout << "  2. Analyze ranges (step2)\n";
    std::cout << "  3. Get type recommendations (step3)\n";
    std::cout << "  4. Explore trade-offs (step4)\n";
    std::cout << "  5. Generate configuration (step5) <-- You are here\n";
    std::cout << "\nSee: complete_workflow.cpp for end-to-end example\n";

    return EXIT_SUCCESS;
}
catch (const char* msg) {
    std::cerr << "Caught exception: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
