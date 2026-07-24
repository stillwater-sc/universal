#pragma once
// containers.hpp: declaration of numeric containers (vector, matrix, tensor)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// -------------------------------------------------------------------------
// DEPRECATED (universal#1204): the <numeric/containers/...> matrix/vector/tensor
// types are being extracted alongside the BLAS. Use MTL5's containers
// (https://github.com/stillwater-sc/mtl5). These headers will be removed from
// Universal after this deprecation release.
// Define UNIVERSAL_SUPPRESS_DEPRECATION to silence this notice.
// -------------------------------------------------------------------------
#ifndef UNIVERSAL_SUPPRESS_DEPRECATION
#pragma message("DEPRECATED: Universal <numeric/containers/...> is being extracted to MTL5 (see universal#1204); define UNIVERSAL_SUPPRESS_DEPRECATION to silence")
#endif

// aggregation types for serialization
constexpr uint32_t UNIVERSAL_AGGREGATE_SCALAR = 0x1001;
constexpr uint32_t UNIVERSAL_AGGREGATE_VECTOR = 0x2002;
constexpr uint32_t UNIVERSAL_AGGREGATE_MATRIX = 0x4004;
constexpr uint32_t UNIVERSAL_AGGREGATE_TENSOR = 0x8008;

#include <numeric/containers/vector.hpp>
#include <numeric/containers/matrix.hpp>
#include <numeric/containers/tensor.hpp>

