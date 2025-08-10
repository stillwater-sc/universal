#pragma once
// q5.hpp: test matrix
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
// 
// This file is part of the universal numbers project.
#include <numeric/containers/matrix.hpp>

// Matrix ID 	= x
// Size 		= 5 x 5 
// Rank 		= 5 
// Norm 		= 66000; 
// Cond 		= 1.1e+04
// symmetric 	= No
// pos.def 		= No 
// NNZ 			= 16

sw::numeric::containers::matrix<double> q5 = {
    { 10.0, -1.0,  2.0,  0.0, 0.0}, 
	{ -1.0, 11.0, -1.0,  3.0, 0.0},
	{  2.0, -1.0, 10.0, -1.0, 0.0},
	{  0.0,  3.0, -1.0,  8.0, 0.0}, 
    {  0.0,  0.0,  0.0,  3.2, -66000} 
};

// Let b = {11, 12, 10, 10 ,66003} for exact solution x = {1,1,1,1,1}.

