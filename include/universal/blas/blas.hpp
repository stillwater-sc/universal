// blas.hpp: top-level include for Universal BLAS library
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
// 
// Super-simple BLAS implementation to aid application,
// numerical, and reproducibility examples.
#ifndef _UNIVERSAL_BLAS_LIBRARY
#define _UNIVERSAL_BLAS_LIBRARY

#include <cstdint>

// aggregation types for serialization
constexpr uint32_t UNIVERSAL_AGGREGATE_SCALAR = 0x1001;
constexpr uint32_t UNIVERSAL_AGGREGATE_VECTOR = 0x2002;
constexpr uint32_t UNIVERSAL_AGGREGATE_MATRIX = 0x4004;
constexpr uint32_t UNIVERSAL_AGGREGATE_TENSOR = 0x8008;

#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>
#include <universal/blas/tensor.hpp>

constexpr uint64_t SIZE_1K   = 1024;
constexpr uint64_t SIZE_2K   = 2 * SIZE_1K;
constexpr uint64_t SIZE_4K   = 4 * SIZE_1K;
constexpr uint64_t SIZE_8K   = 8 * SIZE_1K;
constexpr uint64_t SIZE_16K  = 16 * SIZE_1K;
constexpr uint64_t SIZE_32K  = 32 * SIZE_1K;
constexpr uint64_t SIZE_64K  = 64 * SIZE_1K;
constexpr uint64_t SIZE_128K = 128 * SIZE_1K;
constexpr uint64_t SIZE_256K = 256 * SIZE_1K;
constexpr uint64_t SIZE_512K = 512 * SIZE_1K;
constexpr uint64_t SIZE_1M   = 1024 * 1024;
constexpr uint64_t SIZE_2M   = 2 * SIZE_1M;
constexpr uint64_t SIZE_4M   = 4 * SIZE_1M;
constexpr uint64_t SIZE_8M   = 8 * SIZE_1M;
constexpr uint64_t SIZE_16M  = 16 * SIZE_1M;
constexpr uint64_t SIZE_32M  = 32 * SIZE_1M;
constexpr uint64_t SIZE_64M  = 64 * SIZE_1M;
constexpr uint64_t SIZE_128M = 128 * SIZE_1M;
constexpr uint64_t SIZE_256M = 256 * SIZE_1M;
constexpr uint64_t SIZE_512M = 512 * SIZE_1M;
constexpr uint64_t SIZE_1G   = 1024 * 1024 * 1024;
constexpr uint64_t SIZE_2G   = 2 * SIZE_1G;
constexpr uint64_t SIZE_4G   = 4 * SIZE_1G;
constexpr uint64_t SIZE_8G   = 8 * SIZE_1G;
constexpr uint64_t SIZE_16G  = 16 * SIZE_1G;
constexpr uint64_t SIZE_32G  = 32 * SIZE_1G;
constexpr uint64_t SIZE_64G  = 64 * SIZE_1G;
constexpr uint64_t SIZE_128G = 128 * SIZE_1G;
constexpr uint64_t SIZE_256G = 256 * SIZE_1G;
constexpr uint64_t SIZE_512G = 512 * SIZE_1G;

// L1 operators
#include <universal/blas/blas_l1.hpp>

// L2
#include <universal/blas/blas_l2.hpp>

// L3
#include <universal/blas/blas_l3.hpp>
#include <universal/blas/inverse.hpp>

// Matrix operators
#include <universal/blas/operators.hpp>
#include <universal/blas/squeeze.hpp>

// solvers
#include <universal/blas/solvers/lu.hpp>
#include <universal/blas/solvers/lsq.hpp>
#include <universal/blas/solvers/qr.hpp>
#include <universal/blas/solvers/svd.hpp>
#include <universal/blas/solvers/plu.hpp>
#include <universal/blas/solvers/backsub.hpp>
#include <universal/blas/solvers/forwsub.hpp>

// Matrix generators
#include <universal/blas/generators.hpp>

// Utilities
#include <universal/blas/scaling.hpp>
#include <universal/blas/linspace.hpp>
#include <universal/blas/utes/matnorm.hpp>
#include <universal/blas/utes/condest.hpp>
#include <universal/blas/utes/nbe.hpp>      // Normwise Backward Error

// Statistics
#include <universal/blas/statistics.hpp>

// MATLAB-style elementary vector functions
#include <universal/blas/vmath/square.hpp>
#include <universal/blas/vmath/sqrt.hpp>
#include <universal/blas/vmath/power.hpp>
#include <universal/blas/vmath/trigonometry.hpp>

// Matrix utilities


#endif // _UNIVERSAL_BLAS_LIBRARY
