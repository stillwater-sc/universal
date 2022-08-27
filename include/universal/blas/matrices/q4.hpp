#pragma once
#include <universal/blas/blas.hpp>

// Size = 4 x 4 
// Rank 4; Matrix Norm = 14.x; Cond. =  2.35
// symmetric = YES
/// pos.def = YES 
// Matrix ID = x
// NNZ = 14

sw::universal::blas::matrix<double>  q4 = {
        { 10.0, -1.0,  2.0,  0.0}, 
		{ -1.0, 11.0, -1.0,  3.0},
		{  2.0, -1.0, 10.0, -1.0},
		{  0.0,  3.0, -1.0,  8.0 } 
};

// Let b = {11, 12, 10, 10} for exact solution x = {1,1,1,1}.