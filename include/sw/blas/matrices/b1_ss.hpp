#pragma once
// b1_ss.hpp: test matrix
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
// 
// This file is part of the universal numbers project.
#include <numeric/containers/matrix.hpp>

// Size = 7 x 7
// Rank 7; 
// Matrix Norm = 2.012180 
// Cond. =  1.973732e+02
// symmetric = NO
/// pos.def = NO 
// Matrix ID = 449
// NNZ = 15
// Kind: Chemical Process Simulation Problem

sw::universal::blas::matrix<double>  b1_ss = {
{0.000000e+00, 1.000000e+00, 1.000000e+00, 1.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00 },
 {0.000000e+00, -1.000000e+00, 0.000000e+00, 0.000000e+00, 4.500000e-01, 0.000000e+00, 0.000000e+00 },
 {0.000000e+00, 0.000000e+00, -1.000000e+00, 0.000000e+00, 0.000000e+00, 1.000000e-01, 0.000000e+00 },
 {0.000000e+00, 0.000000e+00, 0.000000e+00, -1.000000e+00, 0.000000e+00, 0.000000e+00, 4.500000e-01 },
 {-3.599942e-02, 0.000000e+00, 0.000000e+00, 0.000000e+00, 1.000000e+00, 0.000000e+00, 0.000000e+00 },
 {-1.763710e-02, 0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 1.000000e+00, 0.000000e+00 },
 {-7.721779e-03, 0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 0.000000e+00, 1.000000e+00 },
 };