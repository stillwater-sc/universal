#pragma once
// lu4.hpp: test matrix
//
// Copyright (c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
// 
// This file is part of the universal numbers project.
#include <numeric/containers/matrix.hpp>

// Matrix ID   = ---
// Size        = 4 x 4 
// Rank        = 4
// Norm        = 10.6585 
// Cond.       = 11.6810 
// symmetric   = No
// pos.def     = No   
// NNZ         = 12

/*
Use to check LU decomposition and Amax, etc.  
Easy to view 4 x 4 matrix.  

See also int3.hpp 
*/

sw::numeric::containers::matrix<double>  lu4 = {
    { 1.0,   1.0,   0.0,   3.0}, 
	{ 0.0,  -1.0,  -1.0,  -5.0},
	{ 0.0,  -4.0,  -1.0,  -7.0},
	{ 0.0,   3.0,   3.0,   2.0 } 
};

/*
l =

    1.0000         0         0         0
         0    1.0000         0         0
         0   -0.7500    1.0000         0
         0    0.2500   -0.3333    1.0000


u =

    1.0000    1.0000         0    3.0000
         0   -4.0000   -1.0000   -7.0000
         0         0    2.2500   -3.2500
         0         0         0   -4.3333


p =

     1     0     0     0
     0     0     1     0
     0     0     0     1
     0     1     0     0

*/