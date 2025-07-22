#pragma once
// q3.hpp: test matrix
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
// 
// This file is part of the universal numbers project.
#include <numeric/containers/matrix.hpp>

// Matrix ID    = ---
// Size         = 3 x 3
// Rank         = 3
// Norm         = 
// Condition    = 1.2857e+06
// symmetric    = NO
// pos.def      = NO 
// NNZ          = 9

/*
Use to check LU decomposition and Amax, etc.  
Easy to view 3 x 3 matrix.  

See also q4, q5
*/

sw::numeric::containers::matrix<double>  q3 = {
	{1.001, 2.01, 0.3},
	{1.000001, 2.0, 0.3333},
	{1.0, 2.0, 0.3333333}
};

/*
sw::universal::blas::matrix<double>  q3 = {
{1.0, 2.0, 0.3333333},
{1.000001, 2.0, 0.3333},
{1.001, 2.01, 0.3}
};
*/
