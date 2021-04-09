#pragma once
// solvers.hpp: constraint solvers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>

// matrix generators
#include <universal/blas/solvers/jacobi.hpp>
#include <universal/blas/solvers/gauss_seidel.hpp>
#include <universal/blas/solvers/sor.hpp>
#include <universal/blas/solvers/find_rank.hpp>
#include <universal/blas/solvers/svd.hpp>

#include <universal/blas/solvers/cg_dot_dot.hpp>
#include <universal/blas/solvers/cg_dot_fdp.hpp>
#include <universal/blas/solvers/cg_fdp_dot.hpp>
#include <universal/blas/solvers/cg_fdp_fdp.hpp>
