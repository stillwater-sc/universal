#pragma once
// q4.hpp: test matrix
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
// 
// This file is part of the universal numbers project.
#include <numeric/containers/matrix.hpp>

// Matrix ID 	= x
// Size = 4 x 4 
// Rank 		= 4 
// Norm 		= 14.x
// Condition #  =  2.35
// symmetric 	= YES
// pos.def 		= YES 
// NNZ 			= 14

/*
Use to check LU decomposition and Amax, etc.  
Easy to view 3 x 3 matrix.  

See also q3, q5
*/


/*
Use to check LU decomposition and Amax, etc.  
Easy to view 3 x 3 matrix.  

See also q3, q5
*/

sw::numeric::containers::matrix<double> q4 = {
    { 10.0, -1.0,  2.0,  0.0}, 
	{ -1.0, 11.0, -1.0,  3.0},
	{  2.0, -1.0, 10.0, -1.0},
	{  0.0,  3.0, -1.0,  8.0} 
};
// Let b = {11, 12, 10, 10} for exact solution x = {1,1,1,1}.

// Let b = {11, 12, 10, 10} for exact solution x = {1,1,1,1}.

