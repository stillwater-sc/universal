#pragma once
// cg_fdp_solvers.hpp: specialized CG constraint solvers that use FDP
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <numeric/containers.hpp>

// specialized conjugate gradient solvers with FDP and dot products
#include <blas/solvers/cg_dot_dot.hpp>
#include <blas/solvers/cg_dot_fdp.hpp>
#include <blas/solvers/cg_fdp_dot.hpp>
#include <blas/solvers/cg_fdp_fdp.hpp>
