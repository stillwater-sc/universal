#pragma once
#include <universal/blas/matrix.hpp>

// Matrix ID    = Integer matrix
// Size         = 3 x 3
// Rank         = 3 
// Norm         = 10.1867; 
// Cond         = 43.6115
// symmetric    = NO
// Pos.def      = NO 
// NNZ          = 9
/*
Use to check LU decomposition and Amax, etc.  
Easy to view 3 x 3 matrix.  

See also lu4.hpp, q3, q4, q5
*/

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