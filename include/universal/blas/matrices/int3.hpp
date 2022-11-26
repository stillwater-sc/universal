#pragma once
#include <universal/blas/blas.hpp>

// Size = 3 x 3
// Rank 3; 
// Matrix Norm = 10.1867; 
// Cond. =  43.6115
// symmetric = NO
/// pos.def = NO 
// Matrix ID = Integer matrix
// NNZ = 9
sw::universal::blas::matrix<double>  int3 = {
{1.0, 2.0, 3.0},
{2.0, 3.0, 4.0},
{3.0, 4.0, 6.0}
};


/*

L = 
1           0       0
0.3333      1       0   
0.6667      0.5     0

U = 
3   4       6
0   0.6667  1
0   0       -0.5

P = 
0   0   1
1   0   0
0   1   0

*/