#pragma once
// containers.hpp: declaration of numeric containers (vector, matrix, tensor)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// aggregation types for serialization
constexpr uint32_t UNIVERSAL_AGGREGATE_SCALAR = 0x1001;
constexpr uint32_t UNIVERSAL_AGGREGATE_VECTOR = 0x2002;
constexpr uint32_t UNIVERSAL_AGGREGATE_MATRIX = 0x4004;
constexpr uint32_t UNIVERSAL_AGGREGATE_TENSOR = 0x8008;

#include <numeric/containers/vector.hpp>
#include <numeric/containers/matrix.hpp>
#include <numeric/containers/tensor.hpp>

