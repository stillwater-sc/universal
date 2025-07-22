#pragma once
// cage3.hpp: test matrix
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
// 
// This file is part of the universal numbers project.
#include <numeric/containers/matrix.hpp>

// Matrix ID    = 904
// Rank         = 5
// Matrix Norm  = 1.136696e+00; 
// Cond.        = 1.884547e+01; 
// Symmetric    = No
// Pos.def      = No 
// NNZ          = 19
// Directed Weighted Graph (1 connected component)
// Source: https://sparse.tamu.edu/vanHeukelum/cage3

sw::universal::blas::matrix<double> cage3 = {
 {6.666667e-01, 3.665560e-01, 3.001107e-01, 3.665560e-01, 3.001107e-01 },
 {1.000369e-01, 5.334071e-01, 0.000000e+00, 2.000738e-01, 0.000000e+00 },
 {1.221853e-01, 0.000000e+00, 5.777040e-01, 0.000000e+00, 2.443707e-01 },
 {5.001844e-02, 1.000369e-01, 0.000000e+00, 2.833149e-01, 1.832780e-01 },
 {6.109267e-02, 0.000000e+00, 1.221853e-01, 1.500553e-01, 2.722407e-01 },
 };
