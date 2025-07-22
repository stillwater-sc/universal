#pragma once
#include <universal/blas/matrix.hpp>

// Matrix ID 	= x
// Size 		= 5 x 5 
// Rank 		= 5 
// Norm 		= 66000; 
// Cond 		= 1.1e+04
// symmetric 	= No
// pos.def 		= No 
// NNZ 			= 16

sw::universal::blas::matrix<double> q5 = {
    	{ 10.0, -1.0,  2.0,  0.0, 0.0}, 
		{ -1.0, 11.0, -1.0,  3.0, 0.0},
		{  2.0, -1.0, 10.0, -1.0, 0.0},
		{  0.0,  3.0, -1.0,  8.0, 0.0}, 
        {  0.0,  0.0,  0.0,  3.2, -66000} 
};

// Let b = {11, 12, 10, 10 ,66003} for exact solution x = {1,1,1,1,1}.

