#pragma once
#include <universal/blas/matrix.hpp>

// Matrix ID   = ---
// Size        = 2 x 2 
// Rank        = 2
// Norm        = ~1; 
// Cond.       = 1.869050824603144e+08
// symmetric   = No
// pos.def     = No   
// NNZ         = 4

/*
2 x 2 in Lambers ENA
See also int3.hpp 
*/

sw::universal::blas::matrix<double>  lambers_ill = {
     { 0.40563526, 0.26686200 }, 
     { 0.73033346, 0.48047658 } 
};

/*
A2 = [0.40563526 0.26686200; 0.73033346 0.48047658];
cond(A2)
c2 = A2*x;
b2 = [0.39812529; 0.92276554];

*/