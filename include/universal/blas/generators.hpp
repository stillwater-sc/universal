#pragma once
// generators.hpp: matrix generators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>

// matrix generators
#include <universal/blas/generators/index.hpp>
#include <universal/blas/generators/magic.hpp>
#include <universal/blas/generators/frank.hpp>
#include <universal/blas/generators/hilbert.hpp>
#include <universal/blas/generators/minij.hpp>

// random matrices
#include <universal/blas/generators/uniform_random.hpp>
#include <universal/blas/generators/gaussian_random.hpp>
#include <universal/blas/generators/randsvd.hpp>

// PDE and ODE matrices
#include <universal/blas/generators/tridiag.hpp>
#include <universal/blas/generators/laplace2D.hpp>
