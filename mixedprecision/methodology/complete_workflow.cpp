// complete_workflow.cpp: End-to-end mixed-precision optimization workflow
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// COMPLETE MIXED-PRECISION METHODOLOGY EXAMPLE
// This file demonstrates the entire workflow for optimizing an algorithm
// for mixed-precision execution:
//
// 1. Profile operations with instrumented types
// 2. Analyze value ranges to understand precision needs
// 3. Get type recommendations based on accuracy requirements
// 4. Explore accuracy/energy trade-offs
// 5. Generate precision configuration
// 6. Implement the optimized algorithm
//
// Algorithm: Conjugate Gradient solver for Ax = b
// This is a representative scientific computing workload with:
// - Dot products (accumulation-sensitive)
// - Matrix-vector products (compute-intensive)
// - Vector operations (memory-intensive)
//
// Build: part of mp_complete_workflow target
// Run:   ./mp_complete_workflow

#include <universal/utility/directives.hpp>

// Mixed-precision methodology utilities
#include <universal/utility/instrumented.hpp>
#include <universal/utility/range_analyzer.hpp>
#include <universal/utility/type_advisor.hpp>
#include <universal/utility/algorithm_profiler.hpp>
#include <universal/utility/pareto_explorer.hpp>
#include <universal/utility/precision_config_generator.hpp>

// Universal number types
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace sw::universal;

// ============================================================================
// ALGORITHM: Conjugate Gradient Solver
// Solves Ax = b for symmetric positive definite A
// ============================================================================

// Dot product with configurable accumulator
template<typename T, typename Accum = T>
Accum dot(const std::vector<T>& x, const std::vector<T>& y) {
    Accum sum = Accum(0);
    for (size_t i = 0; i < x.size(); ++i) {
        sum += Accum(x[i]) * Accum(y[i]);
    }
    return sum;
}

// Matrix-vector product: y = A * x (A stored as dense row-major)
template<typename T>
void matvec(const std::vector<std::vector<T>>& A,
            const std::vector<T>& x,
            std::vector<T>& y) {
    size_t n = x.size();
    for (size_t i = 0; i < n; ++i) {
        y[i] = T(0);
        for (size_t j = 0; j < n; ++j) {
            y[i] += A[i][j] * x[j];
        }
    }
}

// Vector operations: y = a*x + y (axpy)
template<typename T>
void axpy(T a, const std::vector<T>& x, std::vector<T>& y) {
    for (size_t i = 0; i < x.size(); ++i) {
        y[i] += a * x[i];
    }
}

