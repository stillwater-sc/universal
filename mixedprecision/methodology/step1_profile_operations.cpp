// step1_profile_operations.cpp: Count arithmetic operations using instrumented types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// STEP 1 OF MIXED-PRECISION METHODOLOGY:
// Profile your algorithm to count exact operation counts.
// This data feeds into energy estimation models.
//
// Key concepts:
// - instrumented<T> wraps any number type to count operations
// - Thread-safe atomic counters for parallel algorithms
// - Zero overhead when not using instrumented types
//
// Build: part of mp_step1_profile target
// Run:   ./mp_step1_profile

#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/instrumented.hpp>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace sw::universal;

// A simple dot product - the algorithm we want to profile
template<typename T>
T dot_product(const std::vector<T>& x, const std::vector<T>& y) {
    T result = T(0);
    for (size_t i = 0; i < x.size(); ++i) {
        result += x[i] * y[i];  // 1 mul + 1 add per iteration
    }
    return result;
}

// Matrix-vector multiply: y = A * x
template<typename T>
void matvec(const std::vector<std::vector<T>>& A,
            const std::vector<T>& x,
            std::vector<T>& y) {
    size_t M = A.size();
    size_t N = x.size();
    for (size_t i = 0; i < M; ++i) {
        y[i] = T(0);
        for (size_t j = 0; j < N; ++j) {
            y[i] += A[i][j] * x[j];  // 1 mul + 1 add per inner iteration
        }
    }
}

int main()
try {
    std::cout << "Step 1: Profile Operations with instrumented<T>\n";
    std::cout << std::string(60, '=') << "\n\n";

    constexpr size_t N = 1000;

    // Create test data
    std::vector<double> x_data(N), y_data(N);
    for (size_t i = 0; i < N; ++i) {
        x_data[i] = std::sin(static_cast<double>(i) * 0.01);
        y_data[i] = std::cos(static_cast<double>(i) * 0.01);
    }

    // =========================================
    // Example 1: Profile dot product with double
    // =========================================
    std::cout << "Example 1: Dot Product (N=" << N << ")\n";
    std::cout << std::string(40, '-') << "\n";

    // Reset counters before profiling
    instrumented_stats::reset();

    // Use instrumented<double> instead of double
    using InstrDouble = instrumented<double>;
    std::vector<InstrDouble> ix(x_data.begin(), x_data.end());
    std::vector<InstrDouble> iy(y_data.begin(), y_data.end());

    InstrDouble result = dot_product(ix, iy);

    std::cout << "Result: " << double(result) << "\n\n";
    std::cout << "Operation counts:\n";
    instrumented_stats::report(std::cout);

    std::cout << "\nExpected: " << N << " muls, " << N << " adds\n";
    std::cout << "Actual:   " << instrumented_stats::muls.load() << " muls, "
              << instrumented_stats::adds.load() << " adds\n\n";

    // =========================================
    // Example 2: Profile with posit type
    // =========================================
    std::cout << "\nExample 2: Dot Product with posit<32,2>\n";
    std::cout << std::string(40, '-') << "\n";

    instrumented_stats::reset();

    using InstrPosit = instrumented<posit<32,2>>;
    std::vector<InstrPosit> px(x_data.begin(), x_data.end());
    std::vector<InstrPosit> py(y_data.begin(), y_data.end());

    InstrPosit presult = dot_product(px, py);

    std::cout << "Result: " << double(presult) << "\n\n";
    instrumented_stats::report(std::cout);

    // =========================================
    // Example 3: Profile matrix-vector multiply
    // =========================================
    constexpr size_t M = 100;
    std::cout << "\nExample 3: Matrix-Vector Multiply (" << M << "x" << M << ")\n";
    std::cout << std::string(40, '-') << "\n";

    // Create matrix and vector
    std::vector<std::vector<InstrDouble>> A(M, std::vector<InstrDouble>(M));
    std::vector<InstrDouble> vec_x(M), vec_y(M);

    for (size_t i = 0; i < M; ++i) {
        vec_x[i] = InstrDouble(static_cast<double>(i) / M);
        for (size_t j = 0; j < M; ++j) {
            A[i][j] = InstrDouble(static_cast<double>(i + j) / (M * M));
        }
    }

    instrumented_stats::reset();
    matvec(A, vec_x, vec_y);

    std::cout << "Operation counts:\n";
    instrumented_stats::report(std::cout);

    std::cout << "\nExpected: " << M*M << " muls, " << M*M << " adds\n";
    std::cout << "Actual:   " << instrumented_stats::muls.load() << " muls, "
              << instrumented_stats::adds.load() << " adds\n";

    // =========================================
    // Summary: How to use this data
    // =========================================
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Next Steps:\n";
    std::cout << "  1. Use operation counts with energy cost models\n";
    std::cout << "  2. Compare operation counts across different algorithms\n";
    std::cout << "  3. Identify compute-bound vs memory-bound characteristics\n";
    std::cout << "  4. Feed into algorithm_profiler for full analysis\n";
    std::cout << "\nSee: include/sw/universal/energy/occurrence_energy.hpp\n";

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
