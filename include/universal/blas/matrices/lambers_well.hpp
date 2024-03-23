#pragma once
#include <universal/blas/matrix.hpp>

// Matrix ID   = ---
// Size        = 2 x 2 
// Rank        = 2
// Norm        = ~1 
// Cond.       = ~10 
// symmetric   = No
// pos.def     = No   
// NNZ         = 4

/*
2 x 2 in Lambers ENA
See also int3.hpp 
*/

sw::universal::blas::matrix<double>  lambers_well = {
     { 0.45368292, 0.19382865}, 
     { 0.70364726, 0.52104011} 
};

/*
b1 = [0.48554638; 0.87421091];  x = [?,?] \ne [1, 1]

x = [1;1];
c1 = A1*x;

*/