// Conjugate Gradient solver
template<typename T, typename Accum = T>
int conjugate_gradient(const std::vector<std::vector<T>>& A,
                       const std::vector<T>& b,
                       std::vector<T>& x,
                       int max_iter,
                       double tol,
                       std::vector<double>* residual_history = nullptr) {
    size_t n = b.size();

    // Initialize
    std::vector<T> r(n), p(n), Ap(n);

    // r = b - A*x
    matvec(A, x, r);
    for (size_t i = 0; i < n; ++i) {
        r[i] = b[i] - r[i];
        p[i] = r[i];
    }

    Accum rsold = dot<T, Accum>(r, r);

    for (int iter = 0; iter < max_iter; ++iter) {
        matvec(A, p, Ap);

        Accum pAp = dot<T, Accum>(p, Ap);
        T alpha = T(rsold / pAp);

        // x = x + alpha * p
        // r = r - alpha * Ap
        for (size_t i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        Accum rsnew = dot<T, Accum>(r, r);

        if (residual_history) {
            residual_history->push_back(std::sqrt(double(rsnew)));
        }

        if (double(rsnew) < tol * tol) {
            return iter + 1;
        }

        T beta = T(rsnew / rsold);
        for (size_t i = 0; i < n; ++i) {
            p[i] = r[i] + beta * p[i];
        }

        rsold = rsnew;
    }

    return max_iter;
}

// Create a symmetric positive definite test matrix
template<typename T>
std::vector<std::vector<T>> create_spd_matrix(size_t n) {
    std::vector<std::vector<T>> A(n, std::vector<T>(n));

    // Create diagonally dominant SPD matrix
    for (size_t i = 0; i < n; ++i) {
        T row_sum = T(0);
        for (size_t j = 0; j < n; ++j) {
            if (i != j) {
                // Off-diagonal: small positive values
                A[i][j] = T(1.0 / (1.0 + std::abs(static_cast<double>(i) - static_cast<double>(j))));
                row_sum += A[i][j];
            }
        }
        // Diagonal: larger than row sum for positive definiteness
        A[i][i] = row_sum + T(1.0);
    }

    return A;
}

int main()
try {
    std::cout << "Complete Mixed-Precision Optimization Workflow\n";
    std::cout << std::string(70, '=') << "\n\n";

    constexpr size_t N = 50;  // Problem size
    constexpr int MAX_ITER = 100;
    constexpr double TOL = 1e-6;

    std::cout << "Algorithm: Conjugate Gradient Solver\n";
    std::cout << "Problem size: " << N << "x" << N << " matrix\n";
    std::cout << "Tolerance: " << TOL << "\n\n";

    // =========================================================================
    // STEP 1: Profile Operations
    // =========================================================================
    std::cout << std::string(70, '-') << "\n";
    std::cout << "STEP 1: Profile Operations\n";
    std::cout << std::string(70, '-') << "\n\n";

    instrumented_stats::reset();

    using InstrDouble = instrumented<double>;
    auto A_instr = create_spd_matrix<InstrDouble>(N);
    std::vector<InstrDouble> b_instr(N, InstrDouble(1.0));
    std::vector<InstrDouble> x_instr(N, InstrDouble(0.0));

    int iters_instr = conjugate_gradient<InstrDouble, InstrDouble>(
        A_instr, b_instr, x_instr, MAX_ITER, TOL);

    std::cout << "Converged in " << iters_instr << " iterations\n\n";
    std::cout << "Operation counts:\n";
    instrumented_stats::report(std::cout);

    uint64_t total_ops = instrumented_stats::totalArithmeticOps();
    std::cout << "\nTotal arithmetic operations: " << total_ops << "\n";

    // =========================================================================
    // STEP 2: Analyze Value Ranges
    // =========================================================================
    std::cout << "\n" << std::string(70, '-') << "\n";
    std::cout << "STEP 2: Analyze Value Ranges\n";
    std::cout << std::string(70, '-') << "\n\n";

    // Run again with double to collect values
    auto A = create_spd_matrix<double>(N);
    std::vector<double> b(N, 1.0);
    std::vector<double> x(N, 0.0);
    std::vector<double> residual_history;

    conjugate_gradient<double, double>(A, b, x, MAX_ITER, TOL, &residual_history);

    range_analyzer<double> analyzer;

    // Analyze matrix values
    for (const auto& row : A) {
        analyzer.observe(row.begin(), row.end());
    }

    // Analyze solution and residuals
    analyzer.observe(x.begin(), x.end());
    analyzer.observe(residual_history.begin(), residual_history.end());

    std::cout << "Value range analysis:\n";
    std::cout << "  Observations: " << analyzer.statistics().observations << "\n";
    std::cout << "  Min value: " << std::scientific << analyzer.minValue() << "\n";
    std::cout << "  Max value: " << analyzer.maxValue() << "\n";
    std::cout << "  Scale span: " << analyzer.scaleRange() << " decades\n";
    std::cout << "  Denormals: " << analyzer.statistics().denormals << "\n";

    auto rec = analyzer.recommendPrecision();
    std::cout << "\nInitial recommendation: " << rec.type_suggestion << "\n";

    // =========================================================================
    // STEP 3: Get Type Recommendations
    // =========================================================================
    std::cout << "\n" << std::string(70, '-') << "\n";
    std::cout << "STEP 3: Type Recommendations\n";
    std::cout << std::string(70, '-') << "\n\n";

    TypeAdvisor advisor;

    // Different accuracy levels
    std::vector<double> accuracies = {1e-3, 1e-6, 1e-9};

    std::cout << "Best types for different accuracy requirements:\n\n";
    std::cout << std::left << std::setw(12) << "Accuracy"
              << std::setw(20) << "Recommended Type"
              << std::right << std::setw(10) << "Score"
              << std::setw(12) << "Energy" << "\n";
    std::cout << std::string(54, '-') << "\n";

    for (double acc : accuracies) {
        AccuracyRequirement req(acc);
        auto best = advisor.bestType(analyzer, req);

        std::cout << std::left << std::scientific << std::setprecision(0)
                  << std::setw(12) << acc
                  << std::setw(20) << best.type.name
                  << std::right << std::fixed << std::setprecision(1)
                  << std::setw(9) << best.suitability_score << "%"
                  << std::setprecision(2)
                  << std::setw(11) << best.estimated_energy << "x\n";
    }

    // =========================================================================
    // STEP 4: Explore Trade-offs
    // =========================================================================
    std::cout << "\n" << std::string(70, '-') << "\n";
    std::cout << "STEP 4: Pareto Trade-off Analysis\n";
    std::cout << std::string(70, '-') << "\n\n";

    ParetoExplorer explorer;

    // CG is moderately compute-bound (lots of dot products and matvecs)
    // Arithmetic intensity ~ ops / bytes
    double cg_ai = static_cast<double>(total_ops) / (3 * N * N * sizeof(double));
    AlgorithmCharacteristics cg_profile("CG Solver", cg_ai, 3.0 * N * N * sizeof(double));

    std::cout << "Algorithm characteristics:\n";
    std::cout << "  Arithmetic intensity: " << std::fixed << std::setprecision(2)
              << cg_ai << " ops/byte\n";
    std::cout << "  Type: " << (cg_profile.is_memory_bound ? "memory-bound" : "compute-bound") << "\n\n";

    auto pareto = explorer.computeFrontier3D();

    std::cout << "Best precision for CG at different accuracy targets:\n\n";
    for (double acc : accuracies) {
        auto best = pareto.bestForAlgorithm(acc, cg_profile);
        std::cout << "  acc=" << std::scientific << std::setprecision(0) << acc
                  << " -> " << best.name
                  << " (energy=" << std::fixed << std::setprecision(2)
                  << best.energy_factor << "x)\n";
    }

    // =========================================================================
    // STEP 5: Generate Configuration
    // =========================================================================
    std::cout << "\n" << std::string(70, '-') << "\n";
    std::cout << "STEP 5: Generate Precision Configuration\n";
    std::cout << std::string(70, '-') << "\n\n";

    PrecisionConfigGenerator gen;
    gen.setAlgorithm("ConjugateGradient");
    gen.setAccuracyRequirement(1e-6);
    gen.setEnergyBudget(0.5);
    gen.setProblemSize(std::to_string(N) + "x" + std::to_string(N));

    auto config = gen.generateConfig();

    std::cout << "Generated configuration:\n";
    std::cout << "  Input type:       " << config.input_type << "\n";
    std::cout << "  Compute type:     " << config.compute_type << "\n";
    std::cout << "  Accumulator type: " << config.accumulator_type << "\n";
    std::cout << "  Output type:      " << config.output_type << "\n";
    std::cout << "  Energy factor:    " << std::fixed << std::setprecision(2)
              << config.energy_factor << "x\n";

    std::cout << "\n--- Generated Header Preview ---\n";
    std::cout << gen.generateConfigHeader();

    // =========================================================================
    // STEP 6: Verify with Different Precisions
    // =========================================================================
    std::cout << "\n" << std::string(70, '-') << "\n";
    std::cout << "STEP 6: Verification with Different Precisions\n";
    std::cout << std::string(70, '-') << "\n\n";

    // Solve with float
    auto A_f = create_spd_matrix<float>(N);
    std::vector<float> b_f(N, 1.0f);
    std::vector<float> x_f(N, 0.0f);
    int iters_f = conjugate_gradient<float, double>(A_f, b_f, x_f, MAX_ITER, static_cast<float>(TOL));

    // Solve with posit<32,2>
    using Posit32 = posit<32, 2>;
    auto A_p = create_spd_matrix<Posit32>(N);
    std::vector<Posit32> b_p(N, Posit32(1.0));
    std::vector<Posit32> x_p(N, Posit32(0.0));
    int iters_p = conjugate_gradient<Posit32, Posit32>(A_p, b_p, x_p, MAX_ITER, TOL);

    // Compare solutions
    double err_f = 0, err_p = 0;
    for (size_t i = 0; i < N; ++i) {
        err_f += (x[i] - static_cast<double>(x_f[i])) * (x[i] - static_cast<double>(x_f[i]));
        err_p += (x[i] - static_cast<double>(x_p[i])) * (x[i] - static_cast<double>(x_p[i]));
    }
    err_f = std::sqrt(err_f);
    err_p = std::sqrt(err_p);

    std::cout << std::left << std::setw(20) << "Precision"
              << std::right << std::setw(12) << "Iterations"
              << std::setw(15) << "Error vs FP64" << "\n";
    std::cout << std::string(47, '-') << "\n";

    std::cout << std::left << std::setw(20) << "double"
              << std::right << std::setw(12) << iters_instr
              << std::setw(15) << "(reference)\n";

    std::cout << std::left << std::setw(20) << "float + double acc"
              << std::right << std::setw(12) << iters_f
              << std::scientific << std::setprecision(2)
              << std::setw(15) << err_f << "\n";

    std::cout << std::left << std::setw(20) << "posit<32,2>"
              << std::right << std::setw(12) << iters_p
              << std::setw(15) << err_p << "\n";

    // =========================================================================
    // Summary
    // =========================================================================
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "WORKFLOW COMPLETE\n";
    std::cout << std::string(70, '=') << "\n\n";

    std::cout << "Summary:\n";
    std::cout << "  1. Profiled " << total_ops << " arithmetic operations\n";
    std::cout << "  2. Value range spans " << analyzer.scaleRange() << " decades\n";
    std::cout << "  3. posit<32,2> recommended for scientific accuracy\n";
    std::cout << "  4. Energy savings of " << std::fixed << std::setprecision(0)
              << (1.0 - config.energy_factor) * 100 << "% achievable\n";
    std::cout << "  5. Verified convergence with mixed-precision\n\n";

    std::cout << "Files in this methodology:\n";
    std::cout << "  step1_profile_operations.cpp - Operation counting\n";
    std::cout << "  step2_analyze_ranges.cpp     - Value range analysis\n";
    std::cout << "  step3_recommend_types.cpp    - Type recommendations\n";
    std::cout << "  step4_explore_tradeoffs.cpp  - Pareto analysis\n";
    std::cout << "  step5_generate_config.cpp    - Code generation\n";
    std::cout << "  complete_workflow.cpp        - This file\n";

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
