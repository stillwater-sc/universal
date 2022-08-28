#pragma once
#include <universal/blas/blas.hpp>

// Size = 3 x 3 
// Rank 3; Matrix Norm = ; Cond. = 15999 
// symmetric = NO
/// pos.def = NO 
// Matrix ID = ---
// NNZ = 9

sw::universal::blas::matrix<double>  faires74x3 = {
        { 3.3330, 15920.0,  -10.333}, 
		{ 2.2220, 16.710,    9.6120},
		{ 1.5611,  5.1791,   1.6852} 
};

/* 
If b = {15913.0, 28.544, 8.4254}  
Then for Ax = b, we have x = {1, 1, 1}

A^{-1 } = {
        { -1.1701e-04, -1.4983e-01,  8.5416e-01}, 
		{  6.2782e-05,  1.2124e-04, -3.0662e-04},
		{ -8.6631e-05,  1.3846e-01, -1.9689e-01} 
};
*/