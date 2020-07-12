#pragma once
// blas.hpp: super-simple BLAS implementation to aid the application examples
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "vector.hpp"
#include "matrix.hpp"

#include "blas_l1.hpp"
#include "blas_l2.hpp"
#include "blas_l3.hpp"

constexpr size_t SIZE_1K   = 1024;
constexpr size_t SIZE_2K   = 2 * SIZE_1K;
constexpr size_t SIZE_4K   = 4 * SIZE_1K;
constexpr size_t SIZE_8K   = 8 * SIZE_1K;
constexpr size_t SIZE_16K  = 16 * SIZE_1K;
constexpr size_t SIZE_32K  = 32 * SIZE_1K;
constexpr size_t SIZE_64K  = 64 * SIZE_1K;
constexpr size_t SIZE_128K = 128 * SIZE_1K;
constexpr size_t SIZE_256K = 256 * SIZE_1K;
constexpr size_t SIZE_512K = 512 * SIZE_1K;
constexpr size_t SIZE_1M   = 1024 * 1024;
constexpr size_t SIZE_2M   = 2 * SIZE_1M;
constexpr size_t SIZE_4M   = 4 * SIZE_1M;
constexpr size_t SIZE_8M   = 8 * SIZE_1M;
constexpr size_t SIZE_16M  = 16 * SIZE_1M;
constexpr size_t SIZE_32M  = 32 * SIZE_1M;
constexpr size_t SIZE_64M  = 64 * SIZE_1M;
constexpr size_t SIZE_128M = 128 * SIZE_1M;
constexpr size_t SIZE_256M = 256 * SIZE_1M;
constexpr size_t SIZE_512M = 512 * SIZE_1M;
constexpr size_t SIZE_1G   = 1024 * 1024 * 1024;
constexpr size_t SIZE_2G   = 2 * SIZE_1G;
constexpr size_t SIZE_4G   = 4 * SIZE_1G;
constexpr size_t SIZE_8G   = 8 * SIZE_1G;
constexpr size_t SIZE_16G  = 16 * SIZE_1G;
constexpr size_t SIZE_32G  = 32 * SIZE_1G;
constexpr size_t SIZE_64G  = 64 * SIZE_1G;
constexpr size_t SIZE_128G = 128 * SIZE_1G;
constexpr size_t SIZE_256G = 256 * SIZE_1G;
constexpr size_t SIZE_512G = 512 * SIZE_1G;


// L2

// L3
#include "lu.hpp"
#include "lsq.hpp"
