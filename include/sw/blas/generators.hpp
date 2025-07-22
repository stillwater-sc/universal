#pragma once
// generators.hpp: matrix generators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <numeric/containers/vector.hpp>
#include <numeric/containers/matrix.hpp>

// matrix generators
#include <blas/generators/index.hpp>
#include <blas/generators/magic.hpp>
#include <blas/generators/frank.hpp>
#include <blas/generators/hilbert.hpp>
#include <blas/generators/minij.hpp>

// random matrices
#include <blas/generators/uniform_random.hpp>
#include <blas/generators/gaussian_random.hpp>
#include <blas/generators/randsvd.hpp>

// PDE and ODE matrices
#include <blas/generators/tridiag.hpp>
#include <blas/generators/laplace2D.hpp>
