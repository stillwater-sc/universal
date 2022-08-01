#pragma once
#include <universal/blas/blas.hpp>

// Size = 4 x 4 
// Rank 4; Matrix Norm = ; Cond. =  
// symmetric = YES
/// pos.def = NO 
// Matrix ID = ---
// NNZ = 14

sw::universal::blas::matrix<double>  q4 = {
        { 10.0, -1.0,  2.0,  0.0}, 
		{ -1.0, 11.0, -1.0,  3.0},
		{  2.0, -1.0, 10.0, -1.0},
		{  0.0,  3.0, -1.0,  8.0 } 
};