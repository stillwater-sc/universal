#pragma once
// h3.hpp: test matrix
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
// 
// This file is part of the universal numbers project.
#include <numeric/containers/matrix.hpp>

// Matrix ID    = higham2019squeezing (Higham 2019 Squeeze Matrix)
// Size         = 3 x 3
// Rank         = 3
// Norm         = 1.4142e+11; 
// Condition    = 1.8478e+11
// symmetric    = NO
// pos.def      = NO 
// NNZ          = 8

sw::universal::blas::matrix<double> h3 = {
{1.0, 1.0, 2.5e6},
{1.0, -1.0, 2.5e6},
{1.0, 1.0, 0.0}
};

/* 
Higham's Examples in Squeeze
Norm = 3.5355e+06
Cond = 4.6194e+06

{1.0, 1.0, 2.5e6},
{1.0, -1.0, 2.5e6},
{1.0, 1.0, 0.0}

{1.0, 1.0, 2.5e11},
{1.0, -1.0, 2.5e11},
{1.0, 1.0, 0.0}

{1.0,       9.0e-15,     9.0e-15},
{9.0e-15,   9.0e-15,    -9.0e-15},
{1.0,      -9.0e-15,     9.0e-15}

{1.0, 1.0e-8, 1.0e-8},
{1.0e-8, 1.0e-8, -1.0e-8},
{1.0, -1.0e-8, 1.0e-8}
*/
// if Amax > 2e12, then 0 (underflow) ==> singular
// subnormals in fp16 if 1e9 < alpha = Amax < 1e12