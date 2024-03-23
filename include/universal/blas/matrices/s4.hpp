#pragma once
#include <universal/blas/matrix.hpp>

// Size = 4 x 4 
// Rank 4; Matrix 1-Norm = 27 ; Cond. = 4.19 
// symmetric = NO
// pos.def = NO   
// Matrix ID = none
// NNZ = 12

sw::universal::blas::matrix<double>  s4 = {
     { 5.0,   1.0,   0.0,   3.0}, 
	{ 0.0,  -7.0,  -1.0,  -5.0},
	{ 0.0,  -4.0,  -8.0,  -7.0},
	{ 0.0,   3.0,   3.0,   12.0 } 
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