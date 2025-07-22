// blas.hpp: top-level include for Universal BLAS library
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
// 
// Super-simple BLAS implementation to aid application,
// numerical, and reproducibility examples.
#ifndef _STILLWATER_BLAS_LIBRARY
#define _STILLWATER_BLAS_LIBRARY

#include <cstdint>

// Numeric containers
#include <numeric/containers.hpp>

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

// BLAS exceptions
#include <blas/exceptions.hpp>

// L1/L2/L3 BLAS functions
#include <blas/blas/blas_l1.hpp>
#include <blas/blas/blas_l2.hpp>
#include <blas/blas/blas_l3.hpp>

// Inverse 
#include <blas/blas/inverse.hpp>

// Matrix operators
#include <blas/operators.hpp>

// solvers
#include <blas/solvers/jacobi.hpp>
#include <blas/solvers/gauss_seidel.hpp>
#include <blas/solvers/sor.hpp>
#include <blas/solvers/find_rank.hpp>
#include <blas/solvers/lu.hpp>
#include <blas/solvers/lsq.hpp>
#include <blas/solvers/qr.hpp>
#include <blas/solvers/svd.hpp>
#include <blas/solvers/plu.hpp>
#include <blas/solvers/backsub.hpp>
#include <blas/solvers/forwsub.hpp>

// Matrix generators
#include <blas/generators.hpp>

// Utilities
#include <blas/linspace.hpp>
#include <blas/scaling.hpp>
#include <blas/utes/matnorm.hpp>
#include <blas/utes/condest.hpp>
#include <blas/utes/nbe.hpp>      // Normwise Backward Error

// Statistics
#include <blas/statistics.hpp>

// MATLAB-style element-wise vector functions
#include <blas/vmath/square.hpp>
#include <blas/vmath/sqrt.hpp>
#include <blas/vmath/power.hpp>
#include <blas/vmath/trigonometry.hpp>

// Matrix utilities


#endif // _STILLWATER_BLAS_LIBRARY
